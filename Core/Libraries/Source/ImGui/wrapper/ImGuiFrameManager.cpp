/**
 * @file ImGuiFrameManager.cpp
 * @brief Simple Frame manager for ImGui widgets
 */
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
#include "ImGuiFrameManager.h"
#include "imgui.h"
#include "imgui_impl_dx8.h"
#include "imgui_impl_win32.h"

bool ImGui::FrameManager::s_frameOpen = false;

void ImGui::FrameManager::BeginFrame()
{
    if (s_frameOpen)
    {
        return;
    }

    ImGui_ImplDX8_NewFrame();
    ImGui_ImplWin32_NewFrame();
    NewFrame();

    s_frameOpen = true;
}

void ImGui::FrameManager::EndFrame()
{
    if (!s_frameOpen)
    {
        return;
    }

    Render();

    s_frameOpen = false;
}


