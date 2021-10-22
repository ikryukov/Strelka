#pragma once
#include "common.h"
#include "computepass.h"
#include "gbuffer.h"
#include "ltcparam.h"

namespace nevk
{
struct LtcResourceDesc
{
    GBuffer* gbuffer = VK_NULL_HANDLE;
    Buffer* lights = VK_NULL_HANDLE;
    Buffer* materials = VK_NULL_HANDLE;
    Buffer* instanceConst = VK_NULL_HANDLE;
    // bindless material
    std::vector<Image*> matTextures;
    std::vector<VkSampler> matSampler;
    // output
    Image* result = VK_NULL_HANDLE;
};

using LtcPassBase = ComputePass<LtcParam>;
class LtcPass : public LtcPassBase
{
private:
    Image* mLtc1 = VK_NULL_HANDLE;
    Image* mLtc2 = VK_NULL_HANDLE;
    VkSampler mLtcSampler = VK_NULL_HANDLE;

public:
    std::vector<VkSampler> mMatSamplers;
    std::vector<Image*> mMatTextures;

    LtcPass(const SharedContext& ctx);
    ~LtcPass();
    void initialize();
    void setResources(const LtcResourceDesc& desc);
};
} // namespace nevk
