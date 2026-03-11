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

// FILE: GameInfo.h //////////////////////////////////////////////////////
// Generals game setup information
// Author: Matthew D. Campbell, December 2001

#pragma once

#include "Common/Snapshot.h"
#include "Common/Money.h"
#include "GameNetwork/NetworkDefs.h"
#include "GameNetwork/FirewallHelper.h"

enum SlotState CPP_11(: Int)
{
	SLOT_OPEN,
	SLOT_CLOSED,
	SLOT_EASY_AI,
	SLOT_MED_AI,
	SLOT_BRUTAL_AI,
	SLOT_PLAYER
};

enum
{
	PLAYERTEMPLATE_RANDOM = -1,
	PLAYERTEMPLATE_OBSERVER = -2,
	PLAYERTEMPLATE_MIN = PLAYERTEMPLATE_OBSERVER
};

/**
  * GameSlot class - maintains information about the contents of a
	* game slot.  This persists throughout the game.
	*/
class GameSlot
{
public:
	GameSlot();
	virtual void reset();

	void setAccept() { m_isAccepted = true; }		///< Accept the current options
	void unAccept();														///< Unaccept (options changed, etc)
	Bool isAccepted() const { return m_isAccepted; }	///< Non-human slots are always accepted

	void setMapAvailability( Bool hasMap );						///< Set whether the slot has the map
	Bool hasMap() const { return m_hasMap; }		///< Non-human slots always have the map

	void setState( SlotState state,
		UnicodeString name = UnicodeString::TheEmptyString,
		UnsignedInt IP = 0);														///< Set the slot's state (human, AI, open, etc)
	SlotState getState() const { return m_state; }		///< Get the slot state

	void setColor( Int color ) { m_color = color; }
	Int getColor() const { return m_color; }

	void setStartPos( Int startPos ) { m_startPos = startPos; }
	Int getStartPos() const { return m_startPos; }

	void setPlayerTemplate( Int playerTemplate )
	{ m_playerTemplate = playerTemplate;
		if (playerTemplate <= PLAYERTEMPLATE_MIN)
			m_startPos = -1;
	 }
	Int getPlayerTemplate() const { return m_playerTemplate; }

	void setTeamNumber( Int teamNumber ) { m_teamNumber = teamNumber; }
	Int getTeamNumber() const { return m_teamNumber; }

	void setName( UnicodeString name ) { m_name = name; }
	UnicodeString getName() const { return m_name; }

	void setIP( UnsignedInt IP ) { m_IP = IP; }
	UnsignedInt getIP() const { return m_IP; }

	void setPort( UnsignedShort port ) { m_port = port; }
	UnsignedShort getPort() const { return m_port; }

	void setNATBehavior( FirewallHelperClass::FirewallBehaviorType NATBehavior) { m_NATBehavior = NATBehavior; }
	FirewallHelperClass::FirewallBehaviorType getNATBehavior() const { return m_NATBehavior; }

	Bool saveOffOriginalInfo();
	Int getOriginalPlayerTemplate() const	{ return m_origPlayerTemplate; }
	Int getOriginalColor() const						{ return m_origColor; }
	Int getOriginalStartPos() const				{ return m_origStartPos; }
	Int getApparentPlayerTemplate() const;
	Int getApparentColor() const;
	Int getApparentStartPos() const;
	UnicodeString getApparentPlayerTemplateDisplayName() const;

	// Various tests
	Bool isHuman() const;															///< Is this slot occupied by a human player?
	Bool isOccupied() const;													///< Is this slot occupied (by a human or an AI)?
	Bool isAI() const;																///< Is this slot occupied by an AI?
	Bool isPlayer( AsciiString userName ) const;						///< Does this slot contain the given user?
	Bool isPlayer( UnicodeString userName ) const;					///< Does this slot contain the given user?
	Bool isPlayer( UnsignedInt ip ) const;									///< Is this slot at this IP?
	Bool isOpen() const;

	void setLastFrameInGame( UnsignedInt frame ) { m_lastFrameInGame = frame; }
	void markAsDisconnected() { m_disconnected = TRUE; }
	UnsignedInt lastFrameInGame() const { return m_lastFrameInGame; }
	Bool disconnected() const { return isHuman() && m_disconnected; }

