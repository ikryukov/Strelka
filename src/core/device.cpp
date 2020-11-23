#include "core/device.h"

Device::Device(Instance* instance):
    GPU{ VK_NULL_HANDLE },
    GPU_interface{ VK_NULL_HANDLE },
    GPU_properties{ VK_NULL_HANDLE },
    GPU_features{ VK_NULL_HANDLE },
    graphicsQueue{ VK_NULL_HANDLE }
{
    pickPhysicalDevice(instance);
    createLogicalDevice();
}

Device::~Device() {
    if (this->GPU_interface != VK_NULL_HANDLE)
        vkDestroyDevice(this->GPU_interface, nullptr);
}

//========================================================================================================
// Public functions

VkDevice Device::getInterface() {
    return this->GPU_interface;
}

//========================================================================================================
// GPU capabilities

// This structure contains indices of current queue families with certain capability
// This is necessary to define variables without value at all using std::optional
struct QueueFamiliesIndices {
    std::optional<uint32_t> graphicsFamilyIndex; // queue family with this index support graphic queue
    // ...

    bool isComplete() {
        return graphicsFamilyIndex.has_value();
    }
};

// Find queue family with certain capabilities
QueueFamiliesIndices getQueueFamiliesIndices(VkPhysicalDevice GPU)
{
    // Find all queue families
    uint32_t queueFamiliesCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(GPU, &queueFamiliesCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamiliesCount);
    vkGetPhysicalDeviceQueueFamilyProperties(GPU, &queueFamiliesCount, queueFamilies.data());

    // Find current families with available capabilities
    QueueFamiliesIndices indices;
    uint32_t queueFamilyIndex = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        // Check graphics capability
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphicsFamilyIndex = queueFamilyIndex;

        if (indices.isComplete())
            break;

        queueFamilyIndex++;
    }
    return indices;
}

bool isDeviceSuitable(VkPhysicalDevice GPU) {
    return getQueueFamiliesIndices(GPU).isComplete();
}

//========================================================================================================
// Private functions

void Device::pickPhysicalDevice(Instance* instance) {

    // Find the count of GPUs
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance->getHandle(), &deviceCount, nullptr);
    if (deviceCount == 0)
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");

    // Find all the GPUs with info (properties and features)
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance->getHandle(), &deviceCount, devices.data());
    std::cout << "GPUs are found: " << std::endl;
    for (const auto& device : devices) {

        // Choose the GPU
        if (isDeviceSuitable(device)) {
            this->GPU = device;
            vkGetPhysicalDeviceProperties(this->GPU, &(this->GPU_properties));
            vkGetPhysicalDeviceFeatures(this->GPU, &(this->GPU_features));
        }

        // Optional output
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);
        std::cout << '\t' << properties.deviceName << std::endl;
    }

    if (this->GPU == VK_NULL_HANDLE)
        throw std::runtime_error("Failed to find a GPU with current queues!");

}

void Device::createLogicalDevice() {

    QueueFamiliesIndices indices = getQueueFamiliesIndices(this->GPU);

    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = indices.graphicsFamilyIndex.value();
    queueCreateInfo.queueCount = 1;

    float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;

    createInfo.pEnabledFeatures = &(this->GPU_features);

    createInfo.enabledExtensionCount = 0;
    createInfo.enabledLayerCount = 0;

    if (vkCreateDevice(this->GPU, &createInfo, nullptr, &(this->GPU_interface)) != VK_SUCCESS)
        throw std::runtime_error("Failed to create logical device!");

    // Get current queue
    vkGetDeviceQueue(this->GPU_interface, indices.graphicsFamilyIndex.value(), 0, &(this->graphicsQueue));
}
