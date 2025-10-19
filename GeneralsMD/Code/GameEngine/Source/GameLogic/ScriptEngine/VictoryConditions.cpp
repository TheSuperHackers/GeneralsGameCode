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

// FILE: VictoryConditions.cpp //////////////////////////////////////////////////////
// Generals multiplayer victory condition specifications
// Author: Matthew D. Campbell, February 2002

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/AudioEventRTS.h"
#include "Common/GameAudio.h"
#include "Common/GameCommon.h"
#include "Common/GameEngine.h"
#include "Common/GameUtility.h"
#include "Common/KindOf.h"
#include "Common/PlayerList.h"
#include "Common/Player.h"
#include "Common/PlayerTemplate.h"
#include "Common/Radar.h"
#include "Common/Recorder.h"

#include "GameClient/InGameUI.h"
#include "GameClient/Diplomacy.h"
#include "GameClient/GameText.h"
#include "GameClient/GUICallbacks.h"
#include "GameClient/MessageBox.h"
#include "GameClient/GameClient.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/ScriptActions.h"
#include "GameLogic/VictoryConditions.h"
#include "GameNetwork/GameInfo.h"
#include "GameNetwork/NetworkDefs.h"


//-------------------------------------------------------------------------------------------------
#define ISSET(x) (m_victoryConditions & VICTORY_##x)

//-------------------------------------------------------------------------------------------------
VictoryConditionsInterface *TheVictoryConditions = NULL;

//-------------------------------------------------------------------------------------------------
inline static Bool areAllies(const Player *p1, const Player *p2)
{
	if (p1 != p2 &&
		p1->getRelationship(p2->getDefaultTeam()) == ALLIES &&
		p2->getRelationship(p1->getDefaultTeam()) == ALLIES)
		return true;

	return false;
}

//-------------------------------------------------------------------------------------------------
class VictoryConditions : public VictoryConditionsInterface
{
public:
	VictoryConditions();

	void init( void );
	void reset( void );
	void update( void );

	Bool hasAchievedVictory(Player *player);					///< has a specific player and his allies won?
	Bool hasBeenDefeated(Player *player);							///< has a specific player and his allies lost?
	Bool hasSinglePlayerBeenDefeated(Player *player);	///< has a specific player lost?

	void cachePlayerPtrs( void );											///< players have been created - cache the ones of interest

	Bool isLocalAlliedVictory( void );								///< convenience function
	Bool isLocalAlliedDefeat( void );									///< convenience function
	Bool isLocalDefeat( void );												///< convenience function
	Bool amIObserver( void ) { return m_isObserver;} 	///< Am I an observer?( need this for scripts )
	virtual UnsignedInt getEndFrame( void ) { return m_endFrame; }	///< on which frame was the game effectively over?
private:
	Player*				m_players[MAX_PLAYER_COUNT];
	Int						m_localSlotNum;
	UnsignedInt		m_endFrame;
	Bool					m_isDefeated[MAX_PLAYER_COUNT];
	Bool					m_localPlayerDefeated;												///< prevents condition from being signaled each frame
	Bool					m_singleAllianceRemaining;										///< prevents condition from being signaled each frame
	Bool					m_isObserver;
};

//-------------------------------------------------------------------------------------------------
VictoryConditionsInterface * createVictoryConditions( void )
{
	// only one created, so no MemoryPool usage
	return NEW VictoryConditions;
}

//-------------------------------------------------------------------------------------------------
VictoryConditions::VictoryConditions()
{
	reset();
}

//-------------------------------------------------------------------------------------------------
void VictoryConditions::init( void )
{
	reset();
}

//-------------------------------------------------------------------------------------------------
void VictoryConditions::reset( void )
{
	for (Int i=0; i<MAX_PLAYER_COUNT; ++i)
	{
		m_players[i] = NULL;
		m_isDefeated[i] = false;
	}
	m_localSlotNum = -1;

	m_localPlayerDefeated = false;
	m_singleAllianceRemaining = false;
	m_isObserver = false;
	m_endFrame = 0;

	m_victoryConditions = VICTORY_NOBUILDINGS | VICTORY_NOUNITS;
}

