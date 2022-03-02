#pragma once
#include <vulkan/vulkan.h>

#include <memory>

namespace oka
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
    ~ResourceManager();

    // Buffer
    Buffer* createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, const char* name = nullptr);
    void destroyBuffer(Buffer* buffer);
    void* getMappedMemory(const Buffer* buffer);
    VkBuffer getVkBuffer(const Buffer* buffer);
    size_t getSize(const Buffer* buffer);
    // Image
    Image* createCubeMapImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, const char* name = nullptr);
    VkImageView createCubeMapImageView(const Image* image, VkImageAspectFlags aspectFlags);

    Image* createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, const char* name = nullptr);
    void destroyImage(Image* image);
    VkImage getVkImage(const Image* image);
    VkImageLayout getImageLayout(Image* image);
    void setImageLayout(Image* image, VkImageLayout newLayout);
    VkImageView getView(Image* image);

    VkImageView createImageView(const Image* image, VkImageAspectFlags aspectFlags);

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
};
} // namespace oka
