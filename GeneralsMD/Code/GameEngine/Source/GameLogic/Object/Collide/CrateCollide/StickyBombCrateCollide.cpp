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
// FILE: StickyBombCrateCollide.cpp 
// Author: Kris Morness, June 2003
// Desc:   A crate (actually a saboteur - mobile crate) that resets the timer on the target supply dropzone.
//	
///////////////////////////////////////////////////////////////////////////////////////////////////
 


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

// #include "Common/GameAudio.h"
// #include "Common/MiscAudio.h"
#include "Common/Player.h"
// #include "Common/PlayerList.h"
#include "Common/Radar.h"
// #include "Common/SpecialPower.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"

// #include "GameClient/Drawable.h"
#include "GameClient/Eva.h"
// #include "GameClient/InGameUI.h"  // useful for printing quick debug strings when we need to

//#include "GameLogic/ExperienceTracker.h"
#include "GameLogic/Object.h"
//#include "GameLogic/PartitionManager.h"
//#include "GameLogic/ScriptEngine.h"

#include "GameLogic/Module/AIUpdate.h"
//#include "GameLogic/Module/ContainModule.h"
//#include "GameLogic/Module/DozerAIUpdate.h"
//#include "GameLogic/Module/HijackerUpdate.h"
//#include "GameLogic/Module/OCLUpdate.h"
#include "GameLogic/Module/StickyBombCrateCollide.h"
#include "GameLogic/Module/StickyBombUpdate.h"
//#include "GameLogic/Module/SpecialPowerModule.h"



//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
StickyBombCrateCollide::StickyBombCrateCollide( Thing *thing, const ModuleData* moduleData ) : CrateCollide( thing, moduleData )
{
} 

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
StickyBombCrateCollide::~StickyBombCrateCollide( void )
{
}  

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Bool StickyBombCrateCollide::isValidToExecute( const Object *other ) const
{
	if( !CrateCollide::isValidToExecute(other) )
	{
		if (other != NULL)
			DEBUG_LOG(("StickyBombCrateCollide::isValidToExecute >>> target '%s' Not Valid.", other->getTemplate()->getName().str()));
		else
			DEBUG_LOG(("StickyBombCrateCollide::isValidToExecute >>> collide with ground!"));
		//Extend functionality.
		return FALSE;
	}

	// Do we need anything here?
	DEBUG_LOG(("StickyBombCrateCollide::isValidToExecute >>> target '%s' Valid!", other->getTemplate()->getName().str()));
	return TRUE;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Bool StickyBombCrateCollide::executeCrateBehavior( Object *other )
{

	DEBUG_LOG(("StickyBombCrateCollide::executeCrateBehavior 0"));
	const StickyBombCrateCollideModuleData* md = getStickyBombCrateCollideModuleData();

	if (m_hasCollided && !md->m_allowMultiCollide)
		return FALSE;

	if (md->m_needsTarget) {
		//Check to make sure that the other object is also the goal object in the AIUpdateInterface
		//in order to prevent an unintentional conversion simply by having the terrorist walk too close
		//to it. This should also be valid for Projectiles hitting their intended targets
		//Assume ai is valid because CrateCollide::isValidToExecute(other) checks it.

		Object* obj = getObject();
		AIUpdateInterface* ai = obj->getAIUpdateInterface();
		if (ai) {
			if (ai->getGoalObject() != other)
			{
				return false;
			}
		}
		else {
			ProjectileUpdateInterface* pi = obj->getProjectileUpdateInterface();
			if (pi && pi->getTargetObject() != other)
			{
				return false;
			}
		}

	}

	if (md->m_showInfiltrationEvent)
		TheRadar->tryInfiltrationEvent( other );

	//Get the template of the special object
	const ThingTemplate* thingTemplate = TheThingFactory->findTemplate(md->m_stickyBombObjectName);
	if (thingTemplate)
	{
		//Create a new special object
		Object* stickyBombObject = TheThingFactory->newObject(thingTemplate, getObject()->getTeam());
		if (stickyBombObject)
		{
			static NameKeyType key_StickyBombUpdate = NAMEKEY("StickyBombUpdate");
			StickyBombUpdate* update = (StickyBombUpdate*)stickyBombObject->findUpdateModule(key_StickyBombUpdate);
			if (!update)
			{
				DEBUG_ASSERTCRASH(0,
					("Unit '%s' attempted to place %s on %s but the bomb requires a StickyBombUpdate module.",
						getObject()->getTemplate()->getName().str(),
						stickyBombObject->getTemplate()->getName().str(),
						other->getTemplate()->getName().str()));
				return TRUE;
			}
			//Setting the producer ID allows the sticky bomb update module to initialize
			//and setup timers, etc.
			update->initStickyBomb(other, getObject());
		}
	}

	m_hasCollided = TRUE;
	return TRUE;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void StickyBombCrateCollide::crc( Xfer *xfer )
{

	// extend base class
	CrateCollide::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void StickyBombCrateCollide::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	CrateCollide::xfer( xfer );

	xfer->xferBool(&m_hasCollided);

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void StickyBombCrateCollide::loadPostProcess( void )
{

	// extend base class
	CrateCollide::loadPostProcess();

}  // end loadPostProcess
