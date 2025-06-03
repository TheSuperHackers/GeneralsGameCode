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

///////////////////////////////////////////////////////////////////////////////////////////////////
//	
// FILE: SabotageSuperweaponCrateCollide.cpp 
// Author: Kris Morness, June 2003
// Desc:   A crate (actually a saboteur - mobile crate) that resets the timer on the target supply dropzone.
//	
///////////////////////////////////////////////////////////////////////////////////////////////////
 


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/GameAudio.h"
#include "Common/MiscAudio.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/Radar.h"
#include "Common/SpecialPower.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"

#include "GameClient/Drawable.h"
#include "GameClient/Eva.h"
#include "GameClient/InGameUI.h"  // useful for printing quick debug strings when we need to

#include "GameLogic/ExperienceTracker.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/ScriptEngine.h"

#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/ContainModule.h"
#include "GameLogic/Module/DozerAIUpdate.h"
#include "GameLogic/Module/HijackerUpdate.h"
#include "GameLogic/Module/OCLUpdate.h"
#include "GameLogic/Module/SabotageSuperweaponCrateCollide.h"
#include "GameLogic/Module/SpecialPowerModule.h"


#ifdef RTS_INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
SabotageSuperweaponCrateCollide::SabotageSuperweaponCrateCollide( Thing *thing, const ModuleData* moduleData ) : CrateCollide( thing, moduleData )
{
} 

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
SabotageSuperweaponCrateCollide::~SabotageSuperweaponCrateCollide( void )
{
}  

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Bool SabotageSuperweaponCrateCollide::isValidToExecute( const Object *other ) const
{
	if( !CrateCollide::isValidToExecute(other) )
	{
		//Extend functionality.
		return FALSE;
	}

	if( other->isEffectivelyDead() )
	{
		//Can't sabotage dead structures
		return FALSE;
	}

	if( !other->isKindOf( KINDOF_FS_SUPERWEAPON ) && !other->isKindOf( KINDOF_FS_STRATEGY_CENTER ) )
	{
		//We can only sabotage superweapon structures.
		return FALSE;
	}

	Relationship r = getObject()->getRelationship( other );
	if( r != ENEMIES )
	{
		//Can only sabotage enemy buildings.
		return FALSE;
	}

	return TRUE;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Bool SabotageSuperweaponCrateCollide::executeCrateBehavior( Object *other )
{
	//Check to make sure that the other object is also the goal object in the AIUpdateInterface
	//in order to prevent an unintentional conversion simply by having the terrorist walk too close
	//to it.
	//Assume ai is valid because CrateCollide::isValidToExecute(other) checks it.
	Object *obj = getObject();
	AIUpdateInterface* ai = obj->getAIUpdateInterface();
	if (ai && ai->getGoalObject() != other)
	{
		return false;
	}

	TheRadar->tryInfiltrationEvent( other );

  doSabotageFeedbackFX( other, CrateCollide::SAB_VICTIM_SUPERWEAPON );

	//When the sabotage occurs, play the appropriate EVA
	//event if the local player is the victim!
	if( other->isLocallyControlled() )
	{
		TheEva->setShouldPlay( EVA_BuildingSabotaged );
	}

	//Reset ALL special powers!
	for( BehaviorModule **m = other->getBehaviorModules(); *m; ++m )
	{
		SpecialPowerModuleInterface* sp = (*m)->getSpecialPower();
		if( !sp )
		{
			continue;
		}
		sp->startPowerRecharge();
	}

	return TRUE;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void SabotageSuperweaponCrateCollide::crc( Xfer *xfer )
{

	// extend base class
	CrateCollide::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void SabotageSuperweaponCrateCollide::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	CrateCollide::xfer( xfer );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void SabotageSuperweaponCrateCollide::loadPostProcess( void )
{

	// extend base class
	CrateCollide::loadPostProcess();

}  // end loadPostProcess
