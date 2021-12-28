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
    // sampler
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    VkResult res = vkCreateSampler(mSharedCtx.mDevice, &samplerInfo, nullptr, &mCubeMapSampler);
    if (res != VK_SUCCESS)
    {
        // error
        assert(0);
    }
    mShaderParams.setSampler("cubeMapSampler", mCubeMapSampler);

    std::string texPath[6] = {"misc/skybox/right.jpg",
                              "misc/skybox/left.jpg",
                              "misc/skybox/top.jpg",
                              "misc/skybox/bottom.jpg",
                              "misc/skybox/front.jpg",
                              "misc/skybox/back.jpg"};
    nevk::TextureManager::Texture cubeMapTex = mSharedCtx.mTextureManager->createCubeMapTextureImage(texPath);
    mCubeMap = cubeMapTex.textureImage;
    mShaderParams.setCubeMap("cubeMap", mSharedCtx.mResManager->getView(mCubeMap));

    //PathTracerBase::initialize("shaders/pathtracer.hlsl");
    //PathTracerBase::initialize("shaders/newPT.hlsl");
    PathTracerBase::initializeFromCode(mShaderCode.c_str());
}

void PathTracer::setResources(const PathTracerDesc& desc)
{
    mMdlSampler = desc.matSampler;
    mMatTextures = desc.matTextures;

    mShaderParams.setBuffer("sampleBuffer", mSharedCtx.mResManager->getVkBuffer(desc.sampleBuffer));
    // mShaderParams.setBuffer("compositingBuffer", mSharedCtx.mResManager->getVkBuffer(desc.compositingBuffer));

    // mdl_ro_data_segment, mdl_argument_block, mdl_resource_infos, mdl_textures_2d, mdl_sampler_tex
    mShaderParams.setBuffer("mdl_ro_data_segment", mSharedCtx.mResManager->getVkBuffer(desc.mdl_ro_data_segment));
    mShaderParams.setBuffer("mdl_argument_block", mSharedCtx.mResManager->getVkBuffer(desc.mdl_argument_block));
    mShaderParams.setBuffer("mdl_resource_infos", mSharedCtx.mResManager->getVkBuffer(desc.mdl_resource_infos));
    mShaderParams.setBuffer("mdlMaterials", mSharedCtx.mResManager->getVkBuffer(desc.mdl_mdlMaterial));
    
    mShaderParams.setTextures("mdl_textures_2d", mMatTextures);
   // mShaderParams.setTextures3d("mdl_textures_3d", mMatTextures);
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
