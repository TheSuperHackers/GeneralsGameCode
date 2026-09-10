#include "SDL3Device/GameClient/SDL3Mouse.h"

#include "Common/GlobalData.h"
#include "Common/LocalFileSystem.h"
#include "GameClient/Display.h"

#include <algorithm>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

namespace
{
struct ParsedAni
{
	std::vector<SDL_Surface *> frames;
	std::vector<Uint32> rates;
	std::vector<Uint32> sequence;
	Int hotX = -1;
	Int hotY = -1;
};

Int toGameCoordinate(float coordinate, Int gameExtent, int windowExtent)
{
	if (gameExtent <= 0 || windowExtent <= 0)
	{
		return static_cast<Int>(coordinate);
	}

	const float scaled = coordinate * static_cast<float>(gameExtent) / static_cast<float>(windowExtent);
	return std::clamp(static_cast<Int>(scaled), 0, gameExtent);
}

Uint16 readLE16(const Uint8 *data)
{
	return static_cast<Uint16>(data[0] | (static_cast<Uint16>(data[1]) << 8));
}

Uint32 readLE32(const Uint8 *data)
{
	return static_cast<Uint32>(data[0]) |
		(static_cast<Uint32>(data[1]) << 8) |
		(static_cast<Uint32>(data[2]) << 16) |
		(static_cast<Uint32>(data[3]) << 24);
}

bool isChunk(const Uint8 *data, const char (&name)[5])
{
	return std::memcmp(data, name, 4) == 0;
}

void destroySurfaces(std::vector<SDL_Surface *> &surfaces)
{
	for (SDL_Surface *surface : surfaces)
	{
		if (surface != nullptr)
			SDL_DestroySurface(surface);
	}
	surfaces.clear();
}

bool hasBytes(size_t offset, size_t count, size_t size)
{
	return offset <= size && count <= size - offset;
}

SDL_Surface *parseIconSurface(const Uint8 *iconData, size_t iconSize, Int *hotX, Int *hotY)
{
	// The icon chunks embedded in an ANI file contain a normal .CUR image:
	// ICONDIR, one CURDIRENTRY, then a Windows DIB and its AND mask.
	if (!hasBytes(0, 22, iconSize) || readLE16(iconData) != 0 ||
		readLE16(iconData + 2) != 2 || readLE16(iconData + 4) == 0)
	{
		return nullptr;
	}

	const Uint8 *entry = iconData + 6;
	const Uint32 imageSize = readLE32(entry + 8);
	const Uint32 imageOffset = readLE32(entry + 12);
	if (imageSize == 0 || !hasBytes(imageOffset, imageSize, iconSize))
		return nullptr;

	const Uint8 *image = iconData + imageOffset;
	const size_t imageLength = imageSize;
	if (!hasBytes(0, 4, imageLength))
		return nullptr;

	const Uint32 headerSize = readLE32(image);
	Int width = 0;
	Int height = 0;
	Uint16 bitsPerPixel = 0;
	Uint32 compression = 0;
	Uint32 colorsUsed = 0;
	Bool topDown = FALSE;
	size_t paletteEntrySize = 4;

	if (headerSize == 12)
	{
		if (!hasBytes(0, 12, imageLength))
			return nullptr;
		width = static_cast<Int>(readLE16(image + 4));
		const Int dibHeight = static_cast<Int>(readLE16(image + 6));
		height = dibHeight / 2;
		bitsPerPixel = readLE16(image + 10);
		paletteEntrySize = 3;
	}
	else
	{
		if (headerSize < 40 || !hasBytes(0, headerSize, imageLength))
			return nullptr;

		const Sint32 dibWidth = static_cast<Sint32>(readLE32(image + 4));
		const Sint32 dibHeight = static_cast<Sint32>(readLE32(image + 8));
		if (dibWidth <= 0 || dibHeight == 0)
			return nullptr;

		width = dibWidth;
		const Sint64 absoluteHeight = dibHeight < 0 ? -static_cast<Sint64>(dibHeight) : dibHeight;
		height = static_cast<Int>(absoluteHeight / 2);
		topDown = dibHeight < 0 ? TRUE : FALSE;
		bitsPerPixel = readLE16(image + 14);
		compression = readLE32(image + 16);
		colorsUsed = readLE32(image + 32);
	}

	// Generals' cursor files are small indexed DIBs. Keep malformed files from
	// causing oversized allocations while still accepting the standard cursor
	// sizes used by Windows and SDL.
	if (width <= 0 || height <= 0 || width > 1024 || height > 1024 ||
		(bitsPerPixel != 1 && bitsPerPixel != 4 && bitsPerPixel != 8 &&
		 bitsPerPixel != 24 && bitsPerPixel != 32) || compression != 0)
	{
		return nullptr;
	}

	const Uint32 defaultPaletteCount = bitsPerPixel <= 8 ? (1u << bitsPerPixel) : 0;
	const Uint32 paletteCount = bitsPerPixel <= 8 ?
		(colorsUsed != 0 ? colorsUsed : defaultPaletteCount) : 0;
	if (paletteCount > 256)
		return nullptr;

	const size_t paletteOffset = headerSize;
	const size_t pixelOffset = paletteOffset + static_cast<size_t>(paletteCount) * paletteEntrySize;
	const size_t xorStride = ((static_cast<size_t>(width) * bitsPerPixel + 31) / 32) * 4;
	const size_t maskStride = ((static_cast<size_t>(width) + 31) / 32) * 4;
	const size_t xorBytes = xorStride * static_cast<size_t>(height);
	const size_t maskBytes = maskStride * static_cast<size_t>(height);
	if (pixelOffset < paletteOffset || !hasBytes(pixelOffset, xorBytes + maskBytes, imageLength))
		return nullptr;

	SDL_Surface *surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_ARGB8888);
	if (surface == nullptr)
		return nullptr;

