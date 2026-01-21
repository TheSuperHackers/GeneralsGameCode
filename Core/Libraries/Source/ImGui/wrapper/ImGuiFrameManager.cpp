#include "ImGuiFrameManager.h"
#include "imgui.h"
#include "imgui_impl_dx8.h"
#include "imgui_impl_win32.h"

bool ImGui::FrameManager::s_frameOpen = false;

void ImGui::FrameManager::BeginFrame()
{
    if (s_frameOpen) {
        return;
    }

    ImGui_ImplDX8_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    s_frameOpen = true;
}

void ImGui::FrameManager::EndFrame()
{
    if (!s_frameOpen) {
        return;
    }

    ImGui::Render();

    s_frameOpen = false;
}

