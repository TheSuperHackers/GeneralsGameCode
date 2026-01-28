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

// FILE: RadarUpdate.cpp //////////////////////////////////////////////////////////////////////////
// Author: Colin Day, April 2002
// Desc:   Updating a radar on an object
///////////////////////////////////////////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/ModelState.h"
#include "Common/Xfer.h"
#include "GameClient/Drawable.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/RadarUpdate.h"

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
RadarUpdateModuleData::RadarUpdateModuleData( void )
{

	m_radarExtendTime = 0.0f;

}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
RadarUpdate::RadarUpdate( Thing *thing, const ModuleData *moduleData )
												: UpdateModule( thing, moduleData )
{

	m_radarActive = FALSE;
	m_extendDoneFrame = 0;
	m_extendComplete = FALSE;

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
RadarUpdate::~RadarUpdate( void )
{

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void RadarUpdate::extendRadar( void )
{
	const RadarUpdateModuleData *modData = getRadarUpdateModuleData();

	// set the model condition for radar extension
	Drawable *draw = getObject()->getDrawable();
	if( draw )
		draw->setModelConditionState( MODELCONDITION_RADAR_EXTENDING );

	// mark the frame that the extension will be done on
	m_extendDoneFrame = TheGameLogic->getFrame() + modData->m_radarExtendTime;

	//Change this to make the radar active after extension...
	m_radarActive = true;

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UpdateSleepTime RadarUpdate::update( void )
{
/// @todo srj use SLEEPY_UPDATE here

	// if no extend frame nothing to do
	if( m_extendDoneFrame == 0 )
		return UPDATE_SLEEP_NONE;

	// check to see if our extension is already done
	if( m_extendComplete == TRUE )
		return UPDATE_SLEEP_NONE;

	// see if it's time to stop the extension
	if( TheGameLogic->getFrame() > m_extendDoneFrame )
	{

		// mark extend as done
		m_extendComplete = TRUE;
		m_extendDoneFrame = 0;  // just to be clean

		// remove the extending condition and set the extended condition
		Drawable *draw = getObject()->getDrawable();
		if( draw )
			draw->clearAndSetModelConditionState( MODELCONDITION_RADAR_EXTENDING,
																						MODELCONDITION_RADAR_UPGRADED );

	}

	return UPDATE_SLEEP_NONE;

}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void RadarUpdate::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void RadarUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// extend done frame
	xfer->xferUnsignedInt( &m_extendDoneFrame );

	// extend complete
	xfer->xferBool( &m_extendComplete );

	// radar active
	xfer->xferBool( &m_radarActive );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void RadarUpdate::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}

