#include "resourcemanager.h"

#include <stdexcept>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

namespace nevk
{

struct Buffer
{
    VkBuffer handle = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
};

class ResourceManager::Context
{
    VkDevice mDevice;
    VkPhysicalDevice mPhysicalDevice;
    VkInstance mInstance;

    VmaAllocator mAllocator = VK_NULL_HANDLE;

    VkResult createAllocator()
    {
        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.device = mDevice;
        allocatorInfo.physicalDevice = mPhysicalDevice;
        allocatorInfo.instance = mInstance;
        return vmaCreateAllocator(&allocatorInfo, &mAllocator);
    }

public:
    Context(VkDevice device, VkPhysicalDevice physicalDevice, VkInstance instance)
        : mDevice(device),
          mPhysicalDevice(physicalDevice),
          mInstance(instance)
    {
        createAllocator();
    }
    ~Context()
    {
        if (mAllocator)
        {
            vmaDestroyAllocator(mAllocator);
            mAllocator = VK_NULL_HANDLE;
        }
    }

    Buffer* createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
    {

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateFlags allocFlags = {};
        VmaMemoryUsage memUsage = {};
        // TODO: need to introduce flag isDevice to api?
        switch (usage)
        {
        case VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT:
        case VK_BUFFER_USAGE_TRANSFER_SRC_BIT: {
            memUsage = VMA_MEMORY_USAGE_CPU_ONLY;
            allocFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
            break;
        }
        case VK_BUFFER_USAGE_TRANSFER_DST_BIT: {
            memUsage = VMA_MEMORY_USAGE_GPU_ONLY;
            allocFlags = 0;
            break;
        }
        default:
            break;
        }

        VmaAllocationCreateInfo vmaAllocInfo = {};
        vmaAllocInfo.usage = memUsage;
        vmaAllocInfo.flags = allocFlags;
        VmaAllocationInfo allocInfo = {};

        Buffer* ret = new Buffer();
        if (vmaCreateBuffer(mAllocator, &bufferInfo, &vmaAllocInfo, &ret->handle, &ret->allocation, &allocInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create buffer!");
        }

        return ret;
    }
    void* getMappedMemory(const Buffer* buffer)
    {
        VmaAllocationInfo allocInfo = {};
        vmaGetAllocationInfo(mAllocator, buffer->allocation, &allocInfo);
        return allocInfo.pMappedData;
    }

    void destroyBuffer(Buffer* buffer)
    {
        vmaDestroyBuffer(mAllocator, buffer->handle, buffer->allocation);
    }

    VkBuffer getVkBuffer(const Buffer* buffer)
    {
        return buffer->handle;
    }
};

ResourceManager::ResourceManager(VkDevice device, VkPhysicalDevice physicalDevice, VkInstance instance, VkCommandPool commandPool, VkQueue graphicsQueue)
{
    mDevice = device;
    mPhysicalDevice = physicalDevice;
    mCommandPool = commandPool;
    mGraphicsQueue = graphicsQueue;
    mContext = std::make_unique<Context>(device, physicalDevice, instance);
}

uint32_t ResourceManager::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

Buffer* ResourceManager::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
{
    return mContext->createBuffer(size, usage, properties);
}

void* ResourceManager::getMappedMemory(const Buffer* buffer)
{
    return mContext->getMappedMemory(buffer);
}

void ResourceManager::destroyBuffer(Buffer* buffer)
{
    mContext->destroyBuffer(buffer);
}

VkBuffer ResourceManager::getVkBuffer(const Buffer* buffer)
{
    return mContext->getVkBuffer(buffer);
}

void ResourceManager::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(mDevice, &imageInfo, nullptr, &image) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(mDevice, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(mDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(mDevice, image, imageMemory, 0);
}
VkCommandBuffer ResourceManager::beginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    allocInfo.commandPool = mCommandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(mDevice, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void ResourceManager::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(mGraphicsQueue);

    vkFreeCommandBuffers(mDevice, mCommandPool, 1, &commandBuffer);
}

void ResourceManager::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
}

} // namespace nevk
