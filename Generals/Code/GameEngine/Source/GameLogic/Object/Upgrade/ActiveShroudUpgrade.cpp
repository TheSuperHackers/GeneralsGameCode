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

// FILE: ActiveShroudUpgrade.cpp ///////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, July 2002
// Desc:	 An upgrade that modifies the object's ShroudRange.
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/Xfer.h"
#include "GameLogic/Module/ActiveShroudUpgrade.h"
#include "GameLogic/Object.h"

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ActiveShroudUpgradeModuleData::ActiveShroudUpgradeModuleData( void )
{

	m_newShroudRange = 0.0f;

}  // end SpecialPowerModuleData

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/* static */ void ActiveShroudUpgradeModuleData::buildFieldParse(MultiIniFieldParse& p)
{
	UpgradeModuleData::buildFieldParse( p );

	static const FieldParse dataFieldParse[] =
	{
		{ "NewShroudRange", INI::parseReal, NULL, offsetof( ActiveShroudUpgradeModuleData, m_newShroudRange ) },
		{ 0, 0, 0, 0 }
	};
	p.add(dataFieldParse);

}  // end buildFieldParse

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ActiveShroudUpgrade::ActiveShroudUpgrade( Thing *thing, const ModuleData* moduleData ) :
							UpgradeModule( thing, moduleData )
{

}  // end ActiveShroudUpgrade

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ActiveShroudUpgrade::~ActiveShroudUpgrade( void )
{

}  // end ~ActiveShroudUpgrade

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void ActiveShroudUpgrade::upgradeImplementation( void )
{
	// Set my object's ability to actively shroud.
	if( getActiveShroudUpgradeModuleData() )
	{
		getObject()->setShroudRange( getActiveShroudUpgradeModuleData()->m_newShroudRange );
		getObject()->handlePartitionCellMaintenance();// To shroud where I am without waiting.
	}
}  // end upgradeImplementation

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void ActiveShroudUpgrade::crc( Xfer *xfer )
{

	// extend base class
	UpgradeModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void ActiveShroudUpgrade::xfer( Xfer *xfer )
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
void ActiveShroudUpgrade::loadPostProcess( void )
{

	// extend base class
	UpgradeModule::loadPostProcess();

}  // end loadPostProcess
