#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <texturemanager.h>

#include "bindless.h"

void oka::TextureManager::savePNG(int32_t width, int32_t height, uint8_t* colorData)
{
    stbi_write_png("result.png", width, height, 3, colorData, 3 * width);
}

int oka::TextureManager::loadTextureGltf(const void* pixels,
                                         const uint32_t width,
                                         const uint32_t height,
                                         const std::string& name)
{
    if (mNameToID.count(name) == 0)
    {
        mNameToID[name] = textures.size();
        Texture tex = createTextureImage(pixels, width, height);
        textures.push_back(tex);
        textureImages.push_back(tex.textureImage);

        createTextureImageView(tex);
    }

    return mNameToID.find(name)->second;
}

VkFormat getVkFormat(const char* format)
{
    if ((strcmp(format, "Color") == 0) || (strcmp(format, "Float<4>") == 0))
    {
        return VK_FORMAT_R32G32B32A32_SFLOAT;
    }
    else if (strcmp(format, "Float<3>") == 0)
    {
        return VK_FORMAT_R32G32B32A32_SFLOAT;
    }
    else
    {
        return VK_FORMAT_R32G32B32A32_SFLOAT;
    }
}

int oka::TextureManager::loadTextureMdl(
    const void* pixels, const uint32_t width, const uint32_t height, const char* format, const std::string& name)
{
    VkFormat vkFormat = getVkFormat(format);
    assert(vkFormat == VK_FORMAT_R32G32B32A32_SFLOAT);
    if (mNameToID.count(name) == 0)
    {
        mNameToID[name] = textures.size();
        Texture tex = createTextureImage(pixels, 4 * 4, vkFormat, width, height, name.c_str());
        textures.push_back(tex);
        textureImages.push_back(tex.textureImage);

        createTextureImageView(tex);
    }

    return mNameToID.find(name)->second;
}

