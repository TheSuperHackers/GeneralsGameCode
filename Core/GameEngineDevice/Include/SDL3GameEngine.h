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

class SDL3GameEngine : public GameEngine
{
public:
	SDL3GameEngine();
	virtual ~SDL3GameEngine();

	// GameEngine interface
	virtual void init() override;
	virtual void reset() override;
	virtual void update() override;
	virtual void serviceWindowsOS() override;
	virtual Bool isActive() override;
	virtual void setIsActive(Bool isActive) override;

	// Factory methods (override GameEngine)
	virtual LocalFileSystem* createLocalFileSystem() override;
	virtual ArchiveFileSystem* createArchiveFileSystem() override;
	virtual GameLogic* createGameLogic() override;
	virtual GameClient* createGameClient() override;
	virtual ModuleFactory* createModuleFactory() override;
	virtual ThingFactory* createThingFactory() override;
	virtual FunctionLexicon* createFunctionLexicon() override;
	virtual Radar* createRadar(Bool dummy) override;
	virtual WebBrowser* createWebBrowser() override;
	virtual ParticleSystemManager* createParticleSystemManager(Bool dummy) override;
	virtual AudioManager* createAudioManager(Bool dummy) override;

	// SDL3 specific
	virtual SDL_Window* getSDLWindow() const { return m_SDLWindow; }
	virtual void forwardTextInputEvent(const char* utf8Text);

protected:
	SDL_Window* m_SDLWindow;
	Bool m_IsInitialized;
	Bool m_IsActive;
	Bool m_IsTextInputActive;
	GameWindow* m_TextInputFocusWindow;

	// Event processing
	void pollSDL3Events();
	void updateTextInputState();
};
