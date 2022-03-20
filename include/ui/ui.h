#pragma once
#include <vector>
#define GLFW_INCLUDE_NONE
#ifdef _WIN32
#    define GLFW_EXPOSE_NATIVE_WIN32
#    undef APIENTRY
#endif
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
//#include <vulkan/vulkan.h>

#include "ImGuiFileDialog.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include <scene/scene.h>
#include <settings/settings.h>

namespace oka
{

class Ui
{
public:
    Ui()
    {
    }
    ~Ui()
    {
    }

    struct RenderConfig
    {
        bool enableUpscale = true;
        bool enablePathTracerAcc = true;
        bool enablePathTracer = true;
        bool recreateBVH = false;
        bool useSwizzleTid = false;
        bool renderCPU = false;
        float upscaleFactor = enableUpscale ? 0.5f : 1.0f; // 1 -- w/o upscaling, 0.5 -- render in half size
        float rayLen = 0.2f;
        float accAlpha = 0.125f;
        float animTime = 0.f;
        float sigma = 2.9f;
        float sigmaNormal = 1.5f;
        float sigmaAO = 2.9f;
        float sigmaAONormal = 1.5f;
        int32_t samples;
        int32_t radius = 3;
        int32_t maxR = 5;
        int32_t radiusAO = 3;
        int32_t maxRAO = 5;
        int32_t maxDepth = 1;
    };

    struct RenderStats
    {
        double msPerFrame = 33.33;
    };

    struct SceneConfig
    {
        uint32_t selectedCamera;
        std::string newModelPath;
    };

    bool init(ImGui_ImplVulkan_InitInfo& init_info, VkFormat framebufferFormat, GLFWwindow* window, VkCommandPool command_pool, VkCommandBuffer command_buffer, int width, int height);
    void updateUI(Scene& scene, RenderConfig& renderConfig, RenderStats& renderStats, SceneConfig& sceneConfig);
    void updateUI(oka::SettingsManager* settingsManager, RenderConfig& renderConfig, RenderStats& renderStats, SceneConfig& sceneConfig);
    
    void render(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    bool createFrameBuffers(VkDevice device, std::vector<VkImageView>& imageViews, uint32_t width, uint32_t height);
    void createVkRenderPass(VkFormat framebufferFormat);
    bool uploadFonts(VkCommandPool command_pool, VkCommandBuffer command_buffer);
    static void setDarkThemeColors();
    void onResize(std::vector<VkImageView>& imageViews, uint32_t width, uint32_t height);
    void onDestroy() const;

    void loadFromJson(Scene& scene);

private:
    ImGui_ImplVulkan_InitInfo mInitInfo{};
    ImGui_ImplVulkanH_Window wd{};
    std::vector<VkFramebuffer> mFrameBuffers;
    VkFormat mFrameBufferFormat = VkFormat::VK_FORMAT_UNDEFINED;
};
} // namespace oka
