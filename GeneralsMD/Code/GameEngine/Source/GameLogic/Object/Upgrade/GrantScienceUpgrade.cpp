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

// FILE: GrantScienceUpgrade.cpp /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//                                                                          
//                       Electronic Arts Los Angeles
//                                                                          
//                       Confidential Information                           
//                Copyright (C) 2003 - All Rights Reserved                  
//                                                                          
//-----------------------------------------------------------------------------
//
//	Created:	  August 2, 2003
//
//	Filename: 	GrantScienceUpgrade.cpp
//
//	Author:		  Kris Morness
//	
//	Purpose:	  Grants specified science once requirements met (typically an upgrade).
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/Player.h"
#include "Common/Xfer.h"

#include "GameLogic/Object.h"

#include "GameLogic/Module/GrantScienceUpgrade.h"

#ifdef RTS_INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void GrantScienceUpgradeModuleData::buildFieldParse(MultiIniFieldParse& p) 
{
  UpgradeModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] = 
	{
		{ "GrantScience",		INI::parseAsciiString,	NULL, offsetof( GrantScienceUpgradeModuleData, m_grantScienceName ) },
		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
GrantScienceUpgrade::GrantScienceUpgrade( Thing *thing, const ModuleData* moduleData ) : UpgradeModule( thing, moduleData )
{
	m_scienceType = SCIENCE_INVALID;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
GrantScienceUpgrade::~GrantScienceUpgrade( void )
{
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void GrantScienceUpgrade::upgradeImplementation( )
{
	if( m_scienceType == SCIENCE_INVALID )
	{
		const GrantScienceUpgradeModuleData *data = getGrantScienceUpgradeModuleData();
		m_scienceType = TheScienceStore->getScienceFromInternalName( data->m_grantScienceName );
	}

	Object *obj = getObject();	
	Player *player = obj->getControllingPlayer();
	if( player )
	{
		player->grantScience( m_scienceType );
	}
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void GrantScienceUpgrade::crc( Xfer *xfer )
{

	// extend base class
	UpgradeModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void GrantScienceUpgrade::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpgradeModule::xfer( xfer );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void GrantScienceUpgrade::loadPostProcess( void )
{

	// extend base class
	UpgradeModule::loadPostProcess();

}  // end loadPostProcess
