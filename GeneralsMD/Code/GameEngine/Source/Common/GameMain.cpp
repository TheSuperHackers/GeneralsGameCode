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
#include "GameLogic/GameLogic.h"

bool SimulateReplayInProcess(AsciiString filename)
{
	STARTUPINFO si = { sizeof(STARTUPINFO) };
	si.dwFlags = STARTF_FORCEOFFFEEDBACK;
	PROCESS_INFORMATION pi = { 0 };
	AsciiString command;
	command.format("generalszh.exe -win -xres 800 -yres 600 -simReplay \"%s\"", filename.str());
	//printf("Starting Exe for Replay \"%s\": %s\n", filename.str(), command.str());
	//fflush(stdout);
	CreateProcessA(NULL, (LPSTR)command.str(),
		NULL, NULL, TRUE, 0,
		NULL, 0, &si, &pi);
	CloseHandle(pi.hThread);
	WaitForSingleObject(pi.hProcess, INFINITE);
	DWORD exitcode = 1;
	GetExitCodeProcess(pi.hProcess, &exitcode);
	CloseHandle(pi.hProcess);
	return exitcode != 0;
}

// TheSuperHackers @feature helmutbuhler 04/13/2025
// Simulate a list of replays without graphics.
// Returns exitcode 1 if mismatch or other error occured
int SimulateReplayList(const std::vector<AsciiString> &filenames, int argc, char *argv[])
{
	// Note that we use printf here because this is run from cmd.
	Int fps = TheGlobalData->m_framesPerSecondLimit;
	int numErrors = 0;
	DWORD totalStartTime = GetTickCount();
	for (size_t i = 0; i < filenames.size(); i++)
	{
		AsciiString filename = filenames[i];
		if (filenames.size() == 1)
		{
			printf("Simulating Replay \"%s\"\n", filename.str());
			fflush(stdout);
			DWORD startTime = GetTickCount();
			if (TheRecorder->simulateReplay(filename))
			{
				UnsignedInt totalTime = TheRecorder->getFrameDuration() / fps;
				while (TheRecorder->isPlaybackInProgress())
				{
					if (TheGameLogic->getFrame() && TheGameLogic->getFrame() % (600*fps) == 0)
					{
						UnsignedInt gameTime = TheGameLogic->getFrame() / fps;
						UnsignedInt realTime = (GetTickCount()-startTime) / 1000;
						printf("Frame %02d:%02d/%02d:%02d  RT: %02d:%02d\n",
							gameTime/60, gameTime%60, totalTime/60, totalTime%60, realTime/60, realTime%60);
						fflush(stdout);
					}
					TheGameLogic->UPDATE();
					if (TheRecorder->sawCRCMismatch())
					{
						numErrors++;
						break;
					}
				}
				UnsignedInt realTime = (GetTickCount()-startTime) / 1000;
				printf("GT: %02d:%02d RT: %02d:%02d\n", totalTime/60, totalTime%60, realTime/60, realTime%60);
				fflush(stdout);
			}
			else
			{
				printf("Cannot open replay\n");
				numErrors++;
			}
		}
		else
		{
			printf("%d/%d ", i+1, filenames.size());
			fflush(stdout);
			bool error = SimulateReplayInProcess(filename);
			numErrors += error ? 1 : 0;
		}
		/*if (i == TheGlobalData->m_simulateReplayList.size()-1)
		{
			if (TheGameLogic->isInGame())
			{
				TheGameLogic->clearGameData();
			}
			delete TheGameEngine;
			TheGameEngine = NULL;
			TheGameEngine = CreateGameEngine();
			TheGameEngine->init(argc, argv);
		}*/
	}
	if (TheGlobalData->m_simulateReplayList.size() > 1)
	{
		if (numErrors)
			printf("Errors occured: %d\n", numErrors);
		else
			printf("Successfully simulated all replays\n");

		UnsignedInt realTime = (GetTickCount()-totalStartTime) / 1000;
		printf("Total Time: %d:%02d:%02d\n", realTime/60/60, realTime/60%60, realTime%60);
		fflush(stdout);
	}

	// TheSuperHackers @todo helmutbuhler 04/13/2025
	// There is a bug somewhere in the destructor of TheGameEngine which doesn't properly
	// clean up the players and causes a crash unless this is called.
	if (TheGameLogic->isInGame())
	{
		TheGameLogic->clearGameData();
	}
	return numErrors != 0 ? 1 : 0;
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
		exitcode = SimulateReplayList(TheGlobalData->m_simulateReplayList, argc, argv);
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

