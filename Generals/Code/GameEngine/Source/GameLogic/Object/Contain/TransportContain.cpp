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

// FILE: TransportContain.cpp //////////////////////////////////////////////////////////////////////
// Author: Steven Johnson, March 2002
// Desc:   Contain module for transport units.
///////////////////////////////////////////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/Player.h"
#include "Common/ThingTemplate.h"
#include "Common/ThingFactory.h"
#include "Common/Xfer.h"
#include "GameClient/Drawable.h"
#include "GameLogic/AI.h"
#include "GameLogic/AIPathfind.h"
#include "GameLogic/Locomotor.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/Module/PhysicsUpdate.h"
#include "GameLogic/Module/StealthUpdate.h"
#include "GameLogic/Module/TransportContain.h"
#include "GameLogic/Object.h"


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
TransportContainModuleData::TransportContainModuleData()
{

	m_slotCapacity = 0;
	m_scatterNearbyOnExit = true;
	m_orientLikeContainerOnExit = false;
	m_keepContainerVelocityOnExit = false;
	m_goAggressiveOnExit = FALSE;
	m_resetMoodCheckTimeOnExit = true;
	m_destroyRidersWhoAreNotFreeToExit = false;
	m_exitPitchRate = 0.0f;
	m_initialPayload.count = 0;
	m_healthRegen = 0.0f;
	m_exitDelay = 0;

	//
	// by default we say that transports can have infantry inside them, this will be totally
	// overwritten by any data provided from the INI entry tho
	//
	m_allowInsideKindOf = MAKE_KINDOF_MASK( KINDOF_INFANTRY );

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void TransportContainModuleData::parseInitialPayload( INI* ini, void *instance, void *store, const void* /*userData*/ )
{
	TransportContainModuleData* self = (TransportContainModuleData*)instance;
	const char* name = ini->getNextToken();
	const char* countStr = ini->getNextTokenOrNull();
	Int count = countStr ? INI::scanInt(countStr) : 1;

	self->m_initialPayload.name.set(name);
	self->m_initialPayload.count = count;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void TransportContainModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  OpenContainModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "Slots",	INI::parseInt,		NULL, offsetof( TransportContainModuleData, m_slotCapacity ) },
		{ "ScatterNearbyOnExit",	INI::parseBool,		NULL, offsetof( TransportContainModuleData, m_scatterNearbyOnExit ) },
		{ "OrientLikeContainerOnExit",	INI::parseBool,		NULL, offsetof( TransportContainModuleData, m_orientLikeContainerOnExit ) },
		{ "KeepContainerVelocityOnExit",	INI::parseBool,		NULL, offsetof( TransportContainModuleData, m_keepContainerVelocityOnExit ) },
		{ "GoAggressiveOnExit",	INI::parseBool,		NULL, offsetof( TransportContainModuleData, m_goAggressiveOnExit ) },
		{ "ResetMoodCheckTimeOnExit",	INI::parseBool,		NULL, offsetof( TransportContainModuleData, m_resetMoodCheckTimeOnExit ) },
		{ "DestroyRidersWhoAreNotFreeToExit",	INI::parseBool,		NULL, offsetof( TransportContainModuleData, m_destroyRidersWhoAreNotFreeToExit ) },
		{ "ExitBone",	INI::parseAsciiString,		NULL, offsetof( TransportContainModuleData, m_exitBone ) },
		{ "ExitPitchRate",	INI::parseAngularVelocityReal,		NULL, offsetof( TransportContainModuleData, m_exitPitchRate ) },
		{ "InitialPayload", parseInitialPayload, NULL, 0 },
		{ "HealthRegen%PerSec", INI::parseReal, NULL, offsetof( TransportContainModuleData, m_healthRegen ) },
		{ "ExitDelay",	INI::parseDurationUnsignedInt,		NULL, offsetof( TransportContainModuleData, m_exitDelay ) },
		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}



// PRIVATE ////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Int TransportContain::getContainMax( void ) const
{
	if (getTransportContainModuleData())
		return getTransportContainModuleData()->m_slotCapacity;

	return 0;
}

