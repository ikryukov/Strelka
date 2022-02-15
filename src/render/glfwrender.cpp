#include "glfwrender.h"

using namespace nevk;

void nevk::GLFWRender::init(int width, int height)
{
    mWindowWidth = width;
    mWindowHeight = height;

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    mWindow = glfwCreateWindow(mWindowWidth, mWindowHeight, "NeVK", nullptr, nullptr);
    glfwSetWindowUserPointer(mWindow, this);
    glfwSetFramebufferSizeCallback(mWindow, framebufferResizeCallback);
    glfwSetKeyCallback(mWindow, keyCallback);
    glfwSetMouseButtonCallback(mWindow, mouseButtonCallback);
    glfwSetCursorPosCallback(mWindow, handleMouseMoveCallback);
    glfwSetScrollCallback(mWindow, scrollCallback);

    // swapchain support
    mDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    initVulkan();

    createSwapChain();
}

bool nevk::GLFWRender::windowShouldClose()
{
    return glfwWindowShouldClose(mWindow);
}

void nevk::GLFWRender::pollEvents()
{
    glfwPollEvents();
}

void nevk::GLFWRender::onBeginFrame()
{
    FrameData& currFrame = getCurrentFrameData();

    vkWaitForFences(mDevice, 1, &currFrame.inFlightFence, VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(mDevice, mSwapChain, UINT64_MAX, currFrame.imageAvailable, VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        //recreateSwapChain();
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
    //double deltaTime = std::chrono::duration<double, std::milli>(currentTime - prevTime).count() / 1000.0;
    prevTime = currentTime;

    const uint32_t frameIndex = imageIndex;
    mSharedCtx.mFrameIndex = frameIndex;
    VkCommandBuffer& cmd = getFrameData(imageIndex).cmdBuffer;
    vkResetCommandBuffer(cmd, 0);
    VkCommandBufferBeginInfo cmdBeginInfo = {};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.pNext = nullptr;
    cmdBeginInfo.pInheritanceInfo = nullptr;
    cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    vkBeginCommandBuffer(cmd, &cmdBeginInfo);
}

void nevk::GLFWRender::onEndFrame()
{
    FrameData& currFrame = getCurrentFrameData();
    const uint32_t frameIndex = mSharedCtx.mFrameIndex;

    VkCommandBuffer& cmd = getFrameData(frameIndex).cmdBuffer;

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

    presentInfo.pImageIndices = &frameIndex;

    VkResult result = vkQueuePresentKHR(mPresentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) // || framebufferResized)
    {
        //framebufferResized = false;
        //recreateSwapChain();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }

    ++mSharedCtx.mFrameNumber;
}

void nevk::GLFWRender::drawFrame(Image* result)
{
    //FrameData& currFrame = getCurrentFrameData();
    const uint32_t frameIndex = mSharedCtx.mFrameIndex;

    VkCommandBuffer& cmd = getFrameData(frameIndex).cmdBuffer;

    // Copy to swapchain image
    {
        recordImageBarrier(cmd, result, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                      VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

        recordBarrier(cmd, mSwapChainImages[frameIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

        VkOffset3D srcBlitSize{};
        srcBlitSize.x = mWindowWidth;
        srcBlitSize.y = mWindowHeight;
        srcBlitSize.z = 1;
        VkImageBlit imageBlitRegion{};
        imageBlitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlitRegion.srcSubresource.layerCount = 1;
        imageBlitRegion.srcOffsets[1] = srcBlitSize;
        imageBlitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlitRegion.dstSubresource.layerCount = 1;
        VkOffset3D blitSwapSize{};
        blitSwapSize.x = mSwapChainExtent.width;
        blitSwapSize.y = mSwapChainExtent.height;
        blitSwapSize.z = 1;
        imageBlitRegion.dstOffsets[1] = blitSwapSize;
        vkCmdBlitImage(cmd, mSharedCtx.mResManager->getVkImage(result), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, mSwapChainImages[frameIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlitRegion, VK_FILTER_NEAREST);

        recordImageBarrier(cmd, result, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

        recordBarrier(cmd, mSwapChainImages[frameIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                      VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    }
}

void nevk::GLFWRender::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    assert(window);
    if (width == 0 || height == 0)
    {
        return;
    }
    //auto app = reinterpret_cast<GLFWRender*>(glfwGetWindowUserPointer(window));
    //app->framebufferResized = true;
    //nevk::Scene* scene = app->getScene();
    //scene->updateCamerasParams(width, height);
}

void nevk::GLFWRender::keyCallback(GLFWwindow* window, [[maybe_unused]] int key, [[maybe_unused]] int scancode, [[maybe_unused]] int action, [[maybe_unused]] int mods)
{
    assert(window);
    //auto app = reinterpret_cast<Render*>(glfwGetWindowUserPointer(window));
    //nevk::Scene* scene = app->getScene();
    //Camera& camera = scene->getCamera(app->getActiveCameraIndex());

    //const bool keyState = ((GLFW_REPEAT == action) || (GLFW_PRESS == action)) ? true : false;
    //switch (key)

    //{
    //case GLFW_KEY_W: {
    //    camera.keys.forward = keyState;
    //    break;
    //}
    //case GLFW_KEY_S: {
    //    camera.keys.back = keyState;
    //    break;
    //}
    //case GLFW_KEY_A: {
    //    camera.keys.left = keyState;
    //    break;
    //}
    //case GLFW_KEY_D: {
    //    camera.keys.right = keyState;
    //    break;
    //}
    //case GLFW_KEY_Q: {
    //    camera.keys.up = keyState;
    //    break;
    //}
    //case GLFW_KEY_E: {
    //    camera.keys.down = keyState;
    //    break;
    //}
    //default:
    //    break;
    //}
}

void nevk::GLFWRender::mouseButtonCallback(GLFWwindow* window, [[maybe_unused]] int button, [[maybe_unused]] int action, [[maybe_unused]] int mods)
{
    assert(window);
    //auto app = reinterpret_cast<Render*>(glfwGetWindowUserPointer(window));
    //nevk::Scene* scene = app->getScene();
    //Camera& camera = scene->getCamera(app->getActiveCameraIndex());
    //if (button == GLFW_MOUSE_BUTTON_RIGHT)
    //{
    //    if (action == GLFW_PRESS)
    //    {
    //        camera.mouseButtons.right = true;
    //    }
    //    else if (action == GLFW_RELEASE)
    //    {
    //        camera.mouseButtons.right = false;
    //    }
    //}
    //else if (button == GLFW_MOUSE_BUTTON_LEFT)
    //{
    //    if (action == GLFW_PRESS)
    //    {
    //        camera.mouseButtons.left = true;
    //    }
    //    else if (action == GLFW_RELEASE)
    //    {
    //        camera.mouseButtons.left = false;
    //    }
    //}
}

void nevk::GLFWRender::handleMouseMoveCallback(GLFWwindow* window, [[maybe_unused]] double xpos, [[maybe_unused]] double ypos)
{
    assert(window);
    //auto app = reinterpret_cast<Render*>(glfwGetWindowUserPointer(window));
    //nevk::Scene* scene = app->getScene();
    //Camera& camera = scene->getCamera(app->getActiveCameraIndex());
    //const float dx = camera.mousePos.x - (float)xpos;
    //const float dy = camera.mousePos.y - (float)ypos;

    //ImGuiIO& io = ImGui::GetIO();
    //bool handled = io.WantCaptureMouse;
    //if (handled)
    //{
    //    camera.mousePos = glm::vec2((float)xpos, (float)ypos);
    //    return;
    //}

    //if (camera.mouseButtons.right)
    //{
    //    camera.rotate(-dx, -dy);
    //}
    //if (camera.mouseButtons.left)
    //{
    //    camera.translate(glm::float3(-0.0f, 0.0f, -dy * .005f * camera.movementSpeed));
    //}
    //if (camera.mouseButtons.middle)
    //{
    //    camera.translate(glm::float3(-dx * 0.01f, -dy * 0.01f, 0.0f));
    //}
    //camera.mousePos = glm::float2((float)xpos, (float)ypos);
}

void nevk::GLFWRender::scrollCallback(GLFWwindow* window, [[maybe_unused]] double xoffset, [[maybe_unused]] double yoffset)
{
    assert(window);
    //ImGuiIO& io = ImGui::GetIO();
    //bool handled = io.WantCaptureMouse;
    //if (handled)
    //{
    //    return;
    //}

    //auto app = reinterpret_cast<Render*>(glfwGetWindowUserPointer(window));
    //nevk::Scene* mScene = app->getScene();
    //Camera& mCamera = mScene->getCamera(app->getActiveCameraIndex());

    //mCamera.translate(glm::vec3(0.0f, 0.0f,
    //                            -yoffset * mCamera.movementSpeed));
}

void nevk::GLFWRender::createLogicalDevice()
{
    nevk::VkRender::createLogicalDevice();
    // add present queue
    QueueFamilyIndices indices = findQueueFamilies(mPhysicalDevice);
    vkGetDeviceQueue(mDevice, indices.presentFamily.value(), 0, &mPresentQueue);
}

void nevk::GLFWRender::createSurface()
{
    if (glfwCreateWindowSurface(mInstance, mWindow, nullptr, &mSurface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }
}

void nevk::GLFWRender::createSwapChain()
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

    VkResult res = vkCreateSwapchainKHR(mDevice, &createInfo, nullptr, &mSwapChain);
    if (res != VK_SUCCESS)
    {
        assert(0);
        return;
    }

    vkGetSwapchainImagesKHR(mDevice, mSwapChain, &imageCount, nullptr);
    mSwapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(mDevice, mSwapChain, &imageCount, mSwapChainImages.data());

    swapChainImageFormat = surfaceFormat.format;
    mSwapChainExtent = extent;
}

nevk::GLFWRender::SwapChainSupportDetails nevk::GLFWRender::querySwapChainSupport(VkPhysicalDevice device)
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

VkSurfaceFormatKHR nevk::GLFWRender::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
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

VkPresentModeKHR nevk::GLFWRender::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
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

VkExtent2D nevk::GLFWRender::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
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

std::vector<const char*> nevk::GLFWRender::getRequiredExtensions()
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

QueueFamilyIndices nevk::GLFWRender::findQueueFamilies(VkPhysicalDevice device)
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


