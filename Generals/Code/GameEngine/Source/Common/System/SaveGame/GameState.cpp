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

// FILE: GameState.cpp ////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, September 2002
// Desc:   Game state singleton from which to load and save the game state
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"
#include "Common/file.h"
#include "Common/FileSystem.h"
#include "Common/GameEngine.h"
#include "Common/GameState.h"
#include "Common/GameStateMap.h"
#include "Common/LatchRestore.h"
#include "Common/MapObject.h"
#include "Common/PlayerList.h"
#include "Common/RandomValue.h"
#include "Common/Radar.h"
#include "Common/Team.h"
#include "Common/WellKnownKeys.h"
#include "Common/XferLoad.h"
#include "Common/XferSave.h"
#include "GameClient/CampaignManager.h"
#include "GameClient/GadgetListBox.h"
#include "GameClient/GameClient.h"
#include "GameClient/GameText.h"
#include "GameClient/MapUtil.h"
#include "GameClient/MessageBox.h"
#include "GameClient/InGameUI.h"
#include "GameClient/ParticleSys.h"
#include "GameClient/TerrainVisual.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/GhostObject.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/ScriptEngine.h"
#include "GameLogic/SidesList.h"
#include "GameLogic/TerrainLogic.h"


// PUBLIC DATA ////////////////////////////////////////////////////////////////////////////////////
GameState *TheGameState = NULL;

// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////////
static const Char *SAVE_FILE_EOF       = "SG_EOF";
static const Char *SAVE_GAME_EXTENSION = ".sav";
static const Char *ZERO_NAME_ONLY      = "00000000";
static const Int MAX_SAVE_FILE_NUMBER  =  99999999;

