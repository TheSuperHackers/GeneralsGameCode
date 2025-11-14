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

// FILE: StatusDamageHelper.h ////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, June 2003
// Desc:   Object helper - Clears Status conditions on a timer.
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"
#include "Common/Xfer.h"

#include "GameLogic/Module/StatusDamageHelper.h"

#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
StatusDamageHelper::StatusDamageHelper( Thing *thing, const ModuleData *modData ) : ObjectHelper( thing, modData )
{
	m_statusToHeal = OBJECT_STATUS_NONE;
	m_frameToHeal = 0;

	setWakeFrame(getObject(), UPDATE_SLEEP_FOREVER);
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
StatusDamageHelper::~StatusDamageHelper( void )
{

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
UpdateSleepTime StatusDamageHelper::update()
{
	DEBUG_ASSERTCRASH(m_frameToHeal <= TheGameLogic->getFrame(), ("StatusDamageHelper woke up too soon.") );

	clearStatusCondition(); // We are sleep driven, so seeing an update means our timer is ready implicitly
	return UPDATE_SLEEP_FOREVER;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void StatusDamageHelper::clearStatusCondition()
{
	if( m_statusToHeal != OBJECT_STATUS_NONE )
	{
		getObject()->clearStatus( MAKE_OBJECT_STATUS_MASK(m_statusToHeal) );
		m_statusToHeal = OBJECT_STATUS_NONE;
		m_frameToHeal = 0;
	}
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void StatusDamageHelper::doStatusDamage( ObjectStatusTypes status, Real duration )
{
	Int durationAsInt = REAL_TO_INT_FLOOR(duration);

	// Clear any different status we may have.  Re-getting the same status will just reset the timer
	if( m_statusToHeal != status )
		clearStatusCondition();

	getObject()->setStatus( MAKE_OBJECT_STATUS_MASK(status) );
	m_statusToHeal = status;
	m_frameToHeal = TheGameLogic->getFrame() + durationAsInt;

	setWakeFrame( getObject(), UPDATE_SLEEP(durationAsInt) );
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void StatusDamageHelper::crc( Xfer *xfer )
{

	// object helper crc
	ObjectHelper::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info;
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void StatusDamageHelper::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// object helper base class
	ObjectHelper::xfer( xfer );

	xfer->xferUser( &m_statusToHeal, sizeof(ObjectStatusTypes) );// an enum
	xfer->xferUnsignedInt( &m_frameToHeal );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void StatusDamageHelper::loadPostProcess( void )
{

	// object helper base class
	ObjectHelper::loadPostProcess();

}

