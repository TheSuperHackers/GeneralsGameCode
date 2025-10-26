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

// FILE: ObjectHelper.cpp /////////////////////////////////////////////////////////////////////////
// Author: Colin Day, Steven Johnson, September 2002
// Desc:   Object helper module base class
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"
#include "Common/Xfer.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/ObjectHelper.h"


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
ObjectHelper::~ObjectHelper( void )
{

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void ObjectHelper::sleepUntil(UnsignedInt when)
{
	if( getObject()->getStatusBits().test( OBJECT_STATUS_DESTROYED ) )
		return;

	// note the setWakeFrame(NEVER) actually awakens immediately, since NEVER==0.
	// when we get NEVER in this case, we really want to sleep forever.
	// so just special case it.
	UpdateSleepTime wakeDelay = (when == NEVER || when == FOREVER) ?
																UPDATE_SLEEP_FOREVER :
																UPDATE_SLEEP(when - TheGameLogic->getFrame());
	setWakeFrame(getObject(), wakeDelay);
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void ObjectHelper::crc( Xfer *xfer )
{

	// update module crc
	UpdateModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/* Xfer method
 * Version Info:
 * 1: Initial Version */
// ------------------------------------------------------------------------------------------------
void ObjectHelper::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// update module xfer
	UpdateModule::xfer( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void ObjectHelper::loadPostProcess( void )
{

	// update module post process
	UpdateModule::loadPostProcess();

}
