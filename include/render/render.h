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
#include "accumulation.h"
#include "bvh.h"
#include "common.h"
#include "debugview.h"
#include "depthpass.h"
#include "gbuffer.h"
#include "gbufferpass.h"
#include "ltcpass.h"
#include "bilateralfilter.h"
#include "renderpass.h"
#include "rtshadowpass.h"
#include "tonemap.h"

#include <modelloader/modelloader.h>
#include <resourcemanager/resourcemanager.h>
#include <scene/scene.h>
#include <shadermanager/ShaderManager.h>
#include <ui/ui.h>

#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <optional>
#include <stdexcept>
#include <vector>

const uint32_t SHADOW_MAP_WIDTH = 1024;
const uint32_t SHADOW_MAP_HEIGHT = 1024;

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
const bool enableValidationLayers = true; // Enable validation in release
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

namespace nevk
{

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

    std::string MODEL_PATH;
    uint32_t WIDTH;
    uint32_t HEIGHT;

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

    ResourceManager* mResManager = nullptr;
    TextureManager* mTexManager = nullptr;

    BvhBuilder mBvhBuilder;

    GbufferPass mGbufferPass;
    ModelLoader* modelLoader = nullptr;
    //disable depth prepass
    //DepthPass mDepthPass;

    SharedContext mSharedCtx;
    RtShadowPass* mRtShadow;
    Accumulation* mAccumulation;
    Tonemap* mTonemap;
    DebugView* mDebugView;
    LtcPass* mLtcPass;
    BilateralFilter* mBilateralFilter;
    Tonemapparam mToneParams;
    Debugviewparam mDebugParams;

    struct ViewData
    {
        uint32_t width;
        uint32_t height;
        GBuffer* gbuffer;
        Image* textureTonemapImage;
        Image* textureDebugViewImage;
        Image* mRtShadowImage;
        Image* mLtcOutputImage;
        Image* mBilateralOutputImage;
        Image* mAccumulationImages[2] = { nullptr, nullptr };
        ResourceManager* mResManager = nullptr;
        ~ViewData()
        {
            assert(mResManager);
            if (gbuffer)
            {
                delete gbuffer;
            }
            if (textureTonemapImage)
            {
                mResManager->destroyImage(textureTonemapImage);
            }
            if (textureDebugViewImage)
            {
                mResManager->destroyImage(textureDebugViewImage);
            }
            if (mRtShadowImage)
            {
                mResManager->destroyImage(mRtShadowImage);
            }
            if (mLtcOutputImage)
            {
                mResManager->destroyImage(mLtcOutputImage);
            }
            if (mBilateralOutputImage)
            {
                mResManager->destroyImage(mBilateralOutputImage);
            }
            for (uint32_t i = 0; i < 2; ++i)
            {
                if (mAccumulationImages[i])
                {
                    mResManager->destroyImage(mAccumulationImages[i]);
                }
            }
        }
    };

    ViewData* mView = nullptr;


    struct SceneRenderData
    {
        float animationTime = 0;
        uint32_t cameraIndex = 0;
        uint32_t mIndicesCount = 0;
        uint32_t mInstanceCount = 0;
        Buffer* mVertexBuffer = nullptr;
        Buffer* mMaterialBuffer = nullptr;
        Buffer* mIndexBuffer = nullptr;
        Buffer* mInstanceBuffer = nullptr;
        Buffer* mLightsBuffer = nullptr;
        Buffer* mBvhNodeBuffer = nullptr;
        Buffer* mBvhTriangleBuffer = nullptr;

        ResourceManager* mResManager = nullptr;
        explicit SceneRenderData(ResourceManager* resManager)
        {
            mResManager = resManager;
        }
        ~SceneRenderData()
        {
            assert(mResManager);
            if (mVertexBuffer)
            {
                mResManager->destroyBuffer(mVertexBuffer);
            }
            if (mIndexBuffer)
            {
                mResManager->destroyBuffer(mIndexBuffer);
            }
            if (mMaterialBuffer)
            {
                mResManager->destroyBuffer(mMaterialBuffer);
            }
            if (mInstanceBuffer)
            {
                mResManager->destroyBuffer(mInstanceBuffer);
            }
            if (mLightsBuffer)
            {
                mResManager->destroyBuffer(mLightsBuffer);
            }
            if (mBvhNodeBuffer)
            {
                mResManager->destroyBuffer(mBvhNodeBuffer);
            }
            if (mBvhTriangleBuffer)
            {
                mResManager->destroyBuffer(mBvhTriangleBuffer);
            }
        }
    };

    SceneRenderData* mCurrentSceneRenderData = nullptr;
    SceneRenderData* mDefaultSceneRenderData = nullptr;

    static constexpr size_t MAX_UPLOAD_SIZE = 1 << 24; // 16mb
    Buffer* mUploadBuffer[MAX_FRAMES_IN_FLIGHT] = { nullptr, nullptr, nullptr };
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

    size_t mFrameNumber = 0;

    // fps counter
    double msPerFrame = 33.33;

    bool framebufferResized = false;

    nevk::Ui mUi;
    nevk::ShaderManager mShaderManager;
    nevk::Scene* mScene = nullptr;
    nevk::Scene* mDefaultScene = nullptr;

    void loadScene(const std::string& modelPath);

    void createDefaultScene();

    void setDescriptors();

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

    static void handleMouseMoveCallback(GLFWwindow* window, double xpos, double ypos);

    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

    void mainLoop();

    double fpsCounter(double frameTime);

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

    ViewData* createView(uint32_t width, uint32_t height);
    GBuffer* createGbuffer(uint32_t width, uint32_t height);
    void createGbufferPass();

    void createCommandPool();

    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    VkFormat findDepthFormat();

    bool hasStencilComponent(VkFormat format);

    void setCamera();

    void createVertexBuffer(nevk::Scene& scene);
    void createMaterialBuffer(nevk::Scene& scene);
    void createLightsBuffer(nevk::Scene& scene);
    void createBvhBuffer(nevk::Scene& scene);
    void createIndexBuffer(nevk::Scene& scene);
    void createInstanceBuffer(nevk::Scene& scene);

    void createDescriptorPool();

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void recordBarrier(VkCommandBuffer& cmd, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccess, VkAccessFlags dstAccess, VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage, VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT);

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
        return mFrameNumber % MAX_FRAMES_IN_FLIGHT;
    }
    FrameData& getCurrentFrameData()
    {
        return mFramesData[mFrameNumber % MAX_FRAMES_IN_FLIGHT];
    }

    nevk::ResourceManager* getResManager()
    {
        return mResManager;
    }

    nevk::Scene* getScene()
    {
        return mScene;
    }

    uint32_t getActiveCameraIndex()
    {
        return mCurrentSceneRenderData->cameraIndex;
    }

    nevk::TextureManager* getTexManager()
    {
        return mTexManager;
    }

    SceneRenderData* getSceneData()
    {
        return mCurrentSceneRenderData;
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

} // namespace nevk
