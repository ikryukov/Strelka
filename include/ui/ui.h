#pragma once
#include <vector>
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include <stdio.h> // printf, fprintf
#include <stdlib.h> // abort
#include <array>

namespace nevk
{

class Ui
{
public:
    ~Ui()
    {
        ImGui::DestroyContext();
        //        // Release all Vulkan resources required for rendering imGui
        //        vertexBuffer.destroy();
        //        indexBuffer.destroy();
        //        vkDestroyImage(device->logicalDevice, fontImage, nullptr);
        //        vkDestroyImageView(device->logicalDevice, fontView, nullptr);
        //        vkFreeMemory(device->logicalDevice, fontMemory, nullptr);
        //        vkDestroySampler(device->logicalDevice, sampler, nullptr);
        //        vkDestroyPipelineCache(device->logicalDevice, pipelineCache, nullptr);
        //        vkDestroyPipeline(device->logicalDevice, pipeline, nullptr);
        //        vkDestroyPipelineLayout(device->logicalDevice, pipelineLayout, nullptr);
        //        vkDestroyDescriptorPool(device->logicalDevice, descriptorPool, nullptr);
        //        vkDestroyDescriptorSetLayout(device->logicalDevice, descriptorSetLayout, nullptr);
    }

    bool init(ImGui_ImplVulkan_InitInfo init_info, VkFormat framebufferFormat, GLFWwindow* window, VkCommandPool command_pool, VkCommandBuffer command_buffer, int width, int height);
    void updateUI(GLFWwindow* window);
    void render(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    bool createFrameBuffers(VkDevice device, std::vector<VkImageView>& imageViews, uint32_t width, uint32_t height);
    void createVkRenderPass(ImGui_ImplVulkan_InitInfo init_info, VkFormat framebufferFormat);
    bool uploadFonts(ImGui_ImplVulkan_InitInfo init_info, VkCommandPool command_pool, VkCommandBuffer command_buffer);
    static void setDarkThemeColors();
    void onResize(ImGui_ImplVulkan_InitInfo init_info, VkDevice device, std::vector<VkImageView>& imageViews, uint32_t width, uint32_t height);
    void onDestroy() const;

private:
    VkBuffer vertexBuffer;
    VkBuffer indexBuffer;
    VkDevice mDevice;
    ImGui_ImplVulkanH_Window wd;
    std::vector<VkFramebuffer> mFrameBuffers;
    VkFormat mFrameBufferFormat;
    int32_t vertexCount = 0;
    int32_t indexCount = 0;
};
} // namespace nevk
