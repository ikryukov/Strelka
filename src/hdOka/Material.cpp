#include "Material.h"

#include <pxr/usd/sdr/registry.h>
#include <pxr/usdImaging/usdImaging/tokens.h>

PXR_NAMESPACE_OPEN_SCOPE

HdOkaMaterial::HdOkaMaterial(const SdfPath& id,
                                     const MaterialNetworkTranslator& translator)
    : HdMaterial(id), m_translator(translator)
{
}

HdOkaMaterial::~HdOkaMaterial()
{
    //if (mMaterial)
    {
        //DestroyMaterial(m_Material);
    }
}

HdDirtyBits HdOkaMaterial::GetInitialDirtyBitsMask() const
{
    //return DirtyBits::DirtyParams;
    return DirtyBits::AllDirty;
}

void HdOkaMaterial::Sync(HdSceneDelegate* sceneDelegate,
                             HdRenderParam* renderParam,
                             HdDirtyBits* dirtyBits)
{
    TF_UNUSED(renderParam);

    bool pullMaterial = (*dirtyBits & DirtyBits::DirtyParams);

    *dirtyBits = DirtyBits::Clean;

    if (!pullMaterial)
    {
        return;
    }

    const SdfPath& id = GetId();
    std::string name = id.GetString();
    const VtValue& resource = sceneDelegate->GetMaterialResource(id);

    if (!resource.IsHolding<HdMaterialNetworkMap>())
    {
        return;
    }

    HdMaterialNetworkMap networkMap = resource.GetWithDefault<HdMaterialNetworkMap>();
    HdMaterialNetwork& surfaceNetwork = networkMap.map[HdMaterialTerminalTokens->surface];

    bool isUsdPreviewSurface = false;
    HdMaterialNode* previewSurfaceNode = nullptr;
    for (auto& node : surfaceNetwork.nodes)
    {
        if (node.identifier == UsdImagingTokens->UsdPreviewSurface)
        {
            previewSurfaceNode = &node;
            isUsdPreviewSurface = true;
        }
    }
    
    HdMaterialNetwork2 network;
    bool isVolume = false;
    HdMaterialNetwork2ConvertFromHdMaterialNetworkMap(networkMap, &network, &isVolume);
    if (isVolume)
    {
        TF_WARN("Volume %s unsupported", id.GetText());
        return;
    }
    
    if (isUsdPreviewSurface)
    {
        mMaterialXCode = m_translator.ParseNetwork(id, network);
    }
    else
    {
        // MDL
        bool res = m_translator.ParseMdlNetwork(id, network, mMdlFileUri, mMdlSubIdentifier);
        if (!res)
        {
            TF_RUNTIME_ERROR("Failed to translate material!");
        }
        mIsMdl = true;
    }
    
}

const std::string& HdOkaMaterial::GetOkaMaterial() const
{
    return mMaterialXCode;
}

PXR_NAMESPACE_CLOSE_SCOPE
