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
    Ui()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
    }

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

    void init(ImGui_ImplVulkan_InitInfo init_info, VkFormat framebufferFormat, GLFWwindow* window, VkCommandPool command_pool, VkCommandBuffer command_buffer, int width, int height);
    void updateImGui(GLFWwindow* window); // CHANGE
    void updateBuffers(VkDevice device, VkBuffer vertexBuffer, VkBuffer indexBuffer);
    void render(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    void createFrameBuffers(VkDevice device, std::vector<VkImageView>& imageViews, uint32_t width, uint32_t height);
    void createVkRenderPass(ImGui_ImplVulkan_InitInfo init_info, VkFormat framebufferFormat);
    static void setDarkThemeColors();
    void onResize(ImGui_ImplVulkan_InitInfo init_info, VkFormat framebufferFormat, VkDevice device, std::vector<VkImageView>& imageViews, VkPipeline graphicsPipeline, VkPipelineLayout pipelineLayout, uint32_t width, uint32_t height);
    void onDestroy(VkDevice device) const;

private:
    ImGui_ImplVulkanH_Window wd;
    std::vector<VkFramebuffer> mFrameBuffers;
    int32_t vertexCount = 0;
    int32_t indexCount = 0;
};
} // namespace nevk