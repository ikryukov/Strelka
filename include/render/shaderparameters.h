#pragma once
#include "common.h"

#include <array>

namespace nevk
{
template <typename T>
class ShaderParameters
{
protected:
    std::array<T, MAX_FRAMES_IN_FLIGHT> mConstants;
    Buffer* mConstantBuffer = nullptr;

    VkDescriptorSetLayout mDescriptorSetLayout = VK_NULL_HANDLE;
    std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> mDescriptorSets;

    std::array<bool, MAX_FRAMES_IN_FLIGHT> needDesciptorSetUpdate = { false, false, false };
    std::array<bool, MAX_FRAMES_IN_FLIGHT> needConstantsUpdate = { false, false, false };


    NeVkResult createUniformBuffers();


    void createDescriptorSetLayout();
    void createDescriptorSets(VkDescriptorPool& descriptorPool);
    void updateDescriptorSet(uint32_t descIndex);
    void updateDescriptorSets();

public:

    VkDescriptorSetLayout getDescriptorSetLayout();
    VkDescriptorSet getDescriptorSet(const uint32_t index);


    NeVkResult create(SharedContext& ctx, const uint32_t shaderId);
};

} // namespace nevk
