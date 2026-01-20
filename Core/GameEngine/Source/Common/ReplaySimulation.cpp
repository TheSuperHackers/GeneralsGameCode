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

#include "Common/ReplaySimulation.h"

#include "Common/GameEngine.h"
#include "Common/GameState.h"
#include "Common/LocalFileSystem.h"
#include "Common/Recorder.h"
#include "Common/WorkerProcess.h"
#include "GameLogic/GameLogic.h"
#include "GameClient/GameClient.h"


Bool ReplaySimulation::s_isRunning = false;
UnsignedInt ReplaySimulation::s_replayIndex = 0;
UnsignedInt ReplaySimulation::s_replayCount = 0;

namespace
{
int countProcessesRunning(const std::vector<WorkerProcess>& processes)
{
	int numProcessesRunning = 0;
	size_t i = 0;
	for (; i < processes.size(); ++i)
	{
		if (processes[i].isRunning())
			++numProcessesRunning;
	}
	return numProcessesRunning;
}
} // namespace

int ReplaySimulation::simulateReplaysInThisProcess(const std::vector<AsciiString> &filenames)
{
	int numErrors = 0;

	if (!TheGlobalData->m_headless)
	{
		s_isRunning = true;
		s_replayIndex = 0;
		s_replayCount = static_cast<UnsignedInt>(filenames.size());

		// If we are not in headless mode, we need to run the replay in the engine.
		for (; s_replayIndex < s_replayCount; ++s_replayIndex)
		{
			TheRecorder->playbackFile(filenames[s_replayIndex]);
			TheGameEngine->execute();
			if (TheRecorder->sawCRCMismatch())
				numErrors++;
			if (!s_isRunning)
				break;
			TheGameEngine->setQuitting(FALSE);
		}
		s_isRunning = false;
		s_replayIndex = 0;
		s_replayCount = 0;
		return numErrors != 0 ? 1 : 0;
	}
	// Note that we use printf here because this is run from cmd.
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

				// Check for checkpoint save BEFORE checking for CRC mismatch
				// so we can save checkpoints even if the replay will eventually fail
				if (TheGlobalData->m_replaySaveAtFrame != 0 &&
					TheGameLogic->getFrame() == TheGlobalData->m_replaySaveAtFrame &&
					!TheGlobalData->m_replaySaveTo.isEmpty())
				{
					// TheSuperHackers @info bobtista 19/01/2026
					// Pass just the filename to saveGame() - it will be saved to the Save directory.
					SaveCode result = TheGameState->saveGame(TheGlobalData->m_replaySaveTo, UnicodeString::TheEmptyString, SAVE_FILE_TYPE_NORMAL);
					if (result != SC_OK)
					{
						numErrors++;
					}
					TheRecorder->stopPlayback();
					break;
				}

				if (TheRecorder->sawCRCMismatch())
				{
					DEBUG_LOG(("CRC mismatch at frame %d", TheGameLogic->getFrame()));
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
		printf("Simulation of all replays completed. Errors occurred: %d\n", numErrors);

		UnsignedInt realTime = (GetTickCount()-totalStartTimeMillis) / 1000;
		printf("Total Time: %d:%02d:%02d\n", realTime/60/60, realTime/60%60, realTime%60);
		fflush(stdout);
	}

	return numErrors != 0 ? 1 : 0;
}

