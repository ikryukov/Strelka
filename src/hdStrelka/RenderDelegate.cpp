#include "RenderDelegate.h"

#include "Camera.h"
#include "Instancer.h"
#include "Light.h"
#include "Material.h"
#include "Mesh.h"
#include "RenderBuffer.h"
#include "RenderPass.h"
#include "Tokens.h"

#include <pxr/base/gf/vec4f.h>
#include <pxr/imaging/hd/resourceRegistry.h>

#include <memory>

PXR_NAMESPACE_OPEN_SCOPE

TF_DEFINE_PRIVATE_TOKENS(_Tokens, (HdStrelkaDriver));

HdStrelkaRenderDelegate::HdStrelkaRenderDelegate(const HdRenderSettingsMap& settingsMap,
                                                 const MaterialNetworkTranslator& translator)
    : m_translator(translator)
{
    m_resourceRegistry = std::make_shared<HdResourceRegistry>();

    m_settingDescriptors.push_back(
        HdRenderSettingDescriptor{ "Samples per pixel", HdStrelkaSettingsTokens->spp, VtValue{ 8 } });
    m_settingDescriptors.push_back(
        HdRenderSettingDescriptor{ "Max bounces", HdStrelkaSettingsTokens->max_bounces, VtValue{ 4 } });

    _PopulateDefaultSettings(m_settingDescriptors);

    for (const auto& setting : settingsMap)
    {
        const TfToken& key = setting.first;
        const VtValue& value = setting.second;

        _settingsMap[key] = value;
    }

    mRenderer.setScene(&mScene);
}

HdStrelkaRenderDelegate::~HdStrelkaRenderDelegate()
{
}

void HdStrelkaRenderDelegate::SetDrivers(HdDriverVector const& drivers)
{
    for (HdDriver* hdDriver : drivers)
    {
        if (hdDriver->name == _Tokens->HdStrelkaDriver && hdDriver->driver.IsHolding<oka::SharedContext*>())
        {
            mSharedCtx = hdDriver->driver.UncheckedGet<oka::SharedContext*>();
            mRenderer.setSharedContext(mSharedCtx);
            mRenderer.init();
            break;
        }
    }
}

HdRenderSettingDescriptorList HdStrelkaRenderDelegate::GetRenderSettingDescriptors() const
{
    return m_settingDescriptors;
}

HdRenderPassSharedPtr HdStrelkaRenderDelegate::CreateRenderPass(HdRenderIndex* index, const HdRprimCollection& collection)
{
    return HdRenderPassSharedPtr(new HdStrelkaRenderPass(index, collection, _settingsMap, &mRenderer, &mScene));
}

HdResourceRegistrySharedPtr HdStrelkaRenderDelegate::GetResourceRegistry() const
{
    return m_resourceRegistry;
}

void HdStrelkaRenderDelegate::CommitResources(HdChangeTracker* tracker)
{
    TF_UNUSED(tracker);

    // We delay BVH building and GPU uploads to the next render call.
}

HdInstancer* HdStrelkaRenderDelegate::CreateInstancer(HdSceneDelegate* delegate, const SdfPath& id)
{
    return new HdStrelkaInstancer(delegate, id);
}

void HdStrelkaRenderDelegate::DestroyInstancer(HdInstancer* instancer)
{
    delete instancer;
}

HdAovDescriptor HdStrelkaRenderDelegate::GetDefaultAovDescriptor(const TfToken& name) const
{
    TF_UNUSED(name);

    HdAovDescriptor aovDescriptor;
    aovDescriptor.format = HdFormatFloat32Vec4;
    aovDescriptor.multiSampled = false;
    aovDescriptor.clearValue = GfVec4f(0.0f, 0.0f, 0.0f, 0.0f);
    return aovDescriptor;
}

const TfTokenVector SUPPORTED_RPRIM_TYPES = { HdPrimTypeTokens->mesh };

const TfTokenVector& HdStrelkaRenderDelegate::GetSupportedRprimTypes() const
{
    return SUPPORTED_RPRIM_TYPES;
}

