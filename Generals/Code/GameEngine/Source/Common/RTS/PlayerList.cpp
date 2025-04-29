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

// FILE: PlayerList.cpp /////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//                                                                          
//                       Westwood Studios Pacific.                          
//                                                                          
//                       Confidential Information                           
//                Copyright (C) 2001 - All Rights Reserved                  
//                                                                          
//-----------------------------------------------------------------------------
//
// Project:   RTS3
//
// File name: PlayerList.cpp
//
// Created:   Steven Johnson, October 2001
//
// Desc:      @todo
//
//-----------------------------------------------------------------------------

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/Errors.h"
#include "Common/DataChunk.h"
#include "Common/GameState.h"
#include "Common/GlobalData.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/PlayerTemplate.h"
#include "Common/Team.h"
#include "Common/WellKnownKeys.h"
#include "Common/Xfer.h"
#ifdef RTS_DEBUG
#include "GameLogic/Object.h"
#endif
#include "GameLogic/SidesList.h"
#include "GameNetwork/NetworkDefs.h"

#ifdef _INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

//-----------------------------------------------------------------------------
/*extern*/ PlayerList *ThePlayerList = NULL;

//-----------------------------------------------------------------------------
PlayerList::PlayerList() :
	m_local(NULL),
	m_playerCount(0)
{
	// we only allocate a few of these, so don't bother pooling 'em
	for (Int i = 0; i < MAX_PLAYER_COUNT; i++)
		m_players[ i ] = NEW Player( i );
	init();
}

//-----------------------------------------------------------------------------
PlayerList::~PlayerList() 
{
	try {
		// the world is happier if we reinit things before destroying them,
		// to avoid debug warnings
		init();
	} catch (...) {
		// nothing
	}
	for( Int i = 0; i < MAX_PLAYER_COUNT; ++i )
		delete m_players[ i ];
}

//-----------------------------------------------------------------------------
Player *PlayerList::getNthPlayer(Int i) 
{ 
	if( i < 0 || i >= MAX_PLAYER_COUNT )
	{
//		DEBUG_CRASH( ("Illegal player index\n") );
		return NULL;
	}
	return m_players[i]; 
}

//-----------------------------------------------------------------------------
Player *PlayerList::findPlayerWithNameKey(NameKeyType key)
{
	for (Int i = 0; i < m_playerCount; i++)
	{
		if (m_players[i]->getPlayerNameKey() == key)
		{
			return m_players[i];
		}
	}
	return NULL;
}

//-----------------------------------------------------------------------------
void PlayerList::reset()
{
	// TheSuperHackers @bugfix xezon 20/03/2025 Delete the Player AI before purging the dependent teams in the factory.
	for (int i = 0; i < MAX_PLAYER_COUNT; i++)
		m_players[i]->deletePlayerAI();

	TheTeamFactory->clear(); // cleans up energy, among other things
	init();
}

