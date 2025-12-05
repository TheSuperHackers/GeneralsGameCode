#include "SDL3Device/GameClient/SDL3Mouse.h"

#include "Common/AsciiString.h"
#include "Common/Debug.h"
#include "Common/GlobalData.h"
#include "Common/LocalFileSystem.h"
#include "GameClient/GameClient.h"
#include "SDL3Device/GameClient/SDL3AniReader.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_properties.h>

#include <cmath>
#include <cstring>

SDL3Mouse *SDL3Mouse::s_instance = NULL;

namespace
{
	static Int clampValue(Int value, Int minValue, Int maxValue)
	{
		if (value < minValue)
			return minValue;
		if (value > maxValue)
			return maxValue;
		return value;
	}

	static Int roundFloatToInt(float value)
	{
		if (value >= 0.0f)
			return static_cast<Int>(value + 0.5f);
		return static_cast<Int>(value - 0.5f);
	}
}

SDL3Mouse::SDL3Mouse(void)
		: m_nextFreeIndex(0),
			m_nextGetIndex(0),
			m_directionFrame(0),
			m_lostFocus(FALSE),
			m_windowID(0),
			m_activeCursor(NULL)
{
	s_instance = this;

	memset(&m_eventBuffer, 0, sizeof(m_eventBuffer));
}

SDL3Mouse::~SDL3Mouse(void)
{
	if (m_activeCursor != NULL)
	{
		SDL_SetCursor(SDL_GetDefaultCursor());
		m_activeCursor = NULL;
	}

	for (Int cursor = 0; cursor < Mouse::NUM_MOUSE_CURSORS; ++cursor)
	{
		for (Int dir = 0; dir < MAX_2D_CURSOR_DIRECTIONS; ++dir)
		{
			CursorFrames &frames = m_cursorResources[cursor][dir];
			if (!frames.loaded)
				continue;
			for (size_t i = 0; i < frames.frames.size(); ++i)
			{
				if (frames.frames[i].cursor != NULL)
				{
					SDL_DestroyCursor(frames.frames[i].cursor);
					frames.frames[i].cursor = NULL;
				}
			}
			frames.frames.clear();
			frames.loaded = FALSE;
		}
	}

	if (s_instance == this)
		s_instance = NULL;
}

void SDL3Mouse::init(void)
{
	Mouse::init();
	m_inputMovesAbsolute = TRUE;
	m_animationState = ActiveCursorState();
	m_currentCursor = NONE;
	m_activeCursor = NULL;
	SDL_ShowCursor();
}

void SDL3Mouse::reset(void)
{
	Mouse::reset();
	m_nextFreeIndex = 0;
	m_nextGetIndex = 0;
	memset(&m_eventBuffer, 0, sizeof(m_eventBuffer));
	m_animationState = ActiveCursorState();
	m_currentCursor = NONE;
	m_activeCursor = NULL;
}

void SDL3Mouse::update(void)
{
	Mouse::update();

	if (!m_lostFocus && m_visible && m_currentCursor >= FIRST_CURSOR && m_currentCursor < Mouse::NUM_MOUSE_CURSORS)
	{
		Int newDirection = computeDirection(m_currentCursor);
		if (newDirection != m_directionFrame)
		{
			m_directionFrame = newDirection;
			applyCursor(m_currentCursor, m_directionFrame, TRUE);
		}

		updateCursorAnimation();
	}
	clampCursorToWindow();
}

void SDL3Mouse::handleEvent(const SDL_Event &event)
{
	switch (event.type)
	{
	case SDL_EVENT_MOUSE_MOTION:
	case SDL_EVENT_MOUSE_BUTTON_DOWN:
	case SDL_EVENT_MOUSE_BUTTON_UP:
	case SDL_EVENT_MOUSE_WHEEL:
		addSDL3Event(event);
		break;
	case SDL_EVENT_WINDOW_FOCUS_LOST:
		loseFocus();
		break;
	case SDL_EVENT_WINDOW_FOCUS_GAINED:
		regainFocus();
		break;
	default:
		break;
	}
}

