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

// FILE: GameStateMap.cpp /////////////////////////////////////////////////////////////////////////
// Author: Colin Day, October 2002
// Desc:   Chunk in the save game file that will hold a pristine version of the map file
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"

#include "Common/file.h"
#include "Common/FileSystem.h"
#include "Common/GameState.h"
#include "Common/GameStateMap.h"
#include "Common/GlobalData.h"
#include "Common/Xfer.h"
#include "GameClient/CampaignManager.h"
#include "GameClient/GameClient.h"
#include "GameClient/MapUtil.h"
#include "GameLogic/GameLogic.h"
#include "GameNetwork/GameInfo.h"

// GLOBALS ////////////////////////////////////////////////////////////////////////////////////////
GameStateMap *TheGameStateMap = NULL;


// METHODS ////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
GameStateMap::GameStateMap( void )
{

}  // end GameStateMap

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
GameStateMap::~GameStateMap( void )
{

	//
	// clear the save directory of any temporary "scratch pad" maps that were extracted
	// from any previously loaded save game files
	//
	clearScratchPadMaps();

}  // end ~GameStateMap

// ------------------------------------------------------------------------------------------------
/** Embed the pristine map into the xfer stream */
// ------------------------------------------------------------------------------------------------
static void embedPristineMap( AsciiString map, Xfer *xfer )
{
 
	// open the map file
	File *file = TheFileSystem->openFile( map.str(), File::READ | File::BINARY );
	if( file == NULL )
	{

		DEBUG_CRASH(( "embedPristineMap - Error opening source file '%s'\n", map.str() ));
		throw SC_INVALID_DATA;

	}  // end if
 
	// how big is the map file
	Int fileSize = file->seek( 0, File::END );
 
	// rewind to beginning of file
	file->seek( 0, File::START );

	// allocate buffer big enough to hold the entire map file
	char *buffer = new char[ fileSize ];
	if( buffer == NULL )
	{

		DEBUG_CRASH(( "embedPristineMap - Unable to allocate buffer for file '%s'\n", map.str() ));
		throw SC_INVALID_DATA;

	}  // end if
 
	// copy the file to the buffer
	if( file->read( buffer, fileSize ) != fileSize )
	{

		DEBUG_CRASH(( "embeddPristineMap - Error reading from file '%s'\n", map.str() ));
		throw SC_INVALID_DATA;

	}  // end if
 
	// close the BIG file
	file->close();
 
	// write the contents to the save file
	DEBUG_ASSERTCRASH( xfer->getXferMode() == XFER_SAVE, ("embedPristineMap - Unsupposed xfer mode\n") );
	xfer->beginBlock();
	xfer->xferUser( buffer, fileSize );
	xfer->endBlock();

	// delete the buffer
	delete [] buffer;
 
}  // end embedPristineMap

// ------------------------------------------------------------------------------------------------
/** Embed an "in use" map into the xfer stream.  An "in use" map is one that has already
	* been pulled out of a save game file and parked in a temporary file in the save directory */
// ------------------------------------------------------------------------------------------------
static void embedInUseMap( AsciiString map, Xfer *xfer )
{
	FILE *fp = fopen( map.str(), "rb" );

	// sanity
	if( fp == NULL )
	{

		DEBUG_CRASH(( "embedInUseMap - Unable to open file '%s'\n", map.str() ));
		throw SC_INVALID_DATA;

	}  // end if

	// how big is the file
	fseek( fp, 0, SEEK_END );
	Int fileSize = ftell( fp );

	// rewind file back to start
	fseek( fp, 0, SEEK_SET );

	// allocate a buffer big enough for the entire file
	char *buffer = new char[ fileSize ];
	if( buffer == NULL )
	{

		DEBUG_CRASH(( "embedInUseMap - Unable to allocate buffer for file '%s'\n", map.str() ));
		throw SC_INVALID_DATA;

	}  // end if

	// read the entire file
	if( fread( buffer, 1, fileSize, fp ) != fileSize )
	{

		DEBUG_CRASH(( "embedInUseMap - Error reading from file '%s'\n", map.str() ));
		throw SC_INVALID_DATA;

	}  // end if

	// embed file into xfer stream
	xfer->beginBlock();
	xfer->xferUser( buffer, fileSize );
	xfer->endBlock();

	// close the file
	fclose( fp );

	// delete buffer
	delete [] buffer;

}  // embedInUseMap

