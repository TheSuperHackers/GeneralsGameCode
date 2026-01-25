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

// FILE: SwayClientUpdate.cpp //////////////////////////////////////////////////////////////////
// Author: Matthew D. Campbell, May 2002
// Desc:   Tree sway client update module
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "GameClient/Drawable.h"
#include "GameClient/Module/SwayClientUpdate.h"
#include "Common/FramePacer.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "Common/RandomValue.h"
#include "Common/PerfTimer.h"
#include "Common/Xfer.h"
#include "GameLogic/Object.h"
#include "GameLogic/ScriptEngine.h"
#include "GameLogic/GameLogic.h"


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
SwayClientUpdate::SwayClientUpdate( Thing *thing, const ModuleData* moduleData ) :
	ClientUpdateModule( thing, moduleData ),
	m_curDelta(0),
	m_curValue(0),
	m_curAngle(0),
	m_curAngleLimit(0),
	m_leanAngle(0),
	m_swaying(true),
	m_unused(false),
	m_curVersion(-1)	// so that we never match the first time
{

	// don't do updateSway here; wait till the first time we go thru our update loop.
	//updateSway();

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
SwayClientUpdate::~SwayClientUpdate( void )
{

}

//-------------------------------------------------------------------------------------------------
// Update the sway parameters.
//-------------------------------------------------------------------------------------------------
void SwayClientUpdate::updateSway()
{
	const BreezeInfo& info = TheScriptEngine->getBreezeInfo();
	if (info.m_randomness == 0.0f)
	{
		m_curValue = 0;
	}
	Real delta				= info.m_randomness * 0.5f;
	m_curAngleLimit		= info.m_intensity * GameClientRandomValueReal(1.0f-delta, 1.0f+delta);
	m_curDelta				= 2*PI/info.m_breezePeriod * GameClientRandomValueReal(1.0f-delta, 1.0f+delta);
	m_leanAngle				= info.m_lean * GameClientRandomValueReal(1.0f-delta, 1.0f+delta);
	m_curVersion			= info.m_breezeVersion;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
/** The client update callback. */
//-------------------------------------------------------------------------------------------------
void SwayClientUpdate::clientUpdate( void )
{
	if( !m_swaying )
		return;

	Drawable *draw = getDrawable();

	// if breeze changes, always process the full update, even if not visible,
	// so that things offscreen won't 'pop' when first viewed
	const BreezeInfo& info = TheScriptEngine->getBreezeInfo();
	if (info.m_breezeVersion != m_curVersion)
	{
		updateSway();
	}
	else
	{
		// Otherwise, only update visible drawables
		if (!draw || !draw->isVisible())
			return;
	}

	// TheSuperHackers @tweak The tree sway time step is now decoupled from the render update.
	const Real timeScale = TheFramePacer->getActualLogicTimeScaleOverFpsRatio();

	m_curValue += m_curDelta * timeScale;
	if (m_curValue > 2*PI)
		m_curValue -= 2*PI;
	Real cosine = Cos(m_curValue);

	Real targetAngle = cosine * m_curAngleLimit + m_leanAngle;
	Real deltaAngle = targetAngle - m_curAngle;

	Matrix3D xfrm = *draw->getInstanceMatrix();
	xfrm.In_Place_Pre_Rotate_X(-deltaAngle * info.m_directionVec.x);
	xfrm.In_Place_Pre_Rotate_Y(deltaAngle * info.m_directionVec.y);
	draw->setInstanceMatrix(&xfrm);

	m_curAngle = targetAngle;

	// burned things don't sway.
	Object* obj = draw->getObject();
	if( obj && obj->getStatusBits().test( OBJECT_STATUS_BURNED ) )
		stopSway();

}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void SwayClientUpdate::crc( Xfer *xfer )
{

	// extend base class
	ClientUpdateModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void SwayClientUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	ClientUpdateModule::xfer( xfer );

	// cur value
	xfer->xferReal( &m_curValue );

	// cur angle
	xfer->xferReal( &m_curAngle );

	// cur delta
	xfer->xferReal( &m_curDelta );

	// cur angle limit
	xfer->xferReal( &m_curAngleLimit );

	// lean angle
	xfer->xferReal( &m_leanAngle );

	// cur version
	xfer->xferShort( &m_curVersion );

	// swaying
	xfer->xferBool( &m_swaying );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void SwayClientUpdate::loadPostProcess( void )
{

	// extend base class
	ClientUpdateModule::loadPostProcess();

	updateSway();

}
