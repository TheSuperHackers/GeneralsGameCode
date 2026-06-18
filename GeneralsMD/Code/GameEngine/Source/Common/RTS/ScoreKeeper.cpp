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

// FILE: ScoreKeeper.cpp /////////////////////////////////////////////////
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
//	Filename: 	ScoreKeeper.cpp
//
//	author:		Chris Huybregts
//
//	purpose:	Score Keeper class will be an object attached to each player
//						that will maintain accurate counts for the various stats we
//						want to show on the score screen.  The information in here
//						could also be used for the observer screen
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/GameState.h"
#include "Common/KindOf.h"
#include "Common/Player.h"
#include "Common/ScoreKeeper.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"

#include "GameLogic/Object.h"
#include "GameLogic/GameLogic.h"

//-----------------------------------------------------------------------------
// DEFINES ////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

ScoreKeeper::ScoreKeeper()
{
	reset(0);
}

ScoreKeeper::~ScoreKeeper()
{

}

static KindOfMaskType scoringBuildingMask;
static KindOfMaskType scoringBuildingDestroyMask;
static KindOfMaskType scoringBuildingCreateMask;

void ScoreKeeper::reset( Int playerIdx )
{
	scoringBuildingMask.set(KINDOF_STRUCTURE);
	scoringBuildingMask.set(KINDOF_SCORE);

	scoringBuildingCreateMask.set(KINDOF_STRUCTURE);
	scoringBuildingCreateMask.set(KINDOF_SCORE_CREATE);

	scoringBuildingDestroyMask.set(KINDOF_STRUCTURE);
	scoringBuildingDestroyMask.set(KINDOF_SCORE_DESTROY);

	m_totalMoneyEarned = m_totalMoneySpent = 0;
	for(int t = 0; t < INCOME_COUNT; ++t)
		m_incomeByType[t] = 0;
	m_totalUnitsLost = m_totalUnitsBuilt = 0;
	m_totalBuildingsLost = m_totalBuildingsBuilt = 0;
	m_totalFactionBuildingsCaptured = m_totalTechBuildingsCaptured = 0;
	m_currentScore = 0;
	m_objectsBuilt.clear();
	m_objectsCaptured.clear();
	m_objectsLost.clear();
	for(int i = 0; i < MAX_PLAYER_COUNT; ++i)
	{
		m_objectsDestroyed[i].clear();
		m_totalBuildingsDestroyed[i] = m_totalUnitsDestroyed[i] = 0;
	}
	m_myPlayerIdx	= playerIdx;
}

void ScoreKeeper::addObjectBuilt( const Object *o)
{
	Bool addToCount = FALSE;

	if (TheGameLogic->isScoringEnabled() == FALSE) {
		return;
	}

	if(o->getTemplate()->isKindOfMulti(scoringBuildingMask, KINDOFMASK_NONE))
	{
		++m_totalBuildingsBuilt;
		addToCount = TRUE;
	}
	else if (o->getTemplate()->isKindOfMulti(scoringBuildingCreateMask, KINDOFMASK_NONE))
	{
		++m_totalBuildingsBuilt;
		addToCount = TRUE;
	}
	else if(o->getTemplate()->isKindOf(KINDOF_INFANTRY) || o->getTemplate()->isKindOf(KINDOF_VEHICLE))
	{
		if (o->getTemplate()->isKindOf(KINDOF_SCORE) || o->getTemplate()->isKindOf(KINDOF_SCORE_CREATE))
		{
			++m_totalUnitsBuilt;
			addToCount = TRUE;
		}
	}

	if(addToCount)
	{
		Int existingCount = 0;
		ObjectCountMapIt it = m_objectsBuilt.find(o->getTemplate());
		if (it != m_objectsBuilt.end())
			existingCount = it->second;
		m_objectsBuilt[o->getTemplate()] = existingCount + 1;
	}
}

Int ScoreKeeper::getTotalUnitsBuilt( KindOfMaskType validMask, KindOfMaskType invalidMask )
{
	Int count = 0;
	for (ObjectCountMapIt it = m_objectsBuilt.begin(); it != m_objectsBuilt.end(); ++it)
	{
		const ThingTemplate *theTemplate = it->first;
		Int numBuilt = it->second;
		if (theTemplate && theTemplate->isKindOfMulti(validMask, invalidMask))
			count += numBuilt;
	}
	return count;
}

Int ScoreKeeper::getTotalObjectsBuilt( const ThingTemplate *pTemplate )
{
	Int count = 0;
	for (ObjectCountMapIt it = m_objectsBuilt.begin(); it != m_objectsBuilt.end(); ++it)
	{
		const ThingTemplate *theTemplate = it->first;
		if (theTemplate && theTemplate->isEquivalentTo(pTemplate))
			++count;
	}
	return count;
}


