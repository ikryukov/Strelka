#include "reflection.h"

namespace nevk
{

Reflection::Reflection(const SharedContext& ctx)
    : ReflectionBase(ctx)
{
}

Reflection::~Reflection()
{
}

void Reflection::initialize()
{
    ReflectionBase::initialize("shaders/reflection.hlsl");
}

void Reflection::setResources(ReflectionDesc& desc)
{
    mShaderParams.setTexture("output", mSharedCtx.mResManager->getView(desc.result));

    mShaderParams.setTexture("gbWPos", mSharedCtx.mResManager->getView(desc.gbuffer->wPos));
    mShaderParams.setTexture("gbNormal", mSharedCtx.mResManager->getView(desc.gbuffer->normal));
    mShaderParams.setTexture("gbInstId", mSharedCtx.mResManager->getView(desc.gbuffer->instId));
    mShaderParams.setTexture("gbUV", mSharedCtx.mResManager->getView(desc.gbuffer->uv));
    mShaderParams.setBuffer("bvhNodes", mSharedCtx.mResManager->getVkBuffer(desc.bvhNodes));
    mShaderParams.setBuffer("vb", mSharedCtx.mResManager->getVkBuffer(desc.vb));
    mShaderParams.setBuffer("ib", mSharedCtx.mResManager->getVkBuffer(desc.ib));
    mShaderParams.setBuffer("instanceConstants", mSharedCtx.mResManager->getVkBuffer(desc.instanceConst));
    mShaderParams.setBuffer("materials", mSharedCtx.mResManager->getVkBuffer(desc.materials));
    mShaderParams.setSamplers("samplers", desc.matSampler);
    mShaderParams.setTextures("textures", desc.matTextures);
}

} // namespace nevk