void SDL3Mouse::addSDL3Event(const SDL_Event &event)
{
	if (event.type == SDL_EVENT_MOUSE_MOTION)
	{
		UnsignedInt previousIndex = (m_nextFreeIndex == 0) ? (Mouse::NUM_MOUSE_EVENTS - 1) : (m_nextFreeIndex - 1);
		SDL3MouseEvent &previousEvent = m_eventBuffer[previousIndex];
		if (previousEvent.valid && previousEvent.event.type == SDL_EVENT_MOUSE_MOTION)
		{
			previousEvent.event = event;
			if (event.motion.windowID != 0)
				m_windowID = event.motion.windowID;
			return;
		}
	}

	while (m_eventBuffer[m_nextFreeIndex].valid)
	{
		// TheSuperHackers @bugfix denysmitin 04/12/2025 Drop stale events when the ring buffer saturates to keep recent input responsive.
		m_eventBuffer[m_nextGetIndex].valid = FALSE;
		++m_nextGetIndex;
		if (m_nextGetIndex >= Mouse::NUM_MOUSE_EVENTS)
			m_nextGetIndex = 0;
	}

	m_eventBuffer[m_nextFreeIndex].event = event;
	m_eventBuffer[m_nextFreeIndex].valid = TRUE;

	SDL_WindowID reportedWindow = 0;
	switch (event.type)
	{
	case SDL_EVENT_MOUSE_MOTION:
		reportedWindow = event.motion.windowID;
		break;
	case SDL_EVENT_MOUSE_BUTTON_DOWN:
	case SDL_EVENT_MOUSE_BUTTON_UP:
		reportedWindow = event.button.windowID;
		break;
	case SDL_EVENT_MOUSE_WHEEL:
		reportedWindow = event.wheel.windowID;
		break;
	default:
		break;
	}

	if (reportedWindow != 0)
		m_windowID = reportedWindow;

	++m_nextFreeIndex;
	if (m_nextFreeIndex >= Mouse::NUM_MOUSE_EVENTS)
		m_nextFreeIndex = 0;
}

SDL3Mouse *SDL3Mouse::getInstance()
{
	return s_instance;
}

UnsignedByte SDL3Mouse::getMouseEvent(MouseIO *result, Bool /*flush*/)
{
	if (!m_eventBuffer[m_nextGetIndex].valid)
		return MOUSE_NONE;

	translateEvent(m_nextGetIndex, result);
	m_eventBuffer[m_nextGetIndex].valid = FALSE;

	++m_nextGetIndex;
	if (m_nextGetIndex >= Mouse::NUM_MOUSE_EVENTS)
		m_nextGetIndex = 0;

	return MOUSE_OK;
}

void SDL3Mouse::translateEvent(UnsignedInt eventIndex, MouseIO *result)
{
	const SDL_Event &event = m_eventBuffer[eventIndex].event;
	UnsignedInt frame = 1;
	if (TheGameClient)
		frame = TheGameClient->getFrame();

	result->leftState = result->middleState = result->rightState = MBS_Up;
	result->leftFrame = result->middleFrame = result->rightFrame = 0;
	result->pos.x = result->pos.y = 0;
	result->wheelPos = 0;
	result->time = convertTimestamp(event);

	switch (event.type)
	{
	case SDL_EVENT_MOUSE_BUTTON_DOWN:
	{
		MouseButtonState state = (event.button.clicks >= 2) ? MBS_DoubleClick : MBS_Down;
		Int posX = roundFloatToInt(event.button.x);
		Int posY = roundFloatToInt(event.button.y);
		if (event.button.button == SDL_BUTTON_LEFT)
		{
			result->leftState = state;
			result->leftFrame = frame;
		}
		else if (event.button.button == SDL_BUTTON_RIGHT)
		{
			result->rightState = state;
			result->rightFrame = frame;
		}
		else if (event.button.button == SDL_BUTTON_MIDDLE)
		{
			result->middleState = state;
			result->middleFrame = frame;
		}
		result->pos.x = posX;
		result->pos.y = posY;
		break;
	}
	case SDL_EVENT_MOUSE_BUTTON_UP:
	{
		Int posX = roundFloatToInt(event.button.x);
		Int posY = roundFloatToInt(event.button.y);
		if (event.button.button == SDL_BUTTON_LEFT)
		{
			result->leftState = MBS_Up;
			result->leftFrame = frame;
		}
		else if (event.button.button == SDL_BUTTON_RIGHT)
		{
			result->rightState = MBS_Up;
			result->rightFrame = frame;
		}
		else if (event.button.button == SDL_BUTTON_MIDDLE)
		{
			result->middleState = MBS_Up;
			result->middleFrame = frame;
		}
		result->pos.x = posX;
		result->pos.y = posY;
		break;
	}
	case SDL_EVENT_MOUSE_MOTION:
	{
		result->pos.x = roundFloatToInt(event.motion.x);
		result->pos.y = roundFloatToInt(event.motion.y);
		break;
	}
	case SDL_EVENT_MOUSE_WHEEL:
	{
		float amount = event.wheel.y;
		if (event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED)
			amount = -amount;
		result->wheelPos = static_cast<Int>(amount * MOUSE_WHEEL_DELTA);
		result->pos.x = roundFloatToInt(event.wheel.mouse_x);
		result->pos.y = roundFloatToInt(event.wheel.mouse_y);
		break;
	}
	default:
		break;
	}
}

