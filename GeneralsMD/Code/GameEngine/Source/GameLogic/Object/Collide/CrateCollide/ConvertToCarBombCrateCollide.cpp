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

// FILE: ConvertToCarBombCrateCollide.cpp ///////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, March 2002
// Desc:   A crate that gives a level of experience to all within n distance
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#include "Common/Player.h"
#include "Common/Radar.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"
#include "GameClient/FXList.h"
#include "GameLogic/ExperienceTracker.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/ConvertToCarBombCrateCollide.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/ScriptEngine.h"
#include "GameLogic/ExperienceTracker.h"

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ConvertToCarBombCrateCollide::ConvertToCarBombCrateCollide( Thing *thing, const ModuleData* moduleData ) : CrateCollide( thing, moduleData )
{

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ConvertToCarBombCrateCollide::~ConvertToCarBombCrateCollide( void )
{

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Bool ConvertToCarBombCrateCollide::isValidToExecute( const Object *other ) const
{
	if( !CrateCollide::isValidToExecute(other) )
	{
		return FALSE;
	}

	if( other->isEffectivelyDead() )
	{
		return FALSE;
	}

	if( other->isKindOf( KINDOF_AIRCRAFT ) || other->isKindOf( KINDOF_BOAT ) )
	{
		//Can't make carbombs out of planes and boats!
		return FALSE;
	}

	if( other->getStatusBits().test( OBJECT_STATUS_IS_CARBOMB ) )
	{
		return FALSE;// oops, sorry, I'll convert the next one.
	}

	// Check to see if this other object has a carbomb weapon set that isn't in use.
	WeaponSetFlags flags;
	flags.set( WEAPONSET_CARBOMB );
	const WeaponTemplateSet* set = other->getTemplate()->findWeaponTemplateSet( flags );
	if( !set )
	{
		//This unit has no weapon set!
		return FALSE;
	}
	if( !set->testWeaponSetFlag( WEAPONSET_CARBOMB ) )
	{
		//This unit has a weaponset, but the best match code above chose a different
		//weaponset.
		return FALSE;
	}

	// Also make sure that the car isn't already a carbomb!
	if( other->testWeaponSetFlag( WEAPONSET_CARBOMB ) )
	{
		return FALSE;
	}

	return TRUE;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Bool ConvertToCarBombCrateCollide::executeCrateBehavior( Object *other )
{
	//Check to make sure that the other object is also the goal object in the AIUpdateInterface
	//in order to prevent an unintentional conversion simply by having the terrorist walk too close
	//to it.
	//Assume ai is valid because CrateCollide::isValidToExecute(other) checks it.
	Object *obj = getObject();
	AIUpdateInterface* ai = obj->getAIUpdateInterface();
	if (ai && ai->getGoalObject() != other)
		return false;

	if( other && other->checkAndDetonateBoobyTrap(obj) )
	{
		// Whoops, it was mined.  Cancel if it or us is now dead.
		if( other->isEffectivelyDead() || getObject()->isEffectivelyDead() )
		{
			return false;
		}
	}

	other->setWeaponSetFlag( WEAPONSET_CARBOMB );

	FXList::doFXObj( getConvertToCarBombCrateCollideModuleData()->m_fxList, other );

	other->defect( getObject()->getControllingPlayer()->getDefaultTeam(), 0);

	//In order to make things easier for the designers, we are going to transfer the terrorist name
	//to the car... so the designer can control the car with their scripts.
	TheScriptEngine->transferObjectName( getObject()->getName(), other );

	//This is kinda special... we will endow our new ride with our vision and shroud range, since we are driving
	other->setVisionRange(getObject()->getVisionRange());
	other->setShroudClearingRange(getObject()->getShroudClearingRange());
	other->setStatus( MAKE_OBJECT_STATUS_MASK( OBJECT_STATUS_IS_CARBOMB ) );

	ExperienceTracker *exp = other->getExperienceTracker();
	if (exp)
	{
		exp->setVeterancyLevel(obj->getExperienceTracker()->getVeterancyLevel());
	}



	TheRadar->removeObject( other );
	TheRadar->addObject( other );




	return TRUE;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void ConvertToCarBombCrateCollide::crc( Xfer *xfer )
{

	// extend base class
	CrateCollide::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void ConvertToCarBombCrateCollide::xfer( Xfer *xfer )
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
void ConvertToCarBombCrateCollide::loadPostProcess( void )
{

	// extend base class
	CrateCollide::loadPostProcess();

}
