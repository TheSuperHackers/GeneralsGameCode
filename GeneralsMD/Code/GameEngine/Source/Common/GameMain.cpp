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
#include "Common/Recorder.h"
#include "Common/CRCDebug.h"
#include "GameLogic/GameLogic.h"

// TheSuperHackers @feature helmutbuhler 04/13/2025
// Simulate a list of replays without graphics.
// Returns exitcode 1 if mismatch occured
int SimulateReplayList(const std::vector<AsciiString> &filenames)
{
	// Note that we use printf here because this is run from cmd.
	Bool sawCRCMismatch = false;
	for (size_t i = 0; i < TheGlobalData->m_simulateReplayList.size(); i++)
	{
		AsciiString filename = TheGlobalData->m_simulateReplayList[i];
		printf("Simulating Replay %s\n", filename.str());
		if (TheRecorder->simulateReplay(filename))
		{
			do
			{
				{
					VERIFY_CRC
				}
				TheGameLogic->UPDATE();
				sawCRCMismatch = TheRecorder->sawCRCMismatch();
			} while (TheRecorder->isAnalysisInProgress() && !sawCRCMismatch);
		}
		if (sawCRCMismatch)
			break;
	}
	if (!sawCRCMismatch)
		printf("Successfully simulated all replays\n");

	// TheSuperHackers @todo helmutbuhler 04/13/2025
	// There is a bug somewhere in the destructor of TheGameEngine which doesn't properly
	// clean up the players and causes a crash in debug unless this is called.
	if (TheGameLogic->isInGame())
	{
		TheGameLogic->clearGameData();
	}
	return sawCRCMismatch ? 1 : 0;
}

/**
 * This is the entry point for the game system.
 */
Int GameMain( int argc, char *argv[] )
{
	int exitcode = 0;
	// initialize the game engine using factory function
	TheGameEngine = CreateGameEngine();
	TheGameEngine->init(argc, argv);

	if (!TheGlobalData->m_simulateReplayList.empty())
	{
		exitcode = SimulateReplayList(TheGlobalData->m_simulateReplayList);
	}
	else
	{
		// run it
		TheGameEngine->execute();
	}

	// since execute() returned, we are exiting the game
	delete TheGameEngine;
	TheGameEngine = NULL;

	return exitcode;
}

