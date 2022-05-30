/*
Copyright (c) 2022, NVIDIA CORPORATION. All rights reserved.

NVIDIA CORPORATION and its licensors retain all intellectual property
and proprietary rights in and to this software, related documentation
and any modifications thereto. Any use, reproduction, disclosure or
distribution of this software and related documentation without an express
license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include "Poisson.hlsli"

// Constants

#define NRD_FRAME                                               0
#define NRD_PIXEL                                               1
#define NRD_RANDOM                                              2 // for experiments only

#define NRD_INF                                                 1e6

// FP16

#ifdef NRD_COMPILER_DXC
    #define half_float float16_t
    #define half_float2 float16_t2
    #define half_float3 float16_t3
    #define half_float4 float16_t4
#else
    #define half_float float
    #define half_float2 float2
    #define half_float3 float3
    #define half_float4 float4
#endif

//==================================================================================================================
// DEFAULT SETTINGS (can be modified)
//==================================================================================================================

#define NRD_USE_QUADRATIC_DISTRIBUTION                          0 // bool
#define NRD_BILATERAL_WEIGHT_VIEWZ_SENSITIVITY                  100.0 // w = 1 / ( 1 + this * z )
#define NRD_BILATERAL_WEIGHT_CUTOFF                             0.03 // normalized % // TODO: 1-2%?
#define NRD_CATROM_SHARPNESS                                    0.5 // [ 0; 1 ], 0.5 matches Catmull-Rom
#define NRD_NORMAL_ENCODING_ERROR                               STL::Math::DegToRad( 0.5 )
#define NRD_PARALLAX_NORMALIZATION                              30.0
#define NRD_RADIANCE_COMPRESSION_MODE                           3 // 0-4, specular color compression for spatial passes

//==================================================================================================================
// CTA & preloading
//==================================================================================================================

#if 0 // CTA override
    #define GROUP_X                                             16
    #define GROUP_Y                                             16
#else
    #ifdef NRD_CTA_8X8
        #define GROUP_X                                         8
        #define GROUP_Y                                         8
    #else
        #define GROUP_X                                         16
        #define GROUP_Y                                         16
    #endif
#endif

#ifdef NRD_USE_BORDER_2
    #define BORDER                                              2
#else
    #define BORDER                                              1
#endif

#define BUFFER_X                                                ( GROUP_X + BORDER * 2 )
#define BUFFER_Y                                                ( GROUP_Y + BORDER * 2 )

#define PRELOAD_INTO_SMEM \
    int2 groupBase = pixelPos - threadPos - BORDER; \
    uint stageNum = ( BUFFER_X * BUFFER_Y + GROUP_X * GROUP_Y - 1 ) / ( GROUP_X * GROUP_Y ); \
    [unroll] \
    for( uint stage = 0; stage < stageNum; stage++ ) \
    { \
        uint virtualIndex = threadIndex + stage * GROUP_X * GROUP_Y; \
        uint2 newId = uint2( virtualIndex % BUFFER_X, virtualIndex / BUFFER_X ); \
        if( stage == 0 || virtualIndex < BUFFER_X * BUFFER_Y ) \
            Preload( newId, groupBase + newId ); \
    } \
    GroupMemoryBarrierWithGroupSync( )

//==================================================================================================================
// SHARED FUNCTIONS
//==================================================================================================================

// Misc

// sigma = standard deviation, variance = sigma ^ 2
#define GetStdDev( m1, m2 )                     sqrt( abs( ( m2 ) - ( m1 ) * ( m1 ) ) ) // sqrt( max( m2 - m1 * m1, 0.0 ) )

#if( NRD_USE_MATERIAL_ID == 1 )
    #define CompareMaterials( m0, m, mask )     ( ( mask ) == 0 ? 1.0 : ( ( m0 ) == ( m ) ) )
#else
    #define CompareMaterials( m0, m, mask )     1.0
#endif

float PixelRadiusToWorld( float unproject, float orthoMode, float pixelRadius, float viewZ )
{
     return pixelRadius * unproject * lerp( viewZ, 1.0, abs( orthoMode ) );
}

float GetHitDistFactor( float hitDist, float frustumHeight, float scale = 1.0 )
{
    return saturate( hitDist / ( hitDist * scale + frustumHeight ) );
}

float4 GetBlurKernelRotation( compiletime const uint mode, uint2 pixelPos, float4 baseRotator, uint frameIndex )
{
    if( mode == NRD_PIXEL )
    {
        float angle = STL::Sequence::Bayer4x4( pixelPos, frameIndex );
        float4 rotator = STL::Geometry::GetRotator( angle * STL::Math::Pi( 2.0 ) );

        baseRotator = STL::Geometry::CombineRotators( baseRotator, rotator );
    }
    else if( mode == NRD_RANDOM )
    {
        STL::Rng::Initialize( pixelPos, frameIndex );

        float2 rnd = STL::Rng::GetFloat2( );
        float4 rotator = STL::Geometry::GetRotator( rnd.x * STL::Math::Pi( 2.0 ) );
        rotator *= 1.0 + ( rnd.y * 2.0 - 1.0 ) * 0.5;

        baseRotator = STL::Geometry::CombineRotators( baseRotator, rotator );
    }

    return baseRotator;
}

float IsInScreen( float2 uv )
{
    return float( all( saturate( uv ) == uv ) );
}

float2 ApplyCheckerboard( inout float2 uv, uint mode, uint counter, float2 screenSize, float2 invScreenSize, uint frameIndex )
{
    int2 uvi = int2( uv * screenSize );
    bool hasData = STL::Sequence::CheckerBoard( uvi, frameIndex ) == mode;
    if( !hasData )
        uvi.x += ( ( counter & 0x1 ) == 0 ) ? -1 : 1;
    uv = ( float2( uvi ) + 0.5 ) * invScreenSize;

    return float2( uv.x * 0.5, uv.y );
}

float GetSpecMagicCurve( float roughness, float power = 0.25 )
{
    // http://fooplot.com/#W3sidHlwZSI6MCwiZXEiOiIxLjAtMl4oLTE1LjAqeCkiLCJjb2xvciI6IiNGMjE4MTgifSx7InR5cGUiOjAsImVxIjoiKDEtMl4oLTIwMCp4KngpKSooeF4wLjI1KSIsImNvbG9yIjoiIzIyRUQxNyJ9LHsidHlwZSI6MCwiZXEiOiIoMS0yXigtMjAwKngqeCkpKih4XjAuNSkiLCJjb2xvciI6IiMxNzE2MTYifSx7InR5cGUiOjEwMDAsIndpbmRvdyI6WyIwIiwiMSIsIjAiLCIxLjEiXSwic2l6ZSI6WzEwMDAsNTAwXX1d

    float f = 1.0 - exp2( -200.0 * roughness * roughness );
    f *= STL::Math::Pow01( roughness, power );

    return f;
}

float ComputeParallax( float3 Xprev, float2 pixelUv, float2 pixelUvPrev, float4x4 mWorldToClip, float3 cameraDelta, float2 rectSize, float unproject, float orthoMode )
{
    float3 Xt = Xprev - cameraDelta;

    float3 clip = STL::Geometry::ProjectiveTransform( mWorldToClip, Xt ).xyw;
    clip.xy /= clip.z;
    clip.y = -clip.y;

    float2 uv = clip.xy * 0.5 + 0.5;
    float2 parallaxInUv = ( orthoMode == 0.0 ? pixelUv : pixelUvPrev ) - uv;
    float parallaxInPixels = length( parallaxInUv * rectSize );

    float radius = PixelRadiusToWorld( unproject, orthoMode, parallaxInPixels, clip.z );
    float distance = clip.z; // TODO: distance to the point?
    float parallax = radius / distance;

    return parallax * NRD_PARALLAX_NORMALIZATION;
}

float GetParallaxInPixels( float parallax, float unproject )
{
    float parallaxInPixels = parallax / ( NRD_PARALLAX_NORMALIZATION * unproject );

    return parallaxInPixels;
}

float GetColorCompressionExposureForSpatialPasses( float roughness )
{
    // Prerequsites:
    // - to minimize biasing the results compression for high roughness should be avoided (diffuse signal compression can lead to darker image)
    // - the compression function must be monotonic for full roughness range
    // - returned exposure must be used with colors in the HDR range used in tonemapping, i.e. "color * exposure"

    // http://fooplot.com/#W3sidHlwZSI6MCwiZXEiOiIwLjUvKDErNTAqeCkiLCJjb2xvciI6IiNGNzBBMEEifSx7InR5cGUiOjAsImVxIjoiMC41KigxLXgpLygxKzYwKngpIiwiY29sb3IiOiIjMkJGRjAwIn0seyJ0eXBlIjowLCJlcSI6IjAuNSooMS14KS8oMSsxMDAwKngqeCkrKDEteF4wLjUpKjAuMDMiLCJjb2xvciI6IiMwMDU1RkYifSx7InR5cGUiOjAsImVxIjoiMC42KigxLXgqeCkvKDErNDAwKngqeCkiLCJjb2xvciI6IiMwMDAwMDAifSx7InR5cGUiOjEwMDAsIndpbmRvdyI6WyIwIiwiMSIsIjAiLCIxIl0sInNpemUiOlsyOTUwLDk1MF19XQ--

    // Moderate compression
    #if( NRD_RADIANCE_COMPRESSION_MODE == 1 )
        return 0.5 / ( 1.0 + 50.0 * roughness );
    // Less compression for mid-high roughness
    #elif( NRD_RADIANCE_COMPRESSION_MODE == 2 )
        return 0.5 * ( 1.0 - roughness ) / ( 1.0 + 60.0 * roughness );
    // Close to the previous one, but offers more compression for low roughness
    #elif( NRD_RADIANCE_COMPRESSION_MODE == 3 )
        return 0.5 * ( 1.0 - roughness ) / ( 1.0 + 1000.0 * roughness * roughness ) + ( 1.0 - sqrt( saturate( roughness ) ) ) * 0.03;
    // A modification of the preious one ( simpler )
    #elif( NRD_RADIANCE_COMPRESSION_MODE == 4 )
        return 0.6 * ( 1.0 - roughness * roughness ) / ( 1.0 + 400.0 * roughness * roughness );
    // No compression
    #else
        return 0;
    #endif
}

// Thin lens

float EstimateCurvature( float3 Ni, float3 Vi, float3 N, float3 X )
{
    // https://computergraphics.stackexchange.com/questions/1718/what-is-the-simplest-way-to-compute-principal-curvature-for-a-mesh-triangle

    float NoV = dot( Vi, N );
    float3 Xi = 0 + Vi * dot( X - 0, N ) / NoV;
    float3 edge = Xi - X;
    float edgeLenSq = STL::Math::LengthSquared( edge );
    float curvature = dot( Ni - N, edge ) / edgeLenSq;

    return curvature;
}

float ApplyThinLensEquation( float NoV, float hitDist, float curvature )
{
    /*
    Why NoV?

    hitDist is not O, we need to find projection to the axis:
        O = hitDist * NoV

    hitDistFocused is not I, we need to reproject it back to the view direction:
        hitDistFocused = I / NoV

    Combine:
        I = 0.5 * O / ( 0.5 + C * O )
        hitDistFocused = [ 0.5 * NoV * hitDist / ( 0.5 + C * NoV * hitDist ) ] / NoV
    */

    float hitDistFocused = 0.5 * hitDist / ( 0.5 + curvature * NoV * hitDist );

    return hitDistFocused;
}

