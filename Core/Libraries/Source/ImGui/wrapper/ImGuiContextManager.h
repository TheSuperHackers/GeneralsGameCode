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

class ContextManager
{
  public:
    ContextManager();
    ~ContextManager();

    void Init(void *hwnd, void *device);

  private:
    bool m_initialized;

    // TODO change this to rule of 5 and use delete instead of declaring without
    // definition once we are using a C++11 or higher C++ Standard
    ContextManager(const ContextManager &);
    ContextManager &operator=(const ContextManager &);
};

} // namespace ImGui
} // namespace rts
