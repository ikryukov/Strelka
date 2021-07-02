#pragma once
#include <vulkan/vulkan.h>

#include <memory>

namespace nevk
{

struct Buffer;
struct Image;

class ResourceManager
{
    class Context;
    std::unique_ptr<Context> mContext;

    VkDevice mDevice;
    VkPhysicalDevice mPhysicalDevice;
    VkCommandPool mCommandPool;
    VkQueue mGraphicsQueue;

public:
    ResourceManager(VkDevice device, VkPhysicalDevice physicalDevice, VkInstance instance, VkCommandPool commandPool, VkQueue graphicsQueue);

    // Buffer
    Buffer* createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    void destroyBuffer(Buffer* buffer);
    void* getMappedMemory(const Buffer* buffer);
    VkBuffer getVkBuffer(const Buffer* buffer);
    // Image
    Image* createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
    void destroyImage(Image* image);
    VkImage getVkImage(const Image* image);

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
};
} // namespace nevk
