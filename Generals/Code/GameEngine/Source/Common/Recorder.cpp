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

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Recorder.h"
#include "Common/file.h"
#include "Common/FileSystem.h"
#include "Common/PlayerList.h"
#include "Common/Player.h"
#include "Common/GlobalData.h"
#include "Common/GameEngine.h"
#include "GameClient/ClientInstance.h"
#include "GameClient/GameWindow.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/InGameUI.h"
#include "GameClient/Shell.h"
#include "GameClient/GameText.h"

#include "GameNetwork/LANAPICallbacks.h"
#include "GameNetwork/GameMessageParser.h"
#include "GameNetwork/GameSpy/PeerDefs.h"
#include "GameNetwork/networkutil.h"
#include "GameLogic/GameLogic.h"
#include "Common/RandomValue.h"
#include "Common/CRCDebug.h"
#include "Common/UserPreferences.h"
#include "Common/version.h"

constexpr const char s_genrep[] = "GENREP";
constexpr const UnsignedInt replayBufferBytes = 8192;

Int REPLAY_CRC_INTERVAL = 100;

const char *replayExtention = ".rep";
const char *lastReplayFileName = "00000000";	// a name the user is unlikely to ever type, but won't cause panic & confusion

// TheSuperHackers @tweak helmutbuhler 25/04/2025
// The replay header contains two time fields; startTime and endTime of type time_t.
// time_t is 32 bit wide on VC6, but on newer compilers it is 64 bit wide.
// In order to remain compatible we need to load and save time values with 32 bits.
// Note that this will overflow on January 18, 2038. @todo Upgrade to 64 bits when we break compatibility.
typedef int32_t replay_time_t;

static time_t startTime;
static const UnsignedInt startTimeOffset = 6;
static const UnsignedInt endTimeOffset = startTimeOffset + sizeof(replay_time_t);
static const UnsignedInt frameCountOffset = endTimeOffset + sizeof(replay_time_t);
static const UnsignedInt desyncOffset = frameCountOffset + sizeof(UnsignedInt);
static const UnsignedInt quitEarlyOffset = desyncOffset + sizeof(Bool);
static const UnsignedInt disconOffset = quitEarlyOffset + sizeof(Bool);

void RecorderClass::logGameStart(AsciiString options)
{
	if (!m_file)
		return;

	time(&startTime);
	UnsignedInt fileSize = m_file->size();
	// move to appropriate offset
	if ( m_file->seek(startTimeOffset, File::seekMode::START) == startTimeOffset )
	{
		// save off start time
		replay_time_t tmp = (replay_time_t)startTime;
		m_file->write(&tmp, sizeof(tmp));
	}
	// move back to end of stream
#ifdef DEBUG_CRASHING
	Int res =
#endif
	m_file->seek(fileSize, File::seekMode::START);
	DEBUG_ASSERTCRASH(res == fileSize, ("Could not seek to end of file!"));

#if defined(RTS_DEBUG)
	if (TheNetwork && TheGlobalData->m_saveStats)
	{
		//if (TheLAN)
		{
			unsigned long bufSize = MAX_COMPUTERNAME_LENGTH + 1;
			char computerName[MAX_COMPUTERNAME_LENGTH + 1];
			if (!GetComputerName(computerName, &bufSize))
			{
				strcpy(computerName, "unknown");
			}
			AsciiString statsFile = TheGlobalData->m_baseStatsDir;
			TheFileSystem->createDirectory(statsFile);
			statsFile.concat(computerName);
			statsFile.concat(".txt");
			FILE *logFP = fopen(statsFile.str(), "a+");
			if (!logFP)
			{
				// try again locally
				TheWritableGlobalData->m_baseStatsDir = TheGlobalData->getPath_UserData();
				statsFile = TheGlobalData->m_baseStatsDir;
				statsFile.concat(computerName);
				statsFile.concat(".txt");
				logFP = fopen(statsFile.str(), "a+");
			}
			if (logFP)
			{
				struct tm *t2 = localtime(&startTime);
				fprintf(logFP, "\nGame start at %s\tOptions are %s\n", asctime(t2), options.str());
				fclose(logFP);
			}
		}
	}
#endif
}

void RecorderClass::logPlayerDisconnect(UnicodeString player, Int slot)
{
	if (!m_file)
		return;

	DEBUG_ASSERTCRASH((slot >= 0) && (slot < MAX_SLOTS), ("Attempting to disconnect an invalid slot number"));
	if ((slot < 0) || (slot >= (MAX_SLOTS)))
	{
		return;
	}
	UnsignedInt fileSize = m_file->size();
	// move to appropriate offset
	Int playerSlotDisconOffset = disconOffset + slot * sizeof(Bool);
	if ( m_file->seek(playerSlotDisconOffset, File::seekMode::START) == playerSlotDisconOffset )
	{
		// save off discon status
		Bool flag = TRUE;
		m_file->write(&flag, sizeof(flag));
	}
	// move back to end of stream
#ifdef DEBUG_CRASHING
	Int res =
#endif
	m_file->seek(fileSize, File::seekMode::START);
	DEBUG_ASSERTCRASH(res == fileSize, ("Could not seek to end of file!"));

#if defined(RTS_DEBUG)
	if (TheGlobalData->m_saveStats)
	{
		unsigned long bufSize = MAX_COMPUTERNAME_LENGTH + 1;
		char computerName[MAX_COMPUTERNAME_LENGTH + 1];
		if (!GetComputerName(computerName, &bufSize))
		{
			strcpy(computerName, "unknown");
		}
		AsciiString statsFile = TheGlobalData->m_baseStatsDir;
		statsFile.concat(computerName);
		statsFile.concat(".txt");
		FILE *logFP = fopen(statsFile.str(), "a+");
		if (logFP)
		{
			time_t t;
			time(&t);
			struct tm *t2 = localtime(&t);
			fprintf(logFP, "\tPlayer %ls dropped at %s", player.str(), asctime(t2));
			fclose(logFP);
		}
	}
#endif
}

void RecorderClass::logCRCMismatch( void )
{
	if (!m_file)
		return;

	UnsignedInt fileSize = m_file->size();
	// move to appropriate offset
	if ( m_file->seek(desyncOffset, File::seekMode::START) == desyncOffset )
	{
		// save off desync status
		Bool flag = TRUE;
		m_file->write(&flag, sizeof(flag));
	}
	// move back to end of stream
#ifdef DEBUG_CRASHING
	Int res =
#endif
	m_file->seek(fileSize, File::seekMode::START);
	DEBUG_ASSERTCRASH(res == fileSize, ("Could not seek to end of file!"));

#if defined(RTS_DEBUG)
	if (TheGlobalData->m_saveStats)
	{
		m_wasDesync = TRUE;
		unsigned long bufSize = MAX_COMPUTERNAME_LENGTH + 1;
		char computerName[MAX_COMPUTERNAME_LENGTH + 1];
		if (!GetComputerName(computerName, &bufSize))
		{
			strcpy(computerName, "unknown");
		}
		AsciiString statsFile = TheGlobalData->m_baseStatsDir;
		statsFile.concat(computerName);
		statsFile.concat(".txt");
		FILE *logFP = fopen(statsFile.str(), "a+");
		if (logFP)
		{
			time_t t;
			time(&t);
			struct tm *t2 = localtime(&t);
			fprintf(logFP, "\tCRC mismatch at %s", asctime(t2));
			fclose(logFP);
		}
	}
#endif
}

