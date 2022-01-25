#include "RenderDelegate.h"
#include "RenderPass.h"
#include "Camera.h"
#include "Mesh.h"
#include "Instancer.h"
#include "RenderBuffer.h"
#include "Material.h"
#include "Light.h"
#include "Tokens.h"

#include <pxr/imaging/hd/resourceRegistry.h>
#include <pxr/base/gf/vec4f.h>

#include <memory>

PXR_NAMESPACE_OPEN_SCOPE

HdNeVKRenderDelegate::HdNeVKRenderDelegate(const HdRenderSettingsMap& settingsMap, const MaterialNetworkTranslator& translator)
    : m_translator(translator)
{
    m_resourceRegistry = std::make_shared<HdResourceRegistry>();

    m_settingDescriptors.push_back(HdRenderSettingDescriptor{ "Samples per pixel", HdNeVKSettingsTokens->spp, VtValue{ 8 } });
    m_settingDescriptors.push_back(HdRenderSettingDescriptor{ "Max bounces", HdNeVKSettingsTokens->max_bounces, VtValue{ 4 } });

    _PopulateDefaultSettings(m_settingDescriptors);

    for (const auto& setting : settingsMap)
    {
        const TfToken& key = setting.first;
        const VtValue& value = setting.second;

        _settingsMap[key] = value;
    }
}

HdNeVKRenderDelegate::~HdNeVKRenderDelegate()
{
}

HdRenderSettingDescriptorList HdNeVKRenderDelegate::GetRenderSettingDescriptors() const
{
    return m_settingDescriptors;
}

HdRenderPassSharedPtr HdNeVKRenderDelegate::CreateRenderPass(HdRenderIndex* index,
                                                             const HdRprimCollection& collection)
{
    return HdRenderPassSharedPtr(new HdNeVKRenderPass(index, collection, _settingsMap));
}

HdResourceRegistrySharedPtr HdNeVKRenderDelegate::GetResourceRegistry() const
{
    return m_resourceRegistry;
}

void HdNeVKRenderDelegate::CommitResources(HdChangeTracker* tracker)
{
    TF_UNUSED(tracker);

    // We delay BVH building and GPU uploads to the next render call.
}

HdInstancer* HdNeVKRenderDelegate::CreateInstancer(HdSceneDelegate* delegate,
                                                   const SdfPath& id)
{
    return new HdNeVKInstancer(delegate, id);
}

void HdNeVKRenderDelegate::DestroyInstancer(HdInstancer* instancer)
{
    delete instancer;
}

HdAovDescriptor HdNeVKRenderDelegate::GetDefaultAovDescriptor(const TfToken& name) const
{
    TF_UNUSED(name);

    HdAovDescriptor aovDescriptor;
    aovDescriptor.format = HdFormatFloat32Vec4;
    aovDescriptor.multiSampled = false;
    aovDescriptor.clearValue = GfVec4f(0.0f, 0.0f, 0.0f, 0.0f);
    return aovDescriptor;
}

const TfTokenVector SUPPORTED_RPRIM_TYPES = {
    HdPrimTypeTokens->mesh
};

const TfTokenVector& HdNeVKRenderDelegate::GetSupportedRprimTypes() const
{
    return SUPPORTED_RPRIM_TYPES;
}

HdRprim* HdNeVKRenderDelegate::CreateRprim(const TfToken& typeId,
                                           const SdfPath& rprimId)
{
    if (typeId == HdPrimTypeTokens->mesh)
    {
        return new HdNeVKMesh(rprimId);
    }

    return nullptr;
}

void HdNeVKRenderDelegate::DestroyRprim(HdRprim* rprim)
{
    delete rprim;
}

const TfTokenVector SUPPORTED_SPRIM_TYPES = {
    HdPrimTypeTokens->camera,
    HdPrimTypeTokens->material,
    HdPrimTypeTokens->light,
    HdPrimTypeTokens->rectLight,
    HdPrimTypeTokens->diskLight
};

const TfTokenVector& HdNeVKRenderDelegate::GetSupportedSprimTypes() const
{
    return SUPPORTED_SPRIM_TYPES;
}

HdSprim* HdNeVKRenderDelegate::CreateSprim(const TfToken& typeId,
                                           const SdfPath& sprimId)
{
    if (typeId == HdPrimTypeTokens->camera)
    {
        return new HdNeVKCamera(sprimId);
    }
    else if (typeId == HdPrimTypeTokens->material)
    {
        return new HdNeVKMaterial(sprimId, m_translator);
    }
    else if (typeId == HdPrimTypeTokens->rectLight)
    {
        // unified light, but currently only rect light supported
        return new HdNeVKLight(sprimId, typeId);
    }
    else if (typeId == HdPrimTypeTokens->diskLight)
    {
        return new HdNeVKLight(sprimId, typeId);
    }

    return nullptr;
}

HdSprim* HdNeVKRenderDelegate::CreateFallbackSprim(const TfToken& typeId)
{
    const SdfPath& sprimId = SdfPath::EmptyPath();

    return CreateSprim(typeId, sprimId);
}

void HdNeVKRenderDelegate::DestroySprim(HdSprim* sprim)
{
    delete sprim;
}

const TfTokenVector SUPPORTED_BPRIM_TYPES = {
    HdPrimTypeTokens->renderBuffer
};

const TfTokenVector& HdNeVKRenderDelegate::GetSupportedBprimTypes() const
{
    return SUPPORTED_BPRIM_TYPES;
}

HdBprim* HdNeVKRenderDelegate::CreateBprim(const TfToken& typeId,
                                           const SdfPath& bprimId)
{
    if (typeId == HdPrimTypeTokens->renderBuffer)
    {
        return new HdNeVKRenderBuffer(bprimId);
    }

    return nullptr;
}

HdBprim* HdNeVKRenderDelegate::CreateFallbackBprim(const TfToken& typeId)
{
    const SdfPath& bprimId = SdfPath::EmptyPath();

    return CreateBprim(typeId, bprimId);
}

void HdNeVKRenderDelegate::DestroyBprim(HdBprim* bprim)
{
    delete bprim;
}

TfToken HdNeVKRenderDelegate::GetMaterialBindingPurpose() const
{
    return HdTokens->full;
}

TfTokenVector HdNeVKRenderDelegate::GetMaterialRenderContexts() const
{
    return TfTokenVector{ HdNeVKRenderContexts->mtlx };
}

TfTokenVector HdNeVKRenderDelegate::GetShaderSourceTypes() const
{
    return TfTokenVector{ HdNeVKSourceTypes->mtlx };
}

PXR_NAMESPACE_CLOSE_SCOPE