float3 GetXvirtual(
    float NoV, float hitDist, float curvature,
    float3 X, float3 Xprev, float3 V,
    float roughness )
{
    /*
    C - curvature
    O - object distance
    I - image distance
    F - focal distance

    The lens equation:
        [Eq 1] 1 / O + 1 / I = 1 / F
        [Eq 2] For a spherical mirror F = -0.5 * R
        [Eq 3] R = 1 / C

    Find I from [Eq 1]:
        1 / I = 1 / F - 1 / O
        1 / I = ( O - F ) / ( F * O )
        I = F * O / ( O - F )

    Apply [Eq 2]:
        I = -0.5 * R * O / ( O + 0.5 * R )

    Apply [Eq 3]:
        I = ( -0.5 * O / C ) / ( O + 0.5 / C )
        I = ( -0.5 * O / C ) / ( ( O * C + 0.5 ) / C )
        I = ( -0.5 * O / C ) * ( C / ( O * C + 0.5 ) )
        I = -0.5 * O / ( 0.5 + C * O )

    Reverse sign because I is negative:
        I = 0.5 * O / ( 0.5 + C * O )
    */

    float hitDistFocused = ApplyThinLensEquation( NoV, hitDist, curvature );

    // "saturate" is needed to clamp values > 1 if curvature is negative
    float compressionRatio = saturate( ( abs( hitDistFocused ) + 1e-6 ) / ( hitDist + 1e-6 ) );

    float f = STL::ImportanceSampling::GetSpecularDominantFactor( NoV, roughness, STL_SPECULAR_DOMINANT_DIRECTION_G2 );
    float3 Xvirtual = lerp( Xprev, X, compressionRatio * f ) - V * hitDistFocused * f;

    return Xvirtual;
}

