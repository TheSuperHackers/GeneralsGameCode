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

// FILE: RadarUpgrade.cpp /////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, March 2002
// Desc:	 Radar upgrades
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/ModelState.h"
#include "Common/Player.h"
#include "Common/Xfer.h"
#include "GameClient/Drawable.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/RadarUpdate.h"
#include "GameLogic/Module/RadarUpgrade.h"

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void RadarUpgradeModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  UpgradeModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "DisableProof",	INI::parseBool,	NULL, offsetof( RadarUpgradeModuleData, m_isDisableProof ) },
		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
RadarUpgrade::RadarUpgrade( Thing *thing, const ModuleData* moduleData ) :
							UpgradeModule( thing, moduleData )
{

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
RadarUpgrade::~RadarUpgrade( void )
{

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void RadarUpgrade::onDelete( void )
{
	const RadarUpgradeModuleData *md = getRadarUpgradeModuleData();

	// if we haven't been upgraded there is nothing to clean up
	if( isAlreadyUpgraded() == FALSE )
		return;

	// If we're currently disabled, we shouldn't do anything, because we've already done it.
	if ( getObject()->isDisabled() )
		return;

	// remove the radar from the player
	Player *player = getObject()->getControllingPlayer();
	if( player )
		player->removeRadar( md->m_isDisableProof );

	// this upgrade module is now "not upgraded"
	setUpgradeExecuted(FALSE);

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void RadarUpgrade::onCapture( Player *oldOwner, Player *newOwner )
{
	const RadarUpgradeModuleData *md = getRadarUpgradeModuleData();

	// do nothing if we haven't upgraded yet
	if( isAlreadyUpgraded() == FALSE )
		return;

	// If we're currently disabled, we shouldn't do anything, because we've already done it.
	if ( getObject()->isDisabled() )
		return;

	// remove radar from old player and add to new player
	if( oldOwner )
	{

		oldOwner->removeRadar( md->m_isDisableProof );
		setUpgradeExecuted(FALSE);

	}
	if( newOwner )
	{

		newOwner->addRadar( md->m_isDisableProof );
		setUpgradeExecuted(TRUE);

	}

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void RadarUpgrade::upgradeImplementation( void )
{
	const RadarUpgradeModuleData *md = getRadarUpgradeModuleData();

	Player *player = getObject()->getControllingPlayer();

	// update the player with another radar facility
	player->addRadar( md->m_isDisableProof );

	// find the radar update module of this object
	NameKeyType radarUpdateKey = NAMEKEY( "RadarUpdate" );
	RadarUpdate *radarUpdate = (RadarUpdate *)getObject()->findUpdateModule( radarUpdateKey );
	if( radarUpdate )
		radarUpdate->extendRadar();

}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void RadarUpgrade::crc( Xfer *xfer )
{

	// extend base class
	UpgradeModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void RadarUpgrade::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpgradeModule::xfer( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void RadarUpgrade::loadPostProcess( void )
{

	// extend base class
	UpgradeModule::loadPostProcess();

}
