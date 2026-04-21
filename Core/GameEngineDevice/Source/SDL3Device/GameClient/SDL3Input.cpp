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

#include "Lib/BaseType.h"

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <array>
#include <functional>
#include <memory>
#include <SDL3/SDL.h>
#include "SDL3Device/GameClient/SDL3Cursor.h"
#include <windows.h> // For timeGetTime()

#include "SDL3Device/GameClient/SDL3Input.h"
#include "Common/Debug.h"
#include "Common/file.h"
#include "Common/FileSystem.h"
#include "Common/GameEngine.h"
#include "Common/MessageStream.h"
#include "GameClient/Display.h"
#include "GameClient/InGameUI.h"
#include "GameLogic/GameLogic.h"
#include "SDL3GameEngine.h"

// GLOBALS ---------------------------------------------------------------------
SDL3InputManager* TheSDL3InputManager = nullptr;

/// ============================================================================
// SDL3MOUSE IMPLEMENTATION
// ============================================================================

/**
 * Constructor - Initialize SDL3Mouse with window handle
 */
SDL3Mouse::SDL3Mouse(SDL_Window* window)
	: Mouse(),
	  m_Window(window),
	  m_IsCaptured(false),
	  m_IsVisible(true),
	  m_LostFocus(false),
	  m_LeftButtonDownTime(0),
	  m_RightButtonDownTime(0),
	  m_MiddleButtonDownTime(0),
	  m_directionFrame(0),
	  m_accumulatedDeltaX(0.0f),
	  m_accumulatedDeltaY(0.0f),
	  m_activeSDLCursor(nullptr)
{
	m_LeftButtonDownPos.x = 0;
	m_LeftButtonDownPos.y = 0;
	m_RightButtonDownPos.x = 0;
	m_RightButtonDownPos.y = 0;
	m_MiddleButtonDownPos.x = 0;
	m_MiddleButtonDownPos.y = 0;
}

/**
 * Destructor
 */
SDL3Mouse::~SDL3Mouse(void)
{
	releaseCapture();
}

/**
 * Initialize mouse subsystem
 */
void SDL3Mouse::init(void)
{
	Mouse::init();

	m_inputMovesAbsolute = TRUE;

	// Show cursor by default
	setVisibility(TRUE);
}

/**
 * Reset mouse to default state
 */
void SDL3Mouse::reset(void)
{
	Mouse::reset();

	releaseCapture();
	setVisibility(TRUE);
}

/**
 * Update mouse state (called per-frame)
 */
void SDL3Mouse::update(void)
{
	Mouse::update();

	if (m_LostFocus)
	{
		return;
	}

	MouseCursor cursor = m_currentCursor;
	
	if (cursor != NONE && cursor != INVALID_MOUSE_CURSOR && m_cursorInfo[cursor].numDirections > 1)
	{
		float dx = 0.0f;
		float dy = 0.0f;
		bool hasMovement = false;

		if (cursor == SCROLL && TheInGameUI && TheInGameUI->isScrolling())
		{
			Coord2D scroll = TheInGameUI->getScrollAmount();
			if (scroll.x != 0.0f || scroll.y != 0.0f)
			{
				dx = scroll.x;
				dy = scroll.y;
				hasMovement = true;
			}
		}

		if (!hasMovement)
		{
			if (SDL_fabsf(m_accumulatedDeltaX) > 0.01f || SDL_fabsf(m_accumulatedDeltaY) > 0.01f)
			{
				dx = m_accumulatedDeltaX;
				dy = m_accumulatedDeltaY;
				hasMovement = true;
				
				m_accumulatedDeltaX = 0.0f;
				m_accumulatedDeltaY = 0.0f;
			}
		}

		if (hasMovement)
		{
			float angle = atan2f(dy, dx);
			if (angle < 0) angle += 2.0f * (float)M_PI;
			float segmentAngle = 2.0f * (float)M_PI / (float)m_cursorInfo[cursor].numDirections;
			m_directionFrame = (int)((angle + (segmentAngle / 2.0f)) / segmentAngle) % m_cursorInfo[cursor].numDirections;
		}
	}
	else
	{
		m_directionFrame = 0;
	}

	SDL_Cursor* requestedHandle = nullptr;
	bool bUseDefaultCursor = false;

	if (cursor == NONE || cursor == INVALID_MOUSE_CURSOR || !m_IsVisible)
	{
		bUseDefaultCursor = true;
	}
	else
	{
		requestedHandle = SDL3CursorManager::getCursor(cursor, m_directionFrame);
		if (!requestedHandle)
		{
			bUseDefaultCursor = true;
		}
	}

	if (bUseDefaultCursor)
	{
		requestedHandle = SDL3CursorManager::getCursor(NORMAL, 0);
		if (!requestedHandle)
		{
			requestedHandle = SDL_GetDefaultCursor();
		}
	}

	if (requestedHandle != m_activeSDLCursor)
	{
		SDL_SetCursor(requestedHandle);
		m_activeSDLCursor = requestedHandle;
	}
}

