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

// FILE: GameState.h //////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, September 2002
// Desc:   Game state singleton from which to load and save the game state
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/STLTypedefs.h"
#include "Common/Snapshot.h"
#include "Common/SubsystemInterface.h"
#include "Common/UnicodeString.h"
#include "GameNetwork/NetworkDefs.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class GameWindow;
class WindowLayout;

///////////////////////////////////////////////////////////////////////////////////////////////////
typedef void (*IterateSaveFileCallback)( AsciiString filename, void *userData );

// ------------------------------------------------------------------------------------------------
/** The save/load window is used for a variety of formats, using this type during the
	* save/load menu initialization you can make that menu allow loading only, or allow
	* both saving and loading from the same menu */
// ------------------------------------------------------------------------------------------------
enum SaveLoadLayoutType CPP_11(: Int)
{
	SLLT_INVALID = 0,
	SLLT_SAVE_AND_LOAD,
	SLLT_LOAD_ONLY,
	SLLT_SAVE_ONLY,
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
struct SaveDate
{

	SaveDate() { year = month = day = dayOfWeek = hour = minute = second = milliseconds = 0; }
	Bool isNewerThan( SaveDate *other );

	UnsignedShort year;
	UnsignedShort month;
	UnsignedShort day;
	UnsignedShort dayOfWeek;
	UnsignedShort hour;
	UnsignedShort minute;
	UnsignedShort second;
	UnsignedShort milliseconds;

};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
enum SaveFileType CPP_11(: Int)
{
	SAVE_FILE_TYPE_NORMAL,		///< a regular save game at any arbitrary point in the game
	SAVE_FILE_TYPE_MISSION,		///< a save game in between missions (a mission save)
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class SaveGameInfo
{

public:

	SaveGameInfo( void );
	~SaveGameInfo( void );

	AsciiString saveGameMapName;			// map name of the "scratch pad" map extracted from save file
	AsciiString pristineMapName;			// pristine map in the map or user maps directory
	AsciiString mapLabel;							// pretty name of this level set in the editor
	SaveDate date;										// date of file save
	AsciiString campaignSide;					// which campaign side we're playing
	Int missionNumber;								// mission number in campaign
	UnicodeString description;				// user description for save game file
	SaveFileType saveFileType;				// type of save file we're dealing with
	AsciiString missionMapName;				// used for mission saves

};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
struct AvailableGameInfo
{

	AsciiString filename;
	SaveGameInfo saveGameInfo;
	AvailableGameInfo *next;
	AvailableGameInfo *prev;

};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
enum SaveCode CPP_11(: Int)
{
	SC_INVALID = -1,
	SC_OK,
	SC_NO_FILE_AVAILABLE,
	SC_FILE_NOT_FOUND,
	SC_UNABLE_TO_OPEN_FILE,
	SC_INVALID_XFER,
	SC_UNKNOWN_BLOCK,
	SC_INVALID_DATA,
	SC_ERROR,
};

enum SnapshotType CPP_11(: Int) {
	SNAPSHOT_SAVELOAD,
	SNAPSHOT_DEEPCRC_LOGICONLY,
	SNAPSHOT_DEEPCRC,
	SNAPSHOT_MAX
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class GameState : public SubsystemInterface,
									public Snapshot
{

public:

	GameState( void );
	virtual ~GameState( void );

	// subsystem interface
	virtual void init( void );
	virtual void reset( void );
	virtual void update( void ) { }

	// save game methods
	SaveCode saveGame( AsciiString filename,
										 UnicodeString desc,
										 SaveFileType saveType,
										 SnapshotType which = SNAPSHOT_SAVELOAD  );  ///< save a game
	SaveCode missionSave( void );																	 ///< do a in between mission save
	SaveCode loadGame( AvailableGameInfo gameInfo );							 ///< load a save file
	SaveGameInfo *getSaveGameInfo( void ) { return &m_gameInfo; }

	// snapshot interaction
	void addPostProcessSnapshot( Snapshot *snapshot );					///< add snapshot to post process load

	// manipulating files
	Bool doesSaveGameExist( AsciiString filename );							///< does the save file exist
	void populateSaveGameListbox( GameWindow *listbox, SaveLoadLayoutType layoutType );	///< populate listbox with available save games
	void getSaveGameInfoFromFile( AsciiString filename, SaveGameInfo *saveGameInfo );		///< get save game info from file

	void friend_xferSaveDataForCRC( Xfer *xfer, SnapshotType which );		///< This should only be called to DeepCRC sanity checking

	Bool isInLoadGame(void) { return m_isInLoadGame; } // Brutal hack to allow bone pos validation while loading games

	void setPristineMapName( AsciiString name ) { m_gameInfo.pristineMapName = name; }
	AsciiString getPristineMapName( void ) { return m_gameInfo.pristineMapName; }

	AsciiString getSaveDirectory() const;
	AsciiString getFilePathInSaveDirectory(const AsciiString& leaf) const;
	Bool isInSaveDirectory(const AsciiString& path) const;

	AsciiString realMapPathToPortableMapPath(const AsciiString& in) const;
	AsciiString portableMapPathToRealMapPath(const AsciiString& in) const;

	AsciiString getMapLeafName(const AsciiString& in) const;

protected:

	// snapshot methods
	virtual void crc( Xfer *xfer ) { }
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void ) { }

private:

	AsciiString findNextSaveFilename( UnicodeString desc );			///< find next acceptable filename for a new save game
	void iterateSaveFiles( IterateSaveFileCallback callback, void *userData );	///< iterate save files on disk

	void xferSaveData( Xfer *xfer, SnapshotType which );				///< save/load the file data

	void gameStatePostProcessLoad( void );											///< post process entry point after a game load

	void clearAvailableGames( void );		///< clear any available games resources we got in our list

	struct SnapshotBlock
	{
		Snapshot *snapshot;								///< the snapshot object that handles this block
		AsciiString blockName;						///< the block name
	};
	typedef std::list< SnapshotBlock > SnapshotBlockList;
	typedef SnapshotBlockList::iterator SnapshotBlockListIterator;
	void addSnapshotBlock( AsciiString blockName, Snapshot *snapshot, SnapshotType which );
	SnapshotBlock *findBlockInfoByToken( AsciiString token, SnapshotType which );

	SnapshotBlockList m_snapshotBlockList[SNAPSHOT_MAX];	///< list of snapshot blocks of save file data
	SaveGameInfo m_gameInfo;						///< save game info struct

	typedef std::list< Snapshot * > SnapshotList;
	typedef SnapshotList::iterator SnapshotListIterator;
	typedef SnapshotList::reverse_iterator SnapshotListReverseIterator;
	SnapshotList m_snapshotPostProcessList;

	AvailableGameInfo *m_availableGames;		///< list of available games we can save over or load from

	Bool m_isInLoadGame; // Brutal hack to allow bone pos validation while loading games
};

// EXTERNALS //////////////////////////////////////////////////////////////////////////////////////
extern GameState *TheGameState;


UnicodeString getUnicodeTimeBuffer(SYSTEMTIME timeVal);
UnicodeString getUnicodeDateBuffer(SYSTEMTIME timeVal);
