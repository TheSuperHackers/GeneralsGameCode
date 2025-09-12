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

// FILE: ImmortalBody.cpp ////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, April 2002
// Desc:	 Just like Active Body, but won't let health drop below 1
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#include "Common/Xfer.h"
#include "GameLogic/Object.h"
#include "GameLogic/Damage.h"
#include "GameLogic/Module/ImmortalBody.h"

// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ImmortalBody::ImmortalBody( Thing *thing, const ModuleData* moduleData )
						 : ActiveBody( thing, moduleData )
{
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ImmortalBody::~ImmortalBody( void )
{

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void ImmortalBody::internalChangeHealth( Real delta )
{
	// Don't let anything changes us to below one hit point
	delta = max( delta, -getHealth() + 1 );

	// extend functionality, but I go first because I can't let you die and then fix it, I must prevent
	ActiveBody::internalChangeHealth( delta );

	// nothing -- never mark it as dead.
	DEBUG_ASSERTCRASH( (getHealth() > 0 && !getObject()->isEffectivelyDead() ), ("Immortal objects should never get marked as dead!"));

}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void ImmortalBody::crc( Xfer *xfer )
{

	// extend base class
	ActiveBody::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void ImmortalBody::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	ActiveBody::xfer( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void ImmortalBody::loadPostProcess( void )
{

	// extend base class
	ActiveBody::loadPostProcess();

}
