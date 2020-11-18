#include "core/device.h"

//========================================================================================================

Device::Device(Instance* instance) {
    pickPhysicalDevice(instance->getHandle());
    createLogicalDevice();
}

Device::~Device() {
    if (this->GPU_interface != VK_NULL_HANDLE)
        vkDestroyDevice(this->GPU_interface, nullptr);
}

//========================================================================================================
// Physical Device (GPU)

void Device::pickPhysicalDevice(VkInstance instance_handle) {

    // Find the count of GPUs
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance_handle, &deviceCount, nullptr);
    if (deviceCount == 0)
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");

    // Find all the GPUs with info (properties and features)
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance_handle, &deviceCount, devices.data());
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
}

//========================================================================================================
// Logical Device (Interface)

void Device::createLogicalDevice() {
    QueueFamilyIndices indices = findQueueFamilies(this->GPU);

    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
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

    vkGetDeviceQueue(this->GPU_interface, indices.graphicsFamily.value(), 0, &(this->graphicsQueue));
}

//========================================================================================================
// GPU capabilities

Device::QueueFamilyIndices Device::findQueueFamilies(VkPhysicalDevice device) {

    // Find queue families
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    // Find queue families index with certain capability
    // We need to find at least one queue family that supports VK_QUEUE_GRAPHICS_BIT
    QueueFamilyIndices indices;
    uint32_t graphicsFamilyIndex = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphicsFamily = graphicsFamilyIndex;
        if (indices.graphicsFamily.has_value())
            break;
        graphicsFamilyIndex++;
    }

    return indices;
}

bool Device::isDeviceSuitable(VkPhysicalDevice GPU) {
    QueueFamilyIndices indices = findQueueFamilies(GPU);
    return indices.isComplete();
}