UnsignedInt SDL3Mouse::convertTimestamp(const SDL_Event &event)
{
	Uint64 timestamp = event.common.timestamp;
	if (timestamp == 0)
		return static_cast<UnsignedInt>(SDL_GetTicks());
	// return timestamp in milliseconds
	return static_cast<UnsignedInt>(timestamp / 1000000ULL);
}

void SDL3Mouse::initCursorResources(void)
{
	for (Int cursor = FIRST_CURSOR; cursor < Mouse::NUM_MOUSE_CURSORS; ++cursor)
	{
		Int numDirections = m_cursorInfo[cursor].numDirections;
		if (numDirections <= 0)
			numDirections = 1;
		numDirections = min(numDirections, MAX_2D_CURSOR_DIRECTIONS);
		for (Int direction = 0; direction < numDirections; ++direction)
		{
			getCursorFrames(static_cast<MouseCursor>(cursor), direction);
		}
	}
}

void SDL3Mouse::setCursor(MouseCursor cursor)
{
	Mouse::setCursor(cursor);

	if (m_lostFocus)
		return;

	if (!m_visible || cursor == NONE)
	{
		SDL_HideCursor();
		m_activeCursor = NULL;
		m_animationState = ActiveCursorState();
		m_currentCursor = cursor;
		return;
	}

	m_directionFrame = computeDirection(cursor);
	applyCursor(cursor, m_directionFrame, FALSE);
	SDL_ShowCursor();
	m_currentCursor = cursor;
}

const std::vector<SDL3AniReader::Frame> *SDL3Mouse::getCursorFrames(MouseCursor cursor, Int direction)
{
	if (cursor < FIRST_CURSOR || cursor >= Mouse::NUM_MOUSE_CURSORS)
		return NULL;
	if (direction < 0 || direction >= MAX_2D_CURSOR_DIRECTIONS)
		return NULL;

	CursorFrames &entry = m_cursorResources[cursor][direction];
	if (!entry.loaded)
	{
		entry.loaded = TRUE;
		entry.frames.clear();

		if (m_cursorInfo[cursor].textureName.isEmpty())
			return NULL;

		char resourcePath[256];
		if (m_cursorInfo[cursor].numDirections > 1)
		{
			sprintf(resourcePath, "Data\\cursors\\%s%d.ANI", m_cursorInfo[cursor].textureName.str(), direction);
		}
		else
		{
			sprintf(resourcePath, "Data\\cursors\\%s.ANI", m_cursorInfo[cursor].textureName.str());
		}

		Bool loaded = FALSE;

		if (TheGlobalData && TheGlobalData->m_modDir.isNotEmpty())
		{
			AsciiString modPath;
			if (m_cursorInfo[cursor].numDirections > 1)
				modPath.format("%sData\\cursors\\%s%d.ANI", TheGlobalData->m_modDir.str(), m_cursorInfo[cursor].textureName.str(), direction);
			else
				modPath.format("%sData\\cursors\\%s.ANI", TheGlobalData->m_modDir.str(), m_cursorInfo[cursor].textureName.str());

			if (TheLocalFileSystem && TheLocalFileSystem->doesFileExist(modPath.str()))
			{
				std::vector<SDL3AniReader::Frame> frames;
				if (loadCursorFromFile(modPath.str(), frames))
				{
					entry.frames.swap(frames);
					loaded = TRUE;
				}
			}
		}

		if (!loaded)
		{
			std::vector<SDL3AniReader::Frame> frames;
			if (loadCursorFromFile(resourcePath, frames))
			{
				entry.frames.swap(frames);
				loaded = TRUE;
			}
		}

		DEBUG_ASSERTCRASH(loaded && !entry.frames.empty(), ("MissingCursor %s", resourcePath));
	}

	if (entry.frames.empty())
		return NULL;

	return &entry.frames;
}