	const SDL_PixelFormatDetails *format = SDL_GetPixelFormatDetails(surface->format);
	if (format == nullptr)
	{
		SDL_DestroySurface(surface);
		return nullptr;
	}

	Bool hasSourceAlpha = FALSE;
	if (bitsPerPixel == 32)
	{
		for (Int y = 0; y < height && !hasSourceAlpha; ++y)
		{
			const size_t sourceY = topDown ? static_cast<size_t>(y) : static_cast<size_t>(height - 1 - y);
			const Uint8 *row = image + pixelOffset + sourceY * xorStride;
			for (Int x = 0; x < width; ++x)
			{
				if (row[x * 4 + 3] != 0)
				{
					hasSourceAlpha = TRUE;
					break;
				}
			}
		}
	}

	for (Int y = 0; y < height; ++y)
	{
		const size_t sourceY = topDown ? static_cast<size_t>(y) : static_cast<size_t>(height - 1 - y);
		const Uint8 *colorRow = image + pixelOffset + sourceY * xorStride;
		const Uint8 *maskRow = image + pixelOffset + xorBytes +
			static_cast<size_t>(height - 1 - y) * maskStride;
		Uint32 *destinationRow = reinterpret_cast<Uint32 *>(
			static_cast<Uint8 *>(surface->pixels) + static_cast<size_t>(y) * surface->pitch);

		for (Int x = 0; x < width; ++x)
		{
			Uint8 red = 0;
			Uint8 green = 0;
			Uint8 blue = 0;
			Uint8 alpha = 255;

			if (bitsPerPixel <= 8)
			{
				Uint8 paletteIndex = 0;
				if (bitsPerPixel == 1)
					paletteIndex = static_cast<Uint8>((colorRow[x / 8] >> (7 - (x & 7))) & 1);
				else if (bitsPerPixel == 4)
					paletteIndex = static_cast<Uint8>((colorRow[x / 2] >> ((x & 1) ? 0 : 4)) & 0x0f);
				else
					paletteIndex = colorRow[x];

				const size_t paletteEntry = paletteOffset + static_cast<size_t>(paletteIndex) * paletteEntrySize;
				if (paletteIndex >= paletteCount || !hasBytes(paletteEntry, paletteEntrySize, imageLength))
				{
					SDL_DestroySurface(surface);
					return nullptr;
				}
				blue = image[paletteEntry];
				green = image[paletteEntry + 1];
				red = image[paletteEntry + 2];
			}
			else if (bitsPerPixel == 24)
			{
				const Uint8 *pixel = colorRow + x * 3;
				blue = pixel[0];
				green = pixel[1];
				red = pixel[2];
			}
			else
			{
				const Uint8 *pixel = colorRow + x * 4;
				blue = pixel[0];
				green = pixel[1];
				red = pixel[2];
				alpha = pixel[3];
			}

			const Bool masked = (maskRow[x / 8] & (1u << (7 - (x & 7)))) != 0;
			if (masked)
				alpha = 0;
			else if (bitsPerPixel != 32 || !hasSourceAlpha)
				alpha = 255;

			destinationRow[x] = SDL_MapRGBA(format, nullptr, red, green, blue, alpha);
		}
	}