// Kernel

float2x3 GetKernelBasis( float3 D, float3 N, float NoD, float worldRadius, float roughness = 1.0, float anisoFade = 1.0 )
{
    float3x3 basis = STL::Geometry::GetBasis( N );

    float3 T = basis[ 0 ];
    float3 B = basis[ 1 ];

    if( roughness < 0.95 && NoD < 0.999 )
    {
        float3 R = reflect( -D, N );
        T = normalize( cross( N, R ) );
        B = cross( R, T );

        float skewFactor = lerp( roughness, 1.0, NoD );
        T *= lerp( skewFactor, 1.0, anisoFade );
    }

    T *= worldRadius;
    B *= worldRadius;

    return float2x3( T, B );
}

float2 GetKernelSampleCoordinates( float4x4 mToClip, float3 offset, float3 X, float3 T, float3 B, float4 rotator = float4( 1, 0, 0, 1 ) )
{
    #if( NRD_USE_QUADRATIC_DISTRIBUTION == 1 )
        offset.xy *= offset.z;
    #endif

    // We can't rotate T and B instead, because T is skewed
    offset.xy = STL::Geometry::RotateVector( rotator, offset.xy );

    float3 p = X + T * offset.x + B * offset.y;
    float3 clip = STL::Geometry::ProjectiveTransform( mToClip, p ).xyw;
    clip.xy /= clip.z;
    clip.y = -clip.y;

    float2 uv = clip.xy * 0.5 + 0.5;

    return uv;
}

