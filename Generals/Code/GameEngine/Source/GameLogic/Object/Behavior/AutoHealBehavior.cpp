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

// FILE: AutoHealBehavior.cpp ///////////////////////////////////////////////////////////////////////
// Author:
// Desc:  
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
#include "GameLogic/Module/AutoHealBehavior.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
struct AutoHealPlayerScanHelper
{
	KindOfMaskType m_kindOfToTest;
	Object *m_theHealer;
	ObjectPointerList *m_objectList;	
};

static void checkForAutoHeal( Object *testObj, void *userData )
{
	AutoHealPlayerScanHelper *helper = (AutoHealPlayerScanHelper*)userData;
	ObjectPointerList *listToAddTo = helper->m_objectList;

	if( testObj->isEffectivelyDead() )
		return;

	if( testObj->getControllingPlayer() != helper->m_theHealer->getControllingPlayer() )
		return;

	if( testObj->isOffMap() )
		return;

	if( !testObj->isAnyKindOf(helper->m_kindOfToTest) )
		return;

	if( testObj->getBodyModule()->getHealth() >= testObj->getBodyModule()->getMaxHealth() )
		return;

	listToAddTo->push_back(testObj);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
AutoHealBehavior::AutoHealBehavior( Thing *thing, const ModuleData* moduleData ) : UpdateModule( thing, moduleData )
{
	const AutoHealBehaviorModuleData *d = getAutoHealBehaviorModuleData();

	m_radiusParticleSystemID = INVALID_PARTICLE_SYSTEM_ID;
	m_soonestHealFrame = 0;
	m_stopped = false;
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

	if (d->m_initiallyActive)
	{
		giveSelfUpgrade();
		// start these guys with random phasings so that we don't
		// have all of 'em check on the same frame.
		UnsignedInt delay = getAutoHealBehaviorModuleData()->m_healingDelay;
		setWakeFrame(getObject(), UPDATE_SLEEP(GameLogicRandomValue(1, delay)));
	}
	else
	{
		setWakeFrame(getObject(), UPDATE_SLEEP_FOREVER);
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
AutoHealBehavior::~AutoHealBehavior( void )
{

	if( m_radiusParticleSystemID != INVALID_PARTICLE_SYSTEM_ID )
		TheParticleSystemManager->destroyParticleSystemByID( m_radiusParticleSystemID );

}

//-------------------------------------------------------------------------------------------------
void AutoHealBehavior::stopHealing()
{
	m_stopped = true;
	m_soonestHealFrame = FOREVER;
	setWakeFrame(getObject(), UPDATE_SLEEP_FOREVER);
}

//-------------------------------------------------------------------------------------------------
void AutoHealBehavior::undoUpgrade()
{
	m_soonestHealFrame = 0;
	setUpgradeExecuted( FALSE );
}

//-------------------------------------------------------------------------------------------------
/** Damage has been dealt, this is an opportunity to reach to that damage */
//-------------------------------------------------------------------------------------------------
void AutoHealBehavior::onDamage( DamageInfo *damageInfo )
{
	if (m_stopped)
		return;

	const AutoHealBehaviorModuleData *d = getAutoHealBehaviorModuleData();
	if (isUpgradeActive() && d->m_radius == 0.0f)
	{
		// if this is nonzero, getting damaged resets our healing process. so go to
		// sleep for this long.
		if (d->m_startHealingDelay > 0)
		{
			setWakeFrame(getObject(), UPDATE_SLEEP(d->m_startHealingDelay));
		}
		else if( TheGameLogic->getFrame() > m_soonestHealFrame )
		{
			// We can only force an immediate wake if we are ready to heal.  Otherwise we will
			// heal on a timer AND at every damage input.
			setWakeFrame(getObject(), UPDATE_SLEEP_NONE);
		}
	}
}

//-------------------------------------------------------------------------------------------------
/** The update callback. */
//-------------------------------------------------------------------------------------------------
UpdateSleepTime AutoHealBehavior::update( void )
{
	if (m_stopped)
		return UPDATE_SLEEP_FOREVER;

	Object *obj = getObject();
	const AutoHealBehaviorModuleData *d = getAutoHealBehaviorModuleData();

	// do not heal if our status bit is not on.
	// do not heal if our status is effectively dead.  There ain't no coming back, man!
	if (!isUpgradeActive() || obj->isEffectivelyDead())
	{
		DEBUG_ASSERTCRASH(isUpgradeActive(), ("hmm, this should not be possible"));
		return UPDATE_SLEEP_FOREVER;
	}

//DEBUG_LOG(("doing auto heal %d",TheGameLogic->getFrame()));

	if( d->m_affectsWholePlayer )
	{
		// Even newer system, I can ignore radius and iterate objects on the owning player.  Faster than scanning range 10,000,000
		ObjectPointerList objectsToHeal;
		Player *owningPlayer = getObject()->getControllingPlayer();
		if( owningPlayer )
		{
			AutoHealPlayerScanHelper helper;
			helper.m_kindOfToTest = getAutoHealBehaviorModuleData()->m_kindOf;
			helper.m_objectList = &objectsToHeal;
			helper.m_theHealer = getObject();

			// Smack all objects with this function, and we will end up with a list of Objects deserving of pulseHealObject
			owningPlayer->iterateObjects( checkForAutoHeal, &helper );

			for( ObjectPointerListIterator iter = objectsToHeal.begin(); iter != objectsToHeal.end(); ++iter )
			{
				pulseHealObject(*iter);
			}
			objectsToHeal.clear();
		}
		return UPDATE_SLEEP(d->m_healingDelay);
	}
	else if( d->m_radius == 0.0f )
	{
		//ORIGINAL SYSTEM -- JUST HEAL SELF!

		// do not heal if we are at max health already
		BodyModuleInterface *body = obj->getBodyModule();
		if( body->getHealth() < body->getMaxHealth() )
		{
  		pulseHealObject( obj );
			return UPDATE_SLEEP(d->m_healingDelay);
		}
		else
		{
			// go to sleep forever -- we'll wake back up when we are damaged again
			return UPDATE_SLEEP_FOREVER;
		}
	}
	else
	{
		//EXPANDED SYSTEM -- HEAL FRIENDLIES IN RADIUS 
		// setup scan filters
		PartitionFilterRelationship relationship( obj, PartitionFilterRelationship::ALLOW_ALLIES );
		PartitionFilterSameMapStatus filterMapStatus(obj);
		PartitionFilterAlive filterAlive;
		PartitionFilter *filters[] = { &relationship, &filterAlive, &filterMapStatus, NULL };

		// scan objects in our region
		ObjectIterator *iter = ThePartitionManager->iterateObjectsInRange( obj->getPosition(), d->m_radius, FROM_CENTER_2D, filters );
		MemoryPoolObjectHolder hold( iter );
		for( obj = iter->first(); obj; obj = iter->next() )
		{
			// do not heal if we are at max health already
			BodyModuleInterface *body = obj->getBodyModule();
			if( body->getHealth() < body->getMaxHealth() && obj->isAnyKindOf( d->m_kindOf ) )
			{
				pulseHealObject( obj );

				if( d->m_singleBurst && TheGameLogic->getDrawIconUI() )
				{
					if( TheAnim2DCollection && TheGlobalData->m_getHealedAnimationName.isEmpty() == FALSE )
					{
						Anim2DTemplate *animTemplate = TheAnim2DCollection->findTemplate( TheGlobalData->m_getHealedAnimationName );

						if ( animTemplate )
						{
							Coord3D iconPosition;
							iconPosition.set(obj->getPosition()->x, 
															 obj->getPosition()->y, 
															 obj->getPosition()->z + obj->getGeometryInfo().getMaxHeightAbovePosition() );
							TheInGameUI->addWorldAnimation( animTemplate,	&iconPosition, WORLD_ANIM_FADE_ON_EXPIRE,
																							TheGlobalData->m_getHealedAnimationDisplayTimeInSeconds,
																							TheGlobalData->m_getHealedAnimationZRisePerSecond);
						}
					}
				}
			
			}
		}  // end for obj

		return UPDATE_SLEEP( d->m_singleBurst ? UPDATE_SLEEP_FOREVER : d->m_healingDelay );
	}
}
 
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void AutoHealBehavior::pulseHealObject( Object *obj )
{
	if (m_stopped)
		return;

	const AutoHealBehaviorModuleData *data = getAutoHealBehaviorModuleData();

	
	if ( data->m_radius == 0.0f )
		obj->attemptHealing(data->m_healingAmount, getObject());
	else
		obj->attemptHealingFromSoleBenefactor( data->m_healingAmount, getObject(), data->m_healingDelay );


	if( data->m_unitHealPulseParticleSystemTmpl )
	{
		ParticleSystem *system = TheParticleSystemManager->createParticleSystem( data->m_unitHealPulseParticleSystemTmpl );
		if( system )
		{
			system->setPosition( obj->getPosition() );
		}
	}
	
	m_soonestHealFrame = TheGameLogic->getFrame() + data->m_healingDelay;// In case onDamage tries to wake us up early
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void AutoHealBehavior::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

	// extend base class
	UpgradeMux::upgradeMuxCRC( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void AutoHealBehavior::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// extend base class
	UpgradeMux::upgradeMuxXfer( xfer );

	// particle system id
	xfer->xferUser( &m_radiusParticleSystemID, sizeof( ParticleSystemID ) );

	// Timer safety
	xfer->xferUnsignedInt( &m_soonestHealFrame );

	// stopped
	xfer->xferBool( &m_stopped );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void AutoHealBehavior::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

	// extend base class
	UpgradeMux::upgradeMuxLoadPostProcess();

}  // end loadPostProcess