/**
 * Initialize cursor resources (load cursor images from ANI files)
 */
void SDL3Mouse::initCursorResources(void)
{
	SDL3CursorManager::initResources(this);
}

void SDL3Mouse::freeCursorResources(void)
{
	SDL3CursorManager::shutdown();
}

/**
 * Set mouse cursor type
 */
void SDL3Mouse::setCursor(MouseCursor cursor)
{
	if (m_currentCursor == cursor)
	{
		return;
	}

	Mouse::setCursor( cursor );
	m_currentCursor = cursor;
}

/**
 * Set cursor visibility
 */
void SDL3Mouse::setVisibility(Bool visible)
{
	Mouse::setVisibility(visible);

	if (visible) {
		SDL_ShowCursor();
	} else {
		SDL_HideCursor();
	}
}

/**
 * Handle window losing focus
 */
void SDL3Mouse::loseFocus()
{
	Mouse::loseFocus();
	releaseCapture();
}

/**
 * Handle window regaining focus
 */
void SDL3Mouse::regainFocus()
{
	Mouse::regainFocus();
}

/**
 * Capture mouse (confine to window)
 */
void SDL3Mouse::capture(void)
{
	if (!m_Window || m_isCursorCaptured) {
		return;
	}

	SDL_CaptureMouse(true);
	SDL_SetWindowMouseGrab(m_Window, true);
	onCursorCaptured(true);
}

/**
 * Release mouse capture
 */
void SDL3Mouse::releaseCapture(void)
{
	if (!m_isCursorCaptured) {
		return;
	}

	SDL_CaptureMouse(false);
	if (m_Window) {
		SDL_SetWindowMouseGrab(m_Window, false);
	}

	onCursorCaptured(false);
}

/**
 * Get next mouse event from the centralized input manager
 */
UnsignedByte SDL3Mouse::getMouseEvent(MouseIO *result, Bool flush)
{
	if (!TheSDL3InputManager) {
		return MOUSE_NONE;
	}

	SDL_Event nextEvent;
	if (!TheSDL3InputManager->getNextMouseEvent(nextEvent)) {
		return MOUSE_NONE;
	}

	translateEvent(nextEvent, result);

	return MOUSE_OK;
}

void SDL3Mouse::addSDLEvent(SDL_Event *event)
{
	if (TheSDL3InputManager && event) {
		TheSDL3InputManager->addMouseSDLEvent(*event);
	}
}

//-----------------------------------------------------------------------------
/** Unified event translation (Clean Slate Rewrite) */
//-----------------------------------------------------------------------------
void SDL3Mouse::translateEvent(const SDL_Event& event, MouseIO *result)
{
	if (!result) return;

	// Reset state
	result->leftState = result->rightState = result->middleState = MBS_None;
	result->wheelPos = 0;
	result->deltaPos.x = result->deltaPos.y = 0;

	// Common timestamp (SDL3 uses nanoseconds, SAGE usually wants ms)
	result->time = (Uint32)(event.common.timestamp / 1000000);

	int rawX = 0;
	int rawY = 0;
	Uint32 windowID = 0;

	switch (event.type) {
		case SDL_EVENT_MOUSE_MOTION:
			rawX = (int)event.motion.x;
			rawY = (int)event.motion.y;
			windowID = event.motion.windowID;
			result->deltaPos.x = (Int)event.motion.xrel;
			result->deltaPos.y = (Int)event.motion.yrel;
			
			m_accumulatedDeltaX += event.motion.xrel;
			m_accumulatedDeltaY += event.motion.yrel;
			break;

		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		case SDL_EVENT_MOUSE_BUTTON_UP:
		{
			rawX = (int)event.button.x;
			rawY = (int)event.button.y;
			windowID = event.button.windowID;
			
			MouseButtonState state = event.button.down ? MBS_Down : MBS_Up;
			if (event.button.down && event.button.clicks >= 2) state = MBS_DoubleClick;

			if (event.button.button == SDL_BUTTON_LEFT) result->leftState = state;
			else if (event.button.button == SDL_BUTTON_RIGHT) result->rightState = state;
			else if (event.button.button == SDL_BUTTON_MIDDLE) result->middleState = state;
			break;
		}

		case SDL_EVENT_MOUSE_WHEEL:
		{
			// For wheel events, use current mouse position
			float mx, my;
			SDL_GetMouseState(&mx, &my);
			rawX = (int)mx;
			rawY = (int)my;
			windowID = event.wheel.windowID;
			result->wheelPos = (Int)(event.wheel.y * 120); // MOUSE_WHEEL_DELTA
			break;
		}

		default:
			return;
	}

	// Dynamic Scaling Guard
	int scaledX, scaledY;
	scaleMouseCoordinates(rawX, rawY, windowID, scaledX, scaledY);
	result->pos.x = scaledX;
	result->pos.y = scaledY;
}