//-------------------------------------------------------------------------------------------------
void VictoryConditions::update( void )
{
	if (!TheRecorder->isMultiplayer() || (m_localSlotNum < 0 && !m_isObserver))
		return;

	// Check for a single winning alliance
	if (!m_singleAllianceRemaining)
	{
		Bool multipleAlliances = false;
		Player *alive = NULL;
		Player *player;
		for (Int i=0; i<MAX_PLAYER_COUNT; ++i)
		{
			player = m_players[i];
			if (player && !hasSinglePlayerBeenDefeated(player))
			{
				if (alive)
				{
					// check to verify they are on the same team
					if (!areAllies(alive, player))
					{
						multipleAlliances = true;
						break;
					}
				}
				else
				{
					alive = player; // save this pointer to check against
				}
			}
		}

		if (!multipleAlliances)
		{
			m_singleAllianceRemaining = true; // don't check again
			m_endFrame = TheGameLogic->getFrame();
		}
	}

	// check for player eliminations
	for (Int i=0; i<MAX_PLAYER_COUNT; ++i)
	{
		Player *p = m_players[i];
		if (p && !m_isDefeated[i] && hasSinglePlayerBeenDefeated(p))
		{
			m_isDefeated[i] = true;
			if (TheGameLogic->getFrame() > 1)
			{
				ThePartitionManager->revealMapForPlayerPermanently( p->getPlayerIndex() );
				TheGameClient->updateFakeDrawables();

				TheInGameUI->message("GUI:PlayerHasBeenDefeated", p->getPlayerDisplayName().str() );
				// People are boneheads. Also play a sound
				static AudioEventRTS leftGameSound("GUIMessageReceived");
				TheAudio->addAudioEvent(&leftGameSound);
			}

			for (Int idx = 0; idx < MAX_SLOTS; ++idx)
			{
				AsciiString pName;
				pName.format("player%d", idx);
				if (p->getPlayerNameKey() == NAMEKEY(pName))
				{
					GameSlot *slot = (TheGameInfo)?TheGameInfo->getSlot(idx):NULL;
					if (slot && slot->isAI())
					{
						DEBUG_LOG(("Marking AI player %s as defeated", pName.str()));
						slot->setLastFrameInGame(TheGameLogic->getFrame());
					}
				}
			}

			// destroy any remaining units (infantry if its a short game, for example)
			p->killPlayer();
			PopulateInGameDiplomacyPopup();
		}
	}

	// Check if the local player has been eliminated
	if (!m_localPlayerDefeated && !m_isObserver)
	{
		Player *localPlayer = m_players[m_localSlotNum];
		if (hasSinglePlayerBeenDefeated(localPlayer))
		{
			if (!m_singleAllianceRemaining)
			{
				//MessageBoxOk(TheGameText->fetch("GUI:Defeat"), TheGameText->fetch("GUI:LocalDefeat"), NULL);
			}
			m_localPlayerDefeated = true;	// don't check again
			TheRadar->forceOn(localPlayer->getPlayerIndex(), TRUE);
			SetInGameChatType( INGAME_CHAT_EVERYONE ); // can't chat to allies after death.  Only to other observers.
		}
	}
}

//-------------------------------------------------------------------------------------------------
Bool VictoryConditions::hasAchievedVictory(Player *player)
{
	if (!player)
		return false;

	if (m_singleAllianceRemaining)
	{
		for (Int i=0; i<MAX_PLAYER_COUNT; ++i)
		{
			if ( m_players[i] && !hasSinglePlayerBeenDefeated(m_players[i]) &&
				(player == m_players[i] || areAllies(m_players[i], player)) )
				return true;
		}
	}

	return false;
}

//-------------------------------------------------------------------------------------------------
Bool VictoryConditions::hasBeenDefeated(Player *player)
{
	if (!player)
		return false;

	if (m_singleAllianceRemaining && !hasAchievedVictory(player))
		return true;

	return false;
}

//-------------------------------------------------------------------------------------------------
Bool VictoryConditions::hasSinglePlayerBeenDefeated(Player *player)
{
	if (!player)
		return false;

	KindOfMaskType mask;
	mask.set(KINDOF_MP_COUNT_FOR_VICTORY);

	if ( ISSET(NOUNITS) && ISSET(NOBUILDINGS) )
	{
		if ( !player->hasAnyObjects() )
		{
			return true;
		}
	}
	else if ( ISSET(NOUNITS) )
	{
		if ( !player->hasAnyUnits() )
		{
			return true;
		}
	}
	else if ( ISSET(NOBUILDINGS) )
	{
		if ( !player->hasAnyBuildings(mask) )
		{
			return true;
		}
	}

	return false;
}

//-------------------------------------------------------------------------------------------------
void VictoryConditions::cachePlayerPtrs( void )
{
	if (!TheRecorder->isMultiplayer())
		return;

	Int playerCount = 0;
	const PlayerTemplate *civTemplate = ThePlayerTemplateStore->findPlayerTemplate( NAMEKEY("FactionCivilian") );
	for (Int i=0; i<MAX_PLAYER_COUNT; ++i)
	{
		Player *player = ThePlayerList->getNthPlayer(i);
		DEBUG_LOG(("Checking whether to cache player %d - [%ls], house [%ls]", i, player?player->getPlayerDisplayName().str():L"<NOBODY>", (player&&player->getPlayerTemplate())?player->getPlayerTemplate()->getDisplayName().str():L"<NONE>"));
		if (player && player != ThePlayerList->getNeutralPlayer() && player->getPlayerTemplate() && player->getPlayerTemplate() != civTemplate && !player->isPlayerObserver())
		{
			DEBUG_LOG(("Caching player"));
			m_players[playerCount] = player;
			if (m_players[playerCount]->isLocalPlayer())
				m_localSlotNum = playerCount;
			++playerCount;
		}
	}
	while (playerCount < MAX_PLAYER_COUNT)
	{
		m_players[playerCount++] = NULL;
	}

	if (m_localSlotNum < 0)
	{
		m_localPlayerDefeated = true;	// if we have no local player, don't check for defeat
		m_isObserver = true;
	}
}

//-------------------------------------------------------------------------------------------------
Bool VictoryConditions::isLocalAlliedVictory( void )
{
	if (m_isObserver)
		return false;

	return (hasAchievedVictory(m_players[m_localSlotNum]));
}

//-------------------------------------------------------------------------------------------------
Bool VictoryConditions::isLocalAlliedDefeat( void )
{
	if (m_isObserver)
		return m_singleAllianceRemaining;

	return (hasBeenDefeated(m_players[m_localSlotNum]));
}

//-------------------------------------------------------------------------------------------------
Bool VictoryConditions::isLocalDefeat( void )
{
	if (m_isObserver)
		return FALSE;

	return (m_localPlayerDefeated);
}



