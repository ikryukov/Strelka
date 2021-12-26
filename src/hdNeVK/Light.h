#pragma once

#include "pxr/pxr.h"
#include <pxr/imaging/hd/light.h>
#include <pxr/imaging/hd/sceneDelegate.h>

#include <scene/scene.h>

PXR_NAMESPACE_OPEN_SCOPE

class HdNeVKLight final : public HdLight
{
public:
    HF_MALLOC_TAG_NEW("new HdNeVKLight");

    HdNeVKLight(const SdfPath& id, TfToken const& lightType);

    ~HdNeVKLight() override;

public:
    void Sync(HdSceneDelegate* delegate,
              HdRenderParam* renderParam,
              HdDirtyBits* dirtyBits) override;

    HdDirtyBits GetInitialDirtyBitsMask() const override;

    nevk::Scene::RectLightDesc getLightDesc();

private:
    TfToken mLightType;
    nevk::Scene::RectLightDesc mLightDesc;
};

PXR_NAMESPACE_CLOSE_SCOPE
