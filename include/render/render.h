#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_SILENT_WARNINGS
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

#include <array>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <optional>
#include <stdexcept>
#include <vector>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
const int MAX_FRAMES_IN_FLIGHT = 3;

const uint32_t SHADOW_MAP_WIDTH = 1024;
const uint32_t SHADOW_MAP_HEIGHT = 1024;

// const std::string MODEL_PATH = "misc/CornellBox-Sphere.obj";
// const std::string MTL_PATH = "misc/";

// const std::string MODEL_PATH = "misc/MetalRoughSpheresNoTextures/glTF/MetalRoughSpheresNoTextures.gltf";
const std::string MODEL_PATH = "misc/Cube/Cube.gltf";
// const std::string MODEL_PATH = "misc/skin/RiggedSimple.gltf";

// const std::string MODEL_PATH = "misc/pica_pica_-_mini_diorama_01/scene.gltf";
const std::string MTL_PATH = "misc/";

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
#ifdef __APPLE__
    ,
    "VK_KHR_portability_subset"
#endif
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif



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

    void initWindow();
    void initVulkan();
    void cleanup();

private:
    GLFWwindow* mWindow;

    VkInstance mInstance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR mSurface;

    VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
    VkDevice mDevice;

    VkQueue mGraphicsQueue;
    VkQueue mPresentQueue;

    VkSwapchainKHR mSwapChain;
    std::vector<VkImage> mSwapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

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
    nevk::RenderPass mPbrPass;
    nevk::ModelLoader* modelLoader;
    nevk::ComputePass mComputePass;
    nevk::DepthPass mDepthPass;

    uint32_t mIndicesCount = 0;
    VkBuffer mVertexBuffer;
    VkDeviceMemory mVertexBufferMemory;
    VkBuffer mMaterialBuffer;
    VkDeviceMemory mMaterialBufferMemory;
    VkBuffer mIndexBuffer;
    VkDeviceMemory mIndexBufferMemory;

    VkDescriptorPool mDescriptorPool;

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

    bool isPBR = true;

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

    static void handleMouseMoveCallback(GLFWwindow* window, double xpos, double ypos);

    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

    void mainLoop();

    void cleanupSwapChain();

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

    bool hasStencilComponent(VkFormat format);

    void loadModel(nevk::ModelLoader& testmodel);

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

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, [[maybe_unused]] void* pUserData)
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
    VkPhysicalDevice getPhysicalDevice()
    {
        return mPhysicalDevice;
    }
    QueueFamilyIndices getQueueFamilies(VkPhysicalDevice mdevice)
    {
        return findQueueFamilies(mdevice);
    }
    VkDescriptorPool getDescriptorPool()
    {
        return mDescriptorPool;
    }
    VkDevice getDevice()
    {
        return mDevice;
    }
    VkInstance getInstance()
    {
        return mInstance;
    }
    VkQueue getGraphicsQueue()
    {
        return mGraphicsQueue;
    }
    VkFormat getSwapChainImageFormat()
    {
        return swapChainImageFormat;
    }
    GLFWwindow* getWindow()
    {
        return mWindow;
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
    size_t getCurrentFrameIndex()
    {
        return mCurrentFrame % MAX_FRAMES_IN_FLIGHT;
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
    nevk::Ui getUi()
    {
        return mUi;
    }
    void setUi(nevk::Ui _mUi)
    {
        mUi = _mUi;
    }
};
