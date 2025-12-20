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
	: m_needsUnload(false)
{
	activate();
}

DbgHelpGuard::~DbgHelpGuard()
{
	deactivate();
}

void DbgHelpGuard::activate()
{
	// Front load the DLL now to prevent other code from loading the potentially wrong DLL.
	DbgHelpLoader::load();
	m_needsUnload = true;
}

void DbgHelpGuard::deactivate()
{
	if (m_needsUnload)
	{
		DbgHelpLoader::unload();
		m_needsUnload = false;
	}
}
