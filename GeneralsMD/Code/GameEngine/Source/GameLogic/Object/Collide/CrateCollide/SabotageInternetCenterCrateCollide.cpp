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
// FILE: SabotageInternetCenterCrateCollide.cpp 
// Author: Kris Morness, July 2003
// Desc:   A crate (actually a saboteur - mobile crate) that temporarily disables an internet center
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
#include "GameClient/GameText.h"
#include "GameClient/InGameUI.h"  // useful for printing quick debug strings when we need to

#include "GameLogic/ExperienceTracker.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/ScriptEngine.h"

#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/BehaviorModule.h"
#include "GameLogic/Module/ContainModule.h"
#include "GameLogic/Module/DozerAIUpdate.h"
#include "GameLogic/Module/HijackerUpdate.h"
#include "GameLogic/Module/OCLUpdate.h"
#include "GameLogic/Module/SabotageInternetCenterCrateCollide.h"
#include "GameLogic/Module/SpyVisionUpdate.h"



//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
SabotageInternetCenterCrateCollide::SabotageInternetCenterCrateCollide( Thing *thing, const ModuleData* moduleData ) : CrateCollide( thing, moduleData )
{
} 

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
SabotageInternetCenterCrateCollide::~SabotageInternetCenterCrateCollide( void )
{
}  

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Bool SabotageInternetCenterCrateCollide::isValidToExecute( const Object *other ) const
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

	if( !other->isKindOf( KINDOF_FS_INTERNET_CENTER ) )
	{
		//We can only sabotage supply dropzones.
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

static void disableHacker( Object *obj, void *userData )
{
	UnsignedInt frame = (UnsignedInt)userData;
	if( obj )
	{
		obj->setDisabledUntil( DISABLED_HACKED, frame );
	}
}

static void disableInternetCenterSpyVision( Object *obj, void *userData )
{
	if( obj && obj->isKindOf( KINDOF_FS_INTERNET_CENTER ) )
	{
		UnsignedInt frame = (UnsignedInt)userData;

		//Loop through all it's SpyVisionUpdates() and wake them all up so they can be shut down. This is weird because
		//it's one of the few update modules that is actually properly sleepified.
		for( BehaviorModule** u = obj->getBehaviorModules(); *u; ++u )
		{
			SpyVisionUpdate *svUpdate = (*u)->getSpyVisionUpdate();
			if( svUpdate )
			{
				//Turn off the vision temporarily. When it recovers from being disabled, the
				//timer will need to start over from scratch so it won't come on right away unless 
				//it's a permanent spy vision.
				svUpdate->setDisabledUntilFrame( frame );
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Bool SabotageInternetCenterCrateCollide::executeCrateBehavior( Object *other )
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

  doSabotageFeedbackFX( other, CrateCollide::SAB_VICTIM_INTERNET_CENTER );

	if( other->isLocallyControlled() )
	{
		TheEva->setShouldPlay( EVA_BuildingSabotaged );
	}

	//Loop through every internet center to temporarily disable the spy vision upgrades.
	UnsignedInt frame = TheGameLogic->getFrame() + getSabotageInternetCenterCrateCollideModuleData()->m_sabotageFrames;
	
	//Disable all internet center spy visions (they stack) without visually disabling the other centers.
	//Kris: Note -- Design has changed that we only can have one center at a time... logically, this code
	//doesn't need to change.
	// This loop goes before the Disabled_Hacked one since that will use the normal disabled code with its cool timers.
	// This loop is for the other centers, but it hits the main one too.
	other->getControllingPlayer()->iterateObjects( disableInternetCenterSpyVision, (void*)frame );

	//Disable the internet center. Note this is purely fluff... because the spy vision update will still run even
	//though we are disabling it. We have to disable the spyvision updates manually because other centers need to
	//be turned off too but without visually disabling them. Yikes!
	other->setDisabledUntil( DISABLED_HACKED, frame );

	//Disable all the hackers inside.
	ContainModuleInterface *contain = other->getContain();
	contain->iterateContained( disableHacker, (void*)frame, FALSE );
		
	return TRUE;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void SabotageInternetCenterCrateCollide::crc( Xfer *xfer )
{

	// extend base class
	CrateCollide::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void SabotageInternetCenterCrateCollide::xfer( Xfer *xfer )
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
void SabotageInternetCenterCrateCollide::loadPostProcess( void )
{

	// extend base class
	CrateCollide::loadPostProcess();

}  // end loadPostProcess