	if (hotX != nullptr)
		*hotX = static_cast<Int>(readLE16(entry + 4));
	if (hotY != nullptr)
		*hotY = static_cast<Int>(readLE16(entry + 6));
	return surface;
}

bool parseAniChunks(const Uint8 *data, size_t begin, size_t end, ParsedAni *result)
{
	size_t offset = begin;
	while (offset < end)
	{
		if (!hasBytes(offset, 8, end))
			return false;

		const Uint8 *chunk = data + offset;
		const Uint32 chunkSize = readLE32(chunk + 4);
		const size_t payload = offset + 8;
		if (!hasBytes(payload, chunkSize, end))
			return false;
		const size_t payloadEnd = payload + chunkSize;

		if (isChunk(chunk, "LIST"))
		{
			if (chunkSize < 4 || !parseAniChunks(data, payload + 4, payloadEnd, result))
				return false;
		}
		else if (isChunk(chunk, "icon"))
		{
			Int hotX = -1;
			Int hotY = -1;
			SDL_Surface *surface = parseIconSurface(data + payload, chunkSize, &hotX, &hotY);
			if (surface == nullptr)
				return false;
			if (result->frames.empty())
			{
				result->hotX = hotX;
				result->hotY = hotY;
			}
			result->frames.push_back(surface);
		}
		else if (isChunk(chunk, "rate"))
		{
			for (size_t rateOffset = 0; rateOffset + 4 <= chunkSize; rateOffset += 4)
				result->rates.push_back(readLE32(data + payload + rateOffset));
		}
		else if (isChunk(chunk, "seq "))
		{
			for (size_t sequenceOffset = 0; sequenceOffset + 4 <= chunkSize; sequenceOffset += 4)
				result->sequence.push_back(readLE32(data + payload + sequenceOffset));
		}

		const size_t paddedSize = static_cast<size_t>(chunkSize) + (chunkSize & 1u);
		if (!hasBytes(payload, paddedSize, end))
			return false;
		offset = payload + paddedSize;
	}
	return true;
}

bool parseAni(const std::vector<Uint8> &data, ParsedAni *result)
{
	if (data.size() < 12 || !isChunk(data.data(), "RIFF") ||
		std::memcmp(data.data() + 8, "ACON", 4) != 0)
	{
		return false;
	}

	const Uint32 riffSize = readLE32(data.data() + 4);
	const size_t riffEnd = std::min(data.size(), static_cast<size_t>(8) + riffSize);
	if (riffEnd < 12 || !parseAniChunks(data.data(), 12, riffEnd, result) || result->frames.empty())
		return false;
	return true;
}

bool readFile(const std::string &path, std::vector<Uint8> *data)
{
	if (TheLocalFileSystem == nullptr)
		return false;

	File *file = TheLocalFileSystem->openFile(path.c_str(), File::READ | File::BINARY);
	if (file == nullptr)
		return false;

	const Int fileSize = file->size();
	char *contents = file->readEntireAndClose();
	if (contents == nullptr || fileSize <= 0)
	{
		delete[] contents;
		return false;
	}

	const Uint8 *begin = reinterpret_cast<const Uint8 *>(contents);
	data->assign(begin, begin + fileSize);
	delete[] contents;
	return true;
}

