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

#include "ScopedFileRenamer.h"


// This class temporarily unloads dbghelp.dll and prevents it from loading during its lifetime.
// This helps avoid crashing on boot using recent AMD/ATI drivers, which attempt to load and use
// dbghelp.dll from the game install directory but are unable to do so correctly because
// the dbghelp.dll that ships with the game is very old and the AMD/ATI code does not handle
// that correctly. This workaround is not required if the dbghelp.dll was loaded from the system
// directory.

class DbgHelpGuard
{
public:

	DbgHelpGuard();
	~DbgHelpGuard();

	void deactivate();
	void reactivate();

private:

	ScopedFileRenamer m_dbgHelpRenamer;
	bool m_wasLoaded;
};
