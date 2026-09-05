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

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/WorkingDirectory.h"

namespace rts
{

enum WorkingDirectorySelection
{
	WORKING_DIRECTORY_EXECUTABLE,
	WORKING_DIRECTORY_CURRENT,
	WORKING_DIRECTORY_PATH,
};

static WorkingDirectorySelection s_workingDirectorySelection = WORKING_DIRECTORY_EXECUTABLE;
static const Char *s_workingDirectoryPath = nullptr;

static Bool setCurrentDirectoryToExecutablePath()
{
	Char buffer[_MAX_PATH];
	const DWORD len = GetModuleFileName(nullptr, buffer, ARRAY_SIZE(buffer));
	if (len == 0 || len >= ARRAY_SIZE(buffer))
	{
		DEBUG_LOG(("Failed to get executable path for working directory (error %d)", GetLastError()));
		return FALSE;
	}

	if (Char *pEnd = strrchr(buffer, '\\'))
	{
		*pEnd = 0;
	}

	if (::SetCurrentDirectory(buffer) == 0)
	{
		DEBUG_LOG(("Failed to set working directory to executable path '%s' (error %d)", buffer, GetLastError()));
		return FALSE;
	}

	return TRUE;
}

static Bool setCurrentDirectoryToPath(const char *path)
{
	if (path == nullptr || path[0] == '\0')
		return FALSE;

	if (::SetCurrentDirectory(path) == 0)
	{
		DEBUG_LOG(("Failed to set working directory to '%s' (error %d)", path, GetLastError()));
		return FALSE;
	}

	return TRUE;
}

void selectCurrentWorkingDirectory()
{
	s_workingDirectorySelection = WORKING_DIRECTORY_CURRENT;
	s_workingDirectoryPath = nullptr;
}

void selectExecutableWorkingDirectory()
{
	s_workingDirectorySelection = WORKING_DIRECTORY_EXECUTABLE;
	s_workingDirectoryPath = nullptr;
}

void selectWorkingDirectoryPath(const char *path)
{
	s_workingDirectorySelection = WORKING_DIRECTORY_PATH;
	s_workingDirectoryPath = path;
}

void applySelectedWorkingDirectory()
{
	if (s_workingDirectorySelection == WORKING_DIRECTORY_CURRENT)
		return;

	if (s_workingDirectorySelection == WORKING_DIRECTORY_PATH && setCurrentDirectoryToPath(s_workingDirectoryPath))
		return;

	setCurrentDirectoryToExecutablePath();
}

} // namespace rts
