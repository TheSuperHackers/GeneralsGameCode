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

#include "Lib/BaseType.h"
#include "SDL3Main.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "AppMain.h"
#include "Common/CommandLine.h"
#include "Common/Debug.h"
#include "Common/GlobalData.h"
#include "Common/Registry.h"
#include "SDL3Device/Common/SDL3GameEngine.h"

SDL_Window* TheSDL3Window = nullptr;
#ifdef _WIN32
HINSTANCE ApplicationHInstance = nullptr;
HWND ApplicationHWnd = nullptr;
#endif

static SDL_Surface* gLoadScreenSurface = nullptr;

GameEngine* CreateGameEngine()
{
	SDL3GameEngine* engine = NEW SDL3GameEngine;
	engine->setIsActive(true);
	return engine;
}

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	Int exitcode = 1;

	if (!AppMain::initBeforeWindow())
	{
		return exitcode;
	}

#ifdef _WIN32
	ApplicationHInstance = GetModuleHandle(nullptr);
#endif

	// Load splash screen surface
	char filePath[512];
	AppMain::getSplashFilePath(filePath, sizeof(filePath));
	gLoadScreenSurface = SDL_LoadBMP(filePath);

	if (!TheGlobalData->m_headless)
	{
		if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_GAMEPAD))
		{
			DEBUG_LOG(("SDL_Init failed: %s", SDL_GetError()));
			AppMain::shutdown();
			return exitcode;
		}

		Uint32 flags = SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE;
		if (!TheGlobalData->m_windowed)
		{
			flags |= SDL_WINDOW_FULLSCREEN;
		}

		Int startWidth, startHeight;
		AppMain::getInitialWindowBounds(startWidth, startHeight);

		TheSDL3Window = SDL_CreateWindow("Command & Conquer Generals", startWidth, startHeight, flags);
		if (!TheSDL3Window)
		{
			DEBUG_LOG(("SDL_CreateWindow failed: %s", SDL_GetError()));
			SDL_Quit();
			AppMain::shutdown();
			return exitcode;
		}

#ifdef _WIN32
		SDL_PropertiesID props = SDL_GetWindowProperties(TheSDL3Window);
		ApplicationHWnd = (HWND)SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
#endif

		SDL_ShowWindow(TheSDL3Window);
		AppMain::setAppActive(true);

		// Render splash screen onto SDL window surface
		if (gLoadScreenSurface != nullptr)
		{
			SDL_Surface* screen = SDL_GetWindowSurface(TheSDL3Window);
			if (screen)
			{
				SDL_ClearSurface(screen, 0.0f, 0.0f, 0.0f, 1.0f);
				float bitmapAspect = 800.0f / 600.0f;
				int drawWidth = (float)screen->w / screen->h > bitmapAspect ? (int)(screen->h * bitmapAspect) : screen->w;
				int drawHeight = (float)screen->w / screen->h > bitmapAspect ? screen->h : (int)(screen->w / bitmapAspect);
				SDL_Rect destRect = { (screen->w - drawWidth) / 2, (screen->h - drawHeight) / 2, drawWidth, drawHeight };
				SDL_BlitSurfaceScaled(gLoadScreenSurface, nullptr, screen, &destRect, SDL_SCALEMODE_LINEAR);
				SDL_UpdateWindowSurface(TheSDL3Window);
			}
		}
	}

	if (gLoadScreenSurface != nullptr)
	{
		SDL_DestroySurface(gLoadScreenSurface);
		gLoadScreenSurface = nullptr;
	}

	if (!AppMain::initAfterWindow())
	{
		if (TheSDL3Window)
		{
			SDL_DestroyWindow(TheSDL3Window);
			TheSDL3Window = nullptr;
		}
		SDL_Quit();
		return exitcode;
	}

	exitcode = AppMain::run();

	if (TheSDL3Window)
	{
		SDL_DestroyWindow(TheSDL3Window);
		TheSDL3Window = nullptr;
	}
	SDL_Quit();

	AppMain::shutdown();

	return exitcode;
}
