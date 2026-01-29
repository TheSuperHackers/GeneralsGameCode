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

// FILE: RailedTransportContain.cpp ///////////////////////////////////////////////////////////////
// Author: Colin Day, August 2002
// Desc: Railed Transport Contain Module
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine
#include "Common/Xfer.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/RailedTransportContain.h"
#include "GameLogic/Module/RailedTransportDockUpdate.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
static RailedTransportDockUpdateInterface *getRailedTransportDockUpdateInterface( Object *obj )
{

	// sanity
	if( obj == nullptr )
		return nullptr;

	// find us our dock interface
	RailedTransportDockUpdateInterface *rtdui = nullptr;
	for( BehaviorModule **u = obj->getBehaviorModules(); *u; ++u )
		if( (rtdui = (*u)->getRailedTransportDockUpdateInterface()) != nullptr )
			break;

	return rtdui;

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
RailedTransportContain::RailedTransportContain( Thing *thing, const ModuleData *moduleData )
											: TransportContain( thing, moduleData )
{

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
RailedTransportContain::~RailedTransportContain( void )
{

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void RailedTransportContain::onRemoving( Object *obj )
{

	// extend functionality
	TransportContain::onRemoving( obj );

	// once we have removed all our contents we are "open" for docking again for more transportation
	if( getContainCount() == 0 )
	{
		DockUpdateInterface *dui = getObject()->getDockUpdateInterface();
		if( dui )
			dui->setDockOpen( TRUE );

	}

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
Bool RailedTransportContain::isSpecificRiderFreeToExit( Object *obj )
{
	Object *us = getObject();

	// if we are in transit we cannot exit, when we're in transit, the dock is closed
	DockUpdateInterface *dui = us->getDockUpdateInterface();
	if( dui && dui->isDockOpen() == FALSE )
		return FALSE;

	// we can now exit, note we're not extending the base class cause *we* handle it all
	return TRUE;

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void RailedTransportContain::exitObjectViaDoor( Object *newObj, ExitDoorType exitDoor )
{
	RailedTransportDockUpdateInterface *rtdui = getRailedTransportDockUpdateInterface();

	if( rtdui == nullptr )
		return;

	// tell the railed dock to exit ONE object, this one
	rtdui->unloadSingleObject( newObj );

}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void RailedTransportContain::crc( Xfer *xfer )
{

	// extend base class
	TransportContain::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void RailedTransportContain::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	TransportContain::xfer( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void RailedTransportContain::loadPostProcess( void )
{

	// extend base class
	TransportContain::loadPostProcess();

}