Bool SDL3Mouse::loadCursorFromFile(const char *path, std::vector<SDL3AniReader::Frame> &outFrames)
{
	SDL3AniReader aniReader;
	std::vector<SDL3AniReader::Frame> frames;
	if (!aniReader.load(path, frames))
		return FALSE;
	outFrames.swap(frames);
	return TRUE;
}

void SDL3Mouse::applyCursor(MouseCursor cursor, Int direction, Bool resetFrame)
{
	Int clampedDirection = direction;
	if (cursor < FIRST_CURSOR || cursor >= Mouse::NUM_MOUSE_CURSORS)
	{
		clampedDirection = 0;
	}
	else
	{
		Int numDirections = m_cursorInfo[cursor].numDirections;
		if (numDirections <= 0)
			numDirections = 1;
		clampedDirection %= numDirections;
		if (clampedDirection < 0)
			clampedDirection += numDirections;
	}

	if (m_animationState.cursor == cursor && m_animationState.direction == clampedDirection)
		return;

	m_directionFrame = clampedDirection;
	const std::vector<SDL3AniReader::Frame> *frames = getCursorFrames(cursor, clampedDirection);
	Uint64 now = SDL_GetTicks();

	if (frames != NULL && !frames->empty())
	{
		Int targetFrame = resetFrame ? 0 : m_animationState.frameIndex;
		if (targetFrame < 0 || targetFrame >= static_cast<Int>(frames->size()))
			targetFrame = 0;
		m_animationState.cursor = cursor;
		m_animationState.direction = clampedDirection;
		Bool forceApply = resetFrame;
		applyCursorFrame(*frames, targetFrame, now, forceApply);
		return;
	}

	SDL_Cursor *defaultCursor = SDL_GetDefaultCursor();
	SDL_SetCursor(defaultCursor);
	m_activeCursor = defaultCursor;
	m_animationState = ActiveCursorState();
}

void SDL3Mouse::applyCursorFrame(const std::vector<SDL3AniReader::Frame> &frames, Int frameIndex, Uint64 now, Bool forceApply)
{
	if (frameIndex < 0 || frameIndex >= static_cast<Int>(frames.size()))
		return;

	SDL_Cursor *cursorHandle = frames[frameIndex].cursor;
	if (cursorHandle == NULL)
		cursorHandle = SDL_GetDefaultCursor();

	if (cursorHandle != NULL)
	{
		Bool needsApply = forceApply || cursorHandle != m_activeCursor;
		if (needsApply)
		{
			if (!SDL_SetCursor(cursorHandle))
			{
				WWDEBUG_ERROR(("SDL_SetCursor failed: %s", SDL_GetError()));
			}
			else
			{
				m_activeCursor = cursorHandle;
			}
		}
	}

	UnsignedInt duration = frames[frameIndex].durationMs;
	if (duration == 0)
		duration = 100;

	m_animationState.frameIndex = frameIndex;
	m_animationState.nextFrameChangeMs = (frames.size() > 1) ? (now + static_cast<Uint64>(duration)) : 0;
}