void SDL3Mouse::scaleMouseCoordinates(int rawX, int rawY, Uint32 windowID, int& scaledX, int& scaledY)
{
	SDL_Window* window = SDL_GetWindowFromID(windowID);
	if (!window || !TheDisplay) {
		scaledX = rawX;
		scaledY = rawY;
		return;
	}

	int winW = 0, winH = 0;
	SDL_GetWindowSizeInPixels(window, &winW, &winH);

	int intW = TheDisplay->getWidth();
	int intH = TheDisplay->getHeight();

	// Guard: If we are at native resolution, bypass all math
	if (winW == intW && winH == intH) {
		scaledX = rawX;
		scaledY = rawY;
		return;
	}

	// Handle Viewport/Letterboxing if active
	int pbX, pbY, pbW, pbH;
	if (TheDisplay->getViewportRect(pbX, pbY, pbW, pbH)) {
		int cx = std::max(0, std::min(pbW, rawX - pbX));
		int cy = std::max(0, std::min(pbH, rawY - pbY));
		scaledX = (int)(cx * (float)intW / pbW);
		scaledY = (int)(cy * (float)intH / pbH);
	} else {
		scaledX = (int)(rawX * (float)intW / winW);
		scaledY = (int)(rawY * (float)intH / winH);
	}
}

// ============================================================================
// SDL3KEYBOARD IMPLEMENTATION
// ============================================================================

/**
 * Lifecycle
 */
SDL3Keyboard::SDL3Keyboard(void) : Keyboard() {}
SDL3Keyboard::~SDL3Keyboard(void) {}

/**
 * SubsystemInterface
 */
void SDL3Keyboard::init(void) { Keyboard::init(); }
void SDL3Keyboard::reset(void) { Keyboard::reset(); }
void SDL3Keyboard::update(void) { Keyboard::update(); }

/**
 * Keyboard Interface
 */
Bool SDL3Keyboard::getCapsState(void) { return FALSE; }

/**
 * SDL3-specific internal methods
 */
void SDL3Keyboard::getKey(KeyboardIO *key)
{
	if (!TheSDL3InputManager) {
		key->key = KEY_NONE;
		key->status = KeyboardIO::STATUS_UNUSED;
		return;
	}

	SDL_Event nextEvent;
	if (!TheSDL3InputManager->getNextKeyboardEvent(nextEvent)) {
		key->key = KEY_NONE;
		key->status = KeyboardIO::STATUS_UNUSED;
		return;
	}
	
	const SDL_KeyboardEvent& keyEvent = nextEvent.key;
	KeyDefType keyDef = translateScanCodeToKeyVal(keyEvent.scancode);
	
	key->key = keyDef;
	key->status = KeyboardIO::STATUS_UNUSED;
	key->state = keyEvent.down ? KEY_STATE_DOWN : KEY_STATE_UP;
	key->keyDownTimeMsec = keyEvent.down ? timeGetTime() : 0;

	SDL_Keymod mod = keyEvent.mod;
	if (mod & SDL_KMOD_LSHIFT) key->state |= KEY_STATE_LSHIFT;
	if (mod & SDL_KMOD_RSHIFT) key->state |= KEY_STATE_RSHIFT;
	if (mod & SDL_KMOD_LCTRL)  key->state |= KEY_STATE_LCONTROL;
	if (mod & SDL_KMOD_RCTRL)  key->state |= KEY_STATE_RCONTROL;
	if (mod & SDL_KMOD_LALT)   key->state |= KEY_STATE_LALT;
	if (mod & SDL_KMOD_RALT)   key->state |= KEY_STATE_RALT;
	if (mod & SDL_KMOD_CAPS)   key->state |= KEY_STATE_CAPSLOCK;

	if (keyDef == KEY_LSHIFT)  key->state &= ~KEY_STATE_LSHIFT;
	if (keyDef == KEY_RSHIFT)  key->state &= ~KEY_STATE_RSHIFT;
	if (keyDef == KEY_LCTRL)   key->state &= ~KEY_STATE_LCONTROL;
	if (keyDef == KEY_RCTRL)   key->state &= ~KEY_STATE_RCONTROL;
	if (keyDef == KEY_LALT)    key->state &= ~KEY_STATE_LALT;
	if (keyDef == KEY_RALT)    key->state &= ~KEY_STATE_RALT;
}

