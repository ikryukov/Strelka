#include "pathtracer.h"
namespace nevk
{

PathTracer::PathTracer(const SharedContext& ctx, std::string& shaderCode)
    : PathTracerBase(ctx), mShaderCode(shaderCode)
{
}

PathTracer::~PathTracer()
{
}

void PathTracer::initialize()
{
    //PathTracerBase::initialize("shaders/pathtracer.hlsl");
    //PathTracerBase::initialize("shaders/newPT.hlsl");
    PathTracerBase::initializeFromCode(mShaderCode.c_str());
}

void PathTracer::setResources(const PathTracerDesc& desc)
{
    mMdlSampler = desc.matSampler;
    mMatTextures = desc.matTextures;
    mShaderParams.setTexture("output", mSharedCtx.mResManager->getView(desc.result));

    // mdl_ro_data_segment, mdl_argument_block, mdl_resource_infos, mdl_textures_2d, mdl_sampler_tex
    mShaderParams.setBuffer("mdl_ro_data_segment", mSharedCtx.mResManager->getVkBuffer(desc.mdl_ro_data_segment));
    mShaderParams.setBuffer("mdl_argument_block", mSharedCtx.mResManager->getVkBuffer(desc.mdl_argument_block));
    mShaderParams.setBuffer("mdl_resource_infos", mSharedCtx.mResManager->getVkBuffer(desc.mdl_resource_infos));
    mShaderParams.setBuffer("mdlMaterials", mSharedCtx.mResManager->getVkBuffer(desc.mdl_mdlMaterial));
    
    mShaderParams.setTextures("mdl_textures_2d", mMatTextures);
    mShaderParams.setSampler("mdl_sampler_tex", mMdlSampler);

    mShaderParams.setTexture("gbWPos", mSharedCtx.mResManager->getView(desc.gbuffer->wPos));
    mShaderParams.setTexture("gbNormal", mSharedCtx.mResManager->getView(desc.gbuffer->normal));
    mShaderParams.setTexture("gbTangent", mSharedCtx.mResManager->getView(desc.gbuffer->tangent));    
    mShaderParams.setTexture("gbInstId", mSharedCtx.mResManager->getView(desc.gbuffer->instId));
    mShaderParams.setTexture("gbUV", mSharedCtx.mResManager->getView(desc.gbuffer->uv));
    mShaderParams.setBuffer("bvhNodes", mSharedCtx.mResManager->getVkBuffer(desc.bvhNodes));
    mShaderParams.setBuffer("vb", mSharedCtx.mResManager->getVkBuffer(desc.vb));
    mShaderParams.setBuffer("ib", mSharedCtx.mResManager->getVkBuffer(desc.ib));
    mShaderParams.setBuffer("instanceConstants", mSharedCtx.mResManager->getVkBuffer(desc.instanceConst));
    mShaderParams.setBuffer("materials", mSharedCtx.mResManager->getVkBuffer(desc.materials));
    mShaderParams.setBuffer("lights", mSharedCtx.mResManager->getVkBuffer(desc.lights));
}

} // namespace nevk
