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

// FILE: ScoreKeeper.h /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Electronic Arts Pacific.
//
//                       Confidential Information
//                Copyright (C) 2002 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
//	created:	Jun 2002
//
//	Filename: 	ScoreKeeper.h
//
//	author:		Chris Huybregts
//
//	purpose:	Header file for the scorekeeper class
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __SCOREKEEPER_H_
#define __SCOREKEEPER_H_

//-----------------------------------------------------------------------------
// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "Common/Snapshot.h"

//-----------------------------------------------------------------------------
// FORWARD REFERENCES /////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
class Object;
class ThingTemplate;
//-----------------------------------------------------------------------------
// TYPE DEFINES ///////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
class ScoreKeeper : public Snapshot
{
public:
	ScoreKeeper( void );
	~ScoreKeeper( void );

	void reset( Int playerIdx );			///< Zero out the variables
	Int calculateScore( void );				///< Performs the equation to calculate the score

	void addMoneySpent( Int money );								///< Adds money to the amount spent
	void addMoneyEarned( Int money );								///< Adds money to the earned amount

	void addObjectBuilt( const Object *o );
	void addObjectDestroyed( const Object *o );
	void addObjectLost( const Object *o );
	void addObjectCaptured( const Object *o );

	void removeObjectBuilt( const Object *o );

	Int getTotalMoneyEarned( void ) { return m_totalMoneyEarned; }
	Int getTotalMoneySpent( void ) { return m_totalMoneySpent; }
	Int getTotalUnitsDestroyed( void );
	Int getTotalUnitsBuilt( void ) { return m_totalUnitsBuilt; }
	Int getTotalUnitsLost( void ) { return m_totalUnitsLost; }
	Int getTotalBuildingsDestroyed( void );
	Int getTotalBuildingsBuilt( void ) { return m_totalBuildingsBuilt; }
	Int getTotalBuildingsLost( void ) { return m_totalBuildingsLost; }
	Int getTotalTechBuildingsCaptured( void ) { return m_totalTechBuildingsCaptured; }
	Int getTotalFactionBuildingsCaptured( void ) { return m_totalFactionBuildingsCaptured; }
	Int getTotalObjectsBuilt( const ThingTemplate *pTemplate ); // get a count of objects built matching a specific thing template

	// for battle honor calculation.  done once at the end of each online game
	Int getTotalUnitsBuilt( KindOfMaskType validMask, KindOfMaskType invalidMask );

protected:

	// snapshot methods
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void );

private:

	Int m_totalMoneyEarned;						///< The total money that was harvested, refined, received in crates
	Int m_totalMoneySpent;						///< The total money spent on units, buildings, repairs
	Int m_totalUnitsDestroyed[MAX_PLAYER_COUNT];				///< The total number of enimies that we've killed
	Int m_totalUnitsBuilt;						///< The total number of units we've created (created meaning that we built from a building)
	Int m_totalUnitsLost;							///< The total number of our units lost
	Int m_totalBuildingsDestroyed[MAX_PLAYER_COUNT];		///< The total number of Buildings we've destroyed
	Int m_totalBuildingsBuilt;				///< The total number of buildings we've constructed
	Int m_totalBuildingsLost;					///< The total number of our buildings lost
	Int m_totalTechBuildingsCaptured;	///< The total number of tech buildings captured
	Int m_totalFactionBuildingsCaptured;	///< The total number of faction buildings captured
	Int m_currentScore;								///< Based off the stats, this is the current score

	Int m_myPlayerIdx;								///< We need to not score kills on ourselves... so we need to know who we are

	typedef std::map<const ThingTemplate *, Int> ObjectCountMap;
	typedef ObjectCountMap::iterator ObjectCountMapIt;
	ObjectCountMap m_objectsBuilt;			///< How many and what kinds of objects did we build
	ObjectCountMap m_objectsDestroyed[MAX_PLAYER_COUNT];		///< How many and what kinds and who's did we kill
	ObjectCountMap m_objectsLost;				///< how many and what kinds of objects did we loose
	ObjectCountMap m_objectsCaptured;
	void xferObjectCountMap( Xfer *xfer, ObjectCountMap *map );

};

//-----------------------------------------------------------------------------
// INLINING ///////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
inline void ScoreKeeper::addMoneySpent( Int money )							{	m_totalMoneySpent += money; }
inline void ScoreKeeper::addMoneyEarned( Int money )						{	m_totalMoneyEarned += money; }

//-----------------------------------------------------------------------------
// EXTERNALS //////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

#endif // __SCOREKEEPER_H_
