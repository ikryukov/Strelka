#include "core/device.h"

Device::Device(VkInstance instance, VkSurfaceKHR surface)
    : instance{ instance },
      surface{ surface },
      GPU{ VK_NULL_HANDLE },
      GPU_interface{ VK_NULL_HANDLE },
      GPU_properties{ VK_NULL_HANDLE },
      GPU_features{ VK_NULL_HANDLE },
      graphicsQueue{ VK_NULL_HANDLE }
{
    pickPhysicalDevice();
    createLogicalDevice();
}

Device::~Device()
{
    if (this->GPU_interface != VK_NULL_HANDLE)
        vkDestroyDevice(this->GPU_interface, nullptr);
}

//========================================================================================================
// Public functions

VkDevice Device::getInterface()
{
    return this->GPU_interface;
}

//========================================================================================================
// GPU capabilities

// This structure contains indices of current queue families with certain capability
// This is necessary to define variables without value at all using std::optional
struct QueueFamiliesIndices
{
    std::optional<uint32_t> graphicsFamilyIndex; // queue family with this index support graphics queue
    std::optional<uint32_t> presentFamilyIndex; // queue family with this index support presentation queue

    bool isComplete()
    {
        return graphicsFamilyIndex.has_value() && presentFamilyIndex.has_value();
    }
};

// Find queue family with certain capabilities
QueueFamiliesIndices getQueueFamiliesIndices(VkPhysicalDevice GPU, VkSurfaceKHR surface)
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
        // Check graphics support
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphicsFamilyIndex = queueFamilyIndex;

        // Check presentation support
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(GPU, queueFamilyIndex, surface, &presentSupport);
        if (presentSupport)
            indices.presentFamilyIndex = queueFamilyIndex;

        if (indices.isComplete())
            break;

        queueFamilyIndex++;
    }
    return indices;
}

bool isDeviceSuitable(VkPhysicalDevice GPU, VkSurfaceKHR surface)
{
    return getQueueFamiliesIndices(GPU, surface).isComplete();
}

//========================================================================================================
// Private functions

void Device::pickPhysicalDevice()
{
    // Find the count of GPUs
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(this->instance, &deviceCount, nullptr);
    if (deviceCount == 0)
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");

    // Find all the GPUs with info (properties and features)
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(this->instance, &deviceCount, devices.data());
    std::cout << "GPUs are found: " << std::endl;
    for (const auto& device : devices)
    {

        // Choose the GPU
        if (isDeviceSuitable(device, surface))
        {
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

void Device::createLogicalDevice()
{
    //=======================================================
    // Set information for each queue

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    QueueFamiliesIndices indices = getQueueFamiliesIndices(this->GPU, surface);
    std::set<uint32_t> queueFamiliesIndices = { indices.graphicsFamilyIndex.value(),
                                                indices.presentFamilyIndex.value() };

    float queuePriority = 1.0f;
    for (uint32_t queueFamilyIndex : queueFamiliesIndices)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

        queueCreateInfo.queueCount = 1;
        queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        queueCreateInfos.push_back(queueCreateInfo);
    }

    //=======================================================
    // Create logical device

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &(this->GPU_features);
    createInfo.enabledExtensionCount = 0;
    createInfo.enabledLayerCount = 0;

    if (vkCreateDevice(this->GPU, &createInfo, nullptr, &(this->GPU_interface)) != VK_SUCCESS)
        throw std::runtime_error("Failed to create logical device!");

    //=======================================================
    // Get the buffers

    vkGetDeviceQueue(this->GPU_interface, indices.graphicsFamilyIndex.value(), 0, &(this->graphicsQueue));
    vkGetDeviceQueue(this->GPU_interface, indices.presentFamilyIndex.value(), 0, &(this->presentQueue));
}
