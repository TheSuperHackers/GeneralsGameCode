#include "SDL3Device/GameClient/SDL3Keyboard.h"

#include "GameClient/GameWindowManager.h"
#include "GameClient/IMEManager.h"
#include "GameClient/KeyDefs.h"

#include <string>

#if defined(_WIN32)
#include <windows.h>
#endif

SDL3Keyboard *SDL3Keyboard::s_instance = nullptr;

SDL3Keyboard::SDL3Keyboard()
	: m_nextFreeIndex(0), m_nextGetIndex(0), m_sequence(0), m_textInputWindow(NULL)
{
	s_instance = this;

	for (KeyEvent &event : m_events)
	{
		event.valid = FALSE;
		event.data.key = KEY_NONE;
		event.data.status = KeyboardIO::STATUS_UNUSED;
		event.data.state = KEY_STATE_NONE;
		event.data.sequence = 0;
	}
}

SDL3Keyboard::~SDL3Keyboard()
{
	stopTextInputSession();

	if (s_instance == this)
	{
		s_instance = nullptr;
	}
}

void SDL3Keyboard::init(void)
{
	Keyboard::init();

	ensureTextInputActive();
}

void SDL3Keyboard::reset(void)
{
	Keyboard::reset();

	m_nextFreeIndex = 0;
	m_nextGetIndex = 0;

	for (KeyEvent &event : m_events)
	{
		event.valid = FALSE;
	}

	ensureTextInputActive();
}

Bool SDL3Keyboard::getCapsState(void)
{
	const SDL_Keymod modifiers = SDL_GetModState();
	return (modifiers & SDL_KMOD_CAPS) != 0;
}

SDL3Keyboard *SDL3Keyboard::getInstance()
{
	return s_instance;
}

void SDL3Keyboard::getKey(KeyboardIO *key)
{
	if (key == nullptr)
	{
		return;
	}

	if (!m_events[m_nextGetIndex].valid)
	{
		key->key = KEY_NONE;
		key->state = KEY_STATE_NONE;
		key->status = KeyboardIO::STATUS_UNUSED;
		key->sequence = 0;
		return;
	}

	*key = m_events[m_nextGetIndex].data;
	m_events[m_nextGetIndex].valid = FALSE;

	m_nextGetIndex++;
	if (m_nextGetIndex >= KEYBOARD_EVENT_BUFFER)
	{
		m_nextGetIndex = 0;
	}
}

void SDL3Keyboard::pushEvent(const KeyboardIO &keyEvent)
{
	if (m_events[m_nextFreeIndex].valid)
	{
		return;
	}

	m_events[m_nextFreeIndex].data = keyEvent;
	m_events[m_nextFreeIndex].valid = TRUE;

	m_nextFreeIndex++;
	if (m_nextFreeIndex >= KEYBOARD_EVENT_BUFFER)
	{
		m_nextFreeIndex = 0;
	}
}

void SDL3Keyboard::handleEvent(const SDL_Event &event)
{
	switch (event.type)
	{
	case SDL_EVENT_KEY_DOWN:
	case SDL_EVENT_KEY_UP:
		ensureTextInputActive();
		handleKeyEvent(event);
		break;
	case SDL_EVENT_TEXT_INPUT:
		ensureTextInputActive();
		handleTextInputEvent(event);
		break;
	case SDL_EVENT_TEXT_EDITING:
		ensureTextInputActive();
		handleTextEditingEvent(event);
		break;
	default:
		break;
	}
}

void SDL3Keyboard::handleKeyEvent(const SDL_Event &event)
{
	const UnsignedByte translated = translateScancode(event.key.scancode);
	if (translated == KEY_NONE)
	{
		return;
	}

	KeyboardIO keyEvent = {};
	keyEvent.key = translated;
	keyEvent.state = (event.type == SDL_EVENT_KEY_DOWN) ? KEY_STATE_DOWN : KEY_STATE_UP;
	if (event.key.repeat)
	{
		keyEvent.state = static_cast<UnsignedShort>(keyEvent.state | KEY_STATE_AUTOREPEAT);
	}
	keyEvent.status = KeyboardIO::STATUS_UNUSED;
	keyEvent.sequence = ++m_sequence;

	pushEvent(keyEvent);
}

void SDL3Keyboard::handleTextInputEvent(const SDL_Event &event)
{
	sendTextToFocusedWindow(event.text.text);
}

void SDL3Keyboard::handleTextEditingEvent(const SDL_Event &event)
{
	/*
	 * SDL provides composition previews via text editing events. The legacy IME
	 * manager already handles Windows composition messages, so for now we ignore
	 * these events and rely on committed text delivered via SDL_EVENT_TEXT_INPUT.
	 */
	(void)event;
}

void SDL3Keyboard::ensureTextInputActive(void)
{
	SDL_Window *window = resolveTextInputWindow();
	if (window == NULL)
	{
		stopTextInputSession();
		return;
	}

	if (m_textInputWindow == window)
	{
		return;
	}

	if (m_textInputWindow != NULL && m_textInputWindow != window)
	{
		SDL_StopTextInput(m_textInputWindow);
	}

	SDL_StartTextInput(window);
	m_textInputWindow = window;
}

