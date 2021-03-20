#pragma once

#include "renderpass/renderpass.h"

namespace nevk
{

class FullScreenPass : public RenderPass
{
protected:
    void createGraphicsPipeline() override;

    void createFrameBuffers(std::vector<VkImageView>& imageViews);

public:
    void record(VkCommandBuffer& cmd, uint32_t imageIndex);
    void onResize(std::vector<VkImageView>& imageViews, uint32_t width, uint32_t height);
};

} // namespace nevk