	void mute( Bool isMuted ) { m_isMuted = isMuted; }
	Bool isMuted() const { return m_isMuted; }
protected:
	SlotState m_state;
	Bool m_isAccepted;
	Bool m_hasMap;
	Bool m_isMuted;
	Bool m_saveOffOriginalInfo;
	Int m_color;																			///< color, or -1 for random
	Int m_startPos;																		///< start position, or -1 for random
	Int m_playerTemplate;															///< PlayerTemplate
	Int m_teamNumber;																	///< alliance, -1 for none
	Int m_origColor;																			///< color, or -1 for random
	Int m_origStartPos;																		///< start position, or -1 for random
	Int m_origPlayerTemplate;															///< PlayerTemplate
	UnicodeString m_name;															///< Only valid for human players
	UnsignedInt m_IP;																	///< Only valid for human players in LAN/WOL
	UnsignedShort m_port;															///< Only valid for human players in LAN/WOL
	FirewallHelperClass::FirewallBehaviorType m_NATBehavior;	///< The NAT behavior for this slot's player.
	UnsignedInt m_lastFrameInGame;	// only valid for human players
	Bool m_disconnected;						// only valid for human players
};

/**
  * GameInfo class - maintains information about the game setup and
	* the contents of its slot list throughout the game.
	*/
class GameInfo
{
public:
	GameInfo();

	void init();
	virtual void reset();

	void clearSlotList();

	Int getNumPlayers() const;									///< How many players (human and AI) are in the game?
	Int getNumNonObserverPlayers() const;				///< How many non-observer players (human and AI) are in the game?
	Int getMaxPlayers() const;									///< How many players (human and AI) can be in the game?

	void enterGame();														///< Mark us as having entered the game
	void leaveGame();														///< Mark us as having left the game
	virtual void startGame( Int gameID );											///< Mark our game as started, and record the game ID
	void endGame();															///< Mark us as out of game
	inline Int getGameID() const;								///< Get the game ID of the current game or the last one if we're not in game

	inline void setInGame();										///< set the m_inGame flag
	inline Bool isInGame() const;											///< Are we (in game or in game setup)?  As opposed to chatting, matching, etc
	inline Bool isGameInProgress() const;							///< Is the game in progress?
	inline void setGameInProgress( Bool inProgress ); ///< Set whether the game is in progress or not.
	void setSlot( Int slotNum, GameSlot slotInfo );		///< Set the slot state (human, open, AI, etc)
	GameSlot* getSlot( Int slotNum );									///< Get the slot
	const GameSlot* getConstSlot( Int slotNum ) const;	///< Get the slot
	virtual Bool amIHost() const;															///< Convenience function - is the local player the game host?
	virtual Int getLocalSlotNum() const;				///< Get the local slot number, or -1 if we're not present
	Int getSlotNum( AsciiString userName ) const;			///< Get the slot number corresponding to a specific user, or -1 if he's not present

	// Game options
	void setMap( AsciiString mapName );								///< Set the map to play on
	void setMapCRC( UnsignedInt mapCRC );							///< Set the map CRC
	void setMapSize( UnsignedInt mapSize );						///< Set the map size
	void setMapContentsMask( Int mask );							///< Set the map contents mask (1=map,2=preview,4=map.ini)
	inline AsciiString getMap() const;								///< Get the game map
	inline UnsignedInt getMapCRC() const;							///< Get the map CRC
	inline UnsignedInt getMapSize() const;						///< Get the map size
	inline Int getMapContentsMask() const;						///< Get the map contents mask
	void setSeed( Int seed );													///< Set the random seed for the game
	inline Int getSeed() const;												///< Get the game seed
	inline Int getUseStats() const;		///< Does this game count towards gamespy stats?
	inline void setUseStats( Int useStats );

  inline UnsignedShort getSuperweaponRestriction() const; ///< Get any optional limits on superweapons
  void setSuperweaponRestriction( UnsignedShort restriction ); ///< Set the optional limits on superweapons
  inline const Money & getStartingCash() const;
  void setStartingCash( const Money & startingCash );

	void setSlotPointer( Int index, GameSlot *slot );	///< Set the slot info pointer

	void setLocalIP( UnsignedInt ip ) { m_localIP =ip; }	///< Set the local IP
	UnsignedInt getLocalIP() const { return m_localIP; }	///< Get the local IP

