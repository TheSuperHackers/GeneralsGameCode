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

// This static class loads and unloads binkw32.dll at runtime.
//
// The Bink functions declared in bink.h are implemented in this class and forward to the
// matching export of the loaded module. Loading the module explicitly, rather than importing
// binkw32.dll into the executable, is what allows the library to be picked up from a working
// directory that was chosen on the command line, because the process no longer needs to
// resolve it before WinMain runs.
//
// When the module is not loaded, every Bink function returns the same neutral value the Bink
// SDK stub library used to return, so a missing binkw32.dll disables video playback instead
// of preventing the program from starting.
//
// Is not thread safe. Every call to load needs a paired call to unload, no matter if the load
// was successful, and both are expected to be called from the main thread while no Bink
// function is in flight.

class BinkLoader
{
public:

	// Returns whether binkw32.dll is loaded
	static bool isLoaded();

	// Returns whether binkw32.dll was attempted to be loaded but failed
	static bool isFailed();

	// Returns the system error code of the failed load attempt
	static unsigned long getLastError();

	static bool load();
	static void unload();
};
