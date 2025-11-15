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

// FILE: LifetimeUpdate.cpp ///////////////////////////////////////////////////////////////////////
// Author: Colin Day, December 2001
// Desc:   Update that will count down a lifetime and destroy object when it reaches zero
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/RandomValue.h"
#include "Common/Xfer.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Module/LifetimeUpdate.h"
#include "GameLogic/Object.h"


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
LifetimeUpdate::LifetimeUpdate( Thing *thing, const ModuleData* moduleData ) : UpdateModule( thing, moduleData )
{
	const LifetimeUpdateModuleData* d = getLifetimeUpdateModuleData();
	m_dieFrame = 0;
	UnsignedInt delay;
	if( getObject()->isKindOf( KINDOF_HULK ) && TheGameLogic->getHulkMaxLifetimeOverride() != -1 )
	{
		delay = calcSleepDelay( TheGameLogic->getHulkMaxLifetimeOverride(), TheGameLogic->getHulkMaxLifetimeOverride() );
	}
	else
	{
		delay = calcSleepDelay(d->m_minFrames, d->m_maxFrames);
	}

	setWakeFrame(getObject(), UPDATE_SLEEP(delay));
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
LifetimeUpdate::~LifetimeUpdate( void )
{
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void LifetimeUpdate::setLifetimeRange( UnsignedInt minFrames, UnsignedInt maxFrames )
{
	UnsignedInt delay = calcSleepDelay(minFrames, maxFrames);
	setWakeFrame(getObject(), UPDATE_SLEEP(delay));
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UnsignedInt LifetimeUpdate::calcSleepDelay(UnsignedInt minFrames, UnsignedInt maxFrames)
{
	UnsignedInt delay = GameLogicRandomValue( minFrames, maxFrames );
	if (delay < 1) delay = 1;
	m_dieFrame = TheGameLogic->getFrame() + delay;
	return delay;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UpdateSleepTime LifetimeUpdate::update( void )
{
	// Kill (NOT destroy) if time is up
	getObject()->kill();
	return UPDATE_SLEEP_FOREVER;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void LifetimeUpdate::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void LifetimeUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// die frame
	xfer->xferUnsignedInt( &m_dieFrame );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void LifetimeUpdate::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}
