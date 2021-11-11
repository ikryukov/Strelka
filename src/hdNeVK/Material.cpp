#include "Material.h"

PXR_NAMESPACE_OPEN_SCOPE

HdNeVKMaterial::HdNeVKMaterial(const SdfPath& id,
                                     const MaterialNetworkTranslator& translator)
    : HdMaterial(id), m_translator(translator)
{
}

HdNeVKMaterial::~HdNeVKMaterial()
{
    //if (mMaterial)
    {
        //DestroyMaterial(m_Material);
    }
}

HdDirtyBits HdNeVKMaterial::GetInitialDirtyBitsMask() const
{
    return DirtyBits::DirtyParams;
}

void HdNeVKMaterial::Sync(HdSceneDelegate* sceneDelegate,
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
    const VtValue& resource = sceneDelegate->GetMaterialResource(id);

    if (!resource.IsHolding<HdMaterialNetworkMap>())
    {
        return;
    }

    const HdMaterialNetworkMap& networkMap = resource.UncheckedGet<HdMaterialNetworkMap>();
    HdMaterialNetwork2 network;
    bool isVolume = false;

    HdMaterialNetwork2ConvertFromHdMaterialNetworkMap(networkMap, &network, &isVolume);
    if (isVolume)
    {
        TF_WARN("Volume %s unsupported", id.GetText());
        return;
    }

    //mMaterial = m_translator.ParseNetwork(id, network);
}

//const Material* HdNeVKMaterial::GetGiMaterial() const
//{
//    return m_Material;
//}

PXR_NAMESPACE_CLOSE_SCOPE
