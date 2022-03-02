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

TF_DEFINE_PRIVATE_TOKENS(
    _Tokens,
    (HdOkaDriver)
 );

HdOkaRenderDelegate::HdOkaRenderDelegate(const HdRenderSettingsMap& settingsMap, const MaterialNetworkTranslator& translator)
    : m_translator(translator)
{
    m_resourceRegistry = std::make_shared<HdResourceRegistry>();

    m_settingDescriptors.push_back(HdRenderSettingDescriptor{ "Samples per pixel", HdOkaSettingsTokens->spp, VtValue{ 8 } });
    m_settingDescriptors.push_back(HdRenderSettingDescriptor{ "Max bounces", HdOkaSettingsTokens->max_bounces, VtValue{ 4 } });

    _PopulateDefaultSettings(m_settingDescriptors);

    for (const auto& setting : settingsMap)
    {
        const TfToken& key = setting.first;
        const VtValue& value = setting.second;

        _settingsMap[key] = value;
    }

    mRenderer.setScene(&mScene);
}

HdOkaRenderDelegate::~HdOkaRenderDelegate()
{
}

void HdOkaRenderDelegate::SetDrivers(HdDriverVector const& drivers)
{
    for (HdDriver* hdDriver : drivers)
    {
        if (hdDriver->name == _Tokens->HdOkaDriver &&
            hdDriver->driver.IsHolding<oka::SharedContext*>())
        {
            mSharedCtx = hdDriver->driver.UncheckedGet<oka::SharedContext*>();
            mRenderer.setSharedContext(mSharedCtx);
            mRenderer.init();
            break;
        }
    }
}

HdRenderSettingDescriptorList HdOkaRenderDelegate::GetRenderSettingDescriptors() const
{
    return m_settingDescriptors;
}

HdRenderPassSharedPtr HdOkaRenderDelegate::CreateRenderPass(HdRenderIndex* index,
                                                             const HdRprimCollection& collection)
{
    return HdRenderPassSharedPtr(new HdOkaRenderPass(index, collection, _settingsMap, &mRenderer, &mScene));
}

HdResourceRegistrySharedPtr HdOkaRenderDelegate::GetResourceRegistry() const
{
    return m_resourceRegistry;
}

void HdOkaRenderDelegate::CommitResources(HdChangeTracker* tracker)
{
    TF_UNUSED(tracker);

    // We delay BVH building and GPU uploads to the next render call.
}

HdInstancer* HdOkaRenderDelegate::CreateInstancer(HdSceneDelegate* delegate,
                                                   const SdfPath& id)
{
    return new HdOkaInstancer(delegate, id);
}

void HdOkaRenderDelegate::DestroyInstancer(HdInstancer* instancer)
{
    delete instancer;
}

HdAovDescriptor HdOkaRenderDelegate::GetDefaultAovDescriptor(const TfToken& name) const
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

const TfTokenVector& HdOkaRenderDelegate::GetSupportedRprimTypes() const
{
    return SUPPORTED_RPRIM_TYPES;
}

HdRprim* HdOkaRenderDelegate::CreateRprim(const TfToken& typeId,
                                           const SdfPath& rprimId)
{
    if (typeId == HdPrimTypeTokens->mesh)
    {
        return new HdOkaMesh(rprimId, &mScene);
    }
    TF_CODING_ERROR("Unknown Rprim Type %s", typeId.GetText());
    return nullptr;
}

void HdOkaRenderDelegate::DestroyRprim(HdRprim* rprim)
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

const TfTokenVector& HdOkaRenderDelegate::GetSupportedSprimTypes() const
{
    return SUPPORTED_SPRIM_TYPES;
}

HdSprim* HdOkaRenderDelegate::CreateSprim(const TfToken& typeId,
                                           const SdfPath& sprimId)
{
    TF_STATUS("CreateSprim Type: %s", typeId.GetText());
    if (typeId == HdPrimTypeTokens->camera)
    {
        return new HdOkaCamera(sprimId, mScene);
    }
    else if (typeId == HdPrimTypeTokens->material)
    {
        return new HdOkaMaterial(sprimId, m_translator);
    }
    else if (typeId == HdPrimTypeTokens->rectLight)
    {
        // unified light, but currently only rect light supported
        return new HdOkaLight(sprimId, typeId);
    }
    else if (typeId == HdPrimTypeTokens->diskLight)
    {
        return new HdOkaLight(sprimId, typeId);
    }
    
    TF_CODING_ERROR("Unknown Sprim Type %s", typeId.GetText());

    return nullptr;
}

HdSprim* HdOkaRenderDelegate::CreateFallbackSprim(const TfToken& typeId)
{
    const SdfPath& sprimId = SdfPath::EmptyPath();

    return CreateSprim(typeId, sprimId);
}

void HdOkaRenderDelegate::DestroySprim(HdSprim* sprim)
{
    delete sprim;
}

const TfTokenVector SUPPORTED_BPRIM_TYPES = {
    HdPrimTypeTokens->renderBuffer
};

const TfTokenVector& HdOkaRenderDelegate::GetSupportedBprimTypes() const
{
    return SUPPORTED_BPRIM_TYPES;
}

HdBprim* HdOkaRenderDelegate::CreateBprim(const TfToken& typeId,
                                           const SdfPath& bprimId)
{
    if (typeId == HdPrimTypeTokens->renderBuffer)
    {
        return new HdOkaRenderBuffer(bprimId, mSharedCtx);
    }

    return nullptr;
}

HdBprim* HdOkaRenderDelegate::CreateFallbackBprim(const TfToken& typeId)
{
    const SdfPath& bprimId = SdfPath::EmptyPath();

    return CreateBprim(typeId, bprimId);
}

void HdOkaRenderDelegate::DestroyBprim(HdBprim* bprim)
{
    delete bprim;
}

TfToken HdOkaRenderDelegate::GetMaterialBindingPurpose() const
{
    return HdTokens->full;
}

TfTokenVector HdOkaRenderDelegate::GetMaterialRenderContexts() const
{
    return TfTokenVector{ HdOkaRenderContexts->mtlx, HdOkaRenderContexts->mdl };
}

TfTokenVector HdOkaRenderDelegate::GetShaderSourceTypes() const
{
    return TfTokenVector{ HdOkaSourceTypes->mtlx, HdOkaSourceTypes->mdl };
}

oka::SharedContext& HdOkaRenderDelegate::getSharedContext()
{
    return mRenderer.getSharedContext();
}

PXR_NAMESPACE_CLOSE_SCOPE
