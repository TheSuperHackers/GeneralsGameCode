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

// This static class loads and unloads usp10.dll from the system directory at runtime.
//
// The Uniscribe functions declared in usp10_adapter.h are implemented in this class and forward
// to the matching export of the loaded module, so they can be called as if usp10.dll was linked
// statically. When the module is not loaded, every Uniscribe function returns E_FAIL, or a null
// pointer for ScriptString_pSize.
//
// Is not thread safe. Every call to load needs a paired call to unload, no matter if the load
// was successful, and both are expected to be called while no Uniscribe function is in flight.

class Usp10Loader
{
public:

	// Returns whether usp10.dll is loaded
	static bool isLoaded();

	// Returns whether usp10.dll was attempted to be loaded but failed
	static bool isFailed();

	// Returns the system error code of the failed load attempt
	static unsigned long getLastError();

	static bool load();
	static void unload();
};
