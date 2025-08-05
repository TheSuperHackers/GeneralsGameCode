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

// FILE: NeutronBlastBehavior.cpp ///////////////////////////////////////////////////////////////////////
// Author: Daniel Teh
///////////////////////////////////////////////////////////////////////////////////////////////////


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#include "GameLogic/Module/NeutronBlastBehavior.h"

#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "GameLogic/Module/ContainModule.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameClient/Drawable.h"


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
NeutronBlastBehavior::NeutronBlastBehavior( Thing *thing, const ModuleData* moduleData ) : UpdateModule( thing, moduleData )
{
	setWakeFrame( getObject(), UPDATE_SLEEP_FOREVER );
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
NeutronBlastBehavior::~NeutronBlastBehavior()
{
	// GAME STUFF DOES NOT GO IN THE DESTRUCTOR
	// (Crash if end game with Neutron shell in air)
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void NeutronBlastBehavior::onDie( const DamageInfo *damageInfo )
{
	// On death, perform the Neutron Blast!!
	Object *self = getObject();
	if (!self)
		return;

	const NeutronBlastBehaviorModuleData *data = getNeutronBlastBehaviorModuleData();
	Real blastRadius = data->m_blastRadius;
	Bool hitAir = data->m_isAffectAirborne;

	// setup scan filters
	PartitionFilterSameMapStatus filterMapStatus( self );
	PartitionFilterAlive filterAlive;
	PartitionFilter *filters[] = { &filterAlive, &filterMapStatus, NULL };

	// scan objects in our region
	ObjectIterator *iter = ThePartitionManager->iterateObjectsInRange( self->getPosition(), blastRadius, FROM_CENTER_2D, filters );
	MemoryPoolObjectHolder hold( iter );

	// Apply neutron blast to object
	for( Object *obj = iter->first(); obj; obj = iter->next() )
	{
		if( hitAir  ||  ( !obj->isKindOf(KINDOF_AIRCRAFT) && !obj->isAirborneTarget() ) )
		{
			neutronBlastToObject( obj );
		}
	}
}



//-------------------------------------------------------------------------------------------------
/** The update callback. */
//-------------------------------------------------------------------------------------------------
UpdateSleepTime NeutronBlastBehavior::update( void )
{
	return UPDATE_SLEEP_FOREVER;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void NeutronBlastBehavior::neutronBlastToObject( Object *obj )
{
	// early exit check
  if ( !obj || obj == getObject() )
		return;

	// Check for allies and quick exit if we are not suppose to hurt our own.
	const NeutronBlastBehaviorModuleData *data = getNeutronBlastBehaviorModuleData();
	if (!data->m_affectAllies && getObject()->getRelationship( obj ) == ALLIES)
	{
		return;
	}

	// Kill if object is infantry
	if (obj->isKindOf(KINDOF_INFANTRY))
	{
		obj->kill();
	}

	// Kill all contained if it is a container
	ContainModuleInterface *contain = obj->getContain();
	if( contain )
	{
		contain->killAllContained();
	}

	// Kill pilots of vehicles
	if( obj->isKindOf( KINDOF_VEHICLE ) && !obj->isKindOf( KINDOF_DRONE ) )
	{
		// If the vehicle is a combat bike, kill the whole thing
		if ( obj->isKindOf( KINDOF_CLIFF_JUMPER ) )
		{
			obj->kill();
		}
		// Just kill the pilot of the vehicle
		else
		{
			// Make it unmanned, so units can easily check the ability to "take control of it"
			obj->setDisabled( DISABLED_UNMANNED );

      if ( obj->getAI() )
        obj->getAI()->aiIdle( CMD_FROM_AI );

			TheGameLogic->deselectObject(obj, PLAYERMASK_ALL, TRUE);

			// Clear any terrain decals here
			Drawable* draw = obj->getDrawable();
			if (draw)
				draw->setTerrainDecal(TERRAIN_DECAL_NONE);

			// Convert it to the neutral team so it renders gray giving visual representation that it is unmanned.
			obj->setTeam( ThePlayerList->getNeutralPlayer()->getDefaultTeam() );
		}
	}
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void NeutronBlastBehavior::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );


}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void NeutronBlastBehavior::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void NeutronBlastBehavior::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();


}  // end loadPostProcess
