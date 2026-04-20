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

#include "Common/GameEngine.h"
#include <SDL3/SDL.h>

// EXTERNALS
// SDL3 window typically provided by WinMain integration
extern SDL_Window* TheSDL3Window;

// Forward declarations for base classes
class AudioManager;
class Mouse;
class Keyboard;
class GameWindow;
class LocalFileSystem;
class ArchiveFileSystem;
class ThingFactory;
class ModuleFactory;
class FunctionLexicon;
class Radar;
class WebBrowser;
class ParticleSystemManager;

/**
 * SDL3GameEngine
 *
 * GameEngine subclass that uses SDL3 for windowing and input.
 * Replaces or supplements Win32-specific window handling with SDL3.
 */
class SDL3GameEngine : public GameEngine
{
public:
	SDL3GameEngine();
	virtual ~SDL3GameEngine();

	// GameEngine interface
	virtual void init(void) override;
	virtual void reset(void) override;
	virtual void update(void) override;
	virtual void serviceWindowsOS(void) override;
	virtual Bool isActive(void) override;
	virtual void setIsActive(Bool isActive) override;

	// Factory methods (override GameEngine)
	virtual LocalFileSystem *createLocalFileSystem(void) override;
	virtual ArchiveFileSystem *createArchiveFileSystem(void) override;
	virtual GameLogic *createGameLogic(void) override;
	virtual GameClient *createGameClient(void) override;
	virtual ModuleFactory *createModuleFactory(void) override;
	virtual ThingFactory *createThingFactory(void) override;
	virtual FunctionLexicon *createFunctionLexicon(void) override;
	virtual Radar *createRadar(Bool dummy) override;
	virtual WebBrowser *createWebBrowser(void) override;
	virtual ParticleSystemManager* createParticleSystemManager(Bool dummy) override;
	virtual AudioManager *createAudioManager(Bool dummy) override;

	// SDL3 specific
	virtual SDL_Window* getSDLWindow(void) const { return m_SDLWindow; }
	virtual void forwardTextInputEvent(const char* utf8Text);

protected:
	SDL_Window*		m_SDLWindow;
	Bool			m_IsInitialized;
	Bool			m_IsActive;
	Bool			m_IsTextInputActive;
	GameWindow*	m_TextInputFocusWindow;

	// Event processing
	void pollSDL3Events(void);
	void updateTextInputState(void);
};