void RecorderClass::logGameEnd( void )
{
	if (!m_file)
		return;

	time_t t;
	time(&t);
	UnsignedInt frameCount = TheGameLogic->getFrame();
	UnsignedInt fileSize = m_file->size();
	// move to appropriate offset
	if ( m_file->seek(endTimeOffset, File::seekMode::START) == endTimeOffset )
	{
		// save off end time
		replay_time_t tmp = (replay_time_t)t;
		m_file->write(&tmp, sizeof(tmp));
	}
	// move to appropriate offset
	if ( m_file->seek(frameCountOffset, File::seekMode::START) == frameCountOffset )
	{
		// save off frameCount
		m_file->write(&frameCount, sizeof(frameCount));
	}
	// move back to end of stream
#ifdef DEBUG_CRASHING
	Int res =
#endif
	m_file->seek(fileSize, File::seekMode::START);
	DEBUG_ASSERTCRASH(res == fileSize, ("Could not seek to end of file!"));

#if defined(RTS_DEBUG)
	if (TheNetwork && TheGlobalData->m_saveStats)
	{
		//if (TheLAN)
		{
			unsigned long bufSize = MAX_COMPUTERNAME_LENGTH + 1;
			char computerName[MAX_COMPUTERNAME_LENGTH + 1];
			if (!GetComputerName(computerName, &bufSize))
			{
				strcpy(computerName, "unknown");
			}
			AsciiString statsFile = TheGlobalData->m_baseStatsDir;
			statsFile.concat(computerName);
			statsFile.concat(".txt");
			FILE *logFP = fopen(statsFile.str(), "a+");
			if (logFP)
			{
				struct tm *t2 = localtime(&t);
				time_t duration = t - startTime;
				Int minutes = duration/60;
				Int seconds = duration%60;
				fprintf(logFP, "Game end at   %s(%d:%2.2d elapsed time)\n", asctime(t2), minutes, seconds);
				fclose(logFP);
			}
		}
	}
#endif
}

void RecorderClass::cleanUpReplayFile( void )
{
#if defined(RTS_DEBUG)
	if (TheGlobalData->m_saveStats)
	{
		char fname[_MAX_PATH+1];
		strlcpy(fname, TheGlobalData->m_baseStatsDir.str(), ARRAY_SIZE(fname));
		strlcat(fname, m_fileName.str(), ARRAY_SIZE(fname));
		DEBUG_LOG(("Saving replay to %s", fname));
		AsciiString oldFname;
		oldFname.format("%s%s", getReplayDir().str(), m_fileName.str());
		CopyFile(oldFname.str(), fname, TRUE);

#ifdef DEBUG_LOGGING
		const char* logFileName = DebugGetLogFileName();
		if (logFileName[0] == '\0')
			return;

		AsciiString debugFname = fname;
		debugFname.truncateBy(3);
		debugFname.concat("txt");
		UnsignedInt fileSize = 0;
		FILE *fp = fopen(logFileName, "rb");
		if (fp)
		{
			fseek(fp, 0, SEEK_END);
			fileSize = ftell(fp);
			fclose(fp);
			fp = NULL;
			DEBUG_LOG(("Log file size was %d", fileSize));
		}

		const int MAX_DEBUG_SIZE = 65536;
		if (fileSize <= MAX_DEBUG_SIZE || TheGlobalData->m_saveAllStats)
		{
			DEBUG_LOG(("Using CopyFile to copy %s", logFileName));
			CopyFile(logFileName, debugFname.str(), TRUE);
		}
		else
		{
			DEBUG_LOG(("manual copy of %s", logFileName));
			FILE *ifp = fopen(logFileName, "rb");
			FILE *ofp = fopen(debugFname.str(), "wb");
			if (ifp && ofp)
			{
				fseek(ifp, fileSize-MAX_DEBUG_SIZE, SEEK_SET);
				char buf[4096];
				Int len;
				while ( (len=fread(buf, 1, 4096, ifp)) > 0 )
				{
					fwrite(buf, 1, len, ofp);
				}
				fclose(ofp);
				fclose(ifp);
				ifp = NULL;
				ofp = NULL;
			}
			else
			{
				if (ifp) fclose(ifp);
				if (ofp) fclose(ofp);
				ifp = NULL;
				ofp = NULL;
			}
		}
#endif // DEBUG_LOGGING
	}
#endif
}

/**
 * The recorder object.
 */
RecorderClass *TheRecorder = NULL;

/**
 * Constructor
 */
RecorderClass::RecorderClass()
{
	m_originalGameMode = GAME_NONE;
	m_mode = RECORDERMODETYPE_RECORD;
	m_file = NULL;
	m_fileName.clear();
	m_currentFilePosition = 0;
	//Added By Sadullah Nader
	//Initializtion(s) inserted
	m_doingAnalysis = FALSE;
	m_archiveReplays = FALSE;
	m_nextFrame = 0;
	m_wasDesync = FALSE;
	//

	init(); // just for the heck of it.
}

/**
 * Destructor
 */
RecorderClass::~RecorderClass() {
}

/**
 * Initialization
 * The recorder will record by default since every game will be recorded.
 * Obviously a game that is being played back will not be recorded.
 * Since the playback is done through a special interface, that interface
 * will set the recorder mode to RECORDERMODETYPE_PLAYBACK.
 */
void RecorderClass::init() {
	m_originalGameMode = GAME_NONE;
	m_mode = RECORDERMODETYPE_NONE;
	m_file = NULL;
	m_fileName.clear();
	m_currentFilePosition = 0;
	m_gameInfo.clearSlotList();
	m_gameInfo.reset();
	if (TheGlobalData->m_pendingFile.isEmpty())
		m_gameInfo.setMap(TheGlobalData->m_mapName);
	else
		m_gameInfo.setMap(TheGlobalData->m_pendingFile);
	m_gameInfo.setSeed(GetGameLogicRandomSeed());
	m_wasDesync = FALSE;
	m_doingAnalysis = FALSE;
	m_playbackFrameCount = 0;

	OptionPreferences optionPref;
	m_archiveReplays = optionPref.getArchiveReplaysEnabled();
}

/**
 * Reset the recorder to the "initialized state."
 */
void RecorderClass::reset() {
	if (m_file != NULL) {
		m_file->close();
		m_file = NULL;
	}
	m_fileName.clear();

	init();
}

/**
 * update
 * Do the update for this frame.
 */
void RecorderClass::update() {
	if (m_mode == RECORDERMODETYPE_RECORD || m_mode == RECORDERMODETYPE_NONE) {
		updateRecord();
	} else if (isPlaybackMode()) {
		updatePlayback();
	}
}

/**
 * Do the update for the next frame of this playback.
 */
void RecorderClass::updatePlayback() {
	// Remove any bad commands that have been inserted by the local user that shouldn't be
	// executed during playback.
	CullBadCommandsResult result = cullBadCommands();

	if (result.hasClearGameDataMessage) {
		// TheSuperHackers @bugfix Stop appending more commands if the replay playback is about to end.
		// Previously this would be able to append more commands, which could have unintended consequences,
		// such as crashing the game when a MSG_PLACE_BEACON is appended after MSG_CLEAR_GAME_DATA.
		// MSG_CLEAR_GAME_DATA is supposed to be processed later this frame, which will then stop this playback.
		return;
	}

	if (m_nextFrame == -1) {
		// This is reached if there are no more commands to be executed.
		return;
	}
	UnsignedInt curFrame = TheGameLogic->getFrame();
	if (m_doingAnalysis)
		curFrame = m_nextFrame;

	// While there are commands to be queued up for this frame, do it.
	while (m_nextFrame == curFrame) {
		appendNextCommand();	// append the next command to TheCommandQueue
		readNextFrame();	// Read the next command's frame number for playback.
	}
}

/**
 * Stop the currently running playback. This is probably due either to the user exiting out of the playback or
 * reaching the end of the playback file.
 */
void RecorderClass::stopPlayback() {
	if (m_file != NULL) {
		m_file->close();
		m_file = NULL;
	}
	m_fileName.clear();

	if (!m_doingAnalysis)
	{
		TheGameLogic->exitGame();
	}
}

/**
 * Update function for recording a game. Basically all the pertinant logic commands for this frame are written out
 * to a file.
 */