void SDL3Keyboard::addSDLEvent(SDL_Event *event)
{
	if (TheSDL3InputManager && event) {
		TheSDL3InputManager->addKeyboardSDLEvent(*event);
	}
}

KeyVal SDL3Keyboard::translateScanCodeToKeyVal(unsigned char scan)
{
	switch ((SDL_Scancode)scan) {
		case SDL_SCANCODE_ESCAPE: return KEY_ESC;
		case SDL_SCANCODE_RETURN: return KEY_ENTER;
		case SDL_SCANCODE_KP_ENTER: return KEY_KPENTER;
		case SDL_SCANCODE_SPACE: return KEY_SPACE;
		case SDL_SCANCODE_TAB: return KEY_TAB;
		case SDL_SCANCODE_BACKSPACE: return KEY_BACKSPACE;
		case SDL_SCANCODE_DELETE: return KEY_DEL;
		case SDL_SCANCODE_HOME: return KEY_HOME;
		case SDL_SCANCODE_END: return KEY_END;
		case SDL_SCANCODE_PAGEUP: return KEY_PGUP;
		case SDL_SCANCODE_PAGEDOWN: return KEY_PGDN;
		case SDL_SCANCODE_INSERT: return KEY_INS;
		case SDL_SCANCODE_LSHIFT: return KEY_LSHIFT;
		case SDL_SCANCODE_RSHIFT: return KEY_RSHIFT;
		case SDL_SCANCODE_LCTRL: return KEY_LCTRL;
		case SDL_SCANCODE_RCTRL: return KEY_RCTRL;
		case SDL_SCANCODE_LALT: return KEY_LALT;
		case SDL_SCANCODE_RALT: return KEY_RALT;
		case SDL_SCANCODE_UP: return KEY_UP;
		case SDL_SCANCODE_DOWN: return KEY_DOWN;
		case SDL_SCANCODE_LEFT: return KEY_LEFT;
		case SDL_SCANCODE_RIGHT: return KEY_RIGHT;
		case SDL_SCANCODE_F1: return KEY_F1;
		case SDL_SCANCODE_F2: return KEY_F2;
		case SDL_SCANCODE_F3: return KEY_F3;
		case SDL_SCANCODE_F4: return KEY_F4;
		case SDL_SCANCODE_F5: return KEY_F5;
		case SDL_SCANCODE_F6: return KEY_F6;
		case SDL_SCANCODE_F7: return KEY_F7;
		case SDL_SCANCODE_F8: return KEY_F8;
		case SDL_SCANCODE_F9: return KEY_F9;
		case SDL_SCANCODE_F10: return KEY_F10;
		case SDL_SCANCODE_F11: return KEY_F11;
		case SDL_SCANCODE_F12: return KEY_F12;
		case SDL_SCANCODE_1: return KEY_1;
		case SDL_SCANCODE_2: return KEY_2;
		case SDL_SCANCODE_3: return KEY_3;
		case SDL_SCANCODE_4: return KEY_4;
		case SDL_SCANCODE_5: return KEY_5;
		case SDL_SCANCODE_6: return KEY_6;
		case SDL_SCANCODE_7: return KEY_7;
		case SDL_SCANCODE_8: return KEY_8;
		case SDL_SCANCODE_9: return KEY_9;
		case SDL_SCANCODE_0: return KEY_0;
		case SDL_SCANCODE_A: return KEY_A;
		case SDL_SCANCODE_B: return KEY_B;
		case SDL_SCANCODE_C: return KEY_C;
		case SDL_SCANCODE_D: return KEY_D;
		case SDL_SCANCODE_E: return KEY_E;
		case SDL_SCANCODE_F: return KEY_F;
		case SDL_SCANCODE_G: return KEY_G;
		case SDL_SCANCODE_H: return KEY_H;
		case SDL_SCANCODE_I: return KEY_I;
		case SDL_SCANCODE_J: return KEY_J;
		case SDL_SCANCODE_K: return KEY_K;
		case SDL_SCANCODE_L: return KEY_L;
		case SDL_SCANCODE_M: return KEY_M;
		case SDL_SCANCODE_N: return KEY_N;
		case SDL_SCANCODE_O: return KEY_O;
		case SDL_SCANCODE_P: return KEY_P;
		case SDL_SCANCODE_Q: return KEY_Q;
		case SDL_SCANCODE_R: return KEY_R;
		case SDL_SCANCODE_S: return KEY_S;
		case SDL_SCANCODE_T: return KEY_T;
		case SDL_SCANCODE_U: return KEY_U;
		case SDL_SCANCODE_V: return KEY_V;
		case SDL_SCANCODE_W: return KEY_W;
		case SDL_SCANCODE_X: return KEY_X;
		case SDL_SCANCODE_Y: return KEY_Y;
		case SDL_SCANCODE_Z: return KEY_Z;
		case SDL_SCANCODE_MINUS: return KEY_MINUS;
		case SDL_SCANCODE_EQUALS: return KEY_EQUAL;
		case SDL_SCANCODE_LEFTBRACKET: return KEY_LBRACKET;
		case SDL_SCANCODE_RIGHTBRACKET: return KEY_RBRACKET;
		case SDL_SCANCODE_SEMICOLON: return KEY_SEMICOLON;
		case SDL_SCANCODE_APOSTROPHE: return KEY_APOSTROPHE;
		case SDL_SCANCODE_GRAVE: return KEY_TICK;
		case SDL_SCANCODE_COMMA: return KEY_COMMA;
		case SDL_SCANCODE_PERIOD: return KEY_PERIOD;
		case SDL_SCANCODE_SLASH: return KEY_SLASH;
		case SDL_SCANCODE_BACKSLASH: return KEY_BACKSLASH;
		case SDL_SCANCODE_KP_1: return KEY_KP1;
		case SDL_SCANCODE_KP_2: return KEY_KP2;
		case SDL_SCANCODE_KP_3: return KEY_KP3;
		case SDL_SCANCODE_KP_4: return KEY_KP4;
		case SDL_SCANCODE_KP_5: return KEY_KP5;
		case SDL_SCANCODE_KP_6: return KEY_KP6;
		case SDL_SCANCODE_KP_7: return KEY_KP7;
		case SDL_SCANCODE_KP_8: return KEY_KP8;
		case SDL_SCANCODE_KP_9: return KEY_KP9;
		case SDL_SCANCODE_KP_0: return KEY_KP0;
		case SDL_SCANCODE_KP_PLUS: return KEY_KPPLUS;
		case SDL_SCANCODE_KP_MINUS: return KEY_KPMINUS;
		case SDL_SCANCODE_KP_MULTIPLY: return KEY_KPSTAR;
		case SDL_SCANCODE_KP_DIVIDE: return KEY_KPSLASH;
		case SDL_SCANCODE_KP_PERIOD: return KEY_KPDEL;
		case SDL_SCANCODE_CAPSLOCK: return KEY_CAPS;
		case SDL_SCANCODE_NUMLOCKCLEAR: return KEY_NUM;
		case SDL_SCANCODE_SCROLLLOCK: return KEY_SCROLL;
		case SDL_SCANCODE_PRINTSCREEN: return KEY_SYSREQ;
		default: return KEY_NONE;
	}
}