void ScoreKeeper::removeObjectBuilt( const Object *o)
{
	if (TheGameLogic->isScoringEnabled() == FALSE) {
		return;
	}

	Bool removeFromCount = FALSE;
	if (o->getTemplate()->isKindOfMulti(scoringBuildingMask, KINDOFMASK_NONE))
	{
		--m_totalBuildingsBuilt;
		removeFromCount = TRUE;
	}
	else if (o->getTemplate()->isKindOfMulti(scoringBuildingCreateMask, KINDOFMASK_NONE))
	{
		--m_totalBuildingsBuilt;
		removeFromCount = TRUE;
	}
	else if (o->getTemplate()->isKindOf(KINDOF_INFANTRY) || o->getTemplate()->isKindOf(KINDOF_VEHICLE))
	{
		if (o->getTemplate()->isKindOf(KINDOF_SCORE) || o->getTemplate()->isKindOf(KINDOF_SCORE_CREATE))
		{
			--m_totalUnitsBuilt;
			removeFromCount = TRUE;
		}
	}

	if (removeFromCount)
	{
		Int existingCount = 0;
		ObjectCountMapIt it = m_objectsBuilt.find(o->getTemplate());
		if (it != m_objectsBuilt.end())
			existingCount = it->second;
		m_objectsBuilt[o->getTemplate()] = existingCount - 1;
	}
}

void ScoreKeeper::addObjectCaptured( const Object *o )
{
	if (TheGameLogic->isScoringEnabled() == FALSE) {
		return;
	}

	Bool addToCount = FALSE;
	if(o->getTemplate()->isKindOf(KINDOF_STRUCTURE))
	{
		if (o->getTemplate()->isKindOf(KINDOF_SCORE))
		{
			++m_totalFactionBuildingsCaptured;
		}
		else
		{
			++m_totalTechBuildingsCaptured;
		}
		addToCount = TRUE;
	}

	if(addToCount)
	{
		Int existingCount = 0;
		ObjectCountMapIt it = m_objectsCaptured.find(o->getTemplate());
		if (it != m_objectsCaptured.end())
			existingCount = it->second;
		m_objectsCaptured[o->getTemplate()] = existingCount + 1;
	}
}



void ScoreKeeper::addObjectDestroyed( const Object *o)
{

	if (TheGameLogic->isScoringEnabled() == FALSE) {
		return;
	}

	Int playerIdx = o->getControllingPlayer()->getPlayerIndex();

	Bool addToCount = FALSE;
	if(o->getTemplate()->isKindOfMulti(scoringBuildingMask, KINDOFMASK_NONE))
	{
		if (!(o->testStatus(OBJECT_STATUS_UNDER_CONSTRUCTION))) {
			++m_totalBuildingsDestroyed[playerIdx];
			addToCount = TRUE;
		}
	}
	else if (o->getTemplate()->isKindOfMulti(scoringBuildingDestroyMask, KINDOFMASK_NONE))
	{
		if (!(o->testStatus(OBJECT_STATUS_UNDER_CONSTRUCTION))) {
			++m_totalBuildingsDestroyed[playerIdx];
			addToCount = TRUE;
		}
	}
	else if(o->getTemplate()->isKindOf(KINDOF_INFANTRY) || o->getTemplate()->isKindOf(KINDOF_VEHICLE))
	{
		if (o->getTemplate()->isKindOf(KINDOF_SCORE) || o->getTemplate()->isKindOf(KINDOF_SCORE_DESTROY))
		{
			if (!(o->testStatus(OBJECT_STATUS_UNDER_CONSTRUCTION))) {
				m_totalUnitsDestroyed[playerIdx]++;
				addToCount = TRUE;
			}
		}
	}

	if(addToCount)
	{
		Int existingCount = 0;
		ObjectCountMapIt it = m_objectsDestroyed[playerIdx].find(o->getTemplate());
		if (it != m_objectsDestroyed[playerIdx].end())
			existingCount = it->second;
		m_objectsDestroyed[playerIdx][o->getTemplate()] = existingCount + 1;
	}
}

