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

// FILE: StickyBombUpdate.cpp //////////////////////////////////////////////////////////////////////
// Author: Kris Morness, July 2002
// Desc:   Updates stickybomb to stay on target while it counts down to detonation (lifetime update)
///////////////////////////////////////////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "GameLogic/Module/StickyBombUpdate.h"

#include "Common/ThingTemplate.h"
#include "Common/Player.h"
#include "Common/Xfer.h"
#include "GameClient/Drawable.h"
#include "GameClient/FXList.h"
#include "GameClient/InGameUI.h"
#include "GameLogic/Object.h"
#include "GameLogic/ObjectIter.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/Weapon.h"
#include "GameLogic/Module/LifetimeUpdate.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/GameLogic.h"

// PRIVATE ////////////////////////////////////////////////////////////////////////////////////////

// PUBLIC /////////////////////////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
StickyBombUpdate::StickyBombUpdate( Thing *thing, const ModuleData *moduleData ) : UpdateModule( thing, moduleData )
{
	m_targetID		= INVALID_ID;
	m_dieFrame		= 0;
	//Added By Sadullah Nader
	//Initialization(s) inserted
	m_nextPingFrame = 0;
	//
	setWakeFrame(getObject(), UPDATE_SLEEP_FOREVER);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
StickyBombUpdate::~StickyBombUpdate( void )
{
}

//-------------------------------------------------------------------------------------------------
void StickyBombUpdate::onObjectCreated()
{
	//This first step is an initialization step. Immediately after the stickybomb is created,
	//the producerID is stored in the object to signify the shooter of this weapon. From there,
	//we can get the target. The target information will be used to determine where the sticky
	//bomb will go.
	ObjectID shooterID = getObject()->getProducerID();
	Object* shooter = TheGameLogic->findObjectByID( shooterID );
	if( shooter )
	{
		//Find the shooters target!
		AIUpdateInterface *ai = shooter->getAIUpdateInterface();
		if( ai )
		{
			Object *target = ai->getGoalObject();
			if( target )
			{
				initStickyBomb( target, NULL);
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------
void StickyBombUpdate::initStickyBomb( Object *target, const Object *bomber, const Coord3D *specificPos )
{
	//Store the target.
	m_targetID = target ? target->getID() : INVALID_ID;

	//Set the target as the producer ID -- so we can access it from other areas.
	getObject()->setProducer( target );
	UnsignedInt now = TheGameLogic->getFrame();

	//Also determine our lifetime.... for countdown purposes.
	static NameKeyType key_LifetimeUpdate = NAMEKEY( "LifetimeUpdate" );
	LifetimeUpdate *update = (LifetimeUpdate*)getObject()->findUpdateModule( key_LifetimeUpdate );
	if( update )
	{
		//we are a timer bomb
		m_dieFrame = update->getDieFrame();

		//Calculate the number of seconds (rounded down)
		UnsignedInt pings = (m_dieFrame - now) / LOGICFRAMES_PER_SECOND;
		//Now determine the next frame we will make a "ping" sound.
		m_nextPingFrame = m_dieFrame - (pings * LOGICFRAMES_PER_SECOND);
	}
	else
	{
		//we are a remotely triggered bomb.
		m_dieFrame = 0;

		//Because we don't die -- make our ping in a second.
		m_nextPingFrame = now + LOGICFRAMES_PER_SECOND;
	}
	setWakeFrame( getObject(), UPDATE_SLEEP_NONE );

	if( target )
	{
		const StickyBombUpdateModuleData* d = getStickyBombUpdateModuleData();
		Coord3D pos = *target->getPosition();

		if( specificPos )
		{
			pos = *specificPos;
			pos.z = TheTerrainLogic->getGroundHeight(pos.x, pos.y);
		}
		else if(target->isKindOf( KINDOF_IMMOBILE ) && bomber )
		{
			// make this exception, if bomber has placed bomb on a structure
			// let the bomb just stay where it was first put, so a mine clearing unit can get to it later
			pos = *bomber->getPosition();
			pos.z = TheTerrainLogic->getGroundHeight(pos.x, pos.y);
			//keep it at ground height for mine clearing units to reach
		}
		else
			pos.z += d->m_offsetZ; // ride on the roof of the truck/tank

		getObject()->setPosition( &pos );

		if( getObject()->isKindOf(KINDOF_BOOBY_TRAP) )
		{
			// This kind of sticky bomb needs to set a status, so the poor victim can trigger us from assorted places
			target->setStatus( MAKE_OBJECT_STATUS_MASK(OBJECT_STATUS_BOOBY_TRAPPED) );
		}

		AudioEventRTS soundCreateBomb = *(getObject()->getTemplate()->getPerUnitSound("StickyBombCreated"));
		soundCreateBomb.setPosition( getObject()->getPosition() );
		TheAudio->addAudioEvent(&soundCreateBomb);

	}
}

//-------------------------------------------------------------------------------------------------
UpdateSleepTime StickyBombUpdate::update( void )
{
	// Continually reset position of stickybomb to match the position of the target.
	const Object *target = getTargetObject();
	Object *self = getObject();
	if( target )
	{
		if( target->isEffectivelyDead() )
		{
			//If the target is dead, then
			TheGameLogic->destroyObject( self );
			return UPDATE_SLEEP_NONE;
		}

		if ( target->isKindOf( KINDOF_IMMOBILE) )
		{
			const Coord3D *pos = self->getPosition();
			Coord3D newPos;
			newPos.x = pos->x;
			newPos.y = pos->y;
			newPos.z = TheTerrainLogic->getGroundHeight(newPos.x, newPos.y);
			//keep it at ground height for mine clearing units to reach

			self->setPosition( &newPos );
		}
		else // make the bomb follow the target around
		{
			const StickyBombUpdateModuleData* d = getStickyBombUpdateModuleData();
			const Coord3D *pos = target->getPosition();
			Coord3D newPos;
			newPos.x = pos->x;
			newPos.y = pos->y;
			newPos.z = pos->z + d->m_offsetZ;

			self->setPosition( &newPos );
		}
	}

	UnsignedInt now = TheGameLogic->getFrame();
	if( now >= m_nextPingFrame )
	{
		m_nextPingFrame += LOGICFRAMES_PER_SECOND;

		//Play the "ping" sound.
		AudioEventRTS sound = *self->getTemplate()->getPerUnitSound( "UnitBombPing" );
		sound.setObjectID( self->getID() );
		TheAudio->addAudioEvent( &sound );
	}

	return UPDATE_SLEEP_NONE;
}

//-------------------------------------------------------------------------------------------------
Object* StickyBombUpdate::getTargetObject() const
{
	return TheGameLogic->findObjectByID( m_targetID );
}

//-------------------------------------------------------------------------------------------------
void StickyBombUpdate::setTargetObject( Object *obj )
{
	m_targetID = obj ? obj->getID() : INVALID_ID;
}

//-------------------------------------------------------------------------------------------------
void StickyBombUpdate::detonate()
{
	const StickyBombUpdateModuleData *data = getStickyBombUpdateModuleData();

	Object* boobyTrappedObject = getTargetObject();
	if( data->m_geometryBasedDamageWeaponTemplate )
	{
		// We need to hurt people based on the size of the thing we are on.  The radius in our weapon
		// is the radius beyond our borders
		if( boobyTrappedObject )
		{
			WeaponBonus nullBonus;
			Real boundingCircle = boobyTrappedObject->getGeometryInfo().getBoundingCircleRadius();
			Real primaryDamage = data->m_geometryBasedDamageWeaponTemplate->getPrimaryDamage(nullBonus);
			Real secondaryDamage = data->m_geometryBasedDamageWeaponTemplate->getSecondaryDamage(nullBonus);
			Real primaryDamageRange = data->m_geometryBasedDamageWeaponTemplate->getPrimaryDamageRadius(nullBonus);
			Real secondaryDamageRange = data->m_geometryBasedDamageWeaponTemplate->getSecondaryDamageRadius(nullBonus);
			primaryDamageRange += boundingCircle;
			secondaryDamageRange += boundingCircle;
			Real primaryDamageRangeSqr = sqr(primaryDamageRange);
			Real radius = max(primaryDamageRange, secondaryDamageRange);

			SimpleObjectIterator *iter;
			iter = ThePartitionManager->iterateObjectsInRange(boobyTrappedObject->getPosition(), radius, FROM_BOUNDINGSPHERE_3D);
			MemoryPoolObjectHolder hold(iter);
			Real curVictimDistSqr;
			Object *curVictim = iter->firstWithNumeric(&curVictimDistSqr);
			DamageInfo damageInfo;
			damageInfo.in.m_damageType = data->m_geometryBasedDamageWeaponTemplate->getDamageType();
			damageInfo.in.m_deathType = data->m_geometryBasedDamageWeaponTemplate->getDeathType();
			damageInfo.in.m_sourceID = getObject()->getID();
			damageInfo.in.m_sourcePlayerMask = getObject()->getControllingPlayer()->getPlayerMask();
			damageInfo.in.m_damageStatusType = data->m_geometryBasedDamageWeaponTemplate->getDamageStatusType();

			for (; curVictim != NULL; curVictim = iter ? iter->nextWithNumeric(&curVictimDistSqr) : NULL)
			{
				damageInfo.in.m_amount = (curVictimDistSqr <= primaryDamageRangeSqr) ? primaryDamage : secondaryDamage;
				curVictim->attemptDamage(&damageInfo);
			}

			if( data->m_geometryBasedDamageFX )
			{
				// And we make FX based on that size too.
				FXList::doFXPos(data->m_geometryBasedDamageFX, boobyTrappedObject->getPosition(), NULL, 0, NULL, secondaryDamageRange);
			}
		}
	}

	if( getObject()->isKindOf(KINDOF_BOOBY_TRAP) && boobyTrappedObject )
	{
		// This kind of sticky bomb needs to set a status, so the poor victim can trigger us from assorted places
		boobyTrappedObject->clearStatus( MAKE_OBJECT_STATUS_MASK(OBJECT_STATUS_BOOBY_TRAPPED) );
	}

	getObject()->kill();// Most things just fire weapons in their death modules
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void StickyBombUpdate::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void StickyBombUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// target id
	xfer->xferObjectID( &m_targetID );

	// die frame
	xfer->xferUnsignedInt( &m_dieFrame );

	//Next frame that a ping sound will play.
	xfer->xferUnsignedInt( &m_nextPingFrame );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void StickyBombUpdate::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}
