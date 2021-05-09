#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "computepass.h"
#include "depthpass.h"
#include "renderpass.h"

#include <modelloader/modelloader.h>
#include <resourcemanager/resourcemanager.h>
#include <scene/scene.h>
#include <shadermanager/ShaderManager.h>
#include <ui/ui.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <optional>
#include <set>
#include <stb_image.h>
#include <stdexcept>
#include <unordered_map>
#include <vector>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
const int MAX_FRAMES_IN_FLIGHT = 3;

const uint32_t SHADOW_MAP_WIDTH = 512;
const uint32_t SHADOW_MAP_HEIGHT = 512;

const std::string MODEL_PATH = "misc/cube.obj";
const std::string MTL_PATH = "misc/";

// const std::string MODEL_PATH = "misc/CornellBox-Sphere.obj";
// const std::string MTL_PATH = "misc/";

// const std::string MODEL_PATH = "misc/San_Miguel/san-miguel-low-poly.obj";
// const std::string MTL_PATH = "misc/San_Miguel/";

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class Render
{
public:
    void run()
    {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    GLFWwindow* window;
    ImGui_ImplVulkan_InitInfo init_info{};

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    VkFormat mFrameBufferFormat;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    VkImage textureCompImage;
    VkDeviceMemory textureCompImageMemory;
    VkImageView textureCompImageView;

    VkImage shadowImage;
    VkDeviceMemory shadowImageMemory;
    VkImageView shadowImageView;

    nevk::ResourceManager* mResManager;
    nevk::TextureManager* mTexManager;

    nevk::RenderPass mPass;
    nevk::Model* model;
    nevk::ComputePass mComputePass;
    nevk::DepthPass mDepthPass;

    uint32_t indicesCount = 0;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer materialBuffer;
    VkDeviceMemory materialBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    VkDescriptorPool descriptorPool;

    struct FrameData
    {
        VkCommandBuffer cmdBuffer;
        VkCommandPool cmdPool;
        VkFence inFlightFence;
        VkFence imagesInFlight;
        VkSemaphore renderFinished;
        VkSemaphore imageAvailable;
    };
    FrameData mFramesData[MAX_FRAMES_IN_FLIGHT] = {};

    FrameData& getFrameData(uint32_t idx)
    {
        return mFramesData[idx % MAX_FRAMES_IN_FLIGHT];
    }

    size_t mCurrentFrame = 0;

    bool framebufferResized = false;

    nevk::Ui mUi;
    nevk::ShaderManager mShaderManager;
    nevk::Scene mScene;

    void initWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(WIDTH, HEIGHT, "NeVK", nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
        glfwSetKeyCallback(window, keyCallback);
        glfwSetMouseButtonCallback(window, mouseButtonCallback);
        glfwSetCursorPosCallback(window, handleMouseMoveCallback);
        glfwSetScrollCallback(window, scrollCallback);
    }

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        auto app = reinterpret_cast<Render*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
        nevk::Scene& mScene = app->getScene();
        mScene.updateCameraParams(width, height);
    }

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        auto app = reinterpret_cast<Render*>(glfwGetWindowUserPointer(window));
        nevk::Scene& scene = app->getScene();
        Camera& camera = scene.getCamera();

        const bool keyState = ((GLFW_REPEAT == action) || (GLFW_PRESS == action)) ? true : false;
        switch (key)

        {
        case GLFW_KEY_W: {
            camera.keys.forward = keyState;
            break;
        }
        case GLFW_KEY_S: {
            camera.keys.back = keyState;
            break;
        }
        case GLFW_KEY_A: {
            camera.keys.left = keyState;
            break;
        }
        case GLFW_KEY_D: {
            camera.keys.right = keyState;
            break;
        }
        case GLFW_KEY_Q: {
            camera.keys.up = keyState;
            break;
        }
        case GLFW_KEY_E: {
            camera.keys.down = keyState;
            break;
        }
        default:
            break;
        }
    }

    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        auto app = reinterpret_cast<Render*>(glfwGetWindowUserPointer(window));
        nevk::Scene& scene = app->getScene();
        Camera& camera = scene.getCamera();
        if (button == GLFW_MOUSE_BUTTON_RIGHT)
        {
            if (action == GLFW_PRESS)
            {
                camera.mouseButtons.right = true;
            }
            else if (action == GLFW_RELEASE)
            {
                camera.mouseButtons.right = false;
            }
        }
        else if (button == GLFW_MOUSE_BUTTON_LEFT)
        {
            if (action == GLFW_PRESS)
            {
                camera.mouseButtons.left = true;
            }
            else if (action == GLFW_RELEASE)
            {
                camera.mouseButtons.left = false;
            }
        }
    }

    static void handleMouseMoveCallback(GLFWwindow* window, double xpos, double ypos)
    {
        auto app = reinterpret_cast<Render*>(glfwGetWindowUserPointer(window));
        nevk::Scene& scene = app->getScene();
        Camera& camera = scene.getCamera();
        const float dx = camera.mousePos.x - xpos;
        const float dy = camera.mousePos.y - ypos;

        ImGuiIO& io = ImGui::GetIO();
        bool handled = io.WantCaptureMouse;
        if (handled)
        {
            camera.mousePos = glm::vec2((float)xpos, (float)ypos);
            return;
        }

        if (camera.mouseButtons.right)
        {
            camera.rotate(-dx, -dy);
        }
        if (camera.mouseButtons.left)
        {
            camera.translate(glm::float3(-0.0f, 0.0f, -dy * .005f * camera.movementSpeed));
        }
        if (camera.mouseButtons.middle)
        {
            camera.translate(glm::float3(-dx * 0.01f, -dy * 0.01f, 0.0f));
        }
        camera.mousePos = glm::float2((float)xpos, (float)ypos);
    }

    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
    {
        ImGuiIO& io = ImGui::GetIO();
        bool handled = io.WantCaptureMouse;
        if (handled)
        {
            return;
        }

        auto app = reinterpret_cast<Render*>(glfwGetWindowUserPointer(window));
        nevk::Scene& mScene = app->getScene();
        Camera& mCamera = mScene.getCamera();

        mCamera.translate(glm::vec3(0.0f, 0.0f,
                                    -yoffset * mCamera.movementSpeed));
    }

    void initVulkan();

    void mainLoop();

    void cleanupSwapChain();

    void cleanup();

    void recreateSwapChain();

    void createInstance();

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    void setupDebugMessenger();

    void createSurface();

    void pickPhysicalDevice();

    void createLogicalDevice();

    void createSwapChain();

    void createImageViews();

    void createCommandPool();

    void createDepthResources();

    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    VkFormat findDepthFormat();

    bool hasStencilComponent(VkFormat format)
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    void loadModel(nevk::Model& testmodel)
    {
        testmodel.loadModel(MODEL_PATH, MTL_PATH, mScene);
        Camera& camera = mScene.getCamera();
        camera.type = Camera::CameraType::firstperson;

        camera.setPerspective(45.0f, (float)swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10000.0f);

        camera.rotationSpeed = 0.0025f;

        camera.movementSpeed = 1.0f;
        camera.setPosition({ 0.0f, 0.0f, 1.0f });
        camera.setRotation(glm::quat({ 1.0f, 0.0f, 0.0f, 0.0f }));
    }

    void createVertexBuffer();

    void createMaterialBuffer();

    void createIndexBuffer();

    void createDescriptorPool();

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void recordCommandBuffer(VkCommandBuffer& cmd, uint32_t imageIndex);

    void createCommandBuffers();

    void createSyncObjects();

    void drawFrame();

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

    bool isDeviceSuitable(VkPhysicalDevice device);

    bool checkDeviceExtensionSupport(VkPhysicalDevice device);

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    std::vector<const char*> getRequiredExtensions();

    bool checkValidationLayerSupport();

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
    {
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            std::cout << "Warning: " << pCallbackData->messageIdNumber << ":" << pCallbackData->pMessageIdName << ":" << pCallbackData->pMessage << std::endl;
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            std::cerr << "Error: " << pCallbackData->messageIdNumber << ":" << pCallbackData->pMessageIdName << ":" << pCallbackData->pMessage << std::endl;
        }
        else
        {
            std::cerr << "Validation: " << pCallbackData->messageIdNumber << ":" << pCallbackData->pMessageIdName << ":" << pCallbackData->pMessage << std::endl;
        }
        return VK_FALSE;
    }

