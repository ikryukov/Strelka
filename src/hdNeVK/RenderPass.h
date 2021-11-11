#pragma once

#include <pxr/imaging/hd/renderDelegate.h>
#include <pxr/imaging/hd/renderPass.h>
#include <pxr/pxr.h>

//#include <scene/camera.h>

PXR_NAMESPACE_OPEN_SCOPE

class HdNeVKCamera;
class HdNeVKMesh;

class HdNeVKRenderPass final : public HdRenderPass
{
public:
    HdNeVKRenderPass(HdRenderIndex* index,
                     const HdRprimCollection& collection,
                     const HdRenderSettingsMap& settings);

    ~HdNeVKRenderPass() override;

public:
    bool IsConverged() const override;

protected:
    void _Execute(const HdRenderPassStateSharedPtr& renderPassState,
                  const TfTokenVector& renderTags) override;

private:
    //void _BakeMeshInstance(const HdNeVKMesh* mesh,
    //                       GfMatrix4d transform,
    //                       uint32_t materialIndex,
    //                       std::vector<gi_face>& faces,
    //                       std::vector<gi_vertex>& vertices) const;

    //void _BakeMeshes(HdRenderIndex* renderIndex,
    //                 GfMatrix4d rootTransform,
    //                 std::vector<gi_vertex>& vertices,
    //                 std::vector<gi_face>& faces,
    //                 std::vector<const gi_material*>& materials) const;

    //void _ConstructNeVKCamera(const HdNeVKCamera& camera, nevk::Camera& giCamera) const;

private:
    const HdRenderSettingsMap& m_settings;
    bool m_isConverged;
    uint32_t m_lastSceneStateVersion;
    uint32_t m_lastRenderSettingsVersion;
    GfMatrix4d m_rootMatrix;
};

PXR_NAMESPACE_CLOSE_SCOPE
