#include "resourcemanager.h"

#include <stdexcept>
#define VMA_IMPLEMENTATION
#include <VmaUsage.h>

namespace nevk
{

struct Buffer
{
    VkBuffer handle = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
};

struct Image
{
    VkImage handle = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkFormat format;
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

    Buffer* createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, const char* name = nullptr)
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
        case VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT: {
            memUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
            allocFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
            break;
        }
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
        std::string bufferName;
        if (name)
        {
            vmaAllocInfo.flags |= VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
            bufferName = "Buffer: " + std::string(name);
            vmaAllocInfo.pUserData = (void*)bufferName.c_str();
        }

        Buffer* ret = new Buffer();
        VkResult res = vmaCreateBuffer(mAllocator, &bufferInfo, &vmaAllocInfo, &ret->handle, &ret->allocation, nullptr);
        if (res != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create buffer!");
        }

        return ret;
    }
    void* getMappedMemory(const Buffer* buffer)
    {
        assert(buffer);
        VmaAllocationInfo allocInfo = {};
        vmaGetAllocationInfo(mAllocator, buffer->allocation, &allocInfo);
        return allocInfo.pMappedData;
    }

    void destroyBuffer(Buffer* buffer)
    {
        assert(buffer);
        vmaDestroyBuffer(mAllocator, buffer->handle, buffer->allocation);
        delete buffer;
        buffer = nullptr;
    }

    VkBuffer getVkBuffer(const Buffer* buffer)
    {
        if (buffer)
            return buffer->handle;
        else
            return nullptr;
    }

    Image* createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, const char* name = nullptr)
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

        VmaAllocationCreateInfo vmaAllocInfo = {};
        vmaAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        VmaAllocationInfo allocInfo = {};
        std::string imageName;
        if (name)
        {
            vmaAllocInfo.flags |= VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
            imageName = "Image: " + std::string(name);
            vmaAllocInfo.pUserData = (void*)imageName.c_str();
        }

        Image* ret = new Image();
        if (vmaCreateImage(mAllocator, &imageInfo, &vmaAllocInfo, &ret->handle, &ret->allocation, nullptr) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image!");
        }
        ret->format = format;

        VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
        if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
        }
        ret->view = createImageView(ret, aspectFlags);

        return ret;
    }

    VkImageView createImageView(const Image* image, VkImageAspectFlags aspectFlags)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image->handle;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = image->format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        if (vkCreateImageView(mDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create texture image view!");
        }
        return imageView;
    }

    void destroyImage(Image* image)
    {
        assert(image);
        vmaDestroyImage(mAllocator, image->handle, image->allocation);
        vkDestroyImageView(mDevice, image->view, nullptr);
        delete image;
        image = nullptr;
    }

    VkImage getVkImage(const Image* image)
    {
        return image->handle;
    }

    void getStats()
    {
        char* stats = nullptr;
        vmaBuildStatsString(mAllocator, &stats, true);
        printf("Alloc stats: %s\n", stats);
        vmaFreeStatsString(mAllocator, stats);
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

ResourceManager::~ResourceManager()
{
    // Uncomment this line to debug memory leak
    //mContext->getStats();
    mContext.reset(nullptr);
}

Buffer* ResourceManager::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, const char* name)
{
    return mContext->createBuffer(size, usage, properties, name);
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

size_t ResourceManager::getSize(const Buffer* buffer)
{
    return buffer->allocation->GetSize();
}

Image* ResourceManager::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, const char* name)
{
    return mContext->createImage(width, height, format, tiling, usage, properties, name);
}

void ResourceManager::destroyImage(Image* image)
{
    return mContext->destroyImage(image);
}

VkImage ResourceManager::getVkImage(const Image* image)
{
    return mContext->getVkImage(image);
}

VkImageView ResourceManager::getView(Image* image)
{
    return image->view;
}

VkImageView ResourceManager::createImageView(const Image* image, VkImageAspectFlags aspectFlags)
{
    return mContext->createImageView(image, aspectFlags);
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
