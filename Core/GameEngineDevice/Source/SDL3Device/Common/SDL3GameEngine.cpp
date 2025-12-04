#include "SDL3Device/Common/SDL3GameEngine.h"

#include "MilesAudioDevice/MilesAudioManager.h"
#include "StdDevice/Common/StdBIGFileSystem.h"
#include "StdDevice/Common/StdLocalFileSystem.h"
#include "W3DDevice/Common/W3DFunctionLexicon.h"
#include "W3DDevice/Common/W3DModuleFactory.h"
#include "W3DDevice/Common/W3DRadar.h"
#include "W3DDevice/Common/W3DThingFactory.h"
#include "W3DDevice/GameClient/W3DGameClient.h"
#include "W3DDevice/GameClient/W3DParticleSys.h"
#include "W3DDevice/GameClient/W3DWebBrowser.h"
#include "W3DDevice/GameLogic/W3DGameLogic.h"

#include "Common/GameSounds.h"
#include "Common/MessageStream.h"
#include "Common/PerfTimer.h"
#include "GameClient/Keyboard.h"
#include "GameClient/Mouse.h"
#include "GameLogic/GameLogic.h"
#include "GameNetwork/LANAPICallbacks.h"

#include <SDL3/SDL.h>
#include "SDL3Device/GameClient/SDL3Keyboard.h"
#include "SDL3Device/GameClient/SDL3Mouse.h"

#if defined(_WIN32)
#include <atlbase.h>
#include <windows.h>
#endif

#if defined(_WIN32)
extern DWORD TheMessageTime;
static UINT s_previousErrorMode = 0;
#endif

SDL3GameEngine::SDL3GameEngine()
{
#if defined(_WIN32)
	s_previousErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
#endif
}

SDL3GameEngine::~SDL3GameEngine()
{
#if defined(_WIN32)
	SetErrorMode(s_previousErrorMode);
#endif
}

void SDL3GameEngine::init(void)
{
	GameEngine::init();
}

void SDL3GameEngine::reset(void)
{
	GameEngine::reset();
}

void SDL3GameEngine::update(void)
{
	GameEngine::update();

#if defined(_WIN32)
	extern HWND ApplicationHWnd;
	if (ApplicationHWnd && ::IsIconic(ApplicationHWnd))
	{
		while (ApplicationHWnd && ::IsIconic(ApplicationHWnd))
		{
			Sleep(5);
			serviceWindowsOS();

			if (TheLAN != NULL)
			{
				TheLAN->setIsActive(isActive());
				TheLAN->update();
			}

			if (TheGameEngine->getQuitting() || TheGameLogic->isInInternetGame() || TheGameLogic->isInLanGame())
			{
				break;
			}
		}
	}
#endif

	serviceWindowsOS();
}

void SDL3GameEngine::serviceWindowsOS(void)
{
	SDL_Event event;
	// TheSuperHackers @feature denysmitin 02/12/2025 Translate SDL events into the existing Windows message loop.
	SDL3Keyboard *keyboard = SDL3Keyboard::getInstance();
	SDL3Mouse *mouse = SDL3Mouse::getInstance();
	while (SDL_PollEvent(&event))
	{
		if (keyboard)
			keyboard->handleEvent(event);

		if (mouse)
			mouse->handleEvent(event);

		switch (event.type)
		{
		case SDL_EVENT_QUIT:
		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
		{
#ifdef RTS_DEBUG
			if (TheMessageStream != NULL)
			{
				TheMessageStream->appendMessage(GameMessage::MSG_META_DEMO_INSTANT_QUIT);
			}
#endif
			if (TheGameEngine != NULL)
			{
				TheGameEngine->setQuitting(TRUE);
			}
#if defined(_WIN32)
			PostQuitMessage(0);
#endif
			break;
		}
		case SDL_EVENT_WINDOW_FOCUS_GAINED:
		{
			if (keyboard)
			{
				keyboard->resetKeys();
			}
			if (TheAudio)
			{
				TheAudio->regainFocus();
			}
			if (TheGameEngine)
			{
				TheGameEngine->setIsActive(TRUE);
			}
			break;
		}
		case SDL_EVENT_WINDOW_FOCUS_LOST:
		{
			if (keyboard)
			{
				keyboard->resetKeys();
			}
			if (TheAudio)
			{
				TheAudio->loseFocus();
			}
			if (TheGameEngine)
			{
				TheGameEngine->setIsActive(FALSE);
			}
			break;
		}
		case SDL_EVENT_WINDOW_MINIMIZED:
		{
			if (TheGameEngine)
			{
				TheGameEngine->setIsActive(FALSE);
			}
			break;
		}
		case SDL_EVENT_WINDOW_RESTORED:
		{
			if (TheGameEngine)
			{
				TheGameEngine->setIsActive(TRUE);
			}
			break;
		}
		case SDL_EVENT_WINDOW_MOVED:
		case SDL_EVENT_WINDOW_RESIZED:
		{
			if (mouse)
			{
				mouse->refreshCursorCapture();
			}
			break;
		}
		default:
			break;
		}
	}

#if defined(_WIN32)
	MSG msg;
	Int returnValue;

	while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		returnValue = GetMessage(&msg, NULL, 0, 0);

		TheMessageTime = msg.time;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		TheMessageTime = 0;
	}
#endif
}

GameLogic *SDL3GameEngine::createGameLogic(void)
{
	return NEW W3DGameLogic;
}

GameClient *SDL3GameEngine::createGameClient(void)
{
	return NEW W3DGameClient;
}

ModuleFactory *SDL3GameEngine::createModuleFactory(void)
{
	return NEW W3DModuleFactory;
}

ThingFactory *SDL3GameEngine::createThingFactory(void)
{
	return NEW W3DThingFactory;
}

FunctionLexicon *SDL3GameEngine::createFunctionLexicon(void)
{
	return NEW W3DFunctionLexicon;
}

LocalFileSystem *SDL3GameEngine::createLocalFileSystem(void)
{
	return NEW StdLocalFileSystem;
}

ArchiveFileSystem *SDL3GameEngine::createArchiveFileSystem(void)
{
	return NEW StdBIGFileSystem;
}

Radar *SDL3GameEngine::createRadar(void)
{
	return NEW W3DRadar;
}

WebBrowser *SDL3GameEngine::createWebBrowser(void)
{
#if defined(_WIN32)
	return NEW CComObject<W3DWebBrowser>;
#else
	return NULL;
#endif
}

ParticleSystemManager *SDL3GameEngine::createParticleSystemManager(void)
{
	return NEW W3DParticleSystemManager;
}

AudioManager *SDL3GameEngine::createAudioManager(void)
{
	return NEW MilesAudioManager;
}
