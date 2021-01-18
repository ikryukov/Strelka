/*
 * Integrating Dear ImGui within your custom engine is a matter of
 * 1) wiring mouse/keyboard/gamepad inputs
 * 2) uploading one texture to your GPU/render engine
 * 3) providing a render function that can bind textures and render textured triangles.
 * The examples/ folder is populated with applications doing just that.
 */

#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include <stdio.h> // printf, fprintf
#include <stdlib.h> // abort
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#    pragma comment(lib, "legacy_stdio_definitions")
#endif

//#define IMGUI_UNLIMITED_FRAME_RATE
#ifdef _DEBUG
#    define IMGUI_VULKAN_DEBUG_REPORT
#endif


namespace nevk
{

class ImGuiUi
{
public:
    ImGuiUi() = default;
    ~ImGuiUi() = default;

    virtual void OnAttach();
    virtual void OnDetach();
    //    virtual void OnEvent();

    static void Begin();
    static void End();

    void BlockEvents(bool block)
    {
        m_BlockEvents = block;
    }

    static void SetDarkThemeColors();

private:
    bool m_BlockEvents = true;
    float m_Time = 0.0f;
};
} // namespace nevk