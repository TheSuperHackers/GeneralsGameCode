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

	void setAccept( void ) { m_isAccepted = true; }		///< Accept the current options
	void unAccept( void );														///< Unaccept (options changed, etc)
	Bool isAccepted( void ) const { return m_isAccepted; }	///< Non-human slots are always accepted

	void setMapAvailability( Bool hasMap );						///< Set whether the slot has the map
	Bool hasMap( void ) const { return m_hasMap; }		///< Non-human slots always have the map

	void setState( SlotState state,
		UnicodeString name = UnicodeString::TheEmptyString,
		UnsignedInt IP = 0);														///< Set the slot's state (human, AI, open, etc)
	SlotState getState( void ) const { return m_state; }		///< Get the slot state

	void setColor( Int color ) { m_color = color; }
	Int getColor( void ) const { return m_color; }

	void setStartPos( Int startPos ) { m_startPos = startPos; }
	Int getStartPos( void ) const { return m_startPos; }

	void setPlayerTemplate( Int playerTemplate )
	{ m_playerTemplate = playerTemplate;
		if (playerTemplate <= PLAYERTEMPLATE_MIN)
			m_startPos = -1;
	 }
	Int getPlayerTemplate( void ) const { return m_playerTemplate; }

	void setTeamNumber( Int teamNumber ) { m_teamNumber = teamNumber; }
	Int getTeamNumber( void ) const { return m_teamNumber; }

	inline void setName( UnicodeString name ) { m_name = name; }
	inline UnicodeString getName( void ) const { return m_name; }

	inline void setIP( UnsignedInt IP ) { m_IP = IP; }
	inline UnsignedInt getIP( void ) const { return m_IP; }

	inline void setPort( UnsignedShort port ) { m_port = port; }
	inline UnsignedShort getPort( void ) const { return m_port; }

	inline void setNATBehavior( FirewallHelperClass::FirewallBehaviorType NATBehavior) { m_NATBehavior = NATBehavior; }
	inline FirewallHelperClass::FirewallBehaviorType getNATBehavior() const { return m_NATBehavior; }

	void saveOffOriginalInfo( void );
	inline Int getOriginalPlayerTemplate( void ) const	{ return m_origPlayerTemplate; }
	inline Int getOriginalColor( void ) const						{ return m_origColor; }
	inline Int getOriginalStartPos( void ) const				{ return m_origStartPos; }
	Int getApparentPlayerTemplate( void ) const;
	Int getApparentColor( void ) const;
	Int getApparentStartPos( void ) const;
	UnicodeString getApparentPlayerTemplateDisplayName( void ) const;

	// Various tests
	Bool isHuman( void ) const;															///< Is this slot occupied by a human player?
	Bool isOccupied( void ) const;													///< Is this slot occupied (by a human or an AI)?
	Bool isAI( void ) const;																///< Is this slot occupied by an AI?
	Bool isPlayer( AsciiString userName ) const;						///< Does this slot contain the given user?
	Bool isPlayer( UnicodeString userName ) const;					///< Does this slot contain the given user?
	Bool isPlayer( UnsignedInt ip ) const;									///< Is this slot at this IP?
	Bool isOpen( void ) const;

	void setLastFrameInGame( UnsignedInt frame ) { m_lastFrameInGame = frame; }
	void markAsDisconnected( void ) { m_disconnected = TRUE; }
	UnsignedInt lastFrameInGame( void ) const { return m_lastFrameInGame; }
	Bool disconnected( void ) const { return isHuman() && m_disconnected; }

	void mute( Bool isMuted ) { m_isMuted = isMuted; }
	Bool isMuted( void ) const { return m_isMuted; }
protected:
	SlotState m_state;
	Bool m_isAccepted;
	Bool m_hasMap;
	Bool m_isMuted;
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
	* the contents of its slot list hroughout the game.
	*/
class GameInfo
{
public:
	GameInfo();

	void init( void );
	virtual void reset( void );

	void clearSlotList( void );

	Int getNumPlayers( void ) const;									///< How many players (human and AI) are in the game?
	Int getNumNonObserverPlayers( void ) const;				///< How many non-observer players (human and AI) are in the game?
	Int getMaxPlayers( void ) const;									///< How many players (human and AI) can be in the game?

	void enterGame( void );														///< Mark us as having entered the game
	void leaveGame( void );														///< Mark us as having left the game
	virtual void startGame( Int gameID );											///< Mark our game as started, and record the game ID
	void endGame( void );															///< Mark us as out of game
	inline Int getGameID( void ) const;								///< Get the game ID of the current game or the last one if we're not in game

