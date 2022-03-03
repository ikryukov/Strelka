#pragma once
#include "common.h"
#include "computepass.h"
#include "gbuffer.h"
#include "pathtracerparam.h"

namespace oka
{
struct PathTracerDesc
{
    PathTracerParam constants;

    // mdl_ro_data_segment, mdl_argument_block, mdl_resource_infos, mdl_textures_2d, mdl_sampler_tex
    Buffer* mdl_ro_data_segment = VK_NULL_HANDLE;
    Buffer* mdl_argument_block = VK_NULL_HANDLE;
    Buffer* mdl_resource_infos = VK_NULL_HANDLE;
    Buffer* mdl_mdlMaterial = nullptr;

    // GBuffer* gbuffer = VK_NULL_HANDLE;
    Buffer* bvhNodes = VK_NULL_HANDLE;
    Buffer* lights = VK_NULL_HANDLE;
    Buffer* vb = VK_NULL_HANDLE;
    Buffer* ib = VK_NULL_HANDLE;
    Buffer* sampleBuffer = VK_NULL_HANDLE;
    Buffer* compositingBuffer = VK_NULL_HANDLE;
    Image* result = VK_NULL_HANDLE;
    Buffer* materials = VK_NULL_HANDLE;
    Buffer* instanceConst = VK_NULL_HANDLE;
    std::vector<Image*> matTextures;
    VkSampler matSampler;
};

using PathTracerBase = ComputePass<PathTracerParam>;
class PathTracer : public PathTracerBase
{
public:
    VkSampler mMdlSampler;
    std::vector<Image*> mMatTextures;
    std::string mShaderCode;

    VkSampler mCubeMapSampler = VK_NULL_HANDLE;
    Image* mCubeMap = VK_NULL_HANDLE;

    PathTracer(const SharedContext& ctx, std::string& shaderCode);
    ~PathTracer();

    void execute(VkCommandBuffer& cmd, const PathTracerDesc& desc, uint32_t width, uint32_t height, uint64_t frameIndex);

    void initialize();
};
} // namespace oka
