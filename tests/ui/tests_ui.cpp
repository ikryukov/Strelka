#include <render/render.h>
#include <ui/ui.h>

#include <doctest.h>

Render initVk()
{
    Render r;

    r.setWindow();
    r.setInstance();
    r.setDebugMessenger();
    r.setSurface();
    r.setPhysicalDevice();
    r.setLogicalDevice();
    r.setSwapChain();
    r.setDescriptorPool();
    r.setCommandPool();

    nevk::ResourceManager* mResManager = new nevk::ResourceManager(r.getDevice(), r.getPhysicalDevice(), r.getCurrentFrameData().cmdPool, r.getGraphicsQueue());
    nevk::TextureManager* mTexManager = new nevk::TextureManager(r.getDevice(), r.getPhysicalDevice(), mResManager);
    r.setTexManager(mTexManager);
    r.setResManager(mResManager);

    r.setImageViews();
    r.setCommandBuffers();
    r.setSyncObjects();

    return r;
}

TEST_CASE("test UI init")
{
    nevk::Ui* mUi = new nevk::Ui();
    Render r = initVk();

    QueueFamilyIndices indicesFamily = r.getQueueFamilies(r.getPhysicalDevice());
    ImGui_ImplVulkan_InitInfo init_info{};
    init_info.DescriptorPool = r.getDescriptorPool();
    init_info.Device = r.getDevice();
    init_info.ImageCount = MAX_FRAMES_IN_FLIGHT;
    init_info.Instance = r.getInstance();
    init_info.MinImageCount = 2;
    init_info.PhysicalDevice = r.getPhysicalDevice();
    init_info.Queue = r.getGraphicsQueue();
    init_info.QueueFamily = indicesFamily.graphicsFamily.value();

    bool init = mUi->init(init_info, r.getSwapChainImageFormat(), r.getWindow(), r.getFramesData()[0].cmdPool, r.getFramesData()[0].cmdBuffer, r.getSwapChainExtent().width, r.getSwapChainExtent().height);
    bool fonts = mUi->uploadFonts(init_info, r.getFramesData()[0].cmdPool, r.getFramesData()[0].cmdBuffer);

    CHECK(init == true);
    CHECK(fonts == true);

    mUi->init(init_info, r.getSwapChainImageFormat(), r.getWindow(), r.getFramesData()[0].cmdPool, r.getFramesData()[0].cmdBuffer, r.getSwapChainExtent().width, r.getSwapChainExtent().height);
    bool frameBuf = mUi->createFrameBuffers(r.getDevice(), r.getSwapChainImageViews(), r.getSwapChainExtent().width, r.getSwapChainExtent().height);

    CHECK(frameBuf == true);
}