// Weight parameters

float2 GetGeometryWeightParams( float planeDistSensitivity, float frustumHeight, float3 Xv, float3 Nv, float scale = 1.0 )
{
    float a = scale / ( planeDistSensitivity * frustumHeight + 1e-6 );
    float b = -dot( Nv, Xv ) * a;

    return float2( a, b );
}

float2 GetHitDistanceWeightParams( float normHitDist, float nonLinearAccumSpeed )
{
    float a = 1.0 / nonLinearAccumSpeed; // TODO: previously was "1.0 / ( normHitDist * F( roughness) * 0.99 + 0.01 )"
    float b = normHitDist * a;

    return float2( a, -b );
}

// Weights

float ExpApprox( float x ) // TODO: use for all weights? definitely a must for "noisy" data comparison when confidence is unclear
{
    // IMPORTANT:
    // - works for "negative x" only
    // - huge error for x < -2, but still applicable for "weight" calculations
    // http://fooplot.com/#W3sidHlwZSI6MCwiZXEiOiJleHAoeCkiLCJjb2xvciI6IiMwMDAwMDAifSx7InR5cGUiOjAsImVxIjoiMS8oeCp4LXgrMSkiLCJjb2xvciI6IiMwRkIwMDAifSx7InR5cGUiOjEwMDAsIndpbmRvdyI6WyItMTAiLCIwIiwiMCIsIjEiXX1d

    return rcp( x * x - x + 1.0 );
}

