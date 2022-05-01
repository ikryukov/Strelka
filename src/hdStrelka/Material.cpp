#include "Material.h"

#include <pxr/usd/sdr/registry.h>
#include <pxr/usdImaging/usdImaging/tokens.h>

PXR_NAMESPACE_OPEN_SCOPE
// clang-format off
TF_DEFINE_PRIVATE_TOKENS(_tokens,
    (diffuse_color_constant)
);
// clang-format on

HdStrelkaMaterial::HdStrelkaMaterial(const SdfPath& id, const MaterialNetworkTranslator& translator)
    : HdMaterial(id), m_translator(translator)
{
}

HdStrelkaMaterial::~HdStrelkaMaterial()
{
    // if (mMaterial)
    {
        // DestroyMaterial(m_Material);
    }
}

HdDirtyBits HdStrelkaMaterial::GetInitialDirtyBitsMask() const
{
    // return DirtyBits::DirtyParams;
    return DirtyBits::AllDirty;
}

void HdStrelkaMaterial::Sync(HdSceneDelegate* sceneDelegate, HdRenderParam* renderParam, HdDirtyBits* dirtyBits)
{
    TF_UNUSED(renderParam);

    bool pullMaterial = (*dirtyBits & DirtyBits::DirtyParams);

    *dirtyBits = DirtyBits::Clean;

    if (!pullMaterial)
    {
        return;
    }

    const SdfPath& id = GetId();
    const std::string& name = id.GetString();
    const VtValue& resource = sceneDelegate->GetMaterialResource(id);

    if (!resource.IsHolding<HdMaterialNetworkMap>())
    {
        return;
    }

    HdMaterialNetworkMap networkMap = resource.GetWithDefault<HdMaterialNetworkMap>();
    HdMaterialNetwork& surfaceNetwork = networkMap.map[HdMaterialTerminalTokens->surface];

    bool isUsdPreviewSurface = false;
    HdMaterialNode* previewSurfaceNode = nullptr;
    // store material parameters
    std::unordered_map<std::string, VtValue> surfaceParams;
    for (auto& node : surfaceNetwork.nodes)
    {
        if (node.identifier == UsdImagingTokens->UsdPreviewSurface)
        {
            previewSurfaceNode = &node;
            isUsdPreviewSurface = true;
        }
        GfVec3f diffuseColor;
        for (std::pair<TfToken, VtValue> params : node.parameters)
        {
            surfaceParams[params.first] = params.second;
            if (params.first == _tokens->diffuse_color_constant)
            {
                if (params.second.IsHolding<GfVec3f>())
                {
                    diffuseColor = params.second.Get<GfVec3f>();
                }
            }
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

const std::string& HdStrelkaMaterial::GetStrelkaMaterial() const
{
    return mMaterialXCode;
}

PXR_NAMESPACE_CLOSE_SCOPE
