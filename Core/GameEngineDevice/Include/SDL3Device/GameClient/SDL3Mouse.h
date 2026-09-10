#pragma once

#include "GameClient/Mouse.h"
#include <SDL3/SDL.h>

class SDL3Mouse : public Mouse
{
public:
	SDL3Mouse() = default;
	~SDL3Mouse() override;
	void init() override;
	void reset() override;
	void update() override;
	void addWheelDelta(Real wheelDelta) override;
	void initCursorResources() override;
	void setCursor(MouseCursor cursor) override;
	void setCursorWithDirection(MouseCursor cursor, Int directionFrame);
	void setVisibility(Bool visible) override;
	void loseFocus() override;
	void regainFocus() override;
protected:
	virtual void applyCursor();
	Bool m_cursorFocused = TRUE;
	void capture() override;
	void releaseCapture() override;
	UnsignedByte getMouseEvent(MouseIO *result, Bool flush) override;

private:
	static SDL_SystemCursor systemCursorFor(MouseCursor cursor);
	Bool loadCursorResource(MouseCursor cursor, Int direction);

	Uint32 m_previousButtons = 0;
	Int m_previousX = 0;
	Int m_previousY = 0;
	Bool m_reportedThisFrame = FALSE;
	SDL_Cursor *m_cursor = nullptr;
	SDL_SystemCursor m_cursorType = SDL_SYSTEM_CURSOR_DEFAULT;
	Real m_pendingWheel = 0.0f;
	Int m_cursorDirection = 0;
	SDL_Cursor *m_cursorResources[NUM_MOUSE_CURSORS][MAX_2D_CURSOR_DIRECTIONS]{};
	Bool m_cursorResourceAttempted[NUM_MOUSE_CURSORS][MAX_2D_CURSOR_DIRECTIONS]{};
};
