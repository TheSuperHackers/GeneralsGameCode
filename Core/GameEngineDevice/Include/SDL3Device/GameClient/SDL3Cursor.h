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

/*
** Derived from the GeneralsX branch by fbraz3
*/

#pragma once

#include "Lib/BaseType.h"
#include <SDL3/SDL.h>
#include <array>

// USER INCLUDES
#include "GameClient/Mouse.h"

/**
 * AnimatedCursor - Wrapper for SDL3 native animated cursors
 */
struct AnimatedCursor {
    SDL_Cursor* m_cursor;

    AnimatedCursor() : m_cursor(nullptr) {}
    ~AnimatedCursor()
    {
        if (m_cursor)
        {
            SDL_DestroyCursor(m_cursor);
            m_cursor = nullptr;
        }
    }

    SDL_Cursor* getCursor() const { return m_cursor; }
};

/**
 * SDL3CursorManager - Manages loading and lifecycle of cursors
 */
class SDL3CursorManager
{
public:
    static void init();
    static void shutdown();

    static SDL_Cursor* getCursor(Mouse::MouseCursor cursor, int direction);
    
    // Internal loader used by Mouse implementation
    static void initResources(Mouse* mouse);

private:
    static AnimatedCursor* loadANI(const char* filepath);
    static AnimatedCursor* m_cursorResources[Mouse::NUM_MOUSE_CURSORS][MAX_2D_CURSOR_DIRECTIONS];
};
