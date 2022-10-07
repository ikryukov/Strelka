#pragma once
#include <pxr/pxr.h>
#include <pxr/imaging/hd/basisCurves.h>

#include <scene/scene.h>

#include <pxr/base/gf/vec2f.h>

PXR_NAMESPACE_OPEN_SCOPE

class HdStrelkaBasisCurves final: public HdBasisCurves
{
public:
    HF_MALLOC_TAG_NEW("new HdStrelkaBasicCurves");

    HdStrelkaBasisCurves(const SdfPath& id, oka::Scene* scene);

    ~HdStrelkaBasisCurves() override;

    void Sync(HdSceneDelegate* sceneDelegate,
              HdRenderParam* renderParam,
              HdDirtyBits* dirtyBits,
              const TfToken& reprToken) override;

    HdDirtyBits GetInitialDirtyBitsMask() const override;

protected:
    void _InitRepr(const TfToken& reprName, HdDirtyBits* dirtyBits) override;
    HdDirtyBits _PropagateDirtyBits(HdDirtyBits bits) const override;
private:
    bool _FindPrimvar(HdSceneDelegate* sceneDelegate, TfToken primvarName, HdInterpolation& interpolation) const;
    void _PullPrimvars(HdSceneDelegate* sceneDelegate,
                                             VtVec3fArray& points,
                                             VtVec3fArray& normals,
                                             VtFloatArray& widths,
                                             bool& indexedNormals,
                                             bool& indexedUVs,
                                             GfVec3f& color,
                                             bool& hasColor) const;
    void _UpdateGeometry(HdSceneDelegate* sceneDelegate);
    oka::Scene* mScene;
    std::string mName;
    GfVec3f mColor;
    std::vector<GfVec3f> mPoints;
    std::vector<GfVec3f> mNormals;
    std::vector<float> mWidths;
    std::vector<GfVec2f> m_uvs;
    std::vector<GfVec3i> m_faces;
};

PXR_NAMESPACE_CLOSE_SCOPE
