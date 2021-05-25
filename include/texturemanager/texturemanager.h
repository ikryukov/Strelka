#pragma once
#include <resourcemanager/resourcemanager.h>
#include <vulkan/vulkan.h>

#include <iostream>
#include <unordered_map>
#include <vector>

namespace nevk
{
class TextureManager
{
private:
    VkDevice mDevice;
    VkPhysicalDevice mPhysicalDevice;

    nevk::ResourceManager* mResManager = nullptr;

public:
    TextureManager(VkDevice device,
                   VkPhysicalDevice phDevice,
                   nevk::ResourceManager* ResManager)
        : mDevice(device), mPhysicalDevice(phDevice), mResManager(ResManager){};

    struct Texture
    {
        VkImage textureImage;
        uint32_t texWidth;
        uint32_t texHeight;
        VkDeviceMemory textureImageMemory;
    };

    std::unordered_map<std::string, uint32_t> mNameToID{};
    std::vector<Texture> textures;
    std::vector<VkImageView> textureImageView;
    VkSampler textureSampler = VK_NULL_HANDLE;
    VkSampler shadowSampler = VK_NULL_HANDLE;

    int loadTexture(const std::string& texture_path, const std::string& MTL_PATH);

    int loadTextureGltf(const void* pixels, const uint32_t width, const uint32_t height, const std::string& name);
    int findTexture(const std::string& name);

    Texture createTextureImage(const std::string& texture_path);
    
    Texture createTextureImage(const void* pixels, uint32_t width, uint32_t height);

    void createTextureImageView(Texture& texture);
    void createTextureSampler();
    void createShadowSampler();

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    void textureDestroy()
    {
        vkDestroySampler(mDevice, textureSampler, nullptr);
        vkDestroySampler(mDevice, shadowSampler, nullptr);

        for (VkImageView& image_view : textureImageView)
        {
            if (image_view != VK_NULL_HANDLE)
            {
                vkDestroyImageView(mDevice, image_view, nullptr);
            }
        }

        for (Texture& tex : textures)
        {
            vkDestroyImage(mDevice, tex.textureImage, nullptr);
            vkFreeMemory(mDevice, tex.textureImageMemory, nullptr);
        }
    }
};
} // namespace nevk