// ============================================================================
// SDL3INPUTMANAGER IMPLEMENTATION
// ============================================================================

/**
 * Lifecycle
 */
SDL3InputManager::SDL3InputManager(SDL_Window* window)
	: m_window(window),
	  m_mouseNextFree(0),
	  m_mouseNextGet(0),
	  m_keyNextFree(0),
	  m_keyNextGet(0),
	  m_gamepad(nullptr),
	  m_precisionMode(FALSE),
	  m_lastUpdateTime(0),
	  m_isQuitting(FALSE)
{
	memset(m_mouseEvents, 0, sizeof(m_mouseEvents));
	memset(m_keyEvents, 0, sizeof(m_keyEvents));
	TheSDL3InputManager = this;
	
	openFirstGamepad();
	m_lastUpdateTime = SDL_GetTicks();
}

SDL3InputManager::~SDL3InputManager()
{
	closeGamepad();
	SDL3Mouse::freeCursorResources();
	TheSDL3InputManager = nullptr;
}

/**
 * Unified Event Loop
 */
void SDL3InputManager::update()
{
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_EVENT_QUIT:
			case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
				m_isQuitting = true;
				break;

			case SDL_EVENT_GAMEPAD_ADDED:
				if (!m_gamepad) openFirstGamepad();
				break;

			case SDL_EVENT_GAMEPAD_REMOVED:
				if (m_gamepad && event.gdevice.which == SDL_GetGamepadID(m_gamepad)) closeGamepad();
				break;

			case SDL_EVENT_WINDOW_FOCUS_GAINED:
				if (TheMouse) {
					TheMouse->regainFocus();
					TheMouse->refreshCursorCapture();
				}
				break;

			case SDL_EVENT_WINDOW_FOCUS_LOST:
				if (TheMouse) TheMouse->loseFocus();
				break;

			case SDL_EVENT_WINDOW_MOUSE_ENTER:
				if (TheMouse) TheMouse->onCursorMovedInside();
				break;

			case SDL_EVENT_WINDOW_MOUSE_LEAVE:
				if (TheMouse) TheMouse->onCursorMovedOutside();
				break;

			case SDL_EVENT_MOUSE_MOTION:
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
			case SDL_EVENT_MOUSE_BUTTON_UP:
			case SDL_EVENT_MOUSE_WHEEL:
				addMouseSDLEvent(event);
				break;

			case SDL_EVENT_KEY_DOWN:
			case SDL_EVENT_KEY_UP:
				if (!event.key.repeat) addKeyboardSDLEvent(event);
				break;

			case SDL_EVENT_TEXT_INPUT:
				if (TheGameEngine) {
					SDL3GameEngine* engine = dynamic_cast<SDL3GameEngine*>(TheGameEngine);
					if (engine) engine->forwardTextInputEvent(event.text.text);
				}
				break;

			default:
				break;
		}
	}

	processGamepadInput();
}