//-----------------------------------------------------------------------------
void PlayerList::newGame()
{
	Int i;

	DEBUG_ASSERTCRASH(this != NULL, ("null this"));

	reset();

	// ok, now create the rest of players we need.
	Bool setLocal = false;
	for( i = 0; i < TheSidesList->getNumSides(); i++)
	{
		Dict *d = TheSidesList->getSideInfo(i)->getDict();
		AsciiString pname = d->getAsciiString(TheKey_playerName);
		if (pname.isEmpty())
			continue;	// it's neutral, which we've already done, so skip it.

		/// @todo The Player class should have a reset() method, instead of directly calling initFromDict() (MSB)
		Player* p = m_players[m_playerCount++];
		p->initFromDict(d);

		// Multiplayer override
		Bool exists;	// throwaway, since we don't care if it exists
		if (d->getBool(TheKey_multiplayerIsLocal, &exists))
		{
			DEBUG_LOG(("Player %s is multiplayer local\n", pname.str()));
			setLocalPlayer(p);
			setLocal = true;
		}

		if (!setLocal && !TheNetwork && d->getBool(TheKey_playerIsHuman))
		{
			setLocalPlayer(p);
			setLocal = true;
		}

		// Set the build list.
		p->setBuildList(TheSidesList->getSideInfo(i)->getBuildList());
		// Build list is attached to player now, so release it from the side info.
		TheSidesList->getSideInfo(i)->releaseBuildList();
	}

	if (!setLocal)
	{
		DEBUG_ASSERTCRASH(TheNetwork, ("*** Map has no human player... picking first nonneutral player for control\n"));
		for( i = 0; i < TheSidesList->getNumSides(); i++)
		{
			Player* p = getNthPlayer(i);
			if (p != getNeutralPlayer())
			{
				p->setPlayerType(PLAYER_HUMAN, false);
				setLocalPlayer(p);
				setLocal = true;
				break;
			}
		}
	}

	// must reset teams *after* creating players.
	TheTeamFactory->initFromSides(TheSidesList);

	for( i = 0; i < TheSidesList->getNumSides(); i++)
	{
		Dict *d = TheSidesList->getSideInfo(i)->getDict();
		Player* p = findPlayerWithNameKey(NAMEKEY(d->getAsciiString(TheKey_playerName)));

		AsciiString tok;

		AsciiString enemies = d->getAsciiString(TheKey_playerEnemies);
		while (enemies.nextToken(&tok))
		{
			Player *p2 = findPlayerWithNameKey(NAMEKEY(tok));
			if (p2)
			{
				p->setPlayerRelationship(p2, ENEMIES);
			}
			else
			{
				DEBUG_LOG(("unknown enemy %s\n",tok.str()));
			}
		}

		AsciiString allies = d->getAsciiString(TheKey_playerAllies);
		while (allies.nextToken(&tok))
		{
			Player *p2 = findPlayerWithNameKey(NAMEKEY(tok));
			if (p2)
			{
				p->setPlayerRelationship(p2, ALLIES);
			}
			else
			{
				DEBUG_LOG(("unknown ally %s\n",tok.str()));
			}
		}

		// finally, make sure self & neutral are correct.
		p->setPlayerRelationship(p, ALLIES);
		if (p != getNeutralPlayer())
			p->setPlayerRelationship(getNeutralPlayer(), NEUTRAL);

		p->setDefaultTeam();
	}

}

//-----------------------------------------------------------------------------
void PlayerList::init()
{
	m_playerCount = 1;
	m_players[0]->init(NULL);

	for (int i = 1; i < MAX_PLAYER_COUNT; i++)
		m_players[i]->init(NULL);

	// call setLocalPlayer so that becomingLocalPlayer() gets called appropriately
	setLocalPlayer(m_players[0]);

}

//-----------------------------------------------------------------------------
void PlayerList::update()
{
	// update all players
	for( Int i = 0; i < MAX_PLAYER_COUNT; i++ )
	{
		m_players[i]->update();
	}  // end for i

}

//-----------------------------------------------------------------------------
void PlayerList::newMap()
{
	// update all players
	for( Int i = 0; i < MAX_PLAYER_COUNT; i++ )
	{
		m_players[i]->newMap();
	}  // end for i

}

// ------------------------------------------------------------------------
void PlayerList::teamAboutToBeDeleted(Team* team)
{
	for( Int i = 0; i < MAX_PLAYER_COUNT; i++ )
	{
		m_players[i]->removeTeamRelationship(team);
	}
}

//=============================================================================
void PlayerList::updateTeamStates(void) 
{
	// Clear team flags for all players.
	for( Int i = 0; i < MAX_PLAYER_COUNT; i++ )
	{
		m_players[i]->updateTeamStates();
	}  // end for i
}

//-----------------------------------------------------------------------------
Team *PlayerList::validateTeam( AsciiString owner )
{
	// owner could be a player or team. first, check team names.
	Team *t = TheTeamFactory->findTeam(owner);
	if (t)
	{
		//DEBUG_LOG(("assigned obj %08lx to team %s\n",obj,owner.str()));
	}	
	else
	{
		DEBUG_CRASH(("no team or player named %s could be found!\n", owner.str()));
		t = getNeutralPlayer()->getDefaultTeam();
	}
	return t;
}

