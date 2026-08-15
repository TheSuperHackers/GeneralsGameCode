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
#include "WWLib/trim.h"

namespace rts
{

Bool setCurrentDirectoryToExecutablePath()
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

Bool setCurrentDirectoryToPath(const char *path)
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

static char *nextWorkingDirectoryParam(char *newSource, const char *seps)
{
	static char *source = nullptr;
	if (newSource)
	{
		source = newSource;
	}
	if (!source)
	{
		return nullptr;
	}

	char *first = source;
	if (first)
	{
		char *firstSep = strpbrk(first, seps);
		char firstChar[2] = {0,0};
		if (firstSep == first)
		{
			firstChar[0] = *first;
			while (*first == firstChar[0]) first++;
		}

		char *end;
		if (firstChar[0])
			end = strpbrk(first, firstChar);
		else
			end = strpbrk(first, seps);

		if (end)
		{
			source = end+1;
			*end = 0;

			if (!*source)
				source = nullptr;
		}
		else
		{
			source = nullptr;
		}

		if (first && !*first)
			first = nullptr;
	}

	return first;
}

void applyStartupWorkingDirectory()
{
	std::vector<char*> argv;
	std::string cmdLine = GetCommandLineA();
	char *token = nextWorkingDirectoryParam(&cmdLine[0], "\" ");
	while (token != nullptr)
	{
		argv.push_back(strtrim(token));
		token = nextWorkingDirectoryParam(nullptr, "\" ");
	}

	const int argc = (int)argv.size();
	for (int arg = 1; arg < argc; ++arg)
	{
		if (stricmp(argv[arg], "-cwd") != 0)
			continue;

		if (arg + 1 < argc && argv[arg + 1] != nullptr && argv[arg + 1][0] != '-' && argv[arg + 1][0] != '\0')
		{
			if (!setCurrentDirectoryToPath(argv[arg + 1]))
				setCurrentDirectoryToExecutablePath();
		}
		return;
	}

	setCurrentDirectoryToExecutablePath();
}

} // namespace rts
