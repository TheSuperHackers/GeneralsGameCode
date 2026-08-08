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

// Forward declarations
class GameEngine;

/**
 * Shared application lifecycle management for both Win32 and SDL3 entry points.
 */
namespace AppMain
{
	/** Initialize core subsystems before window creation (memory, working dir, debug flags, command line) */
	Bool initBeforeWindow();

	/** Initialize subsystems after window creation (version info, single-instance mutex check) */
	Bool initAfterWindow();

	/** Execute the main game loop */
	Int run();

	/** Get initial window dimensions from GlobalData or defaults (800x600) */
	void getInitialWindowBounds(Int& outWidth, Int& outHeight);

	/** Update application focus active state across backends */
	void setAppActive(Bool active);

	/** Get current application focus active state */
	Bool isAppActive();

	/** Get the localized or fallback splash screen bitmap file path */
	void getSplashFilePath(Char* outBuffer, UnsignedInt bufferSize);

	/** Shutdown core subsystems (version, minidump, memory manager) */
	void shutdown();
}

/** Engine creation helper */
GameEngine* CreateGameEngine();

/** Main engine loop */
Int GameMain();
