#include "SDL3Device/Common/SDL3GameEngine.h"
#include <SDL3/SDL.h>
#include "Common/GlobalData.h"
#include "GameClient/Display.h"
#include "GameClient/IMEManager.h"
#include "GameClient/Keyboard.h"
#include "GameClient/Mouse.h"
#include "Platform/SDLPlatformWindow.h"

namespace
{
struct DisplaySizeChange
{
	bool changed = false;
	UnsignedInt oldWidth = 0;
	UnsignedInt oldHeight = 0;
	UnsignedInt newWidth = 0;
	UnsignedInt newHeight = 0;
};

SDL_Window *windowForEvent(const SDL_Event &event)
{
	SDL_WindowID windowID = 0;
	switch (event.type)
	{
	case SDL_EVENT_KEY_DOWN:
	case SDL_EVENT_KEY_UP:
		windowID = event.key.windowID;
		break;
	case SDL_EVENT_MOUSE_WHEEL:
		windowID = event.wheel.windowID;
		break;
	case SDL_EVENT_WINDOW_RESIZED:
	case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
	case SDL_EVENT_WINDOW_DISPLAY_CHANGED:
	case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
	case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
	case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
	case SDL_EVENT_WINDOW_FOCUS_GAINED:
	case SDL_EVENT_WINDOW_FOCUS_LOST:
		windowID = event.window.windowID;
		break;
	default:
		break;
	}

	if (windowID != 0)
		return SDL_GetWindowFromID(windowID);
	return SDL_GetKeyboardFocus();
}

DisplaySizeChange synchronizeDisplayToWindow(SDL_Window *window)
{
	if (window == nullptr || TheDisplay == nullptr || TheTacticalView == nullptr ||
		TheDisplay->getWidth() == 0 || TheDisplay->getHeight() == 0)
	{
		return {};
	}

	int pixelWidth = 0;
	int pixelHeight = 0;
	if (!SDL_GetWindowSizeInPixels(window, &pixelWidth, &pixelHeight) ||
		pixelWidth <= 0 || pixelHeight <= 0)
	{
		return {};
	}

	const Bool windowed = (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN) == 0 ? TRUE : FALSE;
	const UnsignedInt bitDepth = TheDisplay->getBitDepth() != 0 ?
		TheDisplay->getBitDepth() : DEFAULT_DISPLAY_BIT_DEPTH;
	if (TheDisplay->getWidth() == static_cast<UnsignedInt>(pixelWidth) &&
		TheDisplay->getHeight() == static_cast<UnsignedInt>(pixelHeight) &&
		TheDisplay->getWindowed() == windowed)
	{
		if (TheWritableGlobalData != nullptr)
			TheWritableGlobalData->m_windowed = windowed;
		return {};
	}

	const UnsignedInt oldWidth = TheDisplay->getWidth();
	const UnsignedInt oldHeight = TheDisplay->getHeight();
	if (!TheDisplay->setDisplayMode(static_cast<UnsignedInt>(pixelWidth),
		static_cast<UnsignedInt>(pixelHeight), bitDepth, windowed))
	{
		return {};
	}

	if (TheWritableGlobalData != nullptr)
		TheWritableGlobalData->m_windowed = windowed;

	return { true, oldWidth, oldHeight, TheDisplay->getWidth(), TheDisplay->getHeight() };
}

DisplaySizeChange toggleFullscreen()
{
	SDL_Window *platformWindow = static_cast<SDL_Window *>(SDLPlatformWindow::window());
	if (platformWindow == nullptr)
		return {};

	const bool fullscreen = SDLPlatformWindow::isFullscreen();
	if (!SDLPlatformWindow::setFullscreen(!fullscreen))
		return {};

	return synchronizeDisplayToWindow(platformWindow);
}
}

void SDL3GameEngine::update()
{
	serviceSDL3();
	GameEngine::update();
}

void SDL3GameEngine::serviceSDL3()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDLPlatformWindow::fullscreenToggleEventType())
		{
			const DisplaySizeChange displayChange = toggleFullscreen();
			if (displayChange.changed)
				onDisplaySizeChanged(displayChange.oldWidth, displayChange.oldHeight,
					displayChange.newWidth, displayChange.newHeight);
			if (TheKeyboard)
				TheKeyboard->resetKeys();
			continue;
		}

		if (event.type == SDL_EVENT_MOUSE_WHEEL && TheMouse != nullptr)
		{
			Real wheelDelta = static_cast<Real>(event.wheel.y);
			if (event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED)
				wheelDelta = -wheelDelta;
			TheMouse->addWheelDelta(wheelDelta);
		}

		if (event.type == SDL_EVENT_KEY_DOWN &&
			(event.key.scancode == SDL_SCANCODE_RETURN || event.key.scancode == SDL_SCANCODE_KP_ENTER) &&
			!event.key.repeat && (event.key.mod & SDL_KMOD_ALT) != 0)
		{
			const DisplaySizeChange displayChange = toggleFullscreen();
			if (displayChange.changed)
				onDisplaySizeChanged(displayChange.oldWidth, displayChange.oldHeight,
					displayChange.newWidth, displayChange.newHeight);
			if (TheKeyboard)
				TheKeyboard->resetKeys();
			continue;
		}

		if (event.type == SDL_EVENT_WINDOW_RESIZED ||
			event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED ||
			event.type == SDL_EVENT_WINDOW_DISPLAY_CHANGED ||
			event.type == SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED ||
			event.type == SDL_EVENT_WINDOW_ENTER_FULLSCREEN ||
			event.type == SDL_EVENT_WINDOW_LEAVE_FULLSCREEN)
		{
			SDL_Window *window = windowForEvent(event);
			const DisplaySizeChange displayChange = synchronizeDisplayToWindow(window);
			if (displayChange.changed)
				onDisplaySizeChanged(displayChange.oldWidth, displayChange.oldHeight,
					displayChange.newWidth, displayChange.newHeight);
			continue;
		}

		if (TheIMEManager != nullptr &&
			(TheIMEManager->serviceIMEMessage(&event, event.type, 0, 0)))
			continue;
		if (event.type == SDL_EVENT_QUIT)
			setQuitting(true);
		else if (event.type == SDL_EVENT_WINDOW_FOCUS_GAINED)
		{
			setIsActive(true);
			if (TheKeyboard)
				TheKeyboard->resetKeys();
			if (TheMouse)
				TheMouse->regainFocus();
		}
		else if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST)
		{
			setIsActive(false);
			if (TheKeyboard)
				TheKeyboard->resetKeys();
			if (TheMouse)
				TheMouse->loseFocus();
		}
	}
}
