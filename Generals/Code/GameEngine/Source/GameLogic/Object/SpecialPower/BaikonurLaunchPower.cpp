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

// FILE: BaikonurLaunchPower.h /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//                                                                          
//                       Electronic Arts Pacific.                          
//                                                                          
//                       Confidential Information                           
//                Copyright (C) 2002 - All Rights Reserved                  
//                                                                          
//-----------------------------------------------------------------------------
//
//	Created:	November 2002
//
//	Filename: BaikonurLaunchPower.h
//
//	Author:		Kris Morness
//
//  Purpose:	Triggers the beginning of the launch for the baikonur launch tower.
//            This is used only by script to trigger the GLA end game.
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/Player.h"
#include "Common/ThingFactory.h"
#include "Common/Xfer.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/BaikonurLaunchPower.h"

BaikonurLaunchPowerModuleData::BaikonurLaunchPowerModuleData( void )
{
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
/*static*/ void BaikonurLaunchPowerModuleData::buildFieldParse(MultiIniFieldParse& p)
{
	SpecialPowerModuleData::buildFieldParse( p );
	
	static const FieldParse dataFieldParse[] = 
	{
		{ "DetonationObject", INI::parseAsciiString, NULL, offsetof( BaikonurLaunchPowerModuleData, m_detonationObject ) },
		{ 0, 0, 0, 0 }
	};
	p.add(dataFieldParse);
	
}  // end buildFieldParse


// ------------------------------------------------------------------------------------------------
BaikonurLaunchPower::BaikonurLaunchPower( Thing *thing, const ModuleData *moduleData )
												: SpecialPowerModule( thing, moduleData )
{

}

// ------------------------------------------------------------------------------------------------
BaikonurLaunchPower::~BaikonurLaunchPower( void )
{

} 

// ------------------------------------------------------------------------------------------------
void BaikonurLaunchPower::doSpecialPower( UnsignedInt commandOptions )
{
	if (getObject()->isDisabled())
		return;

	// call the base class action cause we are *EXTENDING* functionality
	SpecialPowerModule::doSpecialPower( commandOptions );

	getObject()->setModelConditionState( MODELCONDITION_DOOR_1_OPENING );
}  

// ------------------------------------------------------------------------------------------------
void BaikonurLaunchPower::doSpecialPowerAtLocation( const Coord3D *loc, Real angle, UnsignedInt commandOptions )
{
	if (getObject()->isDisabled())
		return;

	const BaikonurLaunchPowerModuleData *data = getBaikonurLaunchPowerModuleData();

	// call the base class action cause we are *EXTENDING* functionality
	SpecialPowerModule::doSpecialPowerAtLocation( loc, angle, commandOptions );

	//Create the detonation
	const ThingTemplate *thing = TheThingFactory->findTemplate( data->m_detonationObject );
	if( thing )
	{
		Object *detonation = TheThingFactory->newObject( thing, getObject()->getTeam() );
		if( detonation )
		{
			detonation->setPosition( loc );
		}
	}
}  


// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void BaikonurLaunchPower::crc( Xfer *xfer )
{

	// extend base class
	SpecialPowerModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void BaikonurLaunchPower::xfer( Xfer *xfer )
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
void BaikonurLaunchPower::loadPostProcess( void )
{

	// extend base class
	SpecialPowerModule::loadPostProcess();

}  // end loadPostProcess