// ------------------------------------------------------------------------------------------------
/** Extract the map from the xfer stream and save as a file with filename 'mapToSave' */
// ------------------------------------------------------------------------------------------------
static void extractAndSaveMap( AsciiString mapToSave, Xfer *xfer )
{
	UnsignedInt dataSize;

	// open handle to output file
	FILE *fp = fopen( mapToSave.str(), "w+b" );
	if( fp == NULL )
	{

		DEBUG_CRASH(( "extractAndSaveMap - Unable to open file '%s'\n", mapToSave.str() ));
		throw SC_INVALID_DATA;

	}  // en

	// read data size from file
	dataSize = xfer->beginBlock();

	// allocate buffer big enough for the entire map file
	char *buffer = new char[ dataSize ];
	if( buffer == NULL )
	{

		DEBUG_CRASH(( "extractAndSaveMap - Unable to allocate buffer for file '%s'\n", mapToSave.str() ));
		throw SC_INVALID_DATA;

	}  // end if

	// read map file
	xfer->xferUser( buffer, dataSize );

	// write contents of buffer to new file
	if( fwrite( buffer, 1, dataSize, fp ) != dataSize )
	{

		DEBUG_CRASH(( "extractAndSaveMap - Error writing to file '%s'\n", mapToSave.str() ));
		throw SC_INVALID_DATA;

	}  // end if

	// close the new file
	fclose( fp );

	// end of data block
	xfer->endBlock();

	// delete the buffer
	delete [] buffer;

}  // end extractAndSaveMap

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version
	* 2: Now storing the game mode from logic. Storing that here cause TheGameLogic->startNewGame
	*     needs to set up the player list based on it.
	*/
