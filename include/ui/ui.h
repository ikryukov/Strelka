#pragma once
#include <vector>
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include "scene/scene.h"
#include <stdio.h> // printf, fprintf
#include <stdlib.h> // abort

#include <array>

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
        ImGui::DestroyContext();
    }
    bool init(ImGui_ImplVulkan_InitInfo& init_info, VkFormat framebufferFormat, GLFWwindow* window, VkCommandPool command_pool, VkCommandBuffer command_buffer, int width, int height,char* path);
    std::string updateUI(GLFWwindow* window, Scene& scene);
    void render(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    bool createFrameBuffers(VkDevice device, std::vector<VkImageView>& imageViews, uint32_t width, uint32_t height);
    void createVkRenderPass(ImGui_ImplVulkan_InitInfo init_info, VkFormat framebufferFormat);
    bool uploadFonts(ImGui_ImplVulkan_InitInfo init_info, VkCommandPool command_pool, VkCommandBuffer command_buffer);
    static void setDarkThemeColors();
    void onResize(ImGui_ImplVulkan_InitInfo& init_info, std::vector<VkImageView>& imageViews, uint32_t width, uint32_t height);
    void onDestroy() const;

private:
    ImGui_ImplVulkan_InitInfo mInitInfo;
    ImGui_ImplVulkanH_Window wd;
    std::vector<VkFramebuffer> mFrameBuffers;
    std::vector<bool> Bools;
    std::vector<std::string> filesName;
    VkFormat mFrameBufferFormat;
};
} // namespace nevk
