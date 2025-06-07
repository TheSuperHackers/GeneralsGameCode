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

#include "Common/Recorder.h"
#include "Common/WorkerProcess.h"
#include "GameLogic/GameLogic.h"
#include "GameClient/GameClient.h"

static int SimulateReplaysInThisProcess(const std::vector<AsciiString> &filenames)
{
	// Note that we use printf here because this is run from cmd.
	int numErrors = 0;
	DWORD totalStartTimeMillis = GetTickCount();
	for (size_t i = 0; i < filenames.size(); i++)
	{
		AsciiString filename = filenames[i];
		printf("Simulating Replay \"%s\"\n", filename.str());
		fflush(stdout);
		DWORD startTimeMillis = GetTickCount();
		if (TheRecorder->simulateReplay(filename))
		{
			UnsignedInt totalTimeSec = TheRecorder->getPlaybackFrameCount() / LOGICFRAMES_PER_SECOND;
			while (TheRecorder->isPlaybackInProgress())
			{
				TheGameClient->updateHeadless();

				const int progressFrameInterval = 10*60*LOGICFRAMES_PER_SECOND;
				if (TheGameLogic->getFrame() != 0 && TheGameLogic->getFrame() % progressFrameInterval == 0)
				{
					// Print progress report
					UnsignedInt gameTimeSec = TheGameLogic->getFrame() / LOGICFRAMES_PER_SECOND;
					UnsignedInt realTimeSec = (GetTickCount()-startTimeMillis) / 1000;
					printf("Elapsed Time: %02d:%02d Game Time: %02d:%02d/%02d:%02d\n",
							realTimeSec/60, realTimeSec%60, gameTimeSec/60, gameTimeSec%60, totalTimeSec/60, totalTimeSec%60);
					fflush(stdout);
				}
				TheGameLogic->UPDATE();
				if (TheRecorder->sawCRCMismatch())
				{
					numErrors++;
					break;
				}
			}
			UnsignedInt gameTimeSec = TheGameLogic->getFrame() / LOGICFRAMES_PER_SECOND;
			UnsignedInt realTimeSec = (GetTickCount()-startTimeMillis) / 1000;
			printf("Elapsed Time: %02d:%02d Game Time: %02d:%02d/%02d:%02d\n",
					realTimeSec/60, realTimeSec%60, gameTimeSec/60, gameTimeSec%60, totalTimeSec/60, totalTimeSec%60);
			fflush(stdout);
		}
		else
		{
			printf("Cannot open replay\n");
			numErrors++;
		}
	}
	if (filenames.size() > 1)
	{
		if (numErrors)
			printf("Errors occured: %d\n", numErrors);
		else
			printf("Successfully simulated all replays\n");

		UnsignedInt realTime = (GetTickCount()-totalStartTimeMillis) / 1000;
		printf("Total Time: %d:%02d:%02d\n", realTime/60/60, realTime/60%60, realTime%60);
		fflush(stdout);
	}

	return numErrors != 0 ? 1 : 0;
}

static int SimulateReplaysInWorkerProcesses(const std::vector<AsciiString> &filenames, int maxProcesses)
{
	DWORD totalStartTimeMillis = GetTickCount();

	WideChar exePath[1024];
	GetModuleFileNameW(NULL, exePath, ARRAY_SIZE(exePath));

	std::vector<WorkerProcess> processes;
	int filenamePositionStarted = 0;
	int filenamePositionDone = 0;
	int numErrors = 0;

	while (true)
	{
		int i;
		for (i = 0; i < processes.size(); i++)
			processes[i].update();

		// Get result of finished processes and print output in order
		while (!processes.empty())
		{
			if (!processes[0].isDone())
				break;
			AsciiString stdOutput = processes[0].getStdOutput();
			printf("%d/%d %s", filenamePositionDone+1, (int)filenames.size(), stdOutput.str());
			DWORD exitcode = processes[0].getExitCode();
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
			if (processes[i].isRunning())
				numProcessesRunning++;
		}

		// Add new processes when we are below the limit and there are replays left
		while (numProcessesRunning < maxProcesses && filenamePositionStarted < filenames.size())
		{
			UnicodeString filenameWide;
			filenameWide.translate(filenames[filenamePositionStarted]);
			UnicodeString command;
			command.format(L"\"%s\" -simReplay \"%s\"", exePath, filenameWide.str());

			processes.push_back(WorkerProcess());
			processes.back().startProcess(command);

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

	printf("Simulation of all replays completed. Errors occured: %d\n", numErrors);

	UnsignedInt realTime = (GetTickCount()-totalStartTimeMillis) / 1000;
	printf("Total Wall Time: %d:%02d:%02d\n", realTime/60/60, realTime/60%60, realTime%60);
	fflush(stdout);

	return numErrors != 0 ? 1 : 0;
}

int SimulateReplays(const std::vector<AsciiString> &filenames, int maxProcesses)
{
	if (maxProcesses == SIMULATE_REPLAYS_SEQUENTIAL)
		return SimulateReplaysInThisProcess(filenames);
	else
		return SimulateReplaysInWorkerProcesses(filenames, maxProcesses);
}
