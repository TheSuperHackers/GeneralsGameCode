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
		return nullptr;
	}

	Int size = file->size();
	std::vector<char> buf(size);
	file->read(buf.data(), size);
	file->close();

    std::vector<SDL_CursorFrameInfo> frames;
    int hot_spot_x = 0, hot_spot_y = 0;
    Uint32 rate = 1;

    // Detect RIFF/ACON container
    if (buf.size() >= 12 && memcmp(buf.data(), "RIFF", 4) == 0 && memcmp(buf.data() + 8, "ACON", 4) == 0)
    {
        char* p = buf.data() + 12;
        char* end = buf.data() + buf.size();
        while (p + 8 <= end)
        {
            Uint32 id, sz;
            memcpy(&id, p, 4);
            memcpy(&sz, p + 4, 4);
            p += 8;

            if (id == *(Uint32*)"anih" && sz >= 36)
            {
                memcpy(&rate, p + 28, 4);
            }
            else if (id == *(Uint32*)"LIST" && sz >= 4 && memcmp(p, "fram", 4) == 0)
            {
                char* lp = p + 4;
                char* le = p + sz;
                while (lp + 8 <= le)
                {
                    Uint32 fid, fsz;
                    memcpy(&fid, lp, 4);
                    memcpy(&fsz, lp + 4, 4);
                    lp += 8;

                    if (fid == *(Uint32*)"icon")
                    {
                        SDL_IOStream* io = SDL_IOFromConstMem(lp, fsz);
                        SDL_Surface* s = IMG_LoadTyped_IO(io, true, "ico");
                        if (s)
                        {
                            if (frames.empty())
                            {
                                SDL_PropertiesID pr = SDL_GetSurfaceProperties(s);
                                hot_spot_x = (int)SDL_GetNumberProperty(pr, SDL_PROP_SURFACE_HOTSPOT_X_NUMBER, 0);
                                hot_spot_y = (int)SDL_GetNumberProperty(pr, SDL_PROP_SURFACE_HOTSPOT_Y_NUMBER, 0);
                            }
                            frames.push_back({ s, (Uint32)(rate * 1000 / 60) });
                        }
                    }
                    lp += (fsz + (fsz & 1));
                }
            }
            p += (sz + (sz & 1));
        }
    }
    else
    {
        // Fallback for direct ICO/CUR files
        SDL_IOStream* io = SDL_IOFromConstMem(buf.data(), buf.size());
        SDL_Surface* s = IMG_LoadTyped_IO(io, true, "ico");
        if (s)
        {
            SDL_PropertiesID pr = SDL_GetSurfaceProperties(s);
            hot_spot_x = (int)SDL_GetNumberProperty(pr, SDL_PROP_SURFACE_HOTSPOT_X_NUMBER, 0);
            hot_spot_y = (int)SDL_GetNumberProperty(pr, SDL_PROP_SURFACE_HOTSPOT_Y_NUMBER, 0);
            frames.push_back({ s, 16 });
        }
    }

    if (frames.empty())
    {
        return nullptr;
    }

    std::unique_ptr<AnimatedCursor> cursor(new AnimatedCursor());
    if (frames.size() > 1)
    {
        cursor->m_cursor = SDL_CreateAnimatedCursor(frames.data(), (int)frames.size(), hot_spot_x, hot_spot_y);
    }
    else
    {
        cursor->m_cursor = SDL_CreateColorCursor(frames[0].surface, hot_spot_x, hot_spot_y);
    }

    for (auto& f : frames)
    {
        SDL_DestroySurface(f.surface);
    }

    return cursor.release();
}
