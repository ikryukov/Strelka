#pragma once

#define GLM_FORCE_SILENT_WARNINGS
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "common.h"
#include "accumulation.h"
#include "bvh.h"
#include "debugview.h"
#include "gbuffer.h"
#include "gbufferpass.h"
#include "pathtracer.h"
#include "tonemap.h"
#include "upscalepass.h"

#include <resourcemanager/resourcemanager.h>
#include <scene/scene.h>
#include <shadermanager/ShaderManager.h>

#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <optional>
#include <stdexcept>
#include <vector>

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
    //VK_KHR_SWAPCHAIN_EXTENSION_NAME
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

class PtRender
{
public:

    void initVulkan();
    void cleanup();

    void drawFrame(const uint8_t* outPixels);

    SharedContext& getSharedContext()
    {
        return mSharedCtx;
    }

    void setScene(Scene* scene)
    {
        mScene = scene;
    }

private:

    VkInstance mInstance;
    VkDebugUtilsMessengerEXT debugMessenger;

    VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
    VkDevice mDevice;

    VkQueue mGraphicsQueue;

    ResourceManager* mResManager = nullptr;
    TextureManager* mTexManager = nullptr;

    BvhBuilder mBvhBuilder;

    SharedContext mSharedCtx;

    GbufferPass mGbufferPass;
    PathTracer* mPathTracer;
    Accumulation* mAccumulationPathTracer;
    Tonemap* mTonemap;
    UpscalePass* mUpscalePass;
    DebugView* mDebugView;

    Tonemapparam mToneParams;
    Upscalepassparam mUpscalePassParam;
    Debugviewparam mDebugParams;

    struct ViewData
    {
        // could be scaled
        uint32_t renderWidth; // = swapChainExtent.width * mRenderConfig.upscaleFactor;
        uint32_t renderHeight; // = swapChainExtent.height * mRenderConfig.upscaleFactor;
        // swapchain size
        uint32_t finalWidth; // = swapChainExtent.width;
        uint32_t finalHeight; // = swapChainExtent.height;
        GBuffer* gbuffer;
        Image* prevDepth;
        Image* textureTonemapImage;
        Image* textureUpscaleImage;
        Image* textureDebugViewImage;
        Image* mPathTracerImage;
        Image* mAccumulationPathTracerImage = nullptr;
        ResourceManager* mResManager = nullptr;
        uint32_t mPtIteration = 0;
        ~ViewData()
        {
            assert(mResManager);
            if (gbuffer)
            {
                delete gbuffer;
            }
            if (prevDepth)
            {
                mResManager->destroyImage(prevDepth);
            }
            if (textureTonemapImage)
            {
                mResManager->destroyImage(textureTonemapImage);
            }
            if (textureUpscaleImage)
            {
                mResManager->destroyImage(textureUpscaleImage);
            }
            if (textureDebugViewImage)
            {
                mResManager->destroyImage(textureDebugViewImage);
            }
            if (mPathTracerImage)
            {
                mResManager->destroyImage(mPathTracerImage);
            }
            if (mAccumulationPathTracerImage)
            {
                mResManager->destroyImage(mAccumulationPathTracerImage);
            }
        }
    };

    ViewData* mPrevView = nullptr;
    std::array<ViewData*, MAX_FRAMES_IN_FLIGHT> mView;
    DebugView::DebugImages mDebugImages{};

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

    std::array<bool, MAX_FRAMES_IN_FLIGHT> needImageViewUpdate = { false, false, false };

    size_t mFrameNumber = 0;
    int32_t mSamples = 1;
    double msPerFrame = 33.33; // fps counter

    bool framebufferResized = false;
    bool prevState = true;

    nevk::ShaderManager mShaderManager;
    nevk::Scene* mScene = nullptr;

    void setDescriptors(uint32_t imageIndex);

    void createInstance();

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    void setupDebugMessenger();

    void pickPhysicalDevice();

    void createLogicalDevice();

    ViewData* createView(uint32_t width, uint32_t height);
    GBuffer* createGbuffer(uint32_t width, uint32_t height);
    void createGbufferPass();

    void createCommandPool();

    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    VkFormat findDepthFormat();

    bool hasStencilComponent(VkFormat format);

    void createVertexBuffer(nevk::Scene& scene);
    void createMaterialBuffer(nevk::Scene& scene);
    void createLightsBuffer(nevk::Scene& scene);
    void createBvhBuffer(nevk::Scene& scene);
    void createIndexBuffer(nevk::Scene& scene);
    void createInstanceBuffer(nevk::Scene& scene);

    void createDescriptorPool();

    void recordBarrier(VkCommandBuffer& cmd, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccess, VkAccessFlags dstAccess, VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage, VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT);

    void createCommandBuffers();

    void createSyncObjects();

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
    FrameData* getFramesData()
    {
        return mFramesData;
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
};

} // namespace nevk
