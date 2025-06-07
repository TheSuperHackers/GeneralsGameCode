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

// FILE: UnitProductionBonusUpgrade.cpp /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//                                                                          
//                       Electronic Arts Pacific.                          
//                                                                          
//                       Confidential Information                           
//                Copyright (C) 2002 - All Rights Reserved                  
//                                                                          
//-----------------------------------------------------------------------------
//
//	created:	Aug 2002
//
//	Filename:  UnitProductionBonusUpgrade.cpp
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

// FILE: UnitProductionBonusUpgrade.cpp /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//                                                                          
//                       Electronic Arts Pacific.                          
//                                                                          
//                       Confidential Information                           
//                Copyright (C) 2002 - All Rights Reserved                  
//                                                                          
//-----------------------------------------------------------------------------
//
//	created:	June 2025
//
//	Filename: 	UnitProductionBonusUpgrade.h
//
//	author:		Andi W
//	
//	purpose:	Upgrade that modifies the cost or build time of a list of units
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/Player.h"
#include "Common/ThingTemplate.h"
#include "Common/ThingFactory.h"
#include "Common/Xfer.h"
#include "GameLogic/Module/UnitProductionBonusUpgrade.h"
#include "GameLogic/Object.h"
#include "Common/BitFlagsIO.h"


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UnitProductionBonusUpgradeModuleData::UnitProductionBonusUpgradeModuleData( void )
{
	m_templateNames.clear();
	m_costPercentage = 0.0f;
	m_timePercentage = 0.0f;
	// m_isOneShot = FALSE;

}  // end UnitProductionBonusUpgradeModuleData

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/* static */ void UnitProductionBonusUpgradeModuleData::buildFieldParse(MultiIniFieldParse& p)
{
	UpgradeModuleData::buildFieldParse( p );

	static const FieldParse dataFieldParse[] = 
	{
		{ "CostModifierPercentage",			INI::parsePercentToReal, NULL, offsetof( UnitProductionBonusUpgradeModuleData, m_costPercentage ) },
		{ "BuildTimeModifierPercentage",			INI::parsePercentToReal, NULL, offsetof( UnitProductionBonusUpgradeModuleData, m_timePercentage ) },
		// { "IsOneShotUpgrade",		INI::parseBool, NULL, offsetof( UnitProductionBonusUpgradeModuleData, m_isOneShot) },
		{ "UnitTemplateName", INI::parseAsciiStringVectorAppend, NULL, offsetof(UnitProductionBonusUpgradeModuleData, m_templateNames) },

		{ 0, 0, 0, 0 } 
	};
	p.add(dataFieldParse);

}  // end buildFieldParse

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UnitProductionBonusUpgrade::UnitProductionBonusUpgrade( Thing *thing, const ModuleData* moduleData ) : 
							UpgradeModule( thing, moduleData )
{

}  // end UnitProductionBonusUpgrade

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UnitProductionBonusUpgrade::~UnitProductionBonusUpgrade( void )
{

}  // end ~UnitProductionBonusUpgrade

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//void UnitProductionBonusUpgrade::onDelete( void )
//{
//
//	// this upgrade module is now "not upgraded"
//	setUpgradeExecuted(FALSE);
//
//}  // end onDelete

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//void UnitProductionBonusUpgrade::onCapture( Player *oldOwner, Player *newOwner )
//{
//	
//
//}  // end onCapture

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void UnitProductionBonusUpgrade::upgradeImplementation( void )
{
	const UnitProductionBonusUpgradeModuleData * d = getUnitProductionBonusUpgradeModuleData();

	Player *player = getObject()->getControllingPlayer();

	for (std::vector<AsciiString>::const_iterator tempName = d->m_templateNames.begin();
		tempName != d->m_templateNames.end(); ++tempName)
	{
		if (d->m_costPercentage != 0.0f) {
			player->addProductionCostChangePercent(*tempName, d->m_costPercentage);
		}

		if (d->m_timePercentage != 0.0f) {
			player->addProductionTimeChangePercent(*tempName, d->m_timePercentage);
		}
	}

}  // end upgradeImplementation

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void UnitProductionBonusUpgrade::crc( Xfer *xfer )
{

	// extend base class
	UpgradeModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void UnitProductionBonusUpgrade::xfer( Xfer *xfer )
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
void UnitProductionBonusUpgrade::loadPostProcess( void )
{

	// extend base class
	UpgradeModule::loadPostProcess();

}  // end loadPostProcess
