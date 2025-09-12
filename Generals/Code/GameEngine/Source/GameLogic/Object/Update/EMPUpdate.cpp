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

// FILE: EMPUpdate.cpp ///////////////////////////////////////////////////////////////////////
// Author: Mark Lorenzen Sept. 2002
// Desc:   Update that makes the electromagnetic pulse field grow, fade and disable junk
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/Thing.h"
#include "Common/ThingTemplate.h"
#include "Common/INI.h"
#include "Common/RandomValue.h"
#include "Common/Player.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Module/EMPUpdate.h"
#include "GameLogic/ObjectIter.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Object.h"
#include "GameClient/Drawable.h"
#include "Common/KindOf.h"
#include "GameClient/ParticleSys.h"









///////////////////////////////////////////////////////////////////////////////////////////
//	Static member initialization
///////////////////////////////////////////////////////////////////////////////////////////
//Bool	EMPUpdate::s_lastInstanceSpunPositive	= FALSE;

//-------------------------------------------------------------------------------------------------
static void saturateRGB(RGBColor& color, Real factor)
{
	color.red *= factor;
	color.green *= factor;
	color.blue *= factor;

	Real halfFactor = factor * 0.5f;

	color.red -= halfFactor;
	color.green -= halfFactor;
	color.blue -= halfFactor;

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
EMPUpdate::EMPUpdate( Thing *thing, const ModuleData* moduleData ) : UpdateModule( thing, moduleData )
{

	//s_lastInstanceSpunPositive = !s_lastInstanceSpunPositive; //TOGGLES STATIC BOOL

	const EMPUpdateModuleData *data = getEMPUpdateModuleData();
	if ( data )
	{
		//SANITY
		DEBUG_ASSERTCRASH( TheGameLogic, ("EMPUpdate::EMPUpdate - TheGameLogic is NULL" ) );
		UnsignedInt now = TheGameLogic->getFrame();

		m_currentScale = data->m_startScale;
		m_dieFrame = REAL_TO_UNSIGNEDINT( now + data->m_lifeFrames );
		m_tintEnvPlayFrame = REAL_TO_UNSIGNEDINT( now + data->m_startFadeFrame );
		m_tintEnvFadeFrames = m_dieFrame - m_tintEnvPlayFrame;
		//m_spinRate = GameLogicRandomValueReal(data->m_spinRateMax * 0.5f, data->m_spinRateMax);
		m_targetScale = GameLogicRandomValueReal(data->m_targetScaleMin, data->m_targetScaleMax);
		//if (s_lastInstanceSpunPositive)
		//{
		//	m_spinRate *= -1.0f;
		//}

		getObject()->setOrientation(GameLogicRandomValueReal(-PI,PI));

		DEBUG_ASSERTCRASH( m_tintEnvPlayFrame < m_dieFrame, ("EMPUpdate::EMPUpdate - you cant play fade after death" ) );

		return;
	}

	//SANITY
	DEBUG_ASSERTCRASH( data, ("EMPUpdate::EMPUpdate - getEMPUpdateModuleData is NULL" ) );
	m_currentScale = 1.0f;
	m_dieFrame = 0;
	m_tintEnvFadeFrames = 0;
	m_tintEnvPlayFrame  = 0;
	//m_spinRate = 0;
	m_targetScale = 1;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
EMPUpdate::~EMPUpdate( void )
{
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

UpdateSleepTime EMPUpdate::update( void )
{
/// @todo srj use SLEEPY_UPDATE here

	Object *obj = getObject();

	const EMPUpdateModuleData *data = getEMPUpdateModuleData();
	Drawable *dr = obj->getDrawable();
	UnsignedInt now = TheGameLogic->getFrame();

	m_currentScale += ( m_targetScale - m_currentScale ) * 0.05f;
	dr->setInstanceScale( m_currentScale );

	if ( now < m_tintEnvPlayFrame)
	{
		RGBColor start = data->m_startColor;
		saturateRGB( start, 2 );
		dr->colorTint( &start );
	}
	if ( now == m_tintEnvPlayFrame)
	{
		RGBColor end = data->m_endColor;
		saturateRGB( end, 5 );
		dr->colorFlash( &end, 9999, m_tintEnvFadeFrames, TRUE );
		doDisableAttack();
	}

	//Real curSpin = obj->getOrientation();
	//curSpin += m_spinRate;
	//curSpin = normalizeAngle(curSpin);
	//obj->setOrientation( curSpin );

	if( now >= m_dieFrame )
		obj->kill();

	return UPDATE_SLEEP_NONE;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void EMPUpdate::doDisableAttack( void )
{
	Object *object = getObject();
	const EMPUpdateModuleData *data = getEMPUpdateModuleData();
	if( !object || !data )
		return; //sanity

	Real radius = 200.0f; ///@todo kluge
	Real curVictimDistSqr;
	const Coord3D *pos = object->getPosition();

	SimpleObjectIterator *iter = NULL;
	Object *curVictim = NULL;

	if (radius > 0.0f)
	{
		iter = ThePartitionManager->iterateObjectsInRange(pos,
			radius, FROM_BOUNDINGSPHERE_3D);

		curVictim = iter->firstWithNumeric(&curVictimDistSqr);
	}

	MemoryPoolObjectHolder hold(iter);

	for ( ; curVictim != NULL; curVictim = iter ? iter->nextWithNumeric(&curVictimDistSqr) : NULL)
	{
		if ( curVictim != object)
		{
			if ( !curVictim->isKindOf( KINDOF_VEHICLE ) && !curVictim->isKindOf(KINDOF_STRUCTURE) && !curVictim->isKindOf(KINDOF_SPAWNS_ARE_THE_WEAPONS) )
			{
				//DONT DISABLE PEOPLE, EXCEPT FOR STINGER SOLDIERS
				continue;
			}
			else if ( curVictim->isKindOf( KINDOF_AIRCRAFT ) && curVictim->isAirborneTarget() )
			{
				if ( curVictim->isKindOf( KINDOF_TRANSPORT ) && curVictim->getRelationship( object ) == ALLIES)
					continue;//DONT DISABLE YOUR OWN TRANSPORT PLANES

				curVictim->kill();// @todo this should use some sort of DEADSTICK DIE or something...
				Drawable *drw = curVictim->getDrawable();
				if ( drw )
				{
					drw->setTintStatus( TINT_STATUS_DISABLED );// paint it black
				}
				continue;
			}
			else if ( curVictim->isKindOf( KINDOF_STRUCTURE ) )
			{
				if ( ! curVictim->isFactionStructure() )
					continue;
			}

			//Disable the target for a specified amount of time.
			curVictim->setDisabledUntil( DISABLED_EMP, TheGameLogic->getFrame() + data->m_disabledDuration );


			Drawable *drw = curVictim->getDrawable();
			if ( drw )
			{

				const ParticleSystemTemplate *tmp = data->m_disableFXParticleSystem;
				if (tmp)
				{
					Real victimHeight = curVictim->getGeometryInfo().getMaxHeightAbovePosition();
					Real victimFootprintArea = curVictim->getGeometryInfo().getFootprintArea();
					Real victimVolume = victimFootprintArea * MIN(victimHeight, 10.0f);

					UnsignedInt emitterCount = MAX(15, REAL_TO_INT_CEIL(data->m_sparksPerCubicFoot * victimVolume));

					for (UnsignedInt e = 0 ; e < emitterCount; ++e)
					{

						ParticleSystem *sys = TheParticleSystemManager->createParticleSystem(tmp);

						if (sys)
						{
							Coord3D offs = {0,0,0};
							curVictim->getGeometryInfo().makeRandomOffsetWithinFootprint( offs );
							offs.z = GameLogicRandomValue(3, victimHeight);

							//This puts all the sparks within a quadrahemicycloid (rectangular dome) volume
							//The same shape as a four cornered camping dome tent, for those with less Greek
							if (offs.length() > victimHeight)
							{
								Real resoreX = offs.x;
								Real resoreY = offs.y;
								offs.normalize();
								offs.z *= victimHeight;
								offs.x = resoreX;
								offs.y = resoreY;
							}

							sys->attachToObject(curVictim);
							sys->setPosition( &offs );
							sys->setSystemLifetime(MAX(0, data->m_disabledDuration - 30));
							sys->setInitialDelay(GameLogicRandomValue(1,100));
						}
					}
				}
			}

		}
	}

}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void EMPUpdate::crc( Xfer *xfer )
{

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void EMPUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void EMPUpdate::loadPostProcess( void )
{

}
