#include "ptrender.h"

//#include "debugUtils.h"
#include "instanceconstants.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>

// profiler
//#include "Tracy.hpp"

namespace fs = std::filesystem;
const uint32_t MAX_LIGHT_COUNT = 100;

[[maybe_unused]] static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
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

[[maybe_unused]] static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

using namespace nevk;

void PtRender::initVulkan()
{
    createInstance();
    setupDebugMessenger();
    if (enableValidationLayers)
    {
        //nevk::debug::setupDebug(mInstance);
    }
    pickPhysicalDevice();
    createLogicalDevice();

    createDescriptorPool();
    createCommandPool();

    mResManager = new nevk::ResourceManager(mDevice, mPhysicalDevice, mInstance, getCurrentFrameData().cmdPool, mGraphicsQueue);
    mTexManager = new nevk::TextureManager(mDevice, mPhysicalDevice, mResManager);

    createCommandBuffers();
    createSyncObjects();

    mSharedCtx.mDescriptorPool = mDescriptorPool;
    mSharedCtx.mDevice = mDevice;
    mSharedCtx.mResManager = mResManager;
    mSharedCtx.mShaderManager = &mShaderManager;
    mSharedCtx.mTextureManager = mTexManager;

    mView[0] = createView(800, 600);

    mDebugView = new DebugView(mSharedCtx);
    mDebugView->initialize();

    mTonemap = new Tonemap(mSharedCtx);
    mTonemap->initialize();

    mUpscalePass = new UpscalePass(mSharedCtx);
    mUpscalePass->initialize();

    mMaterialManager = new MaterialManager();
    const char* paths[3] = { "./misc/test_data/mdl/", "./misc/test_data/mdl/resources/",
                             "./misc/vespa" };
    bool res = mMaterialManager->addMdlSearchPath(paths, 3);
    if (!res)
    {
        // failed to load MDL
        return;
    }

    MaterialManager::Module* mdlModule = mMaterialManager->createModule("tutorials.mdl");
    MaterialManager::MaterialInstance* materialInst = mMaterialManager->createMaterialInstance(mdlModule, "example_material");

    std::vector<MaterialManager::CompiledMaterial*> materials;
    MaterialManager::CompiledMaterial* materialComp = mMaterialManager->compileMaterial(materialInst);
    materials.push_back(materialComp);

    const fs::path cwd = fs::current_path();
    std::ifstream pt(cwd.string() + "/shaders/pathtracerMdl.hlsl");
    std::stringstream ptcode;
    ptcode << pt.rdbuf();

    assert(materials.size() != 0);
    const MaterialManager::TargetCode* mdlTargetCode = mMaterialManager->generateTargetCode(materials);
    const char* hlsl = mMaterialManager->getShaderCode(mdlTargetCode);
    
    mCurrentSceneRenderData = new SceneRenderData(mResManager);

    mCurrentSceneRenderData->mMaterialTargetCode = mdlTargetCode;
    
    std::string newPTCode = std::string(hlsl) + "\n" + ptcode.str();

    mPathTracer = new PathTracer(mSharedCtx, newPTCode);
    mPathTracer->initialize();

    mAccumulationPathTracer = new Accumulation(mSharedCtx);
    mAccumulationPathTracer->initialize();

    TextureManager::TextureSamplerDesc defSamplerDesc{ VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT };
    mTexManager->createTextureSampler(defSamplerDesc);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        mUploadBuffer[i] = mResManager->createBuffer(MAX_UPLOAD_SIZE, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }

    createGbufferPass();

    QueueFamilyIndices indicesFamily = findQueueFamilies(mPhysicalDevice);
}

void PtRender::cleanup()
{

    // mDepthPass.onDestroy();
    mGbufferPass.onDestroy();

    vkDestroyDescriptorPool(mDevice, mDescriptorPool, nullptr);

    mTexManager->textureDestroy();
    mTexManager->delTexturesFromQueue();

    delete mView[0];
    delete mView[1];
    delete mView[2];

    delete mTonemap;
    delete mUpscalePass;
    delete mDebugView;
    delete mPathTracer;
    delete mAccumulationPathTracer;

    delete mDefaultSceneRenderData;
    if (mCurrentSceneRenderData != mDefaultSceneRenderData)
    {
        delete mCurrentSceneRenderData;
    }

    for (FrameData& fd : mFramesData)
    {
        vkDestroySemaphore(mDevice, fd.renderFinished, nullptr);
        vkDestroySemaphore(mDevice, fd.imageAvailable, nullptr);
        vkDestroyFence(mDevice, fd.inFlightFence, nullptr);

        vkDestroyCommandPool(mDevice, fd.cmdPool, nullptr);
    }

    for (nevk::Buffer* buff : mUploadBuffer)
    {
        if (buff)
        {
            mResManager->destroyBuffer(buff);
        }
    }

    delete mResManager;

    vkDestroyDevice(mDevice, nullptr);

    if (enableValidationLayers)
    {
        DestroyDebugUtilsMessengerEXT(mInstance, debugMessenger, nullptr);
    }

    vkDestroyInstance(mInstance, nullptr);
}

void PtRender::createInstance()
{
    if (enableValidationLayers && !checkValidationLayerSupport())
    {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "NEVK";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "NoErrorVulkan Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &mInstance) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create instance!");
    }
}

void PtRender::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