HdRprim* HdStrelkaRenderDelegate::CreateRprim(const TfToken& typeId, const SdfPath& rprimId)
{
    if (typeId == HdPrimTypeTokens->mesh)
    {
        return new HdStrelkaMesh(rprimId, &mScene);
    }
    TF_CODING_ERROR("Unknown Rprim Type %s", typeId.GetText());
    return nullptr;
}

void HdStrelkaRenderDelegate::DestroyRprim(HdRprim* rprim)
{
    delete rprim;
}

const TfTokenVector SUPPORTED_SPRIM_TYPES = {
    HdPrimTypeTokens->camera,    HdPrimTypeTokens->material,  HdPrimTypeTokens->light,
    HdPrimTypeTokens->rectLight, HdPrimTypeTokens->diskLight, HdPrimTypeTokens->sphereLight,
};

const TfTokenVector& HdStrelkaRenderDelegate::GetSupportedSprimTypes() const
{
    return SUPPORTED_SPRIM_TYPES;
}

HdSprim* HdStrelkaRenderDelegate::CreateSprim(const TfToken& typeId, const SdfPath& sprimId)
{
    TF_STATUS("CreateSprim Type: %s", typeId.GetText());
    if (typeId == HdPrimTypeTokens->camera)
    {
        return new HdStrelkaCamera(sprimId, mScene);
    }
    else if (typeId == HdPrimTypeTokens->material)
    {
        return new HdStrelkaMaterial(sprimId, m_translator);
    }
    else if (typeId == HdPrimTypeTokens->rectLight)
    {
        return new HdStrelkaLight(sprimId, typeId);
    }
    else if (typeId == HdPrimTypeTokens->diskLight)
    {
        return new HdStrelkaLight(sprimId, typeId);
    }
    else if (typeId == HdPrimTypeTokens->sphereLight)
    {
        return new HdStrelkaLight(sprimId, typeId);
    }

    TF_CODING_ERROR("Unknown Sprim Type %s", typeId.GetText());

    return nullptr;
}

HdSprim* HdStrelkaRenderDelegate::CreateFallbackSprim(const TfToken& typeId)
{
    const SdfPath& sprimId = SdfPath::EmptyPath();

    return CreateSprim(typeId, sprimId);
}

void HdStrelkaRenderDelegate::DestroySprim(HdSprim* sprim)
{
    delete sprim;
}

const TfTokenVector SUPPORTED_BPRIM_TYPES = { HdPrimTypeTokens->renderBuffer };

const TfTokenVector& HdStrelkaRenderDelegate::GetSupportedBprimTypes() const
{
    return SUPPORTED_BPRIM_TYPES;
}

HdBprim* HdStrelkaRenderDelegate::CreateBprim(const TfToken& typeId, const SdfPath& bprimId)
{
    if (typeId == HdPrimTypeTokens->renderBuffer)
    {
        return new HdStrelkaRenderBuffer(bprimId, mSharedCtx);
    }

    return nullptr;
}

HdBprim* HdStrelkaRenderDelegate::CreateFallbackBprim(const TfToken& typeId)
{
    const SdfPath& bprimId = SdfPath::EmptyPath();

    return CreateBprim(typeId, bprimId);
}

void HdStrelkaRenderDelegate::DestroyBprim(HdBprim* bprim)
{
    delete bprim;
}

TfToken HdStrelkaRenderDelegate::GetMaterialBindingPurpose() const
{
    return HdTokens->full;
}

TfTokenVector HdStrelkaRenderDelegate::GetMaterialRenderContexts() const
{
    return TfTokenVector{ HdStrelkaRenderContexts->mtlx, HdStrelkaRenderContexts->mdl };
}

TfTokenVector HdStrelkaRenderDelegate::GetShaderSourceTypes() const
{
    return TfTokenVector{ HdStrelkaSourceTypes->mtlx, HdStrelkaSourceTypes->mdl };
}

oka::SharedContext& HdStrelkaRenderDelegate::getSharedContext()
{
    return mRenderer.getSharedContext();
}

PXR_NAMESPACE_CLOSE_SCOPE