void ScoreKeeper::addObjectLost( const Object *o )
{
	if (TheGameLogic->isScoringEnabled() == FALSE) {
		return;
	}

	Bool addToCount = FALSE;
	if(o->getTemplate()->isKindOfMulti(scoringBuildingMask, KINDOFMASK_NONE))
	{
		if (!(o->testStatus(OBJECT_STATUS_UNDER_CONSTRUCTION))) {
			++m_totalBuildingsLost;
			addToCount = TRUE;
		}
	}
	else if (o->getTemplate()->isKindOfMulti(scoringBuildingDestroyMask, KINDOFMASK_NONE))
	{
		if (!(o->testStatus(OBJECT_STATUS_UNDER_CONSTRUCTION))) {
			++m_totalBuildingsLost;
			addToCount = TRUE;
		}
	}
	else if(o->getTemplate()->isKindOf(KINDOF_INFANTRY) || o->getTemplate()->isKindOf(KINDOF_VEHICLE))
	{
		if (o->getTemplate()->isKindOf(KINDOF_SCORE) || o->getTemplate()->isKindOf(KINDOF_SCORE_DESTROY))
		{
			if (!(o->testStatus(OBJECT_STATUS_UNDER_CONSTRUCTION))) {
				++m_totalUnitsLost;
				addToCount = TRUE;
			}
		}
	}

	if(addToCount)
	{
		Int existingCount = 0;
		ObjectCountMapIt it = m_objectsLost.find(o->getTemplate());
		if (it != m_objectsLost.end())
			existingCount = it->second;
		m_objectsLost[o->getTemplate()] = existingCount + 1;
	}
}

// Accumulate income into its source bucket. Kept independent of
// m_totalMoneyEarned (and therefore of calculateScore) so adding source
// tracking to deposit sites that historically were not counted as "earned"
// (kill bounty, theft) does not change the displayed score.
void ScoreKeeper::recordIncome( Int money, IncomeType type )
{
	if( type < 0 || type >= INCOME_COUNT )
		type = INCOME_OTHER;
	m_incomeByType[type] += money;
}

Int ScoreKeeper::calculateScore()
{
	Int score = 0;
	score += m_totalUnitsBuilt * 100;
	score += m_totalMoneyEarned;
	score += m_totalBuildingsBuilt * 100;
	for (Int i = 0; i < MAX_PLAYER_COUNT; ++i)
	{
		if(i == m_myPlayerIdx)
			continue;
		score += m_totalUnitsDestroyed[i] * 100;
		score += m_totalBuildingsDestroyed[i] * 100;
	}

	m_currentScore = score;
	return m_currentScore;

}

//-----------------------------------------------------------------------------
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

Int ScoreKeeper::getTotalBuildingsDestroyed()
{
	int count = 0;
	for (int i = 0; i< MAX_PLAYER_COUNT; ++i)
	{
	// Design change, display even if we killed our own
//		if(i == m_myPlayerIdx)
//			continue;
		count += m_totalBuildingsDestroyed[i];
		//for (ObjectCountMapIt it = m_objectsDestroyed[i].begin(); it != m_objectsDestroyed[i].end(); ++it)
		//		{
		//
		//			count += it->second;
		//		}

	}
	return count;
}
Int ScoreKeeper::getTotalUnitsDestroyed()
{
	int count = 0;
	for (int i = 0; i< MAX_PLAYER_COUNT; ++i)
	{
	// Design change, display even if we killed our own
//		if(i == m_myPlayerIdx)
//			continue;
		count += m_totalUnitsDestroyed[i];
//		for (ObjectCountMapIt it = m_objectsDestroyed[i].begin(); it != m_objectsDestroyed[i].end(); ++it)
//		{
//		}
	}
	return count;
}

// Sum (count * buildCost) across a map, filtered by KINDOF_STRUCTURE so
// we can distinguish unit value from building value. Entries already
// passed addObjectBuilt/Lost/Destroyed's scoring gates, so non-structure
// entries are guaranteed to be score-worthy units. const-iterates a
// non-const map by passing the map address through the function param.
static Int sumBuildCost(const ScoreKeeper::ObjectCountMap *map, Bool wantStructures)
{
	Int total = 0;
	if (map == nullptr)
		return 0;
	for (ScoreKeeper::ObjectCountMap::const_iterator it = map->begin(); it != map->end(); ++it)
	{
		const ThingTemplate *t = it->first;
		if (t == nullptr)
			continue;
		const Bool isStructure = t->isKindOf(KINDOF_STRUCTURE);
		if (isStructure == wantStructures)
			total += it->second * (Int)t->friend_getBuildCost();
	}
	return total;
}

Int ScoreKeeper::getTotalUnitsBuiltValue() const
{
	return sumBuildCost(&m_objectsBuilt, FALSE);
}

Int ScoreKeeper::getTotalUnitsLostValue() const
{
	return sumBuildCost(&m_objectsLost, FALSE);
}

Int ScoreKeeper::getTotalUnitsDestroyedValue() const
{
	Int total = 0;
	for (Int i = 0; i < MAX_PLAYER_COUNT; ++i)
		total += sumBuildCost(&m_objectsDestroyed[i], FALSE);
	return total;
}

Int ScoreKeeper::getTotalBuildingsBuiltValue() const
{
	return sumBuildCost(&m_objectsBuilt, TRUE);
}

