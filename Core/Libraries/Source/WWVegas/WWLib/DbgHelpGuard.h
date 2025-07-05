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

#include "always.h"


// This class temporarily loads and unloads dbghelp.dll from the desired location to prevent
// other code from potentially loading it from an undesired location.
// This helps avoid crashing on boot using recent AMD/ATI drivers, which attempt to load and use
// dbghelp.dll from the game install directory but are unable to do so without crashing because
// the dbghelp.dll that ships with the game is very old and the AMD/ATI code does not handle
// that correctly.

class DbgHelpGuard
{
public:

	DbgHelpGuard();
	~DbgHelpGuard();

	void activate();
	void deactivate();

private:

	bool m_hasLoaded;
};
