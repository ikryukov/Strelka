#include "Light.h"

#include <pxr/imaging/hd/instancer.h>
#include <pxr/imaging/hd/meshUtil.h>
#include <pxr/imaging/hd/smoothNormals.h>
#include <pxr/imaging/hd/vertexAdjacency.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/matrix_decompose.hpp>

PXR_NAMESPACE_OPEN_SCOPE

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
    // return HdChangeTracker::DirtyPoints |
    //        HdChangeTracker::DirtyNormals |
    //        HdChangeTracker::DirtyTopology |
    //        HdChangeTracker::DirtyInstancer |
    //        HdChangeTracker::DirtyInstanceIndex |
    //        HdChangeTracker::DirtyTransform |
    //        HdChangeTracker::DirtyMaterialId;
}

nevk::Scene::RectLightDesc HdNeVKLight::getLightDesc()
{
    return mLightDesc;
}

PXR_NAMESPACE_CLOSE_SCOPE
