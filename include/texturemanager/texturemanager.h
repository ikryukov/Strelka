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
        : mDevice(device), mPhysicalDevice(phDevice), mResManager(ResManager)
    {
        // sampler
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = 16.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

        VkResult res = vkCreateSampler(mDevice, &samplerInfo, nullptr, &mMdlSampler);
        if (res != VK_SUCCESS)
        {
            // error
            assert(0);
        }
    };

    struct Texture
    {
        Image* textureImage;
        uint32_t texWidth;
        uint32_t texHeight;
    };

    struct TextureSamplerDesc
    {
        VkFilter magFilter;
        VkFilter minFilter;
        VkSamplerAddressMode addressModeU;
        VkSamplerAddressMode addressModeV;

        TextureSamplerDesc()
        {
        }
        TextureSamplerDesc(VkFilter magF, VkFilter minF, VkSamplerAddressMode modeU, VkSamplerAddressMode modeV)
        {
            magFilter = magF;
            minFilter = minF;
            addressModeU = modeU;
            addressModeV = modeV;
        }

        bool operator==(const TextureSamplerDesc& samplerDesc) const
        {
            return (magFilter == samplerDesc.magFilter && minFilter == samplerDesc.minFilter && addressModeU == samplerDesc.addressModeU && addressModeV == samplerDesc.addressModeV);
        }

        struct HashFunction
        {
            size_t operator()(const TextureSamplerDesc& samplerDesc) const
            {
                return (((((std::hash<VkFilter>()(samplerDesc.magFilter) ^
                    (std::hash<VkFilter>()(samplerDesc.minFilter) << 1)) >> 1) ^
                    (std::hash<VkSamplerAddressMode>()(samplerDesc.addressModeU) << 1)) >> 1) ^
                    (std::hash<VkSamplerAddressMode>()(samplerDesc.addressModeV) << 1));
            }
        };
    };

    std::unordered_map<std::string, uint32_t> mNameToID{};
    std::unordered_map<TextureSamplerDesc, uint32_t, TextureSamplerDesc::HashFunction> sampDescToId;
    std::vector<Texture> textures;
    std::vector<VkSampler> texSamplers;
    std::vector<Image*> textureImages;
    std::vector<VkImageView> textureImageView;
    VkSampler shadowSampler = VK_NULL_HANDLE;
    VkSampler mMdlSampler = VK_NULL_HANDLE;

    std::vector<Image*> delTextures;
    std::vector<VkImageView> delTextureImageView;
    std::vector<VkSampler> delShadowSampler;

    int loadTextureGltf(const void* pixels, const uint32_t width, const uint32_t height, const std::string& name);
    int loadTextureMdl(const void* pixels, const uint32_t width, const uint32_t height, const char* format, const std::string& name);
    int findTexture(const std::string& name);
    void savePNG(int32_t width, int32_t height, uint8_t* colorData);

    void createTextureSampler(TextureSamplerDesc& texSamplerData);

    Texture createCubeMapTextureImage(std::string texture_path[6]);
    Texture createCubeMapImage(const uint8_t* pixels[6], uint32_t bytesPerPixel, VkFormat format, uint32_t width, uint32_t height, const char* name);

    Texture createTextureImage(const std::string& texture_path);
    Texture createTextureImage(const void* pixels, uint32_t width, uint32_t height);
    Texture createTextureImage(const void* pixels, VkFormat format, uint32_t width, uint32_t height);
    Texture createTextureImage(const void* pixels, uint32_t bytesPerPixel, VkFormat format, uint32_t width, uint32_t height, const char* name = "");

    void createTextureImageView(Texture& texture);
    void createShadowSampler();

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t layer = 0);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layer = 0);

    void textureDestroy()
    {
        vkDestroySampler(mDevice, shadowSampler, nullptr);

        for (VkSampler sampler : texSamplers)
        {
            if (sampler != VK_NULL_HANDLE)
                vkDestroySampler(mDevice, sampler, nullptr);
        }

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

        textureImages.clear();

        textures.clear();
        textureImageView.clear();
        mNameToID.clear();
    }

    void saveTexturesInDelQueue()
    {
        delShadowSampler.push_back(shadowSampler);

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

        textureImages.clear();
        textureImageView.clear();
        textures.clear();
        mNameToID.clear();
        shadowSampler = VK_NULL_HANDLE;
    }

    void delTexturesFromQueue()
    {
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
        delShadowSampler.clear();
    }
};
} // namespace nevk
