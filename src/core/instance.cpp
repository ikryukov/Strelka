#include "core/instance.h"

Instance::Instance(const std::string& applicationName,
                   const std::vector<const char*>& requiredExtensions,
                   uint32_t apiVersion)
{
    //=======================================================
    // Optional:

    // Set additional info
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = apiVersion;

    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

    appInfo.pApplicationName = applicationName.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);

    // Output available extensions
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());
    std::cout << extensionCount << " available extensions:" << std::endl;;
    for (const auto& extension : availableExtensions)
        std::cout << '\t' << extension.extensionName << std::endl;

    // Output required extensions
    std::cout << requiredExtensions.size() << " required extnesions:" << std::endl;
    for (const auto& required : requiredExtensions)
        std::cout << '\t' << required << std::endl;

    //=======================================================
    // Necessary:

    // Set instance info
    VkInstanceCreateInfo instanceInfo{};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;

    instanceInfo.enabledExtensionCount = requiredExtensions.size();
    instanceInfo.ppEnabledExtensionNames = requiredExtensions.data();

    instanceInfo.enabledLayerCount = 0;

    // Create instance
    if (vkCreateInstance(&instanceInfo, nullptr, &this->handle) != VK_SUCCESS)
        throw std::runtime_error("Failed to create instance!");

}

Instance::~Instance() { 
    if (this->handle != VK_NULL_HANDLE)
        vkDestroyInstance(this->handle, nullptr);
}


VkInstance Instance::getHandle() { return this->handle; }