	inline void setInGame( void );										///< set the m_inGame flag
	inline Bool isInGame( void ) const;											///< Are we (in game or in game setup)?  As opposed to chatting, matching, etc
	inline Bool isGameInProgress( void ) const;							///< Is the game in progress?
	inline void setGameInProgress( Bool inProgress ); ///< Set whether the game is in progress or not.
	void setSlot( Int slotNum, GameSlot slotInfo );		///< Set the slot state (human, open, AI, etc)
	GameSlot* getSlot( Int slotNum );									///< Get the slot
	const GameSlot* getConstSlot( Int slotNum ) const;	///< Get the slot
	virtual Bool amIHost( void ) const;															///< Convenience function - is the local player the game host?
	virtual Int getLocalSlotNum( void ) const;				///< Get the local slot number, or -1 if we're not present
	Int getSlotNum( AsciiString userName ) const;			///< Get the slot number corresponding to a specific user, or -1 if he's not present

	// Game options
	void setMap( AsciiString mapName );								///< Set the map to play on
	void setMapCRC( UnsignedInt mapCRC );							///< Set the map CRC
	void setMapSize( UnsignedInt mapSize );						///< Set the map size
	void setMapContentsMask( Int mask );							///< Set the map contents mask (1=map,2=preview,4=map.ini)
	inline AsciiString getMap( void ) const;								///< Get the game map
	inline UnsignedInt getMapCRC( void ) const;							///< Get the map CRC
	inline UnsignedInt getMapSize( void ) const;						///< Get the map size
	inline Int getMapContentsMask( void ) const;						///< Get the map contents mask
	void setSeed( Int seed );													///< Set the random seed for the game
	inline Int getSeed( void ) const;												///< Get the game seed
	inline Int getUseStats( void ) const;		///< Does this game count towards gamespy stats?
	inline void setUseStats( Int useStats );

  inline UnsignedShort getSuperweaponRestriction( void ) const; ///< Get any optional limits on superweapons
  void setSuperweaponRestriction( UnsignedShort restriction ); ///< Set the optional limits on superweapons
  inline const Money & getStartingCash(void) const;
  void setStartingCash( const Money & startingCash );

	void setSlotPointer( Int index, GameSlot *slot );	///< Set the slot info pointer

	void setLocalIP( UnsignedInt ip ) { m_localIP =ip; }	///< Set the local IP
	UnsignedInt getLocalIP( void ) const { return m_localIP; }	///< Get the local IP

	Bool isColorTaken(Int colorIdx, Int slotToIgnore = -1 ) const;
	Bool isStartPositionTaken(Int positionIdx, Int slotToIgnore = -1 ) const;

	virtual void resetAccepted(void);															///< Reset the accepted flag on all players
	virtual void resetStartSpots(void);						///< reset the start spots for the new map.
	virtual void adjustSlotsForMap(void);					///< adjusts the slots to open and closed depending on the players in the game and the number of players the map can hold.

	virtual void closeOpenSlots(void);						///< close all slots that are currently unoccupied.

	// CRC checking hack
	void setCRCInterval( Int val ) { m_crcInterval = (val<100)?val:100; }
	inline Int getCRCInterval( void ) const { return m_crcInterval; }

	Bool haveWeSurrendered(void) { return m_surrendered; }
	void markAsSurrendered(void) { m_surrendered = TRUE; }

	Bool isSkirmish(void); // TRUE if 1 human & 1+ AI are present && !isSandbox()
	Bool isMultiPlayer(void); // TRUE if 2+ human are present
	Bool isSandbox(void); // TRUE if everybody is on the same team

	Bool isPlayerPreorder(Int index);
	void markPlayerAsPreorder(Int index);

  inline Bool oldFactionsOnly(void) const;
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
Int					GameInfo::getGameID( void ) const								{ return m_gameID; }
AsciiString	GameInfo::getMap( void ) const									{ return m_mapName; }
UnsignedInt	GameInfo::getMapCRC( void ) const								{ return m_mapCRC; }
UnsignedInt	GameInfo::getMapSize( void ) const							{ return m_mapSize; }
Int					GameInfo::getMapContentsMask( void ) const			{ return m_mapMask; }
Int					GameInfo::getSeed( void ) const									{ return m_seed; }
Bool				GameInfo::isInGame( void ) const								{ return m_inGame; }
void				GameInfo::setInGame( void )											{ m_inGame = true; }
Bool				GameInfo::isGameInProgress( void ) const				{ return m_inProgress; }
void				GameInfo::setGameInProgress( Bool inProgress )	{ m_inProgress = inProgress; }
Int					GameInfo::getUseStats( void ) const             { return m_useStats; }
void				GameInfo::setUseStats( Int useStats )           { m_useStats = useStats; }
const Money&GameInfo::getStartingCash( void ) const         { return m_startingCash; }
UnsignedShort GameInfo::getSuperweaponRestriction( void ) const { return m_superweaponRestriction; }
Bool        GameInfo::oldFactionsOnly(void) const           { return m_oldFactionsOnly; }
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
	virtual void loadPostProcess( void );

public:
	SkirmishGameInfo()
	{
		for (Int i = 0; i< MAX_SLOTS; ++i)
			setSlotPointer(i, &m_skirmishSlot[i]);
	}
};

extern SkirmishGameInfo *TheSkirmishGameInfo;
extern SkirmishGameInfo *TheChallengeGameInfo;
