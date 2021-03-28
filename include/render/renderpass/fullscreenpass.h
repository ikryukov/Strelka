#pragma once
#include <vulkan/vulkan.h>
#include "vertex.h"
#include <vector>
#include <array>
#include <resourcemanager.h>
#include <glm/gtx/compatibility.hpp>
#include "renderpass.h"

namespace nevk
{
class FullscreenPass : public RenderPass
{
private:
    VkImageView mTextureImageView;
    VkImageView mOutImageView;
    VkSampler mTextureSampler;

public:

    void init(VkDevice& device, const char* vsCode, uint32_t vsCodeSize, const char* psCode, uint32_t psCodeSize, VkDescriptorPool descpool, ResourceManager* resMngr, uint32_t width, uint32_t height);

    void record(VkCommandBuffer& cmd, uint32_t indicesCount, uint32_t width, uint32_t height, uint32_t imageIndex);

    void createFrameBuffers(std::vector<VkImageView>& imageViews, VkImageView& depthImageView, uint32_t width, uint32_t height);

    void onResize(std::vector<VkImageView>& imageViews, VkImageView& depthImageView, uint32_t width, uint32_t height);

    void createDescriptorSets(VkDescriptorPool& descriptorPool);

    void updateDescriptorSets();

    void setOutputImageView(VkImageView imageView)
    {
        mOutImageView = imageView;
    }
	
	FullscreenPass(/* args */);
    ~FullscreenPass();

};

} // namespace nevk