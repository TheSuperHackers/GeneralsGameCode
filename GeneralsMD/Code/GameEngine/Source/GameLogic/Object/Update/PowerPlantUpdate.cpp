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

// FILE: PowerPlantUpdate.cpp //////////////////////////////////////////////////////////////////////////
// Author: Amit Kumar, August 2002
// Desc:   Updating the power plant
///////////////////////////////////////////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/ModelState.h"
#include "Common/Xfer.h"
#include "GameClient/Drawable.h"
#include "GameClient/InGameUI.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/PowerPlantUpdate.h"


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
PowerPlantUpdateModuleData::PowerPlantUpdateModuleData( void )
{

	m_rodsExtendTime = 0;

}  // end PowerPlantUpdateModuleData

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
PowerPlantUpdate::PowerPlantUpdate( Thing *thing, const ModuleData *moduleData )
												: UpdateModule( thing, moduleData )
{

	m_extended = FALSE;
	setWakeFrame(getObject(), UPDATE_SLEEP_FOREVER);

}  // end PowerPlantUpdate

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
PowerPlantUpdate::~PowerPlantUpdate( void )
{

}  // end PowerPlantUpdate

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void PowerPlantUpdate::extendRods( Bool extend )
{
	if (extend)
	{
		if (!m_extended)
		{
			const PowerPlantUpdateModuleData *modData = getPowerPlantUpdateModuleData();

			// set the model condition for radar extension
			Drawable *draw = getObject()->getDrawable();
			if( draw )
				draw->setModelConditionState( MODELCONDITION_POWER_PLANT_UPGRADING );

			m_extended = TRUE;
			setWakeFrame(getObject(), UPDATE_SLEEP(modData->m_rodsExtendTime));
		}
	}
	else
	{
		// they de-extend instantly.
		Drawable *draw = getObject()->getDrawable();
		if( draw )
			draw->clearModelConditionFlags( MAKE_MODELCONDITION_MASK2( MODELCONDITION_POWER_PLANT_UPGRADING,
																														MODELCONDITION_POWER_PLANT_UPGRADED) );

		m_extended = FALSE;
		setWakeFrame(getObject(), UPDATE_SLEEP_FOREVER);
	}

}  // end PowerPlantUpdate

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UpdateSleepTime PowerPlantUpdate::update( void )
{
	// remove the extending condition and set the extened condition
	Drawable *draw = getObject()->getDrawable();
	if( draw )
		draw->clearAndSetModelConditionState( MODELCONDITION_POWER_PLANT_UPGRADING,
																					MODELCONDITION_POWER_PLANT_UPGRADED );

	m_extended = TRUE;
	return UPDATE_SLEEP_FOREVER;
}  // end update

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void PowerPlantUpdate::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void PowerPlantUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// extend complete
	xfer->xferBool( &m_extended );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void PowerPlantUpdate::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}  // end loadPostProcess
