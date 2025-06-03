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

// FILE: CleanupAreaPower.cpp /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//                                                                          
//                       Electronic Arts Pacific.                          
//                                                                          
//                       Confidential Information                           
//                Copyright (C) 2002 - All Rights Reserved                  
//                                                                          
//-----------------------------------------------------------------------------
//
//	Created:	September 2002
//
//	Author:		Kris Morness
//	
//  Makes use of the cleanup hazard update by augmenting the cleanup range 
//  until there is nothing left to cleanup at which time it goes idle.
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/Player.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"
#include "GameLogic/Module/CleanupAreaPower.h"
#include "GameLogic/Module/CleanupHazardUpdate.h"
#include "GameLogic/Object.h"

//-------------------------------------------------------------------------------------------------
CleanupAreaPowerModuleData::CleanupAreaPowerModuleData()
{
	m_cleanupMoveRange = 0.0;
} 

//-------------------------------------------------------------------------------------------------
void CleanupAreaPowerModuleData::buildFieldParse(MultiIniFieldParse& p)
{
	SpecialPowerModuleData::buildFieldParse( p );

	static const FieldParse dataFieldParse[] = 
	{
		{ "MaxMoveDistanceFromLocation",			INI::parseReal, NULL, offsetof( CleanupAreaPowerModuleData, m_cleanupMoveRange ) },
		{ 0, 0, 0, 0 } 
	};
	p.add(dataFieldParse);

}  // end buildFieldParse

//-------------------------------------------------------------------------------------------------
CleanupAreaPower::CleanupAreaPower( Thing *thing, const ModuleData* moduleData ) : SpecialPowerModule( thing, moduleData )
{

}

//-------------------------------------------------------------------------------------------------
CleanupAreaPower::~CleanupAreaPower()
{

}

//-------------------------------------------------------------------------------------------------
void CleanupAreaPower::doSpecialPowerAtLocation( const Coord3D *loc, Real angle, UnsignedInt commandOptions )
{
	if (getObject()->isDisabled())
		return;

	Object *obj = getObject();
	const CleanupAreaPowerModuleData *data = getCleanupAreaPowerModuleData();

	static NameKeyType key_CleanupHazardUpdate = NAMEKEY( "CleanupHazardUpdate" );
	CleanupHazardUpdate *update = (CleanupHazardUpdate*)obj->findUpdateModule( key_CleanupHazardUpdate );
	if( update )
	{
		update->setCleanupAreaParameters( loc, data->m_cleanupMoveRange );
	}
	else
	{
		//This case will only happen should the ambulance not have a cleanuphazard update module.
		DEBUG_CRASH( ("%s is attempting to use CleanupAreaPower, but requires a CleanupHazardUpdate module to work!", obj->getTemplate()->getName().str() ) );
	}
}


//-------------------------------------------------------------------------------------------------
void CleanupAreaPower::crc( Xfer *xfer )
{

	// extend base class
	SpecialPowerModule::crc( xfer );

}  // end crc

//-------------------------------------------------------------------------------------------------
// Xfer method
// Version Info:
// 1: Initial version 
//-------------------------------------------------------------------------------------------------
void CleanupAreaPower::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	SpecialPowerModule::xfer( xfer );

}  // end xfer

//-------------------------------------------------------------------------------------------------
void CleanupAreaPower::loadPostProcess( void )
{

	// extend base class
	SpecialPowerModule::loadPostProcess();

}  // end loadPostProcess