#define _ComputeWeight( params, value ) STL::Math::SmoothStep01( 1.0 - abs( value * params.x + params.y ) )

#define GetRoughnessWeight( params, value ) _ComputeWeight( params, value )

float GetHitDistanceWeight( float2 params, float hitDist )
{
    // Comparison:
    // http://fooplot.com/#W3sidHlwZSI6MCwiZXEiOiIxLWFicygoeC0wLjUpLzAuMikiLCJjb2xvciI6IiMwMDAwMDAifSx7InR5cGUiOjAsImVxIjoiZXhwKC0yKmFicygoeC0wLjUpLzAuMikpIiwiY29sb3IiOiIjRkYwMDE1In0seyJ0eXBlIjowLCJlcSI6ImV4cCgtNSphYnMoKHgtMC41KS8wLjIpKSIsImNvbG9yIjoiIzAwQTgyNyJ9LHsidHlwZSI6MTAwMCwid2luZG93IjpbIjAiLCIxIiwiMCIsIjEiXX1d

    #if 1
        // "Tail" given by approximation is much higher than the original one,
        // -5.0 helps to preserve same energy comparing with "weight with cut off"
        float x = abs( hitDist * params.x + params.y );
        return ExpApprox( -5.0 * x );
    #else
        return _ComputeWeight( params, hitDist );
    #endif
}

float GetGeometryWeight( float2 params, float3 n0, float3 p )
{
    float d = dot( n0, p );

    return _ComputeWeight( params, d );
}

float GetGaussianWeight( float r )
{
    return exp( -0.66 * r * r ); // assuming r is normalized to 1
}

#define _GetBilateralWeight( z, zc ) \
    z = abs( z - zc ) * rcp( min( abs( z ), abs( zc ) ) + 0.001 ); \
    z = rcp( 1.0 + NRD_BILATERAL_WEIGHT_VIEWZ_SENSITIVITY * z ) * step( z, NRD_BILATERAL_WEIGHT_CUTOFF );

float GetBilateralWeight( float z, float zc )
{ _GetBilateralWeight( z, zc ); return z; }

float2 GetBilateralWeight( float2 z, float zc )
{ _GetBilateralWeight( z, zc ); return z; }

float4 GetBilateralWeight( float4 z, float zc )
{ _GetBilateralWeight( z, zc ); return z; }

// Upsampling

#define _BicubicFilterNoCornersWithFallbackToBilinearFilterWithCustomWeights_Init \
    /* Catmul-Rom with 12 taps ( excluding corners ) */ \
    float2 centerPos = floor( samplePos - 0.5 ) + 0.5; \
    float2 f = saturate( samplePos - centerPos ); \
    float2 f2 = f * f; \
    float2 f3 = f * f2; \
    float2 w0 = -NRD_CATROM_SHARPNESS * f3 + 2.0 * NRD_CATROM_SHARPNESS * f2 - NRD_CATROM_SHARPNESS * f; \
    float2 w1 = ( 2.0 - NRD_CATROM_SHARPNESS ) * f3 - ( 3.0 - NRD_CATROM_SHARPNESS ) * f2 + 1.0; \
    float2 w2 = -( 2.0 - NRD_CATROM_SHARPNESS ) * f3 + ( 3.0 - 2.0 * NRD_CATROM_SHARPNESS ) * f2 + NRD_CATROM_SHARPNESS * f; \
    float2 w3 = NRD_CATROM_SHARPNESS * f3 - NRD_CATROM_SHARPNESS * f2; \
    float2 w12 = w1 + w2; \
    float2 tc = w2 / w12; \
    float4 w; \
    w.x = w12.x * w0.y; \
    w.y = w0.x * w12.y; \
    w.z = w12.x * w12.y; \
    w.w = w3.x * w12.y; \
    float w4 = w12.x * w3.y; \
    /* Fallback to custom bilinear */ \
    w = useBicubic ? w : bilinearCustomWeights; \
    w4 = useBicubic ? w4 : 0.0; \
    float sum = dot( w, 1.0 ) + w4; \
    /* Texture coordinates */ \
    float2 uv0 = centerPos + ( useBicubic ? float2( tc.x, -1.0 ) : float2( 0, 0 ) ); \
    float2 uv1 = centerPos + ( useBicubic ? float2( -1.0, tc.y ) : float2( 1, 0 ) ); \
    float2 uv2 = centerPos + ( useBicubic ? float2( tc.x, tc.y ) : float2( 0, 1 ) ); \
    float2 uv3 = centerPos + ( useBicubic ? float2( 2.0, tc.y ) : float2( 1, 1 ) ); \
    float2 uv4 = centerPos + ( useBicubic ? float2( tc.x, 2.0 ) : f ); // can be used to get a free bilinear sample after some massaging