public:
    void setWindow()
    {
        initWindow();
    }
    void setInstance()
    {
        createInstance();
    }
    void setDebugMessenger()
    {
        setupDebugMessenger();
    }
    void setSurface()
    {
        createSurface();
    }
    void setPhysicalDevice()
    {
        pickPhysicalDevice();
    }
    void setLogicalDevice()
    {
        createLogicalDevice();
    }
    void setSwapChain()
    {
        createSwapChain();
    }
    void setTexManager(nevk::TextureManager* _mTexManager)
    {
        mTexManager = _mTexManager;
    }
    void setResManager(nevk::ResourceManager* _mResManager)
    {
        mResManager = _mResManager;
    }
    void setImageViews()
    {
        createImageViews();
    }
    void setDescriptorPool()
    {
        createDescriptorPool();
    }
    void setCommandPool()
    {
        createCommandPool();
    }
    void setCommandBuffers()
    {
        createCommandBuffers();
    }
    void setSyncObjects()
    {
        createSyncObjects();
    }
    VkPhysicalDevice getPhysicalDevice()
    {
        return physicalDevice;
    }
    QueueFamilyIndices getQueueFamilies(VkPhysicalDevice mdevice)
    {
        return findQueueFamilies(mdevice);
    }
    VkDescriptorPool getDescriptorPool()
    {
        return descriptorPool;
    }
    VkDevice getDevice()
    {
        return device;
    }
    VkInstance getInstance()
    {
        return instance;
    }
    VkQueue getGraphicsQueue()
    {
        return graphicsQueue;
    }
    VkFormat getSwapChainImageFormat()
    {
        return swapChainImageFormat;
    }
    GLFWwindow* getWindow()
    {
        return window;
    }
    FrameData* getFramesData()
    {
        return mFramesData;
    }
    VkExtent2D getSwapChainExtent()
    {
        return swapChainExtent;
    }
    std::vector<VkImageView>& getSwapChainImageViews()
    {
        return swapChainImageViews;
    }
    FrameData& getCurrentFrameData()
    {
        return mFramesData[mCurrentFrame % MAX_FRAMES_IN_FLIGHT];
    }
    nevk::TextureManager* getTexManager()
    {
        return mTexManager;
    }
    nevk::ResourceManager* getResManager()
    {
        return mResManager;
    }
    nevk::Scene& getScene()
    {
        return mScene;
    }
    void setDepthResources()
    {
        createDepthResources();
    }
};