// ------------------------------------------------------------------------------------------------
void GameStateMap::xfer( Xfer *xfer )
{
	if( xfer->getXferMode() == XFER_LOAD )
	{
		TheGameLogic->setLoadingSave( TRUE );
	}

	// version
	const XferVersion currentVersion = 2;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// get save game info
	SaveGameInfo *saveGameInfo = TheGameState->getSaveGameInfo();

	//
	// map filename, for purposes of saving we will always be saving a with a map filename
	// that refers to map in the save directory so we must always save a filename into
	// the file that is in the save directory
	//
	Bool firstSave = FALSE;  // TRUE if we haven't yet saved a pristine load of a new map
	if( xfer->getXferMode() == XFER_SAVE )
	{

		AsciiString mapLeafName = TheGameState->getMapLeafName(TheGlobalData->m_mapName);

		// construct filename to map in the save directory
		saveGameInfo->saveGameMapName = TheGameState->getFilePathInSaveDirectory(mapLeafName);

		// write map name. For cross-machine compatibility, we always write
		// it as just "Save\filename", not a full path.
		{
			AsciiString tmp = TheGameState->realMapPathToPortableMapPath(saveGameInfo->saveGameMapName);
			xfer->xferAsciiString( &tmp );
		}

		//
		// write pristine map name which is already in the member 'pristineMapName' from
		// a previous load, or in the instance where we are saving for the first time
		// and the global data map name refers to a pristine map we will copy it in there first
		//
		if (!TheGameState->isInSaveDirectory(TheGlobalData->m_mapName))
		{

			// copy the pristine name
			saveGameInfo->pristineMapName = TheGlobalData->m_mapName;

			//
			// this is also an indication that we are saving for the first time a brand new
			// map that has never been saved into this save file before (a save is also considered
			// to be a first save as long as we are writing data to disk without having loaded
			// this particluar map from the save file ... so if you load USA01 for the first
			// time and save, that is a first save ... then, without quitting, if you save
			// again that is *also* considered a first save).  First save just determines
			// whether the map file we embed in the save file is taken from the maps directory
			// or from the temporary map extracted to the save directory from a load
			//
			firstSave = TRUE;

		}  // end if

		// save the pristine name
		// For cross-machine compatibility, we always write
		// it as just "Save\filename", not a full path.
		{
			AsciiString tmp = TheGameState->realMapPathToPortableMapPath(saveGameInfo->pristineMapName);
			xfer->xferAsciiString( &tmp );
		}

		if (currentVersion >= 2) 
		{
			// save the game mode.
			Int gameMode = TheGameLogic->getGameMode();
			xfer->xferInt( &gameMode);
		}

	}  // end if, save
	else
	{

		// read the save game map name
		AsciiString tmp;
		xfer->xferAsciiString( &tmp );

		saveGameInfo->saveGameMapName = TheGameState->portableMapPathToRealMapPath(tmp);

		if (!TheGameState->isInSaveDirectory(saveGameInfo->saveGameMapName))
		{
			DEBUG_CRASH(("GameState::xfer - The map filename read from the file '%s' is not in the SAVE directory, but should be\n",
												 saveGameInfo->saveGameMapName.str()) );
			throw SC_INVALID_DATA;
		}

		// set this map as the map to load in the global data
		TheWritableGlobalData->m_mapName = saveGameInfo->saveGameMapName;

		// read the pristine map filename
		xfer->xferAsciiString( &saveGameInfo->pristineMapName );
		saveGameInfo->pristineMapName = TheGameState->portableMapPathToRealMapPath(saveGameInfo->pristineMapName);

		if (currentVersion >= 2) 
		{
			// get the game mode.
			Int gameMode;
			xfer->xferInt(&gameMode);
			TheGameLogic->setGameMode(gameMode);
		}

	}  // end else, load

	// map data
	if( xfer->getXferMode() == XFER_SAVE )
	{

		//
		// if this is a first save from a pristine map load, we need to copy the pristine
		// map into the save game file
		//
		if( firstSave == TRUE )
		{

			embedPristineMap( saveGameInfo->pristineMapName, xfer );

		}  // end if, first save
		else
		{

			//
			// this is *NOT* a first save from a pristine map, just read the map file
			// that was extracted from the save game file during the last load and embedd
			// that into the save game file
			//
			embedInUseMap( saveGameInfo->saveGameMapName, xfer );

		}  // end else

	}  // end if, save
	else
	{

		//
		// take the embedded map file out of the save file, and save as its own .map file
		// in the save directory temporarily
		//
		extractAndSaveMap( saveGameInfo->saveGameMapName, xfer );

	}  // end else

	//
	// it's important that early in the load process, we xfer the object ID counter
	// in the game logic ... this is necessary because there are flows of code that
	// create objects during the load of a save game that is *NOT FROM THE OBJECT BLOCK* of
	// code, such as loading bridges (which creates the bridge and tower objects).  We
	// are fancy and "do the right thing" in those situations, but we don't want to run
	// the risk of these objects being created and having overlapping IDs of anything
	// that we will load from the save file
	//
	ObjectID highObjectID = TheGameLogic->getObjectIDCounter();
	xfer->xferObjectID( &highObjectID );
	TheGameLogic->setObjectIDCounter( highObjectID );

	//
	// it is also equally important to xfer the drawable id counter early in the load process
	// because the act of creating objects also creates drawables, so when it comes time
	// to load the block of drawables we want to make sure that newly created drawables
	// at that time will never overlap IDs with any drawable created as part of an object
	// which then had its ID transferred in from the save file
	//
	DrawableID highDrawableID = TheGameClient->getDrawableIDCounter();
	xfer->xferDrawableID( &highDrawableID );
	TheGameClient->setDrawableIDCounter( highDrawableID );

	// Save the Game Info so the game can be started with the correct players on load
	if( TheGameLogic->getGameMode()==GAME_SKIRMISH ) 
	{
		if( TheSkirmishGameInfo==NULL ) 
		{
			TheSkirmishGameInfo = NEW SkirmishGameInfo;
			TheSkirmishGameInfo->init();  
			TheSkirmishGameInfo->clearSlotList();
			TheSkirmishGameInfo->reset();
		}
		xfer->xferSnapshot(TheSkirmishGameInfo);
	} 
	else 
	{
		if( TheSkirmishGameInfo ) 
		{
			delete TheSkirmishGameInfo;
			TheSkirmishGameInfo = NULL;
		}
	}

	//
	// for loading, start a new game (flagged from a save game) ... this will load the map and all the
	// things in the map file that don't don't change (terrain, triggers, teams, script
	// definitions) etc
	//
	if( xfer->getXferMode() == XFER_LOAD ) 
	{
		TheGameLogic->startNewGame( TRUE );
		TheGameLogic->setLoadingSave( FALSE );
	}

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Delete any scratch pad maps in the save directory.  Scratch pad maps are maps that
	* were embedded in previously loaded save game files and temporarily written out as
	* their own file so that those map files could be loaded as a part of the load game
	* process */
// ------------------------------------------------------------------------------------------------
void GameStateMap::clearScratchPadMaps( void )
{

	// remember the current directory
	char currentDirectory[ _MAX_PATH ];
	GetCurrentDirectory( _MAX_PATH, currentDirectory );

	// switch into the save directory
	SetCurrentDirectory( TheGameState->getSaveDirectory().str() );

	// iterate all items in the directory
	AsciiString fileToDelete;
	WIN32_FIND_DATA item;  // search item
	HANDLE hFile = INVALID_HANDLE_VALUE;  // handle for search resources
	Bool done = FALSE;
	Bool first = TRUE;
	while( done == FALSE )
	{

		// first, clear flag for deleting file
		fileToDelete.clear();

		// if our first time through we need to start the search
		if( first )
		{

			// start search
			hFile = FindFirstFile( "*", &item );
			if( hFile == INVALID_HANDLE_VALUE )
				return;

			// we are no longer on our first item
			first = FALSE;

		}  // end if, first

		// see if this is a file, and therefore a possible .map file
		if( !(item.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
		{

			// see if there is a ".map" at end of this filename
			Char *c = strrchr( item.cFileName, '.' );
			if( c && stricmp( c, ".map" ) == 0 )
				fileToDelete.set( item.cFileName );  // we want to delete this one

		}  // end if

		//
		// find the next file before we delete this one, this is probably not necessary
		// to strcuture things this way so that the find next occurs before the file
		// delete, but it seems more correct to do so
		//
		if( FindNextFile( hFile, &item ) == 0 )
			done = TRUE;

		// delete file if set
		if( fileToDelete.isEmpty() == FALSE )
			DeleteFile( fileToDelete.str() );

	}  // end while

	// close search resources
	FindClose( hFile );

	// restore our directory to the current directory
	SetCurrentDirectory( currentDirectory );

}  // end clearScratchPadMaps