// TheSuperHackers @feature denysmitin 04/12/2025 SDL3 animated cursor playback using ANI frames.
void SDL3Mouse::updateCursorAnimation(void)
{
	if (m_animationState.cursor < FIRST_CURSOR || m_animationState.cursor >= Mouse::NUM_MOUSE_CURSORS)
		return;
	if (!m_visible || m_lostFocus)
		return;

	const std::vector<SDL3AniReader::Frame> *frames = getCursorFrames(m_animationState.cursor, m_animationState.direction);
	if (frames == NULL || frames->size() <= 1)
		return;

	Uint64 now = SDL_GetTicks();
	if (m_animationState.nextFrameChangeMs == 0 || now < m_animationState.nextFrameChangeMs)
		return;

	Int nextFrame = m_animationState.frameIndex + 1;
	if (nextFrame >= static_cast<Int>(frames->size()))
		nextFrame = 0;

	applyCursorFrame(*frames, nextFrame, now, FALSE);
}

Int SDL3Mouse::computeDirection(MouseCursor cursor) const
{
	if (cursor < FIRST_CURSOR || cursor >= Mouse::NUM_MOUSE_CURSORS)
		return 0;

	const CursorInfo &info = m_cursorInfo[cursor];
	Int numDirections = info.numDirections;
	if (numDirections <= 1)
		return 0;

	if (TheInGameUI && TheInGameUI->isScrolling())
	{
		Coord2D offset = TheInGameUI->getScrollAmount();
		if (offset.x != 0 || offset.y != 0)
		{
			offset.normalize();
			const Real twoPi = static_cast<Real>(6.28318530717958647692);
			Real theta = static_cast<Real>(atan2(offset.y, offset.x));
			theta = fmod(theta + twoPi, twoPi);
			Real segment = twoPi / static_cast<Real>(numDirections);
			Int frame = static_cast<Int>((theta / segment) + 0.5f);
			if (frame >= numDirections)
				frame = 0;
			return frame;
		}
	}

	return 0;
}

void SDL3Mouse::setVisibility(Bool visible)
{
	Mouse::setVisibility(visible);
	if (visible)
	{
		SDL_ShowCursor();
	}
	else
	{
		SDL_HideCursor();
	}
	setCursor(getMouseCursor());
}

void SDL3Mouse::capture(void)
{
	if (SDL_CaptureMouse(true))
	{
		onCursorCaptured(TRUE);
		clampCursorToWindow();
	}
	else
	{
		WWDEBUG_ERROR(("SDL_CaptureMouse(true) failed: %s", SDL_GetError()));
	}
}

void SDL3Mouse::releaseCapture(void)
{
	if (!SDL_CaptureMouse(false))
	{
		WWDEBUG_ERROR(("SDL_CaptureMouse(false) failed: %s", SDL_GetError()));
	}
	onCursorCaptured(FALSE);
}

SDL_Window *SDL3Mouse::resolveWindow(void) const
{
	if (m_windowID != 0)
	{
		SDL_Window *window = SDL_GetWindowFromID(m_windowID);
		if (window != NULL)
			return window;
	}
	return SDL_GetMouseFocus();
}

void SDL3Mouse::clampCursorToWindow(void)
{
	if (!m_isCursorCaptured)
		return;

	SDL_Window *window = resolveWindow();
	if (window == NULL)
		return;

	float fx = 0.0f;
	float fy = 0.0f;
	SDL_GetMouseState(&fx, &fy);

	Int x = roundFloatToInt(fx);
	Int y = roundFloatToInt(fy);
	Int clampedX = clampValue(x, m_minX, m_maxX);
	Int clampedY = clampValue(y, m_minY, m_maxY);

	if (clampedX != x || clampedY != y)
	{
		SDL_WarpMouseInWindow(window, static_cast<float>(clampedX), static_cast<float>(clampedY));
	}
}

void SDL3Mouse::loseFocus(void)
{
	Mouse::loseFocus();
	m_lostFocus = TRUE;
}

void SDL3Mouse::regainFocus(void)
{
	Mouse::regainFocus();
	m_lostFocus = FALSE;
	setCursor(getMouseCursor());
}
