#pragma once

#include <pxr/imaging/hd/camera.h>

PXR_NAMESPACE_OPEN_SCOPE

class HdNeVKCamera final : public HdCamera
{
public:
    HdNeVKCamera(const SdfPath& id);

    ~HdNeVKCamera() override;

public:
    float GetVFov() const;

public:
    void Sync(HdSceneDelegate* sceneDelegate,
              HdRenderParam* renderParam,
              HdDirtyBits* dirtyBits) override;

    HdDirtyBits GetInitialDirtyBitsMask() const override;

private:
    float m_vfov;
};

PXR_NAMESPACE_CLOSE_SCOPE