void PtRender::setupDebugMessenger()
{
    if (!enableValidationLayers)
        return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

void PtRender::pickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(mInstance, &deviceCount, devices.data());

    for (const VkPhysicalDevice& device : devices)
    {
        if (isDeviceSuitable(device))
        {
            mPhysicalDevice = device;
            break;
        }
    }

    if (mPhysicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

void PtRender::createLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamilies(mPhysicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    {
        VkPhysicalDeviceDescriptorIndexingFeaturesEXT indexingFeatures{};
        indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
        indexingFeatures.pNext = nullptr;

        VkPhysicalDeviceFeatures2 deviceFeatures{};
        deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        deviceFeatures.pNext = &indexingFeatures;
        vkGetPhysicalDeviceFeatures2(mPhysicalDevice, &deviceFeatures);

        if (indexingFeatures.descriptorBindingPartiallyBound && indexingFeatures.runtimeDescriptorArray)
        {
            // all set to use unbound arrays of textures
        }
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.shaderSampledImageArrayDynamicIndexing = VK_TRUE;

    VkPhysicalDeviceDescriptorIndexingFeaturesEXT indexingFeatures{};
    indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
    indexingFeatures.pNext = nullptr;
    indexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
    indexingFeatures.runtimeDescriptorArray = VK_TRUE;
    indexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    indexingFeatures.shaderStorageImageArrayNonUniformIndexing = VK_TRUE;
    indexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;

    VkPhysicalDeviceRobustness2FeaturesEXT robustnessFeatures{};
    robustnessFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT;
    robustnessFeatures.nullDescriptor = VK_TRUE;
    robustnessFeatures.pNext = &indexingFeatures;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = &robustnessFeatures;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mDevice) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(mDevice, indices.graphicsFamily.value(), 0, &mGraphicsQueue);
}

PtRender::ViewData* PtRender::createView(uint32_t width, uint32_t height)
{
    assert(mResManager);
    ViewData* view = new ViewData();
    view->finalWidth = width;
    view->finalHeight = height;
    view->renderWidth = (uint32_t) (width * 1.0f); //mRenderConfig.upscaleFactor
    view->renderHeight = (uint32_t) (height * 1.0f); //mRenderConfig.upscaleFactor
    view->mResManager = mResManager;
    view->gbuffer = createGbuffer(view->renderWidth, view->renderHeight);
    view->prevDepth = mResManager->createImage(view->renderWidth, view->renderHeight, view->gbuffer->depthFormat, VK_IMAGE_TILING_OPTIMAL,
                                               VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Prev depth");
    mTexManager->transitionImageLayout(mResManager->getVkImage(view->prevDepth), view->gbuffer->depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    view->textureDebugViewImage = mResManager->createImage(width, height, VK_FORMAT_R16G16B16A16_SFLOAT,
                                                           VK_IMAGE_TILING_OPTIMAL,
                                                           VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "DebugView result");
    mTexManager->transitionImageLayout(mResManager->getVkImage(view->textureDebugViewImage), VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    view->textureTonemapImage = mResManager->createImage(view->renderWidth, view->renderHeight, VK_FORMAT_R32G32B32A32_SFLOAT,
                                                         VK_IMAGE_TILING_OPTIMAL,
                                                         VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Tonemap result");
    view->textureUpscaleImage = mResManager->createImage(width, height, VK_FORMAT_R32G32B32A32_SFLOAT,
                                                         VK_IMAGE_TILING_OPTIMAL,
                                                         VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Upscale Output");
    mTexManager->transitionImageLayout(mResManager->getVkImage(view->textureUpscaleImage), VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    
    view->mPathTracerImage = mResManager->createImage(view->renderWidth, view->renderHeight, VK_FORMAT_R32G32B32A32_SFLOAT,
                                                      VK_IMAGE_TILING_OPTIMAL,
                                                      VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Path Tracer Output");

    const std::string imageName = "Accumulation Image";
    view->mAccumulationPathTracerImage = mResManager->createImage(view->renderWidth, view->renderHeight, VK_FORMAT_R32G32B32A32_SFLOAT,
                                                                  VK_IMAGE_TILING_OPTIMAL,
                                                                  VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "PT accumulation");
    mTexManager->transitionImageLayout(mResManager->getVkImage(view->mAccumulationPathTracerImage), VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    return view;
}

GBuffer* PtRender::createGbuffer(uint32_t width, uint32_t height)
{
    assert(mResManager);
    GBuffer* res = new GBuffer();
    res->mResManager = mResManager;
    res->width = width;
    res->height = height;
    // Depth
    res->depthFormat = findDepthFormat();
    res->depth = mResManager->createImage(width, height, res->depthFormat, VK_IMAGE_TILING_OPTIMAL,
                                          VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "depth");
    // Normals
    res->normal = mResManager->createImage(width, height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                           VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "normal");
    // Tangent
    res->tangent = mResManager->createImage(width, height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "tangent");
    // wPos
    res->wPos = mResManager->createImage(width, height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                         VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "wPos");
    // UV
    res->uv = mResManager->createImage(width, height, VK_FORMAT_R16G16_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                       VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "UV");
    // InstId
    res->instId = mResManager->createImage(width, height, VK_FORMAT_R32_SINT, VK_IMAGE_TILING_OPTIMAL,
                                           VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "instId");
    // Motion
    res->motion = mResManager->createImage(width, height, VK_FORMAT_R32G32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                           VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Motion");
    // Debug
    res->debug = mResManager->createImage(width, height, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Debug");
    return res;
}

void PtRender::createGbufferPass()
{
    uint32_t vertId = mShaderManager.loadShader("shaders/gbuffer.hlsl", "vertexMain", nevk::ShaderManager::Stage::eVertex);
    uint32_t fragId = mShaderManager.loadShader("shaders/gbuffer.hlsl", "fragmentMain", nevk::ShaderManager::Stage::ePixel);
    const char* vertShaderCode = nullptr;
    uint32_t vertShaderCodeSize = 0;
    const char* fragShaderCode = nullptr;
    uint32_t fragShaderCodeSize = 0;
    mShaderManager.getShaderCode(vertId, vertShaderCode, vertShaderCodeSize);
    mShaderManager.getShaderCode(fragId, fragShaderCode, fragShaderCodeSize);

    mGbufferPass.setTextureSamplers(mTexManager->texSamplers);
    assert(mView[0]);
    mGbufferPass.init(mDevice, enableValidationLayers, vertShaderCode, vertShaderCodeSize, fragShaderCode, fragShaderCodeSize, mDescriptorPool, mResManager, mView[0]->gbuffer);
    mGbufferPass.createFrameBuffers(*mView[0]->gbuffer, 0);
}

void PtRender::createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(mPhysicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    for (FrameData& fd : mFramesData)
    {
        if (vkCreateCommandPool(mDevice, &poolInfo, nullptr, &fd.cmdPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create graphics command pool!");
        }
    }
}

VkFormat PtRender::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(mPhysicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
        {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

VkFormat PtRender::findDepthFormat()
{
    return findSupportedFormat(
        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

bool PtRender::hasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void PtRender::createVertexBuffer(nevk::Scene& scene)
{
    std::vector<nevk::Scene::Vertex>& sceneVertices = scene.getVertices();
    VkDeviceSize bufferSize = sizeof(nevk::Scene::Vertex) * sceneVertices.size();
    if (bufferSize == 0)
    {
        return;
    }
    Buffer* stagingBuffer = mResManager->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    void* stagingBufferMemory = mResManager->getMappedMemory(stagingBuffer);
    memcpy(stagingBufferMemory, sceneVertices.data(), (size_t)bufferSize);
    mCurrentSceneRenderData->mVertexBuffer = mResManager->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "VB");
    mResManager->copyBuffer(mResManager->getVkBuffer(stagingBuffer), mResManager->getVkBuffer(mCurrentSceneRenderData->mVertexBuffer), bufferSize);
    mResManager->destroyBuffer(stagingBuffer);
}

void PtRender::createMaterialBuffer(nevk::Scene& scene)
{
    std::vector<Material>& sceneMaterials = scene.getMaterials();

    VkDeviceSize bufferSize = sizeof(Material) * (sceneMaterials.size() + MAX_LIGHT_COUNT); // Reserve extra for lights material
    if (bufferSize == 0)
    {
        return;
    }
    Buffer* stagingBuffer = mResManager->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    void* stagingBufferMemory = mResManager->getMappedMemory(stagingBuffer);
    memcpy(stagingBufferMemory, sceneMaterials.data(), sceneMaterials.size() * sizeof(Material));
    mCurrentSceneRenderData->mMaterialBuffer = mResManager->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Materials");
    mResManager->copyBuffer(mResManager->getVkBuffer(stagingBuffer), mResManager->getVkBuffer(mCurrentSceneRenderData->mMaterialBuffer), bufferSize);
    mResManager->destroyBuffer(stagingBuffer);
}

void PtRender::createLightsBuffer(nevk::Scene& scene)
{
    std::vector<nevk::Scene::Light>& sceneLights = scene.getLights();

    VkDeviceSize bufferSize = sizeof(nevk::Scene::Light) * MAX_LIGHT_COUNT;
    mCurrentSceneRenderData->mLightsBuffer = mResManager->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Lights");
    
    if (!sceneLights.empty())
    {
        Buffer* stagingBuffer = mResManager->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        void* stagingBufferMemory = mResManager->getMappedMemory(stagingBuffer);
        memcpy(stagingBufferMemory, sceneLights.data(), sceneLights.size() * sizeof(nevk::Scene::Light));
        mResManager->copyBuffer(mResManager->getVkBuffer(stagingBuffer), mResManager->getVkBuffer(mCurrentSceneRenderData->mLightsBuffer), bufferSize);
        mResManager->destroyBuffer(stagingBuffer);
    }
}

void PtRender::createBvhBuffer(nevk::Scene& scene)
{
    const std::vector<Scene::Vertex>& vertices = scene.getVertices();
    const std::vector<uint32_t>& indices = scene.getIndices();
    const std::vector<Instance>& instances = scene.getInstances();
    const std::vector<Mesh>& meshes = scene.getMeshes();

    std::vector<BVHInputPosition> positions;
    positions.reserve(indices.size());
    uint32_t currInstId = 0;
    for (const Instance& currInstance : instances)
    {
        const uint32_t currentMeshId = currInstance.mMeshId;
        const Mesh& mesh = meshes[currentMeshId];
        const uint32_t indexOffset = mesh.mIndex;
        const uint32_t indexCount = mesh.mCount;
        assert(indexCount % 3 == 0);
        const uint32_t triangleCount = indexCount / 3;
        const glm::float4x4 m = currInstance.transform;

        for (uint32_t i = 0; i < triangleCount; ++i)
        {
            uint32_t i0 = indices[indexOffset + i * 3 + 0];
            uint32_t i1 = indices[indexOffset + i * 3 + 1];
            uint32_t i2 = indices[indexOffset + i * 3 + 2];

            glm::float3 v0 = m * glm::float4(vertices[i0].pos, 1.0);
            glm::float3 v1 = m * glm::float4(vertices[i1].pos, 1.0);
            glm::float3 v2 = m * glm::float4(vertices[i2].pos, 1.0);

            BVHInputPosition p0;
            p0.pos = v0;
            p0.instId = currInstId;
            p0.primId = i;
            positions.push_back(p0);

            BVHInputPosition p1;
            p1.pos = v1;
            p1.instId = currInstId;
            p1.primId = i;
            positions.push_back(p1);

            BVHInputPosition p2;
            p2.pos = v2;
            p2.instId = currInstId;
            p2.primId = i;
            positions.push_back(p2);
        }
        ++currInstId;
    }

    BVH sceneBvh = mBvhBuilder.build(positions);
    {
        VkDeviceSize bufferSize = sizeof(BVHNode) * sceneBvh.nodes.size();
        if (bufferSize == 0)
        {
            return;
        }
        Buffer* stagingBuffer = mResManager->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        void* stagingBufferMemory = mResManager->getMappedMemory(stagingBuffer);
        memcpy(stagingBufferMemory, sceneBvh.nodes.data(), (size_t)bufferSize);
        mCurrentSceneRenderData->mBvhNodeBuffer = mResManager->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "BVH");
        mResManager->copyBuffer(mResManager->getVkBuffer(stagingBuffer), mResManager->getVkBuffer(mCurrentSceneRenderData->mBvhNodeBuffer), bufferSize);
        mResManager->destroyBuffer(stagingBuffer);
    }
}

void PtRender::createIndexBuffer(nevk::Scene& scene)
{
    std::vector<uint32_t>& sceneIndices = scene.getIndices();
    mCurrentSceneRenderData->mIndicesCount = (uint32_t)sceneIndices.size();
    VkDeviceSize bufferSize = sizeof(uint32_t) * sceneIndices.size();
    if (bufferSize == 0)
    {
        return;
    }

    Buffer* stagingBuffer = mResManager->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    void* stagingBufferMemory = mResManager->getMappedMemory(stagingBuffer);
    memcpy(stagingBufferMemory, sceneIndices.data(), (size_t)bufferSize);
    mCurrentSceneRenderData->mIndexBuffer = mResManager->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "IB");
    mResManager->copyBuffer(mResManager->getVkBuffer(stagingBuffer), mResManager->getVkBuffer(mCurrentSceneRenderData->mIndexBuffer), bufferSize);
    mResManager->destroyBuffer(stagingBuffer);
}

void PtRender::createInstanceBuffer(nevk::Scene& scene)
{
    const std::vector<Mesh>& meshes = scene.getMeshes();
    const std::vector<nevk::Instance>& sceneInstances = scene.getInstances();
    mCurrentSceneRenderData->mInstanceCount = (uint32_t)sceneInstances.size();
    VkDeviceSize bufferSize = sizeof(InstanceConstants) * (sceneInstances.size() + MAX_LIGHT_COUNT); // Reserve some extra space for lights
    if (bufferSize == 0)
    {
        return;
    }
    std::vector<InstanceConstants> instanceConsts;
    instanceConsts.resize(sceneInstances.size());
    for (int i = 0; i < sceneInstances.size(); ++i)
    {
        instanceConsts[i].materialId = sceneInstances[i].mMaterialId;
        instanceConsts[i].objectToWorld = sceneInstances[i].transform;
        instanceConsts[i].worldToObject = glm::inverse(sceneInstances[i].transform);
        instanceConsts[i].normalMatrix = glm::inverse(glm::transpose(sceneInstances[i].transform));

        const uint32_t currentMeshId = sceneInstances[i].mMeshId;
        instanceConsts[i].indexOffset = meshes[currentMeshId].mIndex;
        instanceConsts[i].indexCount = meshes[currentMeshId].mCount;
    }

    Buffer* stagingBuffer = mResManager->createBuffer(sizeof(InstanceConstants) * sceneInstances.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    void* stagingBufferMemory = mResManager->getMappedMemory(stagingBuffer);
    memcpy(stagingBufferMemory, instanceConsts.data(), sizeof(InstanceConstants) * sceneInstances.size());
    mCurrentSceneRenderData->mInstanceBuffer = mResManager->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Instance consts");
    mResManager->copyBuffer(mResManager->getVkBuffer(stagingBuffer), mResManager->getVkBuffer(mCurrentSceneRenderData->mInstanceBuffer), sizeof(InstanceConstants) * sceneInstances.size());
    mResManager->destroyBuffer(stagingBuffer);
}

void nevk::PtRender::createMdlBuffers()
{
    const MaterialManager::TargetCode* code = mCurrentSceneRenderData->mMaterialTargetCode;
    mMaterialManager->getArgBufferData(code);

    const uint32_t argSize = mMaterialManager->getArgBufferSize(code);
    const uint32_t roSize = mMaterialManager->getReadOnlyBlockSize(code);
    const uint32_t infoSize = mMaterialManager->getResourceInfoSize(code);
    const uint32_t mdlMaterialSize = mMaterialManager->getMdlMaterialSize(code);

    VkDeviceSize stagingSize = std::max(std::max(roSize, mdlMaterialSize), std::max(argSize, infoSize));

    Buffer* stagingBuffer = mResManager->createBuffer(stagingSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, "Staging MDL");
    void* stagingBufferMemory = mResManager->getMappedMemory(stagingBuffer);

    auto createGpuBuffer = [&](Buffer*& dest, const uint8_t* src, uint32_t size, const char* name) {
        memcpy(stagingBufferMemory, src, size);
        dest = mResManager->createBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, name);
        mResManager->copyBuffer(mResManager->getVkBuffer(stagingBuffer), mResManager->getVkBuffer(dest), size);
    };

    createGpuBuffer(mCurrentSceneRenderData->mMdlArgBuffer, mMaterialManager->getArgBufferData(code), argSize, "MDL args");
    createGpuBuffer(mCurrentSceneRenderData->mMdlInfoBuffer, mMaterialManager->getResourceInfoData(code), infoSize, "MDL info");
    createGpuBuffer(mCurrentSceneRenderData->mMdlRoBuffer, mMaterialManager->getReadOnlyBlockData(code), roSize, "MDL read only");
    createGpuBuffer(mCurrentSceneRenderData->mMdlMaterialBuffer, mMaterialManager->getMdlMaterialData(code), mdlMaterialSize, "MDL mdl material");

    mResManager->destroyBuffer(stagingBuffer);
}

void PtRender::createDescriptorPool()
{
    VkDescriptorPoolSize pool_sizes[] = {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 10000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.poolSizeCount = sizeof(pool_sizes) / sizeof(VkDescriptorPoolSize);
    poolInfo.pPoolSizes = pool_sizes;
    poolInfo.maxSets = 1000 * poolInfo.poolSizeCount;

    if (vkCreateDescriptorPool(mDevice, &poolInfo, nullptr, &mDescriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void PtRender::recordBarrier(VkCommandBuffer& cmd, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccess, VkAccessFlags dstAccess, VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage, VkImageAspectFlags aspectMask)
{
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = aspectMask;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    barrier.srcAccessMask = srcAccess;
    barrier.dstAccessMask = dstAccess;

    vkCmdPipelineBarrier(
        cmd,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier);
}

void PtRender::createCommandBuffers()
{
    for (FrameData& fd : mFramesData)
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = fd.cmdPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(mDevice, &allocInfo, &fd.cmdBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }
}

void PtRender::createSyncObjects()
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        if (vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mFramesData[i].renderFinished) != VK_SUCCESS ||
            vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mFramesData[i].imageAvailable) != VK_SUCCESS ||
            vkCreateFence(mDevice, &fenceInfo, nullptr, &mFramesData[i].inFlightFence) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

void PtRender::setDescriptors(uint32_t imageIndex)
{
    ViewData* currView = mView[imageIndex];

    {
        mGbufferPass.setTextureImageView(mTexManager->textureImageView);
        mGbufferPass.setTextureSamplers(mTexManager->texSamplers);
        mGbufferPass.setMaterialBuffer(mResManager->getVkBuffer(mCurrentSceneRenderData->mMaterialBuffer));
        mGbufferPass.setInstanceBuffer(mResManager->getVkBuffer(mCurrentSceneRenderData->mInstanceBuffer));
    }

    {
        mAccumulationPathTracer->setInputTexture4(currView->mPathTracerImage);
        mAccumulationPathTracer->setMotionTexture(currView->gbuffer->motion);
        mAccumulationPathTracer->setPrevDepthTexture(currView->prevDepth);
        mAccumulationPathTracer->setWposTexture(currView->gbuffer->wPos);
        mAccumulationPathTracer->setCurrDepthTexture(currView->gbuffer->depth);
    }
    //{
    //    mDebugImages.normal = currView->gbuffer->normal;
    //    mDebugImages.debug = currView->gbuffer->debug;
    //    mDebugImages.motion = currView->gbuffer->motion;
    //    mDebugImages.pathTracer = currView->mPathTracerImage;

    //    mDebugView->setParams(mDebugParams);
    //    mDebugView->setInputTexture(mDebugImages);
    //    mDebugView->setOutputTexture(mResManager->getView(currView->textureDebugViewImage));
    //}
    {
        mTonemap->setParams(mToneParams);
        mTonemap->setInputTexture(mResManager->getView(currView->mPathTracerImage));
        mTonemap->setOutputTexture(mResManager->getView(currView->textureTonemapImage));
    }
    {
        mUpscalePass->setParams(mUpscalePassParam);
        mUpscalePass->setInputTexture(mResManager->getView(currView->mPathTracerImage));
        mUpscalePass->setOutputTexture(mResManager->getView(currView->textureUpscaleImage));
    }
}

void PtRender::drawFrame(const uint8_t* outPixels)
{
    assert(mScene);
    //ZoneScoped;
    
    FrameData& currFrame = getCurrentFrameData();
    const uint32_t imageIndex = 0;
    



    createVertexBuffer(*mScene);
    createIndexBuffer(*mScene);
    createInstanceBuffer(*mScene);
    createLightsBuffer(*mScene);
    createMaterialBuffer(*mScene);
    createMdlBuffers();
    createBvhBuffer(*mScene); // need to update descriptors after it

    ViewData* currView = mView[imageIndex];
    uint32_t renderWidth = currView->renderWidth;
    uint32_t renderHeight = currView->renderHeight;
    uint32_t finalWidth = currView->finalWidth;
    uint32_t finalHeight = currView->finalHeight;

    mScene->updateCamerasParams(renderWidth, renderHeight);

    Camera& cam = mScene->getCamera(getActiveCameraIndex());
    cam.updateViewMatrix();

    setDescriptors(imageIndex);
    

    mGbufferPass.onResize(currView->gbuffer, 0);
    mGbufferPass.updateUniformBuffer(imageIndex, *mScene, getActiveCameraIndex());

    PathTracerParam pathTracerParam{};
    pathTracerParam.dimension = glm::int2(renderWidth, renderHeight);
    pathTracerParam.frameNumber = (uint32_t)mFrameNumber;
    pathTracerParam.maxDepth = 2;
    pathTracerParam.debug = (uint32_t)(mScene->mDebugViewSettings == Scene::DebugView::ePTDebug);
    pathTracerParam.camPos = glm::float4(cam.getPosition(), 1.0f);
    pathTracerParam.viewToWorld = glm::inverse(cam.matrices.view);
    pathTracerParam.worldToView = cam.matrices.view; //
    //pathTracerParam.clipToView = glm::inverse(cam.matrices.perspective);
    pathTracerParam.clipToView = cam.matrices.invPerspective;
    pathTracerParam.viewToClip = cam.matrices.perspective; //
    pathTracerParam.len = (int) 0;
    pathTracerParam.numLights = (uint32_t) 1;
    pathTracerParam.invDimension.x = 1.0f / (float)renderWidth;
    pathTracerParam.invDimension.y = 1.0f / (float)renderHeight;
    mPathTracer->setParams(pathTracerParam);

    AccumulationParam accParam{};
    accParam.alpha = 0.1f;
    accParam.dimension = glm::int2(renderWidth, renderHeight);
    //glm::double4x4 persp = cam.prevMatrices.perspective;
    //accParam.prevClipToView = glm::inverse(persp);
    accParam.prevClipToView = cam.prevMatrices.invPerspective;
    accParam.prevViewToClip = cam.prevMatrices.perspective;
    accParam.prevWorldToView = cam.prevMatrices.view;
    glm::double4x4 view = cam.prevMatrices.view;
    accParam.prevViewToWorld = glm::inverse(view);
    // debug
    // accParam.clipToView = glm::inverse(cam.matrices.perspective);
    accParam.clipToView = cam.matrices.invPerspective;
    accParam.viewToWorld = glm::inverse(cam.matrices.view);
    accParam.isPt = 1;
    currView->mPtIteration = 0;
    accParam.iteration = currView->mPtIteration;
    mAccumulationPathTracer->setParams(accParam);

    VkCommandBuffer& cmd = getFrameData(imageIndex).cmdBuffer;
    vkResetCommandBuffer(cmd, 0);

    VkCommandBufferBeginInfo cmdBeginInfo = {};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.pNext = nullptr;
    cmdBeginInfo.pInheritanceInfo = nullptr;
    cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(cmd, &cmdBeginInfo);

    // upload data
    //{
    //    const std::vector<Mesh>& meshes = mScene->getMeshes();
    //    const std::vector<nevk::Instance>& sceneInstances = scene->getInstances();
    //    mCurrentSceneRenderData->mInstanceCount = (uint32_t)sceneInstances.size();
    //    Buffer* stagingBuffer = mUploadBuffer[imageIndex];
    //    void* stagingBufferMemory = mResManager->getMappedMemory(stagingBuffer);
    //    size_t stagingBufferOffset = 0;
    //    bool needBarrier = false;
    //    if (!sceneInstances.empty())
    //    {
    //        // This struct must match shader's version
    //        struct InstanceConstants
    //        {
    //            glm::float4x4 model;
    //            glm::float4x4 normalMatrix;
    //            int32_t materialId;
    //            int32_t indexOffset;
    //            int32_t indexCount;
    //            int32_t pad2;
    //        };
    //        std::vector<InstanceConstants> instanceConsts;
    //        instanceConsts.resize(sceneInstances.size());
    //        for (uint32_t i = 0; i < sceneInstances.size(); ++i)
    //        {
    //            instanceConsts[i].materialId = sceneInstances[i].mMaterialId;
    //            instanceConsts[i].model = sceneInstances[i].transform;
    //            instanceConsts[i].normalMatrix = glm::inverse(glm::transpose(sceneInstances[i].transform));

    //            const uint32_t currentMeshId = sceneInstances[i].mMeshId;
    //            instanceConsts[i].indexOffset = meshes[currentMeshId].mIndex;
    //            instanceConsts[i].indexCount = meshes[currentMeshId].mCount;
    //        }
    //        size_t bufferSize = sizeof(InstanceConstants) * sceneInstances.size();
    //        memcpy(stagingBufferMemory, instanceConsts.data(), bufferSize);

    //        VkBufferCopy copyRegion{};
    //        copyRegion.size = bufferSize;
    //        copyRegion.dstOffset = 0;
    //        copyRegion.srcOffset = stagingBufferOffset;
    //        vkCmdCopyBuffer(cmd, mResManager->getVkBuffer(stagingBuffer), mResManager->getVkBuffer(mCurrentSceneRenderData->mInstanceBuffer), 1, &copyRegion);

    //        stagingBufferOffset += bufferSize;
    //        needBarrier = true;
    //    }
    //    const std::vector<nevk::Scene::Light>& lights = scene->getLights();
    //    if (!lights.empty())
    //    {
    //        size_t bufferSize = sizeof(nevk::Scene::Light) * MAX_LIGHT_COUNT;
    //        memcpy((void*)((char*)stagingBufferMemory + stagingBufferOffset), lights.data(), bufferSize);

    //        VkBufferCopy copyRegion{};
    //        copyRegion.size = bufferSize;
    //        copyRegion.dstOffset = 0;
    //        copyRegion.srcOffset = stagingBufferOffset;
    //        vkCmdCopyBuffer(cmd, mResManager->getVkBuffer(stagingBuffer), mResManager->getVkBuffer(mCurrentSceneRenderData->mLightsBuffer), 1, &copyRegion);

    //        stagingBufferOffset += bufferSize;
    //        needBarrier = true;
    //    }

    //    const std::vector<Material>& materials = scene->getMaterials();
    //    if (!materials.empty())
    //    {
    //        size_t bufferSize = sizeof(Material) * MAX_LIGHT_COUNT;
    //        memcpy((void*)((char*)stagingBufferMemory + stagingBufferOffset), materials.data(), bufferSize);

    //        VkBufferCopy copyRegion{};
    //        copyRegion.size = bufferSize;
    //        copyRegion.dstOffset = 0;
    //        copyRegion.srcOffset = stagingBufferOffset;
    //        vkCmdCopyBuffer(cmd, mResManager->getVkBuffer(stagingBuffer), mResManager->getVkBuffer(mCurrentSceneRenderData->mMaterialBuffer), 1, &copyRegion);

    //        stagingBufferOffset += bufferSize;
    //        needBarrier = true;
    //    }

    //    if (needBarrier)
    //    {
    //        VkMemoryBarrier memoryBarrier = {};
    //        memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    //        memoryBarrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    //        memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;

    //        vkCmdPipelineBarrier(cmd,
    //                             VK_PIPELINE_STAGE_TRANSFER_BIT, // srcStageMask
    //                             VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, // dstStageMask
    //                             0,
    //                             1, // memoryBarrierCount
    //                             &memoryBarrier, // pMemoryBarriers
    //                             0, nullptr, 0, nullptr);
    //    }
    //}

    mGbufferPass.record(cmd, mResManager->getVkBuffer(mCurrentSceneRenderData->mVertexBuffer), mResManager->getVkBuffer(mCurrentSceneRenderData->mIndexBuffer), *mScene, renderWidth, renderHeight, imageIndex, getActiveCameraIndex());

    const GBuffer& gbuffer = *currView->gbuffer;

    // barriers
    {
        recordBarrier(cmd, mResManager->getVkImage(gbuffer.wPos), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        recordBarrier(cmd, mResManager->getVkImage(gbuffer.normal), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        recordBarrier(cmd, mResManager->getVkImage(gbuffer.tangent), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        recordBarrier(cmd, mResManager->getVkImage(gbuffer.uv), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        recordBarrier(cmd, mResManager->getVkImage(gbuffer.instId), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        recordBarrier(cmd, mResManager->getVkImage(gbuffer.motion), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        recordBarrier(cmd, mResManager->getVkImage(gbuffer.depth), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                      VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
        recordBarrier(cmd, mResManager->getVkImage(gbuffer.debug), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    }

    Image* finalPathTracerImage = mView[imageIndex]->mPathTracerImage;
    Image* finalImage = nullptr;

    // Path Tracer
    {
        {
            PathTracerDesc desc{};
            desc.result = mView[imageIndex]->mPathTracerImage;
            desc.gbuffer = mView[imageIndex]->gbuffer;
            desc.bvhNodes = mCurrentSceneRenderData->mBvhNodeBuffer;
            desc.vb = mCurrentSceneRenderData->mVertexBuffer;
            desc.ib = mCurrentSceneRenderData->mIndexBuffer;
            desc.instanceConst = mCurrentSceneRenderData->mInstanceBuffer;
            desc.lights = mCurrentSceneRenderData->mLightsBuffer;
            desc.materials = mCurrentSceneRenderData->mMaterialBuffer;
            desc.matSampler = mTexManager->mMdlSampler;
            desc.matTextures = mTexManager->textureImages;

            desc.mdl_argument_block = mCurrentSceneRenderData->mMdlArgBuffer;
            desc.mdl_ro_data_segment = mCurrentSceneRenderData->mMdlRoBuffer;
            desc.mdl_resource_infos = mCurrentSceneRenderData->mMdlInfoBuffer;
            desc.mdl_mdlMaterial = mCurrentSceneRenderData->mMdlMaterialBuffer;

            mPathTracer->setResources(desc);
        }
        recordBarrier(cmd, mResManager->getVkImage(mView[imageIndex]->mPathTracerImage), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
                      VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        mPathTracer->execute(cmd, renderWidth, renderHeight, imageIndex);
        recordBarrier(cmd, mResManager->getVkImage(mView[imageIndex]->mPathTracerImage), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                      VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

        finalPathTracerImage = mView[imageIndex]->mPathTracerImage;

        //if (0)
        //{
        //    // Accumulation pass
        //    Image* accHist = mPrevView->mAccumulationPathTracerImage;
        //    Image* accOut = mView[imageIndex]->mAccumulationPathTracerImage;

        //    mAccumulationPathTracer->setHistoryTexture4(accHist);
        //    mAccumulationPathTracer->setOutputTexture4(accOut);

        //    recordBarrier(cmd, mResManager->getVkImage(accOut), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL,
        //                  VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        //    mAccumulationPathTracer->execute(cmd, renderWidth, renderHeight, imageIndex);
        //    recordBarrier(cmd, mResManager->getVkImage(accOut),  VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        //                  VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

        //    finalPathTracerImage = accOut;
        //}
    }

    {
        Image* tmpImage = finalPathTracerImage;
  
        // Tonemap
        {
            recordBarrier(cmd, mResManager->getVkImage(mView[imageIndex]->textureTonemapImage), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
                          VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
           // recordBarrier(cmd, mResManager->getVkImage(tmpImage), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
             //             VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
            mToneParams.dimension.x = renderWidth;
            mToneParams.dimension.y = renderHeight;
            mTonemap->setParams(mToneParams);
            mTonemap->setInputTexture(mResManager->getView(tmpImage));
            mTonemap->execute(cmd, renderWidth, renderHeight, imageIndex);
            finalImage = mView[imageIndex]->textureTonemapImage;
        }

        // Upscale
        if (1)
        {
            recordBarrier(cmd, mResManager->getVkImage(finalImage), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                          VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
            mUpscalePassParam.dimension.x = finalWidth;
            mUpscalePassParam.dimension.y = finalHeight;
            mUpscalePassParam.invDimension.x =  1.0f / (float) finalWidth;
            mUpscalePassParam.invDimension.y = 1.0f / (float) finalHeight;
            mUpscalePass->setParams(mUpscalePassParam);
            mUpscalePass->setInputTexture(mResManager->getView(finalImage));
            mUpscalePass->execute(cmd, finalWidth, finalHeight, imageIndex);
            recordBarrier(cmd, mResManager->getVkImage(finalImage), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL,
                          VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
            finalImage = mView[imageIndex]->textureUpscaleImage;
        }
    }

    // Copy to swapchain image

    recordBarrier(cmd, mResManager->getVkImage(finalImage), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                  VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

    VkDeviceSize imageSize = finalWidth * finalHeight * 16;
    Buffer* stagingBuffer = mResManager->createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = finalWidth;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageSubresource.mipLevel = 0;
    region.imageOffset.x = 0;
    region.imageOffset.y = 0;
    region.imageOffset.z = 0;
    region.imageExtent.width = finalWidth;
    region.imageExtent.height = finalHeight;
    region.imageExtent.depth = 1;
    vkCmdCopyImageToBuffer(cmd, mResManager->getVkImage(finalImage), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, mResManager->getVkBuffer(stagingBuffer), 1, &region);

    //    VkOffset3D blitSize{};
    //    blitSize.x = finalWidth;
    //    blitSize.y = finalHeight;
    //    blitSize.z = 1;
    //    VkImageBlit imageBlitRegion{};
    //    imageBlitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    //    imageBlitRegion.srcSubresource.layerCount = 1;
    //    imageBlitRegion.srcOffsets[1] = blitSize;
    //    imageBlitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    //    imageBlitRegion.dstSubresource.layerCount = 1;
    //    imageBlitRegion.dstOffsets[1] = blitSize;
    //    vkCmdBlitImage(cmd, mResManager->getVkImage(finalImage), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, mSwapChainImages[imageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlitRegion, VK_FILTER_NEAREST);

    //    recordBarrier(cmd, mResManager->getVkImage(finalImage), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL,
    //                  VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    //    recordBarrier(cmd, mSwapChainImages[imageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    //                  VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    //}

    // copy current depth from gbuffer to prev gbuffer
    //{
    //    recordBarrier(cmd, mResManager->getVkImage(mView[imageIndex]->gbuffer->depth), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    //                  VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
    //    recordBarrier(cmd, mResManager->getVkImage(mView[imageIndex]->prevDepth), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    //                  VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
    //    VkImageCopy region{};
    //    region.extent.width = renderWidth;
    //    region.extent.height = renderHeight;
    //    region.extent.depth = 1;

    //    region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    //    region.srcSubresource.mipLevel = 0;
    //    region.srcSubresource.layerCount = 1;
    //    region.srcSubresource.baseArrayLayer = 0;

    //    region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    //    region.dstSubresource.mipLevel = 0;
    //    region.dstSubresource.layerCount = 1;
    //    region.dstSubresource.baseArrayLayer = 0;

    //    vkCmdCopyImage(cmd, mResManager->getVkImage(mView[imageIndex]->gbuffer->depth), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, mResManager->getVkImage(mView[imageIndex]->prevDepth), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    //    recordBarrier(cmd, mResManager->getVkImage(mView[imageIndex]->prevDepth), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    //                  VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
    //}

    // assign prev resources for next frame rendering
    mPrevView = mView[imageIndex];

    if (vkEndCommandBuffer(cmd) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;

    vkResetFences(mDevice, 1, &currFrame.inFlightFence);

    if (vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, currFrame.inFlightFence) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    vkWaitForFences(mDevice, 1, &currFrame.inFlightFence, true, UINT64_MAX);

    vkDeviceWaitIdle(mDevice);

    void* data = mResManager->getMappedMemory(stagingBuffer);
    memcpy((void*)outPixels, data, static_cast<size_t>(imageSize));

    ++mFrameNumber;
}

bool PtRender::isDeviceSuitable(VkPhysicalDevice device)
{
    QueueFamilyIndices indices = findQueueFamilies(device);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return indices.isComplete() && extensionsSupported && supportedFeatures.samplerAnisotropy;
}

bool PtRender::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

QueueFamilyIndices PtRender::findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
        }

        {
            indices.presentFamily = i;
        }

        if (indices.isComplete())
        {
            break;
        }

        i++;
    }

    return indices;
}

std::vector<const char*> PtRender::getRequiredExtensions()
{
    //uint32_t glfwExtensionCount = 0;
    //const char** glfwExtensions;
    ///glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    //std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    std::vector<const char*> extensions;

    if (enableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    return extensions;
}

bool PtRender::checkValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers)
    {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            return false;
        }
    }

    return true;
}
