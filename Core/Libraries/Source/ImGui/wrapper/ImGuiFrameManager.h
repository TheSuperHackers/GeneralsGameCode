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

#pragma once

namespace rts
{
namespace ImGui
{

class FrameManager
{
  public:
    static void BeginFrame();
    // TheSuperHackers @info jurassicLizard 27/01/2026 The following static function
    // calls ImGui::EndFrame() and ImGui::Render() but does NOT call
    // ImGui::RenderDrawData(draw_data). The latter is not part of this wrapper
    // by design (separation of concerns) and must be called explicitly. This is to be done
    // when sending draw data off to the GPU to be drawn on screen, which in our case happens in DX8Wrapper::End_Scene()
    static void EndFrame();

  private:
    static bool s_frameOpen;
};

} // namespace ImGui
} // namespace rts