///////////////////////////////////////////////////////////////////////////////////////////////////
#define GAME_STATE_BLOCK_STRING "CHUNK_GameState"  // block of save game data with game info data
#define CAMPAIGN_BLOCK_STRING "CHUNK_Campaign"		 // block of game data that has campaign info

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
SaveGameInfo::SaveGameInfo( void )
{

	date.day					= 0;
	date.dayOfWeek		= 0;
	date.hour					= 0;
	date.milliseconds = 0;
	date.minute				= 0;
	date.month				= 0;
	date.second				= 0;
	date.year					= 0;
	missionNumber			= 0;
	saveFileType			= SAVE_FILE_TYPE_NORMAL;

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
SaveGameInfo::~SaveGameInfo( void )
{

}

// ------------------------------------------------------------------------------------------------
/** Is this date newer than the other one passed in? */
// ------------------------------------------------------------------------------------------------
Bool SaveDate::isNewerThan( SaveDate *other )
{

	// year
	if( year > other->year )
		return TRUE;
	else if( year < other->year )
		return FALSE;
	else
	{

		// month
		if( month > other->month )
			return TRUE;
		else if( month < other->month )
			return FALSE;
		else
		{

			// day
			if( day > other->day )
				return TRUE;
			else if( day < other->day )
				return FALSE;
			else
			{

				// hour
				if( hour > other->hour )
					return TRUE;
				else if( hour < other->hour )
					return FALSE;
				else
				{

					// minute
					if( minute > other->minute )
						return TRUE;
					else if( minute < other->minute )
						return FALSE;
					else
					{

						// second
						if( second > other->second )
							return TRUE;
						else if( second < other->second )
							return FALSE;
						else
						{

							// millisecond
							if( milliseconds > other->milliseconds )
								return TRUE;
							else
								return FALSE;

						}

					}

				}

			}

		}

	}

}

// ------------------------------------------------------------------------------------------------
/** Find a snapshot block info that matches the token passed in */
// ------------------------------------------------------------------------------------------------
GameState::SnapshotBlock *GameState::findBlockInfoByToken( AsciiString token, SnapshotType which )
{

	// sanity
	if( token.isEmpty() )
		return NULL;

	// search for match our list
	SnapshotBlock *blockInfo;
	SnapshotBlockListIterator it;
	for( it = m_snapshotBlockList[which].begin(); it != m_snapshotBlockList[which].end(); ++it )
	{

		// get info
		blockInfo = &(*it);

		// check for match
		if( blockInfo->blockName == token )
			return blockInfo;

	}

	// not found
	return NULL;

}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
UnicodeString getUnicodeDateBuffer(SYSTEMTIME timeVal)
{
	// setup date buffer for local region date format
	#define DATE_BUFFER_SIZE 256
	OSVERSIONINFO	osvi;
	osvi.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
	UnicodeString displayDateBuffer;
	if (GetVersionEx(&osvi))
	{	//check if we're running Win9x variant since they may need different characters
		if (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
		{
			char dateBuffer[ DATE_BUFFER_SIZE ];
			GetDateFormat( LOCALE_SYSTEM_DEFAULT,
										 DATE_SHORTDATE,
										 &timeVal,
										 NULL,
										 dateBuffer, sizeof(dateBuffer) );
			displayDateBuffer.translate(dateBuffer);
			return displayDateBuffer;
		}
	}
	wchar_t dateBuffer[ DATE_BUFFER_SIZE ];
	GetDateFormatW( LOCALE_SYSTEM_DEFAULT,
								 DATE_SHORTDATE,
								 &timeVal,
								 NULL,
								 dateBuffer, ARRAY_SIZE(dateBuffer) );
	displayDateBuffer.set(dateBuffer);
	return displayDateBuffer;
	//displayDateBuffer.format( L"%ls", dateBuffer );
}

UnicodeString getUnicodeTimeBuffer(SYSTEMTIME timeVal)
{
	// setup time buffer for local region time format
	UnicodeString displayTimeBuffer;
	OSVERSIONINFO	osvi;
	osvi.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
	if (GetVersionEx(&osvi))
	{	//check if we're running Win9x variant since they may need different characters
		if (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
		{
			char timeBuffer[ DATE_BUFFER_SIZE ];
			GetTimeFormat( LOCALE_SYSTEM_DEFAULT,
										 TIME_NOSECONDS|TIME_FORCE24HOURFORMAT|TIME_NOTIMEMARKER,
										 &timeVal,
										 NULL,
										 timeBuffer, sizeof(timeBuffer) );
			displayTimeBuffer.translate(timeBuffer);
			return displayTimeBuffer;
		}
	}
	// setup time buffer for local region time format
	#define TIME_BUFFER_SIZE 256
	wchar_t timeBuffer[ TIME_BUFFER_SIZE ];
	GetTimeFormatW( LOCALE_SYSTEM_DEFAULT,
								 TIME_NOSECONDS,
								 &timeVal,
								 NULL,
								 timeBuffer,
								 ARRAY_SIZE(timeBuffer) );
	displayTimeBuffer.set(timeBuffer);
	return displayTimeBuffer;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
GameState::GameState( void )
{

	m_availableGames = NULL;
	m_isInLoadGame = FALSE;

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
GameState::~GameState( void )
{

	// clear our snapshot block list
	for (Int i=0; i<SNAPSHOT_MAX; ++i)
	m_snapshotBlockList[i].clear();

	// make certain that the post process list is clean
	m_snapshotPostProcessList.clear();

	// clear any available game
	clearAvailableGames();

}

// ------------------------------------------------------------------------------------------------
/** Init the game state subsystem */
// ------------------------------------------------------------------------------------------------
void GameState::init( void )
{

	// add all the snapshot objects to our list of data blocks for save game files
	addSnapshotBlock( GAME_STATE_BLOCK_STRING,				TheGameState,							SNAPSHOT_SAVELOAD );
	addSnapshotBlock( CAMPAIGN_BLOCK_STRING,					TheCampaignManager,				SNAPSHOT_SAVELOAD );
	addSnapshotBlock( "CHUNK_GameStateMap",						TheGameStateMap,					SNAPSHOT_SAVELOAD );
	addSnapshotBlock( "CHUNK_TerrainLogic",						TheTerrainLogic,					SNAPSHOT_SAVELOAD );
	addSnapshotBlock( "CHUNK_TeamFactory",						TheTeamFactory,						SNAPSHOT_SAVELOAD );
	addSnapshotBlock( "CHUNK_Players",								ThePlayerList,						SNAPSHOT_SAVELOAD );
	addSnapshotBlock( "CHUNK_GameLogic",							TheGameLogic,							SNAPSHOT_SAVELOAD );
	addSnapshotBlock( "CHUNK_Radar",									TheRadar,									SNAPSHOT_SAVELOAD );
	addSnapshotBlock( "CHUNK_ScriptEngine",						TheScriptEngine,					SNAPSHOT_SAVELOAD );
	addSnapshotBlock( "CHUNK_SidesList",							TheSidesList,							SNAPSHOT_SAVELOAD );
	addSnapshotBlock( "CHUNK_TacticalView",						TheTacticalView,					SNAPSHOT_SAVELOAD );
	addSnapshotBlock( "CHUNK_GameClient",							TheGameClient,						SNAPSHOT_SAVELOAD );
	addSnapshotBlock( "CHUNK_InGameUI",								TheInGameUI,							SNAPSHOT_SAVELOAD );
	addSnapshotBlock( "CHUNK_Partition",							ThePartitionManager,			SNAPSHOT_SAVELOAD );
	addSnapshotBlock( "CHUNK_ParticleSystem",					TheParticleSystemManager,	SNAPSHOT_SAVELOAD );
	addSnapshotBlock( "CHUNK_TerrainVisual",					TheTerrainVisual,					SNAPSHOT_SAVELOAD );
	addSnapshotBlock( "CHUNK_GhostObject",						TheGhostObjectManager,		SNAPSHOT_SAVELOAD );

	// add all the snapshot objects to our list of data blocks for deep CRCs of logic
	addSnapshotBlock( "CHUNK_TeamFactory",						TheTeamFactory,						SNAPSHOT_DEEPCRC_LOGICONLY );
	addSnapshotBlock( "CHUNK_Players",								ThePlayerList,						SNAPSHOT_DEEPCRC_LOGICONLY );
	addSnapshotBlock( "CHUNK_GameLogic",							TheGameLogic,							SNAPSHOT_DEEPCRC_LOGICONLY );
	addSnapshotBlock( "CHUNK_ScriptEngine",						TheScriptEngine,					SNAPSHOT_DEEPCRC_LOGICONLY );
	addSnapshotBlock( "CHUNK_SidesList",							TheSidesList,							SNAPSHOT_DEEPCRC_LOGICONLY );
	addSnapshotBlock( "CHUNK_Partition",							ThePartitionManager,			SNAPSHOT_DEEPCRC_LOGICONLY );

	m_isInLoadGame = FALSE;

}

// ------------------------------------------------------------------------------------------------
/** Reset */
// ------------------------------------------------------------------------------------------------a
void GameState::reset( void )
{

	// clear the post process snapshot list
	m_snapshotPostProcessList.clear();

	// clear any available game
	clearAvailableGames();

	m_isInLoadGame = FALSE;

}

// ------------------------------------------------------------------------------------------------
/** Clear any available games entries */
// ------------------------------------------------------------------------------------------------
void GameState::clearAvailableGames( void )
{
	AvailableGameInfo *gameInfo;

	while( m_availableGames )
	{

		gameInfo = m_availableGames->next;
		delete m_availableGames;
		m_availableGames = gameInfo;

	}

}

// ------------------------------------------------------------------------------------------------
/** Add a snapshot and block name pair to the systems used to load and save */
// ------------------------------------------------------------------------------------------------
void GameState::addSnapshotBlock( AsciiString blockName, Snapshot *snapshot, SnapshotType which )
{

	// sanity
	if( blockName.isEmpty() || snapshot == NULL )
	{

		DEBUG_CRASH(( "addSnapshotBlock: Invalid parameters" ));
		return;

	}

	// add to the list
	SnapshotBlock blockInfo;
	blockInfo.snapshot = snapshot;
	blockInfo.blockName = blockName;
	m_snapshotBlockList[which].push_back( blockInfo );

}

// ------------------------------------------------------------------------------------------------
/** Given the filename of a save file, find the highest filename number */
// ------------------------------------------------------------------------------------------------
static void findHighFileNumber( AsciiString filename, void *userData )
{

	// sanity
	if( filename.isEmpty() )
		return;

	// sanity check for ".sav" at the end of the filename
	if( filename.endsWithNoCase( SAVE_GAME_EXTENSION ) == FALSE )
		return;

	// strip off the extension at the end of the filename
	AsciiString nameOnly = filename;
	nameOnly.truncateBy( strlen( SAVE_GAME_EXTENSION ) );

	// convert filename (which is only numbers) to a number
	Int fileNumber = atoi( nameOnly.str() );

	//
	// atoi will return zero if the string could not be converted, if the filename is
	// not literally "00000000.sav" then that means the conversion could not be done so
	// we should reject this filename from further processing
	//
	if( fileNumber == 0 && nameOnly.compare( ZERO_NAME_ONLY ) != 0 )
		return;

	// compare against the file number we're keeping tally of in the user data parameter
	Int *highFileNumber = (Int *)userData;
	if( fileNumber >= *highFileNumber )
		*highFileNumber = fileNumber;

}

// ------------------------------------------------------------------------------------------------
/** Given the save files on disk, find the "next" filename to use when saving a game */
// ------------------------------------------------------------------------------------------------
AsciiString GameState::findNextSaveFilename( UnicodeString desc )
{
// works, but needs approval from mgmt (srj)
	// GS activating for patch
#define COCKBEER_DONT_USE_NORMAL_FILE_NAMES_THANKS
#ifdef USE_NORMAL_FILE_NAMES_THANKS

	Int i;
	AsciiString adesc;
	for (i = 0; i < desc.getLength(); ++i)
	{
		char c = (char)desc.getCharAt(i);
		if (isalnum(c))
			adesc.concat(c);
		else
			adesc.concat('_');
	}

	for (i = 1; i <= 9999; ++i)
	{
		AsciiString leaf;
		leaf.format("%s_%04d%s", adesc.str(), i, SAVE_GAME_EXTENSION);

		AsciiString path = getFilePathInSaveDirectory(leaf);
		if( _access( path.str(), 0 ) == -1 )
			return leaf;	// note that this returns the leaf, not the full path
	}
#else
	//
	// This method has code to support two modes of finding the next filename, one of
	// them starts with a filename of 00000000.sav and counts up the number looking for
	// a file that doesn't exist and therefore a filename we can use (LOWEST_NUMBER).
	// The other method iterates all save files and returns a filename that is one larger
	// than the largest filename number encountered (HIGHEST_NUMBER)
	//
	enum FindNextFileType { LOWEST_NUMBER = 0, HIGHEST_NUMBER = 1 };
	FindNextFileType searchType = LOWEST_NUMBER;

	if( searchType == HIGHEST_NUMBER )
	{

		// iterate all the save files in the directory and find the highest file number
		Int highFileNumber = -1;
		iterateSaveFiles( findHighFileNumber, &highFileNumber );

		// check for a filename that is too big (this is unlikely but theoretically possible)
		if( highFileNumber + 1 > MAX_SAVE_FILE_NUMBER )
			return AsciiString::TheEmptyString;

		// construct filename with a number higher than the highest one we found
		AsciiString filename;
		filename.format( "%08d%s", highFileNumber + 1, SAVE_GAME_EXTENSION );
		return filename;

	}
	else if( searchType == LOWEST_NUMBER )
	{
		AsciiString filename;
		AsciiString fullPath;
		Int i = 0;

		while( TRUE )
		{

			// construct filename (########.sav)
			filename.format( "%08d%s", i, SAVE_GAME_EXTENSION );

			// construct full path to file given the filename
			fullPath = getFilePathInSaveDirectory(filename);

			// if file does not exist we're all good
			if( _access( fullPath.str(), 0 ) == -1 )
				return filename;

			// test the text filename
			i++;

			// check for at the max limit (this is highly unlikely but possible)
			if( i > MAX_SAVE_FILE_NUMBER )
				return AsciiString::TheEmptyString;

		}

	}
	else
	{

		DEBUG_CRASH(( "GameState::findNextSaveFilename - Unknown file search type '%d'", searchType ));
		return AsciiString::TheEmptyString;

	}
#endif

	// no appropriate filename could be found, return the empty string
	return AsciiString::TheEmptyString;

}

// ------------------------------------------------------------------------------------------------
/** Save the current state of the engine in a save file
	* NOTE: filename is a *filename only* */
// ------------------------------------------------------------------------------------------------
SaveCode GameState::saveGame( AsciiString filename, UnicodeString desc,
															SaveFileType saveType, SnapshotType which )
{

	// if there is no filename, this is a new file being created, find an appropriate filename
	if( filename.isEmpty() )
		filename = findNextSaveFilename( desc );
	if( filename.isEmpty() )
	{

		DEBUG_CRASH(( "GameState::saveGame - Unable to find valid filename for save game" ));
		return SC_NO_FILE_AVAILABLE;

	}

	// make absolutely sure the save directory exists
	CreateDirectory( getSaveDirectory().str(), NULL );

	// construct path to file
	AsciiString filepath = getFilePathInSaveDirectory(filename);

	// save description as current description in the game state
	m_gameInfo.description = desc;

	// open the save file
	XferSave xferSave;
	try {
		xferSave.open( filepath );
	} catch(...) {
		// print error message to the user
		TheInGameUI->message( "GUI:Error" );
		DEBUG_LOG(( "Error opening file '%s'", filepath.str() ));
		return SC_ERROR;
	}

	// save our save file type
	SaveGameInfo *gameInfo = getSaveGameInfo();
	gameInfo->saveFileType = saveType;

	// save our mission map name if applicable
	if( saveType == SAVE_FILE_TYPE_MISSION )
		gameInfo->missionMapName = TheCampaignManager->getCurrentMap();
	else
		gameInfo->missionMapName.clear();

	// set the pristine map to the current campaign map
	// this is now done during startNewGame()
//	gameInfo->pristineMapName = TheCampaignManager->getCurrentMap();

	// write the save file
	try
	{

		// save file
		xferSaveData( &xferSave, which );

	}
	catch( ... )
	{

		UnicodeString ufilepath;
		ufilepath.translate(filepath);

		UnicodeString msg;
		msg.format( TheGameText->fetch("GUI:ErrorSavingGame"), ufilepath.str() );

		MessageBoxOk(TheGameText->fetch("GUI:Error"), msg, NULL);

		// close the file and get out of here
		xferSave.close();
		return SC_ERROR;

	}

	// close the file
	xferSave.close();

	// print message to the user for game successfully saved
	UnicodeString msg = TheGameText->fetch( "GUI:GameSaveComplete" );
	TheInGameUI->message( msg );

	return SC_OK;

}

// ------------------------------------------------------------------------------------------------
/** A mission save */
// ------------------------------------------------------------------------------------------------
SaveCode GameState::missionSave( void )
{

	// get campaign
	Campaign *campaign = TheCampaignManager->getCurrentCampaign();

	// get mission #
	Int missionNumber = TheCampaignManager->getCurrentMissionNumber() + 1;

	// format a string for the mission save description
	UnicodeString format = TheGameText->fetch( "GUI:MissionSave" );
	UnicodeString desc;
	desc.format( format, TheGameText->fetch( campaign->m_campaignNameLabel ).str(), missionNumber );

	// do an automatic mission save
	return TheGameState->saveGame( AsciiString(""), desc, SAVE_FILE_TYPE_MISSION );

}

// ------------------------------------------------------------------------------------------------
/** Load the save game pointed to by filename */
// ------------------------------------------------------------------------------------------------
SaveCode GameState::loadGame( AvailableGameInfo gameInfo )
{

	// sanity check for file
	if( doesSaveGameExist( gameInfo.filename ) == FALSE )
		return SC_FILE_NOT_FOUND;

	// clear game data just like loading from the debug map load screen for mission saves
	if( gameInfo.saveGameInfo.saveFileType == SAVE_FILE_TYPE_MISSION )
	{

		if (TheGameLogic->isInGame())
			TheGameLogic->clearGameData( FALSE );

	}

	//
	// clear the save directory of any temporary "scratch pad" maps that were extracted
	// from any previously loaded save game files
	//
	TheGameStateMap->clearScratchPadMaps();

	// construct path to file
	AsciiString filepath = getFilePathInSaveDirectory(gameInfo.filename);

	// open the save file
	XferLoad xferLoad;
	xferLoad.open( filepath );

	// clear out the game engine
	TheGameEngine->reset();

	// lock creation of new ghost objects
	TheGhostObjectManager->saveLockGhostObjects( TRUE );

	LatchRestore<Bool> inLoadGame(m_isInLoadGame, TRUE);

	// load the save data
	Bool error = FALSE;
	try
	{

		// load file
		xferSaveData( &xferLoad, SNAPSHOT_SAVELOAD );

	}
	catch( ... )
	{
		error = TRUE;
	}

	// close the file
	xferLoad.close();

	// un-savelock the ghost objects
	TheGhostObjectManager->saveLockGhostObjects( FALSE );

	try
	{
		// do the post-process from a save game load
		gameStatePostProcessLoad();
	}
	catch (...)
	{
		error = TRUE;
	}

	// check for error
	if( error == TRUE )
	{
		// clear it out, again
		if (TheGameLogic->isInGame())
			TheGameLogic->clearGameData( FALSE );
		TheGameEngine->reset();

		// print error message to the user
		UnicodeString ufilepath;
		ufilepath.translate(filepath);

		UnicodeString msg;
		msg.format( TheGameText->fetch("GUI:ErrorLoadingGame"), ufilepath.str() );

		MessageBoxOk(TheGameText->fetch("GUI:Error"), msg, NULL);

		return SC_INVALID_DATA;	// you can't use a naked "throw" outside of a catch statement!

	}

	//
	// when loading a mission save, we want to do as much normal loading stuff as we
	// can cause we don't have any real save game data to load other than the
	// game state map info and campaign manager stuff
	//
	if( getSaveGameInfo()->saveFileType == SAVE_FILE_TYPE_MISSION )
	{

		InitRandom(0);

		TheWritableGlobalData->m_pendingFile = getSaveGameInfo()->missionMapName;
		GameMessage *msg = TheMessageStream->appendMessage( GameMessage::MSG_NEW_GAME );
		msg->appendIntegerArgument(GAME_SINGLE_PLAYER);
		msg->appendIntegerArgument(TheCampaignManager->getGameDifficulty());
		msg->appendIntegerArgument(TheCampaignManager->getRankPoints());

		// remove the mission save data, we've got all we need and have started the load
		SaveGameInfo *gameInfo = getSaveGameInfo();
		gameInfo->saveFileType = SAVE_FILE_TYPE_NORMAL;
		gameInfo->missionMapName.clear();

	}

	return SC_OK;

}

//-------------------------------------------------------------------------------------------------
AsciiString GameState::getSaveDirectory() const
{
	AsciiString tmp = TheGlobalData->getPath_UserData();
	tmp.concat("Save\\");
	return tmp;
}

//-------------------------------------------------------------------------------------------------
AsciiString GameState::getFilePathInSaveDirectory(const AsciiString& leaf) const
{
	AsciiString tmp = getSaveDirectory();
	tmp.concat(leaf);
	return tmp;
}

//-------------------------------------------------------------------------------------------------
Bool GameState::isInSaveDirectory(const AsciiString& path) const
{
	return FileSystem::isPathInDirectory(path, getSaveDirectory());
}

// ------------------------------------------------------------------------------------------------
AsciiString GameState::getMapLeafName(const AsciiString& in) const
{
	const char* p = strrchr(in.str(), '\\');
	if (p)
	{
		//
		// p points to the last '\' (if found), however, if a '\' was found there better
		// be another character beyond it, otherwise the map filename would actually
		// be a *directory*  Just move to the first character beyond it so we are looking
		// at the name only
		//
		++p;
		DEBUG_ASSERTCRASH( p != NULL && *p != 0, ("GameState::xfer - Illegal map name encountered") );
		return p;
	}
	else
	{
		return in;
	}
}

// ------------------------------------------------------------------------------------------------
static const char* findLastBackslashInRangeInclusive(const char* start, const char* end)
{
	while (end >= start)
	{
		if (*end == '\\')
			return end;
		--end;
	}
	return NULL;
}

// ------------------------------------------------------------------------------------------------
static AsciiString getMapLeafAndDirName(const AsciiString& in)
{
	const char* start = in.str();
	const char* end = in.str() + in.getLength() - 1;
	const char* p = findLastBackslashInRangeInclusive(start, end);
	if (p)
	{
		const char* p2 = findLastBackslashInRangeInclusive(start, p-1);
		if (p2)
		{
			// we have something like:
			//	maps\foo\foo.map
			//	c:\mydocs\c&cdata\maps\foo\foo.map
			return p2 + 1;
		}
		else
		{
			// we have something like:
			//  save\foo.map
			return in;
		}
	}
	else
	{
		DEBUG_CRASH(("Illegal map-dir-name... should have at least one backslash"));
		return in;
	}
}

// ------------------------------------------------------------------------------------------------
static AsciiString removeExtension(const AsciiString& in)
{
	if (const char* end = in.reverseFind('.'))
	{
		const char* begin = in.str();
		return AsciiString(begin, end - begin);
	}

	return in;
}

// ------------------------------------------------------------------------------------------------
const char* PORTABLE_SAVE				= "Save\\";
const char* PORTABLE_MAPS				= "Maps\\";
const char* PORTABLE_USER_MAPS	= "UserData\\Maps\\";

// ------------------------------------------------------------------------------------------------
AsciiString GameState::realMapPathToPortableMapPath(const AsciiString& in) const
{
	AsciiString prefix;
	if (in.startsWithNoCase(getSaveDirectory()))
	{
		prefix = PORTABLE_SAVE;
		prefix.concat(getMapLeafName(in));
	}
	else if (in.startsWithNoCase(TheMapCache->getMapDir()))
	{
		prefix = PORTABLE_MAPS;
		prefix.concat(getMapLeafAndDirName(in));
	}
	else if (in.startsWithNoCase(TheMapCache->getUserMapDir()))
	{
		prefix = PORTABLE_USER_MAPS;
		prefix.concat(getMapLeafAndDirName(in));
	}
	else
	{
		DEBUG_CRASH(("Map file was not found in any of the expected directories; this is impossible"));
		//throw INI_INVALID_DATA;
		// uncaught exceptions crash us. better to just use a bad path.
		prefix = in;
	}
	prefix.toLower();
	return prefix;
}

// ------------------------------------------------------------------------------------------------
AsciiString GameState::portableMapPathToRealMapPath(const AsciiString& in) const
{
	AsciiString prefix;
	// The directory where the real map path should be contained in.
	AsciiString containingBasePath;
	if (in.startsWithNoCase(PORTABLE_SAVE))
	{
		// the save dir ends with "\\"
		prefix = getSaveDirectory();
		containingBasePath = prefix;
		prefix.concat(getMapLeafName(in));
	}
	else if (in.startsWithNoCase(PORTABLE_MAPS))
	{
		// the map dir DOES NOT end with "\\", must add it
		prefix = TheMapCache->getMapDir();
		prefix.concat("\\");
		containingBasePath = prefix;
		prefix.concat(getMapLeafAndDirName(in));
	}
	else if (in.startsWithNoCase(PORTABLE_USER_MAPS))
	{
		// the map dir DOES NOT end with "\\", must add it
		prefix = TheMapCache->getUserMapDir();
		prefix.concat("\\");
		containingBasePath = prefix;
		prefix.concat(getMapLeafAndDirName(in));
	}
	else
	{
		DEBUG_CRASH(("Map file was not found in any of the expected directories; this is impossible"));
		// Empty string represents a failure, either caused by an invalid prefix or a relative path leading outside the base path
		return AsciiString::TheEmptyString;
	}

	if (!FileSystem::isPathInDirectory(prefix, containingBasePath))
	{
		DEBUG_LOG(("Normalized file path for '%s' was outside the expected base path of '%s'.", prefix.str(), containingBasePath.str()));
		return AsciiString::TheEmptyString;
	}

	prefix.toLower();
	return prefix;
}

// ------------------------------------------------------------------------------------------------
/** Does the save game file exist */
// ------------------------------------------------------------------------------------------------
Bool GameState::doesSaveGameExist( AsciiString filename )
{

	// construct full path to file
	AsciiString filepath = getFilePathInSaveDirectory(filename);

	// open file
	XferLoad xfer;
	try
	{

		// try to open it
		xfer.open( filepath );

	}
	catch( ... )
	{

		// unable to open file, it must not be here
		return FALSE;

	}

	// close the file, we don't want to to anything with it right now
	xfer.close();

	return TRUE;

}

// ------------------------------------------------------------------------------------------------
/** Get save game info from the filename specified */
// ------------------------------------------------------------------------------------------------
void GameState::getSaveGameInfoFromFile( AsciiString filename, SaveGameInfo *saveGameInfo )
{
	AsciiString token;
	Int blockSize;
	Bool done = FALSE;
	SnapshotBlock *blockInfo;

	// sanity
	if( filename.isEmpty() == TRUE || saveGameInfo == NULL )
	{

		DEBUG_CRASH(( "GameState::getSaveGameInfoFromFile - Illegal parameters" ));
		return;

	}

	// open file for partial loading
	XferLoad xferLoad;
	xferLoad.open( filename );

	//
	// disable post processing cause we're not really doing a load of game data that
	// needs post processing and we don't want to keep track of any snapshots we loaded
	//
	xferLoad.setOptions( XO_NO_POST_PROCESSING );

	// read all data blocks in the file
	while( done == FALSE )
	{

		// read next token
		xferLoad.xferAsciiString( &token );

		// check for end of file token
		if( token.compareNoCase( SAVE_FILE_EOF ) == 0 )
		{

			// we should never get here, if we did, we didn't find block of data we needed
			DEBUG_CRASH(( "GameState::getSaveGameInfoFromFile - Game info not found in file '%s'", filename.str() ));
			done = TRUE;

		}
		else
		{

			// find matching token in the save file lexicon
			blockInfo = findBlockInfoByToken( token, SNAPSHOT_SAVELOAD );
			if( blockInfo == NULL )
				throw SC_UNKNOWN_BLOCK;

			// read the data size of this block
			blockSize = xferLoad.beginBlock();

			// is this the block of game info data
			if( stricmp( token.str(), GAME_STATE_BLOCK_STRING ) == 0 )
			{
				GameState tempGameState;

				// parse this data
				try
				{

					// load data
					xferLoad.xferSnapshot( &tempGameState );

				}
				catch( ... )
				{

					DEBUG_CRASH(( "GameState::getSaveGameInfoFromFile - Error loading block '%s' in file '%s'",
												blockInfo->blockName.str(), filename.str() ));
					throw;

				}

				// data was found, copy game state info over
				*saveGameInfo = *tempGameState.getSaveGameInfo();

				// we're all done with this file now
				done = TRUE;

			}
			else
			{

				// not a block we care about, just skip it
				xferLoad.skip( blockSize );

				// end of block
				xferLoad.endBlock();

			}

		}

	}

	// close the file
	xferLoad.close();

}

// ------------------------------------------------------------------------------------------------
/** Create game info and add to available list */
// ------------------------------------------------------------------------------------------------
static void addGameToAvailableList( AsciiString filename, void *userData )
{
	AvailableGameInfo **listHead = (AvailableGameInfo **)userData;

	// sanity
	DEBUG_ASSERTCRASH( listHead != NULL, ("addGameToAvailableList - Illegal parameters") );
	DEBUG_ASSERTCRASH( filename.isEmpty() == FALSE, ("addGameToAvailableList - Illegal filename") );

	try {
	// get header info from this listbox
	SaveGameInfo saveGameInfo;
	TheGameState->getSaveGameInfoFromFile( filename, &saveGameInfo );

	// allocate new info
	AvailableGameInfo *newInfo = new AvailableGameInfo;

	// assign data
	newInfo->prev = NULL;
	newInfo->next = NULL;
	newInfo->saveGameInfo = saveGameInfo;
	newInfo->filename = filename;

	// attach to list
	if( *listHead == NULL )
		*listHead = newInfo;
	else
	{
		AvailableGameInfo *curr, *prev;

		// insert this info so that the most recent games are always at the top of this list
		for( curr = *listHead; curr != NULL; curr = curr->next )
		{

			// save current as previous
			prev = curr;

			// check to see if curr is older than the new info, if so, put new info just ahead of curr
			if( newInfo->saveGameInfo.date.isNewerThan( &curr->saveGameInfo.date ) )
			{

				if( curr->prev )
					curr->prev->next = newInfo;
				else
					*listHead = newInfo;
				newInfo->prev = curr->prev;
				curr->prev = newInfo;
				newInfo->next = curr;

				break;

			}

		}

		// if not inserted, put at end
		if( curr == NULL )
		{

			prev->next = newInfo;
			newInfo->prev = prev;

		}

	}
	} catch(...) {
		// Do nothing - just return.
	}


}

// ------------------------------------------------------------------------------------------------
/** Populate the listbox passed in with a list of the save games present on the hard drive */
// ------------------------------------------------------------------------------------------------
void GameState::populateSaveGameListbox( GameWindow *listbox, SaveLoadLayoutType layoutType )
{
	Int index;

	// sanity
	if( listbox == NULL )
		return;

	// first clear all entries in the listbox
	GadgetListBoxReset( listbox );

	// setup the first entry of the listbox to be a new game when saving is allowed
	if( layoutType != SLLT_LOAD_ONLY )
	{
		UnicodeString newGameText = TheGameText->fetch( "GUI:NewSaveGame" );
		Color newGameColor = GameMakeColor( 200, 200, 255, 255 );

		index = GadgetListBoxAddEntryText( listbox, newGameText, newGameColor, -1 );
		GadgetListBoxSetItemData( listbox, NULL, index );

	}

	// clear the available games
	clearAvailableGames();

	// iterate all the save files in the directory and populate the listbox
	iterateSaveFiles( addGameToAvailableList, &m_availableGames );

	// add all games found to the list box
	AvailableGameInfo *info;
	SaveGameInfo *saveGameInfo;
	SYSTEMTIME systemTime;
	UnsignedInt count = 0;
	for( info = m_availableGames; info; info = info->next, count++ )
	{

		// get save game info
		saveGameInfo = &info->saveGameInfo;

		// setup a system time structure given the data we saved in the file
		systemTime.wYear = saveGameInfo->date.year;
		systemTime.wMonth = saveGameInfo->date.month;
		systemTime.wDayOfWeek = saveGameInfo->date.dayOfWeek;
		systemTime.wDay = saveGameInfo->date.day;
		systemTime.wHour = saveGameInfo->date.hour;
		systemTime.wMinute = saveGameInfo->date.minute;
		systemTime.wSecond = saveGameInfo->date.second;
		systemTime.wMilliseconds = saveGameInfo->date.milliseconds;

		// setup date buffer for local region date format
		UnicodeString displayDateBuffer = getUnicodeDateBuffer(systemTime);

		// setup time buffer for local region time format
		UnicodeString displayTimeBuffer = getUnicodeTimeBuffer(systemTime);

		// description string
		UnicodeString displayLabel = saveGameInfo->description;
		if( displayLabel.isEmpty() == TRUE )
		{
			Bool exists = FALSE;

			displayLabel = TheGameText->fetch( saveGameInfo->mapLabel, &exists );
			if( exists == FALSE )
				displayLabel.format( L"%S", saveGameInfo->mapLabel.str() );

		}

		// pick color for text (we alternate it each game)
		Color color;
		if( saveGameInfo->saveFileType == SAVE_FILE_TYPE_MISSION )
			color = GameMakeColor( 200, 255, 200, 255 );
		else if( count & 0x1 )
			color = GameMakeColor( 255, 128, 0, 255 );
		else
			color = GameMakeColor( 255, 192, 0, 255 );

		// add string to listbox
		index = GadgetListBoxAddEntryText( listbox, displayLabel, color, -1, 0 );
		GadgetListBoxAddEntryText( listbox, displayTimeBuffer, color, index, 1 );
		GadgetListBoxAddEntryText( listbox, displayDateBuffer, color, index, 2 );

		// add this available game info in the user data pointer of that listbox item
		GadgetListBoxSetItemData( listbox, info, index );

	}

	// select the top "new game" entry
	GadgetListBoxSetSelected( listbox, 0 );

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS ////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------------------
/** Iterate the save game files */
// ------------------------------------------------------------------------------------------------
void GameState::iterateSaveFiles( IterateSaveFileCallback callback, void *userData )
{

	// sanity
	if( callback == NULL )
		return;

	// save the current directory
	char currentDirectory[ _MAX_PATH ];
	GetCurrentDirectory( _MAX_PATH, currentDirectory );

	// switch into the save directory
	SetCurrentDirectory( getSaveDirectory().str() );

	// iterate all items in the directory
	WIN32_FIND_DATA item;  // search item
	HANDLE hFile = INVALID_HANDLE_VALUE;  // handle for search resources
	Bool done = FALSE;
	Bool first = TRUE;
	while( done == FALSE )
	{

		// if our first time through we need to start the search
		if( first )
		{

			// start search
			hFile = FindFirstFile( "*", &item );
			if( hFile == INVALID_HANDLE_VALUE )
				return;

			// we are no longer on our first item
			first = FALSE;

		}

		// see if this is a file, and therefore a possible save file
		if( !(item.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
		{

			// see if there is a ".sav" at end of this filename
			Char *c = strrchr( item.cFileName, '.' );
			if( c && stricmp( c, ".sav" ) == 0 )
			{

				// construction asciistring filename
				AsciiString filename;
				filename.set( item.cFileName );

				// call the callback
				callback( filename, userData );

			}

		}

		// on to the next file
		if( FindNextFile( hFile, &item ) == 0 )
			done = TRUE;

	}

	// close search resources
	FindClose( hFile );

	// restore the current directory
	SetCurrentDirectory( currentDirectory );

}

// ------------------------------------------------------------------------------------------------
/** Save game to xfer or load game using xfer */
// ------------------------------------------------------------------------------------------------
void GameState::friend_xferSaveDataForCRC( Xfer *xfer, SnapshotType which )
{
	DEBUG_LOG(("GameState::friend_xferSaveDataForCRC() - SnapshotType %d", which));
	SaveGameInfo *gameInfo = getSaveGameInfo();
	gameInfo->description.clear();
	gameInfo->saveFileType = SAVE_FILE_TYPE_NORMAL;
	gameInfo->missionMapName.clear();
	gameInfo->pristineMapName.clear();

	xferSaveData(xfer, which);
}

// ------------------------------------------------------------------------------------------------
/** Save game to xfer or load game using xfer */
// ------------------------------------------------------------------------------------------------
void GameState::xferSaveData( Xfer *xfer, SnapshotType which )
{

	// sanity
	if( xfer == NULL )
		throw SC_INVALID_XFER;

	// save or load all blocks
	if( xfer->getXferMode() == XFER_SAVE )
	{
		DEBUG_LOG(("GameState::xferSaveData() - XFER_SAVE"));

		// save all blocks
		AsciiString blockName;
		SnapshotBlock *blockInfo;
		SnapshotBlockListIterator it;
		for( it = m_snapshotBlockList[which].begin(); it != m_snapshotBlockList[which].end(); ++it )
		{

			// get list data
			blockInfo = &(*it);

			// get block name
			blockName = blockInfo->blockName;

			DEBUG_LOG(("Looking at block '%s'", blockName.str()));

			//
			// for mission save files, we only save the game state block and campaign manager
			// because anything else is not needed.
			//
			if( getSaveGameInfo()->saveFileType != SAVE_FILE_TYPE_MISSION ||
					(blockName.compareNoCase( GAME_STATE_BLOCK_STRING ) == 0 ||
					 blockName.compareNoCase( CAMPAIGN_BLOCK_STRING ) == 0) )
			{

				// xfer block name
				xfer->xferAsciiString( &blockName );

				// xfer this block
				try
				{

					// begin new data block
					xfer->beginBlock();

					// xfer block data
					xfer->xferSnapshot( blockInfo->snapshot );

					// end this block
					xfer->endBlock();

				}
				catch( ... )
				{

					DEBUG_CRASH(( "Error saving block '%s' in file '%s'",
												blockName.str(), xfer->getIdentifier().str() ));
					throw;

				}

			}

		}

		// write an end of file token
		AsciiString eofToken = SAVE_FILE_EOF;
		xfer->xferAsciiString( &eofToken );

	}
	else
	{
		DEBUG_LOG(("GameState::xferSaveData() - not XFER_SAVE"));
		AsciiString token;
		Int blockSize;
		Bool done = FALSE;
		SnapshotBlock *blockInfo;

		// read all data blocks in the file
		while( done == FALSE )
		{

			// read next token
			xfer->xferAsciiString( &token );

			// check for end of file token
			if( token.compareNoCase( SAVE_FILE_EOF ) == 0 )
			{

				// all done
				done = TRUE;

			}
			else
			{

				// find matching token in the save file lexicon
				blockInfo = findBlockInfoByToken( token, which );
				if( blockInfo == NULL )
				{

					// log the block not found
					DEBUG_LOG(( "GameState::xferSaveData - Skipping unknown block '%s'", token.str() ));

					//
					// block was not found, this could have been a block from an older file
					// format where the block was removed, skip the block data and try to continue
					//
					Int dataSize = xfer->beginBlock();
					xfer->skip( dataSize );

					// continue with while loop reading block tokens
					continue;

				}

				try
				{

					// read block start
					blockSize = xfer->beginBlock();

					// parse this data
					xfer->xferSnapshot( blockInfo->snapshot );

					// read block end
					xfer->endBlock();

				}
				catch( ... )
				{

					DEBUG_CRASH(( "Error loading block '%s' in file '%s'",
												blockInfo->blockName.str(), xfer->getIdentifier().str() ));
					throw;

				}

			}

		}

	}

}

// ------------------------------------------------------------------------------------------------
/** Add a snapshot to the post process list for later */
// ------------------------------------------------------------------------------------------------
void GameState::addPostProcessSnapshot( Snapshot *snapshot )
{

	// sanity
	if( snapshot == NULL )
	{

		DEBUG_CRASH(( "GameState::addPostProcessSnapshot - invalid parameters" ));
		return;

	}

/*
	//
	// This is n^2 and gets real real, REAL slow on game maps.  jba.
	// Please keep this code around tho, it can be useful in debugging save games
	//

	// verify the snapshot isn't in the list already
	SnapshotListIterator it;
	for( it = m_snapshotPostProcessList.begin(); it != m_snapshotPostProcessList.end(); ++it )
	{

		if( (*it) == snapshot )
		{

			DEBUG_CRASH(( "GameState::addPostProcessSnapshot - snapshot is already in list!" ));
			return;

		}

	}
*/

	// add to the list
	m_snapshotPostProcessList.push_back( snapshot );

}

// ------------------------------------------------------------------------------------------------
/** Post process entry point after all game data has been xferd from disk */
// ------------------------------------------------------------------------------------------------
void GameState::gameStatePostProcessLoad( void )
{

	// post process each snapshot that registered with us
	SnapshotListIterator it;
	Snapshot *snapshot;
	for( it = m_snapshotPostProcessList.begin(); it != m_snapshotPostProcessList.end(); /*emtpy*/ )
	{

		// get snapshot
		snapshot = *it;

		// increment iterator
		++it;

		// do processing
		snapshot->loadPostProcess();

	}

	// clear the snapshot post process list as we are now done with it
	m_snapshotPostProcessList.clear();

	// evil... must ensure this is updated prior to the script engine running the first time.
	ThePartitionManager->update();

}

// ------------------------------------------------------------------------------------------------
/** Xfer method for the game state itself
	* Version Info:
	* 1: Initial version
	* 2: Added save file type and mission map name (regular save vs automatic mission save) */
// ------------------------------------------------------------------------------------------------
void GameState::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 2;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// get structure for our current game info
	SaveGameInfo *saveGameInfo = getSaveGameInfo();

	// version 2
	if( version >= 2 )
	{

		// file type
		xfer->xferUser( &saveGameInfo->saveFileType, sizeof( SaveFileType ) );

		// mission map name
		xfer->xferAsciiString( &saveGameInfo->missionMapName );

	}

	// current system time
	SYSTEMTIME systemTime;
	GetLocalTime( &systemTime );

	// date and time
	saveGameInfo->date.year = systemTime.wYear;
	xfer->xferUnsignedShort( &saveGameInfo->date.year );
	saveGameInfo->date.month = systemTime.wMonth;
	xfer->xferUnsignedShort( &saveGameInfo->date.month );
	saveGameInfo->date.day = systemTime.wDay;
	xfer->xferUnsignedShort( &saveGameInfo->date.day );
	saveGameInfo->date.dayOfWeek = systemTime.wDayOfWeek;
	xfer->xferUnsignedShort( &saveGameInfo->date.dayOfWeek );
	saveGameInfo->date.hour = systemTime.wHour;
	xfer->xferUnsignedShort( &saveGameInfo->date.hour );
	saveGameInfo->date.minute = systemTime.wMinute;
	xfer->xferUnsignedShort( &saveGameInfo->date.minute );
	saveGameInfo->date.second = systemTime.wSecond;
	xfer->xferUnsignedShort( &saveGameInfo->date.second );
	saveGameInfo->date.milliseconds = systemTime.wMilliseconds;
	xfer->xferUnsignedShort( &saveGameInfo->date.milliseconds );

	// user description
	xfer->xferUnicodeString( &saveGameInfo->description );

	Bool exists = FALSE;
	Dict *dict = MapObject::getWorldDict();
	if( dict )
		saveGameInfo->mapLabel = dict->getAsciiString( TheKey_mapName, &exists );

	// if no label was found, we'll use the map name (just filename, no directory info)
	if (exists == FALSE || saveGameInfo->mapLabel == AsciiString::TheEmptyString)
	{
		const char* p = TheGlobalData->m_mapName.reverseFind('\\');
		if (p == NULL)
			saveGameInfo->mapLabel = TheGlobalData->m_mapName;
		else
		{
			p++;  // skip the '\' we're on
			saveGameInfo->mapLabel.set(p);
		}
	}

	// xfer map label
	xfer->xferAsciiString( &saveGameInfo->mapLabel );

	// campaign info
	Campaign *campaign = TheCampaignManager->getCurrentCampaign();
	if( campaign )
	{

		// campaign side
		saveGameInfo->campaignSide = campaign->m_name;
		xfer->xferAsciiString( &saveGameInfo->campaignSide );

		// campaign mission number
		saveGameInfo->missionNumber = TheCampaignManager->getCurrentMissionNumber();
		xfer->xferInt( &saveGameInfo->missionNumber );


	}
	else
	{

		// write empty campaign side
		saveGameInfo->campaignSide = AsciiString::TheEmptyString;
		xfer->xferAsciiString( &saveGameInfo->campaignSide );

		// invalid mission number
		saveGameInfo->missionNumber = CampaignManager::INVALID_MISSION_NUMBER;
		xfer->xferInt( &saveGameInfo->missionNumber );

	}

}
