// Vulkan
#include <vulkan/vulkan.h>

// GLFW (OpenGL Framework)
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// GLSL (OpenGL Shader Language)
    // Vertex shader - it is about calculating the shape of the figure
    // Fragment shader - it is about calculating the color of the figure
#include "shaders.h"

// Common includes
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>

class HelloTriangleApplication {
public:
    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    GLFWwindow* window{};
    VkInstance instance{};
    VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};

    void initWindow() {

        // GLFW: Initialize and configure
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        // Monitor
        GLFWmonitor* monitor = nullptr; // use glfwGetPrimaryMonitor() for full screen

        // Window
        window = glfwCreateWindow(WIDTH, HEIGHT, "MyOpenGL", monitor, nullptr);
        if (window == nullptr) {
            glfwTerminate();
            throw std::runtime_error("Failed to create GLFW window");
        }
    }

    void createInstance() {

        //=======================================================
        // Optional:

        // Set additional info
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pEngineName = "No Engine";
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        // Check available extensions
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
        std::cout << extensionCount << " available extensions:\n";
        for (const auto& extension: extensions)
            std::cout << '\t' << extension.extensionName << std::endl;;

        //=======================================================
        // Necessary:

        // GLFW extensions are required for instance
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::cout << "GLFW Extenstions are required:" << std::endl;
        for (uint16_t i = 0; i < glfwExtensionCount; i++)
            std::cout << '\t' << glfwExtensions[i] << std::endl;

        // Creation info for instance
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;
        createInfo.enabledLayerCount = 0;

        // Create instance
        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
            throw std::runtime_error("Failed to create instance!");
    }

    void pickPhysicalDevice() {

        // Find the count of GPUs
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        if (deviceCount == 0)
            throw std::runtime_error("Failed to find GPUs with Vulkan support!");

        // Find all the GPUs with info (properties and features)
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
        std::cout << "GPUs are found: " << std::endl;
        for (const auto& device: devices) {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(device, &properties);
            std::cout << '\t' << properties.deviceName << std::endl;
        }

        // Choose the GPU
        physicalDevice = devices[0];
    }

    void initVulkan() {
        createInstance();
        pickPhysicalDevice();
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents(); // Check keyboard and mouse events
        }
    }

    void cleanup() {
        vkDestroyInstance(instance, nullptr);
        glfwDestroyWindow(window);
        glfwTerminate();
    }

};

int main()
{
    try {
        HelloTriangleApplication app;
        app.run();
    }
    catch (const std::exception& error) {
        std::cerr << error.what() << std::endl;
        return EXIT_FAILURE;
    }
    return 0;
}