// IMPORTANT: 0 can be returned if only a single tap is valid from the 2x2 footprint and pure bilinear
// weights are close to 0 near this tap. The caller must handle this case manually. "Footprint quality"
// can be used to accelerate accumulation and avoid the problem.
#define _BicubicFilterNoCornersWithFallbackToBilinearFilterWithCustomWeights_Color( color, tex ) \
    /* Sampling */ \
    color = tex.SampleLevel( linearSampler, uv0 * invTextureSize, 0 ) * w.x; \
    color += tex.SampleLevel( linearSampler, uv1 * invTextureSize, 0 ) * w.y; \
    color += tex.SampleLevel( linearSampler, uv2 * invTextureSize, 0 ) * w.z; \
    color += tex.SampleLevel( linearSampler, uv3 * invTextureSize, 0 ) * w.w; \
    color += tex.SampleLevel( linearSampler, uv4 * invTextureSize, 0 ) * w4; \
    /* Normalize similarly to "STL::Filtering::ApplyBilinearCustomWeights()" */ \
    color = sum < 0.0001 ? 0 : color * rcp( sum ); \
    /* Avoid negative values from CatRom, but doesn't suit for YCoCg or negative input */ \
    color = max( color, 0 )

void BicubicFilterNoCornersWithFallbackToBilinearFilterWithCustomWeights(
    float2 samplePos, float2 invTextureSize, float4 bilinearCustomWeights,
    SamplerState linearSampler, bool useBicubic,
    Texture2D<float4> tex0, out float4 c0 )
{
    _BicubicFilterNoCornersWithFallbackToBilinearFilterWithCustomWeights_Init;
    _BicubicFilterNoCornersWithFallbackToBilinearFilterWithCustomWeights_Color( c0, tex0 );
}

void BicubicFilterNoCornersWithFallbackToBilinearFilterWithCustomWeights(
    float2 samplePos, float2 invTextureSize, float4 bilinearCustomWeights,
    SamplerState linearSampler, bool useBicubic,
    Texture2D<float4> tex0, out float4 c0,
    Texture2D<float4> tex1, out float4 c1 )
{
    _BicubicFilterNoCornersWithFallbackToBilinearFilterWithCustomWeights_Init;
    _BicubicFilterNoCornersWithFallbackToBilinearFilterWithCustomWeights_Color( c0, tex0 );
    _BicubicFilterNoCornersWithFallbackToBilinearFilterWithCustomWeights_Color( c1, tex1 );
}

