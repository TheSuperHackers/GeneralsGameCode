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

// Derived from the GeneralsX branch by fbraz3

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

AnimatedCursor* SDL3CursorManager::m_cursorResources[Mouse::NUM_MOUSE_CURSORS][MAX_2D_CURSOR_DIRECTIONS] = {nullptr};

void SDL3CursorManager::init()
{
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
	if (cursor < 0 || cursor >= Mouse::NUM_MOUSE_CURSORS)
		return nullptr;
	if (direction < 0 || direction >= MAX_2D_CURSOR_DIRECTIONS)
		direction = 0;

	AnimatedCursor* anim = m_cursorResources[cursor][direction];
	return anim ? anim->getCursor() : nullptr;
}

void SDL3CursorManager::initResources(Mouse* mouse)
{
	if (!mouse)
		return;

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

	// thesuperhackers @info
	// Command & Conquer Generals .ani files write the total file size as the `cbSize`
	// field inside the RIFF header (offset 4) rather than `size - 8` bytes.
	// The native SDL3_image ANI parser strictly validates that (offset + chunk_size) <= cbSize,
	// causing it to fail at EOF with a truncated data error.
	if (size >= 12 && memcmp(buf.data(), "RIFF", 4) == 0 && memcmp(buf.data() + 8, "ACON", 4) == 0)
	{
		Uint32 correct_cbSize = (Uint32)(size - 8);
		memcpy(buf.data() + 4, &correct_cbSize, 4);
	}

	SDL_IOStream* io = SDL_IOFromConstMem(buf.data(), buf.size());
	if (!io)
	{
		return nullptr;
	}

	IMG_Animation* anim = IMG_LoadAnimation_IO(io, true);
	if (!anim)
	{
		return nullptr;
	}

	if (anim->count <= 0)
	{
		IMG_FreeAnimation(anim);
		return nullptr;
	}

	int hot_spot_x = 0, hot_spot_y = 0;
	if (anim->frames && anim->frames[0])
	{
		SDL_PropertiesID pr = SDL_GetSurfaceProperties(anim->frames[0]);
		hot_spot_x = (int)SDL_GetNumberProperty(pr, SDL_PROP_SURFACE_HOTSPOT_X_NUMBER, 0);
		hot_spot_y = (int)SDL_GetNumberProperty(pr, SDL_PROP_SURFACE_HOTSPOT_Y_NUMBER, 0);
	}

	std::unique_ptr<AnimatedCursor> cursor(new AnimatedCursor());

	if (anim->count > 1)
	{
		std::vector<SDL_CursorFrameInfo> sdl_frames(anim->count);
		for (int i = 0; i < anim->count; ++i)
		{
			sdl_frames[i].surface = anim->frames[i];
			sdl_frames[i].duration = anim->delays[i];
		}
		cursor->m_cursor = SDL_CreateAnimatedCursor(sdl_frames.data(), anim->count, hot_spot_x, hot_spot_y);
	}
	else
	{
		cursor->m_cursor = SDL_CreateColorCursor(anim->frames[0], hot_spot_x, hot_spot_y);
	}

	if (!cursor->m_cursor)
	{
		DEBUG_LOG(("loadANI: Failed to create cursor from %s. hot=(%d, %d), count=%d. Error: %s", filepath, hot_spot_x, hot_spot_y, anim->count, SDL_GetError()));
	}

	IMG_FreeAnimation(anim);
	return cursor.release();
}
