#include "Light.h"

#include <pxr/imaging/hd/instancer.h>
#include <pxr/imaging/hd/meshUtil.h>
#include <pxr/imaging/hd/smoothNormals.h>
#include <pxr/imaging/hd/vertexAdjacency.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/compatibility.hpp>

PXR_NAMESPACE_OPEN_SCOPE

//  Lookup table from:
//  Colour Rendering of Spectra
//  by John Walker
//  https://www.fourmilab.ch/documents/specrend/specrend.c
//
//  Covers range from 1000k to 10000k in 500k steps
//  assuming Rec709 / sRGB colorspace chromaticity.
//
// NOTE: 6500K doesn't give a pure white because the D65
//       illuminant used by Rec. 709 doesn't lie on the
//       Planckian Locus. We would need to compute the
//       Correlated Colour Temperature (CCT) using Ohno's
//       method to get pure white. Maybe one day.
//
// Note that the beginning and ending knots are repeated to simplify
// boundary behavior.  The last 4 knots represent the segment starting
// at 1.0.
//
static GfVec3f const _blackbodyRGB[] = {
    GfVec3f(1.000000f, 0.027490f, 0.000000f), //  1000 K (Approximation)
    GfVec3f(1.000000f, 0.027490f, 0.000000f), //  1000 K (Approximation)
    GfVec3f(1.000000f, 0.149664f, 0.000000f), //  1500 K (Approximation)
    GfVec3f(1.000000f, 0.256644f, 0.008095f), //  2000 K
    GfVec3f(1.000000f, 0.372033f, 0.067450f), //  2500 K
    GfVec3f(1.000000f, 0.476725f, 0.153601f), //  3000 K
    GfVec3f(1.000000f, 0.570376f, 0.259196f), //  3500 K
    GfVec3f(1.000000f, 0.653480f, 0.377155f), //  4000 K
    GfVec3f(1.000000f, 0.726878f, 0.501606f), //  4500 K
    GfVec3f(1.000000f, 0.791543f, 0.628050f), //  5000 K
    GfVec3f(1.000000f, 0.848462f, 0.753228f), //  5500 K
    GfVec3f(1.000000f, 0.898581f, 0.874905f), //  6000 K
    GfVec3f(1.000000f, 0.942771f, 0.991642f), //  6500 K
    GfVec3f(0.906947f, 0.890456f, 1.000000f), //  7000 K
    GfVec3f(0.828247f, 0.841838f, 1.000000f), //  7500 K
    GfVec3f(0.765791f, 0.801896f, 1.000000f), //  8000 K
    GfVec3f(0.715255f, 0.768579f, 1.000000f), //  8500 K
    GfVec3f(0.673683f, 0.740423f, 1.000000f), //  9000 K
    GfVec3f(0.638992f, 0.716359f, 1.000000f), //  9500 K
    GfVec3f(0.609681f, 0.695588f, 1.000000f), // 10000 K
    GfVec3f(0.609681f, 0.695588f, 1.000000f), // 10000 K
    GfVec3f(0.609681f, 0.695588f, 1.000000f)  // 10000 K
};

// Catmull-Rom basis
static const float _basis[4][4] = {
    {-0.5f,  1.5f, -1.5f,  0.5f},
    { 1.f,  -2.5f,  2.0f, -0.5f},
    {-0.5f,  0.0f,  0.5f,  0.0f},
    { 0.f,   1.0f,  0.0f,  0.0f}
};

static inline float _Rec709RgbToLuma(const GfVec3f &rgb)
{
    return GfDot(rgb, GfVec3f(0.2126f, 0.7152f, 0.0722f));
}

