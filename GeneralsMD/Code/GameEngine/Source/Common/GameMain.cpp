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

#if 0
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
#else
class ReplayProcess
{
public:
	ReplayProcess()
	{
		m_processHandle = INVALID_HANDLE_VALUE;
		m_readHandle = INVALID_HANDLE_VALUE;
		m_exitcode = 0;
		m_isDone = false;
	}
	bool StartProcess(AsciiString filename)
	{
		m_stdOutput = "";
		m_isDone = false;

		PROCESS_INFORMATION pi = { 0 };

		SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES) };
	    saAttr.bInheritHandle = TRUE;
		
		HANDLE writeHandle = NULL;
		CreatePipe(&m_readHandle, &writeHandle, &saAttr, 0);
		SetHandleInformation(m_readHandle, HANDLE_FLAG_INHERIT, 0);

		STARTUPINFO si = { sizeof(STARTUPINFO) };
		si.dwFlags = STARTF_FORCEOFFFEEDBACK;
		si.hStdError = writeHandle;
		si.hStdOutput = writeHandle;
		si.dwFlags |= STARTF_USESTDHANDLES;

		AsciiString command;
		command.format("generalszh.exe -win -xres 800 -yres 600 -simReplay \"%s\"", filename.str());
		//printf("Starting Exe for Replay \"%s\": %s\n", filename.str(), command.str());
		fflush(stdout);
		if (!CreateProcessA(NULL, (LPSTR)command.str(),
				NULL, NULL, TRUE, 0,
				NULL, 0, &si, &pi))
		{
			printf("Couldn't start exe: %s\n", command.str());
			fflush(stdout);
			return false;
		}
		CloseHandle(pi.hThread);
		CloseHandle(writeHandle);
		m_processHandle = pi.hProcess;
		return true;
	}
	bool IsRunning() const
	{
		return m_processHandle != INVALID_HANDLE_VALUE;
	}
	void Update()
	{
		if (!IsRunning())
			return;

		while (true)
		{
			// Call PeekNamedPipe to make sure ReadFile won't block
			DWORD bytesAvailable = 0;
			BOOL success = PeekNamedPipe(m_readHandle, NULL, 0, NULL, &bytesAvailable, NULL);
			if (!success)
				break;
			if (bytesAvailable == 0)
			{
				// Child process is still running and we have all output so far
				return;
			}

			DWORD readBytes = 0;
			char buffer[1024];
			success = ReadFile(m_readHandle, buffer, 1024-1, &readBytes, NULL);
			if (!success)
				break;
			DEBUG_ASSERTCRASH(readBytes != 0, ("expected readBytes to be non null"));

			// Remove \r, otherwise each new line it doubled when we output it again
			for (int i = 0; i < readBytes; i++)
				if (buffer[i] == '\r')
					buffer[i] = ' ';
			buffer[readBytes] = 0;
			m_stdOutput.concat(buffer);
		}

		// Pipe broke, that means the process already exited. But we call this just to make sure
		WaitForSingleObject(m_processHandle, INFINITE);
		GetExitCodeProcess(m_processHandle, &m_exitcode);
		CloseHandle(m_processHandle);
		m_processHandle = INVALID_HANDLE_VALUE;

		CloseHandle(m_readHandle);
		m_readHandle = INVALID_HANDLE_VALUE;

		m_isDone = true;
	}
	bool IsDone(DWORD *exitcode, AsciiString *stdOutput)
	{
		*exitcode = m_exitcode;
		*stdOutput = m_stdOutput;
		return m_isDone;
	}
	void Close()
	{
		if (m_processHandle != INVALID_HANDLE_VALUE)
		{
			TerminateProcess(m_processHandle, 1);
			CloseHandle(m_processHandle);
			m_processHandle = INVALID_HANDLE_VALUE;
		}

		CloseHandle(m_readHandle);
		m_readHandle = INVALID_HANDLE_VALUE;

		m_stdOutput = "";
		m_isDone = false;
	}
private:
	HANDLE m_processHandle;
	HANDLE m_readHandle;
	AsciiString m_stdOutput;
	DWORD m_exitcode;
	bool m_isDone;
};
#endif

int SimulateReplayListMultiProcess(const std::vector<AsciiString> &filenames);

// TheSuperHackers @feature helmutbuhler 04/13/2025
// Simulate a list of replays without graphics.
// Returns exitcode 1 if mismatch or other error occured
int SimulateReplayList(const std::vector<AsciiString> &filenames, int argc, char *argv[])
{
	if (filenames.size() != 1)
	{
		return SimulateReplayListMultiProcess(filenames);
	}

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
					TheGameClient->updateHeadless();
					//TheParticleSystemManager->reset();

					if (TheGameLogic->getFrame() && TheGameLogic->getFrame() % (600*fps) == 0)
					{
						// Print progress report
						UnsignedInt gameTime = TheGameLogic->getFrame() / fps;
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
				UnsignedInt gameTime = TheGameLogic->getFrame() / fps;
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
		else
		{
			/*printf("%d/%d ", i+1, filenames.size());
			fflush(stdout);
			//bool error = SimulateReplayInProcess(filename);
			ReplayProcess p;
			p.StartProcess(filename);
			DWORD exitcode;
			AsciiString stdOutput;
			while (!p.Update(&exitcode, &stdOutput))
			{}
			printf("%s", stdOutput.str());
			fflush(stdout);
			numErrors += exitcode == 0 ? 0 : 1;*/
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

int SimulateReplayListMultiProcess(const std::vector<AsciiString> &filenames)
{
	DWORD totalStartTime = GetTickCount();

	std::vector<ReplayProcess> processes;
	const int maxProcesses = 4;
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
			ReplayProcess p;
			p.StartProcess(filenames[filenamePositionStarted]);
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

