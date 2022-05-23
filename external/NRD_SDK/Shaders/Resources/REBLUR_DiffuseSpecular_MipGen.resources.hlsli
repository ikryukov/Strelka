/*
Copyright (c) 2022, NVIDIA CORPORATION. All rights reserved.

NVIDIA CORPORATION and its licensors retain all intellectual property
and proprietary rights in and to this software, related documentation
and any modifications thereto. Any use, reproduction, disclosure or
distribution of this software and related documentation without an express
license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

NRD_SAMPLER_START
    NRD_SAMPLER( SamplerState, gNearestClamp, s, 0 )
    NRD_SAMPLER( SamplerState, gNearestMirror, s, 1 )
    NRD_SAMPLER( SamplerState, gLinearClamp, s, 2 )
    NRD_SAMPLER( SamplerState, gLinearMirror, s, 3 )
NRD_SAMPLER_END

NRD_CONSTANTS_START
    REBLUR_SHARED_CB_DATA
NRD_CONSTANTS_END

#if( defined REBLUR_DIFFUSE && defined REBLUR_SPECULAR )

    NRD_INPUT_TEXTURE_START
        NRD_INPUT_TEXTURE( Texture2D<float4>, gIn_Normal_Roughness, t, 0 )
        NRD_INPUT_TEXTURE( Texture2D<float4>, gIn_Diff, t, 1 )
        NRD_INPUT_TEXTURE( Texture2D<float4>, gIn_Spec, t, 2 )
    NRD_INPUT_TEXTURE_END

    NRD_OUTPUT_TEXTURE_START
        NRD_OUTPUT_TEXTURE( RWTexture2D<float4>, gOut_Diff, u, 0 )
        NRD_OUTPUT_TEXTURE( RWTexture2D<float4>, gOut_Spec, u, 1 )
        NRD_OUTPUT_TEXTURE( RWTexture2D<float>, gIn_ScaledViewZ, u, 2 )
        NRD_OUTPUT_TEXTURE( RWTexture2D<float4>, gOut_Diff_x2, u, 3 )
        NRD_OUTPUT_TEXTURE( RWTexture2D<float4>, gOut_Spec_x2, u, 4 )
        NRD_OUTPUT_TEXTURE( RWTexture2D<float>, gOut_ScaledViewZ_x2, u, 5 )
        NRD_OUTPUT_TEXTURE( RWTexture2D<float4>, gOut_Diff_x4, u, 6 )
        NRD_OUTPUT_TEXTURE( RWTexture2D<float4>, gOut_Spec_x4, u, 7 )
        NRD_OUTPUT_TEXTURE( RWTexture2D<float>, gOut_ScaledViewZ_x4, u, 8 )
        NRD_OUTPUT_TEXTURE( RWTexture2D<float4>, gOut_Diff_x8, u, 9 )
        NRD_OUTPUT_TEXTURE( RWTexture2D<float4>, gOut_Spec_x8, u, 10 )
        NRD_OUTPUT_TEXTURE( RWTexture2D<float>, gOut_ScaledViewZ_x8, u, 11 )
    NRD_OUTPUT_TEXTURE_END

#elif( defined REBLUR_DIFFUSE )

    NRD_INPUT_TEXTURE_START
        NRD_INPUT_TEXTURE( Texture2D<float4>, gIn_Diff, t, 0 )
    NRD_INPUT_TEXTURE_END

    NRD_OUTPUT_TEXTURE_START
        NRD_OUTPUT_TEXTURE( RWTexture2D<float4>, gOut_Diff, u, 0 )
        NRD_OUTPUT_TEXTURE( RWTexture2D<float>, gIn_ScaledViewZ, u, 1 )
        NRD_OUTPUT_TEXTURE( RWTexture2D<float4>, gOut_Diff_x2, u, 2 )
        NRD_OUTPUT_TEXTURE( RWTexture2D<float>, gOut_ScaledViewZ_x2, u, 3 )
        NRD_OUTPUT_TEXTURE( RWTexture2D<float4>, gOut_Diff_x4, u, 4 )
        NRD_OUTPUT_TEXTURE( RWTexture2D<float>, gOut_ScaledViewZ_x4, u, 5 )
        NRD_OUTPUT_TEXTURE( RWTexture2D<float4>, gOut_Diff_x8, u, 6 )
        NRD_OUTPUT_TEXTURE( RWTexture2D<float>, gOut_ScaledViewZ_x8, u, 7 )
    NRD_OUTPUT_TEXTURE_END

#else

    NRD_INPUT_TEXTURE_START
        NRD_INPUT_TEXTURE( Texture2D<float4>, gIn_Normal_Roughness, t, 0 )
        NRD_INPUT_TEXTURE( Texture2D<float4>, gIn_Spec, t, 1 )
    NRD_INPUT_TEXTURE_END

    NRD_OUTPUT_TEXTURE_START
        NRD_OUTPUT_TEXTURE( RWTexture2D<float4>, gOut_Spec, u, 0 )
        NRD_OUTPUT_TEXTURE( RWTexture2D<float>, gIn_ScaledViewZ, u, 1 )
        NRD_OUTPUT_TEXTURE( RWTexture2D<float4>, gOut_Spec_x2, u, 2 )
        NRD_OUTPUT_TEXTURE( RWTexture2D<float>, gOut_ScaledViewZ_x2, u, 3 )
        NRD_OUTPUT_TEXTURE( RWTexture2D<float4>, gOut_Spec_x4, u, 4 )
        NRD_OUTPUT_TEXTURE( RWTexture2D<float>, gOut_ScaledViewZ_x4, u, 5 )
        NRD_OUTPUT_TEXTURE( RWTexture2D<float4>, gOut_Spec_x8, u, 6 )
        NRD_OUTPUT_TEXTURE( RWTexture2D<float>, gOut_ScaledViewZ_x8, u, 7 )
    NRD_OUTPUT_TEXTURE_END

#endif

// Macro magic
#if( REBLUR_USE_5X5_ANTI_FIREFLY == 1 )
    #define NRD_USE_BORDER_2
#endif

#define NRD_CTA_8X8
