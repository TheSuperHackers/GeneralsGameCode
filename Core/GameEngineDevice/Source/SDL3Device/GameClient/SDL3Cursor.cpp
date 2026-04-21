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

// RIFF/ANI parsing helpers (moved from SDL3Input.cpp)
typedef std::array<char, 4> FourCC;
constexpr FourCC riff_id = {'R', 'I', 'F', 'F'};
constexpr FourCC acon_id = {'A', 'C', 'O', 'N'};
constexpr FourCC anih_id = {'a', 'n', 'i', 'h'};
constexpr FourCC fram_id = {'f', 'r', 'a', 'm'};
constexpr FourCC icon_id = {'i', 'c', 'o', 'n'};
constexpr FourCC list_id = {'L', 'I', 'S', 'T'};

struct ANIHeader
{
	uint32_t size;
	uint32_t frames;
	uint32_t steps;
	uint32_t width;
	uint32_t height;
	uint32_t bitsPerPixel;
	uint32_t planes;
	uint32_t displayRate;
	uint32_t flags;
};

struct RIFFChunk
{
	FourCC id;
	uint32_t size;
	FourCC type;
};

static RIFFChunk* getNextChunk(RIFFChunk* chunk, const char* buffer_end)
{
	if (!chunk) return nullptr;
	char* next = (char*)chunk + 8 + chunk->size;
	if (chunk->size % 2 != 0) next++;
	if (next >= buffer_end) return nullptr;
	return (RIFFChunk*)next;
}

static void* getChunkData(RIFFChunk* chunk)
{
	if (chunk->id == list_id || chunk->id == riff_id)
		return (char*)chunk + 12;
	return (char*)chunk + 8;
}

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
	if (size < (Int)sizeof(RIFFChunk))
	{
		DEBUG_LOG(("loadANI: File too small [%s]", filepath));
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

	char* buffer_start = file_buffer.get();
	char* buffer_end = buffer_start + size;

	RIFFChunk *riff_header = (RIFFChunk*)buffer_start;
	if (riff_header->id != riff_id || riff_header->type != acon_id)
	{
		DEBUG_LOG(("loadANI: Not a valid RIFF/ACON file [%s]", filepath));
		return nullptr;
	}

	DEBUG_LOG(("loadANI: Loading %s", filepath));
	
    std::vector<SDL_CursorFrameInfo> frames;
    int frameRate = 0;
    int hot_spot_x = 0;
    int hot_spot_y = 0;
    bool hot_spot_set = false;

	// Top level chunks start after the RIFF header (8 bytes + 'ACON' = 12 bytes)
	RIFFChunk* chunk = (RIFFChunk*)(buffer_start + 12);

	while (chunk != nullptr && (char *)chunk + 8 <= buffer_end)
	{
		if (chunk->id == anih_id)
		{
			if (chunk->size >= sizeof(ANIHeader))
			{
			    ANIHeader *ani_header = (ANIHeader*)getChunkData(chunk);
			    frameRate = ani_header->displayRate; // Display rate in 1/60th of a second
			}
		}
		else if (chunk->id == list_id && chunk->type == fram_id)
		{
			RIFFChunk *frame = (RIFFChunk*)((char *)chunk + 12);
			char* list_end = (char*)chunk + 8 + chunk->size;
			if (list_end > buffer_end) list_end = buffer_end;

			while (frame != nullptr && (char *)frame + 8 <= list_end)
			{
				if (frame->id == icon_id)
				{
					if ((char*)frame + 8 + frame->size <= list_end)
					{
						const void *frame_buffer = getChunkData(frame);
						SDL_IOStream *io_stream = SDL_IOFromConstMem(frame_buffer, frame->size);
						if (io_stream)
						{
							SDL_Surface *surface = IMG_LoadTyped_IO(io_stream, true, "ico");
							if (surface)
							{
                                if (!hot_spot_set)
                                {
								    SDL_PropertiesID props = SDL_GetSurfaceProperties(surface);
								    hot_spot_x = (int)SDL_GetNumberProperty(props, SDL_PROP_SURFACE_HOTSPOT_X_NUMBER, 0);
								    hot_spot_y = (int)SDL_GetNumberProperty(props, SDL_PROP_SURFACE_HOTSPOT_Y_NUMBER, 0);
                                    hot_spot_set = true;
                                }

                                SDL_CursorFrameInfo info;
                                info.surface = surface;
                                // SAGE's displayRate is in 1/60th of a second. SDL3 wants milliseconds.
                                info.duration = (frameRate * 1000) / 60;
                                frames.push_back(info);
							}
						}
					}
				}
				frame = getNextChunk(frame, list_end);
			}
		}
		chunk = getNextChunk(chunk, buffer_end);
	}

    if (frames.empty()) return nullptr;

    std::unique_ptr<AnimatedCursor> cursor(new AnimatedCursor());
    if (frames.size() == 1)
    {
        cursor->m_cursor = SDL_CreateColorCursor(frames[0].surface, hot_spot_x, hot_spot_y);
    }
    else
    {
        cursor->m_cursor = SDL_CreateAnimatedCursor(frames.data(), (int)frames.size(), hot_spot_x, hot_spot_y);
    }

    // Clean up all surfaces
    for (auto& f : frames)
    {
        SDL_DestroySurface(f.surface);
    }

	return cursor.release();
}
