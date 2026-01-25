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

// FILE: UnitCrateCollide.cpp ///////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, March 2002
// Desc:   A crate that gives n units of type m the the pickerupper
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine
#include "Common/AudioEventRTS.h"
#include "Common/MiscAudio.h"
#include "Common/Player.h"
#include "Common/ThingFactory.h"
#include "Common/Xfer.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/Module/UnitCrateCollide.h"

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UnitCrateCollide::UnitCrateCollide( Thing *thing, const ModuleData* moduleData ) : CrateCollide( thing, moduleData )
{

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UnitCrateCollide::~UnitCrateCollide( void )
{

}

//-------------------------------------------------------------------------------------------------
Bool UnitCrateCollide::executeCrateBehavior( Object *other )
{
	UnsignedInt unitCount = getUnitCrateCollideModuleData()->m_unitCount;
	ThingTemplate const *unitType = TheThingFactory->findTemplate( getUnitCrateCollideModuleData()->m_unitType );

	if( unitType == nullptr )
	{
		return FALSE;
	}

	for( UnsignedInt unitIndex = 0; unitIndex < unitCount; unitIndex++ )
	{
		Team *creationTeam = other->getControllingPlayer()->getDefaultTeam();
		Object *newObj = TheThingFactory->newObject( unitType, creationTeam );
		if( newObj )
		{
			Coord3D creationPoint = *other->getPosition();
			/// @todo As a user of the future findLegalPositionAround, I wouldn't mind not having to specify range.  I just want a non colliding point.
			FindPositionOptions fpOptions;
			fpOptions.minRadius = 0.0f;
			fpOptions.maxRadius = 20.0f;
			ThePartitionManager->findPositionAround( &creationPoint,
																							 &fpOptions,
																							 &creationPoint );

			newObj->setOrientation( other->getOrientation() );
			newObj->setPosition( &creationPoint );
		}
	}

	//Play a crate pickup sound.
	AudioEventRTS soundToPlay = TheAudio->getMiscAudio()->m_crateFreeUnit;
	soundToPlay.setObjectID( other->getID() );
	TheAudio->addAudioEvent(&soundToPlay);

	return TRUE;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void UnitCrateCollide::crc( Xfer *xfer )
{

	// extend base class
	CrateCollide::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void UnitCrateCollide::xfer( Xfer *xfer )
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
void UnitCrateCollide::loadPostProcess( void )
{

	// extend base class
	CrateCollide::loadPostProcess();

}