/**
 * Buffer Management
 */
Bool SDL3InputManager::getNextMouseEvent(SDL_Event& outEvent)
{
	if (m_mouseEvents[m_mouseNextGet].type == SDL_EVENT_FIRST) return FALSE;

	SDL_Event* event = &m_mouseEvents[m_mouseNextGet];
	m_mouseNextGet = (m_mouseNextGet + 1) % MAX_MOUSE_EVENTS;
	
	outEvent = *event;
	event->type = SDL_EVENT_FIRST;
	return TRUE;
}

Bool SDL3InputManager::getNextKeyboardEvent(SDL_Event& outEvent)
{
	if (m_keyEvents[m_keyNextGet].type == SDL_EVENT_FIRST) return FALSE;

	SDL_Event* event = &m_keyEvents[m_keyNextGet];
	m_keyNextGet = (m_keyNextGet + 1) % MAX_KEY_EVENTS;

	outEvent = *event;
	event->type = SDL_EVENT_FIRST;
	return TRUE;
}

void SDL3InputManager::addMouseSDLEvent(const SDL_Event& event)
{
	UnsignedInt nextFree = (m_mouseNextFree + 1) % MAX_MOUSE_EVENTS;
	if (nextFree == m_mouseNextGet) return;
	m_mouseEvents[m_mouseNextFree] = event;
	m_mouseNextFree = nextFree;
}

void SDL3InputManager::addKeyboardSDLEvent(const SDL_Event& event)
{
	UnsignedInt nextFree = (m_keyNextFree + 1) % MAX_KEY_EVENTS;
	if (nextFree == m_keyNextGet) return;
	m_keyEvents[m_keyNextFree] = event;
	m_keyNextFree = nextFree;
}

/**
 * Gamepad Logic
 */
void SDL3InputManager::openFirstGamepad()
{
	int count = 0;
	SDL_JoystickID* joysticks = SDL_GetGamepads(&count);
	if (joysticks) {
		for (int i = 0; i < count; ++i) {
			m_gamepad = SDL_OpenGamepad(joysticks[i]);
			if (m_gamepad) {
				DEBUG_LOG(("SDL3InputManager: Opened gamepad: %s", SDL_GetGamepadName(m_gamepad)));
				break;
			}
		}
		SDL_free(joysticks);
	}
}

void SDL3InputManager::closeGamepad()
{
	if (m_gamepad) {
		SDL_CloseGamepad(m_gamepad);
		m_gamepad = nullptr;
	}
}

void SDL3InputManager::virtualPulseKey(SDL_Scancode scancode, bool down)
{
	SDL_Event keyEvent;
	memset(&keyEvent, 0, sizeof(keyEvent));
	keyEvent.type = down ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
	keyEvent.key.scancode = scancode;
	keyEvent.key.down = down;
	
	if (scancode == SDL_SCANCODE_LCTRL) keyEvent.key.mod = SDL_KMOD_LCTRL;
	else if (scancode == SDL_SCANCODE_LSHIFT) keyEvent.key.mod = SDL_KMOD_LSHIFT;
	else if (scancode == SDL_SCANCODE_LALT) keyEvent.key.mod = SDL_KMOD_LALT;

	addKeyboardSDLEvent(keyEvent);
}

void SDL3InputManager::virtualPulseMouse(Uint8 button, bool down)
{
	SDL_Event clickEvent;
	memset(&clickEvent, 0, sizeof(clickEvent));
	clickEvent.type = down ? SDL_EVENT_MOUSE_BUTTON_DOWN : SDL_EVENT_MOUSE_BUTTON_UP;
	clickEvent.button.button = button;
	clickEvent.button.clicks = 1;
	clickEvent.button.down = down;
	
	float mx, my;
	SDL_GetMouseState(&mx, &my);
	clickEvent.button.x = mx;
	clickEvent.button.y = my;

	if (m_window)
	{
		clickEvent.button.windowID = SDL_GetWindowID(m_window);
	}
	
	addMouseSDLEvent(clickEvent);
}

