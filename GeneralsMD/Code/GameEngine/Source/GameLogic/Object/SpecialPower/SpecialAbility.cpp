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

// FILE: SpecialAbility.cpp /////////////////////////////////////////////////////////////
// Author: Kris Morness, July 2002
// Desc:   This is the class that handles processing of any special attack from a unit. There are 
//         many different styles and rules for various attacks.
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/Player.h"
#include "Common/Xfer.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/SpecialAbility.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
SpecialAbility::SpecialAbility( Thing *thing, const ModuleData *moduleData )
												: SpecialPowerModule( thing, moduleData )
{

}  // end SpecialAbility

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
SpecialAbility::~SpecialAbility( void )
{

}  // end ~SpecialAbility

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void SpecialAbility::doSpecialPowerAtLocation( const Coord3D *loc, Real angle, UnsignedInt commandOptions )
{
	if (getObject()->isDisabled())
		return;

	// sanity
	if( loc == NULL )
		return;

	// call the base class action cause we are *EXTENDING* functionality
	SpecialPowerModule::doSpecialPowerAtLocation( loc, angle, commandOptions );
}  

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void SpecialAbility::doSpecialPowerAtObject( Object *obj, UnsignedInt commandOptions )
{
	if (getObject()->isDisabled())
		return;

	if (!obj)
		return;

	// call the base class action cause we are *EXTENDING* functionality
	SpecialPowerModule::doSpecialPowerAtObject( obj, commandOptions );
}  


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void SpecialAbility::doSpecialPower( UnsignedInt commandOptions )
{
	if (getObject()->isDisabled())
		return;

	// call the base class action cause we are *EXTENDING* functionality
	SpecialPowerModule::doSpecialPower( commandOptions );
}  

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void SpecialAbility::crc( Xfer *xfer )
{

	// extend base class
	SpecialPowerModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void SpecialAbility::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	SpecialPowerModule::xfer( xfer );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void SpecialAbility::loadPostProcess( void )
{

	// extend base class
	SpecialPowerModule::loadPostProcess();

}  // end loadPostProcess
