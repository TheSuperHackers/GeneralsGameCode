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

// FILE: EnemyNearUpdate.cpp ///////////////////////////////////////////////////////////////////////////
// Author: Matthew D. Campbell, December 2002
// Desc:   Reacts when an enemy is within range
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/PerfTimer.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"
#include "GameClient/Drawable.h"
#include "GameLogic/Module/EnemyNearUpdate.h"
#include "GameLogic/Object.h"
#include "GameLogic/AI.h"
#include "GameLogic/Module/AIUpdate.h"

//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
EnemyNearUpdate::EnemyNearUpdate( Thing *thing, const ModuleData* moduleData ) : UpdateModule( thing, moduleData ),
	m_enemyNear(false),
	m_enemyScanDelay(0)
{
	// bias a random amount so everyone doesn't spike at once
	m_enemyScanDelay += GameLogicRandomValue(0, getEnemyNearUpdateModuleData()->m_enemyScanDelayTime);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
EnemyNearUpdate::~EnemyNearUpdate( void )
{
}


//-------------------------------------------------------------------------------------------------
/**
 * Look around us for enemies.
 */
void EnemyNearUpdate::checkForEnemies( void )
{
	// periodic enemy checks
	if (m_enemyScanDelay == 0)
	{
		m_enemyScanDelay = getEnemyNearUpdateModuleData()->m_enemyScanDelayTime;

		Real visionRange = getObject()->getVisionRange();
		Object* enemy = TheAI->findClosestEnemy( getObject(), visionRange, AI::CAN_SEE );
		m_enemyNear = (enemy != NULL);
	}
	else
	{
		--m_enemyScanDelay;
	}
}

//-------------------------------------------------------------------------------------------------
///< Sit around until an enemy gets near.
//-------------------------------------------------------------------------------------------------
UpdateSleepTime EnemyNearUpdate::update()
{
/// @todo srj use SLEEPY_UPDATE here
	Bool enemyWasNear = m_enemyNear;

	checkForEnemies();

	if (m_enemyNear && !enemyWasNear)
	{
		// change the state of the art to an "enemy near" state
		Drawable *draw = getObject()->getDrawable();
		if( draw )
			draw->setModelConditionState( MODELCONDITION_ENEMYNEAR );
	}
	else if (!m_enemyNear && enemyWasNear)
	{
		// change the state of the art to an idle state
		Drawable *draw = getObject()->getDrawable();
		if( draw )
			draw->clearModelConditionState( MODELCONDITION_ENEMYNEAR );
	}
	return UPDATE_SLEEP_NONE;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void EnemyNearUpdate::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void EnemyNearUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// enemy scan delay
	xfer->xferUnsignedInt( &m_enemyScanDelay );

	// enemy near
	xfer->xferBool( &m_enemyNear );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void EnemyNearUpdate::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}  // end loadPostProcess