	Bool isColorTaken(Int colorIdx, Int slotToIgnore = -1 ) const;
	Bool isStartPositionTaken(Int positionIdx, Int slotToIgnore = -1 ) const;

	virtual void resetAccepted();															///< Reset the accepted flag on all players
	virtual void resetStartSpots();						///< reset the start spots for the new map.
	virtual void adjustSlotsForMap();					///< adjusts the slots to open and closed depending on the players in the game and the number of players the map can hold.

	virtual void closeOpenSlots();						///< close all slots that are currently unoccupied.

	// CRC checking hack
	void setCRCInterval( Int val ) { m_crcInterval = (val<100)?val:100; }
	Int getCRCInterval() const { return m_crcInterval; }

	Bool haveWeSurrendered() { return m_surrendered; }
	void markAsSurrendered() { m_surrendered = TRUE; }

	Bool isSkirmish(); // TRUE if 1 human & 1+ AI are present && !isSandbox()
	Bool isMultiPlayer(); // TRUE if 2+ human are present
	Bool isSandbox(); // TRUE if everybody is on the same team

	Bool isPlayerPreorder(Int index);
	void markPlayerAsPreorder(Int index);

  inline Bool oldFactionsOnly() const;
  inline void setOldFactionsOnly( Bool oldFactionsOnly );

protected:
	Int m_preorderMask;
	Int m_crcInterval;
	Bool m_inGame;
	Bool m_inProgress;
	Bool m_surrendered;
	Int m_gameID;
	GameSlot *m_slot[MAX_SLOTS];

	UnsignedInt m_localIP;

	// Game options
	AsciiString m_mapName;
	UnsignedInt m_mapCRC;
	UnsignedInt m_mapSize;
	Int m_mapMask;
	Int m_seed;
	Int m_useStats;
  Money         m_startingCash;
  UnsignedShort m_superweaponRestriction;
  Bool m_oldFactionsOnly; // Only USA, China, GLA -- not USA Air Force General, GLA Toxic General, et al
};

extern GameInfo *TheGameInfo;

// Inline functions
Int					GameInfo::getGameID() const								{ return m_gameID; }
AsciiString	GameInfo::getMap() const									{ return m_mapName; }
UnsignedInt	GameInfo::getMapCRC() const								{ return m_mapCRC; }
UnsignedInt	GameInfo::getMapSize() const							{ return m_mapSize; }
Int					GameInfo::getMapContentsMask() const			{ return m_mapMask; }
Int					GameInfo::getSeed() const									{ return m_seed; }
Bool				GameInfo::isInGame() const								{ return m_inGame; }
void				GameInfo::setInGame()											{ m_inGame = true; }
Bool				GameInfo::isGameInProgress() const				{ return m_inProgress; }
void				GameInfo::setGameInProgress( Bool inProgress )	{ m_inProgress = inProgress; }
Int					GameInfo::getUseStats() const             { return m_useStats; }
void				GameInfo::setUseStats( Int useStats )           { m_useStats = useStats; }
const Money&GameInfo::getStartingCash() const         { return m_startingCash; }
UnsignedShort GameInfo::getSuperweaponRestriction() const { return m_superweaponRestriction; }
Bool        GameInfo::oldFactionsOnly() const           { return m_oldFactionsOnly; }
void        GameInfo::setOldFactionsOnly( Bool oldFactionsOnly ) { m_oldFactionsOnly = oldFactionsOnly; }

AsciiString GameInfoToAsciiString( const GameInfo *game );
Bool ParseAsciiStringToGameInfo( GameInfo *game, AsciiString options );


/**
  * The SkirmishGameInfo class holds information about the skirmish game and
	* the contents of its slot list.
	*/

class SkirmishGameInfo : public GameInfo, public Snapshot
{
private:
	GameSlot m_skirmishSlot[MAX_SLOTS];

protected:
	// snapshot methods
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();

public:
	SkirmishGameInfo()
	{
		for (Int i = 0; i< MAX_SLOTS; ++i)
			setSlotPointer(i, &m_skirmishSlot[i]);
	}
};

extern SkirmishGameInfo *TheSkirmishGameInfo;
extern SkirmishGameInfo *TheChallengeGameInfo;
