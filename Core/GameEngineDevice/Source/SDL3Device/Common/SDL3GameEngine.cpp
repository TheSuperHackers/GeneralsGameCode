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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <SDL3/SDL.h>

#include "Common/GameEngine.h"
#include "GameClient/Gadget.h"
#include "GameClient/GameWindow.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/Keyboard.h"
#include "GameClient/Mouse.h"
#include "GameLogic/GameLogic.h"
#include "GameNetwork/LANAPICallbacks.h"
#include "GameNetwork/NetworkInterface.h"
#include "MilesAudioDevice/MilesAudioManager.h"
#include "SDL3Device/Common/SDL3GameEngine.h"
#include "SDL3Device/GameClient/SDL3Input.h"
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
#include "WWLib/utf8.h"

// Extern globals for input devices (set by GameClient)
extern Mouse* TheMouse;
extern Keyboard* TheKeyboard;
extern GameWindowManager* TheWindowManager;

SDL3GameEngine::SDL3GameEngine()
	: GameEngine()
	, m_SDLWindow(nullptr)
	, m_IsInitialized(false)
	, m_IsActive(false)
	, m_IsTextInputActive(false)
	, m_TextInputFocusWindow(nullptr)
{
}

SDL3GameEngine::~SDL3GameEngine()
{
	if (m_SDLWindow && m_IsTextInputActive)
	{
		SDL_StopTextInput(m_SDLWindow);
		m_IsTextInputActive = false;
		m_TextInputFocusWindow = nullptr;
	}

	if (TheSDL3InputManager)
	{
		delete TheSDL3InputManager;
	}
}

void SDL3GameEngine::init()
{
	// Verify window was created by SDL3Main integration
	extern SDL_Window* TheSDL3Window;
	extern HWND ApplicationHWnd;

	if (TheSDL3Window && ApplicationHWnd)
	{
		// Store window reference locally
		m_SDLWindow = TheSDL3Window;
		m_IsInitialized = true;
		m_IsActive = true;

		// Initialize the unified input manager
		if (!TheSDL3InputManager)
		{
			TheSDL3InputManager = new SDL3InputManager(m_SDLWindow);
		}
	}

	// Call parent init to initialize game subsystems
	GameEngine::init();
}

void SDL3GameEngine::reset()
{
	if (m_SDLWindow && m_IsTextInputActive)
	{
		SDL_StopTextInput(m_SDLWindow);
		m_IsTextInputActive = false;
		m_TextInputFocusWindow = nullptr;
	}
	GameEngine::reset();
}

void SDL3GameEngine::update()
{
	pollSDL3Events();
	GameEngine::update();

	// If the window is minimized, enter a throttled loop to save resources
	// while keeping the network connection alive, matching legacy Win32 behavior.
	if (m_SDLWindow && (SDL_GetWindowFlags(m_SDLWindow) & SDL_WINDOW_MINIMIZED))
	{
		while (m_SDLWindow && (SDL_GetWindowFlags(m_SDLWindow) & SDL_WINDOW_MINIMIZED))
		{
			// Prevent CPU/GPU pinning while alt-tabbed
			SDL_Delay(5);

			// Stay responsive to events (so we can see when we're un-minimized)
			pollSDL3Events();

			// Keep the LAN subsystem alive to prevent multiplayer disconnects
			if (TheLAN != nullptr)
			{
				TheLAN->setIsActive(isActive());
				TheLAN->update();
			}

			// If we are in a network game, we must NOT stay in this loop,
			// as the engine needs to keep pumping logic frames to avoid desyncs.
			if (getQuitting() || (TheGameLogic && (TheGameLogic->isInInternetGame() || TheGameLogic->isInLanGame())))
			{
				break;
			}
		}
	}
}

void SDL3GameEngine::serviceWindowsOS()
{
	pollSDL3Events();
}

Bool SDL3GameEngine::isActive()
{
	return m_IsActive;
}

void SDL3GameEngine::setIsActive(Bool isActive)
{
	m_IsActive = isActive;
}

