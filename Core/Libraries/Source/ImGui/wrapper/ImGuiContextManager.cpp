/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 TheSuperHackers
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "ImGuiContextManager.h"
#include "imgui.h"
#include "imgui_impl_dx8.h"
#include "imgui_impl_win32.h"
#include <d3d8.h>
#include <windows.h>

rts::ImGui::ContextManager::ContextManager() : m_initialized(false) {}

rts::ImGui::ContextManager::~ContextManager()
{
    if (m_initialized)
    {
        ImGui_ImplDX8_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ::ImGui::DestroyContext();
    }
}

void rts::ImGui::ContextManager::Init(void *hwnd, void *device)
{
    if (m_initialized)
    {
        return;
    }

    ::IMGUI_CHECKVERSION();
    ::ImGui::CreateContext();
    ImGuiIO &io = ::ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

    ::ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(static_cast<HWND>(hwnd));
    ImGui_ImplDX8_Init(static_cast<IDirect3DDevice8 *>(device));

    io.Fonts->AddFontDefault();

    m_initialized = true;
}