void BicubicFilterNoCornersWithFallbackToBilinearFilterWithCustomWeights(
    float2 samplePos, float2 invTextureSize, float4 bilinearCustomWeights,
    SamplerState linearSampler, bool useBicubic,
    Texture2D<float4> tex0, out float4 c0,
    Texture2D<float4> tex1, out float4 c1,
    Texture2D<float4> tex2, out float4 c2,
    Texture2D<float4> tex3, out float4 c3 )
{
    _BicubicFilterNoCornersWithFallbackToBilinearFilterWithCustomWeights_Init;
    _BicubicFilterNoCornersWithFallbackToBilinearFilterWithCustomWeights_Color( c0, tex0 );
    _BicubicFilterNoCornersWithFallbackToBilinearFilterWithCustomWeights_Color( c1, tex1 );
    _BicubicFilterNoCornersWithFallbackToBilinearFilterWithCustomWeights_Color( c2, tex2 );
    _BicubicFilterNoCornersWithFallbackToBilinearFilterWithCustomWeights_Color( c3, tex3 );
}

void BicubicFilterNoCornersWithFallbackToBilinearFilterWithCustomWeights1(
    float2 samplePos, float2 invTextureSize, float4 bilinearCustomWeights,
    SamplerState linearSampler, bool useBicubic,
    Texture2D<float> tex0, out float c0 )
{
    _BicubicFilterNoCornersWithFallbackToBilinearFilterWithCustomWeights_Init;
    _BicubicFilterNoCornersWithFallbackToBilinearFilterWithCustomWeights_Color( c0, tex0 );
}

void BicubicFilterNoCornersWithFallbackToBilinearFilterWithCustomWeights1(
    float2 samplePos, float2 invTextureSize, float4 bilinearCustomWeights,
    SamplerState linearSampler, bool useBicubic,
    Texture2D<float> tex0, out float c0,
    Texture2D<float> tex1, out float c1 )
{
    _BicubicFilterNoCornersWithFallbackToBilinearFilterWithCustomWeights_Init;
    _BicubicFilterNoCornersWithFallbackToBilinearFilterWithCustomWeights_Color( c0, tex0 );
    _BicubicFilterNoCornersWithFallbackToBilinearFilterWithCustomWeights_Color( c1, tex1 );
}

void BicubicFilterNoCorners(
    float2 samplePos, float2 invTextureSize,
    SamplerState linearSampler, bool useBicubic,
    Texture2D<float4> tex0, out float4 c0 )
{
    if( !useBicubic )
    {
        c0 = tex0.SampleLevel( linearSampler, samplePos * invTextureSize, 0 );

        return;
    }

    float4 bilinearCustomWeights = 0;

    _BicubicFilterNoCornersWithFallbackToBilinearFilterWithCustomWeights_Init;
    _BicubicFilterNoCornersWithFallbackToBilinearFilterWithCustomWeights_Color( c0, tex0 );
}

void BicubicFilterNoCorners(
    float2 samplePos, float2 invTextureSize,
    SamplerState linearSampler, bool useBicubic,
    Texture2D<float4> tex0, out float4 c0,
    Texture2D<float4> tex1, out float4 c1 )
{
    if( !useBicubic )
    {
        c0 = tex0.SampleLevel( linearSampler, samplePos * invTextureSize, 0 );
        c1 = tex1.SampleLevel( linearSampler, samplePos * invTextureSize, 0 );

        return;
    }

    float4 bilinearCustomWeights = 0;

    _BicubicFilterNoCornersWithFallbackToBilinearFilterWithCustomWeights_Init;
    _BicubicFilterNoCornersWithFallbackToBilinearFilterWithCustomWeights_Color( c0, tex0 );
    _BicubicFilterNoCornersWithFallbackToBilinearFilterWithCustomWeights_Color( c1, tex1 );
}

void BicubicFilterNoCorners(
    float2 samplePos, float2 invTextureSize,
    SamplerState linearSampler, bool useBicubic,
    Texture2D<float> tex0, out float c0 )
{
    if( !useBicubic )
    {
        c0 = tex0.SampleLevel( linearSampler, samplePos * invTextureSize, 0 );

        return;
    }

    float4 bilinearCustomWeights = 0;

    _BicubicFilterNoCornersWithFallbackToBilinearFilterWithCustomWeights_Init;
    _BicubicFilterNoCornersWithFallbackToBilinearFilterWithCustomWeights_Color( c0, tex0 );
}
