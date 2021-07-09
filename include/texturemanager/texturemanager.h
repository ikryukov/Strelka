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
        Image* textureImage;
        uint32_t texWidth;
        uint32_t texHeight;
    };

    std::unordered_map<std::string, uint32_t> mNameToID{};
    std::vector<Texture> textures;
    std::vector<VkImageView> textureImageView;
    VkSampler textureSampler = VK_NULL_HANDLE;
    VkSampler shadowSampler = VK_NULL_HANDLE;

    std::vector<Image*> delTextures;
    std::vector<VkImageView> delTextureImageView;
    std::vector<VkSampler> delTextureSampler;
    std::vector<VkSampler> delShadowSampler;

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

        for (VkImageView& imageView : textureImageView)
        {
            if (imageView != VK_NULL_HANDLE)
            {
                vkDestroyImageView(mDevice, imageView, nullptr);
            }
        }

        for (Texture& tex : textures)
        {
            mResManager->destroyImage(tex.textureImage);
        }

        textures.clear();
        textureImageView.clear();
        mNameToID.clear();
    }

    void saveTexturesInDelQueue()
    {
        delShadowSampler.push_back(shadowSampler);
        delTextureSampler.push_back(textureSampler);

        for (VkImageView& imageView : textureImageView)
        {
            if (imageView != VK_NULL_HANDLE)
            {
                delTextureImageView.push_back(imageView);
            }
        }

        for (Texture& tex : textures)
        {
            delTextures.push_back(tex.textureImage);
        }

        textureImageView.clear();
        textures.clear();
        mNameToID.clear();
        textureSampler = VK_NULL_HANDLE;
        shadowSampler = VK_NULL_HANDLE;
    }

    void delTexturesFromQueue()
    {
        for (VkSampler sampler : delTextureSampler)
        {
            if (sampler != VK_NULL_HANDLE)
                vkDestroySampler(mDevice, sampler, nullptr);
        }

        for (VkSampler sampler : delShadowSampler)
        {
            if (sampler != VK_NULL_HANDLE)
                vkDestroySampler(mDevice, sampler, nullptr);
        }

        for (VkImageView imageView : delTextureImageView)
        {
            if (imageView != VK_NULL_HANDLE)
            {
                vkDestroyImageView(mDevice, imageView, nullptr);
            }
        }

        for (Image* delTexture : delTextures)
        {
            mResManager->destroyImage(delTexture);
        }

        delTextures.clear();
        delTextureImageView.clear();
        delTextureSampler.clear();
        delShadowSampler.clear();
    }
};
} // namespace nevk
