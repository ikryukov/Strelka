#pragma once
#include "common.h"
#include "computepass.h"
#include "gbuffer.h"
#include "ltcparam.h"

namespace nevk
{
struct LtcResourceDesc
{
    GBuffer* gbuffer;
    Buffer* lights;
    Buffer* materials;
    Buffer* instanceConst;
    // bindless material
    std::vector<Image*> matTextures;
    std::vector<VkSampler> matSampler;
    // output
    Image* result;
};

using LtcPassBase = ComputePass<LtcParam>;
class LtcPass : public LtcPassBase
{
private:
    Image* mLtc1 = nullptr;
    Image* mLtc2 = nullptr;
    VkSampler mLtcSampler = VK_NULL_HANDLE;

public:
    LtcPass(const SharedContext& ctx);
    ~LtcPass();
    void initialize();
    void setResources(LtcResourceDesc& desc);
};
} // namespace nevk
