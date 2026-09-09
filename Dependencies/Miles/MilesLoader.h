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

// This static class loads and unloads mss32.dll at runtime.
//
// The Miles functions declared in mss/mss.h are implemented in this class and forward to
// the matching export of the loaded module. Loading the module explicitly, rather than importing
// mss32.dll into the executable, is what allows the library to be picked up from a working
// directory that was chosen on the command line, because the process no longer needs to resolve
// it before WinMain runs.
//
// A missing export is not an error. The retail mss32.dll does not export every function the
// engine was built against, and an unresolved function returns the same neutral value the Miles
// SDK stub library used to return. A missing mss32.dll therefore disables audio instead of
// preventing the program from starting.
//
// Is not thread safe. Every call to load needs a paired call to unload, no matter if the load
// was successful. Both are expected to be called from the main thread, before AIL_startup has
// created the Miles timer thread and after AIL_shutdown has ended it, which is why the forwarding
// functions can stay free of locking.

class MilesLoader
{
public:

	// Returns whether mss32.dll is loaded
	static bool isLoaded();

	// Returns whether mss32.dll was attempted to be loaded but failed
	static bool isFailed();

	// Returns the system error code of the failed load attempt
	static unsigned long getLastError();

	static bool load();
	static void unload();
};
