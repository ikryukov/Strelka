#pragma once
#include <vulkan/vulkan.h>

namespace nevk
{

class ResourceManager
{
private:
    VkDevice mDevice;
    VkPhysicalDevice mPhysicalDevice;
    
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

public:
    ResourceManager(VkDevice device, VkPhysicalDevice physicalDevice)
        : mDevice(device),
          mPhysicalDevice(physicalDevice)

    {
    }

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
};
} // namespace nevk
