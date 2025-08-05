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

// FILE: DestroyModule.cpp ////////////////////////////////////////////////////////////////////////
// Author: Colin Day, October 2002
// Desc:   Destroy module base class
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"
#include "Common/Xfer.h"
#include "GameLogic/Module/DestroyModule.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
DestroyModule::DestroyModule( Thing *thing, const ModuleData* moduleData )
							: BehaviorModule( thing, moduleData )
{

}  // end DestroyModule

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
DestroyModule::~DestroyModule( void )
{

}  // end ~DestroyModule

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void DestroyModule::crc( Xfer *xfer )
{

	// extend base class
	BehaviorModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void DestroyModule::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	BehaviorModule::xfer( xfer );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void DestroyModule::loadPostProcess( void )
{

	// extend base class
	BehaviorModule::loadPostProcess();

}  // end loadPostProcess

