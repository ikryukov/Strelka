#pragma once

#include <pxr/imaging/hd/camera.h>

#include <scene/scene.h>

PXR_NAMESPACE_OPEN_SCOPE

class HdNeVKCamera final : public HdCamera
{
public:
    HdNeVKCamera(const SdfPath& id, nevk::Scene& scene);

    ~HdNeVKCamera() override;

public:
    float GetVFov() const;

    uint32_t GetCameraIndex() const;

public:
    void Sync(HdSceneDelegate* sceneDelegate,
              HdRenderParam* renderParam,
              HdDirtyBits* dirtyBits) override;

    HdDirtyBits GetInitialDirtyBitsMask() const override;

private:
    nevk::Camera _ConstructNeVKCamera();

    float m_vfov;
    nevk::Scene& mScene;
    uint32_t mCameraIndex = -1;
};

PXR_NAMESPACE_CLOSE_SCOPE
