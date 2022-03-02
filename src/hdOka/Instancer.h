#pragma once

#include <pxr/imaging/hd/instancer.h>

PXR_NAMESPACE_OPEN_SCOPE

class HdOkaInstancer final : public HdInstancer
{
public:
    HdOkaInstancer(HdSceneDelegate* delegate,
                       const SdfPath& id);

    ~HdOkaInstancer() override;

public:
    VtMatrix4dArray ComputeInstanceTransforms(const SdfPath& prototypeId);

    void Sync(HdSceneDelegate* sceneDelegate,
              HdRenderParam* renderParam,
              HdDirtyBits* dirtyBits) override;

private:
    TfHashMap<TfToken, VtValue, TfToken::HashFunctor> m_primvarMap;
};

PXR_NAMESPACE_CLOSE_SCOPE
