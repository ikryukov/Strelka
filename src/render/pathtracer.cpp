#include "pathtracer.h"
namespace nevk
{

PathTracer::PathTracer(const SharedContext& ctx)
    : PathTracerBase(ctx)
{
}

PathTracer::~PathTracer()
{
}

void PathTracer::initialize()
{
    PathTracerBase::initialize("shaders/pathtracer.hlsl");
}

void PathTracer::setResources(const PathTracerDesc& desc)
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

    mMatSamplers = desc.matSampler;
    mMatTextures = desc.matTextures;
    mShaderParams.setSamplers("samplers", mMatSamplers);
    mShaderParams.setTextures("textures", mMatTextures);
}

} // namespace nevk
