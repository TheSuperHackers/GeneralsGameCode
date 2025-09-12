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

// FILE: GrantStealthBehavior.cpp ///////////////////////////////////////////////////////////////////////
// Author: Lorenzen
///////////////////////////////////////////////////////////////////////////////////////////////////


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#include "Common/Thing.h"
#include "Common/ThingTemplate.h"
#include "Common/INI.h"
#include "Common/Player.h"
#include "Common/Xfer.h"
#include "GameClient/ParticleSys.h"
#include "GameClient/Anim2D.h"
#include "GameClient/InGameUI.h"
#include "GameLogic/Module/ContainModule.h"
#include "GameLogic/Module/GrantStealthBehavior.h"
#include "GameLogic/Module/StealthUpdate.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"



//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
struct GrantStealthPlayerScanHelper
{
	KindOfMaskType m_kindOfToTest;
	Object *m_theGrantor;
	ObjectPointerList *m_objectList;
};

static void checkForGrantStealth( Object *testObj, void *userData )
{
	GrantStealthPlayerScanHelper *helper = (GrantStealthPlayerScanHelper*)userData;
	ObjectPointerList *listToAddTo = helper->m_objectList;

	if( testObj->isEffectivelyDead() )
		return;

	if( testObj->getControllingPlayer() != helper->m_theGrantor->getControllingPlayer() )
		return;

	if( testObj->isOffMap() )
		return;

	if( !testObj->isAnyKindOf(helper->m_kindOfToTest) )
		return;

	listToAddTo->push_back(testObj);

	if( testObj->getContain() )
	{
		// have to tag visible riders too, or they will float around and look silly.
		Object *rider = (Object*)testObj->getContain()->friend_getRider();
		if( rider )
		{
			listToAddTo->push_back(rider);
		}
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
GrantStealthBehavior::GrantStealthBehavior( Thing *thing, const ModuleData* moduleData ) : UpdateModule( thing, moduleData )
{
	const GrantStealthBehaviorModuleData *d = getGrantStealthBehaviorModuleData();

	m_radiusParticleSystemID = INVALID_PARTICLE_SYSTEM_ID;

  m_currentScanRadius = d->m_startRadius;


  Object *obj = getObject();

	{
		if( d->m_radiusParticleSystemTmpl )
		{
			ParticleSystem *particleSystem;

			particleSystem = TheParticleSystemManager->createParticleSystem( d->m_radiusParticleSystemTmpl );
			if( particleSystem )
			{
				particleSystem->setPosition( obj->getPosition() );
				m_radiusParticleSystemID = particleSystem->getSystemID();
			}
		}
	}

		setWakeFrame( getObject(), UPDATE_SLEEP_NONE );
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
GrantStealthBehavior::~GrantStealthBehavior( void )
{

	if( m_radiusParticleSystemID != INVALID_PARTICLE_SYSTEM_ID )
		TheParticleSystemManager->destroyParticleSystemByID( m_radiusParticleSystemID );

}



//-------------------------------------------------------------------------------------------------
/** The update callback. */
//-------------------------------------------------------------------------------------------------
UpdateSleepTime GrantStealthBehavior::update( void )
{

	Object *self = getObject();

	if ( self->isEffectivelyDead())
		return UPDATE_SLEEP_FOREVER;

	const GrantStealthBehaviorModuleData *d = getGrantStealthBehaviorModuleData();
	// setup scan filters
	PartitionFilterRelationship relationship( self, PartitionFilterRelationship::ALLOW_ALLIES );
	PartitionFilterSameMapStatus filterMapStatus( self );
	PartitionFilterAlive filterAlive;
	PartitionFilter *filters[] = { &relationship, &filterAlive, &filterMapStatus, NULL };


  m_currentScanRadius += d->m_radiusGrowRate;

  Bool thisIsFinalScan = FALSE;
  if ( m_currentScanRadius >=  d->m_finalRadius )
  {
    m_currentScanRadius = d->m_finalRadius;
    thisIsFinalScan = TRUE;
  }

	// scan objects in our region
	ObjectIterator *iter = ThePartitionManager->iterateObjectsInRange( self->getPosition(), m_currentScanRadius, FROM_CENTER_2D, filters );
	MemoryPoolObjectHolder hold( iter );
	// GRANT STEALTH TO FRIENDLIES IN RADIUS
	for( Object *obj = iter->first(); obj; obj = iter->next() )
    grantStealthToObject( obj );

  if ( thisIsFinalScan )
  {

    TheGameLogic->destroyObject( self );
    return UPDATE_SLEEP_FOREVER;
  }

  return UPDATE_SLEEP_NONE;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void GrantStealthBehavior::grantStealthToObject( Object *obj )
{

  if ( obj == getObject() )
    return;


	const GrantStealthBehaviorModuleData *d = getGrantStealthBehaviorModuleData();
  if ( ! obj->isAnyKindOf( d->m_kindOf ) )
    return;

  StealthUpdate* stealth = obj->getStealth();
	if( stealth )
	{
		stealth->receiveGrant();
    Drawable *draw = obj->getDrawable();
    if ( draw )
    {
      draw->flashAsSelected();
    }
	}


}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void GrantStealthBehavior::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );


}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void GrantStealthBehavior::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );


	// particle system id
	xfer->xferUser( &m_radiusParticleSystemID, sizeof( ParticleSystemID ) );

	// Timer safety
	xfer->xferReal( &m_currentScanRadius );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void GrantStealthBehavior::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();


}
