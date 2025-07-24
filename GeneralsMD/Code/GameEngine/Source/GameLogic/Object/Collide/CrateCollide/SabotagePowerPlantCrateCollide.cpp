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
// FILE: SabotagePowerPlantCrateCollide.cpp 
// Author: Kris Morness, June 2003
// Desc:   A crate (actually a saboteur - mobile crate) that makes the target powerplant lose power
//	
///////////////////////////////////////////////////////////////////////////////////////////////////
 


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/GameAudio.h"
#include "Common/MiscAudio.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/Radar.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"

#include "GameClient/Drawable.h"
#include "GameClient/Eva.h"
#include "GameClient/InGameUI.h"  // useful for printing quick debug strings when we need to

#include "GameLogic/ExperienceTracker.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/SabotagePowerPlantCrateCollide.h"
#include "GameLogic/Module/HijackerUpdate.h"
#include "GameLogic/Module/ContainModule.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/ScriptEngine.h"
#include "GameLogic/Module/DozerAIUpdate.h"


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
SabotagePowerPlantCrateCollide::SabotagePowerPlantCrateCollide( Thing *thing, const ModuleData* moduleData ) : CrateCollide( thing, moduleData )
{
} 

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
SabotagePowerPlantCrateCollide::~SabotagePowerPlantCrateCollide( void )
{
}  

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Bool SabotagePowerPlantCrateCollide::isValidToExecute( const Object *other ) const
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

	if( !other->isKindOf( KINDOF_FS_POWER ) )
	{
		//We can only sabotage power plants.
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
Bool SabotagePowerPlantCrateCollide::executeCrateBehavior( Object *other )
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

  doSabotageFeedbackFX( other, CrateCollide::SAB_VICTIM_POWER_PLANT );

	//When the sabotage occurs, play the appropriate EVA
	//event if the local player is the victim!
	if( other->isLocallyControlled() )
	{
		TheEva->setShouldPlay( EVA_BuildingSabotaged );
	}

	Player *player = other->getControllingPlayer();
	if( player )
	{
		//Set the duration inside the player's energy class to record the length of the power outage.
		UnsignedInt frame = TheGameLogic->getFrame() + getSabotagePowerPlantCrateCollideModuleData()->m_powerSabotageFrames;
		player->getEnergy()->setPowerSabotagedTillFrame( frame );

		//Trigger the callback function that will turn everything off.
		player->onPowerBrownOutChange( TRUE );

		//Note: Player::update() will check to turn it back on again once the timer expires.
	}
	
	return TRUE;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void SabotagePowerPlantCrateCollide::crc( Xfer *xfer )
{

	// extend base class
	CrateCollide::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void SabotagePowerPlantCrateCollide::xfer( Xfer *xfer )
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
void SabotagePowerPlantCrateCollide::loadPostProcess( void )
{

	// extend base class
	CrateCollide::loadPostProcess();

}  // end loadPostProcess
