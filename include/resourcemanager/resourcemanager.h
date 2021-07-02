#pragma once
#include <vulkan/vulkan.h>

#include <memory>

namespace nevk
{

struct Buffer;

class ResourceManager
{
    class Context;
    std::unique_ptr<Context> mContext;

    VkDevice mDevice;
    VkPhysicalDevice mPhysicalDevice;
    VkCommandPool mCommandPool;
    VkQueue mGraphicsQueue;

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

public:
    ResourceManager(VkDevice device, VkPhysicalDevice physicalDevice, VkInstance instance, VkCommandPool commandPool, VkQueue graphicsQueue);

    Buffer* createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    void destroyBuffer(Buffer* buffer);
    void* getMappedMemory(const Buffer* buffer);
    VkBuffer getVkBuffer(const Buffer* buffer);

    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);


    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
};
} // namespace nevk
