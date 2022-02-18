#pragma once

// to supress warnings and faster compilation
#ifdef _WIN32
#    define VC_EXTRALEAN
#    define WIN32_LEAN_AND_MEAN
#    include <Windows.h>
#endif

#define GLFW_INCLUDE_VULKAN
#include "render/vkrender.h"

#include <GLFW/glfw3.h>

namespace nevk
{

class InputHandler
{
public:
    virtual void keyCallback(int key, [[maybe_unused]] int scancode, int action, [[maybe_unused]] int mods) = 0;
};

class GLFWRender : public VkRender
{
public:
    void init(int width, int height);

    void setWindowTitle(const char* title);

    void setInputHandler(InputHandler* handler)
    {
        mInputHandler = handler;
    }
    InputHandler* getInputHandler()
    {
        return mInputHandler;
    }

    bool windowShouldClose();
    void pollEvents();

    void onBeginFrame();
    void onEndFrame();

    void drawFrame(Image* result);

protected:
    InputHandler* mInputHandler;
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
    static void keyCallback(GLFWwindow* window, int key, [[maybe_unused]] int scancode, int action, [[maybe_unused]] int mods);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void handleMouseMoveCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

    void createLogicalDevice() override;
    void createSurface() override;
    void createSwapChain();

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    std::vector<const char*> getRequiredExtensions() override;
    // need to find present queue
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) override;

    int mWindowWidth = 800;
    int mWindowHeight = 600;

    GLFWwindow* mWindow;
    VkSurfaceKHR mSurface;
    VkSwapchainKHR mSwapChain;
    std::vector<VkImage> mSwapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D mSwapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;
};

} // namespace nevk