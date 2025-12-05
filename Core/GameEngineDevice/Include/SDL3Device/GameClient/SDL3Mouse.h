#pragma once

#include "GameClient/Mouse.h"
#include "SDL3Device/GameClient/SDL3AniReader.h"

#include <SDL3/SDL.h>

#include <vector>

// TheSuperHackers @feature denysmitin 03/12/2025 SDL3 mouse device implementation.
class SDL3Mouse : public Mouse
{

public:
	SDL3Mouse(void);
	~SDL3Mouse(void) override;

	void init(void) override;
	void reset(void) override;
	void update(void) override;
	void initCursorResources(void) override;
	void setCursor(MouseCursor cursor) override;
	void setVisibility(Bool visible) override;
	void loseFocus(void) override;
	void regainFocus(void) override;

	void handleEvent(const SDL_Event &event);
	void addSDL3Event(const SDL_Event &event);

	static SDL3Mouse *getInstance();

protected:
	void capture(void) override;
	void releaseCapture(void) override;
	UnsignedByte getMouseEvent(MouseIO *result, Bool flush) override;

	struct CursorFrames
	{
		CursorFrames() : loaded(FALSE) {}

		Bool loaded;
		std::vector<SDL3AniReader::Frame> frames;
	};

	struct ActiveCursorState
	{
		ActiveCursorState()
				: cursor(NONE), direction(0), frameIndex(0), nextFrameChangeMs(0) {}

		MouseCursor cursor;
		Int direction;
		Int frameIndex;
		Uint64 nextFrameChangeMs;
	};

	struct SDL3MouseEvent
	{
		Bool valid;
		SDL_Event event;
	};

	void translateEvent(UnsignedInt eventIndex, MouseIO *result);
	const std::vector<SDL3AniReader::Frame> *getCursorFrames(MouseCursor cursor, Int direction);
	Bool loadCursorFromFile(const char *path, std::vector<SDL3AniReader::Frame> &outFrames);
	void applyCursor(MouseCursor cursor, Int direction, Bool resetFrame);
	void applyCursorFrame(const std::vector<SDL3AniReader::Frame> &frames, Int frameIndex, Uint64 now, Bool forceApply);
	void updateCursorAnimation(void);
	Int computeDirection(MouseCursor cursor) const;
	SDL_Window *resolveWindow(void) const;
	void clampCursorToWindow(void);
	static UnsignedInt convertTimestamp(const SDL_Event &event);

	SDL3MouseEvent m_eventBuffer[Mouse::NUM_MOUSE_EVENTS];
	UnsignedInt m_nextFreeIndex;
	UnsignedInt m_nextGetIndex;
	Int m_directionFrame;
	Bool m_lostFocus;
	SDL_WindowID m_windowID;
	CursorFrames m_cursorResources[Mouse::NUM_MOUSE_CURSORS][MAX_2D_CURSOR_DIRECTIONS];
	SDL_Cursor *m_activeCursor;
	ActiveCursorState m_animationState;

	static SDL3Mouse *s_instance;
};
