#include "core/instance.h"

Instance::Instance(std::string applicationName, std::vector<const char*>& requiredExtensions, uint32_t apiVersion)
    : handle{ VK_NULL_HANDLE }
{
    //=======================================================
    // Optional:

    // Set application info
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = apiVersion;

    appInfo.pEngineName = "NeVKengine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

    appInfo.pApplicationName = applicationName.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);

    //=======================================================
    // Necessary:

    // Find available extensions
    uint32_t availableExtensionsCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionsCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(availableExtensionsCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionsCount, availableExtensions.data());

    // Set instance info
    VkInstanceCreateInfo instanceInfo{};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;

    instanceInfo.enabledExtensionCount = (uint32_t)requiredExtensions.size();
    instanceInfo.ppEnabledExtensionNames = requiredExtensions.data();

    // Create instance
    if (vkCreateInstance(&instanceInfo, nullptr, &(this->handle)) != VK_SUCCESS)
        throw std::runtime_error("Failed to create instance!");
}

Instance::~Instance()
{
    if (this->handle != VK_NULL_HANDLE)
        vkDestroyInstance(this->handle, nullptr);
}

VkInstance Instance::getHandle()
{
    return this->handle;
}
