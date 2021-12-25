#pragma once

#include "materialmanager.h"
#include "MaterialNetworkTranslator.h"

#include <pxr/imaging/hd/material.h>
#include <pxr/imaging/hd/sceneDelegate.h>

PXR_NAMESPACE_OPEN_SCOPE

class HdNeVKMaterial final : public HdMaterial
{
public:
    HdNeVKMaterial(const SdfPath& id,
                   const MaterialNetworkTranslator& translator);

    ~HdNeVKMaterial() override;

public:
    HdDirtyBits GetInitialDirtyBitsMask() const override;

    void Sync(HdSceneDelegate* sceneDelegate,
              HdRenderParam* renderParam,
              HdDirtyBits* dirtyBits) override;

public:
    const std::string& GetNeVKMaterial() const;

private:
    const MaterialNetworkTranslator& m_translator;
    std::string mMaterialXCode;
};

PXR_NAMESPACE_CLOSE_SCOPE
