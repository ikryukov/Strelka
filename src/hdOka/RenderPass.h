#pragma once

#include <pxr/imaging/hd/renderDelegate.h>
#include <pxr/imaging/hd/renderPass.h>
#include <pxr/pxr.h>

#include <scene/camera.h>
#include <scene/scene.h>
#include <render/ptrender.h>

PXR_NAMESPACE_OPEN_SCOPE

class HdOkaCamera;
class HdOkaMesh;

class HdOkaRenderPass final : public HdRenderPass
{
public:
    HdOkaRenderPass(HdRenderIndex* index,
                     const HdRprimCollection& collection,
                     const HdRenderSettingsMap& settings,
                     oka::PtRender* renderer,
                     oka::Scene* scene);

    ~HdOkaRenderPass() override;

public:
    bool IsConverged() const override;

protected:
    void _Execute(const HdRenderPassStateSharedPtr& renderPassState,
                  const TfTokenVector& renderTags) override;

private:
    void _BakeMeshInstance(const HdOkaMesh* mesh,
                           GfMatrix4d transform,
                           uint32_t materialIndex);

    void _BakeMeshes(HdRenderIndex* renderIndex,
                     GfMatrix4d rootTransform);

private:
    const HdRenderSettingsMap& m_settings;
    bool m_isConverged;
    uint32_t m_lastSceneStateVersion;
    uint32_t m_lastRenderSettingsVersion;
    GfMatrix4d m_rootMatrix;

    oka::Scene* mScene;
    // ptr to global render
    oka::PtRender* mRenderer;
};

PXR_NAMESPACE_CLOSE_SCOPE
