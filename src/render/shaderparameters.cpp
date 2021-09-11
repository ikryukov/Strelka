#include "shaderparameters.h"


namespace nevk
{
template <typename T>
NeVkResult ShaderParameters<T>::createUniformBuffers()
{
    VkDeviceSize bufferSize = sizeof(T) * MAX_FRAMES_IN_FLIGHT;
    mConstantBuffer = mResManager->createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (!mConstantBuffer)
    {
        return NeVkResult::eOutOfMemory;
    }
    return NeVkResult::eOk;
}

template <typename T>
NeVkResult ShaderParameters<T>::create(SharedContext& ctx, const uint32_t shaderId)
{

    return NeVkResult();
}

} // namespace nevk