int ReplaySimulation::simulateReplaysInWorkerProcesses(const std::vector<AsciiString> &filenames, int maxProcesses)
{
	DWORD totalStartTimeMillis = GetTickCount();

	WideChar exePath[1024];
	GetModuleFileNameW(nullptr, exePath, ARRAY_SIZE(exePath));

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

		int numProcessesRunning = countProcessesRunning(processes);

		// Add new processes when we are below the limit and there are replays left
		while (numProcessesRunning < maxProcesses && filenamePositionStarted < filenames.size())
		{
			UnicodeString filenameWide;
			filenameWide.translate(filenames[filenamePositionStarted]);
			UnicodeString command;
			command.format(L"\"%s\"%s%s -replay \"%s\"",
				exePath,
				TheGlobalData->m_windowed ? L" -win" : L"",
				TheGlobalData->m_headless ? L" -headless" : L"",
				filenameWide.str());

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

	printf("Simulation of all replays completed. Errors occurred: %d\n", numErrors);

	UnsignedInt realTime = (GetTickCount()-totalStartTimeMillis) / 1000;
	printf("Total Wall Time: %d:%02d:%02d\n", realTime/60/60, realTime/60%60, realTime%60);
	fflush(stdout);

	return numErrors != 0 ? 1 : 0;
}

std::vector<AsciiString> ReplaySimulation::resolveFilenameWildcards(const std::vector<AsciiString> &filenames)
{
	// If some filename contains wildcards, search for actual filenames.
	// Note that we cannot do this in parseReplay because we require TheLocalFileSystem initialized.
	std::vector<AsciiString> filenamesResolved;
	for (std::vector<AsciiString>::const_iterator filename = filenames.begin(); filename != filenames.end(); ++filename)
	{
		if (filename->find('*') || filename->find('?'))
		{
			AsciiString dir1 = TheRecorder->getReplayDir();
			AsciiString dir2 = *filename;
			AsciiString wildcard = *filename;
			{
				int len = dir2.getLength();
				while (len)
				{
					char c = dir2.getCharAt(len-1);
					if (c == '/' || c == '\\')
					{
						wildcard.set(wildcard.str()+dir2.getLength());
						break;
					}
					dir2.removeLastChar();
					len--;
				}
			}

			FilenameList files;
			TheLocalFileSystem->getFileListInDirectory(dir2.str(), dir1.str(), wildcard, files, FALSE);
			for (FilenameList::iterator it = files.begin(); it != files.end(); ++it)
			{
				AsciiString file;
				file.set(it->str() + dir1.getLength());
				filenamesResolved.push_back(file);
			}
		}
		else
			filenamesResolved.push_back(*filename);
	}
	return filenamesResolved;
}

int ReplaySimulation::simulateReplays(const std::vector<AsciiString> &filenames, int maxProcesses)
{
	std::vector<AsciiString> filenamesResolved = resolveFilenameWildcards(filenames);
	if (maxProcesses == SIMULATE_REPLAYS_SEQUENTIAL)
		return simulateReplaysInThisProcess(filenamesResolved);
	else
		return simulateReplaysInWorkerProcesses(filenamesResolved, maxProcesses);
}

int ReplaySimulation::continueReplayFromCheckpoint(const AsciiString &checkpointFile)
{
	int numErrors = 0;

	// TheSuperHackers @info bobtista 19/01/2026
	// We need to initialize the replay BEFORE loading the checkpoint because:
	// 1. loadGame() calls startNewGame() which creates team prototypes based on the map
	// 2. During replay playback, startNewGame() uses TheRecorder->getGameInfo() to get the correct team setup
	// 3. But CHUNK_Recorder is loaded AFTER startNewGame() runs
	// So we must initialize the replay first to provide the correct game info.
	if (TheGlobalData->m_simulateReplays.empty())
	{
		DEBUG_LOG(("No replay file specified for checkpoint loading"));
		return 1;
	}

	AsciiString replayFile = TheGlobalData->m_simulateReplays[0];
	DEBUG_LOG(("Initializing replay from %s before loading checkpoint", replayFile.str()));

	// Initialize the recorder from the replay file so that when loadGame() calls
	// startNewGame(), it will have the correct game info from the replay header.
	if (!TheRecorder->initializeReplayForCheckpointLoad(replayFile))
	{
		DEBUG_LOG(("Failed to initialize replay from %s", replayFile.str()));
		return 1;
	}

	DEBUG_LOG(("Loading checkpoint from %s", checkpointFile.str()));

	AvailableGameInfo gameInfo;
	gameInfo.filename = checkpointFile;
	TheGameState->getSaveGameInfoFromFile(checkpointFile, &gameInfo.saveGameInfo);

	SaveCode result = TheGameState->loadGame(gameInfo);
	if (result != SC_OK)
	{
		DEBUG_LOG(("Failed to load checkpoint (error %d)", result));
		return 1;
	}

	if (!TheRecorder->isPlaybackMode())
	{
		DEBUG_LOG(("Checkpoint was not saved during replay playback"));
		return 1;
	}

	DEBUG_LOG(("Resuming replay from frame %d", TheGameLogic->getFrame()));

#ifdef _DEBUG
	DWORD startTimeMillis = GetTickCount();
	UnsignedInt totalTimeSec = TheRecorder->getPlaybackFrameCount() / LOGICFRAMES_PER_SECOND;
#endif

	while (TheRecorder->isPlaybackInProgress())
	{
		TheGameClient->updateHeadless();

#ifdef _DEBUG
		const int progressFrameInterval = 10*60*LOGICFRAMES_PER_SECOND;
		if (TheGameLogic->getFrame() != 0 && TheGameLogic->getFrame() % progressFrameInterval == 0)
		{
			UnsignedInt gameTimeSec = TheGameLogic->getFrame() / LOGICFRAMES_PER_SECOND;
			UnsignedInt realTimeSec = (GetTickCount()-startTimeMillis) / 1000;
			DEBUG_LOG(("Elapsed Time: %02d:%02d Game Time: %02d:%02d/%02d:%02d",
					realTimeSec/60, realTimeSec%60, gameTimeSec/60, gameTimeSec%60, totalTimeSec/60, totalTimeSec%60));
		}
#endif
		TheGameLogic->UPDATE();
		if (TheRecorder->sawCRCMismatch())
		{
			numErrors++;
			break;
		}
	}

#ifdef _DEBUG
	{
		UnsignedInt gameTimeSec = TheGameLogic->getFrame() / LOGICFRAMES_PER_SECOND;
		UnsignedInt realTimeSec = (GetTickCount()-startTimeMillis) / 1000;
		DEBUG_LOG(("Elapsed Time: %02d:%02d Game Time: %02d:%02d/%02d:%02d",
				realTimeSec/60, realTimeSec%60, gameTimeSec/60, gameTimeSec%60, totalTimeSec/60, totalTimeSec%60));

		if (TheRecorder->sawCRCMismatch())
			DEBUG_LOG(("CRC Mismatch detected!"));
		else
			DEBUG_LOG(("Replay completed successfully"));
	}
#endif

	return numErrors != 0 ? 1 : 0;
}