int oka::TextureManager::loadTextureMdl(const std::string& texture_path)
{
    if (mNameToID.find(texture_path) == mNameToID.end())
    {
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(texture_path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4;
        if (!pixels)
        {
            throw std::runtime_error("failed to load texture image!");
        }
        mNameToID[texture_path] = textures.size();
        Texture tex = createTextureImage(pixels, 4 * sizeof(char), VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, texture_path.c_str());
        textures.push_back(tex);
        textureImages.push_back(tex.textureImage);

        createTextureImageView(tex);
        stbi_image_free(pixels);

        if (textures.size() >= BINDLESS_TEXTURE_COUNT)
        {
            printf("WARNING: texture count is limited to %d, output image might be corrupted.", BINDLESS_TEXTURE_COUNT);
        }
    }
    return mNameToID.find(texture_path)->second;
}

oka::TextureManager::Texture oka::TextureManager::createCubeMapTextureImage(std::string texture_path[6])
{
    int texWidth, texHeight, texChannels;
    const uint8_t* pixels[6];

    for (uint32_t i = 0; i < 6; ++i)
    {
        pixels[i] = stbi_load(texture_path[i].c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4;
        if (!pixels[i])
        {
            throw std::runtime_error("failed to load texture image!");
        }
    }

    Texture res = createCubeMapImage(pixels, 4, VK_FORMAT_R8G8B8A8_UNORM, texWidth, texHeight, "Cube Map Texture");
    for (uint32_t i = 0; i < 6; ++i)
    {
        //  stbi_image_free(pixels[i]);
    }

    return res;
}

oka::TextureManager::Texture oka::TextureManager::createTextureImage(const std::string& texture_path)
{
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(texture_path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;
    if (!pixels)
    {
        throw std::runtime_error("failed to load texture image!");
    }

    Texture res = createTextureImage(pixels, texWidth, texHeight);

    stbi_image_free(pixels);

    return res;
}

oka::TextureManager::Texture oka::TextureManager::createTextureImage(const void* pixels, uint32_t width, uint32_t height)
{
    return createTextureImage(pixels, VK_FORMAT_R8G8B8A8_UNORM, width, height);
}

oka::TextureManager::Texture oka::TextureManager::createTextureImage(const void* pixels,
                                                                     VkFormat format,
                                                                     uint32_t width,
                                                                     uint32_t height)
{
    return createTextureImage(pixels, 4, format, width, height);
}

oka::TextureManager::Texture oka::TextureManager::createCubeMapImage(
    const uint8_t* pixels[6], uint32_t bytesPerPixel, VkFormat format, uint32_t width, uint32_t height, const char* name)
{
    VkDeviceSize imageSize = width * height * bytesPerPixel;
    Buffer* stagingBuffer =
        mResManager->createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT); // same

    Image* textureImage = mResManager->createCubeMapImage(width, height, format, VK_IMAGE_TILING_OPTIMAL,
                                                          VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, name);

    void* stagingBufferMemory = mResManager->getMappedMemory(stagingBuffer);
    for (uint32_t i = 0; i < 6; ++i)
    {
        memcpy(stagingBufferMemory, pixels[i], static_cast<size_t>(imageSize));

        transitionImageLayout(mResManager->getVkImage(textureImage), format, VK_IMAGE_LAYOUT_UNDEFINED,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, i);
        copyBufferToImage(mResManager->getVkBuffer(stagingBuffer), mResManager->getVkImage(textureImage),
                          static_cast<uint32_t>(width), static_cast<uint32_t>(height), i);
        transitionImageLayout(mResManager->getVkImage(textureImage), format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, i);
    }
    mResManager->destroyBuffer(stagingBuffer);

    return Texture{ textureImage, width, height };
}

oka::TextureManager::Texture oka::TextureManager::createTextureImage(
    const void* pixels, uint32_t bytesPerPixel, VkFormat format, uint32_t width, uint32_t height, const char* name)
{
    VkDeviceSize imageSize = width * height * bytesPerPixel;
    Buffer* stagingBuffer =
        mResManager->createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* stagingBufferMemory = mResManager->getMappedMemory(stagingBuffer);
    memcpy(stagingBufferMemory, pixels, static_cast<size_t>(imageSize));

    Image* textureImage = mResManager->createImage(width, height, format, VK_IMAGE_TILING_OPTIMAL,
                                                   VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, name);

    transitionImageLayout(
        mResManager->getVkImage(textureImage), format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(mResManager->getVkBuffer(stagingBuffer), mResManager->getVkImage(textureImage),
                      static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    transitionImageLayout(mResManager->getVkImage(textureImage), format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    mResManager->destroyBuffer(stagingBuffer);

    return Texture{ textureImage, width, height };
}

int oka::TextureManager::findTexture(const std::string& name)
{
    auto it = mNameToID.find(name);
    if (it == mNameToID.end())
    {
        return -1;
    }
    return it->second;
}

void oka::TextureManager::createTextureImageView(Texture& texture)
{
    textureImageView.push_back(mResManager->getView(texture.textureImage));
}

void oka::TextureManager::createTextureSampler(TextureSamplerDesc& texSamplerData)
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(mPhysicalDevice, &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = texSamplerData.magFilter;
    samplerInfo.minFilter = texSamplerData.minFilter;
    samplerInfo.addressModeU = texSamplerData.addressModeU;
    samplerInfo.addressModeV = texSamplerData.addressModeV;
    samplerInfo.addressModeW = texSamplerData.addressModeV;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    VkSampler tmpSampler;
    if (vkCreateSampler(mDevice, &samplerInfo, nullptr, &tmpSampler) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture sampler!");
    }

    sampDescToId[texSamplerData] = texSamplers.size();
    texSamplers.push_back(tmpSampler);
}

void oka::TextureManager::createShadowSampler()
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(mPhysicalDevice, &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 1.0f;

    if (vkCreateSampler(mDevice, &samplerInfo, nullptr, &shadowSampler) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

VkImageView oka::TextureManager::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
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

void oka::TextureManager::transitionImageLayout(
    VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t layer)
{
    VkCommandBuffer commandBuffer = mResManager->beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = layer;
    barrier.subresourceRange.layerCount = 1; // ?

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    // TODO: need to verify!
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_GENERAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.subresourceRange.aspectMask =
            (format == VK_FORMAT_D32_SFLOAT) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    }
    else
    {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    mResManager->endSingleTimeCommands(commandBuffer);
}

void oka::TextureManager::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layer)
{
    VkCommandBuffer commandBuffer = mResManager->beginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = layer;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { width, height, 1 };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    mResManager->endSingleTimeCommands(commandBuffer);
}
