/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 TheSuperHackers
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

namespace rts
{

// TheSuperHackers @feature 14/08/2026
// Process working directory helpers. CommandLine::parseCommandLineForStartup()
// applies these: default is the executable directory, -useCwd keeps the OS
// directory, and -setCwd <path> uses that path.

Bool setCurrentDirectoryToExecutablePath();
Bool setCurrentDirectoryToPath(const char *path);

} // namespace rts