UnsignedInt SDL3GameEngine::getTicksMsec() const
{
	return (UnsignedInt)SDL_GetTicks();
}

void SDL3GameEngine::pollSDL3Events()
{
	if (!m_SDLWindow || !TheSDL3InputManager)
	{
		return;
	}

	updateTextInputState();

	// Process all events via the dedicated manager
	TheSDL3InputManager->update();

	// Check if we should quit
	if (TheSDL3InputManager->isQuitting())
	{
		m_quitting = true;
	}
}

void SDL3GameEngine::updateTextInputState()
{
	if (!m_SDLWindow || !TheWindowManager)
	{
		return;
	}

	GameWindow* focusedWindow = TheWindowManager->winGetFocus();
	const Bool wantsTextInput =
		focusedWindow != nullptr && (BitIsSet(focusedWindow->winGetStyle(), GWS_ENTRY_FIELD) || BitIsSet(focusedWindow->winGetStyle(), GWS_COMBO_BOX));

	if (wantsTextInput)
	{
		if (!m_IsTextInputActive)
		{
			if (SDL_StartTextInput(m_SDLWindow))
			{
				m_IsTextInputActive = true;
			}
		}
		m_TextInputFocusWindow = focusedWindow;
	}
	else
	{
		if (m_IsTextInputActive)
		{
			SDL_StopTextInput(m_SDLWindow);
			m_IsTextInputActive = false;
		}
		m_TextInputFocusWindow = nullptr;
	}
}

void SDL3GameEngine::forwardTextInputEvent(const char* utf8Text)
{
	if (!utf8Text || !TheWindowManager)
	{
		return;
	}

	GameWindow* targetWindow = m_TextInputFocusWindow;
	if (!targetWindow || !BitIsSet(targetWindow->winGetStyle(), GWS_ENTRY_FIELD))
	{
		return;
	}

	const size_t textLength = strlen(utf8Text);
	const size_t wideLength = Utf8_To_Wide_Len(utf8Text, textLength);
	if (wideLength == UTF8_INVALID || wideLength == 0)
	{
		return;
	}

	WideChar* wideText = NEW WideChar[wideLength + 1];
	Utf8_To_Wide(wideText, wideLength + 1, utf8Text, textLength);

	for (size_t i = 0; i < wideLength; ++i)
	{
		TheWindowManager->winSendInputMsg(targetWindow, GWM_IME_CHAR, static_cast<WindowMsgData>(wideText[i]), 0);
	}

	delete [] wideText;
}

LocalFileSystem* SDL3GameEngine::createLocalFileSystem()
{
	return NEW StdLocalFileSystem;
}

ArchiveFileSystem* SDL3GameEngine::createArchiveFileSystem()
{
	return NEW StdBIGFileSystem;
}

GameLogic* SDL3GameEngine::createGameLogic()
{
	return NEW W3DGameLogic;
}

GameClient* SDL3GameEngine::createGameClient()
{
	return NEW W3DGameClient;
}

ModuleFactory* SDL3GameEngine::createModuleFactory()
{
	return NEW W3DModuleFactory;
}

ThingFactory* SDL3GameEngine::createThingFactory()
{
	return NEW W3DThingFactory;
}

FunctionLexicon* SDL3GameEngine::createFunctionLexicon()
{
	return NEW W3DFunctionLexicon;
}

Radar* SDL3GameEngine::createRadar(Bool dummy)
{
	if (dummy)
		return NEW RadarDummy;
	return NEW W3DRadar;
}

ParticleSystemManager* SDL3GameEngine::createParticleSystemManager(Bool dummy)
{
	if (dummy)
		return NEW ParticleSystemManagerDummy;
	return NEW W3DParticleSystemManager;
}

WebBrowser* SDL3GameEngine::createWebBrowser()
{
	return nullptr;
}

AudioManager* SDL3GameEngine::createAudioManager(Bool dummy)
{
	if (dummy)
		return NEW MilesAudioManagerDummy;
	return NEW MilesAudioManager;
}
