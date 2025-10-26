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

// FILE: POWTruckBehavior.cpp /////////////////////////////////////////////////////////////////////
// Author: Colin Day
// Desc:   POW Truck
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"
#include "GameClient/InGameUI.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/POWTruckAIUpdate.h"
#include "GameLogic/Module/POWTruckBehavior.h"

#ifdef ALLOW_SURRENDER

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
POWTruckBehaviorModuleData::POWTruckBehaviorModuleData( void )
{

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
/*static*/ void POWTruckBehaviorModuleData::buildFieldParse( MultiIniFieldParse &p )
{
  OpenContainModuleData::buildFieldParse( p );

	static const FieldParse dataFieldParse[] =
	{

		{ 0, 0, 0, 0 }

	};

  p.add(dataFieldParse);

}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
POWTruckBehavior::POWTruckBehavior( Thing *thing, const ModuleData *moduleData )
								: OpenContain( thing, moduleData )
{

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
POWTruckBehavior::~POWTruckBehavior( void )
{

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void POWTruckBehavior::onCollide( Object *other, const Coord3D *loc, const Coord3D *normal )
{
	Object *us = getObject();

	// sanity
	if( other == NULL )
		return;

	// if other isn't slated to be picked up by us, ignore
	AIUpdateInterface *otherAi = other->getAIUpdateInterface();
	if( otherAi == NULL || otherAi->isSurrendered() == FALSE )
		return;

	// get our AI info
	AIUpdateInterface *ourAI = us->getAIUpdateInterface();
	DEBUG_ASSERTCRASH( ourAI, ("POWTruckBehavior::onCollide - '%s' has no AI",
														us->getTemplate()->getName().str()) );
	POWTruckAIUpdateInterface *powTruckAI = ourAI->getPOWTruckAIUpdateInterface();
	DEBUG_ASSERTCRASH( powTruckAI, ("POWTruckBehavior::onCollide - '%s' has no POWTruckAI",
																 us->getTemplate()->getName().str()) );

	// pick up the prisoner
	powTruckAI->loadPrisoner( other );

}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void POWTruckBehavior::crc( Xfer *xfer )
{

	// extend base class
	OpenContain::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void POWTruckBehavior::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	OpenContain::xfer( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void POWTruckBehavior::loadPostProcess( void )
{

	// extend base class
	OpenContain::loadPostProcess();

}

#endif