//-----------------------------------------------------------------------------
void PlayerList::setLocalPlayer(Player *player)
{
	// can't set local player to null -- if you try, you get neutral.
	if (player == NULL)
	{
		DEBUG_CRASH(("local player may not be null"));
		player = getNeutralPlayer();
	}

	if (player != m_local)
	{
		// m_local can be null the very first time we call this.
		if (m_local)
			m_local->becomingLocalPlayer(false);
		m_local = player;
		player->becomingLocalPlayer(true);
	}

#ifdef INTENSE_DEBUG
	if (player)
	{
		DEBUG_LOG(("\n----------\n"));
		// did you know? you can use "%ls" to print a doublebyte string, even in a single-byte printf...
		DEBUG_LOG(("Switching local players. The new player is named '%ls' (%s) and owns the following objects:\n",
			player->getPlayerDisplayName().str(),
			TheNameKeyGenerator->keyToName(player->getPlayerNameKey()).str()
		));
		for (Object *obj = player->getFirstOwnedObject(); obj; obj = obj->getNextOwnedObject())
		{
			DEBUG_LOG(("Obj %08lx is of type %s",obj,obj->getTemplate()->getName().str()));
			if (!player->canBuild(obj->getTemplate()))
			{
				DEBUG_LOG((" (NOT BUILDABLE)"));
			}
			DEBUG_LOG(("\n"));
		}
		DEBUG_LOG(("\n----------\n"));
	}
#endif

}

//-----------------------------------------------------------------------------
Player *PlayerList::getPlayerFromMask( PlayerMaskType mask )
{
	Player *player = NULL;
	Int i;

	for( i = 0; i < MAX_PLAYER_COUNT; i++ )
	{
		
		player = getNthPlayer( i );
		if( player && player->getPlayerMask() == mask )
			return player;

	}  // end for i

	DEBUG_CRASH( ("Player does not exist for mask\n") );
	return NULL; // mask not found

}  // end getPlayerFromMask

//-----------------------------------------------------------------------------
Player *PlayerList::getEachPlayerFromMask( PlayerMaskType& maskToAdjust )
{
	Player *player = NULL;
	Int i;

	for( i = 0; i < MAX_PLAYER_COUNT; i++ )
	{
		
		player = getNthPlayer( i );
		if ( player && BitIsSet(player->getPlayerMask(), maskToAdjust ))
		{
			maskToAdjust &= (~player->getPlayerMask());
			return player;
		}
	}  // end for i

	DEBUG_CRASH( ("No players found that contain any matching masks.\n") );
	maskToAdjust = 0;
	return NULL; // mask not found
}


//-------------------------------------------------------------------------------------------------
PlayerMaskType PlayerList::getPlayersWithRelationship( Int srcPlayerIndex, UnsignedInt allowedRelationships )
{
	PlayerMaskType retVal = 0;

	if (allowedRelationships == 0)
		return retVal;

	Player *srcPlayer = getNthPlayer(srcPlayerIndex);
	if (!srcPlayer)
		return retVal;

	if (BitIsSet(allowedRelationships, ALLOW_SAME_PLAYER))
		BitSet(retVal, srcPlayer->getPlayerMask());

	for ( Int i = 0; i < getPlayerCount(); ++i )
	{
		Player *player = getNthPlayer(i);
		if (!player)
			continue;

		if (player == srcPlayer)
			continue;

		switch (srcPlayer->getRelationship(player->getDefaultTeam()))
		{
			case ENEMIES:
				if (BitIsSet(allowedRelationships, ALLOW_ENEMIES))
					BitSet(retVal, player->getPlayerMask());
				break;
			case ALLIES:
				if (BitIsSet(allowedRelationships, ALLOW_ALLIES))
					BitSet(retVal, player->getPlayerMask());
				break;
			case NEUTRAL:
				if (BitIsSet(allowedRelationships, ALLOW_NEUTRAL))
					BitSet(retVal, player->getPlayerMask());
				break;
		}
	}

	return retVal;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void PlayerList::crc( Xfer *xfer )
{
	xfer->xferInt( &m_playerCount );

	for( Int i = 0; i < m_playerCount; ++i )
		xfer->xferSnapshot( m_players[ i ] );
}

// ------------------------------------------------------------------------------------------------
/** Xfer Method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void PlayerList::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// xfer the player count
	Int playerCount = m_playerCount;
	xfer->xferInt( &playerCount );

	//
	// sanity, the player count read from the file should match our player count that
	// was setup from the bare bones map load since that data can't change during run time
	//
	if( playerCount != m_playerCount )
	{

		DEBUG_CRASH(( "Invalid player count '%d', should be '%d'\n", playerCount, m_playerCount ));
		throw SC_INVALID_DATA;

	}  // end if

	// xfer each of the player data
	for( Int i = 0; i < playerCount; ++i )
		xfer->xferSnapshot( m_players[ i ] );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void PlayerList::loadPostProcess( void )
{

}  // end postProcessLoad