static GfVec3f _BlackbodyTemperatureAsRgb(float temp)
{
    // Catmull-Rom interpolation of _blackbodyRGB
    constexpr int numKnots = sizeof(_blackbodyRGB) / sizeof(_blackbodyRGB[0]);
    // Parametric distance along spline
    const float u_spline = GfClamp((temp - 1000.0f) / 9000.0f, 0.0f, 1.0f);
    // Last 4 knots represent a trailing segment starting at u_spline==1.0,
    // to simplify boundary behavior
    constexpr int numSegs = (numKnots-4);
    const float x = u_spline * numSegs;
    const int seg = int(floor(x));
    const float u_seg = x-seg; // Parameter within segment
    // Knot values for this segment
    GfVec3f k0 = _blackbodyRGB[seg+0];
    GfVec3f k1 = _blackbodyRGB[seg+1];
    GfVec3f k2 = _blackbodyRGB[seg+2];
    GfVec3f k3 = _blackbodyRGB[seg+3];
    // Compute cubic coefficients.  Could fold constants (zero, one) here
    // if speed is a concern.
    GfVec3f a=_basis[0][0]*k0+_basis[0][1]*k1+_basis[0][2]*k2+_basis[0][3]*k3;
    GfVec3f b=_basis[1][0]*k0+_basis[1][1]*k1+_basis[1][2]*k2+_basis[1][3]*k3;
    GfVec3f c=_basis[2][0]*k0+_basis[2][1]*k1+_basis[2][2]*k2+_basis[2][3]*k3;
    GfVec3f d=_basis[3][0]*k0+_basis[3][1]*k1+_basis[3][2]*k2+_basis[3][3]*k3;
    // Eval cubic polynomial.
    GfVec3f rgb = ((a*u_seg+b)*u_seg+c)*u_seg+d;
    // Normalize to the same luminance as (1,1,1)
    rgb /= _Rec709RgbToLuma(rgb);
    // Clamp at zero, since the spline can produce small negative values,
    // e.g. in the blue component at 1300k.
    rgb[0] = GfMax(rgb[0], 0.f);
    rgb[1] = GfMax(rgb[1], 0.f);
    rgb[2] = GfMax(rgb[2], 0.f);
    return rgb;
}

HdNeVKLight::HdNeVKLight(const SdfPath& id, TfToken const& lightType)
    : HdLight(id), mLightType(lightType)
{
}

HdNeVKLight::~HdNeVKLight()
{
}

void HdNeVKLight::Sync(HdSceneDelegate* sceneDelegate,
                      HdRenderParam* renderParam,
                      HdDirtyBits* dirtyBits)
{
    TF_UNUSED(renderParam);

    bool pullLight = (*dirtyBits & DirtyBits::DirtyParams);
    
    *dirtyBits = DirtyBits::Clean;
    
    if (!pullLight)
    {
        return;
    }

    const SdfPath& id = GetId();
    const VtValue& resource = sceneDelegate->GetMaterialResource(id);

    if (mLightType == HdPrimTypeTokens->rectLight)
    {
        float width = 0.0f;
        float height = 0.0f;
        VtValue widthVal =
            sceneDelegate->GetLightParamValue(id, HdLightTokens->width);
        if (widthVal.IsHolding<float>())
        {
            width = widthVal.Get<float>();
        }
        VtValue heightVal =
            sceneDelegate->GetLightParamValue(id, HdLightTokens->height);
        if (heightVal.IsHolding<float>())
        {
            height = heightVal.Get<float>();
        }
        // Get the color of the light
        GfVec3f hdc = sceneDelegate->GetLightParamValue(id, HdLightTokens->color)
                          .Get<GfVec3f>();

        // Color temperature
        VtValue enableColorTemperatureVal = sceneDelegate->GetLightParamValue(id,
                                                                              HdLightTokens->enableColorTemperature);
        if (enableColorTemperatureVal.GetWithDefault<bool>(false))
        {
            VtValue colorTemperatureVal = sceneDelegate->GetLightParamValue(id,
                                                                            HdLightTokens->colorTemperature);
            if (colorTemperatureVal.IsHolding<float>())
            {
                float colorTemperature = colorTemperatureVal.Get<float>();
                hdc = GfCompMult(hdc,
                                 _BlackbodyTemperatureAsRgb(colorTemperature));
            }
        }

        // Intensity
        float intensity =
            sceneDelegate->GetLightParamValue(id, HdLightTokens->intensity)
                .Get<float>();

        // Exposure
        float exposure =
            sceneDelegate->GetLightParamValue(id, HdLightTokens->exposure)
                .Get<float>();
        intensity *= powf(2.0f, GfClamp(exposure, -50.0f, 50.0f));

        // Transform
        {
            GfMatrix4d transform = sceneDelegate->GetTransform(id);
            glm::float4x4 xform;
            for (int i = 0; i < 4; ++i)
            {
                for (int j = 0; j < 4; ++j)
                {
                    xform[i][j] = (float)transform[i][j];
                }
            }
            mLightDesc.xform = xform;
            mLightDesc.useXform = true;
        }
        mLightDesc.color = glm::float3(hdc[0], hdc[1], hdc[2]);
        mLightDesc.intensity = intensity;
        mLightDesc.height = height;
        mLightDesc.width = width;
    }
    else
    {
        // TODO:
    }

}

HdDirtyBits HdNeVKLight::GetInitialDirtyBitsMask() const
{
    return (DirtyParams | DirtyTransform);
}

nevk::Scene::RectLightDesc HdNeVKLight::getLightDesc()
{
    return mLightDesc;
}

PXR_NAMESPACE_CLOSE_SCOPE
