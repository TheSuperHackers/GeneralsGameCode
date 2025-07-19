/*
**	Command & Conquer Generals Zero Hour(tm)
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

// GameMain.cpp
// The main entry point for the game
// Author: Michael S. Booth, April 2001

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/GameEngine.h"
#include "Common/ReplaySimulation.h"


static void delayGameInit()
{
	const UnsignedInt showTime = TheGameStartTime + 3000;
	while (showTime > timeGetTime())
	{
		TheGameEngine->serviceWindowsOS();
		Sleep(5);
	}
}

/**
 * This is the entry point for the game system.
 */
Int GameMain()
{
	int exitcode = 0;
	// initialize the game engine using factory function
	TheGameEngine = CreateGameEngine();

	// TheSuperHackers @tweak Delay the game launch for a moment to present the game splash screen.
	if (!TheGlobalData->m_quickstart)
		delayGameInit();

	TheGameEngine->init();

	if (!TheGlobalData->m_simulateReplays.empty())
	{
		exitcode = ReplaySimulation::simulateReplays(TheGlobalData->m_simulateReplays, TheGlobalData->m_simulateReplayJobs);
	}
	else
	{
		TheGameEngine->execute();
	}

	// since execute() returned, we are exiting the game
	delete TheGameEngine;
	TheGameEngine = NULL;

	return exitcode;
}

