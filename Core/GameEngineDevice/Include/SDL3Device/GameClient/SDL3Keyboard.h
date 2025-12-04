#pragma once

#include "GameClient/Keyboard.h"

#include <SDL3/SDL.h>

// TheSuperHackers @feature denysmitin 02/12/2025 SDL3 keyboard device implementation.
class SDL3Keyboard : public Keyboard
{
public:
	SDL3Keyboard();
	~SDL3Keyboard() override;

	void init(void) override;
	void reset(void) override;
	Bool getCapsState(void) override;
	void handleEvent(const SDL_Event &event);

	static SDL3Keyboard *getInstance();

protected:
	void getKey(KeyboardIO *key) override;

private:
	struct KeyEvent
	{
		Bool valid;
		KeyboardIO data;
	};

	void pushEvent(const KeyboardIO &keyEvent);
	void handleKeyEvent(const SDL_Event &event);
	void handleTextInputEvent(const SDL_Event &event);
	void handleTextEditingEvent(const SDL_Event &event);
	void sendTextToFocusedWindow(const char *utf8Text);
	void ensureTextInputActive(void);
	void stopTextInputSession(void);
	SDL_Window *resolveTextInputWindow(void) const;
	static UnsignedByte translateScancode(SDL_Scancode scancode);

	static SDL3Keyboard *s_instance;

	enum
	{
		KEYBOARD_EVENT_BUFFER = 256
	};
	KeyEvent m_events[KEYBOARD_EVENT_BUFFER];
	UnsignedInt m_nextFreeIndex;
	UnsignedInt m_nextGetIndex;
	UnsignedInt m_sequence;
	SDL_Window *m_textInputWindow;
};
