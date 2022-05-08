#pragma once

#include "pxr/pxr.h"
#include <pxr/imaging/hd/mesh.h>

#include <scene/scene.h>

PXR_NAMESPACE_OPEN_SCOPE

class HdStrelkaMesh final : public HdMesh
{
public:
    HF_MALLOC_TAG_NEW("new HdStrelkaMesh");

    HdStrelkaMesh(const SdfPath& id, oka::Scene* scene);

    ~HdStrelkaMesh() override;

public:
    void Sync(HdSceneDelegate* delegate,
              HdRenderParam* renderParam,
              HdDirtyBits* dirtyBits,
              const TfToken& reprToken) override;

    HdDirtyBits GetInitialDirtyBitsMask() const override;

    const TfTokenVector& GetBuiltinPrimvarNames() const override;

    const std::vector<GfVec3f>& GetPoints() const;

    const std::vector<GfVec3f>& GetNormals() const;

    const std::vector<GfVec3i>& GetFaces() const;

    const GfMatrix4d& GetPrototypeTransform() const;

    const GfVec3f& GetColor() const;

    bool HasColor() const;

    const char* getName() const;

protected:
    HdDirtyBits _PropagateDirtyBits(HdDirtyBits bits) const override;

    void _InitRepr(const TfToken& reprName, HdDirtyBits* dirtyBits) override;

private:
    void _ConvertMesh();

    void _UpdateGeometry(HdSceneDelegate* sceneDelegate);

    bool _FindPrimvar(HdSceneDelegate* sceneDelegate, TfToken primvarName, HdInterpolation& interpolation) const;

    void _PullPrimvars(HdSceneDelegate* sceneDelegate,
                       VtVec3fArray& points,
                       VtVec3fArray& normals,
                       bool& indexedNormals,
                       GfVec3f& color,
                       bool& hasColor) const;

private:
    GfMatrix4d m_prototypeTransform;
    std::vector<GfVec3f> m_points;
    std::vector<GfVec3f> m_normals;
    std::vector<GfVec3i> m_faces;
    GfVec3f m_color;
    bool m_hasColor;
    oka::Scene* mScene;
    std::string mName;
};

PXR_NAMESPACE_CLOSE_SCOPE
