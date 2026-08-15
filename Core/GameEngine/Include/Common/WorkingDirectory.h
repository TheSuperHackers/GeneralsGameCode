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
// Startup working directory helpers. By default the process working directory is
// the executable directory. -cwd keeps the OS directory. -cwd <path> uses that path.

Bool setCurrentDirectoryToExecutablePath();
Bool setCurrentDirectoryToPath(const char *path);

// For tools that do not parse CommandLine startup flags.
void applyStartupWorkingDirectory();

} // namespace rts
