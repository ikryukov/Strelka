#include <render/render.h>
#include <ui/ui.h>

#include <doctest.h>

const std::string MODEL_PATH = "misc/Cube/Cube.gltf";

TEST_CASE("test UI init")
{
    Render r;
    r.MODEL_PATH = MODEL_PATH;
    r.HEIGHT = 600;
    r.WIDTH = 800;
    r.initWindow();
    r.initVulkan();
    r.getUi().onDestroy();

    nevk::Ui* mUi = new nevk::Ui();

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
    CHECK(init == true);

    bool frameBuf = mUi->createFrameBuffers(r.getDevice(), r.getSwapChainImageViews(), r.getSwapChainExtent().width, r.getSwapChainExtent().height);
    CHECK(frameBuf == true);

    r.setUi(*mUi);
    r.cleanup();
}