Int ScoreKeeper::getTotalBuildingsLostValue() const
{
	return sumBuildCost(&m_objectsLost, TRUE);
}

Int ScoreKeeper::getTotalBuildingsDestroyedValue() const
{
	Int total = 0;
	for (Int i = 0; i < MAX_PLAYER_COUNT; ++i)
		total += sumBuildCost(&m_objectsDestroyed[i], TRUE);
	return total;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void ScoreKeeper::crc( Xfer *xfer )
{

}

// ------------------------------------------------------------------------------------------------
/** Xfer of an object count map
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void ScoreKeeper::xferObjectCountMap( Xfer *xfer, ObjectCountMap *map )
{

	// sanity
	if( map == nullptr )
	{

		DEBUG_CRASH(( "xferObjectCountMap - Invalid map parameter" ));
		throw SC_INVALID_DATA;

	}

	// version info
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// size of the map
	UnsignedShort mapSize = map->size();
	xfer->xferUnsignedShort( &mapSize );

	// map data
	Int count;
	const ThingTemplate *thingTemplate;
	AsciiString thingTemplateName;
	if( xfer->getXferMode() == XFER_SAVE )
	{
		ObjectCountMapIt it;

		// save all entries
		for( it = map->begin(); it != map->end(); ++it )
		{

			// thing template
			thingTemplate = it->first;
			thingTemplateName = thingTemplate->getName();
			xfer->xferAsciiString( &thingTemplateName );

			// the count
			count = it->second;
			xfer->xferInt( &count );

		}

	}
	else
	{

		// read all entries
		for( UnsignedShort i = 0; i < mapSize; ++i )
		{

			// read thing template name
			xfer->xferAsciiString( &thingTemplateName );
			thingTemplate = TheThingFactory->findTemplate( thingTemplateName );
			if( thingTemplate == nullptr )
			{

				DEBUG_CRASH(( "xferObjectCountMap - Unknown thing template '%s'", thingTemplateName.str() ));
				throw SC_INVALID_DATA;

			}

			// read count
			xfer->xferInt( &count );

			// add to map
			(*map)[ thingTemplate ] = count;

		}

	}

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version
	* 2: TheSuperHackers @tweak Serialize per-source income breakdown */
// ------------------------------------------------------------------------------------------------
void ScoreKeeper::xfer( Xfer *xfer )
{

	// version
#if RETAIL_COMPATIBLE_XFER_SAVE
	XferVersion currentVersion = 1;
#else
	XferVersion currentVersion = 2;
#endif
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// money earned
	xfer->xferInt( &m_totalMoneyEarned );

	// money spent
	xfer->xferInt( &m_totalMoneySpent );

	// per-source income breakdown
	if( version >= 2 )
	{
		xfer->xferUser( m_incomeByType, sizeof( m_incomeByType ) );
	}

	// units destroyed
	xfer->xferUser( m_totalUnitsDestroyed, sizeof( Int ) * MAX_PLAYER_COUNT );

	// units built
	xfer->xferInt( &m_totalUnitsBuilt );

	// units lost
	xfer->xferInt( &m_totalUnitsLost );

	// buildings destroyed
	xfer->xferUser( m_totalBuildingsDestroyed, sizeof( Int ) * MAX_PLAYER_COUNT );

	// buildings built
	xfer->xferInt( &m_totalBuildingsBuilt );

	// buildings lost
	xfer->xferInt( &m_totalBuildingsLost );

	// tech buildings captured
	xfer->xferInt( &m_totalTechBuildingsCaptured );

	// faction buildings captured
	xfer->xferInt( &m_totalFactionBuildingsCaptured );

	// current score
	xfer->xferInt( &m_currentScore );

	// player index
	xfer->xferInt( &m_myPlayerIdx );

	// objects built
	xferObjectCountMap( xfer, &m_objectsBuilt );

	// objects destroyed
	UnsignedShort destroyedArraySize = MAX_PLAYER_COUNT;
	xfer->xferUnsignedShort( &destroyedArraySize );
	if( destroyedArraySize != MAX_PLAYER_COUNT )
	{

		DEBUG_CRASH(( "ScoreKeeper::xfer - size of objects destroyed array has changed" ));
		throw SC_INVALID_DATA;

	}
	for( UnsignedShort i = 0; i < destroyedArraySize; ++i )
	{

		// xfer map data
		xferObjectCountMap( xfer, &m_objectsDestroyed[ i ] );

	}

	// objects lost
	xferObjectCountMap( xfer, &m_objectsLost );

	// objects captured
	xferObjectCountMap( xfer, &m_objectsCaptured );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void ScoreKeeper::loadPostProcess()
{

}
