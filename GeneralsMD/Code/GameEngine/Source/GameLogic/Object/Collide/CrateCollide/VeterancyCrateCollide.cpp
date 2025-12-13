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

// FILE: VeterancyCrateCollide.cpp ///////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, March 2002
// Desc:   A crate that gives a level of experience to all within n distance
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine
#include "Common/Player.h"
#include "Common/Xfer.h"
#include "GameLogic/ExperienceTracker.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/Module/VeterancyCrateCollide.h"
#include "GameLogic/ScriptEngine.h"
#include "GameLogic/Module/AIUpdate.h"

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
VeterancyCrateCollide::VeterancyCrateCollide( Thing *thing, const ModuleData* moduleData ) : CrateCollide( thing, moduleData )
{

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
VeterancyCrateCollide::~VeterancyCrateCollide( void )
{

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Int VeterancyCrateCollide::getLevelsToGain() const
{
	const VeterancyCrateCollideModuleData* d = getVeterancyCrateCollideModuleData();
	if (!d || !d->m_addsOwnerVeterancy)
		return 1;

	// this requires that "regular" is 0, vet is 1, etc.
	return (Int)getObject()->getVeterancyLevel();
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Bool VeterancyCrateCollide::isValidToExecute( const Object *other ) const
{
	const VeterancyCrateCollideModuleData* d = getVeterancyCrateCollideModuleData();
	if( !d )
	{
		return false;
	}

	if(!CrateCollide::isValidToExecute(other))
		return false;

	if (other->isEffectivelyDead())
		return false;

	if( other->isSignificantlyAboveTerrain() )
	{
		return false;
	}

	Int levelsToGain = getLevelsToGain();
	if (levelsToGain <= 0)
		return false;

	const ExperienceTracker *et = other->getExperienceTracker();
	if( !et || !et->isTrainable() )
	{
		//If the other unit can't gain experience, then we can't help promote it!
		return false;
	}

	if (!et || !et->canGainExpForLevel(levelsToGain))
		return false;

	if( d->m_isPilot )
	{
		if( other->getControllingPlayer() != getObject()->getControllingPlayer() )
		{
			//This is a pilot and we are checking to make sure the pilot is entering a vehicle on
			//the same team. If it's not, then don't allow it.. this is particularly the case for
			//pilots attempting to enter civilian vehicles.
			return false;
		}

		if( other->isUsingAirborneLocomotor() )
		{
			// Can't upgrade a helicopter or plane, but we will think we can for a moment while it
			// is on the ground from being built.
			return false;
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Bool VeterancyCrateCollide::executeCrateBehavior( Object *other )
{
 	//Make sure the pilot is actually *TRYING* to enter the object
 	//unlike other crates
 	AIUpdateInterface *ai = (AIUpdateInterface*)getObject()->getAIUpdateInterface();
	const VeterancyCrateCollideModuleData *md = getVeterancyCrateCollideModuleData();

 	if( !ai || ai->getGoalObject() != other )
 	{
 		return false;
 	}

	Int levelsToGain = getLevelsToGain();
	Real range = md->m_rangeOfEffect;
	if (range == 0)
	{
		// do just the collider
		if (other != NULL)
		{
			other->getExperienceTracker()->gainExpForLevel( levelsToGain, ( ! md->m_isPilot) );
		}
	}
	else
	{
		PartitionFilterSamePlayer othersPlayerFilter( other->getControllingPlayer() );
		PartitionFilterSameMapStatus filterMapStatus(other);
		PartitionFilter *filters[] = { &othersPlayerFilter, &filterMapStatus, NULL };
		ObjectIterator *iter = ThePartitionManager->iterateObjectsInRange( other, range, FROM_CENTER_2D, filters, ITER_FASTEST );
		MemoryPoolObjectHolder hold(iter);

		for( Object *potentialObject = iter->first(); potentialObject; potentialObject = iter->next() )
		{
			// This function will give just enough exp for the Object to gain a level, if it can
			potentialObject->getExperienceTracker()->gainExpForLevel( levelsToGain, ( ! md->m_isPilot) );
		}
	}

	//In order to make things easier for the designers, we are going to transfer the terrorist name
	//to the car... so the designer can control the car with their scripts.
	if( md->m_isPilot )
	{
		TheScriptEngine->transferObjectName( getObject()->getName(), other );
	}

	return TRUE;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void VeterancyCrateCollide::crc( Xfer *xfer )
{

	// extend base class
	CrateCollide::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void VeterancyCrateCollide::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	CrateCollide::xfer( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void VeterancyCrateCollide::loadPostProcess( void )
{

	// extend base class
	CrateCollide::loadPostProcess();

}
