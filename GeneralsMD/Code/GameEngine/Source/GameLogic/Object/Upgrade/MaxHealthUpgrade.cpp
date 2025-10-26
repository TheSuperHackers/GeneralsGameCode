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

// FILE: MaxHealthUpgrade.cpp /////////////////////////////////////////////////////////////////////////////
// Author: Kris Morness, September 2002
// Desc:	 UpgradeModule that adds a scalar to the object's experience gain.
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#define DEFINE_MAXHEALTHCHANGETYPE_NAMES
#include "Common/Xfer.h"
#include "GameLogic/Object.h"
#include "GameLogic/ExperienceTracker.h"
#include "GameLogic/Module/MaxHealthUpgrade.h"
#include "GameLogic/Module/BodyModule.h"

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
MaxHealthUpgradeModuleData::MaxHealthUpgradeModuleData( void )
{
	m_addMaxHealth = 0.0f;
	m_maxHealthChangeType = SAME_CURRENTHEALTH;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void MaxHealthUpgradeModuleData::buildFieldParse(MultiIniFieldParse& p)
{

  UpgradeModuleData::buildFieldParse( p );

	static const FieldParse dataFieldParse[] =
	{
		{ "AddMaxHealth",					INI::parseReal,					NULL,										offsetof( MaxHealthUpgradeModuleData, m_addMaxHealth ) },
		{ "ChangeType",						INI::parseIndexList,		TheMaxHealthChangeTypeNames, offsetof( MaxHealthUpgradeModuleData, m_maxHealthChangeType ) },
		{ 0, 0, 0, 0 }
	};

  p.add(dataFieldParse);

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
MaxHealthUpgrade::MaxHealthUpgrade( Thing *thing, const ModuleData* moduleData ) : UpgradeModule( thing, moduleData )
{
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
MaxHealthUpgrade::~MaxHealthUpgrade( void )
{
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void MaxHealthUpgrade::upgradeImplementation( )
{
	const MaxHealthUpgradeModuleData *data = getMaxHealthUpgradeModuleData();

	//Simply add the xp scalar to the xp tracker!
	Object *obj = getObject();

	BodyModuleInterface *body = obj->getBodyModule();
	if( body )
	{
		body->setMaxHealth( body->getMaxHealth() + data->m_addMaxHealth, data->m_maxHealthChangeType );
	}
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void MaxHealthUpgrade::crc( Xfer *xfer )
{

	// extend base class
	UpgradeModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void MaxHealthUpgrade::xfer( Xfer *xfer )
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
void MaxHealthUpgrade::loadPostProcess( void )
{

	// extend base class
	UpgradeModule::loadPostProcess();

}