std::string cursorAssetName(const CursorInfo &cursorInfo, Int direction)
{
	std::string name = "data\\cursors\\";
	name += cursorInfo.textureName.str();
	if (cursorInfo.numDirections > 1)
		name += std::to_string(direction);
	name += ".ANI";
	return name;
}

bool readCursorAsset(const CursorInfo &cursorInfo, Int direction, std::vector<Uint8> *data)
{
	const std::string name = cursorAssetName(cursorInfo, direction);
	if (TheLocalFileSystem == nullptr)
		return false;

	// Match Win32Mouse: a mod-provided loose cursor takes precedence over the
	// installed game cursor, while the base cursor remains a normal local file.
	if (TheGlobalData != nullptr && !TheGlobalData->m_modDir.isEmpty())
	{
		const std::string modName = std::string(TheGlobalData->m_modDir.str()) + name;
		if (TheLocalFileSystem->doesFileExist(modName.c_str()) && readFile(modName, data))
			return true;
	}

	return readFile(name, data);
}

bool orderAniFrames(ParsedAni *ani)
{
	if (ani->sequence.empty())
		return true;

	std::vector<SDL_Surface *> source = std::move(ani->frames);
	std::vector<SDL_Surface *> ordered;
	std::vector<SDL_Surface *> duplicates;
	std::vector<Bool> used(source.size(), FALSE);
	for (Uint32 index : ani->sequence)
	{
		if (index >= source.size())
			continue;

		SDL_Surface *frame = nullptr;
		if (used[index])
		{
			frame = SDL_DuplicateSurface(source[index]);
			if (frame != nullptr)
				duplicates.push_back(frame);
		}
		else
		{
			frame = source[index];
			used[index] = TRUE;
		}

		if (frame == nullptr)
		{
			destroySurfaces(duplicates);
			destroySurfaces(source);
			return false;
		}
		ordered.push_back(frame);
	}

	if (ordered.empty())
	{
		destroySurfaces(source);
		return false;
	}

	for (size_t i = 0; i < source.size(); ++i)
	{
		if (!used[i])
			SDL_DestroySurface(source[i]);
	}
	source.clear();
	ani->frames.swap(ordered);
	return true;
}

bool normalizeAniFrames(ParsedAni *ani)
{
	if (ani->frames.empty())
		return false;

	const Int width = ani->frames.front()->w;
	const Int height = ani->frames.front()->h;
	for (SDL_Surface *&frame : ani->frames)
	{
		if (frame->w == width && frame->h == height)
			continue;

		SDL_Surface *scaled = SDL_ScaleSurface(frame, width, height, SDL_SCALEMODE_NEAREST);
		if (scaled == nullptr)
		{
			destroySurfaces(ani->frames);
			return false;
		}
		SDL_DestroySurface(frame);
		frame = scaled;
	}
	return true;
}

Uint32 aniFrameDuration(const ParsedAni &ani, size_t frame, const CursorInfo &cursorInfo, size_t frameCount)
{
	if (frame < ani.rates.size() && ani.rates[frame] != 0)
	{
		// ANI rates are in 1/60 second jiffies.
		const Uint64 milliseconds = (static_cast<Uint64>(ani.rates[frame]) * 1000 + 30) / 60;
		return static_cast<Uint32>(std::max<Uint64>(1, std::min<Uint64>(milliseconds, 0xffffffffu)));
	}

	if (frameCount <= 1)
		return 0;
	const Int framesPerSecond = static_cast<Int>(cursorInfo.fps);
	return framesPerSecond > 0 ? static_cast<Uint32>(std::max(1, 1000 / framesPerSecond)) : 50;
}

