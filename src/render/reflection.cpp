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
    mShaderParams.setTexture("instId", mSharedCtx.mResManager->getView(desc.gbuffer->instId));
    mShaderParams.setBuffer("bvhNodes", mSharedCtx.mResManager->getVkBuffer(desc.bvhNodes));
    mShaderParams.setBuffer("bvhTriangles", mSharedCtx.mResManager->getVkBuffer(desc.bvhTriangles));
    mShaderParams.setBuffer("lights", mSharedCtx.mResManager->getVkBuffer(desc.lights));
}

} // namespace nevk
