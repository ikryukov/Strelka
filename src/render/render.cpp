#include "render.h"

#include "debugUtils.h"
#include "instanceconstants.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>

// profiler
#include "Tracy.hpp"

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

void Render::initVulkan()
{
    createInstance();
    setupDebugMessenger();
    if (enableValidationLayers)
    {
        //nevk::debug::setupDebug(mInstance);
    }
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();

    // load shaders
    const char* shShaderCode = nullptr;
    uint32_t shShaderCodeSize = 0;
    const char* pbrVertShaderCode = nullptr;
    uint32_t pbrVertShaderCodeSize = 0;
    const char* pbrFragShaderCode = nullptr;
    uint32_t pbrFragShaderCodeSize = 0;

    uint32_t shId = mShaderManager.loadShader("shaders/shadowmap.hlsl", "vertexMain", nevk::ShaderManager::Stage::eVertex);
    mShaderManager.getShaderCode(shId, shShaderCode, shShaderCodeSize);

    uint32_t pbrVertId = mShaderManager.loadShader("shaders/pbr.hlsl", "vertexMain", nevk::ShaderManager::Stage::eVertex);
    uint32_t pbrFragId = mShaderManager.loadShader("shaders/pbr.hlsl", "fragmentMain", nevk::ShaderManager::Stage::ePixel);

    mShaderManager.getShaderCode(pbrVertId, pbrVertShaderCode, pbrVertShaderCodeSize);
    mShaderManager.getShaderCode(pbrFragId, pbrFragShaderCode, pbrFragShaderCodeSize);

    createDescriptorPool();
    createCommandPool();

    mResManager = new nevk::ResourceManager(mDevice, mPhysicalDevice, mInstance, getCurrentFrameData().cmdPool, mGraphicsQueue);
    mTexManager = new nevk::TextureManager(mDevice, mPhysicalDevice, mResManager);

    createImageViews();
    createCommandBuffers();
    createSyncObjects();

    mView[0] = createView(swapChainExtent.width, swapChainExtent.height);
    mView[1] = createView(swapChainExtent.width, swapChainExtent.height);
    mView[2] = createView(swapChainExtent.width, swapChainExtent.height);

    mSharedCtx.mDescriptorPool = mDescriptorPool;
    mSharedCtx.mDevice = mDevice;
    mSharedCtx.mResManager = mResManager;
    mSharedCtx.mShaderManager = &mShaderManager;
    mSharedCtx.mTextureManager = mTexManager;

    TextureManager::TextureSamplerDesc defSamplerDesc{ VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT };
    mTexManager->createTextureSampler(defSamplerDesc);
    modelLoader = new nevk::ModelLoader(mTexManager);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        mUploadBuffer[i] = mResManager->createBuffer(MAX_UPLOAD_SIZE, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }

    mTexManager->createShadowSampler();
    QueueFamilyIndices indicesFamily = findQueueFamilies(mPhysicalDevice);

    ImGui_ImplVulkan_InitInfo init_info{};
    init_info.DescriptorPool = mDescriptorPool;
    init_info.Device = mDevice;
    init_info.ImageCount = MAX_FRAMES_IN_FLIGHT;
    init_info.Instance = mInstance;
    init_info.MinImageCount = 2;
    init_info.PhysicalDevice = mPhysicalDevice;
    init_info.Queue = mGraphicsQueue;
    init_info.QueueFamily = indicesFamily.graphicsFamily.value();

    mUi.init(init_info, swapChainImageFormat, mWindow, mFramesData[0].cmdPool, mFramesData[0].cmdBuffer, swapChainExtent.width, swapChainExtent.height);
    mUi.createFrameBuffers(mDevice, swapChainImageViews, swapChainExtent.width, swapChainExtent.height);
}

void Render::initPasses()
{
    mDebugView = new DebugView(mSharedCtx);
    mDebugView->initialize();

    mTonemap = new Tonemap(mSharedCtx);
    mTonemap->initialize();

    mUpscalePass = new UpscalePass(mSharedCtx);
    mUpscalePass->initialize();

    mComposition = new Composition(mSharedCtx);
    mComposition->initialize();

    mLtcPass = new LtcPass(mSharedCtx);
    mLtcPass->initialize();

    mBilateralFilter = new BilateralFilter(mSharedCtx);
    mBilateralFilter->initialize();

    mAOBilateralFilter = new BilateralFilter(mSharedCtx);
    mAOBilateralFilter->initialize();

    mRtShadow = new RtShadowPass(mSharedCtx);
    mRtShadow->initialize();

    mReflection = new Reflection(mSharedCtx);
    mReflection->initialize();

    mAO = new AOPass(mSharedCtx);
    mAO->initialize();

    mAccumulationShadows = new Accumulation(mSharedCtx);
    mAccumulationShadows->initialize();

    mAccumulationAO = new Accumulation(mSharedCtx);
    mAccumulationAO->initialize();

    mAccumulationPathTracer = new Accumulation(mSharedCtx);
    mAccumulationPathTracer->initialize();

    createDefaultScene();
    if (!MODEL_PATH.empty())
    {
        // Workaround:
        //mTexManager->saveTexturesInDelQueue();
        loadScene(MODEL_PATH);
    }

    // Material manager
    const fs::path cwd = fs::current_path();
    std::ifstream pt(cwd.string() + "/shaders/pathtracerMdl.hlsl");
    std::stringstream ptcode;
    ptcode << pt.rdbuf();

    mMaterialManager = new MaterialManager();
    const char* paths[3] = { "./misc/test_data/mdl/", "./misc/test_data/mdl/resources/",
                             "./misc/vespa" };
    bool res = mMaterialManager->addMdlSearchPath(paths, 3);
    if (!res)
    {
        // failed to load MDL
        return;
    }

    std::unique_ptr<MaterialManager::Module> currModule = mMaterialManager->createModule("gltf_support.mdl");

    std::vector<std::unique_ptr<MaterialManager::CompiledMaterial>> materials;
    {
        std::vector<Material> gltfMaterials = mScene->getMaterials();
        for (int i = 0; i < gltfMaterials.size(); ++i)
        {
            const Material& gltfMaterial = gltfMaterials[i];
            // create MDL mat instance
            std::unique_ptr<MaterialManager::MaterialInstance> materialInst1 = mMaterialManager->createMaterialInstance(currModule.get(), "gltf_material");

            // create Textures for MDL from gltf
            auto createTexture = [&](int32_t id, const char* paramName) {
                auto texIter = mScene->mTexIdToTexName.find(id);
                if (texIter != mScene->mTexIdToTexName.end())
                {
                    std::string texName = texIter->second;
                    MaterialManager::TextureDescription* texDesc = mMaterialManager->createTextureDescription(texName.c_str(), "linear");
                    if (texDesc != nullptr)
                    {
                        res = mMaterialManager->changeParam(materialInst1.get(), MaterialManager::ParamType::eTexture, paramName, (const void*)texDesc);
                        assert(res);
                    }
                }
            };

            createTexture(gltfMaterial.texBaseColor, "base_color_texture");
            createTexture(gltfMaterial.texNormalId, "normal_texture");
            createTexture(gltfMaterial.texEmissive, "emissive_texture");
            createTexture(gltfMaterial.texOcclusion, "occlusion_texture");
            createTexture(gltfMaterial.texMetallicRoughness, "metallic_roughness_texture");

            // set params: colors, floats... textures
            res = mMaterialManager->changeParam(materialInst1.get(), MaterialManager::ParamType::eColor, "base_color_factor", &gltfMaterial.baseColorFactor);
            assert(res);
            res = mMaterialManager->changeParam(materialInst1.get(), MaterialManager::ParamType::eFloat, "metallic_factor", &gltfMaterial.metallicFactor);
            assert(res);
            res = mMaterialManager->changeParam(materialInst1.get(), MaterialManager::ParamType::eFloat, "roughness_factor", &gltfMaterial.roughnessFactor);
            assert(res);
            res = mMaterialManager->changeParam(materialInst1.get(), MaterialManager::ParamType::eColor, "emissive_factor", &gltfMaterial.emissiveFactor);
            assert(res);

            // compile Materials
            std::unique_ptr<MaterialManager::CompiledMaterial> materialComp1 = mMaterialManager->compileMaterial(materialInst1.get());
            materials.push_back(std::move(materialComp1));
        }
    }

    // MTLX
    //    const char* path[2] = { "/Users/jswark/school/USD_Build/mdl/", "misc/test_data/mtlx" };
    //    bool res = mMaterialManager->addMdlSearchPath(path, 2);
    //    assert(res);
    //
    //    std::string file = "misc/test_data/mtlx/standard_surface_wood_tiled.mtlx";
    //    std::unique_ptr<MaterialManager::Module> currModule = mMaterialManager->createMtlxModule(file.c_str());
    //    assert(currModule);
    //    std::unique_ptr<MaterialManager::MaterialInstance> materialInst1 = mMaterialManager->createMaterialInstance(currModule.get(), "");
    //    assert(materialInst1);
    //    std::unique_ptr<MaterialManager::CompiledMaterial> materialComp1 = mMaterialManager->compileMaterial(materialInst1.get());
    //    assert(materialComp1);
    //
    //    std::vector<std::unique_ptr<MaterialManager::CompiledMaterial>> materials;
    //    materials.push_back(std::move(materialComp1));
    // MTLX

    // generate code for PT
    assert(materials.size() != 0);
    const MaterialManager::TargetCode* code = mMaterialManager->generateTargetCode(materials);
    const char* hlsl = mMaterialManager->getShaderCode(code);
    //std::cout << hlsl << std::endl;

    // MTLX
    //    uint32_t texSize = mMaterialManager->getTextureCount(code);
    //    for (uint32_t i = 1; i < texSize; ++i)
    //    {
    //        const float* data = mMaterialManager->getTextureData(code, i);
    //        uint32_t width = mMaterialManager->getTextureWidth(code, i);
    //        uint32_t height = mMaterialManager->getTextureHeight(code, i);
    //        const char* type = mMaterialManager->getTextureType(code, i);
    //        std::string name = mMaterialManager->getTextureName(code, i);
    //        if (data != NULL) // todo: for bsdf_text it is NULL ?? in COMPILATION_CLASS. in default class there is no bsdf_tex, so it is ok
    //        {
    //            mTexManager->loadTextureMdl(data, width, height, type, name);
    //        }
    //        else
    //        {
    //            std::cout << "tiled " << name << std::endl;
    //        }
    //    }
    // MTLX

    std::string newPTCode = std::string(hlsl) + "\n" + ptcode.str();
    //std::string newPTCode = ptcode.str();

    //std::string ptFile = cwd.string() + "/shaders/newPT.hlsl";
    //std::ofstream outHLSLShaderFile(ptFile.c_str());
    //outHLSLShaderFile << newPTCode;

    mPathTracer = new PathTracer(mSharedCtx, newPTCode);
    mPathTracer->initialize();

    // Workaround:
    mCurrentSceneRenderData->mMaterialTargetCode = code;

    createMdlBuffers();
    createMdlTextures();

    createGbufferPass();

    setDescriptors(0);
}

