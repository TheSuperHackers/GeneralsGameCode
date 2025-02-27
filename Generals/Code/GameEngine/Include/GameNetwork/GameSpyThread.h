/*
**	Command & Conquer Generals(tm)
**	Copyright 2025 Electronic Arts Inc.
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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: GameSpyThread.h //////////////////////////////////////////////////////
// Generals GameSpy thread class
// Author: Matthew D. Campbell, March 2002

#pragma once

#include "mutex.h"
#include "thread.h"

class GameSpyThreadClass : public ThreadClass
{

public:
	GameSpyThreadClass::GameSpyThreadClass() : ThreadClass() { m_doLogin = false; m_readStats = false; m_updateWins = false; m_updateLosses = false; m_updateLocale = false; m_showLocaleSelect = false; m_nextShellScreen.clear(); }
	void queueLogin(AsciiString nick, AsciiString pass, AsciiString email) { m_nick = nick; m_pass = pass; m_email = email; m_doLogin = true; }
	void queueReadPersistentStatsFromServer( void ) { m_readStats = true; }
	void queueUpdateLocale( AsciiString locale ) { m_locale = locale; m_updateLocale = true; }
	void queueUpdateWins  ( AsciiString wins   ) { m_wins   = wins;   m_updateWins   = true; }
	void queueUpdateLosses( AsciiString losses ) { m_losses = losses; m_updateLosses = true; }

	void Thread_Function();

	AsciiString getNextShellScreen( void );
	Bool showLocaleSelect( void );

	void setNextShellScreen( AsciiString nextShellScreen );
	void setShowLocaleSelect( Bool val );

private:
	AsciiString m_nick, m_pass, m_email;
	Bool m_doLogin, m_readStats, m_updateWins, m_updateLosses, m_updateLocale;
	AsciiString m_locale, m_wins, m_losses;
	AsciiString m_nextShellScreen;
	Bool m_showLocaleSelect;
};

extern GameSpyThreadClass *TheGameSpyThread;
extern MutexClass TheGameSpyMutex;