void RecorderClass::updateRecord()
{
	Bool needFlush = FALSE;
	static Int lastFrame = -1;
	GameMessage *msg = TheCommandList->getFirstMessage();
	while (msg != NULL) {
		if (msg->getType() == GameMessage::MSG_NEW_GAME &&
			 msg->getArgument(0)->integer != GAME_SHELL &&
			 msg->getArgument(0)->integer != GAME_NONE) {
			m_originalGameMode = msg->getArgument(0)->integer;
			DEBUG_LOG(("RecorderClass::updateRecord() - original game is mode %d", m_originalGameMode));
			lastFrame = 0;
			GameDifficulty diff = DIFFICULTY_NORMAL;
			if (msg->getArgumentCount() >= 2)
				diff = (GameDifficulty)msg->getArgument(1)->integer;
			Int rankPoints = 0;
			if (msg->getArgumentCount() >= 3)
				rankPoints = msg->getArgument(2)->integer;
			Int maxFPS = 0;
			if (msg->getArgumentCount() >= 4)
				maxFPS = msg->getArgument(3)->integer;

			startRecording(diff, m_originalGameMode, rankPoints, maxFPS);
		} else if (msg->getType() == GameMessage::MSG_CLEAR_GAME_DATA) {
			if (m_file != NULL) {
				lastFrame = -1;
				writeToFile(msg);
				stopRecording();
				needFlush = FALSE;
			}
			m_fileName.clear();
		} else {
			if (m_file != NULL) {
				if ((msg->getType() > GameMessage::MSG_BEGIN_NETWORK_MESSAGES) &&
						(msg->getType() < GameMessage::MSG_END_NETWORK_MESSAGES)) {
					// Only write the important messages to the file.
					writeToFile(msg);
					needFlush = TRUE;
				}
			}
		}
		msg = msg->next();
	}

	if (needFlush) {
		DEBUG_ASSERTCRASH(m_file != NULL, ("RecorderClass::updateRecord() - unexpected call to fflush(m_file)"));
		m_file->flush();
	}
}

/**
 * Start a new file for recording. This will always overwrite the "LastReplay.rep" file with the new one.
 * So don't call this unless you really mean it.
 */
void RecorderClass::startRecording(GameDifficulty diff, Int originalGameMode, Int rankPoints, Int maxFPS) {
	DEBUG_ASSERTCRASH(m_file == NULL, ("Starting to record game while game is in progress."));

	reset();

	m_mode = RECORDERMODETYPE_RECORD;

	AsciiString filepath = getReplayDir();

	// We have to make sure the replay dir exists.
	TheFileSystem->createDirectory(filepath);

	m_fileName = getLastReplayFileName();
	m_fileName.concat(getReplayExtention());
	filepath.concat(m_fileName);
	m_file = TheFileSystem->openFile(filepath.str(), File::WRITE | File::BINARY);
	if (m_file == NULL) {
		DEBUG_ASSERTCRASH(m_file != NULL, ("Failed to create replay file"));
		return;
	}
	// TheSuperHackers @info the null terminator needs to be ignored to maintain retail replay file layout
	m_file->writeFormat("%s", s_genrep);

	//
	// save space for stats to be filled in.
	//
	// **** if this changes, change the LAN code above ****
	//
	replay_time_t time = 0;
	m_file->write(&time, sizeof(time));	// reserve space for start time
	m_file->write(&time, sizeof(time));	// reserve space for end time

	UnsignedInt frames = 0;
	m_file->write(&frames, sizeof(frames));	// reserve space for duration in frames

	Bool flag = FALSE;
	m_file->write(&flag, sizeof(flag));	// reserve space for flag (true if we desync)
	m_file->write(&flag, sizeof(flag));	// reserve space for flag (true if we quit early)
	for (Int i=0; i<MAX_SLOTS; ++i)
	{
		m_file->write(&flag, sizeof(flag));	// reserve space for flag (true if player i disconnects)
	}

	// Print out the name of the replay.
	UnicodeString replayName;
	replayName = TheGameText->fetch("GUI:LastReplay");
	m_file->writeFormat(L"%s", replayName.str());
	m_file->writeChar(L"\0");

	// Date and Time
	SYSTEMTIME systemTime;
	GetLocalTime( &systemTime );
	m_file->write(&systemTime, sizeof(systemTime));

	// write out version info
	UnicodeString versionString = TheVersion->getUnicodeVersion();
	UnicodeString versionTimeString = TheVersion->getUnicodeBuildTime();
	UnsignedInt versionNumber = TheVersion->getVersionNumber();
	m_file->writeFormat(L"%s", versionString.str());
	m_file->writeChar(L"\0");
	m_file->writeFormat(L"%s", versionTimeString.str());
	m_file->writeChar(L"\0");
	m_file->write(&versionNumber, sizeof(versionNumber));
	m_file->write(&(TheGlobalData->m_exeCRC), sizeof(TheGlobalData->m_exeCRC));
	m_file->write(&(TheGlobalData->m_iniCRC), sizeof(TheGlobalData->m_iniCRC));

	// Number of players
	/*
	Int numPlayers = ThePlayerList->getPlayerCount();
	fwrite(&numPlayers, sizeof(numPlayers), 1, m_file);
	*/

	// Write the slot list.
	AsciiString theSlotList;
	Int localIndex = -1;
	if (TheNetwork)
	{
		if (TheLAN)
		{
			GameInfo *game = TheLAN->GetMyGame();
			DEBUG_ASSERTCRASH(game, ("Starting a LAN game with no LANGameInfo object!"));
			theSlotList = GameInfoToAsciiString(game);

			for (Int i=0; i<MAX_SLOTS; ++i)
			{
				if (game->getLocalIP() == game->getSlot(i)->getIP())
				{
					localIndex = i;
					break;
				}
			}
		}
		else
		{
			theSlotList = GameInfoToAsciiString(TheGameSpyGame);
			localIndex = TheGameSpyGame->getLocalSlotNum();
		}
	}
	else
	{
    if(TheSkirmishGameInfo)
    {
			TheSkirmishGameInfo->setCRCInterval(REPLAY_CRC_INTERVAL);
      theSlotList = GameInfoToAsciiString(TheSkirmishGameInfo);
      DEBUG_LOG(("GameInfo String: %s",theSlotList.str()));
			localIndex = 0;
    }
    else
    {
		  // single player.  format the generic (empty) slotlist
			m_gameInfo.setCRCInterval(REPLAY_CRC_INTERVAL);
		  theSlotList = GameInfoToAsciiString(&m_gameInfo);
    }
	}
	logGameStart(theSlotList);
	DEBUG_LOG(("RecorderClass::startRecording - theSlotList = %s", theSlotList.str()));

	// write slot list (starting spots, color, alliances, etc
	m_file->writeFormat("%s", theSlotList.str());
	m_file->writeChar("\0");

	m_file->writeFormat("%d", localIndex);
	m_file->writeChar("\0");

	/*
	/// @todo fix this to use starting spots and player alliances when those are put in the game.
	for (Int i = 0; i < numPlayers; ++i) {
		Player *player = ThePlayerList->getNthPlayer(i);
		if (player == NULL) {
			continue;
		}
		UnicodeString name = player->getPlayerDisplayName();
		fwprintf(m_file, L"%s", name.str());
		fputwc(0, m_file);
		UnicodeString faction = player->getFaction()->getFactionDisplayName();
		fwprintf(m_file, L"%s", faction.str());
		fputwc(0, m_file);
		Int color = player->getColor()->getAsInt();
		fwrite(&color, sizeof(color), 1, m_file);
		Int team = 0;
		Int startingSpot = 0;
		fwrite(&startingSpot, sizeof(Int), 1, m_file);
		fwrite(&team, sizeof(Int), 1, m_file);
	}
	*/

	// Write the game difficulty.
	m_file->write(&diff, sizeof(diff));

	// Write original game mode
	m_file->write(&originalGameMode, sizeof(originalGameMode));

	// Write rank points to add at game start
	m_file->write(&rankPoints, sizeof(rankPoints));

	// Write maxFPS chosen
	m_file->write(&maxFPS, sizeof(maxFPS));

	DEBUG_LOG(("RecorderClass::startRecording() - diff=%d, mode=%d, FPS=%d", diff, originalGameMode, maxFPS));

	/*
	// Write the map name.
	fprintf(m_file, "%s", (TheGlobalData->m_mapName).str());
	fputc(0, m_file);
	*/

	/// @todo Need to write game options when there are some to be written.
}

