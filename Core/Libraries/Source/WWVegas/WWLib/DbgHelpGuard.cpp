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

#include "DbgHelpGuard.h"

#include "DbgHelpLoader.h"


DbgHelpGuard::DbgHelpGuard()
{
	deactivate();
}

DbgHelpGuard::~DbgHelpGuard()
{
	reactivate();
}

void DbgHelpGuard::deactivate()
{
	DbgHelpLoader::blockLoad();

	if (DbgHelpLoader::isLoadedFromSystem())
	{
		// This is ok. Do nothing.
	}
	else if (DbgHelpLoader::isLoaded())
	{
		DbgHelpLoader::unload();
		m_wasLoaded = true;
		m_dbgHelpRenamer.rename("dbghelp.dll", "dbghelp.dll.bak");
	}
	else
	{
		m_dbgHelpRenamer.rename("dbghelp.dll", "dbghelp.dll.bak");
	}
}

void DbgHelpGuard::reactivate()
{
	m_dbgHelpRenamer.revert();
	DbgHelpLoader::unblockLoad();

	if (m_wasLoaded)
	{
		DbgHelpLoader::load();
	}
}