void SDL3Keyboard::stopTextInputSession(void)
{
	if (m_textInputWindow != NULL)
	{
		SDL_StopTextInput(m_textInputWindow);
		m_textInputWindow = NULL;
	}
}

SDL_Window *SDL3Keyboard::resolveTextInputWindow(void) const
{
	SDL_Window *window = SDL_GetKeyboardFocus();
	if (window != NULL)
	{
		return window;
	}

	window = SDL_GetMouseFocus();
	if (window != NULL)
	{
		return window;
	}

	window = SDL_GetGrabbedWindow();
	if (window != NULL)
	{
		return window;
	}

	return NULL;
}

void SDL3Keyboard::sendTextToFocusedWindow(const char *utf8Text)
{
	if (utf8Text == NULL || utf8Text[0] == '\0')
	{
		return;
	}

	// TheSuperHackers @bugfix denysmitin 19/12/2025 Forward SDL text input to GUI windows.
	GameWindow *targetWindow = NULL;
	if (TheIMEManager != NULL)
	{
		targetWindow = TheIMEManager->getWindow();
	}
	if (targetWindow == NULL && TheWindowManager != NULL)
	{
		targetWindow = TheWindowManager->winGetFocus();
	}
	if (targetWindow == NULL || TheWindowManager == NULL)
	{
		return;
	}

#if defined(_WIN32)
	const int requiredLength = MultiByteToWideChar(CP_UTF8, 0, utf8Text, -1, NULL, 0);
	if (requiredLength <= 0)
	{
		return;
	}

	std::wstring wideText(static_cast<size_t>(requiredLength), L'\0');
	if (MultiByteToWideChar(CP_UTF8, 0, utf8Text, -1, wideText.data(), requiredLength) <= 0)
	{
		return;
	}

	for (wchar_t ch : wideText)
	{
		if (ch == L'\0')
		{
			break;
		}

		TheWindowManager->winSendInputMsg(targetWindow, GWM_IME_CHAR, static_cast<WindowMsgData>(ch), 0);
	}
#else
	for (const unsigned char *cursor = reinterpret_cast<const unsigned char *>(utf8Text); *cursor != 0; ++cursor)
	{
		TheWindowManager->winSendInputMsg(targetWindow, GWM_IME_CHAR, static_cast<WindowMsgData>(*cursor), 0);
	}
#endif
}