bool addHighDpiCursorImage(SDL_Surface *surface, Int hotX, Int hotY)
{
	if (surface == nullptr || surface->w <= 0 || surface->h <= 0)
		return false;

	// The original cursor art is the 100% representation. SDL's Windows
	// backend uses that representation unless a high-DPI alternate is attached
	// and SDL_HINT_MOUSE_DPI_SCALE_CURSORS is enabled. Keep the game art crisp
	// by providing a native 200% image instead of letting Windows enlarge a
	// 32x32 cursor at the last possible stage.
	SDL_Surface *highDpi = SDL_ScaleSurface(surface, surface->w * 2, surface->h * 2, SDL_SCALEMODE_NEAREST);
	if (highDpi == nullptr)
		return false;

	const SDL_PropertiesID properties = SDL_GetSurfaceProperties(highDpi);
	const bool hotspotSet = properties != 0 &&
		SDL_SetNumberProperty(properties, SDL_PROP_SURFACE_HOTSPOT_X_NUMBER, static_cast<Sint64>(hotX) * 2) &&
		SDL_SetNumberProperty(properties, SDL_PROP_SURFACE_HOTSPOT_Y_NUMBER, static_cast<Sint64>(hotY) * 2);
	const bool added = hotspotSet && SDL_AddSurfaceAlternateImage(surface, highDpi);
	SDL_DestroySurface(highDpi);
	return added;
}

SDL_Cursor *createGameCursor(const CursorInfo &cursorInfo, Int direction)
{
	std::vector<Uint8> data;
	if (!readCursorAsset(cursorInfo, direction, &data))
		return nullptr;

	ParsedAni ani;
	if (!parseAni(data, &ani) || !orderAniFrames(&ani) || !normalizeAniFrames(&ani))
	{
		destroySurfaces(ani.frames);
		return nullptr;
	}

	const Int width = ani.frames.front()->w;
	const Int height = ani.frames.front()->h;
	const Int fallbackHotX = std::clamp(cursorInfo.hotSpotPosition.x, 0, width - 1);
	const Int fallbackHotY = std::clamp(cursorInfo.hotSpotPosition.y, 0, height - 1);
	const Int hotX = std::clamp(ani.hotX >= 0 ? ani.hotX : fallbackHotX, 0, width - 1);
	const Int hotY = std::clamp(ani.hotY >= 0 ? ani.hotY : fallbackHotY, 0, height - 1);
	for (SDL_Surface *frame : ani.frames)
	{
		if (!addHighDpiCursorImage(frame, hotX, hotY))
		{
			destroySurfaces(ani.frames);
			return nullptr;
		}
	}

	std::vector<SDL_CursorFrameInfo> frames;
	frames.reserve(ani.frames.size());
	for (size_t i = 0; i < ani.frames.size(); ++i)
		frames.push_back({ ani.frames[i], aniFrameDuration(ani, i, cursorInfo, ani.frames.size()) });

	SDL_Cursor *cursor = SDL_CreateAnimatedCursor(frames.data(), static_cast<int>(frames.size()), hotX, hotY);
	destroySurfaces(ani.frames);
	return cursor;
}
} // namespace

SDL3Mouse::~SDL3Mouse()
{
	SDL_SetCursor(nullptr);
	for (Int cursor = 0; cursor < NUM_MOUSE_CURSORS; ++cursor)
	{
		for (Int direction = 0; direction < MAX_2D_CURSOR_DIRECTIONS; ++direction)
		{
			if (m_cursorResources[cursor][direction] != nullptr)
				SDL_DestroyCursor(m_cursorResources[cursor][direction]);
		}
	}

	if (m_cursor != nullptr)
	{
		SDL_DestroyCursor(m_cursor);
		m_cursor = nullptr;
	}
}

