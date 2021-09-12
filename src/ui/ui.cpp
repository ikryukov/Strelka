#include "ui.h"

#include <glm/gtc/type_ptr.hpp>

#include <ImGuizmo.h>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <utility>

namespace fs = std::filesystem;

#include <json.hpp>
using json = nlohmann::json;

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

    // setDarkThemeColors();

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

static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
static const float identityMatrix[16] = { 1.f, 0.f, 0.f, 0.f,
                                          0.f, 1.f, 0.f, 0.f,
                                          0.f, 0.f, 1.f, 0.f,
                                          0.f, 0.f, 0.f, 1.f };
static bool useWindow = false;
static int gizmoCount = 1;

void EditTransform(Camera& cam, float camDistance, float* matrix, bool editTransformDecomposition)
{
    static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);
    static bool useSnap = false;
    static float snap[3] = { 1.f, 1.f, 1.f };
    static float bounds[] = { -0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f };
    static float boundsSnap[] = { 0.1f, 0.1f, 0.1f };
    static bool boundSizing = false;
    static bool boundSizingSnap = false;

    if (editTransformDecomposition)
    {
        if (ImGui::IsKeyPressed(90))
            mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
        if (ImGui::IsKeyPressed(69))
            mCurrentGizmoOperation = ImGuizmo::ROTATE;
        if (ImGui::IsKeyPressed(82)) // r Key
            mCurrentGizmoOperation = ImGuizmo::SCALE;
        if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
            mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
        ImGui::SameLine();
        if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
            mCurrentGizmoOperation = ImGuizmo::ROTATE;
        ImGui::SameLine();
        if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
            mCurrentGizmoOperation = ImGuizmo::SCALE;
        //   if (ImGui::RadioButton("Universal", mCurrentGizmoOperation == ImGuizmo::UNIVERSAL))
        //      mCurrentGizmoOperation = ImGuizmo::UNIVERSAL;

        float matrixTranslation[3], matrixRotation[3], matrixScale[3];
        ImGuizmo::DecomposeMatrixToComponents(matrix, matrixTranslation, matrixRotation, matrixScale);
        ImGui::DragFloat3("Translation", matrixTranslation);
        ImGui::DragFloat3("Rotation", matrixRotation);
        ImGui::DragFloat3("Scale", matrixScale);
        ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, matrix);

        if (mCurrentGizmoOperation != ImGuizmo::SCALE)
        {
            if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
                mCurrentGizmoMode = ImGuizmo::LOCAL;
            ImGui::SameLine();
            if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
                mCurrentGizmoMode = ImGuizmo::WORLD;
        }
        if (ImGui::IsKeyPressed(83))
            useSnap = !useSnap;
        ImGui::Checkbox("", &useSnap);
        ImGui::SameLine();

        switch (mCurrentGizmoOperation)
        {
        case ImGuizmo::TRANSLATE:
            ImGui::InputFloat3("Snap", &snap[0]);
            break;
        case ImGuizmo::ROTATE:
            ImGui::InputFloat("Angle Snap", &snap[0]);
            break;
        case ImGuizmo::SCALE:
            ImGui::InputFloat("Scale Snap", &snap[0]);
            break;
        }
        ImGui::Checkbox("Bound Sizing", &boundSizing);
        if (boundSizing)
        {
            ImGui::PushID(3);
            ImGui::Checkbox("", &boundSizingSnap);
            ImGui::SameLine();
            ImGui::InputFloat3("Snap", boundsSnap);
            ImGui::PopID();
        }
    }

    ImGuiIO& io = ImGui::GetIO();
    float viewManipulateRight = io.DisplaySize.x;
    float viewManipulateTop = 0;
    if (useWindow)
    {
        ImGui::SetNextWindowSize(ImVec2(800, 400));
        ImGui::SetNextWindowPos(ImVec2(400, 20));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, (ImVec4)ImColor(0.35f, 0.3f, 0.3f));
        ImGui::Begin("Gizmo", 0, ImGuiWindowFlags_NoMove);
        ImGuizmo::SetDrawlist();
        float windowWidth = (float)ImGui::GetWindowWidth();
        float windowHeight = (float)ImGui::GetWindowHeight();
        ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);
        viewManipulateRight = ImGui::GetWindowPos().x + windowWidth;
        viewManipulateTop = ImGui::GetWindowPos().y;
    }
    else
    {
        ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    }

    // ImGuizmo::DrawGrid(cameraView, cameraProjection, identityMatrix, 100.f);
    // ImGuizmo::DrawCubes(cameraView, cameraProjection, matrix, gizmoCount);

    glm::float4x4 cameraView = cam.getView();
    glm::float4x4 cameraProjection = cam.getPerspective();

    ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection), mCurrentGizmoOperation, mCurrentGizmoMode, matrix, NULL, useSnap ? &snap[0] : NULL, boundSizing ? bounds : NULL, boundSizingSnap ? boundsSnap : NULL);
    ImGuizmo::ViewManipulate(glm::value_ptr(cameraView), camDistance, ImVec2(viewManipulateRight - 128, viewManipulateTop), ImVec2(128, 128), 0x10101010);

    cam.matrices.view = cameraView;

    if (useWindow)
    {
        ImGui::End();
        ImGui::PopStyleColor(1);
    }
}

