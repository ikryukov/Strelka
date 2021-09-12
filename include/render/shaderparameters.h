#pragma once
#include "common.h"

#include <array>
#include <unordered_map>
#include <vector>

namespace nevk
{
template <typename T>
class ShaderParameters
{
protected:
    VkDevice mDevice;
    ResourceManager* mResManager = nullptr;
    std::array<T, MAX_FRAMES_IN_FLIGHT> mConstants;
    Buffer* mConstantBuffer = nullptr;

    VkDescriptorSetLayout mDescriptorSetLayout = VK_NULL_HANDLE;
    std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> mDescriptorSets;

    std::array<bool, MAX_FRAMES_IN_FLIGHT> needDesciptorSetUpdate = { false, false, false };
    std::array<bool, MAX_FRAMES_IN_FLIGHT> needConstantsUpdate = { false, false, false };

    std::vector<ShaderManager::ResourceDesc> mResourcesDescs;
    std::unordered_map<std::string, ShaderManager::ResourceDesc> mNameToDesc;

    struct ResourceDescriptor
    {
        ShaderManager::ResourceType type;
        union
        {
            VkImageView imageView;
            VkBuffer buffer;
            VkSampler sampler;
        };
    };
    std::array<std::unordered_map<std::string, ResourceDescriptor>, MAX_FRAMES_IN_FLIGHT> mResUpdate = {};

    std::array<std::vector<VkWriteDescriptorSet>, MAX_FRAMES_IN_FLIGHT> mDescriptorWrites = {};

    NeVkResult createUniformBuffers()
    {
        VkDeviceSize bufferSize = sizeof(T) * MAX_FRAMES_IN_FLIGHT;
        mConstantBuffer = mResManager->createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        if (!mConstantBuffer)
        {
            return NeVkResult::eOutOfMemory;
        }
        return NeVkResult::eOk;
    }


    NeVkResult createDescriptorSetLayout()
    {
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        bindings.reserve(mResourcesDescs.size());
        for (const auto& desc : mResourcesDescs)
        {
            VkDescriptorType descType = VK_DESCRIPTOR_TYPE_MAX_ENUM;
            switch (desc.type)
            {
            case ShaderManager::ResourceType::eConstantBuffer: {
                descType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                break;
            }
            case ShaderManager::ResourceType::eTexture2D: {
                descType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                break;
            }
            case ShaderManager::ResourceType::eRWTexture2D: {
                descType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                break;
            }
            case ShaderManager::ResourceType::eStructuredBuffer: {
                descType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                break;
            }
            default:
                break;
            }
            VkDescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = desc.binding;
            layoutBinding.descriptorCount = 1; // TODO: fix count
            layoutBinding.descriptorType = descType;
            layoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT; // TODO: fix for graphics
            bindings.push_back(layoutBinding);
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        VkResult res = vkCreateDescriptorSetLayout(mDevice, &layoutInfo, nullptr, &mDescriptorSetLayout);
        if (res != VK_SUCCESS)
        {
            return NeVkResult::eFail;
        }
        return NeVkResult::eOk;
    }

    NeVkResult createDescriptorSets(const VkDescriptorPool& descriptorPool)
    {
        std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, mDescriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        VkResult res = vkAllocateDescriptorSets(mDevice, &allocInfo, mDescriptorSets.data());
        if (res != VK_SUCCESS)
        {
            return NeVkResult::eFail;
        }
        return NeVkResult::eOk;
    }

    NeVkResult updateDescriptorSet(uint32_t descIndex)
    {
        VkDescriptorSet& dstDescSet = mDescriptorSets[descIndex];

        auto& resourcesToUpdate = mResUpdate[descIndex];

        uint32_t buffCount = 0;
        uint32_t texCount = 0;

        for (auto& currRes : resourcesToUpdate)
        {
            switch (currRes->second.type)
            {
            case ShaderManager::ResourceType::eConstantBuffer:
            case ShaderManager::ResourceType::eStructuredBuffer: {
                ++buffCount;
                break;
            }
            case ShaderManager::ResourceType::eTexture2D:
            case ShaderManager::ResourceType::eRWTexture2D: {
                ++texCount;
            }
            default:
                break;
            }
        }

        std::vector<VkDescriptorImageInfo> imageInfos(texCount);
        uint32_t imageInfosOffset = 0;
        std::vector<VkDescriptorBufferInfo> bufferInfos(texCount);
        uint32_t bufferInfosOffset = 0;

        std::vector<VkWriteDescriptorSet> descriptorWrites;
        for (auto& currRes : resourcesToUpdate)
        {
            std::string& name = currRes->first;
            auto& it = mNameToDesc.find(name);
            if (it != mNameToDesc.end())
            {
                auto& resDesc = it->second;

                switch (resDesc.type)
                {
                case ShaderManager::ResourceType::eTexture2D: {
                    VkDescriptorImageInfo& imageInfo = imageInfos[imageInfosOffset++];
                    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    imageInfo.imageView = view;

                    VkWriteDescriptorSet descWrite{};
                    descWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descWrite.dstSet = dstDescSet;
                    descWrite.dstBinding = resDesc.binding;
                    descWrite.dstArrayElement = 0;
                    descWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                    descWrite.descriptorCount = 1; // TODO:
                    descWrite.pImageInfo = &imageInfo;

                    descriptorWrites.push_back(descWrite);
                    break;
                }
                case ShaderManager::ResourceType::eStructuredBuffer: {
                    VkDescriptorBufferInfo& bufferInfo = bufferInfos[bufferInfosOffset++];
                    bufferInfo.buffer = mInstanceConstantsBuffer;
                    bufferInfo.offset = 0;
                    bufferInfo.range = VK_WHOLE_SIZE;

                    VkWriteDescriptorSet descWrite{};
                    descWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descWrite.dstSet = dstDescSet;
                    descWrite.dstBinding = resDesc.binding;
                    descWrite.dstArrayElement = 0;
                    descWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    descWrite.descriptorCount = 1; // TODO:
                    descWrite.pBufferInfo = &bufferInfo;

                    descriptorWrites.push_back(descWrite);
                    break;
                }
                default:
                    break;
                }
            }
            else
            {
                // not found
                return NeVkResult::eFail;
            }
        }

        vkUpdateDescriptorSets(mDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        needDesciptorSetUpdate[descIndex] = false;

        return NeVkResult::eOk;
    }

public:
    VkDescriptorSetLayout getDescriptorSetLayout()
    {
        return mDescriptorSetLayout;
    }
    VkDescriptorSet getDescriptorSet(const uint32_t index)
    {
        if (needDesciptorSetUpdate[index])
        {
            updateDescriptorSet(index);
        }
        return mDescriptorSets[index];
    }

    NeVkResult create(const SharedContext& ctx, const uint32_t shaderId)
    {
        mResourcesDescs = ctx.mShaderManager->getResourcesDesc(shaderId);
        for (auto& desc : mResourcesDescs)
        {
            mNameToDesc[desc.name] = desc;
        }
        mDevice = ctx.mDevice;
        mResManager = ctx.mResManager;

        NeVkResult res = createDescriptorSetLayout();

        res = createDescriptorSets(ctx.mDescriptorPool);

        res = createUniformBuffers();


        return res;
    }

    // setters
    void setTexture(const std::string& name, VkImageView view);
    void setBuffer(const std::string& name, VkBuffer buff)
    {
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
        {
            ResourceDescriptor resDescriptor{};
            resDescriptor.type = ShaderManager::ResourceType::eTexture2D;
            resDescriptor.imageView = view;
            mResUpdate[i][name] = resDescriptor;
        }
    }
};

} // namespace nevk
