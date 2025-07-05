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

// FILE: PrisonDockUpdate.cpp /////////////////////////////////////////////////////////////////////
// Author: Colin Day, August 2002
// Desc:   Dock update for prison structures
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/ContainModule.h"
#include "GameLogic/Module/POWTruckAIUpdate.h"
#include "GameLogic/Module/PrisonDockUpdate.h"

#ifdef ALLOW_SURRENDER

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
PrisonDockUpdate::PrisonDockUpdate( Thing *thing, const ModuleData* moduleData )
								: DockUpdate( thing, moduleData )
{

}  // end PrisonDockUpdate

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
PrisonDockUpdate::~PrisonDockUpdate( void )
{

}  // end ~PrisonDockUpdate

// ------------------------------------------------------------------------------------------------
/** Do the action while docked
	* Return TRUE to continue the docking process
	* Return FALSE to complete the dockin process */
// ------------------------------------------------------------------------------------------------
Bool PrisonDockUpdate::action( Object *docker, Object *drone )
{

	// sanity
	if( docker == NULL )
		return FALSE;

	// if docker has no contents, do nothing and stop the dock process
	ContainModuleInterface *contain = docker->getContain();
	if( contain && contain->getContainCount() == 0 )
		return FALSE;

	// unload the prisoners from the docker into us
	AIUpdateInterface *ai = docker->getAIUpdateInterface();
	DEBUG_ASSERTCRASH( ai, ("'%s' docking with prison has no AI",
												 docker->getTemplate()->getName().str()) );
	POWTruckAIUpdateInterface *powAI = ai->getPOWTruckAIUpdateInterface();
	DEBUG_ASSERTCRASH( powAI, ("'s' docking with prison has no POW Truck AI",
														docker->getTemplate()->getName().str()) );

	powAI->unloadPrisonersToPrison( getObject() );

	// end docking
	return FALSE;

}  // end action

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void PrisonDockUpdate::crc( Xfer *xfer )
{

	// extend base class
	DockUpdate::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void PrisonDockUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	DockUpdate::xfer( xfer );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void PrisonDockUpdate::loadPostProcess( void )
{

	// extend base class
	DockUpdate::loadPostProcess();

}  // end loadPostProcess

#endif
