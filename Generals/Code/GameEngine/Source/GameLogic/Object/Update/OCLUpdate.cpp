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

// FILE: OCLUpdate.cpp /////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, August2002
// Desc:   Update Spits out an OCL on a timer
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/RandomValue.h"
#include "Common/Xfer.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/ObjectCreationList.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/Module/OCLUpdate.h"
#include "GameLogic/TerrainLogic.h"

//-------------------------------------------------------------------------------------------------
OCLUpdateModuleData::OCLUpdateModuleData()
{
	m_minDelay = 0;
	m_maxDelay = 0;
	m_ocl = NULL;
	m_isCreateAtEdge = FALSE;
}

//-------------------------------------------------------------------------------------------------
/*static*/ void OCLUpdateModuleData::buildFieldParse(MultiIniFieldParse& p) 
{
  UpdateModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] = 
	{
		{ "OCL",					INI::parseObjectCreationList,		NULL, offsetof( OCLUpdateModuleData, m_ocl ) },
		{ "MinDelay",			INI::parseDurationUnsignedInt,	NULL, offsetof( OCLUpdateModuleData, m_minDelay ) },
		{ "MaxDelay",			INI::parseDurationUnsignedInt,	NULL, offsetof( OCLUpdateModuleData, m_maxDelay ) },
		{ "CreateAtEdge",	INI::parseBool,									NULL, offsetof( OCLUpdateModuleData, m_isCreateAtEdge ) },
		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
OCLUpdate::OCLUpdate( Thing *thing, const ModuleData* moduleData ) : UpdateModule( thing, moduleData )
{
	m_nextCreationFrame = 0;
	m_timerStartedFrame = 0;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
OCLUpdate::~OCLUpdate( void )
{
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UpdateSleepTime OCLUpdate::update( void )
{
/// @todo srj use SLEEPY_UPDATE here
	if( shouldCreate() )
	{
		if( m_nextCreationFrame == 0 )
		{
			// You don't get to actually spread the first try, you start on a timer, then go
			setNextCreationFrame();
			return UPDATE_SLEEP_NONE;
		}

		setNextCreationFrame();

		Coord3D creationCoord;
		if( getOCLUpdateModuleData()->m_isCreateAtEdge )
			creationCoord = TheTerrainLogic->findClosestEdgePoint( getObject()->getPosition() );
		else
			creationCoord = *getObject()->getPosition();

		ObjectCreationList::create( getOCLUpdateModuleData()->m_ocl, getObject(), &creationCoord, getObject()->getPosition() );
	}
	return UPDATE_SLEEP_NONE;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
Bool OCLUpdate::shouldCreate()
{
	if( TheGameLogic->getFrame() < m_nextCreationFrame )
		return FALSE;//too soon

	if( BitIsSet( getObject()->getStatusBits(), OBJECT_STATUS_UNDER_CONSTRUCTION ) == TRUE )
		return FALSE;// not built yet

	return TRUE;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void OCLUpdate::setNextCreationFrame()
{
	UnsignedInt delay = GameLogicRandomValue( getOCLUpdateModuleData()->m_minDelay, 
																						getOCLUpdateModuleData()->m_maxDelay );
	m_timerStartedFrame = TheGameLogic->getFrame();
	m_nextCreationFrame = m_timerStartedFrame + delay;

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
Real OCLUpdate::getCountdownPercent() const
{
	UnsignedInt now = TheGameLogic->getFrame();

	return 1.0f - (( m_nextCreationFrame - now ) / (float)( m_nextCreationFrame - m_timerStartedFrame ));
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
UnsignedInt OCLUpdate::getRemainingFrames() const
{
	UnsignedInt now = TheGameLogic->getFrame();

	return ( m_nextCreationFrame - now );
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void OCLUpdate::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void OCLUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// next creation frame
	xfer->xferUnsignedInt( &m_nextCreationFrame );

	// timer stated frame
	xfer->xferUnsignedInt( &m_timerStartedFrame );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void OCLUpdate::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}  // end loadPostProcess