UnsignedByte SDL3Keyboard::translateScancode(SDL_Scancode scancode)
{
	switch (scancode)
	{
	case SDL_SCANCODE_ESCAPE:
		return KEY_ESC;
	case SDL_SCANCODE_F1:
		return KEY_F1;
	case SDL_SCANCODE_F2:
		return KEY_F2;
	case SDL_SCANCODE_F3:
		return KEY_F3;
	case SDL_SCANCODE_F4:
		return KEY_F4;
	case SDL_SCANCODE_F5:
		return KEY_F5;
	case SDL_SCANCODE_F6:
		return KEY_F6;
	case SDL_SCANCODE_F7:
		return KEY_F7;
	case SDL_SCANCODE_F8:
		return KEY_F8;
	case SDL_SCANCODE_F9:
		return KEY_F9;
	case SDL_SCANCODE_F10:
		return KEY_F10;
	case SDL_SCANCODE_F11:
		return KEY_F11;
	case SDL_SCANCODE_F12:
		return KEY_F12;
	case SDL_SCANCODE_PRINTSCREEN:
		return KEY_SYSREQ;
	case SDL_SCANCODE_SCROLLLOCK:
		return KEY_SCROLL;
	case SDL_SCANCODE_PAUSE:
		return KEY_NONE;
	case SDL_SCANCODE_GRAVE:
		return KEY_TICK;
	case SDL_SCANCODE_1:
		return KEY_1;
	case SDL_SCANCODE_2:
		return KEY_2;
	case SDL_SCANCODE_3:
		return KEY_3;
	case SDL_SCANCODE_4:
		return KEY_4;
	case SDL_SCANCODE_5:
		return KEY_5;
	case SDL_SCANCODE_6:
		return KEY_6;
	case SDL_SCANCODE_7:
		return KEY_7;
	case SDL_SCANCODE_8:
		return KEY_8;
	case SDL_SCANCODE_9:
		return KEY_9;
	case SDL_SCANCODE_0:
		return KEY_0;
	case SDL_SCANCODE_MINUS:
		return KEY_MINUS;
	case SDL_SCANCODE_EQUALS:
		return KEY_EQUAL;
	case SDL_SCANCODE_BACKSPACE:
		return KEY_BACKSPACE;
	case SDL_SCANCODE_TAB:
		return KEY_TAB;
	case SDL_SCANCODE_Q:
		return KEY_Q;
	case SDL_SCANCODE_W:
		return KEY_W;
	case SDL_SCANCODE_E:
		return KEY_E;
	case SDL_SCANCODE_R:
		return KEY_R;
	case SDL_SCANCODE_T:
		return KEY_T;
	case SDL_SCANCODE_Y:
		return KEY_Y;
	case SDL_SCANCODE_U:
		return KEY_U;
	case SDL_SCANCODE_I:
		return KEY_I;
	case SDL_SCANCODE_O:
		return KEY_O;
	case SDL_SCANCODE_P:
		return KEY_P;
	case SDL_SCANCODE_LEFTBRACKET:
		return KEY_LBRACKET;
	case SDL_SCANCODE_RIGHTBRACKET:
		return KEY_RBRACKET;
	case SDL_SCANCODE_RETURN:
		return KEY_ENTER;
	case SDL_SCANCODE_CAPSLOCK:
		return KEY_CAPS;
	case SDL_SCANCODE_A:
		return KEY_A;
	case SDL_SCANCODE_S:
		return KEY_S;
	case SDL_SCANCODE_D:
		return KEY_D;
	case SDL_SCANCODE_F:
		return KEY_F;
	case SDL_SCANCODE_G:
		return KEY_G;
	case SDL_SCANCODE_H:
		return KEY_H;
	case SDL_SCANCODE_J:
		return KEY_J;
	case SDL_SCANCODE_K:
		return KEY_K;
	case SDL_SCANCODE_L:
		return KEY_L;
	case SDL_SCANCODE_SEMICOLON:
		return KEY_SEMICOLON;
	case SDL_SCANCODE_APOSTROPHE:
		return KEY_APOSTROPHE;
	case SDL_SCANCODE_BACKSLASH:
		return KEY_BACKSLASH;
	case SDL_SCANCODE_LSHIFT:
		return KEY_LSHIFT;
	case SDL_SCANCODE_NONUSBACKSLASH:
		return KEY_102;
	case SDL_SCANCODE_Z:
		return KEY_Z;
	case SDL_SCANCODE_X:
		return KEY_X;
	case SDL_SCANCODE_C:
		return KEY_C;
	case SDL_SCANCODE_V:
		return KEY_V;
	case SDL_SCANCODE_B:
		return KEY_B;
	case SDL_SCANCODE_N:
		return KEY_N;
	case SDL_SCANCODE_M:
		return KEY_M;
	case SDL_SCANCODE_COMMA:
		return KEY_COMMA;
	case SDL_SCANCODE_PERIOD:
		return KEY_PERIOD;
	case SDL_SCANCODE_SLASH:
		return KEY_SLASH;
	case SDL_SCANCODE_RSHIFT:
		return KEY_RSHIFT;
	case SDL_SCANCODE_KP_MULTIPLY:
		return KEY_KPSTAR;
	case SDL_SCANCODE_LCTRL:
		return KEY_LCTRL;
	case SDL_SCANCODE_SPACE:
		return KEY_SPACE;
	case SDL_SCANCODE_LALT:
		return KEY_LALT;
	case SDL_SCANCODE_RALT:
		return KEY_RALT;
	case SDL_SCANCODE_RCTRL:
		return KEY_RCTRL;
	case SDL_SCANCODE_LEFT:
		return KEY_LEFT;
	case SDL_SCANCODE_RIGHT:
		return KEY_RIGHT;
	case SDL_SCANCODE_UP:
		return KEY_UP;
	case SDL_SCANCODE_DOWN:
		return KEY_DOWN;
	case SDL_SCANCODE_INSERT:
		return KEY_INS;
	case SDL_SCANCODE_DELETE:
		return KEY_DEL;
	case SDL_SCANCODE_HOME:
		return KEY_HOME;
	case SDL_SCANCODE_END:
		return KEY_END;
	case SDL_SCANCODE_PAGEUP:
		return KEY_PGUP;
	case SDL_SCANCODE_PAGEDOWN:
		return KEY_PGDN;
	case SDL_SCANCODE_NUMLOCKCLEAR:
		return KEY_NUM;
	case SDL_SCANCODE_KP_0:
		return KEY_KP0;
	case SDL_SCANCODE_KP_1:
		return KEY_KP1;
	case SDL_SCANCODE_KP_2:
		return KEY_KP2;
	case SDL_SCANCODE_KP_3:
		return KEY_KP3;
	case SDL_SCANCODE_KP_4:
		return KEY_KP4;
	case SDL_SCANCODE_KP_5:
		return KEY_KP5;
	case SDL_SCANCODE_KP_6:
		return KEY_KP6;
	case SDL_SCANCODE_KP_7:
		return KEY_KP7;
	case SDL_SCANCODE_KP_8:
		return KEY_KP8;
	case SDL_SCANCODE_KP_9:
		return KEY_KP9;
	case SDL_SCANCODE_KP_MINUS:
		return KEY_KPMINUS;
	case SDL_SCANCODE_KP_PLUS:
		return KEY_KPPLUS;
	case SDL_SCANCODE_KP_PERIOD:
		return KEY_KPDEL;
	case SDL_SCANCODE_KP_ENTER:
		return KEY_KPENTER;
	case SDL_SCANCODE_KP_DIVIDE:
		return KEY_KPSLASH;
	default:
		break;
	}

	return KEY_NONE;
}
