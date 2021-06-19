#include "ui.h"

#include <stdexcept>
#include <utility>

namespace nevk
{

static bool mousePressed[2] = { false, false };

static void check_vk_result(VkResult err)
{
    if (err == 0)
        return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}

static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (action == GLFW_PRESS && button >= 0 && button < 2)
        mousePressed[button] = true;
}

static void glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MouseWheel += (float)yoffset; // Use fractional mouse wheel, 1.0 unit 5 lines.
}

static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    ImGuiIO& io = ImGui::GetIO();
    if (action == GLFW_PRESS)
        io.KeysDown[key] = true;
    if (action == GLFW_RELEASE)
        io.KeysDown[key] = false;
    io.KeyCtrl = (mods & GLFW_MOD_CONTROL) != 0;
    io.KeyShift = (mods & GLFW_MOD_SHIFT) != 0;
}

static void glfw_char_callback(GLFWwindow* window, unsigned int c)
{
    if (c > 0 && c < 0x10000)
        ImGui::GetIO().AddInputCharacter((unsigned short)c);
}


bool Ui::init(ImGui_ImplVulkan_InitInfo& init_info, VkFormat framebufferFormat, GLFWwindow* window, VkCommandPool command_pool, VkCommandBuffer command_buffer, int width, int height)
{
    mInitInfo = init_info;
    wd.Width = width;
    wd.Height = height;

    mFrameBufferFormat = framebufferFormat;
    createVkRenderPass(framebufferFormat);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext(); //this initializes the core structures of imgui

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(width, height);
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
    (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(window, true);

    // Set debug callback
    init_info.CheckVkResultFn = check_vk_result;
    bool ret = ImGui_ImplVulkan_Init(&init_info, wd.RenderPass);

    // Upload Fonts
    if (!uploadFonts(command_pool, command_buffer))
    {
        ret = false;
    }

    setDarkThemeColors();

    return ret;
}

bool Ui::uploadFonts(VkCommandPool command_pool, VkCommandBuffer command_buffer)
{
    // Use any command queue
    VkResult err = vkResetCommandPool(mInitInfo.Device, command_pool, 0);
    check_vk_result(err);
    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    err = vkBeginCommandBuffer(command_buffer, &begin_info);
    check_vk_result(err);

    bool ret = ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

    VkSubmitInfo end_info = {};
    end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    end_info.commandBufferCount = 1;
    end_info.pCommandBuffers = &command_buffer;
    err = vkEndCommandBuffer(command_buffer);
    check_vk_result(err);
    err = vkQueueSubmit(mInitInfo.Queue, 1, &end_info, VK_NULL_HANDLE);
    check_vk_result(err);

    err = vkDeviceWaitIdle(mInitInfo.Device);
    check_vk_result(err);
    ImGui_ImplVulkan_DestroyFontUploadObjects();

    return ret;
}

void Ui::setDarkThemeColors()
{
    ImGuiStyle* style = &ImGui::GetStyle();

    style->WindowPadding = ImVec2(15, 15);
    style->WindowRounding = 5.0f;
    style->FramePadding = ImVec2(5, 5);
    style->FrameRounding = 4.0f;
    style->ItemSpacing = ImVec2(12, 8);
    style->ItemInnerSpacing = ImVec2(8, 6);
    style->IndentSpacing = 25.0f;
    style->ScrollbarSize = 15.0f;
    style->ScrollbarRounding = 9.0f;
    style->GrabMinSize = 5.0f;
    style->GrabRounding = 3.0f;

    style->Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
    style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style->Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 0.90f);
    style->Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 0.90f);
    style->Colors[ImGuiCol_Border] = ImVec4(0.80f, 0.80f, 0.83f, 0.88f);
    style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
    style->Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
    style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
    style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
    style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
    style->Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);
}