void showGizmo(Camera& cam, float camDistance, float* matrix, ImGuizmo::OPERATION operation)
{
    static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);
    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    glm::float4x4 cameraView = cam.getView();
    glm::float4x4 cameraProjection = cam.getPerspective();
    ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection), operation, mCurrentGizmoMode, matrix, NULL, nullptr, nullptr, nullptr);
}

void displayLightSettings(uint32_t& lightId, Scene& scene, const uint32_t& selectedCamera, const std::string& currentPath, const std::string& currentFileName)
{
    Camera& cam = scene.getCamera(selectedCamera);
    glm::float3 camPos = cam.getPosition();

    // get CPU light
    std::vector<Scene::RectLightDesc>& lightDescs = scene.getLightsDesc();
    Scene::RectLightDesc& currLightDesc = lightDescs[lightId];

    if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
        mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
    ImGui::SameLine();
    if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
        mCurrentGizmoOperation = ImGuizmo::ROTATE;

    glm::float2 scale = { currLightDesc.width, currLightDesc.height };
    ImGui::Text("Rectangle light");
    ImGui::Spacing();
    ImGui::AlignTextToFramePadding();
    ImGui::DragFloat3("Position", &currLightDesc.position.x);
    ImGui::Spacing();
    ImGui::DragFloat3("Orientation", &currLightDesc.orientation.x);
    ImGui::Spacing();
    ImGui::DragFloat2("Width/Height", &scale.x, 0.005f, 1.0f);
    ImGui::Spacing();
    ImGui::ColorEdit3("Color", &currLightDesc.color.x);
    // upd current scale params.
    currLightDesc.width = scale.x;
    currLightDesc.height = scale.y;

    ImGuizmo::SetID(lightId);

    // construct final xform for imguizmo
    const glm::float4x4 translationMatrix = glm::translate(glm::float4x4(1.0f), currLightDesc.position);
    glm::quat rotation = glm::quat(glm::radians(currLightDesc.orientation)); // to quaternion
    const glm::float4x4 rotationMatrix{ rotation };
    // light have o-y o-z scaling
    const glm::float4x4 scaleMatrix = glm::scale(glm::float4x4(1.0f), glm::float3(1.0f, currLightDesc.width, currLightDesc.height));

    float camDist = glm::distance(camPos, currLightDesc.position);
    glm::float4x4 lightXform = translationMatrix * rotationMatrix * scaleMatrix;

    // show controls
    showGizmo(cam, camDist, &lightXform[0][0], mCurrentGizmoOperation);

    // need to deconstruct final xform to components
    float matrixTranslation[3], matrixRotation[3], matrixScale[3];
    ImGuizmo::DecomposeMatrixToComponents(&lightXform[0][0], matrixTranslation, matrixRotation, matrixScale);

    // write result to description
    currLightDesc.position = glm::float3(matrixTranslation[0], matrixTranslation[1], matrixTranslation[2]);
    currLightDesc.orientation = glm::float3(matrixRotation[0], matrixRotation[1], matrixRotation[2]);
    currLightDesc.width = matrixScale[1];
    currLightDesc.height = matrixScale[2];

    // update in scene
    Scene::RectLightDesc desc = { currLightDesc.position, currLightDesc.orientation, currLightDesc.width, currLightDesc.height, currLightDesc.color };
    scene.updateLight(lightId, desc);
    if (ImGui::Button("Download"))
    {
        std::string fileName = currentFileName.substr(0, currentFileName.rfind('.')); // w/o extension
        std::string jsonPath = currentPath + "/" + fileName + "_light" + ".json";
        if (fs::exists(jsonPath))
        {
            std::ifstream i(jsonPath);
            json light;
            i >> light;

            for (uint32_t j = 0; j < light["lights"].size(); ++j)
            {
                Scene::RectLightDesc desc = { glm::float3(light["lights"][j]["position"][0], light["lights"][j]["position"][1], light["lights"][j]["position"][2]),
                                              glm::float3(light["lights"][j]["orientation"][0], light["lights"][j]["orientation"][1], light["lights"][j]["orientation"][2]),
                                              float(light["lights"][j]["width"]), light["lights"][j]["height"],
                                              glm::float3(light["lights"][j]["color"][0], light["lights"][j]["color"][1], light["lights"][j]["color"][2]) };

                lightId = scene.createLight(desc);
            }
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Add"))
    {
        Scene::RectLightDesc desc = { currLightDesc.position, currLightDesc.orientation, currLightDesc.width, currLightDesc.height, currLightDesc.color };
        lightId = scene.createLight(desc);
    }
    ImGui::SameLine();
    if (ImGui::Button("Save"))
    {
        json lightSettings[lightDescs.size()];
        for (uint32_t i = 0; i < lightDescs.size(); ++i)
        {
            lightSettings[i]["position"] = { lightDescs[i].position.x, lightDescs[i].position.y, lightDescs[i].position.z };
            lightSettings[i]["orientation"] = { lightDescs[i].orientation.x, lightDescs[i].orientation.y, lightDescs[i].orientation.z };
            lightSettings[i]["width"] = lightDescs[i].width;
            lightSettings[i]["height"] = lightDescs[i].height;
            lightSettings[i]["color"] = { lightDescs[i].color.x, lightDescs[i].color.y, lightDescs[i].color.z };
        }

        json lights;
        for (uint32_t i = 0; i < lightDescs.size(); ++i)
        {
            lights["lights"].push_back(lightSettings[i]);
        }
        std::string fileName = currentFileName.substr(0, currentFileName.rfind('.')); // w/o extension
        std::string jsonPath = currentPath + "/" + fileName + "_lightSaved" + ".json";
        std::ofstream o(jsonPath);
        o << std::setw(4) << lights << std::endl;

        lights.clear();
        lightSettings->clear();
    }
    ImGui::SameLine();
    if (ImGui::Button("Remove"))
    {
        if (scene.mLights.size() > 1)
        {
            scene.removeLight(lightId);
            lightId = 0;
        }
    }
}

void Ui::updateUI(Scene& scene, DepthPass& depthPass, double msPerFrame, std::string& newModelPath, uint32_t& selectedCamera)
{
    ImGuiIO& io = ImGui::GetIO();
    bool openFD = false;
    static uint32_t showPropertiesId = -1;
    static uint32_t lightId = 0;
    static bool isLight = false;
    static bool openInspector = false;
    static std::string currentPath;
    static std::string currentFileName;

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGuizmo::SetOrthographic(false);
    ImGuizmo::BeginFrame();
    const std::vector<Instance>& instances = scene.getInstances();

    Camera& cam = scene.getCamera(selectedCamera);
    glm::float3 camPos = cam.getPosition();

    ImGui::Begin("Menu:"); // begin window
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open File"))
            {
                openFD = true;
            }
            if (ImGui::MenuItem("Inspector"))
            {
                openInspector = true;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    // open file dialog
    if (openFD)
        ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".gltf,.obj", ".");
    // display
    if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            newModelPath = ImGuiFileDialog::Instance()->GetFilePathName();
            currentPath = ImGuiFileDialog::Instance()->GetCurrentPath();
            currentFileName = ImGuiFileDialog::Instance()->GetCurrentFileName();

            showPropertiesId = -1; // new scene, updated properties
            openInspector = true;
        }
        ImGuiFileDialog::Instance()->Close();
    }

    // open new window w/ scene tree
    const std::vector<nevk::Instance>& currInstance = scene.getInstances();
    if (openInspector)
    {
        ImGui::Begin("Inspector", &openInspector); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        {
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
            ImGui::BeginChild("Properties", ImVec2(0, 200), true, ImGuiWindowFlags_MenuBar);
            if (ImGui::BeginMenuBar())
            {
                ImGui::TextUnformatted("Properties");
                ImGui::EndMenuBar();
            }
            if (!isLight)
            {
                if (showPropertiesId != -1)
                {
                    ImGuizmo::SetID(showPropertiesId);
                    float camDist = glm::distance(camPos, instances[showPropertiesId].massCenter);
                    glm::float4x4 xform = instances[showPropertiesId].transform;
                    EditTransform(cam, camDist, glm::value_ptr(xform), true);
                    scene.updateInstanceTransform(showPropertiesId, xform);

                    ImGui::Text("Material ID: %d", currInstance[showPropertiesId].mMaterialId);
                    ImGui::Text("Mass Center: %f %f %f", currInstance[showPropertiesId].massCenter.x, currInstance[showPropertiesId].massCenter.y, currInstance[showPropertiesId].massCenter.z);
                }
            }
            if (isLight)
            {
                displayLightSettings(lightId, scene, selectedCamera, currentPath, currentFileName);
            }
            ImGui::EndChild();
            ImGui::PopStyleVar();
        }
        {
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
            ImGui::BeginChild("SceneView", ImVec2(0, 260), true, ImGuiWindowFlags_MenuBar);
            if (ImGui::BeginMenuBar())
            {
                ImGui::TextUnformatted("Scene View");
                ImGui::EndMenuBar();
            }
            if (ImGui::TreeNode("Scene"))
            {
                if (ImGui::TreeNode("Lights"))
                {
                    for (uint32_t i = 0; i < scene.mLights.size(); i++)
                    {
                        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf;
                        if (ImGui::TreeNodeEx((void*)(intptr_t)i, flags, "Light ID: %d", i))
                        {
                            if (ImGui::IsItemClicked())
                            {
                                lightId = i;
                                isLight = true;
                            }
                            ImGui::TreePop();
                        }
                    }
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("Instances"))
                {
                    for (uint32_t i = 0; i < currInstance.size(); i++)
                    {
                        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf;
                        if (ImGui::TreeNodeEx((void*)(intptr_t)i, flags, "Instance ID: %d", currInstance[i].mMeshId))
                        {
                            if (ImGui::IsItemClicked())
                            {
                                showPropertiesId = i;
                                isLight = false;
                            }
                            ImGui::TreePop();
                        }
                    }
                    ImGui::TreePop();
                }
                ImGui::TreePop();
            }
            ImGui::EndChild();
            ImGui::PopStyleVar();
        }
        ImGui::End();
    }

    // simple settings
    ImGui::Text("MsPF = %f", msPerFrame);
    ImGui::Text("FPS = %f", 1000.0 / msPerFrame);

    const std::vector<nevk::Camera>& cameras = scene.getCameras();
    assert(selectedCamera < cameras.size());
    const char* currentCameraName = cameras[selectedCamera].name.c_str();

    if (ImGui::BeginCombo("Camera", currentCameraName))
    {
        for (size_t i = 0; i < cameras.size(); ++i)
        {
            bool isSelected = (currentCameraName == cameras[i].name.c_str());
            if (ImGui::Selectable(cameras[i].name.c_str(), isSelected))
            {
                currentCameraName = cameras[i].name.c_str();
                selectedCamera = i;
            }
            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    const char* items[] = { "None" };
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
