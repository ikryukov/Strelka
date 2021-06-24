#include "render.h"

#include "debugUtils.h"

#include <chrono>
#include <filesystem>

namespace fs = std::filesystem;

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


void Render::initVulkan()
{
    createInstance();
    setupDebugMessenger();
    if (enableValidationLayers)
    {
        nevk::debug::setupDebug(mInstance);
    }
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();

    // load shaders
    uint32_t csId = mShaderManager.loadShader("shaders/compute.hlsl", "computeMain", nevk::ShaderManager::Stage::eCompute);
    mShaderManager.getShaderCode(csId, csShaderCode, csShaderCodeSize);

    uint32_t shId = mShaderManager.loadShader("shaders/shadowmap.hlsl", "vertexMain", nevk::ShaderManager::Stage::eVertex);
    mShaderManager.getShaderCode(shId, shShaderCode, shShaderCodeSize);

    uint32_t pbrVertId = mShaderManager.loadShader("shaders/pbr.hlsl", "vertexMain", nevk::ShaderManager::Stage::eVertex);
    uint32_t pbrFragId = mShaderManager.loadShader("shaders/pbr.hlsl", "fragmentMain", nevk::ShaderManager::Stage::ePixel);

    mShaderManager.getShaderCode(pbrVertId, pbrVertShaderCode, pbrVertShaderCodeSize);
    mShaderManager.getShaderCode(pbrFragId, pbrFragShaderCode, pbrFragShaderCodeSize);

    uint32_t simpleVertId = mShaderManager.loadShader("shaders/simple.hlsl", "vertexMain", nevk::ShaderManager::Stage::eVertex);
    uint32_t simpleFragId = mShaderManager.loadShader("shaders/simple.hlsl", "fragmentMain", nevk::ShaderManager::Stage::ePixel);

    mShaderManager.getShaderCode(simpleVertId, simpleVertShaderCode, simpleVertShaderCodeSize);
    mShaderManager.getShaderCode(simpleFragId, simpleFragShaderCode, simpleFragShaderCodeSize);

    createDescriptorPool();
    createCommandPool();

    mResManager = new nevk::ResourceManager(mDevice, mPhysicalDevice, getCurrentFrameData().cmdPool, mGraphicsQueue);
    mTexManager = new nevk::TextureManager(mDevice, mPhysicalDevice, mResManager);

    createImageViews();
    createCommandBuffers();
    createSyncObjects();

    createDepthResources();

    createDefaultScene();

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

    //mComputePass.setOutputImageView(textureCompImageView);
    //mComputePass.setInImageView();
    //mComputePass.setTextureSampler(mTexManager->textureSampler);
    //mComputePass.init(device, csShaderCode, csShaderCodeSize, descriptorPool, mResManager);
}

void Render::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto app = reinterpret_cast<Render*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
    nevk::Scene* scene = app->getScene();
    scene->updateCameraParams(width, height);
}

inline void Render::keyCallback(GLFWwindow* window, int key, [[maybe_unused]] int scancode, int action, [[maybe_unused]] int mods)
{
    auto app = reinterpret_cast<Render*>(glfwGetWindowUserPointer(window));
    nevk::Scene* scene = app->getScene();
    Camera& camera = scene->getCamera();

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
    Camera& camera = scene->getCamera();
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
    Camera& camera = scene->getCamera();
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
    Camera& mCamera = mScene->getCamera();

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
    }

    vkDeviceWaitIdle(mDevice);
}

