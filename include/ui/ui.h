#pragma once
#include <vector>
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include "ImGuiFileDialog.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "scene/scene.h"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <array>
#include <stdio.h> // printf, fprintf
#include <stdlib.h> // abort

namespace nevk
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
        bool enableShadowsAcc = false;
        bool enableAO = false;
        bool enableAOAcc = false;
        bool enableFilter = false;
        bool enableAOFilter = false;
        bool enableShadows = false;
        bool enablePathTracer = false;
        bool enableReflections = false;
        float rayLen = 0.2f;
        float accAlpha = 0.125f;
        float accAOAlpha = 0.125f;
        float animTime = 0.f;
        int32_t samples;
        float sigma = 2.9f;
        float sigmaNormal = 1.5f;
        int radius = 3;
        int maxR = 5;
        float sigmaAO = 2.9f;
        float sigmaAONormal = 1.5f;
        int radiusAO = 3;
        int maxRAO = 5;
        int maxDepth = 1;
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
} // namespace nevk
