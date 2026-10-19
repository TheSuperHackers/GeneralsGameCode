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

// C ABI only. No allocation or C++ object crosses the game/DLL boundary.
// The game loads these exports explicitly so a missing DLL permits fallback.
typedef int (__cdecl* RtsCrashpadInitializeFunction)(const char* userDirectory,
    const char* game, const char* version, const char* revision, int dirty);
typedef void (__cdecl* RtsCrashpadCaptureFatalFunction)();
typedef void (__cdecl* RtsCrashpadShutdownFunction)();

extern "C" int __cdecl RtsCrashpadInitialize(const char* userDirectory,
    const char* game, const char* version, const char* revision, int dirty);
extern "C" void __cdecl RtsCrashpadCaptureFatal();
extern "C" void __cdecl RtsCrashpadShutdown();
