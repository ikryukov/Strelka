#pragma once

#include <pxr/imaging/hd/material.h>
#include <pxr/imaging/hd/sceneDelegate.h>

#include "MaterialNetworkTranslator.h"

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
  //const Material* getNeVKMaterial() const;

private:
  const MaterialNetworkTranslator& m_translator;
  //Material* mMaterial = nullptr;
};

PXR_NAMESPACE_CLOSE_SCOPE
