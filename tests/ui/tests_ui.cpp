#define protected public
#define private public
#include <render/render.h>
#include <ui/ui.h>

#include <doctest.h>

Render initVK(){
    Render r;

    r.initWindow();
    r.createInstance();
    r.setupDebugMessenger();
    r.createSurface();
    r.pickPhysicalDevice();
    r.createLogicalDevice();
    r.createSwapChain();
    r.createImageViews();

    r.createDescriptorPool();
    r.createCommandPool();
    r.createCommandBuffers();
    r.createSyncObjects();

    return r;
}

TEST_CASE("test UI init")
{
    auto* mUi = new nevk::Ui();
    Render r = initVK();

    QueueFamilyIndices indicesFamily = r.findQueueFamilies(r.physicalDevice);
    ImGui_ImplVulkan_InitInfo init_info{};
    init_info.DescriptorPool = r.descriptorPool;
    init_info.Device = r.device;
    init_info.ImageCount = MAX_FRAMES_IN_FLIGHT;
    init_info.Instance = r.instance;
    init_info.MinImageCount = 2;
    init_info.PhysicalDevice = r.physicalDevice;
    init_info.Queue = r.graphicsQueue;
    init_info.QueueFamily = indicesFamily.graphicsFamily.value();

    bool init = mUi->init(init_info, r.swapChainImageFormat, r.window, r.mFramesData[0].cmdPool, r.mFramesData[0].cmdBuffer, r.swapChainExtent.width, r.swapChainExtent.height);
    bool fonts = mUi->uploadFonts(init_info, r.mFramesData[0].cmdPool, r.mFramesData[0].cmdBuffer);

    CHECK(init == true);
    CHECK(fonts == true);
}

TEST_CASE("test frame buffer creation")
{
    auto* mUi = new nevk::Ui();
    Render r = initVK();

    QueueFamilyIndices indicesFamily = r.findQueueFamilies(r.physicalDevice);
    ImGui_ImplVulkan_InitInfo init_info{};
    init_info.DescriptorPool = r.descriptorPool;
    init_info.Device = r.device;
    init_info.ImageCount = MAX_FRAMES_IN_FLIGHT;
    init_info.Instance = r.instance;
    init_info.MinImageCount = 2;
    init_info.PhysicalDevice = r.physicalDevice;
    init_info.Queue = r.graphicsQueue;
    init_info.QueueFamily = indicesFamily.graphicsFamily.value();

    mUi->init(init_info, r.swapChainImageFormat, r.window, r.mFramesData[0].cmdPool, r.mFramesData[0].cmdBuffer, r.swapChainExtent.width, r.swapChainExtent.height);
    bool frameBuf = mUi->createFrameBuffers(r.device, r.swapChainImageViews, r.swapChainExtent.width, r.swapChainExtent.height);
    CHECK(frameBuf == true);
}
