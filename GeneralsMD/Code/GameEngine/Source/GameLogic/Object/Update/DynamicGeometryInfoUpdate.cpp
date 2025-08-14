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

// FILE: DynamicGeometryInfoUpdate.cpp //////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, April 2002
// Desc:   Update module that changes the object's GeometryInfo
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/Xfer.h"
#include "GameLogic/Module/DynamicGeometryInfoUpdate.h"
#include "GameLogic/Object.h"

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
DynamicGeometryInfoUpdateModuleData::DynamicGeometryInfoUpdateModuleData()
{

	m_initialDelay = 0;

	m_initialHeight = 0;
	m_initialMajorRadius = 0;
	m_initialMinorRadius = 0;

	m_finalHeight = 0;
	m_finalMajorRadius = 0;
	m_finalMinorRadius = 0;

	m_transitionTime = 1;
	m_reverseAtTransitionTime = FALSE;

}

//-------------------------------------------------------------------------------------------------
/*static*/ void DynamicGeometryInfoUpdateModuleData::buildFieldParse(MultiIniFieldParse& p)
{
	ModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{

		{ "InitialDelay",					INI::parseDurationUnsignedInt,		NULL, offsetof(DynamicGeometryInfoUpdateModuleData, m_initialDelay) },

		{ "InitialHeight",				INI::parseReal,										NULL, offsetof(DynamicGeometryInfoUpdateModuleData, m_initialHeight) },
		{ "InitialMajorRadius",		INI::parseReal,										NULL, offsetof(DynamicGeometryInfoUpdateModuleData, m_initialMajorRadius) },
		{ "InitialMinorRadius",		INI::parseReal,										NULL, offsetof(DynamicGeometryInfoUpdateModuleData, m_initialMinorRadius) },

		{ "FinalHeight",					INI::parseReal, NULL, offsetof(DynamicGeometryInfoUpdateModuleData, m_finalHeight) },
		{ "FinalMajorRadius",			INI::parseReal, NULL, offsetof(DynamicGeometryInfoUpdateModuleData, m_finalMajorRadius) },
		{ "FinalMinorRadius",			INI::parseReal, NULL, offsetof(DynamicGeometryInfoUpdateModuleData, m_finalMinorRadius) },

		{ "TransitionTime",				INI::parseDurationUnsignedInt,		NULL, offsetof(DynamicGeometryInfoUpdateModuleData, m_transitionTime) },
		{ "ReverseAtTransitionTime", INI::parseBool,	 NULL, offsetof( DynamicGeometryInfoUpdateModuleData, m_reverseAtTransitionTime ) },

		{ 0, 0, 0, 0 }

	};
	p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
DynamicGeometryInfoUpdate::DynamicGeometryInfoUpdate( Thing *thing, const ModuleData* moduleData ) : UpdateModule( thing, moduleData )
{
	DynamicGeometryInfoUpdateModuleData *modData = (DynamicGeometryInfoUpdateModuleData *)moduleData;
	m_startingDelayCountdown = modData->m_initialDelay;
	m_startingDelayCountdown = max( m_startingDelayCountdown, 1u );
	m_timeActive = 0;

	m_started = FALSE;
	m_finished = FALSE;

	m_reverseAtTransitionTime = modData->m_reverseAtTransitionTime;
	m_switchedDirections = FALSE;

	// record in our instance what initial and final height are
	m_initialHeight = modData->m_initialHeight;
	m_initialMajorRadius = modData->m_initialMajorRadius;
	m_initialMinorRadius = modData->m_initialMinorRadius;
	m_finalHeight = modData->m_finalHeight;
	m_finalMajorRadius = modData->m_finalMajorRadius;
	m_finalMinorRadius = modData->m_finalMinorRadius;

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
DynamicGeometryInfoUpdate::~DynamicGeometryInfoUpdate( void )
{
}

//-------------------------------------------------------------------------------------------------
/** The update callback. */
//-------------------------------------------------------------------------------------------------
UpdateSleepTime DynamicGeometryInfoUpdate::update( void )
{
/// @todo srj use SLEEPY_UPDATE here
	if( m_finished )
		return UPDATE_SLEEP_NONE;

	if( !m_started )
	{

		m_startingDelayCountdown--;
		if( m_startingDelayCountdown > 0 )
			return UPDATE_SLEEP_NONE;

		m_started = TRUE;

	}  // end if

	// Either we've been running, or we just started right now.  Doesn't matter.
	const DynamicGeometryInfoUpdateModuleData *data = getDynamicGeometryInfoUpdateModuleData();
	Object *me = getObject();
	Real newHeight, newMajor, newMinor;

	Real ratio = (float)m_timeActive / (float)data->m_transitionTime;

	newHeight = m_initialHeight + (ratio * (m_finalHeight - m_initialHeight));
	newMajor = m_initialMajorRadius + (ratio * (m_finalMajorRadius - m_initialMajorRadius));
	newMinor = m_initialMinorRadius + (ratio * (m_finalMinorRadius - m_initialMinorRadius));

	// make a new geometry info with the new values
	const GeometryInfo oldGeom = me->getGeometryInfo();
	GeometryInfo newGeom( oldGeom.getGeomType(), oldGeom.getIsSmall(), newHeight, newMajor, newMinor );
	me->setGeometryInfo( newGeom );

	// we've not been active another frame .. increment out counter
	m_timeActive++;

	// greater, so an update that is exactly right will be done
	if( m_timeActive > data->m_transitionTime )
	{

		// if we're supposed to reverse at transition time do so now if we haven't already
		if( m_reverseAtTransitionTime == TRUE )
		{

			// switch directions, reset time active, and turn off the switch at transition time
			m_switchedDirections = TRUE;
			m_timeActive = 0;
			m_reverseAtTransitionTime = FALSE;

			// swap the initial and final values
			m_initialHeight = data->m_finalHeight;
			m_initialMajorRadius = data->m_finalMajorRadius;
			m_initialMinorRadius = data->m_finalMinorRadius;
			m_finalHeight = data->m_initialHeight;
			m_finalMajorRadius = data->m_initialMajorRadius;
			m_finalMinorRadius = data->m_initialMinorRadius;

		}  // end if
		else
		{

			// no switch needed ... we're all done
			m_finished = TRUE;

		}  // end else

	}  // end if, time active is longer than transition time

	return UPDATE_SLEEP_NONE;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void DynamicGeometryInfoUpdate::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void DynamicGeometryInfoUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// starting delay countdown
	xfer->xferUnsignedInt( &m_startingDelayCountdown );

	// time active
	xfer->xferUnsignedInt( &m_timeActive );

	// started
	xfer->xferBool( &m_started );

	// finished
	xfer->xferBool( &m_finished );

	// reverse at transition time
	xfer->xferBool( &m_reverseAtTransitionTime );

	// direction
	xfer->xferUser( &m_direction, sizeof( DynamicGeometryDirection ) );

	// switched directions
	xfer->xferBool( &m_switchedDirections );

	// initial height
	xfer->xferReal( &m_initialHeight );

	// initial major radius
	xfer->xferReal( &m_initialMajorRadius );

	// initial minor radius
	xfer->xferReal( &m_initialMinorRadius );

	// final height
	xfer->xferReal( &m_finalHeight );

	// final major radius
	xfer->xferReal( &m_finalMajorRadius );

	// final minor radius
	xfer->xferReal( &m_finalMinorRadius );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void DynamicGeometryInfoUpdate::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}  // end loadPostProcess
