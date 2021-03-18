#pragma once
#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>
#include <unordered_map>

#include <resourcemanager/resourcemanager.h>

namespace nevk
{
class TextureManager
{
private:
    VkDevice mDevice;
    VkPhysicalDevice mPhysicalDevice;

    nevk::ResourceManager* mResManager;

public:
    TextureManager(VkDevice device,
                   VkPhysicalDevice phDevice,
                   nevk::ResourceManager* ResManager)
        : mDevice(device), mPhysicalDevice(phDevice), mResManager(ResManager){};

    struct Texture
    {
        VkImage textureImage;
        int texWidth;
        int texHeight;
        VkDeviceMemory textureImageMemory;
    };

    std::unordered_map<std::string , uint32_t > nameID{};
    std::vector<Texture> textures;
    std::vector<VkImageView> textureImageView;
    VkSampler textureSampler;

    int loadTexture(std::string& texture_path, const std::string& MTL_PATH);

    Texture createTextureImage(std::string texture_path);
    void createTextureImageView(Texture texture);
    void createTextureSampler();

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    void textureDestroy()
    {
        vkDestroySampler(mDevice, textureSampler, nullptr);

        for (VkImageView image_view : textureImageView) {
            vkDestroyImageView(mDevice, image_view, nullptr);
        }

        for (Texture tex : textures)
        {
            vkDestroyImage(mDevice, tex.textureImage, nullptr);
            vkFreeMemory(mDevice, tex.textureImageMemory, nullptr);
        }
    }
};
} // namespace nevk