void Render::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    if (width == 0 || height == 0)
    {
        return;
    }
    auto app = reinterpret_cast<Render*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
    nevk::Scene* scene = app->getScene();
    scene->updateCamerasParams(width, height);
}

inline void Render::keyCallback(GLFWwindow* window, int key, [[maybe_unused]] int scancode, int action, [[maybe_unused]] int mods)
{
    auto app = reinterpret_cast<Render*>(glfwGetWindowUserPointer(window));
    nevk::Scene* scene = app->getScene();
    Camera& camera = scene->getCamera(app->getActiveCameraIndex());

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

inline void Render::mouseButtonCallback(GLFWwindow* window, int button, int action, [[maybe_unused]] int mods)
{
    auto app = reinterpret_cast<Render*>(glfwGetWindowUserPointer(window));
    nevk::Scene* scene = app->getScene();
    Camera& camera = scene->getCamera(app->getActiveCameraIndex());
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

void Render::handleMouseMoveCallback(GLFWwindow* window, double xpos, double ypos)
{
    auto app = reinterpret_cast<Render*>(glfwGetWindowUserPointer(window));
    nevk::Scene* scene = app->getScene();
    Camera& camera = scene->getCamera(app->getActiveCameraIndex());
    const float dx = camera.mousePos.x - (float)xpos;
    const float dy = camera.mousePos.y - (float)ypos;

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

void Render::scrollCallback(GLFWwindow* window, [[maybe_unused]] double xoffset, double yoffset)
{
    ImGuiIO& io = ImGui::GetIO();
    bool handled = io.WantCaptureMouse;
    if (handled)
    {
        return;
    }

    auto app = reinterpret_cast<Render*>(glfwGetWindowUserPointer(window));
    nevk::Scene* mScene = app->getScene();
    Camera& mCamera = mScene->getCamera(app->getActiveCameraIndex());

    mCamera.translate(glm::vec3(0.0f, 0.0f,
                                -yoffset * mCamera.movementSpeed));
}

double Render::fpsCounter(double frameTime)
{
    static double elapsedTime = 0.0;
    static uint64_t framesCounter = 0;

    elapsedTime += frameTime;
    ++framesCounter;

    if (elapsedTime >= 1000.0)
    {
        msPerFrame = elapsedTime / framesCounter;
        framesCounter = 0;
        elapsedTime = 0;
    }

    return msPerFrame;
}

void Render::mainLoop()
{
    while (!glfwWindowShouldClose(mWindow))
    {
        auto start = std::chrono::high_resolution_clock::now();

        glfwPollEvents();
        drawFrame();

        auto finish = std::chrono::high_resolution_clock::now();

        double frameTime = std::chrono::duration<double, std::milli>(finish - start).count();
        msPerFrame = fpsCounter(frameTime);
        FrameMark;
    }

    vkDeviceWaitIdle(mDevice);
}

void Render::cleanupSwapChain()
{
    for (auto& framebuffer : swapChainFramebuffers)
    {
        vkDestroyFramebuffer(mDevice, framebuffer, nullptr);
    }

    for (auto& imageView : swapChainImageViews)
    {
        vkDestroyImageView(mDevice, imageView, nullptr);
    }

    vkDestroySwapchainKHR(mDevice, mSwapChain, nullptr);
}

void Render::cleanup()
{
    cleanupSwapChain();

    // mDepthPass.onDestroy();
    mGbufferPass.onDestroy();

    mUi.onDestroy();

    vkDestroyDescriptorPool(mDevice, mDescriptorPool, nullptr);

    mTexManager->textureDestroy();
    mTexManager->delTexturesFromQueue();

    delete mView[0];
    delete mView[1];
    delete mView[2];

    delete mTonemap;
    delete mUpscalePass;
    delete mComposition;
    delete mDebugView;
    delete mLtcPass;
    delete mBilateralFilter;
    delete mAOBilateralFilter;
    delete mRtShadow;
    delete mPathTracer;
    delete mReflection;
    delete mAO;
    delete mAccumulationShadows;
    delete mAccumulationAO;
    delete mAccumulationPathTracer;

    delete mDefaultSceneRenderData;
    if (mCurrentSceneRenderData != mDefaultSceneRenderData)
    {
        delete mCurrentSceneRenderData;
    }

    delete mDefaultScene;

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

    vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
    vkDestroyInstance(mInstance, nullptr);

    glfwDestroyWindow(mWindow);

    glfwTerminate();
}

void Render::initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    mWindow = glfwCreateWindow(WIDTH, HEIGHT, "NeVK", nullptr, nullptr);
    glfwSetWindowUserPointer(mWindow, this);
    glfwSetFramebufferSizeCallback(mWindow, framebufferResizeCallback);
    glfwSetKeyCallback(mWindow, keyCallback);
    glfwSetMouseButtonCallback(mWindow, mouseButtonCallback);
    glfwSetCursorPosCallback(mWindow, handleMouseMoveCallback);
    glfwSetScrollCallback(mWindow, scrollCallback);
}

void Render::recreateSwapChain()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(mWindow, &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(mWindow, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(mDevice);

    cleanupSwapChain();

    createSwapChain();
    createImageViews();

    delete mView[0];
    delete mView[1];
    delete mView[2];

    mPrevView = nullptr;

    mView[0] = createView(swapChainExtent.width, swapChainExtent.height);
    mView[1] = createView(swapChainExtent.width, swapChainExtent.height);
    mView[2] = createView(swapChainExtent.width, swapChainExtent.height);

    mGbufferPass.onResize(mView[0]->gbuffer, 0);
    mGbufferPass.onResize(mView[1]->gbuffer, 1);
    mGbufferPass.onResize(mView[2]->gbuffer, 2);

    setDescriptors(0);

    mUi.onResize(swapChainImageViews, width, height);

    // update all cameras
    mScene->updateCamerasParams(swapChainExtent.width, swapChainExtent.height);
}

void Render::createInstance()
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

void Render::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}


void Render::setupDebugMessenger()
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

void Render::createSurface()
{
    if (glfwCreateWindowSurface(mInstance, mWindow, nullptr, &mSurface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }
}

void Render::pickPhysicalDevice()
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

void Render::createLogicalDevice()
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
    vkGetDeviceQueue(mDevice, indices.presentFamily.value(), 0, &mPresentQueue);
}

void Render::createSwapChain()
{
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(mPhysicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = mSurface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    QueueFamilyIndices indices = findQueueFamilies(mPhysicalDevice);
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    if (vkCreateSwapchainKHR(mDevice, &createInfo, nullptr, &mSwapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(mDevice, mSwapChain, &imageCount, nullptr);
    mSwapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(mDevice, mSwapChain, &imageCount, mSwapChainImages.data());

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void Render::createImageViews()
{
    swapChainImageViews.resize(mSwapChainImages.size());

    for (uint32_t i = 0; i < mSwapChainImages.size(); i++)
    {
        swapChainImageViews[i] = mTexManager->createImageView(mSwapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

Render::ViewData* Render::createView(uint32_t width, uint32_t height) // swapchain width&height
{
    assert(mResManager);
    ViewData* view = new ViewData();
    view->finalWidth = width;
    view->finalHeight = height;
    view->renderWidth = (uint32_t)(width * mRenderConfig.upscaleFactor);
    view->renderHeight = (uint32_t)(height * mRenderConfig.upscaleFactor);
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

    view->textureTonemapImage = mResManager->createImage(view->renderWidth, view->renderHeight, VK_FORMAT_R16G16B16A16_SFLOAT,
                                                         VK_IMAGE_TILING_OPTIMAL,
                                                         VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Tonemap result");
    view->textureUpscaleImage = mResManager->createImage(width, height, VK_FORMAT_R32G32B32A32_SFLOAT,
                                                         VK_IMAGE_TILING_OPTIMAL,
                                                         VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Upscale Output");
    mTexManager->transitionImageLayout(mResManager->getVkImage(view->textureUpscaleImage), VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    view->textureCompositionImage = mResManager->createImage(view->renderWidth, view->renderHeight, VK_FORMAT_R16G16B16A16_SFLOAT,
                                                             VK_IMAGE_TILING_OPTIMAL,
                                                             VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Composition result");
    mTexManager->transitionImageLayout(mResManager->getVkImage(view->textureCompositionImage), VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    view->mLtcOutputImage = mResManager->createImage(view->renderWidth, view->renderHeight, VK_FORMAT_R32G32B32A32_SFLOAT,
                                                     VK_IMAGE_TILING_OPTIMAL,
                                                     VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Ltc Output");
    view->mBilateralOutputImage = mResManager->createImage(view->renderWidth, view->renderHeight, VK_FORMAT_R16_SFLOAT,
                                                           VK_IMAGE_TILING_OPTIMAL,
                                                           VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Bilateral Output");
    view->mBilateralVarianceOutputImage = mResManager->createImage(view->renderWidth, view->renderHeight, VK_FORMAT_R16_SFLOAT,
                                                                   VK_IMAGE_TILING_OPTIMAL,
                                                                   VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Bilateral Variance Output");
    view->mAOBilateralOutputImage = mResManager->createImage(view->renderWidth, view->renderHeight, VK_FORMAT_R16_SFLOAT,
                                                             VK_IMAGE_TILING_OPTIMAL,
                                                             VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "AO Bilateral Output");
    view->mAOBilateralVarianceOutputImage = mResManager->createImage(view->renderWidth, view->renderHeight, VK_FORMAT_R16_SFLOAT,
                                                                     VK_IMAGE_TILING_OPTIMAL,
                                                                     VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                                                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "AO Bilateral Variance Output");
    view->mRtShadowImage = mResManager->createImage(view->renderWidth, view->renderHeight, VK_FORMAT_R16_SFLOAT,
                                                    VK_IMAGE_TILING_OPTIMAL,
                                                    VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "RT Shadow");
    view->mPathTracerImage = mResManager->createImage(view->renderWidth, view->renderHeight, VK_FORMAT_R32G32B32A32_SFLOAT,
                                                      VK_IMAGE_TILING_OPTIMAL,
                                                      VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Path Tracer Output");

    view->mReflectionImage = mResManager->createImage(view->renderWidth, view->renderHeight, VK_FORMAT_R32G32B32A32_SFLOAT,
                                                      VK_IMAGE_TILING_OPTIMAL,
                                                      VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Reflection Output");
    view->mAOImage = mResManager->createImage(view->renderWidth, view->renderHeight, VK_FORMAT_R16_SFLOAT,
                                              VK_IMAGE_TILING_OPTIMAL,
                                              VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "AO Output");

    const std::string imageName = "Accumulation Image";
    view->mAccumulationImages = mResManager->createImage(view->renderWidth, view->renderHeight, VK_FORMAT_R16_SFLOAT,
                                                         VK_IMAGE_TILING_OPTIMAL,
                                                         VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, imageName.c_str());
    mTexManager->transitionImageLayout(mResManager->getVkImage(view->mAccumulationImages), VK_FORMAT_R16_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    view->mAccumulationAOImages = mResManager->createImage(view->renderWidth, view->renderHeight, VK_FORMAT_R16_SFLOAT,
                                                           VK_IMAGE_TILING_OPTIMAL,
                                                           VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, imageName.c_str());
    mTexManager->transitionImageLayout(mResManager->getVkImage(view->mAccumulationAOImages), VK_FORMAT_R16_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);


    view->mAccumulationPathTracerImage = mResManager->createImage(view->renderWidth, view->renderHeight, VK_FORMAT_R32G32B32A32_SFLOAT,
                                                                  VK_IMAGE_TILING_OPTIMAL,
                                                                  VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "PT accumulation");
    mTexManager->transitionImageLayout(mResManager->getVkImage(view->mAccumulationPathTracerImage), VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    return view;
}

GBuffer* Render::createGbuffer(uint32_t width, uint32_t height)
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

void Render::createGbufferPass()
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
    assert(mView[1]);
    assert(mView[2]);
    mGbufferPass.init(mDevice, enableValidationLayers, vertShaderCode, vertShaderCodeSize, fragShaderCode, fragShaderCodeSize, mDescriptorPool, mResManager, mView[0]->gbuffer);
    mGbufferPass.createFrameBuffers(*mView[0]->gbuffer, 0);
    mGbufferPass.createFrameBuffers(*mView[1]->gbuffer, 1);
    mGbufferPass.createFrameBuffers(*mView[2]->gbuffer, 2);
}

void Render::createCommandPool()
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

VkFormat Render::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
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

VkFormat Render::findDepthFormat()
{
    return findSupportedFormat(
        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

bool Render::hasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void Render::setCamera()
{
    Camera& camera = mScene->getCamera(getActiveCameraIndex());
    camera.setPerspective(45.0f, (float)swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10000.0f);
    camera.setRotation(glm::quat({ 1.0f, 0.0f, 0.0f, 0.0f }));
}

void nevk::Render::createMdlTextures()
{
    const MaterialManager::TargetCode* code = mCurrentSceneRenderData->mMaterialTargetCode;

    uint32_t texSize = mMaterialManager->getTextureCount(code);
    for (uint32_t i = 1; i < texSize; ++i)
    {
        const float* data = mMaterialManager->getTextureData(code, i);
        uint32_t width = mMaterialManager->getTextureWidth(code, i);
        uint32_t height = mMaterialManager->getTextureHeight(code, i);
        const char* type = mMaterialManager->getTextureType(code, i);
        std::string name = mMaterialManager->getTextureName(code, i);

        std::cout << "Load MDL texture: " << name << std::endl;

        mTexManager->loadTextureMdl(data, width, height, type, name);
    }
}

void nevk::Render::createMdlBuffers()
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

void Render::createVertexBuffer(nevk::Scene& scene)
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

void Render::createMaterialBuffer(nevk::Scene& scene)
{
    std::vector<Material>& sceneMaterials = scene.getMaterials();

    VkDeviceSize bufferSize = sizeof(Material) * (sceneMaterials.size() + MAX_LIGHT_COUNT); // Reserve extra for lights material
    if (bufferSize == 0)
    {
        return;
    }
    Buffer* stagingBuffer = mResManager->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    void* stagingBufferMemory = mResManager->getMappedMemory(stagingBuffer);
    memcpy(stagingBufferMemory, sceneMaterials.data(), sizeof(Material) * sceneMaterials.size());
    mCurrentSceneRenderData->mMaterialBuffer = mResManager->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Materials");
    mResManager->copyBuffer(mResManager->getVkBuffer(stagingBuffer), mResManager->getVkBuffer(mCurrentSceneRenderData->mMaterialBuffer), bufferSize);
    mResManager->destroyBuffer(stagingBuffer);
}

void Render::createLightsBuffer(nevk::Scene& scene)
{
    std::vector<nevk::Scene::Light>& sceneLights = scene.getLights();

    VkDeviceSize bufferSize = sizeof(nevk::Scene::Light) * MAX_LIGHT_COUNT;
    if (bufferSize == 0)
    {
        return;
    }
    Buffer* stagingBuffer = mResManager->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    void* stagingBufferMemory = mResManager->getMappedMemory(stagingBuffer);
    memcpy(stagingBufferMemory, sceneLights.data(), sizeof(nevk::Scene::Light) * sceneLights.size());
    mCurrentSceneRenderData->mLightsBuffer = mResManager->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "Lights");
    mResManager->copyBuffer(mResManager->getVkBuffer(stagingBuffer), mResManager->getVkBuffer(mCurrentSceneRenderData->mLightsBuffer), bufferSize);
    mResManager->destroyBuffer(stagingBuffer);
}

#include "ray.h"

struct Hit
{
    glm::float2 bary;
    float t;
    uint32_t instId;
    uint32_t primId;
};

struct BVHTriangle
{
    glm::float3 v0;
    glm::float3 e0;
    glm::float3 e1;
};

void Render::createBvhBuffer(nevk::Scene& scene)
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
        //if (currInstId == 0)
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
        }
        ++currInstId;
    }

    BVH sceneBvh = mBvhBuilder.build(positions);
    {
        std::cout << sceneBvh.nodes.size() << std::endl;
        std::cout << sceneBvh.nodes[0].minBounds.x << " " << sceneBvh.nodes[0].minBounds.y << " " << sceneBvh.nodes[0].minBounds.z << " " << sceneBvh.nodes[0].maxBounds.x << " " << sceneBvh.nodes[0].maxBounds.y << " " << sceneBvh.nodes[0].maxBounds.z << std::endl;

        //for (uint32_t i = 0; i < sceneBvh.nodes.size(); ++i)
        //{
        //    BVHNode& currNode = sceneBvh.nodes[i];
        //    if (currNode.instId != -1) // leaf
        //    {
        //        union
        //        {
        //            uint32_t primUint;
        //            float primFloat;
        //        } leaf;
        //        leaf.primFloat = sceneBvh.nodes[i].minBounds.x;
        //        //std::cout << i << " " << sceneBvh.nodes[i].instId << " "  << leaf.primUint << " " << sceneBvh.nodes[i].nodeOffset  <<std::endl;
        //        printf("index: %d \t instId: %d \t triNum: %d \t offset: %d \t \n", i, sceneBvh.nodes[i].instId, leaf.primUint, sceneBvh.nodes[i].nodeOffset);
        //    }
        //    else
        //    {
        //        printf("index: %d \t instId: %d \t minBounds: (%f, %f, %f) \t maxBounds: (%f, %f, %f) \t offset: %d \t \n", i, sceneBvh.nodes[i].instId, sceneBvh.nodes[i].minBounds.x, sceneBvh.nodes[i].minBounds.y, sceneBvh.nodes[i].minBounds.z, sceneBvh.nodes[i].maxBounds.x, sceneBvh.nodes[i].maxBounds.y, sceneBvh.nodes[i].maxBounds.z, sceneBvh.nodes[i].nodeOffset);
        //    }
        //}
        if (0)
        {

            int32_t image_width = 200;
            int32_t image_height = 150;
            std::vector<uint8_t> colorData;
            colorData.resize(image_width * image_height * 3);
            int index = 0;

            Camera tmpCam;
            tmpCam = mScene->getCamera(0);

            tmpCam.updateViewMatrix();
            tmpCam.setPerspective(45, (float)image_width / image_height, 0.1f, 1000.f);

            std::vector<BVHNode>& nodesBvh = sceneBvh.nodes;
            for (int32_t y = 0; y < image_height; ++y)
            {
                for (int32_t x = 0; x < image_width; ++x)
                {
                    glm::float3 color = glm::float3(0.0);

                    auto rayGen = [&](glm::int2 pixelIndex, Camera& cam, glm::int2 dimension) {
                        glm::float2 pixelPos = glm::float2(pixelIndex) + 0.5f;
                        glm::float2 pixelNDC = (pixelPos / glm::float2(dimension)) * 2.0f - 1.0f;

                        glm::float4 clip = glm::float4(pixelNDC, 1.0, 1.0);
                        glm::float4 viewSpace = glm::inverse(cam.matrices.perspective) * clip;

                        glm::float4 wpos = glm::inverse(cam.matrices.view) * glm::float4(viewSpace.x, viewSpace.y, viewSpace.z, 0.0f);

                        Ray ray;
                        ray.o = glm::float4(cam.getPosition(), 0.0f);
                        ray.o.w = 1e9;
                        glm::float3 normDir = glm::normalize(wpos);
                        ray.d.x = normDir.x;
                        ray.d.y = normDir.y;
                        ray.d.z = normDir.z;

                        return ray;
                    };

                    auto getTriangleCPU = [&](uint32_t instId, uint32_t primitiveId) {
                        const nevk::Instance instConst = mScene->getInstances()[instId];
                        const std::vector<Mesh>& meshes = mScene->getMeshes();

                        const uint32_t currentMeshId = instConst.mMeshId;
                        int offset = meshes[currentMeshId].mIndex;
                        uint32_t i0 = mScene->mIndices[offset + primitiveId * 3 + 0];
                        uint32_t i1 = mScene->mIndices[offset + primitiveId * 3 + 1];
                        uint32_t i2 = mScene->mIndices[offset + primitiveId * 3 + 2];

                        // read and transform vertices, calculate edges
                        glm::float4x4 objectToWorld = instConst.transform;
                        glm::float4 tmpv0 = (objectToWorld * glm::float4(mScene->mVertices[i0].pos, 1.0));
                        glm::float3 v0 = { tmpv0.x, tmpv0.y, tmpv0.z };
                        glm::float4 tmpe0 = (objectToWorld * glm::float4(mScene->mVertices[i1].pos, 1.0)) - tmpv0;
                        glm::float3 e0 = { tmpe0.x, tmpe0.y, tmpe0.z };
                        glm::float4 tmpe1 = (objectToWorld * glm::float4(mScene->mVertices[i2].pos, 1.0)) - tmpv0;
                        glm::float3 e1 = { tmpe1.x, tmpe1.y, tmpe1.z };

                        BVHTriangle res;
                        res.v0 = v0;
                        res.e0 = e0;
                        res.e1 = e1;

                        return res;
                    };

                    auto interpolateAttrib = [](glm::float3 attr1, glm::float3 attr2, glm::float3 attr3, glm::float2 bary) {
                        return attr1 * (1 - bary.x - bary.y) + attr2 * bary.x + attr3 * bary.y;
                    };

                    //  valid range of coordinates [-1; 1]
                    auto unpackNormal = [](uint32_t val) {
                        glm::float3 normal;
                        normal.z = ((val & 0xfff00000) >> 20) / 511.99999f * 2.0f - 1.0f;
                        normal.y = ((val & 0x000ffc00) >> 10) / 511.99999f * 2.0f - 1.0f;
                        normal.x = (val & 0x000003ff) / 511.99999f * 2.0f - 1.0f;

                        return normal;
                    };

                    auto rayTriangleIntersectCPU = [&](const glm::float3 orig,
                                                       const glm::float3 dir,
                                                       glm::float3 v0,
                                                       glm::float3 e0,
                                                       glm::float3 e1,
                                                       float& t,
                                                       glm::float2& bCoord) {
                        const glm::float3 pvec = cross(dir, e1);

                        float det = glm::dot(e0, pvec);

                        // Backface culling
                        ///if (det < 1e-6)
                        //{
                        //    return false;
                        //}

                        if (abs(det) < 1e-6)
                        {
                            return false;
                        }

                        float invDet = 1.0f / det;

                        glm::float3 tvec = orig - v0;
                        float u = glm::dot(tvec, pvec) * invDet;
                        if (u < 0.0 || u > 1.0)
                        {
                            return false;
                        }

                        glm::float3 qvec = glm::cross(tvec, e0);
                        float v = glm::dot(dir, qvec) * invDet;
                        if (v < 0.0 || (u + v) > 1.0)
                        {
                            return false;
                        }

                        t = glm::dot(e1, qvec) * invDet;

                        if (t < 1e-6)
                        {
                            return false;
                        }

                        bCoord.x = u;
                        bCoord.y = v;

                        return true;
                    };

                    auto closestHit = [&](std::vector<BVHNode>& bvhNodes, Ray& ray, Hit& hit) {
                        const glm::float3 invdir = glm::float3(1.0 / ray.d.x, 1.0 / ray.d.y, 1.0 / ray.d.z);

                        float minHit = 1e9f;
                        bool isFound = false;

                        uint32_t nodeIndex = 0;
                        while (nodeIndex != -1)
                        {
                            BVHNode& node = bvhNodes[nodeIndex];
                            const uint32_t instanceIndex = node.instId;
                            float boxT = 1e9f;
                            hit.t = 0.0;
                            if (instanceIndex != -1) // leaf
                            {
                                union
                                {
                                    uint32_t primUint;
                                    float primFloat;
                                } leaf;
                                leaf.primFloat = node.minBounds.x;
                                const uint32_t primitiveIndex = leaf.primUint; // triangle index

                                BVHTriangle triangle = getTriangleCPU(instanceIndex, primitiveIndex);
                                float curT = 0;
                                glm::float2 bary = glm::float2(0.0);
                                const glm::float3 origin = { ray.o.x, ray.o.y, ray.o.z };
                                const glm::float3 dir = { ray.d.x, ray.d.y, ray.d.z };
                                bool isIntersected = rayTriangleIntersectCPU(origin, dir, triangle.v0, triangle.e0, triangle.e1, curT, bary);
                                if (isIntersected && (curT < ray.o.w) && (curT < minHit))
                                {
                                    minHit = curT;
                                    hit.t = curT;
                                    hit.bary = bary;
                                    hit.instId = instanceIndex;
                                    hit.primId = primitiveIndex;
                                    isFound = true;
                                }
                                nodeIndex = node.nodeOffset;
                                continue;
                            }
                            else if (intersectRayBox(ray, invdir, node.minBounds, node.maxBounds, boxT))
                            {
                                if (boxT > ray.o.w) // check max ray trace distance: skip this node if collision far away
                                {
                                    nodeIndex = node.nodeOffset;
                                    continue;
                                }
                                ++nodeIndex;
                                continue;
                            }

                            nodeIndex = node.nodeOffset;
                        }

                        return isFound;
                    };
                    // rayGen = [&](glm::int2& pixelIndex, Camera& cam, glm::int2 dimension)
                    Ray ray = rayGen(glm::int2(x, y), tmpCam, glm::int2(image_width, image_height));
                    // auto closestHit = [&](std::vector<BVHNode>& bvhNodes, Ray& ray, Hit& hit) {
                    Hit hit;
                    bool f = closestHit(nodesBvh, ray, hit);
                    if (f)
                    {
                        InstanceConstants instConst = {};

                        const uint32_t currentMeshId = mScene->mInstances[hit.instId].mMeshId;

                        instConst.objectToWorld = mScene->mInstances[hit.instId].transform;
                        instConst.indexOffset = mScene->mMeshes[currentMeshId].mIndex;
                        instConst.normalMatrix = glm::inverse(glm::transpose(instConst.objectToWorld));

                        uint32_t i0 = mScene->mIndices[instConst.indexOffset + hit.primId * 3 + 0];
                        uint32_t i1 = mScene->mIndices[instConst.indexOffset + hit.primId * 3 + 1];
                        uint32_t i2 = mScene->mIndices[instConst.indexOffset + hit.primId * 3 + 2];

                        glm::float3 p0 = glm::float3(instConst.objectToWorld * glm::float4(mScene->mVertices[i0].pos, 1.0f));
                        glm::float3 p1 = glm::float3(instConst.objectToWorld * glm::float4(mScene->mVertices[i1].pos, 1.0f));
                        glm::float3 p2 = glm::float3(instConst.objectToWorld * glm::float4(mScene->mVertices[i2].pos, 1.0f));

                        glm::float3 geom_normal = normalize(cross(p1 - p0, p2 - p0));

                        glm::float3 n0 = ((glm::float3x3)instConst.normalMatrix, unpackNormal(mScene->mVertices[i0].normal));
                        glm::float3 n1 = ((glm::float3x3)instConst.normalMatrix, unpackNormal(mScene->mVertices[i1].normal));
                        glm::float3 n2 = ((glm::float3x3)instConst.normalMatrix, unpackNormal(mScene->mVertices[i2].normal));

                        glm::float3 world_normal = normalize(interpolateAttrib(n0, n1, n2, hit.bary));

                        color = world_normal;
                    }
                    else
                    {
                        color = glm::float3(0.0f);
                    }

                    auto r = color.x;
                    auto g = color.y;
                    auto b = color.z;

                    int ir = static_cast<int>(256 * glm::clamp(r, 0.0f, 0.999f)), ig = static_cast<int>(256 * glm::clamp(g, 0.0f, 0.999f)),
                        ib = static_cast<int>(256 * glm::clamp(b, 0.0f, 0.999f));

                    colorData[index++] = (uint8_t)ir;
                    colorData[index++] = (uint8_t)ig;
                    colorData[index++] = (uint8_t)ib;
                }
            }
            mTexManager->savePNG(image_width, image_height, (uint8_t*)colorData.data());
        }

        VkDeviceSize bufferSize = sizeof(BVHNode) * sceneBvh.nodes.size();
        lenBVH = (int32_t) sceneBvh.nodes.size();
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

void Render::createIndexBuffer(nevk::Scene& scene)
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

void Render::createInstanceBuffer(nevk::Scene& scene)
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

void Render::createDescriptorPool()
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

uint32_t Render::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void Render::recordBarrier(VkCommandBuffer& cmd, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccess, VkAccessFlags dstAccess, VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage, VkImageAspectFlags aspectMask)
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

void Render::createCommandBuffers()
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

void Render::createSyncObjects()
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

// load into normal scene
void Render::loadScene(const std::string& modelPath)
{
    if (mScene != nullptr && mScene != mDefaultScene)
        delete mScene;
    mScene = new nevk::Scene;

    mCurrentSceneRenderData = new SceneRenderData(mResManager);
    MODEL_PATH = modelPath;
    mScene->createLightMesh();
    bool res = modelLoader->loadModelGltf(MODEL_PATH, *mScene);
    if (!res)
    {
        return;
    }
    mScene->modelPath = MODEL_PATH;

    std::string currentFileName = mScene->getSceneFileName();
    std::string fileName = currentFileName.substr(0, currentFileName.rfind('.')); // w/o extension
    std::string lightPath = mScene->getSceneDir() + "/" + fileName + "_light" + ".json";
    if (fs::exists(lightPath))
    {
        mUi.loadFromJson(*mScene);
    }
    else
    {
        // for pica pica
        Scene::RectLightDesc desc{};
        desc.position = glm::float3{ 0, 30, 10 };
        desc.orientation = glm::float3{ 0, 90, 0 };
        desc.width = 50.f;
        desc.height = 50.f;
        desc.color = glm::float3{ 1.0, 1.0, 1.0 };
        desc.intensity = 1.0;
        mScene->createLight(desc);
    }
    if (!mScene->mAnimations.empty())
    {
        mCurrentSceneRenderData->animationTime = mScene->mAnimations[0].start;
    }
    createMaterialBuffer(*mScene);
    createInstanceBuffer(*mScene);
    createLightsBuffer(*mScene);
    createBvhBuffer(*mScene);

    mTexManager->createShadowSampler();

    createIndexBuffer(*mScene);
    createVertexBuffer(*mScene);

    //setDescriptors(0);
}

void Render::setDescriptors(uint32_t imageIndex)
{
    ViewData* currView = mView[imageIndex];

    {
        mGbufferPass.setTextureImageView(mTexManager->textureImageView);
        mGbufferPass.setTextureSamplers(mTexManager->texSamplers);
        mGbufferPass.setMaterialBuffer(mResManager->getVkBuffer(mCurrentSceneRenderData->mMaterialBuffer));
        mGbufferPass.setInstanceBuffer(mResManager->getVkBuffer(mCurrentSceneRenderData->mInstanceBuffer));
    }
    {
        RtShadowPassDesc desc{};
        desc.result = currView->mRtShadowImage;
        desc.lights = mCurrentSceneRenderData->mLightsBuffer;
        desc.gbuffer = currView->gbuffer;
        desc.bvhNodes = mCurrentSceneRenderData->mBvhNodeBuffer;
        desc.instanceConstants = mCurrentSceneRenderData->mInstanceBuffer;
        desc.vb = mCurrentSceneRenderData->mVertexBuffer;
        desc.ib = mCurrentSceneRenderData->mIndexBuffer;

        mRtShadow->setResources(desc);
    }
    {
        PathTracerDesc desc{};
        desc.result = currView->mPathTracerImage;
        desc.gbuffer = currView->gbuffer;
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
    {
        ReflectionDesc desc{};
        desc.result = currView->mReflectionImage;
        desc.gbuffer = currView->gbuffer;
        desc.bvhNodes = mCurrentSceneRenderData->mBvhNodeBuffer;
        desc.vb = mCurrentSceneRenderData->mVertexBuffer;
        desc.ib = mCurrentSceneRenderData->mIndexBuffer;
        desc.instanceConst = mCurrentSceneRenderData->mInstanceBuffer;
        desc.materials = mCurrentSceneRenderData->mMaterialBuffer;
        desc.matSampler = mTexManager->texSamplers;
        desc.matTextures = mTexManager->textureImages;
        mReflection->setResources(desc);
    }
    {
        AOPassDesc desc{};
        desc.result = currView->mAOImage;
        desc.gbuffer = currView->gbuffer;
        desc.bvhNodes = mCurrentSceneRenderData->mBvhNodeBuffer;
        desc.instanceConstants = mCurrentSceneRenderData->mInstanceBuffer;
        desc.vb = mCurrentSceneRenderData->mVertexBuffer;
        desc.ib = mCurrentSceneRenderData->mIndexBuffer;

        mAO->setResources(desc);
    }
    {
        mAccumulationShadows->setInputTexture1(currView->mRtShadowImage);
        mAccumulationShadows->setMotionTexture(currView->gbuffer->motion);
        mAccumulationShadows->setPrevDepthTexture(currView->prevDepth);
        mAccumulationShadows->setWposTexture(currView->gbuffer->wPos);
        mAccumulationShadows->setCurrDepthTexture(currView->gbuffer->depth);
    }
    {
        mAccumulationAO->setInputTexture1(currView->mAOImage);
        mAccumulationAO->setMotionTexture(currView->gbuffer->motion);
        mAccumulationAO->setPrevDepthTexture(currView->prevDepth);
        mAccumulationAO->setWposTexture(currView->gbuffer->wPos);
        mAccumulationAO->setCurrDepthTexture(currView->gbuffer->depth);
    }
    {
        mAccumulationPathTracer->setInputTexture4(currView->mPathTracerImage);
        mAccumulationPathTracer->setMotionTexture(currView->gbuffer->motion);
        mAccumulationPathTracer->setPrevDepthTexture(currView->prevDepth);
        mAccumulationPathTracer->setWposTexture(currView->gbuffer->wPos);
        mAccumulationPathTracer->setCurrDepthTexture(currView->gbuffer->depth);
    }
    {
        LtcResourceDesc desc{};
        desc.gbuffer = currView->gbuffer;
        desc.instanceConst = mCurrentSceneRenderData->mInstanceBuffer;
        desc.lights = mCurrentSceneRenderData->mLightsBuffer;
        desc.materials = mCurrentSceneRenderData->mMaterialBuffer;
        desc.result = currView->mLtcOutputImage;
        desc.matSampler = mTexManager->texSamplers;
        desc.matTextures = mTexManager->textureImages;
        mLtcPass->setResources(desc);
    }
    {
        mDebugImages.shadow = currView->mRtShadowImage;
        mDebugImages.LTC = currView->mLtcOutputImage;
        mDebugImages.normal = currView->gbuffer->normal;
        mDebugImages.debug = currView->gbuffer->debug;
        mDebugImages.AO = currView->mAOImage;
        mDebugImages.motion = currView->gbuffer->motion;
        mDebugImages.reflection = currView->mReflectionImage;
        mDebugImages.variance = currView->mBilateralVarianceOutputImage;
        mDebugImages.pathTracer = currView->mPathTracerImage;
    }
    {
        BilateralResourceDesc desc{};
        desc.gbuffer = currView->gbuffer;
        desc.result = currView->mBilateralOutputImage;
        desc.variance = currView->mBilateralVarianceOutputImage;
        desc.input = currView->mRtShadowImage;
        mBilateralFilter->setResources(desc);
    }
    {
        BilateralResourceDesc desc{};
        desc.gbuffer = currView->gbuffer;
        desc.result = currView->mAOBilateralOutputImage;
        desc.variance = currView->mAOBilateralVarianceOutputImage;
        desc.input = currView->mRtShadowImage;
        mAOBilateralFilter->setResources(desc);
    }
    {
        mDebugView->setParams(mDebugParams);
        mDebugView->setInputTexture(mDebugImages);
        mDebugView->setOutputTexture(mResManager->getView(currView->textureDebugViewImage));
    }
    {
        mTonemap->setParams(mToneParams);
        mTonemap->setInputTexture(mResManager->getView(currView->textureCompositionImage));
        mTonemap->setOutputTexture(mResManager->getView(currView->textureTonemapImage));
    }
    {
        mUpscalePass->setParams(mUpscalePassParam);
        mUpscalePass->setInputTexture(mResManager->getView(currView->mPathTracerImage));
        mUpscalePass->setOutputTexture(mResManager->getView(currView->textureUpscaleImage));
    }
    {
        mCompositionImages.shadow = currView->mRtShadowImage;
        mCompositionImages.LTC = currView->mLtcOutputImage;
        mCompositionImages.AO = currView->mAOImage;
        mCompositionImages.reflections = currView->mReflectionImage;

        mComposition->setParams(mCompositionParam);
        mComposition->setInputTexture(mCompositionImages);
        mComposition->setOutputTexture(mResManager->getView(currView->textureCompositionImage));
    }
}

// set default scene
void Render::createDefaultScene()
{
    mDefaultScene = new nevk::Scene;
    mScene = mDefaultScene;
    mDefaultSceneRenderData = new SceneRenderData(mResManager);
    mCurrentSceneRenderData = mDefaultSceneRenderData;

    Camera camera = {};
    camera.setPerspective(45.0f, (float)swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10000.0f);
    camera.setRotation(glm::quat({ 1.0f, 0.0f, 0.0f, 0.0f }));

    mScene->addCamera(camera);

    // for pica pica
    mScene->createLightMesh();
    Scene::RectLightDesc desc{};
    desc.position = glm::float3{ 0, 30, 10 };
    desc.orientation = glm::float3{ 0, 90, 0 };
    desc.width = 50.f;
    desc.height = 50.f;
    desc.color = glm::float3{ 1.0, 1.0, 1.0 };
    desc.intensity = 1.0;
    mScene->createLight(desc);

    createMaterialBuffer(*mScene);
    createInstanceBuffer(*mScene);
    createLightsBuffer(*mScene);
    //createBvhBuffer(*mScene);

    createIndexBuffer(*mScene);
    createVertexBuffer(*mScene);
}

void Render::drawFrame()
{
    ZoneScoped;
    FrameData& currFrame = getCurrentFrameData();

    vkWaitForFences(mDevice, 1, &currFrame.inFlightFence, VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(mDevice, mSwapChain, UINT64_MAX, currFrame.imageAvailable, VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapChain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    if (getFrameData(imageIndex).imagesInFlight != VK_NULL_HANDLE)
    {
        vkWaitForFences(mDevice, 1, &getFrameData(imageIndex).imagesInFlight, VK_TRUE, UINT64_MAX);
    }
    getFrameData(imageIndex).imagesInFlight = currFrame.inFlightFence;

    static auto prevTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    double deltaTime = std::chrono::duration<double, std::milli>(currentTime - prevTime).count() / 1000.0;
    prevTime = currentTime;

    const uint32_t frameIndex = imageIndex;

    nevk::Scene* scene = getScene();

    static bool needReloadBVH = false;
    static uint32_t releaseBVHAfterFrames = 0;
    static Buffer* toRemoveBVH = nullptr;
    static uint32_t releaseSceneAfterFrames = 0;
    static bool needReloadScene = false;
    static SceneRenderData* toRemoveSceneData = nullptr;
    std::string newModelPath;

    mRenderStats.msPerFrame = msPerFrame;
    mSceneConfig.selectedCamera = mCurrentSceneRenderData->cameraIndex;
    mSceneConfig.newModelPath = newModelPath;
    mRenderConfig.animTime = mCurrentSceneRenderData->animationTime;
    mRenderConfig.samples = mSamples;

    mUi.updateUI(*scene, mRenderConfig, mRenderStats, mSceneConfig);
    mCurrentSceneRenderData->animationTime = mRenderConfig.animTime;

    if (mRenderConfig.recreateBVH)
    {
        toRemoveBVH = mCurrentSceneRenderData->mBvhNodeBuffer;
        needReloadBVH = true;
        releaseBVHAfterFrames = MAX_FRAMES_IN_FLIGHT;

        createBvhBuffer(*mScene); // need to update descriptors after it
    }

    if (!mSceneConfig.newModelPath.empty() && fs::exists(mSceneConfig.newModelPath) && mSceneConfig.newModelPath != MODEL_PATH)
    {
        if (mScene != mDefaultScene) // if we reload non-default scene
        {
            // save scene data to remove
            toRemoveSceneData = mCurrentSceneRenderData;
            needReloadScene = true;
            releaseSceneAfterFrames = MAX_FRAMES_IN_FLIGHT;
        }
        mTexManager->saveTexturesInDelQueue();
        loadScene(mSceneConfig.newModelPath);
    }

    scene = getScene();
    Camera& cam = scene->getCamera(getActiveCameraIndex());
    // save curr to prev
    cam.prevMatrices = cam.matrices;

    bool needResetPt = false;
    if (cam.moving())
    {
        needResetPt = true;
    }

    if (scene->mAnimState == Scene::AnimationState::ePlay || scene->mAnimState == Scene::AnimationState::eScroll)
    {
        if (scene->mAnimState == Scene::AnimationState::ePlay)
        {
            mCurrentSceneRenderData->animationTime += (float)deltaTime;
            if (mCurrentSceneRenderData->animationTime > scene->mAnimations[0].end)
            {
                mCurrentSceneRenderData->animationTime = scene->mAnimations[0].start; // ring
            }
            needResetPt = true;
        }
        else
        {
            scene->mAnimState = Scene::AnimationState::eStop;
            needResetPt = false;
        }

        scene->updateAnimation(mCurrentSceneRenderData->animationTime);
        glm::float4x4 xform = scene->getTransformFromRoot(cam.node);
        {
            glm::vec3 scale;
            glm::quat rotation;
            glm::vec3 translation;
            glm::vec3 skew;
            glm::vec4 perspective;
            glm::decompose(xform, scale, rotation, translation, skew, perspective);
            rotation = glm::conjugate(rotation);
            cam.position = translation * scale;
            cam.mOrientation = rotation;
            cam.updateViewMatrix();
        }
    }
    else
    {
        cam.update((float)deltaTime);
    }

    if (mRenderConfig.enableUpscale != prevState)
    {
        needImageViewUpdate = { true, true, true };
        prevState = mRenderConfig.enableUpscale;
    }

    if (needImageViewUpdate[imageIndex])
    {
        delete mView[imageIndex];
        mView[imageIndex] = createView(swapChainExtent.width, swapChainExtent.height);
        mGbufferPass.onResize(mView[imageIndex]->gbuffer, imageIndex);

        needResetPt = true;
        needImageViewUpdate[imageIndex] = false;
        setDescriptors(imageIndex);
    }

    setDescriptors(imageIndex);

    uint32_t renderWidth = mView[imageIndex]->renderWidth;
    uint32_t renderHeight = mView[imageIndex]->renderHeight;
    // swapchain size
    uint32_t finalWidth = mView[imageIndex]->finalWidth;
    uint32_t finalHeight = mView[imageIndex]->finalHeight;

    scene->updateCamerasParams(finalWidth, finalHeight);

    mGbufferPass.updateUniformBuffer(frameIndex, *scene, getActiveCameraIndex());

    RtShadowParam rtShadowParam{};
    rtShadowParam.dimension = glm::int2(renderWidth, renderHeight);
    rtShadowParam.frameNumber = (uint32_t)mFrameNumber;
    mRtShadow->setParams(rtShadowParam);

    PathTracerParam pathTracerParam{};
    pathTracerParam.dimension = glm::int2(renderWidth, renderHeight);
    pathTracerParam.frameNumber = (uint32_t)mFrameNumber;
    pathTracerParam.maxDepth = mRenderConfig.maxDepth;
    pathTracerParam.debug = (uint32_t)(mScene->mDebugViewSettings == Scene::DebugView::ePTDebug);
    pathTracerParam.camPos = glm::float4(cam.getPosition(), 1.0f);
    pathTracerParam.viewToWorld = glm::inverse(cam.matrices.view);
    pathTracerParam.worldToView = cam.matrices.view; //
    pathTracerParam.clipToView = cam.matrices.invPerspective;
    pathTracerParam.viewToClip = cam.matrices.perspective; //
    pathTracerParam.len = lenBVH;
    pathTracerParam.numLights = (uint32_t)scene->getLights().size();
    pathTracerParam.invDimension.x = 1.0f / (float)renderWidth;
    pathTracerParam.invDimension.y = 1.0f / (float)renderHeight;
    mPathTracer->setParams(pathTracerParam);

    ReflectionParam reflectionParam{};
    reflectionParam.camPos = cam.getPosition();
    reflectionParam.dimension = glm::int2(renderWidth, renderHeight);
    reflectionParam.frameNumber = (uint32_t)mFrameNumber;
    mReflection->setParams(reflectionParam);

    AOParam aoParam{};
    aoParam.dimension = glm::int2(renderWidth, renderHeight);
    aoParam.frameNumber = (uint32_t)mFrameNumber;
    aoParam.samples = (uint32_t)mSamples;
    aoParam.rayLen = mRenderConfig.rayLen;
    mAO->setParams(aoParam);

    AccumulationParam accParam{};
    accParam.alpha = mRenderConfig.accAlpha;
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
    accParam.isPt = 0;

    AccumulationParam accParamPT = accParam;
    AccumulationParam accParamAO = accParam;
    accParamAO.alpha = mRenderConfig.accAOAlpha;

    accParamPT.isPt = 1;
    if (needResetPt)
    {
        mView[imageIndex]->mPtIteration = 0;
    }
    else
    {
        ++mView[imageIndex]->mPtIteration;
    }
    accParamPT.iteration = mView[imageIndex]->mPtIteration;

    mAccumulationShadows->setParams(accParam);
    mAccumulationAO->setParams(accParamAO);
    mAccumulationPathTracer->setParams(accParamPT);

    LtcParam ltcparams{};
    ltcparams.CameraPos = cam.getPosition();
    ltcparams.dimension = glm::int2(renderWidth, renderHeight);
    ltcparams.frameNumber = (uint32_t)mFrameNumber;
    ltcparams.lightsCount = (uint32_t)scene->getLights().size();
    mLtcPass->setParams(ltcparams);

    BilateralParam bilateralparams{};
    bilateralparams.dimension = glm::int2(renderWidth, renderHeight);
    bilateralparams.dipatchGridDim = (bilateralparams.dimension + glm::int2(15, 15)) / glm::int2(16, 16);
    bilateralparams.useSwizzleTid = (int)mRenderConfig.useSwizzleTid;
    bilateralparams.sigma = mRenderConfig.sigma;
    bilateralparams.sigmaNormal = mRenderConfig.sigmaNormal;
    bilateralparams.radius = mRenderConfig.radius;
    bilateralparams.zfar = cam.zfar;
    bilateralparams.znear = cam.znear;
    bilateralparams.maxR = mRenderConfig.maxR;
    bilateralparams.invProj = glm::inverse(cam.getPerspective());
    mBilateralFilter->setParams(bilateralparams);

    BilateralParam bilateralAOparams{};
    bilateralAOparams.dipatchGridDim = (bilateralparams.dimension + glm::int2(15, 15)) / glm::int2(16, 16);
    bilateralAOparams.useSwizzleTid = (int)mRenderConfig.useSwizzleTid;
    bilateralAOparams.dimension = glm::int2(renderWidth, renderHeight);
    bilateralAOparams.sigma = mRenderConfig.sigmaAO;
    bilateralAOparams.sigmaNormal = mRenderConfig.sigmaAONormal;
    bilateralAOparams.radius = mRenderConfig.radiusAO;
    bilateralAOparams.zfar = cam.zfar;
    bilateralAOparams.znear = cam.znear;
    bilateralAOparams.maxR = mRenderConfig.maxRAO;
    bilateralAOparams.invProj = glm::inverse(cam.getPerspective());
    mAOBilateralFilter->setParams(bilateralAOparams);

    if (needReloadScene && releaseSceneAfterFrames == 0)
    {
        mTexManager->delTexturesFromQueue();
        delete toRemoveSceneData;
        needReloadScene = false;
    }
    if (needReloadScene)
    {
        --releaseSceneAfterFrames;
    }

    if (needReloadBVH && releaseBVHAfterFrames == 0)
    {
        mResManager->destroyBuffer(toRemoveBVH);
        needReloadBVH = false;
    }
    if (needReloadBVH)
    {
        --releaseBVHAfterFrames;
    }

    glfwSetWindowTitle(mWindow, (std::string("NeVK") + " [" + std::to_string(msPerFrame) + " ms]").c_str());

    VkCommandBuffer& cmd = getFrameData(imageIndex).cmdBuffer;
    vkResetCommandBuffer(cmd, 0);

    VkCommandBufferBeginInfo cmdBeginInfo = {};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.pNext = nullptr;
    cmdBeginInfo.pInheritanceInfo = nullptr;
    cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    vkBeginCommandBuffer(cmd, &cmdBeginInfo);

    // upload data
    {
        const std::vector<Mesh>& meshes = scene->getMeshes();
        const std::vector<nevk::Instance>& sceneInstances = scene->getInstances();
        mCurrentSceneRenderData->mInstanceCount = (uint32_t)sceneInstances.size();
        Buffer* stagingBuffer = mUploadBuffer[frameIndex];
        void* stagingBufferMemory = mResManager->getMappedMemory(stagingBuffer);
        size_t stagingBufferOffset = 0;
        bool needBarrier = false;
        if (!sceneInstances.empty())
        {
            std::vector<InstanceConstants> instanceConsts;
            instanceConsts.resize(sceneInstances.size());
            for (uint32_t i = 0; i < sceneInstances.size(); ++i)
            {
                instanceConsts[i].materialId = sceneInstances[i].mMaterialId;
                instanceConsts[i].objectToWorld = sceneInstances[i].transform;
                instanceConsts[i].worldToObject = glm::inverse(sceneInstances[i].transform);
                instanceConsts[i].normalMatrix = glm::inverse(glm::transpose(sceneInstances[i].transform));

                const uint32_t currentMeshId = sceneInstances[i].mMeshId;
                instanceConsts[i].indexOffset = meshes[currentMeshId].mIndex;
                instanceConsts[i].indexCount = meshes[currentMeshId].mCount;
            }
            size_t bufferSize = sizeof(InstanceConstants) * sceneInstances.size();
            memcpy(stagingBufferMemory, instanceConsts.data(), bufferSize);

            VkBufferCopy copyRegion{};
            copyRegion.size = bufferSize;
            copyRegion.dstOffset = 0;
            copyRegion.srcOffset = stagingBufferOffset;
            vkCmdCopyBuffer(cmd, mResManager->getVkBuffer(stagingBuffer), mResManager->getVkBuffer(mCurrentSceneRenderData->mInstanceBuffer), 1, &copyRegion);

            stagingBufferOffset += bufferSize;
            needBarrier = true;
        }
        const std::vector<nevk::Scene::Light>& lights = scene->getLights();
        if (!lights.empty())
        {
            size_t bufferSize = sizeof(nevk::Scene::Light) * MAX_LIGHT_COUNT;
            memcpy((void*)((char*)stagingBufferMemory + stagingBufferOffset), lights.data(), lights.size() * sizeof(nevk::Scene::Light));

            VkBufferCopy copyRegion{};
            copyRegion.size = bufferSize;
            copyRegion.dstOffset = 0;
            copyRegion.srcOffset = stagingBufferOffset;
            vkCmdCopyBuffer(cmd, mResManager->getVkBuffer(stagingBuffer), mResManager->getVkBuffer(mCurrentSceneRenderData->mLightsBuffer), 1, &copyRegion);

            stagingBufferOffset += bufferSize;
            needBarrier = true;
        }

        const std::vector<Material>& materials = scene->getMaterials();
        if (!materials.empty())
        {
            size_t bufferSize = sizeof(Material) * MAX_LIGHT_COUNT;
            memcpy((void*)((char*)stagingBufferMemory + stagingBufferOffset), materials.data(), materials.size() * sizeof(Material));

            VkBufferCopy copyRegion{};
            copyRegion.size = bufferSize;
            copyRegion.dstOffset = 0;
            copyRegion.srcOffset = stagingBufferOffset;
            vkCmdCopyBuffer(cmd, mResManager->getVkBuffer(stagingBuffer), mResManager->getVkBuffer(mCurrentSceneRenderData->mMaterialBuffer), 1, &copyRegion);

            stagingBufferOffset += bufferSize;
            needBarrier = true;
        }


        if (needBarrier)
        {
            VkMemoryBarrier memoryBarrier = {};
            memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            memoryBarrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
            memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;

            vkCmdPipelineBarrier(cmd,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT, // srcStageMask
                                 VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, // dstStageMask
                                 0,
                                 1, // memoryBarrierCount
                                 &memoryBarrier, // pMemoryBarriers
                                 0, nullptr, 0, nullptr);
        }
    }

    assert(mView[imageIndex]);

    mGbufferPass.record(cmd, mResManager->getVkBuffer(mCurrentSceneRenderData->mVertexBuffer), mResManager->getVkBuffer(mCurrentSceneRenderData->mIndexBuffer), *mScene, renderWidth, renderHeight, imageIndex, getActiveCameraIndex());

    const GBuffer& gbuffer = *mView[imageIndex]->gbuffer;

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

    Image* finalAOImage = mView[imageIndex]->mAOImage;
    Image* finalRtImage = mView[imageIndex]->mRtShadowImage;
    Image* finalPathTracerImage = mView[imageIndex]->mPathTracerImage;
    Image* finalImage = nullptr;

    // Path Tracer
    if (mRenderConfig.enablePathTracer)
    {
        recordBarrier(cmd, mResManager->getVkImage(mView[imageIndex]->mPathTracerImage), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
                      VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        mPathTracer->execute(cmd, renderWidth, renderHeight, imageIndex);
        recordBarrier(cmd, mResManager->getVkImage(mView[imageIndex]->mPathTracerImage), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                      VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

        finalPathTracerImage = mView[imageIndex]->mPathTracerImage;

        if (mRenderConfig.enablePathTracerAcc && mPrevView)
        {
            // Accumulation pass
            Image* accHist = mPrevView->mAccumulationPathTracerImage;
            Image* accOut = mView[imageIndex]->mAccumulationPathTracerImage;

            mAccumulationPathTracer->setHistoryTexture4(accHist);
            mAccumulationPathTracer->setOutputTexture4(accOut);

            recordBarrier(cmd, mResManager->getVkImage(accOut), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL,
                          VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
            mAccumulationPathTracer->execute(cmd, renderWidth, renderHeight, imageIndex);
            recordBarrier(cmd, mResManager->getVkImage(accOut), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                          VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

            finalPathTracerImage = accOut;
        }
    }
    else
    {
        // LTC
        recordBarrier(cmd, mResManager->getVkImage(mView[imageIndex]->mLtcOutputImage), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
                      VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        mLtcPass->execute(cmd, renderWidth, renderHeight, imageIndex);
        recordBarrier(cmd, mResManager->getVkImage(mView[imageIndex]->mLtcOutputImage), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                      VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

        // Reflections
        if (mRenderConfig.enableReflections)
        {
            recordBarrier(cmd, mResManager->getVkImage(mView[imageIndex]->mReflectionImage), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
                          VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
            mReflection->execute(cmd, renderWidth, renderHeight, imageIndex);
            recordBarrier(cmd, mResManager->getVkImage(mView[imageIndex]->mReflectionImage), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                          VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
        }

        // Shadows
        if (mRenderConfig.enableShadows)
        {
            recordBarrier(cmd, mResManager->getVkImage(mView[imageIndex]->mRtShadowImage), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
                          VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
            mRtShadow->execute(cmd, renderWidth, renderHeight, imageIndex);
            recordBarrier(cmd, mResManager->getVkImage(mView[imageIndex]->mRtShadowImage), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                          VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

            finalRtImage = mView[imageIndex]->mRtShadowImage;

            if (mRenderConfig.enableShadowsAcc && mPrevView)
            {
                // Accumulation pass
                Image* accHist = mPrevView->mAccumulationImages;
                Image* accOut = mView[imageIndex]->mAccumulationImages;

                mAccumulationShadows->setHistoryTexture1(accHist);
                mAccumulationShadows->setOutputTexture1(accOut);

                recordBarrier(cmd, mResManager->getVkImage(accOut), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL,
                              VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
                mAccumulationShadows->execute(cmd, renderWidth, renderHeight, imageIndex);
                recordBarrier(cmd, mResManager->getVkImage(accOut), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                              VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

                finalRtImage = accOut;
            }

            if (mRenderConfig.enableFilter)
            {
                // Bilateral Filter
                BilateralResourceDesc desc{};
                desc.gbuffer = mView[imageIndex]->gbuffer;
                desc.result = mView[imageIndex]->mBilateralOutputImage;
                desc.variance = mView[imageIndex]->mBilateralVarianceOutputImage;
                desc.input = finalRtImage;
                mBilateralFilter->setResources(desc);

                recordBarrier(cmd, mResManager->getVkImage(mView[imageIndex]->mBilateralOutputImage), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
                              VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
                recordBarrier(cmd, mResManager->getVkImage(mView[imageIndex]->mBilateralVarianceOutputImage), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
                              VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

                mBilateralFilter->execute(cmd, renderWidth, renderHeight, imageIndex);

                recordBarrier(cmd, mResManager->getVkImage(mView[imageIndex]->mBilateralVarianceOutputImage), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                              VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
                recordBarrier(cmd, mResManager->getVkImage(mView[imageIndex]->mBilateralOutputImage), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                              VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

                finalRtImage = mView[imageIndex]->mBilateralOutputImage;
            }
        }

        // Ambient occlusion
        if (mRenderConfig.enableAO)
        {
            recordBarrier(cmd, mResManager->getVkImage(mView[imageIndex]->mAOImage), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
                          VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
            mAO->execute(cmd, renderWidth, renderHeight, imageIndex);
            recordBarrier(cmd, mResManager->getVkImage(mView[imageIndex]->mAOImage), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                          VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

            finalAOImage = mView[imageIndex]->mAOImage;

            if (mRenderConfig.enableAOAcc && mPrevView)
            {
                // Accumulation AO pass
                Image* accHistAO = mPrevView->mAccumulationAOImages;
                Image* accOutAO = mView[imageIndex]->mAccumulationAOImages;

                mAccumulationAO->setHistoryTexture1(accHistAO);
                mAccumulationAO->setOutputTexture1(accOutAO);

                recordBarrier(cmd, mResManager->getVkImage(accOutAO), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL,
                              VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
                mAccumulationAO->execute(cmd, renderWidth, renderHeight, imageIndex);
                recordBarrier(cmd, mResManager->getVkImage(accOutAO), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                              VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

                finalAOImage = accOutAO;
            }
            if (mRenderConfig.enableAOFilter)
            {
                // Bilateral Filter AO
                BilateralResourceDesc desc{};
                desc.gbuffer = mView[imageIndex]->gbuffer;
                desc.result = mView[imageIndex]->mAOBilateralOutputImage;
                desc.variance = mView[imageIndex]->mAOBilateralVarianceOutputImage;
                desc.input = finalAOImage;
                mAOBilateralFilter->setResources(desc);

                recordBarrier(cmd, mResManager->getVkImage(mView[imageIndex]->mAOBilateralOutputImage), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
                              VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
                recordBarrier(cmd, mResManager->getVkImage(mView[imageIndex]->mAOBilateralVarianceOutputImage), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
                              VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

                mAOBilateralFilter->execute(cmd, renderWidth, renderHeight, imageIndex);

                recordBarrier(cmd, mResManager->getVkImage(mView[imageIndex]->mAOBilateralVarianceOutputImage), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                              VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
                recordBarrier(cmd, mResManager->getVkImage(mView[imageIndex]->mAOBilateralOutputImage), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                              VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

                finalAOImage = mView[imageIndex]->mAOBilateralOutputImage;
            }
        }
    }

    if (mScene->mDebugViewSettings != Scene::DebugView::eNone && !mRenderConfig.enablePathTracer)
    {
        mDebugImages.shadow = finalRtImage;
        mDebugImages.LTC = mView[imageIndex]->mLtcOutputImage;
        mDebugImages.normal = mView[imageIndex]->gbuffer->normal;
        mDebugImages.debug = mView[imageIndex]->gbuffer->debug;
        mDebugImages.AO = finalAOImage;
        mDebugImages.motion = mView[imageIndex]->gbuffer->motion;
        mDebugImages.variance = mView[imageIndex]->mBilateralVarianceOutputImage;
        mDebugImages.reflection = mView[imageIndex]->mReflectionImage;
        mDebugImages.pathTracer = finalPathTracerImage;

        mDebugParams.dimension.x = renderWidth;
        mDebugParams.dimension.y = renderHeight;
        mDebugParams.debugView = (uint32_t)mScene->mDebugViewSettings;
        mDebugView->setParams(mDebugParams);
        mDebugView->setInputTexture(mDebugImages);
        mDebugView->execute(cmd, finalWidth, finalHeight, imageIndex);
        finalImage = mView[imageIndex]->textureDebugViewImage;
    }
    else
    {
        Image* tmpImage = nullptr;
        if (mRenderConfig.enablePathTracer)
        {
            tmpImage = finalPathTracerImage;
        }
        else
        {
            if (!mRenderConfig.enableShadows)
            {
                recordBarrier(cmd, mResManager->getVkImage(finalRtImage), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                              VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
            }
            if (!mRenderConfig.enableAO)
            {
                recordBarrier(cmd, mResManager->getVkImage(finalAOImage), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                              VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
            }
            if (!mRenderConfig.enableReflections)
            {
                recordBarrier(cmd, mResManager->getVkImage(mView[imageIndex]->mReflectionImage), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                              VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
            }

            mCompositionImages.shadow = finalRtImage;
            mCompositionImages.LTC = mView[imageIndex]->mLtcOutputImage;
            mCompositionImages.AO = finalAOImage;
            mCompositionImages.reflections = mView[imageIndex]->mReflectionImage;

            // compose final image ltc + reflections + rtshadow + ao
            recordBarrier(cmd, mResManager->getVkImage(mView[imageIndex]->textureCompositionImage), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
                          VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

            mCompositionParam.dimension.x = renderWidth;
            mCompositionParam.dimension.y = renderHeight;
            mCompositionParam.enableShadows = (int32_t)mRenderConfig.enableShadows;
            mCompositionParam.enableAO = (int32_t)mRenderConfig.enableAO;
            mCompositionParam.enableReflections = (int32_t)mRenderConfig.enableReflections;
            mComposition->setParams(mCompositionParam);
            mComposition->setInputTexture(mCompositionImages);
            mComposition->execute(cmd, renderWidth, renderHeight, imageIndex);
            recordBarrier(cmd, mResManager->getVkImage(mView[imageIndex]->textureCompositionImage), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                          VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
            tmpImage = mView[imageIndex]->textureCompositionImage;
        }

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
        if (mRenderConfig.enableUpscale)
        {
            recordBarrier(cmd, mResManager->getVkImage(finalImage), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                          VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
            mUpscalePassParam.dimension.x = finalWidth;
            mUpscalePassParam.dimension.y = finalHeight;
            mUpscalePassParam.invDimension.x = 1.0f / (float)finalWidth;
            mUpscalePassParam.invDimension.y = 1.0f / (float)finalHeight;
            mUpscalePass->setParams(mUpscalePassParam);
            mUpscalePass->setInputTexture(mResManager->getView(finalImage));
            mUpscalePass->execute(cmd, finalWidth, finalHeight, imageIndex);
            recordBarrier(cmd, mResManager->getVkImage(finalImage), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL,
                          VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
            finalImage = mView[imageIndex]->textureUpscaleImage;
        }
    }

    // Copy to swapchain image
    {

        recordBarrier(cmd, mResManager->getVkImage(finalImage), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                      VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

        recordBarrier(cmd, mSwapChainImages[imageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

        VkOffset3D blitSize{};
        blitSize.x = finalWidth;
        blitSize.y = finalHeight;
        blitSize.z = 1;
        VkImageBlit imageBlitRegion{};
        imageBlitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlitRegion.srcSubresource.layerCount = 1;
        imageBlitRegion.srcOffsets[1] = blitSize;
        imageBlitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlitRegion.dstSubresource.layerCount = 1;
        imageBlitRegion.dstOffsets[1] = blitSize;
        vkCmdBlitImage(cmd, mResManager->getVkImage(finalImage), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, mSwapChainImages[imageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlitRegion, VK_FILTER_NEAREST);

        recordBarrier(cmd, mResManager->getVkImage(finalImage), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL,
                      VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

        recordBarrier(cmd, mSwapChainImages[imageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                      VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    }

    mUi.render(cmd, imageIndex);

    // copy current depth from gbuffer to prev gbuffer
    {
        recordBarrier(cmd, mResManager->getVkImage(mView[imageIndex]->gbuffer->depth), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                      VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
        recordBarrier(cmd, mResManager->getVkImage(mView[imageIndex]->prevDepth), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                      VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
        VkImageCopy region{};
        region.extent.width = renderWidth;
        region.extent.height = renderHeight;
        region.extent.depth = 1;

        region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        region.srcSubresource.mipLevel = 0;
        region.srcSubresource.layerCount = 1;
        region.srcSubresource.baseArrayLayer = 0;

        region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        region.dstSubresource.mipLevel = 0;
        region.dstSubresource.layerCount = 1;
        region.dstSubresource.baseArrayLayer = 0;

        vkCmdCopyImage(cmd, mResManager->getVkImage(mView[imageIndex]->gbuffer->depth), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, mResManager->getVkImage(mView[imageIndex]->prevDepth), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        recordBarrier(cmd, mResManager->getVkImage(mView[imageIndex]->prevDepth), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                      VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
    }

    // assign prev resources for next frame rendering
    mPrevView = mView[imageIndex];

    if (vkEndCommandBuffer(cmd) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { currFrame.imageAvailable };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;

    VkSemaphore signalSemaphores[] = { currFrame.renderFinished };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(mDevice, 1, &currFrame.inFlightFence);

    if (vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, currFrame.inFlightFence) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { mSwapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(mPresentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
    {
        framebufferResized = false;
        recreateSwapChain();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }

    ++mFrameNumber;
}

VkSurfaceFormatKHR Render::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR Render::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Render::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != UINT32_MAX)
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(mWindow, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

SwapChainSupportDetails Render::querySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, mSurface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formatCount, nullptr);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, mSurface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, mSurface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

bool Render::isDeviceSuitable(VkPhysicalDevice device)
{
    QueueFamilyIndices indices = findQueueFamilies(device);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

bool Render::checkDeviceExtensionSupport(VkPhysicalDevice device)
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

QueueFamilyIndices Render::findQueueFamilies(VkPhysicalDevice device)
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

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mSurface, &presentSupport);

        if (presentSupport)
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

std::vector<const char*> Render::getRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    return extensions;
}

bool Render::checkValidationLayerSupport()
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