void Render::cleanupSwapChain()
{
    vkDestroyImageView(mDevice, depthImageView, nullptr);
    vkDestroyImage(mDevice, depthImage, nullptr);
    vkFreeMemory(mDevice, depthImageMemory, nullptr);

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

    mPbrPass.onDestroy();
    mPass.onDestroy();
    mDepthPass.onDestroy();
    mUi.onDestroy();

    vkDestroyDescriptorPool(mDevice, mDescriptorPool, nullptr);

    mTexManager->textureDestroy();

    vkDestroyImageView(mDevice, textureCompImageView, nullptr);
    vkDestroyImage(mDevice, textureCompImage, nullptr);
    vkFreeMemory(mDevice, textureCompImageMemory, nullptr);

    // different for default and normal scene ?
    vkDestroyImageView(mDevice, shadowImageView, nullptr);
    vkDestroyImage(mDevice, shadowImage, nullptr);
    vkFreeMemory(mDevice, shadowImageMemory, nullptr);

    isDefaultScene = true;
    freeSceneData();
    isDefaultScene = false;
    freeSceneData();

    for (FrameData& fd : mFramesData)
    {
        vkDestroySemaphore(mDevice, fd.renderFinished, nullptr);
        vkDestroySemaphore(mDevice, fd.imageAvailable, nullptr);
        vkDestroyFence(mDevice, fd.inFlightFence, nullptr);

        vkDestroyCommandPool(mDevice, fd.cmdPool, nullptr);
    }

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
    createDepthResources();

    mPass.onResize(swapChainImageViews, depthImageView, width, height);
    mPbrPass.onResize(swapChainImageViews, depthImageView, width, height);
    mUi.onResize(swapChainImageViews, width, height);

    nevk::Scene* scene = getScene();
    Camera& camera = scene->getCamera();
    camera.setPerspective(45.0f, (float)swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10000.0f);
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

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = &indexingFeatures;
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
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

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

void Render::createDepthResources()
{
    VkFormat depthFormat = findDepthFormat();

    mResManager->createImage(swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
    depthImageView = mTexManager->createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
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

void Render::loadModel(nevk::ModelLoader& testmodel, nevk::Scene& scene)
{
    isPBR = true;
    bool res = testmodel.loadModelGltf(MODEL_PATH, scene);
    // bool res = testmodel.loadModel(MODEL_PATH, MTL_PATH, *mScene);
    if (!res && !isDefaultScene)
    {
        return;
    }
    Camera& camera = scene.getCamera();
    camera.type = Camera::CameraType::firstperson;
    camera.setPerspective(45.0f, (float)swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10000.0f);
    camera.rotationSpeed = 0.05f;
    camera.movementSpeed = 5.0f;
    //camera.setPosition({ -1.0f, 3.0f, 8.0f });
    camera.setPosition({ 0.0f, 0.0f, 10.0f });
    camera.setRotation(glm::quat({ 1.0f, 0.0f, 0.0f, 0.0f }));
}

void Render::createVertexBuffer(nevk::Scene& scene)
{
    SceneData* sceneData = getSceneData();
    std::vector<nevk::Scene::Vertex>& sceneVertices = scene.getVertices();
    VkDeviceSize bufferSize = sizeof(nevk::Scene::Vertex) * sceneVertices.size();
    if (bufferSize == 0)
    {
        return;
    }
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    mResManager->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(mDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, sceneVertices.data(), (size_t)bufferSize);
    vkUnmapMemory(mDevice, stagingBufferMemory);

    mResManager->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sceneData->mVertexBuffer, sceneData->mVertexBufferMemory);

    mResManager->copyBuffer(stagingBuffer, sceneData->mVertexBuffer, bufferSize);

    vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
    vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
}

void Render::createMaterialBuffer(nevk::Scene& scene)
{
    SceneData* sceneData = getSceneData();
    std::vector<nevk::Scene::Material>& sceneMaterials = scene.getMaterials();

    VkDeviceSize bufferSize = sizeof(nevk::Scene::Material) * sceneMaterials.size();
    if (bufferSize == 0)
    {
        return;
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    mResManager->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(mDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, sceneMaterials.data(), (size_t)bufferSize);
    vkUnmapMemory(mDevice, stagingBufferMemory);

    mResManager->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sceneData->mMaterialBuffer, sceneData->mMaterialBufferMemory);

    mResManager->copyBuffer(stagingBuffer, sceneData->mMaterialBuffer, bufferSize);

    vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
    vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
}

void Render::createIndexBuffer(nevk::Scene& scene)
{
    SceneData* sceneData = getSceneData();
    std::vector<uint32_t>& sceneIndices = scene.getIndices();
    sceneData->mIndicesCount = (uint32_t)sceneIndices.size();
    VkDeviceSize bufferSize = sizeof(uint32_t) * sceneIndices.size();
    if (bufferSize == 0)
    {
        return;
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    mResManager->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(mDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, sceneIndices.data(), (size_t)bufferSize);
    vkUnmapMemory(mDevice, stagingBufferMemory);

    mResManager->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sceneData->mIndexBuffer, sceneData->mIndexBufferMemory);

    mResManager->copyBuffer(stagingBuffer, sceneData->mIndexBuffer, bufferSize);

    vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
    vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
}

void Render::createDescriptorPool()
{
    VkDescriptorPoolSize pool_sizes[] = {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
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

void Render::recordCommandBuffer(VkCommandBuffer& cmd, uint32_t imageIndex)
{
    SceneData* sceneData = getSceneData();
    nevk::Scene* scene = getScene();

    mDepthPass.record(cmd, sceneData->mVertexBuffer, sceneData->mIndexBuffer, *scene, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, imageIndex);

    if (isPBR)
    {
        mPbrPass.record(cmd, sceneData->mVertexBuffer, sceneData->mIndexBuffer, *scene, swapChainExtent.width, swapChainExtent.height, imageIndex);
    }
    else
    {
        mPass.record(cmd, sceneData->mVertexBuffer, sceneData->mIndexBuffer, *scene, swapChainExtent.width, swapChainExtent.height, imageIndex);
    }

    //mComputePass.record(cmd, swapChainExtent.width, swapChainExtent.height, imageIndex);
    mUi.render(cmd, imageIndex);
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

// dumbest destroyer ever, that doesnt even work properly, but i can pretend as it is and the only dumb here is me.
void Render::freeSceneData()
{
    SceneData* sceneData = getSceneData();

    vkDestroyBuffer(mDevice, sceneData->mIndexBuffer, nullptr); // seems like im trying to update empty scene
    vkFreeMemory(mDevice, sceneData->mIndexBufferMemory, nullptr);

    vkDestroyBuffer(mDevice, sceneData->mMaterialBuffer, nullptr);
    vkFreeMemory(mDevice, sceneData->mMaterialBufferMemory, nullptr);

    vkDestroyBuffer(mDevice, sceneData->mVertexBuffer, nullptr);
    vkFreeMemory(mDevice, sceneData->mVertexBufferMemory, nullptr);
}

// load into normal scene
void Render::loadScene(const std::string& modelPath)
{
    // freeSceneData();
    // different managers for default and normal scene ?
    if (modelLoader != nullptr)
        delete modelLoader; // ? deleting null pointer has no effect anyway....
    if (mScene != nullptr)
        delete mScene;
    if (mTexManager != nullptr)
        delete mTexManager;

    mScene = new nevk::Scene;
    mTexManager = new nevk::TextureManager(mDevice, mPhysicalDevice, mResManager);
    modelLoader = new nevk::ModelLoader(mTexManager);
    isDefaultScene = false;
    MODEL_PATH = modelPath;
    loadModel(*modelLoader, *mScene);

    initPasses();
}

void Render::setDescriptors() // set on default from drawframe, уыыуу
{
    SceneData* sceneData = &defaultSceneData;
    {
        mPbrPass.setFrameBufferFormat(swapChainImageFormat);
        mPbrPass.setDepthBufferFormat(findDepthFormat());
        mPbrPass.setTextureImageView(mTexManager->textureImageView);
        mPbrPass.setTextureSampler(mTexManager->textureSampler);
        mPbrPass.setShadowImageView(shadowImageView);
        mPbrPass.setShadowSampler(mTexManager->shadowSampler);
        mPbrPass.setMaterialBuffer(sceneData->mMaterialBuffer);
    }
    {
        mPass.setFrameBufferFormat(swapChainImageFormat);
        mPass.setDepthBufferFormat(findDepthFormat());
        mPass.setTextureImageView(mTexManager->textureImageView);
        mPass.setTextureSampler(mTexManager->textureSampler);
        mPass.setShadowImageView(shadowImageView);
        mPass.setShadowSampler(mTexManager->shadowSampler);
        mPass.setMaterialBuffer(sceneData->mMaterialBuffer);
    }
}

void Render::initPasses()
{
    SceneData* sceneData = getSceneData();
    nevk::Scene* scene = getScene();
    createMaterialBuffer(*scene);
    {
        mResManager->createImage(SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, findDepthFormat(),
                                 VK_IMAGE_TILING_OPTIMAL,
                                 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                 shadowImage, shadowImageMemory);
        shadowImageView = mTexManager->createImageView(shadowImage, findDepthFormat(), VK_IMAGE_ASPECT_DEPTH_BIT);

        mDepthPass.init(mDevice, enableValidationLayers, shShaderCode, shShaderCodeSize, mDescriptorPool, mResManager, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);
        mDepthPass.createFrameBuffers(shadowImageView, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);
    }

    mTexManager->createShadowSampler();
    mTexManager->createTextureSampler();

    // PBR
    {
        mPbrPass.setFrameBufferFormat(swapChainImageFormat);
        mPbrPass.setDepthBufferFormat(findDepthFormat());
        mPbrPass.setTextureImageView(mTexManager->textureImageView);
        mPbrPass.setTextureSampler(mTexManager->textureSampler);
        mPbrPass.setShadowImageView(shadowImageView);
        mPbrPass.setShadowSampler(mTexManager->shadowSampler);
        mPbrPass.setMaterialBuffer(sceneData->mMaterialBuffer);
        mPbrPass.init(mDevice, enableValidationLayers, pbrVertShaderCode, pbrVertShaderCodeSize, pbrFragShaderCode, pbrFragShaderCodeSize, mDescriptorPool, mResManager, swapChainExtent.width, swapChainExtent.height);

        mPbrPass.createFrameBuffers(swapChainImageViews, depthImageView, swapChainExtent.width, swapChainExtent.height);
    }
    // Simple
    {
        mPass.setFrameBufferFormat(swapChainImageFormat);
        mPass.setDepthBufferFormat(findDepthFormat());
        mPass.setTextureImageView(mTexManager->textureImageView);
        mPass.setTextureSampler(mTexManager->textureSampler);
        mPass.setShadowImageView(shadowImageView);
        mPass.setShadowSampler(mTexManager->shadowSampler);
        mPass.setMaterialBuffer(sceneData->mMaterialBuffer);
        mPass.init(mDevice, enableValidationLayers, simpleVertShaderCode, simpleVertShaderCodeSize, simpleFragShaderCode, simpleFragShaderCodeSize, mDescriptorPool, mResManager, swapChainExtent.width, swapChainExtent.height);

        mPass.createFrameBuffers(swapChainImageViews, depthImageView, swapChainExtent.width, swapChainExtent.height);
    }

    mResManager->createImage(swapChainExtent.width, swapChainExtent.height, VK_FORMAT_R32G32B32A32_SFLOAT,
                             VK_IMAGE_TILING_OPTIMAL,
                             VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                             textureCompImage, textureCompImageMemory);
    mTexManager->transitionImageLayout(textureCompImage, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
    textureCompImageView = mTexManager->createImageView(textureCompImage, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);

    createIndexBuffer(*scene);
    createVertexBuffer(*scene);
}

// set default scene
void Render::createDefaultScene()
{
    mDefaultScene = new nevk::Scene;

    modelLoader = new nevk::ModelLoader(mTexManager);

    loadModel(*modelLoader, *mDefaultScene);

    initPasses();
}

void Render::drawFrame()
{
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
    Camera& cam = scene->getCamera();

    cam.update((float)deltaTime);

    const glm::float4x4 lightSpaceMatrix = mDepthPass.computeLightSpaceMatrix((glm::float3&)scene->mLightPosition);

    mDepthPass.updateUniformBuffer(frameIndex, lightSpaceMatrix);
    mPass.updateUniformBuffer(frameIndex, lightSpaceMatrix, *scene);
    mPbrPass.updateUniformBuffer(frameIndex, lightSpaceMatrix, *scene);

    std::string newModelPath;
    std::string savedPath;
    mUi.updateUI(*scene, mDepthPass, msPerFrame, newModelPath);

    if (!newModelPath.empty() && fs::exists(newModelPath))
    { //todo last path != new path
        if (!isDefaultScene) // if we reload non-default scene
        {
            isDefaultScene = true;
            setDescriptors(); // set descriptors on default scene
            needReload = true; // need to remove last scene data
            savedPath = newModelPath;
        }
        else {
         // if the previous one was default we dont need to remove anything just load
        loadScene(newModelPath);
        }
    }

    if (needReload)
        ++countFrames;
    if (needReload && countFrames == 3)
    {
        isDefaultScene = false;
        freeSceneData(); // remove past non-default
        loadScene(savedPath); // load new scene
        countFrames = 0;
        needReload = false;
    }
    std::cout << countFrames << std::endl;

    glfwSetWindowTitle(mWindow, (std::string("NeVK") + " [" + std::to_string(msPerFrame) + " ms]").c_str());

    VkCommandBuffer& cmdBuff = getFrameData(imageIndex).cmdBuffer;
    vkResetCommandBuffer(cmdBuff, 0);

    VkCommandBufferBeginInfo cmdBeginInfo = {};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.pNext = nullptr;
    cmdBeginInfo.pInheritanceInfo = nullptr;
    cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    vkBeginCommandBuffer(cmdBuff, &cmdBeginInfo);

    recordCommandBuffer(cmdBuff, frameIndex);

    if (vkEndCommandBuffer(cmdBuff) != VK_SUCCESS)
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
    submitInfo.pCommandBuffers = &cmdBuff;

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

    mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
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
