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

#include "SDL3Device/GameClient/SDL3Cursor.h"
#include <SDL3_image/SDL_image.h>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <vector>
#include <memory>

#include "Common/Debug.h"
#include "Common/file.h"
#include "Common/FileSystem.h"

// Initialize static member
AnimatedCursor* SDL3CursorManager::m_cursorResources[Mouse::NUM_MOUSE_CURSORS][MAX_2D_CURSOR_DIRECTIONS] = { nullptr };

void SDL3CursorManager::init()
{
    // Cursors are typically initialized via initResources when the Mouse device is ready
    shutdown();
}

void SDL3CursorManager::shutdown()
{
    for (int i = 0; i < Mouse::NUM_MOUSE_CURSORS; ++i)
    {
        for (int j = 0; j < MAX_2D_CURSOR_DIRECTIONS; ++j)
        {
            if (m_cursorResources[i][j])
            {
                delete m_cursorResources[i][j];
                m_cursorResources[i][j] = nullptr;
            }
        }
    }
}

SDL_Cursor* SDL3CursorManager::getCursor(Mouse::MouseCursor cursor, int direction)
{
    if (cursor < 0 || cursor >= Mouse::NUM_MOUSE_CURSORS) return nullptr;
    if (direction < 0 || direction >= MAX_2D_CURSOR_DIRECTIONS) direction = 0;

    AnimatedCursor* anim = m_cursorResources[cursor][direction];
    return anim ? anim->getCursor() : nullptr;
}

void SDL3CursorManager::initResources(Mouse* mouse)
{
    if (!mouse) return;

    for (Int cursor = Mouse::FIRST_CURSOR; cursor < Mouse::NUM_MOUSE_CURSORS; cursor++)
    {
        for (Int direction = 0; direction < mouse->m_cursorInfo[cursor].numDirections; direction++)
        {
            if (!m_cursorResources[cursor][direction] && !mouse->m_cursorInfo[cursor].textureName.isEmpty())
            {
                char resourcePath[256];
                if (mouse->m_cursorInfo[cursor].numDirections > 1)
                    snprintf(resourcePath, sizeof(resourcePath), "Data/Cursors/%s%d.ani", mouse->m_cursorInfo[cursor].textureName.str(), direction);
                else
                    snprintf(resourcePath, sizeof(resourcePath), "Data/Cursors/%s.ani", mouse->m_cursorInfo[cursor].textureName.str());

                m_cursorResources[cursor][direction] = loadANI(resourcePath);
                DEBUG_ASSERTCRASH(m_cursorResources[cursor][direction], ("MissingCursor %s\n", resourcePath));
            }
        }
    }
}

AnimatedCursor* SDL3CursorManager::loadANI(const char* filepath)
{
	File* file = TheFileSystem->openFile(filepath, File::READ | File::BINARY);
	if (!file)
	{
		DEBUG_LOG(("loadANI: Failed to open ANI cursor [%s]", filepath));
		return nullptr;
	}

	Int size = file->size();
	if (size <= 0)
	{
		DEBUG_LOG(("loadANI: File is empty [%s]", filepath));
		file->close();
		return nullptr;
	}

	std::unique_ptr<char[]> file_buffer(new char[size]);
	if (file->read(file_buffer.get(), size) != size)
	{
		DEBUG_LOG(("loadANI: Failed to read ANI cursor [%s]", filepath));
		file->close();
		return nullptr;
	}
	file->close();

	DEBUG_LOG(("loadANI: Loading %s", filepath));
	
    SDL_IOStream *io = SDL_IOFromConstMem(file_buffer.get(), (size_t)size);
    if (!io) return nullptr;

    // Use SDL3_image to load the animation (handles RIFF/ANI container and frame decoding)
    IMG_Animation *anim = IMG_LoadAnimation_IO(io, true);
    if (!anim) 
    {
        DEBUG_LOG(("loadANI: IMG_LoadAnimation_IO failed for [%s]: %s", filepath, SDL_GetError()));
        return nullptr;
    }

    if (anim->count == 0)
    {
        IMG_FreeAnimation(anim);
        return nullptr;
    }

    // Get hotspots from the first frame's properties (SDL3_image sets these for ICO/CUR)
    SDL_PropertiesID props = SDL_GetSurfaceProperties(anim->frames[0]);
    int hot_spot_x = (int)SDL_GetNumberProperty(props, SDL_PROP_SURFACE_HOTSPOT_X_NUMBER, 0);
    int hot_spot_y = (int)SDL_GetNumberProperty(props, SDL_PROP_SURFACE_HOTSPOT_Y_NUMBER, 0);

    // Create the animated cursor resource
    std::unique_ptr<AnimatedCursor> cursor(new AnimatedCursor());
    if (anim->count == 1)
    {
        cursor->m_cursor = SDL_CreateColorCursor(anim->frames[0], hot_spot_x, hot_spot_y);
    }
    else
    {
        std::vector<SDL_CursorFrameInfo> frames(anim->count);
        for (int i = 0; i < anim->count; i++)
        {
            frames[i].surface = anim->frames[i];
            frames[i].duration = (Uint32)anim->delays[i];
        }
        cursor->m_cursor = SDL_CreateAnimatedCursor(frames.data(), anim->count, hot_spot_x, hot_spot_y);
    }

    IMG_FreeAnimation(anim);
	return cursor.release();
}