void SDL3InputManager::handleGamepadButton(SDL_GamepadButton button, bool& currentState, bool isDown, std::function<void(bool)> action)
{
	if (isDown != currentState) {
		action(isDown);
		currentState = isDown;
	}
}

void SDL3InputManager::processGamepadInput()
{
	if (!m_gamepad) return;

	Uint64 now = SDL_GetTicks();
	float deltaTime = (now - m_lastUpdateTime) / 1000.0f;
	m_lastUpdateTime = now;

	const float DEADZONE = DEFAULT_DEADZONE;
	const float CURSOR_SPEED = DEFAULT_CURSOR_SPEED;

	// 1. TRIGGERS (Modifiers & Precision)
	bool ltPressed = SDL_GetGamepadAxis(m_gamepad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER) > TRIGGER_THRESHOLD;
	if (ltPressed != m_state.ltDown) {
		m_state.ltDown = ltPressed;
		m_precisionMode = m_state.ltDown;
	}

	bool rtPressed = SDL_GetGamepadAxis(m_gamepad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) > TRIGGER_THRESHOLD;
	if (rtPressed != m_state.rtDown) {
		m_state.rtDown = rtPressed;
		virtualPulseKey(SDL_SCANCODE_LCTRL, m_state.rtDown);
	}

	// 2. STICKS (Movement & Panning)
	float lx = SDL_GetGamepadAxis(m_gamepad, SDL_GAMEPAD_AXIS_LEFTX) / AXIS_MAX;
	float ly = SDL_GetGamepadAxis(m_gamepad, SDL_GAMEPAD_AXIS_LEFTY) / AXIS_MAX;

	if (SDL_fabsf(lx) > DEADZONE || SDL_fabsf(ly) > DEADZONE) {
		float speed = CURSOR_SPEED;
		if (m_precisionMode) speed *= 0.3f;

		SDL_Event motionEvent;
		memset(&motionEvent, 0, sizeof(motionEvent));
		motionEvent.type = SDL_EVENT_MOUSE_MOTION;
		motionEvent.motion.xrel = lx * speed * deltaTime;
		motionEvent.motion.yrel = ly * speed * deltaTime;
		
		float mx, my;
		SDL_GetMouseState(&mx, &my);
		motionEvent.motion.x = mx + motionEvent.motion.xrel;
		motionEvent.motion.y = my + motionEvent.motion.yrel;

		if (m_window)
		{
			motionEvent.motion.windowID = SDL_GetWindowID(m_window);
		}
		
		addMouseSDLEvent(motionEvent);
		SDL_WarpMouseInWindow(m_window, motionEvent.motion.x, motionEvent.motion.y);
	}

	float rx = SDL_GetGamepadAxis(m_gamepad, SDL_GAMEPAD_AXIS_RIGHTX) / AXIS_MAX;
	float ry = SDL_GetGamepadAxis(m_gamepad, SDL_GAMEPAD_AXIS_RIGHTY) / AXIS_MAX;

	handleGamepadButton(SDL_GAMEPAD_BUTTON_INVALID, m_state.stickLeft, rx < -DEADZONE, [&](bool d){ virtualPulseKey(SDL_SCANCODE_LEFT, d); });
	handleGamepadButton(SDL_GAMEPAD_BUTTON_INVALID, m_state.stickRight, rx > DEADZONE, [&](bool d){ virtualPulseKey(SDL_SCANCODE_RIGHT, d); });
	handleGamepadButton(SDL_GAMEPAD_BUTTON_INVALID, m_state.stickUp, ry < -DEADZONE, [&](bool d){ virtualPulseKey(SDL_SCANCODE_UP, d); });
	handleGamepadButton(SDL_GAMEPAD_BUTTON_INVALID, m_state.stickDown, ry > DEADZONE, [&](bool d){ virtualPulseKey(SDL_SCANCODE_DOWN, d); });

	// 3. BUTTONS & D-PAD (Actions & Hotkeys)
	handleGamepadButton(SDL_GAMEPAD_BUTTON_SOUTH, m_state.buttonState[SDL_GAMEPAD_BUTTON_SOUTH], SDL_GetGamepadButton(m_gamepad, SDL_GAMEPAD_BUTTON_SOUTH), [&](bool d){ virtualPulseMouse(SDL_BUTTON_LEFT, d); });
	handleGamepadButton(SDL_GAMEPAD_BUTTON_EAST, m_state.buttonState[SDL_GAMEPAD_BUTTON_EAST], SDL_GetGamepadButton(m_gamepad, SDL_GAMEPAD_BUTTON_EAST), [&](bool d){ virtualPulseMouse(SDL_BUTTON_RIGHT, d); });
	handleGamepadButton(SDL_GAMEPAD_BUTTON_WEST, m_state.buttonState[SDL_GAMEPAD_BUTTON_WEST], SDL_GetGamepadButton(m_gamepad, SDL_GAMEPAD_BUTTON_WEST), [&](bool d){ virtualPulseKey(SDL_SCANCODE_A, d); });
	handleGamepadButton(SDL_GAMEPAD_BUTTON_NORTH, m_state.buttonState[SDL_GAMEPAD_BUTTON_NORTH], SDL_GetGamepadButton(m_gamepad, SDL_GAMEPAD_BUTTON_NORTH), [&](bool d){ if (d) TheMessageStream->appendMessage(GameMessage::MSG_META_STOP); });
	handleGamepadButton(SDL_GAMEPAD_BUTTON_LEFT_SHOULDER, m_state.buttonState[SDL_GAMEPAD_BUTTON_LEFT_SHOULDER], SDL_GetGamepadButton(m_gamepad, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER), [&](bool d){ virtualPulseKey(SDL_SCANCODE_Q, d); });
	handleGamepadButton(SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, m_state.buttonState[SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER], SDL_GetGamepadButton(m_gamepad, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER), [&](bool d){ virtualPulseKey(SDL_SCANCODE_LSHIFT, d); });
	handleGamepadButton(SDL_GAMEPAD_BUTTON_START, m_state.buttonState[SDL_GAMEPAD_BUTTON_START], SDL_GetGamepadButton(m_gamepad, SDL_GAMEPAD_BUTTON_START), [&](bool d){ virtualPulseKey(SDL_SCANCODE_ESCAPE, d); });
	handleGamepadButton(SDL_GAMEPAD_BUTTON_BACK, m_state.buttonState[SDL_GAMEPAD_BUTTON_BACK], SDL_GetGamepadButton(m_gamepad, SDL_GAMEPAD_BUTTON_BACK), [&](bool d){ virtualPulseKey(SDL_SCANCODE_SPACE, d); });
	handleGamepadButton(SDL_GAMEPAD_BUTTON_DPAD_LEFT, m_state.buttonState[SDL_GAMEPAD_BUTTON_DPAD_LEFT], SDL_GetGamepadButton(m_gamepad, SDL_GAMEPAD_BUTTON_DPAD_LEFT), [&](bool d){ virtualPulseKey(SDL_SCANCODE_1, d); });
	handleGamepadButton(SDL_GAMEPAD_BUTTON_DPAD_UP, m_state.buttonState[SDL_GAMEPAD_BUTTON_DPAD_UP], SDL_GetGamepadButton(m_gamepad, SDL_GAMEPAD_BUTTON_DPAD_UP), [&](bool d){ virtualPulseKey(SDL_SCANCODE_2, d); });
	handleGamepadButton(SDL_GAMEPAD_BUTTON_DPAD_RIGHT, m_state.buttonState[SDL_GAMEPAD_BUTTON_DPAD_RIGHT], SDL_GetGamepadButton(m_gamepad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT), [&](bool d){ virtualPulseKey(SDL_SCANCODE_3, d); });
	handleGamepadButton(SDL_GAMEPAD_BUTTON_DPAD_DOWN, m_state.buttonState[SDL_GAMEPAD_BUTTON_DPAD_DOWN], SDL_GetGamepadButton(m_gamepad, SDL_GAMEPAD_BUTTON_DPAD_DOWN), [&](bool d){ virtualPulseKey(SDL_SCANCODE_4, d); });
	handleGamepadButton(SDL_GAMEPAD_BUTTON_LEFT_STICK, m_state.buttonState[SDL_GAMEPAD_BUTTON_LEFT_STICK], SDL_GetGamepadButton(m_gamepad, SDL_GAMEPAD_BUTTON_LEFT_STICK), [&](bool d){ if (d) TheMessageStream->appendMessage(GameMessage::MSG_META_SELECT_NEXT_IDLE_WORKER); });
	handleGamepadButton(SDL_GAMEPAD_BUTTON_RIGHT_STICK, m_state.buttonState[SDL_GAMEPAD_BUTTON_RIGHT_STICK], SDL_GetGamepadButton(m_gamepad, SDL_GAMEPAD_BUTTON_RIGHT_STICK), [&](bool d){ if (d) TheMessageStream->appendMessage(GameMessage::MSG_META_VIEW_COMMAND_CENTER); });
}
