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

// SYSTEM INCLUDES
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <array>
#include <functional>

// USER INCLUDES
#include "GameClient/Mouse.h"
#include "GameClient/Keyboard.h"
#include "GameClient/KeyDefs.h"

// FORWARD REFERENCES
struct AnimatedCursor;
class SDL3InputManager;

// GLOBALS ---------------------------------------------------------------------
extern SDL3InputManager* TheSDL3InputManager;

// TYPE DEFINES ----------------------------------------------------------------
typedef KeyDefType KeyVal;

// SDL3Mouse ------------------------------------------------------------------
/** Mouse interface using SDL3 APIs */
//-----------------------------------------------------------------------------
class SDL3Mouse : public Mouse
{
public:
	SDL3Mouse(SDL_Window* window);
	virtual ~SDL3Mouse(void);

	// SubsystemInterface
	virtual void init(void) override;
	virtual void reset(void) override;
	virtual void update(void) override;
	virtual void initCursorResources(void) override;
	static void freeCursorResources(void);

	// Mouse interface
	virtual void setCursor(MouseCursor cursor) override;
	virtual void setVisibility(Bool visible) override;
	virtual void loseFocus() override;
	virtual void regainFocus() override;

	// SDL3-specific methods
	void addSDLEvent(SDL_Event *event);
	
protected:
	virtual void capture(void) override;
	virtual void releaseCapture(void) override;
	virtual UnsignedByte getMouseEvent(MouseIO *result, Bool flush) override;

private:
	// Event translation from SDL_Event (Clean Slate implementation)
	void translateEvent(const SDL_Event& event, MouseIO *result);

	// Scale raw SDL window coordinates to game internal resolution
	void scaleMouseCoordinates(int rawX, int rawY, Uint32 windowID, int& scaledX, int& scaledY);
	
	// Load cursor from ANI file (fighter19 pattern)
	AnimatedCursor* loadCursorFromFile(const char* filepath);

	SDL_Window* m_Window;
	Bool m_IsCaptured;
	Bool m_IsVisible;
	Bool m_LostFocus;
	
	Uint32 m_LeftButtonDownTime;
	Uint32 m_RightButtonDownTime;
	Uint32 m_MiddleButtonDownTime;
	UnsignedInt m_LastFrameNumber;
	
	ICoord2D m_LeftButtonDownPos;
	ICoord2D m_RightButtonDownPos;
	ICoord2D m_MiddleButtonDownPos;
	
	Int m_directionFrame;
	UnsignedInt m_inputFrame;

	float m_accumulatedDeltaX;
	float m_accumulatedDeltaY;
	
	SDL_Cursor* m_activeSDLCursor;
	Bool m_cursorDirty;
};

// SDL3Keyboard ---------------------------------------------------------------
/** Keyboard interface using SDL3 APIs */
//-----------------------------------------------------------------------------
class SDL3Keyboard : public Keyboard
{
public:
	SDL3Keyboard(void);
	virtual ~SDL3Keyboard(void);

	// SubsystemInterface
	virtual void init(void) override;
	virtual void reset(void) override;
	virtual void update(void) override;

	// Keyboard interface
	virtual Bool getCapsState(void) override;
	
	// SDL3-specific methods
	void addSDLEvent(SDL_Event *event);

protected:
	virtual void getKey(KeyboardIO *key) override;
	virtual KeyVal translateScanCodeToKeyVal(unsigned char scan);

private:
	void translateKeyEvent(const SDL_KeyboardEvent& event);
};

// SDL3InputManager -----------------------------------------------------------
/** Unified manager for SDL3 input events */
//-----------------------------------------------------------------------------
class SDL3InputManager
{
public:
	SDL3InputManager(SDL_Window* window);
	virtual ~SDL3InputManager();

	void update();
	
	// Buffer access
	Bool getNextMouseEvent(SDL_Event& outEvent);
	Bool getNextKeyboardEvent(SDL_Event& outEvent);
	
	void addMouseSDLEvent(const SDL_Event& event);
	void addKeyboardSDLEvent(const SDL_Event& event);

	Bool isQuitting() const { return m_isQuitting; }

	// Constants
	static constexpr float AXIS_MAX = 32767.0f;
	static constexpr int TRIGGER_THRESHOLD = 16384;
	static constexpr float DEFAULT_DEADZONE = 0.15f;
	static constexpr float DEFAULT_CURSOR_SPEED = 800.0f;

private:
	struct GamepadState {
		bool buttonState[SDL_GAMEPAD_BUTTON_COUNT];
		bool stickLeft, stickRight, stickUp, stickDown;
		bool ltDown, rtDown;

		GamepadState() {
			memset(buttonState, 0, sizeof(buttonState));
			stickLeft = stickRight = stickUp = stickDown = false;
			ltDown = rtDown = false;
		}
	};

private:
	// Gamepad management
	void openFirstGamepad();
	void closeGamepad();

	SDL_Window* m_window;
	SDL_Gamepad* m_gamepad;
	void processGamepadInput();
	void handleGamepadButton(SDL_GamepadButton button, bool& currentState, bool isDown, std::function<void(bool)> action);
	
	// Virtual event injection
	void virtualPulseKey(SDL_Scancode scancode, bool down);
	void virtualPulseMouse(Uint8 button, bool down);

	// Event buffers
	static const UnsignedInt MAX_MOUSE_EVENTS = 256;
	static const UnsignedInt MAX_KEY_EVENTS = 256;
	
	SDL_Event m_mouseEvents[MAX_MOUSE_EVENTS];
	UnsignedInt m_mouseNextFree;
	UnsignedInt m_mouseNextGet;

	SDL_Event m_keyEvents[MAX_KEY_EVENTS];
	UnsignedInt m_keyNextFree;
	UnsignedInt m_keyNextGet;

	// Gamepad state
	GamepadState m_state;

	Bool m_precisionMode;
	Uint64 m_lastUpdateTime;
	Bool m_isQuitting;
};
