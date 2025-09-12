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

// FILE: HighlanderBody.cpp ////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, November 2002
// Desc:	 Takes damage according to armor, but can't die from normal damage.  Can die from Unresistable though
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#include "Common/Xfer.h"

#include "GameLogic/Module/HighlanderBody.h"

// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
HighlanderBody::HighlanderBody( Thing *thing, const ModuleData* moduleData )
						 : ActiveBody( thing, moduleData )
{
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
HighlanderBody::~HighlanderBody( void )
{

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void HighlanderBody::attemptDamage( DamageInfo *damageInfo )
{
	// Bind to one hitpoint remaining afterwards, unless it is Unresistable damage
	if( damageInfo->in.m_damageType != DAMAGE_UNRESISTABLE )
		damageInfo->in.m_amount = min( damageInfo->in.m_amount, getHealth() - 1 );

	ActiveBody::attemptDamage(damageInfo);
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void HighlanderBody::crc( Xfer *xfer )
{

	// extend base class
	ActiveBody::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void HighlanderBody::xfer( Xfer *xfer )
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
void HighlanderBody::loadPostProcess( void )
{

	// extend base class
	ActiveBody::loadPostProcess();

}