bool Ui::createFrameBuffers(VkDevice device, std::vector<VkImageView>& imageViews, uint32_t width, uint32_t height)
{
    mFrameBuffers.resize(3);
    VkResult err;

    for (size_t i = 0; i < 3; i++)
    {
        std::array<VkImageView, 1> attachments = {
            imageViews[i],
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = wd.RenderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = width;
        framebufferInfo.height = height;
        framebufferInfo.layers = 1;

        err = vkCreateFramebuffer(device, &framebufferInfo, nullptr, &mFrameBuffers[i]);
        check_vk_result(err);
    }
    return err == 0;
}

void Ui::updateUI(Scene& scene, DepthPass& depthPass, double msPerFrame, std::string& newModelPath)
{
    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Settings:"); // begin window

    ImGui::Text("MsPF = %f", msPerFrame);
    ImGui::Text("FPS = %f", 1000.0 / msPerFrame);

    // open Dialog Simple
    if (ImGui::Button("Open File Dialog"))
        ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".gltf,.obj", ".");
    // display
    if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            newModelPath = filePathName;
            std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
        }
        ImGuiFileDialog::Instance()->Close();
    }

    ImGui::Text("Light Position");
    ImGui::SliderFloat("pos coordinate X", &scene.mLightPosition.x, -100.0f, 100.0f);
    ImGui::SliderFloat("pos coordinate Y", &scene.mLightPosition.y, -100.0f, 100.0f);
    ImGui::SliderFloat("pos coordinate Z", &scene.mLightPosition.z, -100.0f, 100.0f);
    if (ImGui::Button("Copy from current camera to light position"))
    {
        (glm::float3&)scene.mLightPosition = scene.getCamera().getPosition();
    }

    ImGui::Text("Light At");
    ImGui::SliderFloat("coordinate X", &depthPass.lightAt.x, -100.0f, 100.0f);
    ImGui::SliderFloat("coordinate Y", &depthPass.lightAt.y, -100.0f, 100.0f);
    ImGui::SliderFloat("coordinate Z", &depthPass.lightAt.z, -100.0f, 100.0f);
    if (ImGui::Button("Copy from current camera to light at"))
    {
        depthPass.lightAt = scene.getCamera().getPosition();
    }

    ImGui::Text("Light Direction Upwards");
    ImGui::SliderFloat("up coordinate X", &depthPass.lightUpwards.x, -100.0f, 100.0f);
    ImGui::SliderFloat("up coordinate Y", &depthPass.lightUpwards.y, -100.0f, 100.0f);
    ImGui::SliderFloat("up coordinate Z", &depthPass.lightUpwards.z, -100.0f, 100.0f);

    ImGui::Text("Other light settings");
    ImGui::SliderFloat("fov angle", &depthPass.fovAngle, -100.0f, 100.0f);
    ImGui::SliderFloat("zNear", &depthPass.zNear, -100.0f, 100.0f);
    ImGui::SliderFloat("zFar", &depthPass.zFar, -100.0f, 100.0f);
    ImGui::SliderFloat("depth bias factor", &depthPass.depthBiasConstant, -100.0f, 100.0f);
    ImGui::SliderFloat("slope depth bias factor", &depthPass.depthBiasSlope, -100.0f, 100.0f);

    const char* items[] = { "None", "Normals", "Shadow b&w", "Shadow PCF", "Shadow Poisson", "Shadow Poisson+PCF" };
    static const char* current_item = items[0];

    if (ImGui::BeginCombo("Debug view", current_item))
    {
        for (int n = 0; n < IM_ARRAYSIZE(items); n++)
        {
            bool is_selected = (current_item == items[n]);
            if (ImGui::Selectable(items[n], is_selected))
            {
                current_item = items[n];
                scene.mDebugViewSettings = (Scene::DebugView)n;
            }
            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    //     transparency settings
    ImGui::Checkbox("Transparent Mode", &scene.transparentMode);
    ImGui::Checkbox("Opaque Mode", &scene.opaqueMode);


    ImGui::End(); // end window
}

void Ui::render(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    // Rendering
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();

    const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
    if (!is_minimized)
    {
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
        memcpy(&wd.ClearValue.color.float32[0], &clear_color, 4 * sizeof(float));
    }

    {
        VkRenderPassBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = wd.RenderPass;
        info.framebuffer = mFrameBuffers[imageIndex];
        info.renderArea.extent.width = wd.Width;
        info.renderArea.extent.height = wd.Height;
        info.clearValueCount = 1;
        info.pClearValues = &wd.ClearValue;
        vkCmdBeginRenderPass(commandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
    }

    // Record dear imgui primitives into command buffer
    ImGui_ImplVulkan_RenderDrawData(draw_data, commandBuffer);

    // Submit command buffer
    vkCmdEndRenderPass(commandBuffer);
}

void Ui::createVkRenderPass(VkFormat framebufferFormat)
{
    VkAttachmentDescription attachment = {};
    attachment.format = framebufferFormat;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    VkAttachmentReference color_attachment = {};
    color_attachment.attachment = 0;
    color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment;
    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    VkRenderPassCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.attachmentCount = 1;
    info.pAttachments = &attachment;
    info.subpassCount = 1;
    info.pSubpasses = &subpass;
    info.dependencyCount = 1;
    info.pDependencies = &dependency;
    VkResult err = vkCreateRenderPass(mInitInfo.Device, &info, nullptr, &wd.RenderPass);
    check_vk_result(err);
}

void Ui::onResize(std::vector<VkImageView>& imageViews, uint32_t width, uint32_t height)
{
    wd.Width = width;
    wd.Height = height;

    for (auto& framebuffer : mFrameBuffers)
    {
        vkDestroyFramebuffer(mInitInfo.Device, framebuffer, nullptr);
    }
    vkDestroyRenderPass(mInitInfo.Device, wd.RenderPass, nullptr);

    createVkRenderPass(mFrameBufferFormat);
    createFrameBuffers(mInitInfo.Device, imageViews, wd.Width, wd.Height);
}

void Ui::onDestroy() const
{
    for (auto& framebuffer : mFrameBuffers)
    {
        vkDestroyFramebuffer(mInitInfo.Device, framebuffer, nullptr);
    }
    vkDestroyRenderPass(mInitInfo.Device, wd.RenderPass, nullptr);
    ImGui_ImplVulkan_Shutdown();
    ImGui::DestroyContext();
}

} // namespace nevk