// PUBLIC /////////////////////////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
TransportContain::TransportContain( Thing *thing, const ModuleData *moduleData ) :
								 OpenContain( thing, moduleData )
{
	m_extraSlotsInUse = 0;
	m_frameExitNotBusy = 0;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
TransportContain::~TransportContain( void )
{

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/**
	can this container contain this kind of object?
	and, if checkCapacity is TRUE, does this container have enough space left to hold the given unit?
*/
Bool TransportContain::isValidContainerFor(const Object* rider, Bool checkCapacity) const
{

	// sanity
	if (!rider)
		return false;

	// The point of this new code is to determine when something is a "fake" container, to
	// look at the object inside of it to use that as the valid check. There is a case, when a
	// paratrooper (an infantry contained in a parachute). In this case, when we pass this object
	// to contain in a transport plane, we want to check the infantry, not the parachute.
	if (rider->getContain() && rider->getContain()->isSpecialZeroSlotContainer())
	{
		// Report the first thing inside it!
		const ContainedItemsList *items = rider->getContain()->getContainedItemsList();
		if (items && !items->empty())
		{
			if (items->front())
			{
				// Replace the object we are checking with the *first* object contained within it.
				rider = items->front();
			}
		}
	}


	// extend functionality
	if( OpenContain::isValidContainerFor( rider, checkCapacity ) == false )
		return false;

//	// only allied objects can be transported.
//	// order matters: we want to know if I consider it to be an ally, not vice versa
//	if (getObject()->getRelationship(rider) != ALLIES)
//		return false;

// no... actually, only OUR OWN units can be transported.
	if (rider->getControllingPlayer() != getObject()->getControllingPlayer())
		return false;

	Int transportSlotCount = rider->getTransportSlotCount();

	// if 0, this object isn't transportable.
	if (transportSlotCount == 0)
		return false;

	if (checkCapacity)
	{
		return (m_extraSlotsInUse + getContainCount() + transportSlotCount <= getContainMax());
	}
	else
	{
		return true;
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void TransportContain::onContaining( Object *rider )
{
	OpenContain::onContaining(rider);

	// objects inside a transport are held
	rider->setDisabled( DISABLED_HELD );

	Int transportSlotCount = rider->getTransportSlotCount();

	DEBUG_ASSERTCRASH(transportSlotCount > 0, ("Hmm, this object isnt transportable"));
	m_extraSlotsInUse += transportSlotCount - 1;
	DEBUG_ASSERTCRASH(m_extraSlotsInUse >= 0 && m_extraSlotsInUse + getContainCount() <= getContainMax(), ("Hmm, bad slot count"));

	//
	// when we go from holding nothing to holding something we have a model condition
	// to visually show the change
	//
	if( getContainCount() == 1 )
	{
		Drawable *draw = getObject()->getDrawable();

		if( draw )
			draw->setModelConditionState( MODELCONDITION_LOADED );

	}

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void TransportContain::onRemoving( Object *rider )
{
	OpenContain::onRemoving(rider);

	// object is no longer held inside a transport
	rider->clearDisabled( DISABLED_HELD );

	const TransportContainModuleData* d = getTransportContainModuleData();

	if (!d->m_exitBone.isEmpty())
	{
		Drawable* draw = getObject()->getDrawable();
		if (draw)
		{
			Coord3D bonePos, worldPos;
			if (draw->getPristineBonePositions(d->m_exitBone.str(), 0, &bonePos, NULL, 1) == 1)
			{
				getObject()->convertBonePosToWorldPos(&bonePos, NULL, &worldPos, NULL);
				rider->setPosition(&worldPos);
			}
		}
	}

	if (d->m_orientLikeContainerOnExit)
	{
		rider->setOrientation(getObject()->getOrientation());
	}

	if (d->m_keepContainerVelocityOnExit)
	{
		PhysicsBehavior* parent = getObject()->getPhysics();
		PhysicsBehavior* child = rider->getPhysics();
		if (parent && child)
		{
			Coord3D startingForce = *parent->getVelocity();
			Real mass = child->getMass();
			startingForce.x *= mass;
			startingForce.y *= mass;
			startingForce.z *= mass;
			child->applyMotiveForce( &startingForce );

			Real pitchRate = child->getCenterOfMassOffset() * d->m_exitPitchRate;
			child->setPitchRate( pitchRate );
		}
	}

	Int transportSlotCount = rider->getTransportSlotCount();
	DEBUG_ASSERTCRASH(transportSlotCount > 0, ("Hmm, this object isnt transportable"));
	m_extraSlotsInUse -= transportSlotCount - 1;
	DEBUG_ASSERTCRASH(m_extraSlotsInUse >= 0 && m_extraSlotsInUse + getContainCount() <= getContainMax(), ("Hmm, bad slot count"));

	// when we are empty again, clear the model condition for loaded
	if( getContainCount() == 0 )
	{
		Drawable *draw = getObject()->getDrawable();

		if( draw )
			draw->clearModelConditionState( MODELCONDITION_LOADED );

	}

	if (getObject()->isAboveTerrain())
	{
		// temporarily mark the guy as being allowed to fall
		// (overriding his locomotor's stick-to-ground attribute).
		// this will be reset (by PhysicsBehavior) when he touches the ground.
		PhysicsBehavior* physics = rider->getPhysics();
		if (physics)
			physics->setAllowToFall(true);
	}

	// AI might need help using this transport in a good way.  Make the passengers aggressive.
	//There is no computer player check since Aggressive only means something for computer players anyway
	if( d->m_goAggressiveOnExit && rider->getAI() )
	{
		rider->getAI()->setAttitude( ATTITUDE_AGGRESSIVE );
	}
	if (getObject()->isEffectivelyDead()) {
		scatterToNearbyPosition(rider);
	}
	if( d->m_resetMoodCheckTimeOnExit && rider->getAI() )
	{
		rider->getAI()->wakeUpAndAttemptToTarget();
	}

	m_frameExitNotBusy = TheGameLogic->getFrame() + d->m_exitDelay;

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void TransportContain::createPayload()
{
	TransportContainModuleData* self = (TransportContainModuleData*)getTransportContainModuleData();

	Int count = self->m_initialPayload.count;
	const ThingTemplate* payloadTemplate = TheThingFactory->findTemplate( self->m_initialPayload.name );
	Object* object = getObject();
	ContainModuleInterface *contain = object->getContain();

	if( contain )
	{
		contain->enableLoadSounds( FALSE );
		for( int i = 0; i < count; i++ )
		{
			//We are creating a transport that comes with a initial payload, so add it now!
			Object* payload = TheThingFactory->newObject( payloadTemplate, object->getControllingPlayer()->getDefaultTeam() );
			if( contain->isValidContainerFor( payload, true ) )
			{
				contain->addToContain( payload );
			}
			else
			{
				DEBUG_CRASH( ( "DeliverPayload: PutInContainer %s is full, or not valid for the payload %s!", object->getName().str(), self->m_initialPayload.name.str() ) );
			}
		}
		contain->enableLoadSounds( TRUE );
	}

	m_payloadCreated = TRUE;

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
UpdateSleepTime TransportContain::update()
{
	const TransportContainModuleData *moduleData = getTransportContainModuleData();

	if( m_payloadCreated == FALSE )
		createPayload();

	if( moduleData && moduleData->m_healthRegen )
	{
		ContainModuleInterface *contain = getObject()->getContain();
		if( contain )
		{
			//This transport has a health regeneration value, so go through and heal all inside.
			const ContainedItemsList* items = contain->getContainedItemsList();
			if( items )
			{
				ContainedItemsList::const_iterator it;
				it = items->begin();

				while( it != items->end() )
				{
					Object *object = *it;

					//Advance to the next iterator
					it++;

					//Determine if we need healing or not.
					BodyModuleInterface *body = object->getBodyModule();
					if( body->getHealth() < body->getMaxHealth() )
					{
						//Calculate the health to be regenerated on each unit.
						Real regen = body->getMaxHealth() * moduleData->m_healthRegen / 100.0f * SECONDS_PER_LOGICFRAME_REAL;

						//Perform the actual healing for this frame.
//						DamageInfo damageInfo;
//						damageInfo.in.m_damageType = DAMAGE_HEALING;
//						damageInfo.in.m_deathType = DEATH_NONE;
//						damageInfo.in.m_sourceID = getObject()->getID();
//						damageInfo.in.m_amount = regen;
//						object->attemptDamage( &damageInfo );
						object->attemptHealing( regen, getObject() );
					}
				}
			}
		}
	}

	return OpenContain::update(); //extend
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void TransportContain::unreserveDoorForExit( ExitDoorType exitDoor )
{
	/* nothing */
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void TransportContain::killRidersWhoAreNotFreeToExit()
{
	const TransportContainModuleData* d = getTransportContainModuleData();
	ContainedItemsList::const_iterator it = getContainList().begin();
	while( it != getContainList().end() )
	{
		// get the object
		Object *obj = *it;
		// increment the iterator, since death will pull this guy from the list somewhere else
		++it;

		if (!isSpecificRiderFreeToExit(obj)) // only TransportContain has a meaningful failure for isFreeToExit
		{
			// If we cannot exit this guy legally, kill the bastard before removeAllContained forces him out.
			if (d->m_destroyRidersWhoAreNotFreeToExit)
				TheGameLogic->destroyObject(obj);
			else
				obj->kill();
		}
	}
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
Bool TransportContain::isSpecificRiderFreeToExit(Object* specificObject)
{
	if( specificObject == NULL )
		return TRUE;	// I can, in general, exit people.

	// This is a override, not an extend.  I will check for game legality for
	// okaying the call to exitObjectViaDoor.
  const Object* me = getObject();

	// this is present solely for some transports to override, so that they can land before
	// allowing people to exit...
	const AIUpdateInterface* ai = me->getAIUpdateInterface();
	if (ai && ai->getAiFreeToExit(specificObject) != FREE_TO_EXIT)
		return FALSE;

  // I can always kick people out if I am in the air, I know what I'm doing
  if (me->isUsingAirborneLocomotor())
   	return TRUE;

  const Coord3D *myPosition = me->getPosition();
 	if (!specificObject->getAIUpdateInterface())
		return FALSE;

	const Locomotor *hisLocomotor = specificObject->getAIUpdateInterface()->getCurLocomotor();
	if( hisLocomotor == FALSE )
   	return FALSE;

  // He can't get to this spot naturally, so I can't force him there.  (amphib transport)
  if (!TheAI->pathfinder()->validMovementTerrain(me->getLayer(), hisLocomotor, myPosition))
   	return FALSE;

  return TRUE;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
ExitDoorType TransportContain::reserveDoorForExit( const ThingTemplate* objType, Object *specificObject )
{
	return isSpecificRiderFreeToExit(specificObject) ? DOOR_1 : DOOR_NONE_AVAILABLE;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
Bool TransportContain::isExitBusy() const	///< Contain style exiters are getting the ability to space out exits, so ask this before reserveDoor as a kind of no-commitment check.
{
	return TheGameLogic->getFrame() < m_frameExitNotBusy;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void TransportContain::onCapture( Player *oldOwner, Player *newOwner )
{
	if( oldOwner != newOwner )
	{
		if( getObject()->isDisabledByType( DISABLED_UNMANNED ) )
		{
			//If this vehicle was sniped, then instantly eject everyone (otherwise, the next guy to eject will recapture it).
			removeAllContained();
		}
		else
		{
			//Use standard
			orderAllPassengersToExit( CMD_FROM_AI );
		}
	}
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void TransportContain::crc( Xfer *xfer )
{

	// extend base class
	OpenContain::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void TransportContain::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	OpenContain::xfer( xfer );

	// payload created
	xfer->xferBool( &m_payloadCreated );

	// extra slots in use
	xfer->xferInt( &m_extraSlotsInUse );

	// frame exit not busy
	xfer->xferUnsignedInt( &m_frameExitNotBusy );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void TransportContain::loadPostProcess( void )
{

	// extend base class
	OpenContain::loadPostProcess();

}
