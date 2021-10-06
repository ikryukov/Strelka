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
    bool init(ImGui_ImplVulkan_InitInfo& init_info, VkFormat framebufferFormat, GLFWwindow* window, VkCommandPool command_pool, VkCommandBuffer command_buffer, int width, int height);
    void updateUI(Scene& scene, double msPerFrame, std::string& newModelFile, uint32_t& selectedCamera, float& animTime, bool& enableAcc, float& accAlpha, int32_t& samples);
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
