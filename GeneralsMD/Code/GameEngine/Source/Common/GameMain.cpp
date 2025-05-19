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
#include "GameClient/GameClient.h"

#include "Common/WorkerProcess.h"


int SimulateReplayListMultiProcess(const std::vector<AsciiString> &filenames);

// TheSuperHackers @feature helmutbuhler 13/04/2025
// Simulate a list of replays without graphics.
// Returns exitcode 1 if mismatch or other error occured
int SimulateReplayList(const std::vector<AsciiString> &filenames)
{
	if (filenames.size() != 1)
	{
		return SimulateReplayListMultiProcess(filenames);
	}

	// Note that we use printf here because this is run from cmd.
	int numErrors = 0;
	DWORD totalStartTime = GetTickCount();
	for (size_t i = 0; i < filenames.size(); i++)
	{
		AsciiString filename = filenames[i];
		printf("Simulating Replay \"%s\"\n", filename.str());
		fflush(stdout);
		DWORD startTime = GetTickCount();
		if (TheRecorder->simulateReplay(filename))
		{
			UnsignedInt totalTime = TheRecorder->getFrameDuration() / LOGICFRAMES_PER_SECOND;
			while (TheRecorder->isPlaybackInProgress())
			{
				TheGameClient->updateHeadless();
				//TheParticleSystemManager->reset();

				if (TheGameLogic->getFrame() && TheGameLogic->getFrame() % (600*LOGICFRAMES_PER_SECOND) == 0)
				{
					// Print progress report
					UnsignedInt gameTime = TheGameLogic->getFrame() / LOGICFRAMES_PER_SECOND;
					UnsignedInt realTime = (GetTickCount()-startTime) / 1000;
					printf("Elapsed Time: %02d:%02d Game Time: %02d:%02d/%02d:%02d\n",
							realTime/60, realTime%60, gameTime/60, gameTime%60, totalTime/60, totalTime%60);
					fflush(stdout);
				}
				TheGameLogic->UPDATE();
				if (TheRecorder->sawCRCMismatch())
				{
					numErrors++;
					break;
				}
			}
			UnsignedInt gameTime = TheGameLogic->getFrame() / LOGICFRAMES_PER_SECOND;
			UnsignedInt realTime = (GetTickCount()-startTime) / 1000;
			printf("Elapsed Time: %02d:%02d Game Time: %02d:%02d/%02d:%02d\n",
					realTime/60, realTime%60, gameTime/60, gameTime%60, totalTime/60, totalTime%60);
			fflush(stdout);
		}
		else
		{
			printf("Cannot open replay\n");
			numErrors++;
		}
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

	// TheSuperHackers @todo helmutbuhler 04/05/2025
	// Some replays cause a crash inside TheGameLogic->clearGameData().
	// It likely has to do with Contains. We really gotta fix this, but for now we just terminate.
	TerminateProcess(GetCurrentProcess(), numErrors != 0 ? 1 : 0);

	// TheSuperHackers @todo helmutbuhler 04/13/2025
	// There is a bug somewhere in the destructor of TheGameEngine which doesn't properly
	// clean up the players and causes a crash unless this is called.
	if (TheGameLogic->isInGame())
	{
		TheGameLogic->clearGameData();
	}
	return numErrors != 0 ? 1 : 0;
}

int SimulateReplayListMultiProcess(const std::vector<AsciiString> &filenames)
{
	DWORD totalStartTime = GetTickCount();

	WideChar exePath[1024];
	GetModuleFileNameW(NULL, exePath, 1024);

	std::vector<WorkerProcess> processes;
	const int maxProcesses = 20;
	int filenamePositionStarted = 0;
	int filenamePositionDone = 0;
	int numErrors = 0;

	while (true)
	{
		int i;
		for (i = 0; i < processes.size(); i++)
			processes[i].Update();

		// Get result of finished processes and print output in order
		while (processes.size() != 0)
		{
			DWORD exitcode;
			AsciiString stdOutput;
			if (!processes[0].IsDone(&exitcode, &stdOutput))
				break;
			printf("%d/%d %s", filenamePositionDone+1, filenames.size(), stdOutput.str());
			if (exitcode != 0)
				printf("Error!\n");
			fflush(stdout);
			numErrors += exitcode == 0 ? 0 : 1;
			processes.erase(processes.begin());
			filenamePositionDone++;
		}

		// Count how many processes are running
		int numProcessesRunning = 0;
		for (i = 0; i < processes.size(); i++)
		{
			if (processes[i].IsRunning())
				numProcessesRunning++;
		}

		// Add new processes when we are below the limit and there are replays left
		while (numProcessesRunning < maxProcesses && filenamePositionStarted < filenames.size())
		{
			UnicodeString filenameWide;
			filenameWide.translate(filenames[filenamePositionStarted]);
			UnicodeString command;
			command.format(L"\"%s\" -headless -simReplay \"%s\"", exePath, filenameWide.str());

			WorkerProcess p;
			p.StartProcess(command);
			processes.push_back(p);

			filenamePositionStarted++;
			numProcessesRunning++;
		}

		if (processes.empty())
			break;

		// Don't waste CPU here, our workers need every bit of CPU time they can get
		Sleep(100);
	}

	DEBUG_ASSERTCRASH(filenamePositionStarted == filenames.size(), ("inconsistent file position 1"));
	DEBUG_ASSERTCRASH(filenamePositionDone == filenames.size(), ("inconsistent file position 2"));

	if (numErrors)
		printf("Errors occured: %d\n", numErrors);
	else
		printf("Successfully simulated all replays\n");

	UnsignedInt realTime = (GetTickCount()-totalStartTime) / 1000;
	printf("Total Wall Time: %d:%02d:%02d\n", realTime/60/60, realTime/60%60, realTime%60);
	fflush(stdout);

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