void SDL3Mouse::init()
{
	Mouse::init();
	// SDL_GetMouseState returns absolute window coordinates.  The legacy
	// Mouse base defaults to relative processing, which would add those
	// coordinates every frame and clamp the cursor at the edge.
	m_inputMovesAbsolute = TRUE;
	// Mouse::init() restores the retail 800x600 limits. Refresh them after
	// switching to SDL coordinates so the full GeneralsMD display is usable.
	setMouseLimits();
	m_previousButtons = 0;
	m_previousX = 0;
	m_previousY = 0;
	m_reportedThisFrame = FALSE;
	m_pendingWheel = 0.0f;
	m_cursorDirection = 0;
}
void SDL3Mouse::reset()
{
	Mouse::reset();
	m_previousButtons = 0;
	m_previousX = m_previousY = 0;
	m_pendingWheel = 0.0f;
	m_cursorDirection = 0;
}
void SDL3Mouse::update()
{
	SDL_PumpEvents();
	m_reportedThisFrame = FALSE;
	Mouse::update();
}
void SDL3Mouse::addWheelDelta(Real wheelDelta)
{
	m_pendingWheel += wheelDelta;
}

void SDL3Mouse::initCursorResources()
{
	// Ask SDL to select alternate cursor images for the monitor's content
	// scale. The source ANI files remain the 100% game representation.
	SDL_SetHint(SDL_HINT_MOUSE_DPI_SCALE_CURSORS, "1");

	for (Int cursor = FIRST_CURSOR; cursor < NUM_MOUSE_CURSORS; ++cursor)
	{
		if (m_cursorInfo[cursor].textureName.isEmpty())
			continue;

		const Int directionCount = std::clamp(m_cursorInfo[cursor].numDirections, 1, MAX_2D_CURSOR_DIRECTIONS);
		for (Int direction = 0; direction < directionCount; ++direction)
			loadCursorResource(static_cast<MouseCursor>(cursor), direction);
	}
}

Bool SDL3Mouse::loadCursorResource(MouseCursor cursor, Int direction)
{
	if (cursor < FIRST_CURSOR || cursor >= NUM_MOUSE_CURSORS ||
		direction < 0 || direction >= MAX_2D_CURSOR_DIRECTIONS)
	{
		return FALSE;
	}

	if (m_cursorResources[cursor][direction] != nullptr)
		return TRUE;
	if (m_cursorResourceAttempted[cursor][direction])
		return FALSE;
	if (TheLocalFileSystem == nullptr)
		return FALSE;

	m_cursorResourceAttempted[cursor][direction] = TRUE;
	m_cursorResources[cursor][direction] = createGameCursor(m_cursorInfo[cursor], direction);
	if (m_cursorResources[cursor][direction] == nullptr)
	{
		DEBUG_LOG(("SDL3Mouse: unable to load game cursor %s (direction %d)",
			m_cursorInfo[cursor].textureName.str(), direction));
		return FALSE;
	}
	return TRUE;
}
void SDL3Mouse::setVisibility(Bool visible)
{
	Mouse::setVisibility(visible);
	applyCursor();
}

void SDL3Mouse::loseFocus()
{
	Mouse::loseFocus();
	m_cursorFocused = FALSE;
	SDL_HideCursor();
}

void SDL3Mouse::regainFocus()
{
	Mouse::regainFocus();
	m_cursorFocused = TRUE;
	applyCursor();
}

SDL_SystemCursor SDL3Mouse::systemCursorFor(MouseCursor cursor)
{
	switch (cursor)
	{
	case NONE:
		return SDL_SYSTEM_CURSOR_DEFAULT;
	case NORMAL:
	case ARROW:
		return SDL_SYSTEM_CURSOR_DEFAULT;
	case CROSS:
	case INVALID_BUILD_PLACEMENT:
	case GENERIC_INVALID:
		return SDL_SYSTEM_CURSOR_CROSSHAIR;
	case MOVETO:
	case ATTACKMOVETO:
	case SELECTING:
		return SDL_SYSTEM_CURSOR_POINTER;
	case SCROLL:
		return SDL_SYSTEM_CURSOR_MOVE;
	default:
		return SDL_SYSTEM_CURSOR_DEFAULT;
	}
}

void SDL3Mouse::setCursor(MouseCursor cursor)
{
	setCursorWithDirection(cursor, 0);
}

