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

// FILE: CheckpointUpdate.cpp ///////////////////////////////////////////////////////////////////////////
// Author: Mark Lorenzen, August 2002
// Desc:   Opens gate when (ally near && ( ! enemy near) )
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/PerfTimer.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"
#include "GameLogic/Module/CheckpointUpdate.h"
#include "GameLogic/Object.h"
#include "GameClient/Drawable.h"
#include "GameLogic/AI.h"
#include "GameLogic/Module/AIUpdate.h"

//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
CheckpointUpdate::CheckpointUpdate( Thing *thing, const ModuleData* moduleData ) : UpdateModule( thing, moduleData ),
m_allyNear(false),
m_enemyNear(false),
m_enemyScanDelay(0),
m_maxMinorRadius(0)
{
	m_maxMinorRadius = getObject()->getGeometryInfo().getMinorRadius();

	// bias a random amount so everyone doesn't spike at once
	m_enemyScanDelay += GameLogicRandomValue(0, getCheckpointUpdateModuleData()->m_enemyScanDelayTime);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
CheckpointUpdate::~CheckpointUpdate( void )
{
}

//-------------------------------------------------------------------------------------------------
/**
 * Look around us for enemies.
 */
void CheckpointUpdate::checkForAlliesAndEnemies( void )
{
	// periodic enemy checks
	if (m_enemyScanDelay == 0 || TRUE)
	{
		m_enemyScanDelay = getCheckpointUpdateModuleData()->m_enemyScanDelayTime;

		// Kind of weird, but we have to set the geometry extents to the max extent
		//before checking for nearest enemies and allies, or else the stretch reaction
		//to finding one will oscillate states open->closed->open...
		Object *obj = getObject();
		GeometryInfo geom = obj->getGeometryInfo();
		Real restoreSpecialRadius = geom.getMinorRadius();
		geom.setMinorRadius( m_maxMinorRadius );
		obj->setGeometryInfo( geom );

		Object *enemy, *ally = NULL;
		Real visionRange = obj->getVisionRange();

		enemy = TheAI->findClosestEnemy( obj, visionRange, 0 );
		m_enemyNear = (enemy != NULL);

		ally = TheAI->findClosestAlly( obj, visionRange, 0 );
		m_allyNear = (ally != NULL);

		// here we restore the radius so that other units can path past the open gate
		geom.setMinorRadius( restoreSpecialRadius );
		obj->setGeometryInfo( geom );

	}
	else
	{
		--m_enemyScanDelay;
	}
}

//-------------------------------------------------------------------------------------------------
///< Sit around until an enemy gets near.
//-------------------------------------------------------------------------------------------------
UpdateSleepTime CheckpointUpdate::update()
{
/// @todo srj use SLEEPY_UPDATE here

	Bool wasAnAlly  = m_allyNear;
	Bool wasAnEnemy = m_enemyNear;

	checkForAlliesAndEnemies();

	Bool change = ( (wasAnAlly != m_allyNear) || (wasAnEnemy != m_enemyNear) );
	Bool open = ( ( ! m_enemyNear) && m_allyNear );
	Object *obj = getObject();
	Drawable *draw = obj->getDrawable();

	if ( draw )
	{
		if ( change )
		{
			if ( open )
			{
				// change the state of the art to open the door!
				if( draw )
				{
				/// @todo srj -- for now, this assumes at most one door
					draw->clearAndSetModelConditionState( MODELCONDITION_DOOR_1_CLOSING, MODELCONDITION_DOOR_1_OPENING );
				}
			}
			else //closed, then
			{
				// change the state of the art to close the door!
				if( draw )
				{
					/// @todo srj -- for now, this assumes at most one door
					draw->clearAndSetModelConditionState( MODELCONDITION_DOOR_1_OPENING, MODELCONDITION_DOOR_1_CLOSING );
				}
			}

		}

		GeometryInfo geom = obj->getGeometryInfo();

		/// @todo ask steven about the distance covered always being zero
	//	Real animScrubScalar = draw->getAnimationScrubScalar();
	//	geom.setMinorRadius( m_maxMinorRadius * animScrubScalar );

		// THis method is more accidental than above, but it works for an unimportant thing like checkpoint
		Real radius = geom.getMinorRadius();

		if ( open )
		{
			if ( radius > 0 ) geom.setMinorRadius( radius - 0.333f );
		}
		else //closed
		{
			if ( radius < m_maxMinorRadius ) geom.setMinorRadius( radius + 0.333f );
		}


		obj->setGeometryInfo( geom );

	}

	return UPDATE_SLEEP_NONE;

}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void CheckpointUpdate::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void CheckpointUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// enemy near
	xfer->xferBool( &m_enemyNear );

	// ally near
	xfer->xferBool( &m_allyNear );

	// max minor radius
	xfer->xferReal( &m_maxMinorRadius );

	// enemy scan delay
	xfer->xferUnsignedInt( &m_enemyScanDelay );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void CheckpointUpdate::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}
