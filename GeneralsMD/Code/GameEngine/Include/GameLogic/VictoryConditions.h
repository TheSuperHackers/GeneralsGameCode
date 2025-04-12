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

// FILE: VictoryConditions.h //////////////////////////////////////////////////////
// Generals multiplayer victory condition specifications
// Author: Matthew D. Campbell, February 2002

#pragma once

#ifndef __VICTORYCONDITIONS_H__
#define __VICTORYCONDITIONS_H__

#include "Common/SubsystemInterface.h"
#include "Lib/BaseType.h"

class Player;

/*
 * bitfield for specifying which victory conditions will apply in multiplayer games
 */
enum VictoryType CPP_11(: Int)
{
	VICTORY_NOBUILDINGS = 1,
	VICTORY_NOUNITS = 2,
};

/**
  * VictoryConditionsInterface class - maintains information about the game setup and
	* the contents of its slot list hroughout the game.
	*/
class VictoryConditionsInterface : public SubsystemInterface
{
public:
	VictoryConditionsInterface() { m_victoryConditions = 0; }
	
	virtual void init( void ) = 0;
	virtual void reset( void ) = 0;
	virtual void update( void ) = 0;

	inline void setVictoryConditions( Int victoryConditions ) { m_victoryConditions = victoryConditions; }
	inline Int getVictoryConditions( void ) { return m_victoryConditions; }

	virtual Bool hasAchievedVictory(Player *player) = 0;					///< has a specific player and his allies won?
	virtual Bool hasBeenDefeated(Player *player) = 0;							///< has a specific player and his allies lost?
	virtual Bool hasSinglePlayerBeenDefeated(Player *player) = 0;	///< has a specific player lost?

	virtual void cachePlayerPtrs( void ) = 0;											///< players have been created - cache the ones of interest

	virtual Bool isLocalAlliedVictory( void ) = 0;								///< convenience function
	virtual Bool isLocalAlliedDefeat( void ) = 0;									///< convenience function
	virtual Bool isLocalDefeat( void ) = 0;												///< convenience function
	virtual Bool amIObserver( void ) = 0;													///< Am I an observer?( need this for scripts )
	virtual UnsignedInt getEndFrame( void ) = 0;									///< on which frame was the game effectively over?
protected:
	Int m_victoryConditions;
};

VictoryConditionsInterface * createVictoryConditions( void );

extern VictoryConditionsInterface *TheVictoryConditions;

#endif // __VICTORYCONDITIONS_H__