void SDL3Mouse::setCursorWithDirection(MouseCursor cursor, Int directionFrame)
{
	if (cursor < NONE || cursor >= NUM_MOUSE_CURSORS)
	{
		return;
	}

	m_cursorDirection = directionFrame;
	Mouse::setCursor(cursor);
	m_currentCursor = cursor;
	applyCursor();
}

void SDL3Mouse::applyCursor()
{
	if (m_currentCursor == NONE || !m_cursorFocused)
	{
		SDL_HideCursor();
		return;
	}

	const Int directionCount = std::clamp(m_cursorInfo[m_currentCursor].numDirections, 1, MAX_2D_CURSOR_DIRECTIONS);
	const Int direction = std::clamp(m_cursorDirection, 0, directionCount - 1);
	loadCursorResource(m_currentCursor, direction);
	SDL_Cursor *cursor = m_cursorResources[m_currentCursor][direction];

	if (cursor == nullptr)
	{
		const SDL_SystemCursor cursorType = systemCursorFor(m_currentCursor);
		if (m_cursor == nullptr || m_cursorType != cursorType)
		{
			if (m_cursor != nullptr)
				SDL_DestroyCursor(m_cursor);

			m_cursor = SDL_CreateSystemCursor(cursorType);
			m_cursorType = cursorType;
		}
		cursor = m_cursor;
	}

	if (cursor != nullptr)
	{
		SDL_SetCursor(cursor);
	}
	else
	{
		SDL_SetCursor(SDL_GetDefaultCursor());
	}

	if (m_visible)
	{
		SDL_ShowCursor();
	}
	else
	{
		SDL_HideCursor();
	}
}

void SDL3Mouse::capture()
{
	onCursorCaptured(SDL_CaptureMouse(true) ? TRUE : FALSE);
}

void SDL3Mouse::releaseCapture()
{
	SDL_CaptureMouse(false);
	onCursorCaptured(FALSE);
}

UnsignedByte SDL3Mouse::getMouseEvent(MouseIO *result, Bool)
{
	if (m_reportedThisFrame)
		return MOUSE_NONE;
	m_reportedThisFrame = TRUE;
	float x = 0.0f;
	float y = 0.0f;
	const Uint32 buttons = SDL_GetMouseState(&x, &y);

	int windowWidth = 0;
	int windowHeight = 0;
	if (SDL_Window *window = SDL_GetMouseFocus(); window != nullptr)
	{
		SDL_GetWindowSize(window, &windowWidth, &windowHeight);
	}

	const Int gameWidth = TheDisplay != nullptr ? TheDisplay->getWidth() : 0;
	const Int gameHeight = TheDisplay != nullptr ? TheDisplay->getHeight() : 0;
	const Int gameX = toGameCoordinate(x, gameWidth, windowWidth);
	const Int gameY = toGameCoordinate(y, gameHeight, windowHeight);
	result->pos.x = gameX;
	result->pos.y = gameY;
	result->deltaPos.x = gameX - m_previousX;
	result->deltaPos.y = gameY - m_previousY;
	const Real wheelUnits = m_pendingWheel * static_cast<Real>(MOUSE_WHEEL_DELTA);
	result->wheelPos = static_cast<Int>(wheelUnits);
	m_pendingWheel -= result->wheelPos / static_cast<Real>(MOUSE_WHEEL_DELTA);
	result->time = SDL_GetTicks();
	const auto transition = [buttons, this](Uint32 mask) {
		const bool wasDown = (m_previousButtons & mask) != 0;
		const bool isDown = (buttons & mask) != 0;
		if (isDown && !wasDown) return MBS_Down;
		if (!isDown && wasDown) return MBS_Up;
		return MBS_None;
	};
	result->leftState = transition(SDL_BUTTON_LMASK);
	result->rightState = transition(SDL_BUTTON_RMASK);
	result->middleState = transition(SDL_BUTTON_MMASK);
	result->leftEvent = result->rightEvent = result->middleEvent = 0;
	m_previousButtons = buttons;
	m_previousX = gameX;
	m_previousY = gameY;
	return MOUSE_OK;
}
