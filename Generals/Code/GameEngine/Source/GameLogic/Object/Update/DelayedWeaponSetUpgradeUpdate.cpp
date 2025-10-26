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

// FILE: DelayedWeaponSetUpgradeUpdate.cpp ////////////////////////////////////////////////////////////////////////
// Author: Will act like an WeaponSet UpgradeModule, but after a delay.
// Desc:   Graham Smallwood, April 2002
///////////////////////////////////////////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/Xfer.h"
#include "GameLogic/Module/DelayedWeaponSetUpgradeUpdate.h"

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
DelayedWeaponSetUpgradeUpdateModuleData::DelayedWeaponSetUpgradeUpdateModuleData( void )
{
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void DelayedWeaponSetUpgradeUpdateModuleData::buildFieldParse(MultiIniFieldParse& p)
{

  UpdateModuleData::buildFieldParse( p );

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
DelayedWeaponSetUpgradeUpdate::DelayedWeaponSetUpgradeUpdate( Thing *thing, const ModuleData* moduleData )
																: UpdateModule( thing, moduleData )
{


}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
DelayedWeaponSetUpgradeUpdate::~DelayedWeaponSetUpgradeUpdate( void )
{

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UpdateSleepTime DelayedWeaponSetUpgradeUpdate::update( void )
{
	return UPDATE_SLEEP_NONE;
}

Bool DelayedWeaponSetUpgradeUpdate::isTriggeredBy( UpgradeMaskType potentialMask )
{
	potentialMask;
	return FALSE;
}

void DelayedWeaponSetUpgradeUpdate::setDelay( UnsignedInt startingDelay )
{
	startingDelay;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void DelayedWeaponSetUpgradeUpdate::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void DelayedWeaponSetUpgradeUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void DelayedWeaponSetUpgradeUpdate::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}