/**
 * This will stop the current recording session and close the file. This should always be called at the end of
 * every game.
 */
void RecorderClass::stopRecording() {
	logGameEnd();
	if (TheNetwork)
	{
		//if (TheLAN)
		{
			if (m_wasDesync)
				cleanUpReplayFile();
			m_wasDesync = FALSE;
		}
	}
	if (m_file != NULL) {
		m_file->close();
		m_file = NULL;

		if (m_archiveReplays)
			archiveReplay(m_fileName);
	}
	m_fileName.clear();
}

/**
 * TheSuperHackers @feature Stubbjax 17/10/2025 Copy the replay file to the archive directory and rename it using the current timestamp.
 */
void RecorderClass::archiveReplay(AsciiString fileName)
{
	SYSTEMTIME st;
	GetLocalTime(&st);

	AsciiString archiveFileName;
	// Use a standard YYYYMMDD_HHMMSS format for simplicity and to avoid conflicts.
	archiveFileName.format("%04d%02d%02d_%02d%02d%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

	AsciiString extension = getReplayExtention();
	AsciiString sourcePath = getReplayDir();
	sourcePath.concat(fileName);

	if (!sourcePath.endsWith(extension))
		sourcePath.concat(extension);

	AsciiString destPath = getReplayArchiveDir();
	TheFileSystem->createDirectory(destPath.str());

	destPath.concat(archiveFileName);
	destPath.concat(extension);

	if (!CopyFile(sourcePath.str(), destPath.str(), FALSE))
		DEBUG_LOG(("RecorderClass::archiveReplay: Failed to copy %s to %s", sourcePath.str(), destPath.str()));
}

/**
 * Write this game message to the record file. This also writes the game message's execution frame.
 */
void RecorderClass::writeToFile(GameMessage * msg) {
	// Write the frame number for this command.
	UnsignedInt frame = TheGameLogic->getFrame();
	m_file->write(&frame, sizeof(frame));

	// Write the command type
	GameMessage::Type type = msg->getType();
	m_file->write(&type, sizeof(type));

	// Write the player index
	Int playerIndex = msg->getPlayerIndex();
	m_file->write(&playerIndex, sizeof(playerIndex));

#ifdef DEBUG_LOGGING
	AsciiString commandName = msg->getCommandAsString();
	if (type < GameMessage::MSG_BEGIN_NETWORK_MESSAGES || type > GameMessage::MSG_END_NETWORK_MESSAGES)
	{
		commandName.concat(" (Non-Network message!)");
	}
	else if (type == GameMessage::MSG_BEGIN_NETWORK_MESSAGES)
	{
		AsciiString tmp;
		tmp.format(" (CRC 0x%8.8X)", msg->getArgument(0)->integer);
		commandName.concat(tmp);
	}

	//DEBUG_LOG(("RecorderClass::writeToFile - Adding %s command from player %d to TheCommandList on frame %d",
		//commandName.str(), msg->getPlayerIndex(), TheGameLogic->getFrame()));
#endif // DEBUG_LOGGING

	GameMessageParser *parser = newInstance(GameMessageParser)(msg);
	UnsignedByte numTypes = parser->getNumTypes();
	m_file->write(&numTypes, sizeof(numTypes));

	GameMessageParserArgumentType *argType = parser->getFirstArgumentType();
	while (argType != NULL) {
		UnsignedByte type = (UnsignedByte)(argType->getType());
		m_file->write(&type, sizeof(type));

		UnsignedByte argTypeCount = (UnsignedByte)(argType->getArgCount());
		m_file->write(&argTypeCount, sizeof(argTypeCount));

		argType = argType->getNext();
	}

//	UnsignedByte lasttype = (UnsignedByte)ARGUMENTDATATYPE_UNKNOWN;
	Int numArgs = msg->getArgumentCount();
	for (Int i = 0; i < numArgs; ++i) {
//		UnsignedByte type = (UnsignedByte)(msg->getArgumentDataType(i));
//		if (lasttype != type) {
//			fwrite(&type, sizeof(type), 1, m_file);
//			lasttype = type;
//		}
		writeArgument(msg->getArgumentDataType(i), *(msg->getArgument(i)));
	}

	deleteInstance(parser);
	parser = NULL;

}

void RecorderClass::writeArgument(GameMessageArgumentDataType type, const GameMessageArgumentType arg) {

	switch (type) {

		case ARGUMENTDATATYPE_INTEGER:
			m_file->write( &(arg.integer), sizeof(arg.integer) );
			break;
		case ARGUMENTDATATYPE_REAL:
			m_file->write( &(arg.real), sizeof(arg.real) );
			break;
		case ARGUMENTDATATYPE_BOOLEAN:
			m_file->write( &(arg.boolean), sizeof(arg.boolean) );
			break;
		case ARGUMENTDATATYPE_OBJECTID:
			m_file->write( &(arg.objectID), sizeof(arg.objectID) );
			break;
		case ARGUMENTDATATYPE_DRAWABLEID:
			m_file->write( &(arg.drawableID), sizeof(arg.drawableID) );
			break;
		case ARGUMENTDATATYPE_TEAMID:
			m_file->write( &(arg.teamID), sizeof(arg.teamID) );
			break;
		case ARGUMENTDATATYPE_LOCATION:
			m_file->write( &(arg.location), sizeof(arg.location) );
			break;
		case ARGUMENTDATATYPE_PIXEL:
			m_file->write( &(arg.pixel), sizeof(arg.pixel) );
			break;
		case ARGUMENTDATATYPE_PIXELREGION:
			m_file->write( &(arg.pixelRegion), sizeof(arg.pixelRegion) );
			break;
		case ARGUMENTDATATYPE_TIMESTAMP:
			m_file->write( &(arg.timestamp), sizeof(arg.timestamp) );
			break;
		case ARGUMENTDATATYPE_WIDECHAR:
			m_file->write( &(arg.wChar), sizeof(arg.wChar) );
			break;
		default:
			DEBUG_LOG(("Unknown GameMessageArgumentDataType in RecorderClass::writeArgument"));
			break;
	}
}

/**
 * Read in a replay header, for (1) populating a replay listbox or (2) starting playback.  In
 * case (2), set FILE *m_file.
 */
Bool RecorderClass::readReplayHeader(ReplayHeader& header)
{
	AsciiString filepath = getReplayDir();
	filepath.concat(header.filename.str());

	// TheSuperHackers @performance More buffered data reduces disk overhead and will improve fast forward playback
	const UnsignedInt buffersize = header.forPlayback ? replayBufferBytes : File::BUFFERSIZE;
	m_file = TheFileSystem->openFile(filepath.str(), File::READ | File::BINARY, buffersize);

	if (m_file == NULL)
	{
		DEBUG_LOG(("Can't open %s (%s)", filepath.str(), header.filename.str()));
		return FALSE;
	}

	// Read the GENREP header.
	char genrep[sizeof(s_genrep) - 1] = {0};
	m_file->read( &genrep, sizeof(s_genrep) - 1 );
	if ( strncmp(genrep, s_genrep, sizeof(s_genrep) - 1 ) ) {
		DEBUG_LOG(("RecorderClass::readReplayHeader - replay file did not have GENREP at the start."));
		m_file->close();
		m_file = NULL;
		return FALSE;
	}

	// read in some stats
	replay_time_t tmp;
	m_file->read(&tmp, sizeof(tmp));
	header.startTime = tmp;
	m_file->read(&tmp, sizeof(tmp));
	header.endTime = tmp;

	m_file->read(&header.frameCount, sizeof(header.frameCount));

	m_file->read(&header.desyncGame, sizeof(header.desyncGame));
	m_file->read(&header.quitEarly, sizeof(header.quitEarly));
	for (Int i=0; i<MAX_SLOTS; ++i)
	{
		m_file->read(&(header.playerDiscons[i]), sizeof(Bool));
	}

	// Read the Replay Name.  We don't actually do anything with it.  Oh well.
	header.replayName = readUnicodeString();

	// Read the date and time.  We don't really do anything with this either. Oh well.
	m_file->read(&header.timeVal, sizeof(header.timeVal));

	// Read in the Version info
	header.versionString = readUnicodeString();
	header.versionTimeString = readUnicodeString();
	m_file->read(&header.versionNumber, sizeof(header.versionNumber));
	m_file->read(&header.exeCRC, sizeof(header.exeCRC));
	m_file->read(&header.iniCRC, sizeof(header.iniCRC));

	// Read in the GameInfo
	header.gameOptions = readAsciiString();
	m_gameInfo.reset();
	m_gameInfo.enterGame();
	DEBUG_LOG(("RecorderClass::readReplayHeader - GameInfo = %s", header.gameOptions.str()));
	if (!ParseAsciiStringToGameInfo(&m_gameInfo, header.gameOptions))
	{
		DEBUG_LOG(("RecorderClass::readReplayHeader - replay file did not have a valid GameInfo string."));
		m_file->close();
		m_file = NULL;
		return FALSE;
	}
	m_gameInfo.startGame(0);

	AsciiString playerIndex = readAsciiString();
	header.localPlayerIndex = atoi(playerIndex.str());
	if (header.localPlayerIndex < -1 || header.localPlayerIndex >= MAX_SLOTS)
	{
		DEBUG_LOG(("RecorderClass::readReplayHeader - invalid local slot number."));
		m_gameInfo.endGame();
		m_gameInfo.reset();
		m_file->close();
		m_file = NULL;
		return FALSE;
	}
	if (header.localPlayerIndex >= 0)
	{
		Int localIP = m_gameInfo.getSlot(header.localPlayerIndex)->getIP();
		m_gameInfo.setLocalIP(localIP);
	}

	if (!header.forPlayback)
	{
		m_gameInfo.endGame();
		m_gameInfo.reset();
		m_file->close();
		m_file = NULL;
	}

	return TRUE;
}

Bool RecorderClass::simulateReplay(AsciiString filename)
{
	Bool success = playbackFile(filename);
	if (success)
		m_mode = RECORDERMODETYPE_SIMULATION_PLAYBACK;
	return success;
}

#if defined(RTS_DEBUG)
Bool RecorderClass::analyzeReplay( AsciiString filename )
{
	m_doingAnalysis = TRUE;
	return playbackFile(filename);
}



#endif

Bool RecorderClass::isPlaybackInProgress( void ) const
{
	return isPlaybackMode() && m_nextFrame != -1;
}

AsciiString RecorderClass::getCurrentReplayFilename( void )
{
	if (isPlaybackMode())
	{
		return m_currentReplayFilename;
	}
	return AsciiString::TheEmptyString;
}

// TheSuperHackers @info helmutbuhler 03/04/2025
// Some info about CRC:
// In each game, each peer periodically calculates a CRC from the local gamestate and sends that
// in a message to all peers (including itself) so that everyone can check that the crc is synchronous.
// In a network game, there is a delay between sending the CRC message and receiving it. This is
// necessary because if you were to wait each frame for all messages from all peers, things would go
// horribly slow.
// But this delay is not a problem for CRC checking because everyone receives the CRC in the same frame
// and every peer just makes sure all the received CRCs are equal.
// While playing replays, this is a problem however: The CRC messages in the replays appear on the frame
// they were received, which can be a few frames delayed if it was a network game. And if we were to
// compare those with the local gamestate, they wouldn't sync up.
// So, in order to fix this, we need to queue up our local CRCs,
// so that we can check it with the crc messages that come later.
// This class is basically that queue.
class CRCInfo
{
public:
	CRCInfo(UnsignedInt localPlayer, Bool isMultiplayer);
	void addCRC(UnsignedInt val);
	UnsignedInt readCRC(void);

	int GetQueueSize() const { return m_data.size(); }

	UnsignedInt getLocalPlayer(void) { return m_localPlayer; }

	void setSawCRCMismatch(void) { m_sawCRCMismatch = TRUE; }
	Bool sawCRCMismatch(void) const { return m_sawCRCMismatch; }

protected:

	Bool m_sawCRCMismatch;
	Bool m_skippedOne;
	std::list<UnsignedInt> m_data;
	UnsignedInt m_localPlayer;
};

CRCInfo::CRCInfo(UnsignedInt localPlayer, Bool isMultiplayer)
{
	m_localPlayer = localPlayer;
	m_skippedOne = !isMultiplayer;
	m_sawCRCMismatch = FALSE;
}

void CRCInfo::addCRC(UnsignedInt val)
{
	// TheSuperHackers @fix helmutbuhler 03/04/2025
	// In Multiplayer, the first MSG_LOGIC_CRC message somehow doesn't make it through the network.
	// Perhaps this happens because the network is not yet set up on frame 0.
	// So we also don't queue up the first local crc message, otherwise the crc
	// messages wouldn't match up anymore and we'd desync immediately during playback.
	if (!m_skippedOne)
	{
		m_skippedOne = TRUE;
		return;
	}

	m_data.push_back(val);
	//DEBUG_LOG(("CRCInfo::addCRC() - crc %8.8X pushes list to %d entries (full=%d)", val, m_data.size(), !m_data.empty()));
}

UnsignedInt CRCInfo::readCRC(void)
{
	if (m_data.empty())
	{
		DEBUG_LOG(("CRCInfo::readCRC() - bailing, full=0, size=%d", m_data.size()));
		return 0;
	}

	UnsignedInt val = m_data.front();
	m_data.pop_front();
	//DEBUG_LOG(("CRCInfo::readCRC() - returning %8.8X, full=%d, size=%d", val, !m_data.empty(), m_data.size()));
	return val;
}

Bool RecorderClass::sawCRCMismatch() const
{
	return m_crcInfo->sawCRCMismatch();
}

void RecorderClass::handleCRCMessage(UnsignedInt newCRC, Int playerIndex, Bool fromPlayback)
{
	if (fromPlayback)
	{
		//DEBUG_LOG(("RecorderClass::handleCRCMessage() - Adding CRC of %X from %d to m_crcInfo", newCRC, playerIndex));
		m_crcInfo->addCRC(newCRC);
		return;
	}

	Int localPlayerIndex = m_crcInfo->getLocalPlayer();
	Bool samePlayer = FALSE;
	AsciiString playerName;
	playerName.format("player%d", localPlayerIndex);
	const Player *p = ThePlayerList->getNthPlayer(playerIndex);
	if (!p || (p->getPlayerNameKey() == NAMEKEY(playerName)))
		samePlayer = TRUE;
	if (samePlayer || (localPlayerIndex < 0))
	{
		UnsignedInt playbackCRC = m_crcInfo->readCRC();
		//DEBUG_LOG(("RecorderClass::handleCRCMessage() - Comparing CRCs of InGame:%8.8X Replay:%8.8X Frame:%d from Player %d",
		//	playbackCRC, newCRC, TheGameLogic->getFrame()-m_crcInfo->GetQueueSize()-1, playerIndex));
		if (TheGameLogic->getFrame() > 0 && newCRC != playbackCRC && !m_crcInfo->sawCRCMismatch())
		{
			// Since we don't seem to have any *visible* desyncs when replaying games, but get this warning
			// virtually every replay, the assumption is our CRC checking is faulty.  Since we're at the
			// tail end of patch season, let's just disable the message, and hope the users believe the
			// problem is fixed. -MDC 3/20/2003
			//
			// TheSuperHackers @tweak helmutbuhler 03/04/2025
			// More than 20 years later, but finally fixed and re-enabled!
			TheInGameUI->message("GUI:CRCMismatch");

			// TheSuperHackers @info helmutbuhler 03/04/2025
			// Note: We subtract the queue size from the frame number. This way we calculate the correct frame
			// the mismatch first happened in case the NetCRCInterval is set to 1 during the game.
			const UnsignedInt mismatchFrame = TheGameLogic->getFrame() - m_crcInfo->GetQueueSize() - 1;

			// Now also prints a UI message for it.
			const UnicodeString mismatchDetailsStr = TheGameText->FETCH_OR_SUBSTITUTE("GUI:CRCMismatchDetails", L"InGame:%8.8X Replay:%8.8X Frame:%d");
			TheInGameUI->message(mismatchDetailsStr, playbackCRC, newCRC, mismatchFrame);

			DEBUG_LOG(("Replay has gone out of sync!\nInGame:%8.8X Replay:%8.8X\nFrame:%d",
				playbackCRC, newCRC, mismatchFrame));

			// Print Mismatch in case we are simulating replays from console.
			printf("CRC Mismatch in Frame %d\n", mismatchFrame);

			// TheSuperHackers @tweak Pause the game on mismatch.
			// But not when a window with focus is opened, because that can make resuming difficult.
			if (TheWindowManager->winGetFocus() == NULL)
			{
				Bool pause = TRUE;
				Bool pauseMusic = FALSE;
				Bool pauseInput = FALSE;
				TheGameLogic->setGamePaused(pause, pauseMusic, pauseInput);

				// Mark this mismatch as seen when we had the chance to pause once.
				m_crcInfo->setSawCRCMismatch();
			}
		}
		return;
	}

	//DEBUG_LOG(("RecorderClass::handleCRCMessage() - Skipping CRC of %8.8X from %d (our index is %d)", newCRC, playerIndex, localPlayerIndex));
}

/**
 * Returns true if this version of the file is the same as our version of the game
 */
Bool RecorderClass::replayMatchesGameVersion(AsciiString filename)
{
	ReplayHeader header;
	header.forPlayback = TRUE;
	header.filename = filename;
	if ( readReplayHeader( header ) )
	{
		return replayMatchesGameVersion( header );
	}
	return FALSE;
}

Bool RecorderClass::replayMatchesGameVersion(const ReplayHeader& header)
{
	// TheSuperHackers @fix No longer checks the build time here to prevent incorrect Replay playback incompatibility messages when the Replay playback would actually be technically compatible.
	if (header.versionString != TheVersion->getUnicodeVersion())
		return false;
	if (header.versionNumber != TheVersion->getVersionNumber())
		return false;
	if (header.exeCRC != TheGlobalData->m_exeCRC)
		return false;
	if (header.iniCRC != TheGlobalData->m_iniCRC)
		return false;
	return true;
}

/**
 * Start playback of the file. Return true or false depending on if the file is
 * a valid replay file or not.
 */
Bool RecorderClass::playbackFile(AsciiString filename)
{
	if (!m_doingAnalysis)
	{
		if (TheGameLogic->isInGame())
		{
			TheGameLogic->clearGameData();
		}
	}

	m_mode = RECORDERMODETYPE_PLAYBACK;

	ReplayHeader header;
	header.forPlayback = TRUE;
	header.filename = filename;
	Bool success = readReplayHeader( header );
	if (!success)
	{
		return FALSE;
	}

#ifdef DEBUG_CRASHING
	Bool versionStringDiff = header.versionString != TheVersion->getUnicodeVersion();
	Bool versionTimeStringDiff = header.versionTimeString != TheVersion->getUnicodeBuildTime();
	Bool versionNumberDiff = header.versionNumber != TheVersion->getVersionNumber();
	Bool exeCRCDiff = header.exeCRC != TheGlobalData->m_exeCRC;
	Bool exeDifferent = versionStringDiff || versionTimeStringDiff || versionNumberDiff || exeCRCDiff;
	Bool iniDifferent = header.iniCRC != TheGlobalData->m_iniCRC;

	AsciiString debugString;
	AsciiString tempStr;
	if (exeDifferent)
	{
		// TheSuperHackers @fix helmutbuhler 05/05/2025 No longer attempts to print unicode as ascii
		// via a call to AsciiString::format with %ls format. It does not work with non-ascii characters.
		UnicodeString tempStrWide;
		debugString = "EXE is different:\n";
		if (versionStringDiff)
		{
			tempStrWide.format(L"   Version [%s] vs [%s]\n", TheVersion->getUnicodeVersion().str(), header.versionString.str());
			tempStr.translate(tempStrWide);
			debugString.concat(tempStr);
		}
		if (versionTimeStringDiff)
		{
			tempStrWide.format(L"   Build Time [%s] vs [%s]\n", TheVersion->getUnicodeBuildTime().str(), header.versionTimeString.str());
			tempStr.translate(tempStrWide);
			debugString.concat(tempStr);
		}
		if (versionNumberDiff)
		{
			tempStr.format("   Version Number %8.8X vs %8.8X\n", TheVersion->getVersionNumber(), header.versionNumber);
			debugString.concat(tempStr);
		}
		if (exeCRCDiff)
		{
			tempStr.format("   CRC %8.8X vs %8.8X\n", TheGlobalData->m_exeCRC, header.exeCRC);
			debugString.concat(tempStr);
		}
	}
	if (iniDifferent)
	{
		debugString.concat("INIs are different:\n");
		tempStr.format("   CRC %8.8X vs %8.8X\n", TheGlobalData->m_iniCRC, header.iniCRC);
		debugString.concat(tempStr);
	}
	DEBUG_ASSERTCRASH(!exeDifferent && !iniDifferent, (debugString.str()));
#endif

	TheWritableGlobalData->m_pendingFile = m_gameInfo.getMap();

#ifdef DEBUG_LOGGING
	if (header.localPlayerIndex >= 0)
	{
		DEBUG_LOG(("Local player is %ls (slot %d, IP %8.8X)",
			m_gameInfo.getSlot(header.localPlayerIndex)->getName().str(), header.localPlayerIndex, m_gameInfo.getSlot(header.localPlayerIndex)->getIP()));
	}
#endif

	Bool isMultiplayer = m_gameInfo.getSlot(header.localPlayerIndex)->getIP() != 0;
	m_crcInfo = NEW CRCInfo(header.localPlayerIndex, isMultiplayer);
	REPLAY_CRC_INTERVAL = m_gameInfo.getCRCInterval();
	DEBUG_LOG(("Player index is %d, replay CRC interval is %d", m_crcInfo->getLocalPlayer(), REPLAY_CRC_INTERVAL));

	Int difficulty = 0;
	m_file->read(&difficulty, sizeof(difficulty));

	m_file->read(&m_originalGameMode, sizeof(m_originalGameMode));

	Int rankPoints = 0;
	m_file->read(&rankPoints, sizeof(rankPoints));

	Int maxFPS = 0;
	m_file->read(&maxFPS, sizeof(maxFPS));

	DEBUG_LOG(("RecorderClass::playbackFile() - original game was mode %d", m_originalGameMode));

	// TheSuperHackers @fix helmutbuhler 03/04/2025
	// In case we restart a replay, we need to clear the command list.
	// Otherwise a crc message remains and messes up the crc calculation on the restarted replay.
	TheCommandList->reset();

	readNextFrame();

	// send a message to the logic for a new game
	if (!m_doingAnalysis)
	{
		// TheSuperHackers @info helmutbuhler 13/04/2025
		// We send the New Game message here directly to the command list and bypass the TheMessageStream.
		// That's ok because Multiplayer is disabled during replay playback and is actually required
		// during replay simulation because we don't update TheMessageStream during simulation.
		GameMessage *msg = newInstance(GameMessage)(GameMessage::MSG_NEW_GAME);
		msg->appendIntegerArgument(GAME_REPLAY);
		msg->appendIntegerArgument(difficulty);
		msg->appendIntegerArgument(rankPoints);
		if( maxFPS != 0 )
			msg->appendIntegerArgument(maxFPS);
		TheCommandList->appendMessage( msg );
		//InitGameLogicRandom( m_gameInfo.getSeed());
		InitRandom( m_gameInfo.getSeed() );
	}

	m_currentReplayFilename = filename;
	m_playbackFrameCount = header.frameCount;
	return TRUE;
}

/**
 * Read a unicode string from the current file position. The string is assumed to be 0-terminated.
 */
UnicodeString RecorderClass::readUnicodeString() {
	WideChar str[1024] = L"";
	Int index = 0;

	Int c = m_file->readWideChar();
	if (c == EOF) {
		str[index] = 0;
	}
	str[index] = c;

	while (index < 1024 && str[index] != 0) {
		++index;
		Int c = m_file->readWideChar();
		if (c == EOF) {
			str[index] = 0;
			break;
		}
		str[index] = c;
	}
	str[1023] = L'\0';

	UnicodeString retval(str);
	return retval;
}

/**
 * Read an ascii string from the current file position. The string is assumed to be 0-terminated.
 */
AsciiString RecorderClass::readAsciiString() {
	char str[1024] = "";
	Int index = 0;

	Int c =	m_file->readChar();
	if (c == EOF) {
		str[index] = 0;
	}
	str[index] = c;

	while (index < 1024 && str[index] != 0) {
		++index;
		Int c = m_file->readChar();
		if (c == EOF) {
			str[index] = 0;
			break;
		}
		str[index] = c;
	}
	str[1023] = '\0';

	AsciiString retval(str);
	return retval;
}

/**
 * Read the frame number for the next command in the playback file. If the end of the file is reached, the playback
 * is stopped and the next frame is said to be -1.
 */
void RecorderClass::readNextFrame() {
	Int bytesRead = m_file->read(&m_nextFrame, sizeof(m_nextFrame));
	if (bytesRead != sizeof(m_nextFrame)) {
		DEBUG_LOG(("RecorderClass::readNextFrame - read failed on frame %d", TheGameLogic->getFrame()));
		m_nextFrame = -1;
		stopPlayback();
	}
}

/**
 * This reads the next command from the replay file and appends it to TheCommandList.
 */
void RecorderClass::appendNextCommand() {
	GameMessage::Type type;
	Int bytesRead = m_file->read(&type, sizeof(type));
	if (bytesRead != sizeof(type)) {
		DEBUG_LOG(("RecorderClass::appendNextCommand - read failed on frame %d", m_nextFrame/*TheGameLogic->getFrame()*/));
		return;
	}

	GameMessage *msg = newInstance(GameMessage)(type);

#ifdef DEBUG_LOGGING
	AsciiString commandName = msg->getCommandAsString();
	if (type < GameMessage::MSG_BEGIN_NETWORK_MESSAGES || type > GameMessage::MSG_END_NETWORK_MESSAGES)
	{
		commandName.concat(" (Non-Network message!)");
	}
	else if (type == GameMessage::MSG_BEGIN_NETWORK_MESSAGES)
	{
		commandName.concat(" (CRC message!)");
	}
#endif // DEBUG_LOGGING

	Int playerIndex = -1;
	m_file->read(&playerIndex, sizeof(playerIndex));
	msg->friend_setPlayerIndex(playerIndex);

	// don't debug log this if we're debugging sync errors, as it will cause diff problems between a game and it's replay...
#ifdef DEBUG_LOGGING
	Bool logCommand = true;
#ifdef DEBUG_CRC
	if (!m_doingAnalysis)
		logCommand = false;
#endif
	if (logCommand)
	{
		DEBUG_LOG(("RecorderClass::appendNextCommand - Adding %s command from player %d to TheCommandList on frame %d",
			commandName.str(), (type == GameMessage::MSG_BEGIN_NETWORK_MESSAGES)?0:msg->getPlayerIndex(), m_nextFrame/*TheGameLogic->getFrame()*/));
	}
#endif

	UnsignedByte numTypes = 0;
	Int totalArgs = 0;
	m_file->read(&numTypes, sizeof(numTypes));

	GameMessageParser *parser = newInstance(GameMessageParser)();
	for (UnsignedByte i = 0; i < numTypes; ++i) {
		UnsignedByte type = (UnsignedByte)ARGUMENTDATATYPE_UNKNOWN;
		m_file->read(&type, sizeof(type));
		UnsignedByte numArgs = 0;
		m_file->read(&numArgs, sizeof(numArgs));
		parser->addArgType((GameMessageArgumentDataType)type, numArgs);
		totalArgs += numArgs;
	}

	GameMessageParserArgumentType *parserArgType = parser->getFirstArgumentType();
	GameMessageArgumentDataType lasttype = ARGUMENTDATATYPE_UNKNOWN;
	Int argsLeftForType = 0;
	if (parserArgType != NULL) {
		lasttype = parserArgType->getType();
		argsLeftForType = parserArgType->getArgCount();
	}
	for (Int j = 0; j < totalArgs; ++j) {
		readArgument(lasttype, msg);

		--argsLeftForType;
		if (argsLeftForType == 0) {
			DEBUG_ASSERTCRASH(parserArgType != NULL, ("parserArgType was NULL when it shouldn't have been."));
			if (parserArgType == NULL) {
				return;
			}

			parserArgType = parserArgType->getNext();
			// parserArgType is allowed to be NULL here, this is the case if there are no more arguments.
			if (parserArgType != NULL) {
				argsLeftForType = parserArgType->getArgCount();
				lasttype = parserArgType->getType();
			}
		}
	}

	if (type != GameMessage::MSG_BEGIN_NETWORK_MESSAGES && type != GameMessage::MSG_CLEAR_GAME_DATA && !m_doingAnalysis)
	{
		TheCommandList->appendMessage(msg);
	}
	else
	{
		deleteInstance(msg);
		msg = NULL;
	}

	deleteInstance(parser);
	parser = NULL;
}

void RecorderClass::readArgument(GameMessageArgumentDataType type, GameMessage *msg) {
	if (type == ARGUMENTDATATYPE_INTEGER) {
		Int theint;
		m_file->read(&theint, sizeof(theint));
		msg->appendIntegerArgument(theint);
#ifdef DEBUG_LOGGING
		if (m_doingAnalysis)
		{
			DEBUG_LOG(("Integer argument: %d (%8.8X)", theint, theint));
		}
#endif
	} else if (type == ARGUMENTDATATYPE_REAL) {
		Real thereal;
		m_file->read(&thereal, sizeof(thereal));
		msg->appendRealArgument(thereal);
#ifdef DEBUG_LOGGING
		if (m_doingAnalysis)
		{
			DEBUG_LOG(("Real argument: %g (%8.8X)", thereal, *(int *)&thereal));
		}
#endif
	} else if (type == ARGUMENTDATATYPE_BOOLEAN) {
		Bool thebool;
		m_file->read(&thebool, sizeof(thebool));
		msg->appendBooleanArgument(thebool);
#ifdef DEBUG_LOGGING
		if (m_doingAnalysis)
		{
			DEBUG_LOG(("Bool argument: %d", thebool));
		}
#endif
	} else if (type == ARGUMENTDATATYPE_OBJECTID) {
		ObjectID theid;
		m_file->read(&theid, sizeof(theid));
		msg->appendObjectIDArgument(theid);
#ifdef DEBUG_LOGGING
		if (m_doingAnalysis)
		{
			DEBUG_LOG(("Object ID argument: %d", theid));
		}
#endif
	} else if (type == ARGUMENTDATATYPE_DRAWABLEID) {
		DrawableID theid;
		m_file->read(&theid, sizeof(theid));
		msg->appendDrawableIDArgument(theid);
#ifdef DEBUG_LOGGING
		if (m_doingAnalysis)
		{
			DEBUG_LOG(("Drawable ID argument: %d", theid));
		}
#endif
	} else if (type == ARGUMENTDATATYPE_TEAMID) {
		UnsignedInt theid;
		m_file->read(&theid, sizeof(theid));
		msg->appendTeamIDArgument(theid);
#ifdef DEBUG_LOGGING
		if (m_doingAnalysis)
		{
			DEBUG_LOG(("Team ID argument: %d", theid));
		}
#endif
	} else if (type == ARGUMENTDATATYPE_LOCATION) {
		Coord3D loc;
		m_file->read(&loc, sizeof(loc));
		msg->appendLocationArgument(loc);
#ifdef DEBUG_LOGGING
		if (m_doingAnalysis)
		{
			DEBUG_LOG(("Coord3D argument: %g %g %g (%8.8X %8.8X %8.8X)", loc.x, loc.y, loc.z,
				*(int *)&loc.x, *(int *)&loc.y, *(int *)&loc.z));
		}
#endif
	} else if (type == ARGUMENTDATATYPE_PIXEL) {
		ICoord2D pixel;
		m_file->read(&pixel, sizeof(pixel));
		msg->appendPixelArgument(pixel);
#ifdef DEBUG_LOGGING
		if (m_doingAnalysis)
		{
			DEBUG_LOG(("Pixel argument: %d,%d", pixel.x, pixel.y));
		}
#endif
	} else if (type == ARGUMENTDATATYPE_PIXELREGION) {
		IRegion2D reg;
		m_file->read(&reg, sizeof(reg));
		msg->appendPixelRegionArgument(reg);
#ifdef DEBUG_LOGGING
		if (m_doingAnalysis)
		{
			DEBUG_LOG(("Pixel Region argument: %d,%d -> %d,%d", reg.lo.x, reg.lo.y, reg.hi.x, reg.hi.y));
		}
#endif
	} else if (type == ARGUMENTDATATYPE_TIMESTAMP) {  // Not to be confused with Terrance Stamp... Kneel before Zod!!!
		UnsignedInt stamp;
		m_file->read(&stamp, sizeof(stamp));
		msg->appendTimestampArgument(stamp);
#ifdef DEBUG_LOGGING
		if (m_doingAnalysis)
		{
			DEBUG_LOG(("Timestamp argument: %d", stamp));
		}
#endif
	} else if (type == ARGUMENTDATATYPE_WIDECHAR) {
		WideChar theid;
		m_file->read(&theid, sizeof(theid));
		msg->appendWideCharArgument(theid);
#ifdef DEBUG_LOGGING
		if (m_doingAnalysis)
		{
			DEBUG_LOG(("WideChar argument: %d (%lc)", theid, theid));
		}
#endif
	}
}

/**
 * This needs to be called for every frame during playback. Basically it prevents the user from inserting.
 */
RecorderClass::CullBadCommandsResult RecorderClass::cullBadCommands() {
	CullBadCommandsResult result;

	if (m_doingAnalysis)
		return result;

	GameMessage *msg = TheCommandList->getFirstMessage();
	GameMessage *next = NULL;

	while (msg != NULL) {
		next = msg->next();
		if ((msg->getType() > GameMessage::MSG_BEGIN_NETWORK_MESSAGES) &&
				(msg->getType() < GameMessage::MSG_END_NETWORK_MESSAGES) &&
				(msg->getType() != GameMessage::MSG_LOGIC_CRC)) {

			deleteInstance(msg);
		}
		else if (msg->getType() == GameMessage::MSG_CLEAR_GAME_DATA)
		{
			result.hasClearGameDataMessage = true;
		}

		msg = next;
	}

	return result;
}

/**
 * returns the directory that holds the replay files.
 */
AsciiString RecorderClass::getReplayDir()
{
	AsciiString tmp = TheGlobalData->getPath_UserData();
	tmp.concat("Replays\\");
	return tmp;
}

/**
 * returns the directory that holds the archived replay files.
 */
AsciiString RecorderClass::getReplayArchiveDir()
{
	AsciiString tmp = TheGlobalData->getPath_UserData();
	tmp.concat("ArchivedReplays\\");
	return tmp;
}

/**
 * returns the file extention for the replay files.
 */
AsciiString RecorderClass::getReplayExtention() {
	return AsciiString(replayExtention);
}

/**
 * returns the file name used for the replay file that is recorded to.
 */
AsciiString RecorderClass::getLastReplayFileName()
{
#if defined(RTS_DEBUG)
	if (TheNetwork && TheGlobalData->m_saveStats)
	{
		GameInfo *game = NULL;
		if (TheLAN)
			game = TheLAN->GetMyGame();
		else if (TheGameSpyInfo)
			game = TheGameSpyGame;
		if (game)
		{
			AsciiString players;
			AsciiString full;
			AsciiString fullPlusNum;
			AsciiString mapName = game->getMap();
			const char *fname = mapName.reverseFind('\\');
			if (fname)
				mapName = fname+1;
			for (Int i=0; i<MAX_SLOTS; ++i)
			{
				GameSlot *slot = game->getSlot(i);
				if (slot && slot->isHuman())
				{
					AsciiString player;
					player.format("%ls_", slot->getName().str());
					players.concat(player);
				}
			}
			full.format("%s%s_%d_%d", players.str(), mapName.str(), game->getSeed(), game->getLocalSlotNum());
			AsciiString testString;
			testString.format("%s%s%s", getReplayDir().str(), full.str(), replayExtention);

			FILE *fp;
			fp = fopen(testString.str(), "rb");
			if (fp)
			{
				fclose(fp);
			}
			else
			{
				return full;
			}
			Int test = 1;
			while (test < 20)
			{
				fullPlusNum.format("%s_%d", full.str(), test);
				testString.format("%s%s%s", getReplayDir().str(), fullPlusNum.str(), replayExtention);
				fp = fopen(testString.str(), "rb");
				if (fp)
				{
					fclose(fp);
					++test;
				}
				else
				{
					return fullPlusNum;
				}
			}
			return fullPlusNum;
		}
	}
#endif

	AsciiString filename;
	if (rts::ClientInstance::getInstanceId() > 1u)
	{
		filename.format("%s_Instance%.2u", lastReplayFileName, rts::ClientInstance::getInstanceId());
	}
	else
	{
		filename = lastReplayFileName;
	}
	return filename;
}

/**
 * return the current operating mode of TheRecorder.
 */
RecorderModeType RecorderClass::getMode() {
	return m_mode;
}

///< Show or Hide the Replay controls
void RecorderClass::initControls()
{
	NameKeyType parentReplayControlID = TheNameKeyGenerator->nameToKey( AsciiString("ReplayControl.wnd:ParentReplayControl") );
	GameWindow *parentReplayControl = TheWindowManager->winGetWindowFromId( NULL, parentReplayControlID );

	Bool show = (getMode() != RECORDERMODETYPE_PLAYBACK);
	if (parentReplayControl)
	{
		parentReplayControl->winHide(show);	// show the replay control window.
	}
}

///< is this a multiplayer game (record OR playback)?
Bool RecorderClass::isMultiplayer( void )
{

	if (isPlaybackMode())
	{
		GameSlot *slot;
		for (int i=0; i<MAX_SLOTS; ++i)
		{
			slot = m_gameInfo.getSlot(i);
			if (slot && slot->isOccupied())	///< slots default to closed for non-networked games
				return true;
		}
	}
	if (TheGameLogic->getGameMode()==GAME_SINGLE_PLAYER) {
		return false; // single player isn't multiplayer.
	}
	if (TheGameLogic->getGameMode()==GAME_SHELL) {
		return false; // shell isn't multiplayer.
	}
	if (TheNetwork || TheSkirmishGameInfo)
		return true;

	return false;
}

/**
 * Create a new recorder object.
 */
RecorderClass * createRecorder() {
	return NEW RecorderClass;
}
