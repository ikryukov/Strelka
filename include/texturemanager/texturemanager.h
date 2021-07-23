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
    std::vector<VkImageView> textureImageView;
    VkSampler shadowSampler = VK_NULL_HANDLE;

    std::vector<Image*> delTextures;
    std::vector<VkImageView> delTextureImageView;
    std::vector<VkSampler> delShadowSampler;

    int loadTexture(const std::string& texture_path, const std::string& MTL_PATH);

    int loadTextureGltf(const void* pixels, const uint32_t width, const uint32_t height, const std::string& name);
    int findTexture(const std::string& name);

    void createTextureSampler(TextureSamplerDesc& texSamplerData);

    Texture createTextureImage(const std::string& texture_path);
    Texture createTextureImage(const void* pixels, uint32_t width, uint32_t height);

    void createTextureImageView(Texture& texture);
    void createShadowSampler();

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

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

    void initSamplers()
    {
        TextureSamplerDesc sampler{};

        sampler = { VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT };
        createTextureSampler(sampler);

        sampler = { VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE};
        createTextureSampler(sampler);

        sampler = { VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT};
        createTextureSampler(sampler);

        sampler = { VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_REPEAT};
        createTextureSampler(sampler);

        sampler = { VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE };
        createTextureSampler(sampler);

        sampler = { VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT };
        createTextureSampler(sampler);

        sampler = { VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT };
        createTextureSampler(sampler);

        sampler = { VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE};
        createTextureSampler(sampler);

        sampler = { VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT};
        createTextureSampler(sampler);

        sampler = { VK_FILTER_LINEAR, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT};
        createTextureSampler(sampler);

        sampler = { VK_FILTER_LINEAR, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE};
        createTextureSampler(sampler);

        sampler = { VK_FILTER_LINEAR, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT};
        createTextureSampler(sampler);

        sampler = { VK_FILTER_LINEAR, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_REPEAT};
        createTextureSampler(sampler);

        sampler = { VK_FILTER_LINEAR, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE};
        createTextureSampler(sampler);

        sampler = { VK_FILTER_LINEAR, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT};
        createTextureSampler(sampler);

        sampler = { VK_FILTER_LINEAR, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT};
        createTextureSampler(sampler);

        sampler = { VK_FILTER_LINEAR, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE};
        createTextureSampler(sampler);

        sampler = { VK_FILTER_LINEAR, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT};
        createTextureSampler(sampler);

        sampler = { VK_FILTER_NEAREST, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT};
        createTextureSampler(sampler);

        sampler = { VK_FILTER_NEAREST, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE};
        createTextureSampler(sampler);

        sampler = { VK_FILTER_NEAREST, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT};
        createTextureSampler(sampler);

        sampler = { VK_FILTER_NEAREST, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_REPEAT};
        createTextureSampler(sampler);

        sampler = { VK_FILTER_NEAREST, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE};
        createTextureSampler(sampler);

        sampler = { VK_FILTER_NEAREST, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT};
        createTextureSampler(sampler);

        sampler = { VK_FILTER_NEAREST, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT};
        createTextureSampler(sampler);

        sampler = { VK_FILTER_NEAREST, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE};
        createTextureSampler(sampler);

        sampler = { VK_FILTER_NEAREST, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT};
        createTextureSampler(sampler);

        sampler = { VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT};
        createTextureSampler(sampler);

        sampler = { VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE};
        createTextureSampler(sampler);

        sampler = { VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT};
        createTextureSampler(sampler);

        sampler = { VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_REPEAT};
        createTextureSampler(sampler);

        sampler = { VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE};
        createTextureSampler(sampler);

        sampler = { VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT};
        createTextureSampler(sampler);

        sampler = { VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT};
        createTextureSampler(sampler);

        sampler = { VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE};
        createTextureSampler(sampler);

        sampler = { VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT};
        createTextureSampler(sampler);
    }
};
} // namespace nevk
