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

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// FILE: HijackerUpdate.cpp
// Author: Mark Lorenzen, July 2002
// Desc:   Allows hijacker to kepp with his hijacked vehicle (though hidden) until it dies, then
// to become a hijacker once more
//
/////////////////////////////////////////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Player.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"
#include "GameClient/Drawable.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/HijackerUpdate.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/Module/EjectPilotDie.h"
#include "GameLogic/Module/ContainModule.h"
#include "GameLogic/ExperienceTracker.h"

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
HijackerUpdate::HijackerUpdate( Thing *thing, const ModuleData *moduleData ) : UpdateModule( thing, moduleData )
{
	m_targetID = INVALID_ID;
	setUpdate( FALSE );
	setIsInVehicle( FALSE );
	m_wasTargetAirborne = false;
	m_ejectPos.zero();
//	m_ejectPilotDMI = nullptr;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
HijackerUpdate::~HijackerUpdate( void )
{
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UpdateSleepTime HijackerUpdate::update( void )
{
/// @todo srj use SLEEPY_UPDATE here

	if ( ! m_update) // have not flagged for updating
	{
		return UPDATE_SLEEP_NONE;
	}

	if (m_isInVehicle)
	{
		Object *obj = getObject();

		// If hijacker has hijacked a vehicle, he needs to move along with it...
		//Continually reset position of hijacker to match the position of the target.
		Object *target = getTargetObject();
		if( target )
		{
			// @todo I think we should test for ! IsEffectivelyDead() as well, here
			obj->setPosition( target->getPosition() );
			m_wasTargetAirborne = target->isSignificantlyAboveTerrain();
			m_ejectPos = *target->getPosition();

			// So, if while I am driving this American war vehicle, I gain skill points, I get to keep them when I wreck the vehicle
			ExperienceTracker *targetExp = target->getExperienceTracker();
			ExperienceTracker *jackerExp = obj->getExperienceTracker();
			if ( targetExp && jackerExp )
			{
				VeterancyLevel highestLevel = MAX(targetExp->getVeterancyLevel(),jackerExp->getVeterancyLevel());
				jackerExp->setVeterancyLevel( highestLevel );
				targetExp->setVeterancyLevel( highestLevel );
			}

		}
		else // the car we have been "driving" is dead now, and has safely ejected us
		{
			{


				//THIS BLOCK RESTORES HIJACKER TO PARTITION MANAGER AND UNHIDES HIM
				ThePartitionManager->registerObject( obj );

				if( obj->getDrawable() )
				{
					// so it is time to unhide ourselves and be a pedestrian hijacker again
					obj->getDrawable()->setDrawableHidden( false );
				}

				// We won't come back here until and unless we have hijacked another vehicle
				obj->clearStatus( MAKE_OBJECT_STATUS_MASK3( OBJECT_STATUS_NO_COLLISIONS, OBJECT_STATUS_MASKED, OBJECT_STATUS_UNSELECTABLE ) );

				AIUpdateInterface* ai = obj->getAIUpdateInterface();
				if ( ai )
				{
					ai->aiIdle( CMD_FROM_AI );
				}

				if (m_wasTargetAirborne)
				{
					const ThingTemplate* putInContainerTmpl = TheThingFactory->findTemplate(getHijackerUpdateModuleData()->m_parachuteName);
					DEBUG_ASSERTCRASH(putInContainerTmpl,("DeliverPayload: PutInContainer %s not found!",getHijackerUpdateModuleData()->m_parachuteName.str()));
					if (putInContainerTmpl)
					{
						Object* container = TheThingFactory->newObject( putInContainerTmpl, obj->getTeam() );
						container->setPosition(&m_ejectPos);
						if (container->getContain()->isValidContainerFor(obj, true))
						{
							container->getContain()->addToContain(obj);
						}
						else
						{
							DEBUG_CRASH(("DeliverPayload: PutInContainer %s is full, or not valid for the payload!",getHijackerUpdateModuleData()->m_parachuteName.str()));
						}
					}

				}


			}

			setTargetObject( nullptr );
			setIsInVehicle( FALSE );
			setUpdate( FALSE );
			m_wasTargetAirborne = false;

		}

	}
	else	// not in vehicle
	{
		m_wasTargetAirborne = false;
	}

	return UPDATE_SLEEP_NONE;
}



void HijackerUpdate::setTargetObject( const Object *object )
{
  if( object )
  {
    m_targetID = object->getID();

		// here we also test the target to see whether it ejects pilots
		// when it dies... if so, stores a pointer to that diemoduleinterface
		// nullptr if not...

//		BehaviorModule **dmi = nullptr;
//		for( dmi = object->getBehaviorModules(); *dmi; ++dmi )
//		{
//			m_ejectPilotDMI = (*dmi)->getEjectPilotDieInterface();
//			if( m_ejectPilotDMI )
//				return;
//		}  // end for dmi
  }
	else
	{
		m_targetID = INVALID_ID;
//		m_ejectPilotDMI = nullptr;
	}

}

Object* HijackerUpdate::getTargetObject() const
{
  if( m_targetID != INVALID_ID )
  {
    return TheGameLogic->findObjectByID( m_targetID );
  }
  return nullptr;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void HijackerUpdate::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void HijackerUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// target ID
	xfer->xferObjectID( &m_targetID );

	// eject pos
	xfer->xferCoord3D( &m_ejectPos );

	// udpate
	xfer->xferBool( &m_update );

	// is in vehicle
	xfer->xferBool( &m_isInVehicle );

	// was target airborne
	xfer->xferBool( &m_wasTargetAirborne );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void HijackerUpdate::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

	// set the target object, this will also tie up the m_ejectPilotDMI pointer
	Object *obj = TheGameLogic->findObjectByID( m_targetID );
	setTargetObject( obj );

}
