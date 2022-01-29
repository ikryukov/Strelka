#pragma once

#include "materialmanager.h"
#include "MaterialNetworkTranslator.h"

#include <pxr/imaging/hd/material.h>
#include <pxr/imaging/hd/sceneDelegate.h>

PXR_NAMESPACE_OPEN_SCOPE

class HdNeVKMaterial final : public HdMaterial
{
public:
    HF_MALLOC_TAG_NEW("new HdNeVKMaterial");

    HdNeVKMaterial(const SdfPath& id,
                   const MaterialNetworkTranslator& translator);

    ~HdNeVKMaterial() override;

    HdDirtyBits GetInitialDirtyBitsMask() const override;

    void Sync(HdSceneDelegate* sceneDelegate,
              HdRenderParam* renderParam,
              HdDirtyBits* dirtyBits) override;

    const std::string& GetNeVKMaterial() const;
    bool isMdl()
    {
        return mIsMdl;
    }
    std::string getFileUri()
    {
        return mMdlFileUri;
    }
    std::string getSubIdentifier()
    {
        return mMdlSubIdentifier;
    }

private:
    const MaterialNetworkTranslator& m_translator;
    bool mIsMdl = false;
    std::string mMaterialXCode;
    // MDL related
    std::string mMdlFileUri;
    std::string mMdlSubIdentifier;
};

PXR_NAMESPACE_CLOSE_SCOPE
