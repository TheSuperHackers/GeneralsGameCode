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

// FILE: MobNexusContain.cpp //////////////////////////////////////////////////////////////////////
// Author: Mark Lorenzen, August 2002
// Desc:   Contain module for mob units.
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
#include "GameLogic/Module/StealthUpdate.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/Module/MobNexusContain.h"
#include "GameLogic/Module/PhysicsUpdate.h"
#include "GameLogic/Object.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
MobNexusContainModuleData::MobNexusContainModuleData()
{

	m_slotCapacity = 0;
	m_scatterNearbyOnExit = true;
	m_orientLikeContainerOnExit = false;
	m_keepContainerVelocityOnExit = false;
	m_exitPitchRate = 0.0f;
	m_initialPayload.count = 0;
	m_healthRegen = 0.0f;

	//
	// by default we say that MobNexae can have infantry inside them, this will be totally
	// overwritten by any data provided from the INI entry tho
	//
	m_allowInsideKindOf = MAKE_KINDOF_MASK( KINDOF_INFANTRY );

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void MobNexusContainModuleData::parseInitialPayload( INI* ini, void *instance, void *store, const void* /*userData*/ )
{
	MobNexusContainModuleData* self = (MobNexusContainModuleData*)instance;
	const char* name = ini->getNextToken();
	const char* countStr = ini->getNextTokenOrNull();
	Int count = countStr ? INI::scanInt(countStr) : 1;
	
	self->m_initialPayload.name.set(name);
	self->m_initialPayload.count = count;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void MobNexusContainModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  OpenContainModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] = 
	{
		{ "Slots",	INI::parseInt,		NULL, offsetof( MobNexusContainModuleData, m_slotCapacity ) },
		{ "ScatterNearbyOnExit",	INI::parseBool,		NULL, offsetof( MobNexusContainModuleData, m_scatterNearbyOnExit ) },
		{ "OrientLikeContainerOnExit",	INI::parseBool,		NULL, offsetof( MobNexusContainModuleData, m_orientLikeContainerOnExit ) },
		{ "KeepContainerVelocityOnExit",	INI::parseBool,		NULL, offsetof( MobNexusContainModuleData, m_keepContainerVelocityOnExit ) },
		{ "ExitBone",	INI::parseAsciiString,		NULL, offsetof( MobNexusContainModuleData, m_exitBone ) },
		{ "ExitPitchRate",	INI::parseAngularVelocityReal,		NULL, offsetof( MobNexusContainModuleData, m_exitPitchRate ) },
		{ "InitialPayload", parseInitialPayload, NULL, 0 },
		{ "HealthRegen%PerSec", INI::parseReal, NULL, offsetof( MobNexusContainModuleData, m_healthRegen ) },
		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}



// PRIVATE ////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Int MobNexusContain::getContainMax( void ) const 
{ 
	if (getMobNexusContainModuleData())
		return getMobNexusContainModuleData()->m_slotCapacity;

	return 0;
}

// PUBLIC /////////////////////////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
MobNexusContain::MobNexusContain( Thing *thing, const ModuleData *moduleData ) : 
								 OpenContain( thing, moduleData )
{
	m_extraSlotsInUse = 0;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
MobNexusContain::~MobNexusContain( void )
{

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/** 
	can this container contain this kind of object? 
	and, if checkCapacity is TRUE, does this container have enough space left to hold the given unit?
*/
Bool MobNexusContain::isValidContainerFor(const Object* rider, Bool checkCapacity) const
{

	// sanity
	if (!rider)
		return false;

	//The point of this new code is to determine when something has a negative 1 contain max, to
	//look at the object inside of it to use that as the valid check. There is a case, when a 
	//paratrooper (an infantry contained in a parachute). In this case, when we pass this object
	//to contain in a --- plane, we want to check the infantry, not the parachute.
//	const MobNexusContainModuleData *modData = getMobNexusContainModuleData();

	//Check if we are a fake container, and if so, get an object inside it to see what kind this object *is*.
	if( rider->getContain() && rider->getContain()->isSpecialZeroSlotContainer() )
	{
		//Report the first thing inside it!
		const ContainedItemsList *items = rider->getContain()->getContainedItemsList();
		if( items )
		{
			ContainedItemsList::const_iterator it;
			it = items->begin();
			if( *it )
			{
				//Replace the object we are checking with the *first* object contained within it.
				rider = *it;
			}
		}
	}
	else
	{
		//blech! This case may or may not occur... in which case, just use the supplied object.
	}


	// extend functionality
	if( OpenContain::isValidContainerFor( rider, checkCapacity ) == false )
		return false;

	// only allied objects can be MobNexused.
	// order matters: we want to know if IT considers ME to be an ally (a reversal of the usual situation)
	if (rider->getRelationship(getObject()) != ALLIES)
		return false;

	Int mobNexusSlotCount = rider->getTransportSlotCount();

	// if 0, this object isn't MobNexusable.
	if (mobNexusSlotCount == 0)
		return false;

	if (checkCapacity)
	{
		return (m_extraSlotsInUse + getContainCount() + mobNexusSlotCount <= getContainMax());
	}
	else
	{
		return true;
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void MobNexusContain::onContaining( Object *rider, Bool wasSelected )
{
	OpenContain::onContaining( rider, wasSelected );

	// objects inside a MobNexus are held
	rider->setDisabled( DISABLED_HELD );

	Int mobNexusSlotCount = rider->getTransportSlotCount();

	DEBUG_ASSERTCRASH(mobNexusSlotCount > 0, ("Hmm, this object isnt MobNexusable"));
	m_extraSlotsInUse += mobNexusSlotCount - 1;
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

	}  // end if

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void MobNexusContain::onRemoving( Object *rider )
{
	OpenContain::onRemoving(rider);

	// object is no longer held inside a MobNexus
	rider->clearDisabled( DISABLED_HELD );

	const MobNexusContainModuleData* d = getMobNexusContainModuleData();

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

	if (d->m_scatterNearbyOnExit)
		scatterToNearbyPosition(rider);

	Int mobNexusSlotCount = rider->getTransportSlotCount();
	DEBUG_ASSERTCRASH(mobNexusSlotCount > 0, ("This object isnt MobNexusable"));
	m_extraSlotsInUse -= mobNexusSlotCount - 1;
	DEBUG_ASSERTCRASH(m_extraSlotsInUse >= 0 && m_extraSlotsInUse + getContainCount() <= getContainMax(), ("Bad slot count, MobNexus"));

	// when we are empty again, clear the model condition for loaded
	if( getContainCount() == 0 )
	{
		Drawable *draw = getObject()->getDrawable();

		if( draw )
			draw->clearModelConditionState( MODELCONDITION_LOADED );

	}  // end if

	if (getObject()->isAboveTerrain())
	{
		// temporarily mark the guy as being allowed to fall 
		// (overriding his locomotor's stick-to-ground attribute).
		// this will be reset (by PhysicsBehavior) when he touches the ground.
		PhysicsBehavior* physics = rider->getPhysics();
		if (physics)
			physics->setAllowToFall(true);
	}

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void MobNexusContain::onObjectCreated()
{
	MobNexusContainModuleData* self = (MobNexusContainModuleData*)getMobNexusContainModuleData();

	Int count = self->m_initialPayload.count;
	const ThingTemplate* payloadTemplate = TheThingFactory->findTemplate( self->m_initialPayload.name );
	Object* object = getObject();

	for( int i = 0; i < count; i++ )
	{
		//We are creating a MobNexus that comes with a initial payload, so add it now!

		Object* payload = TheThingFactory->newObject( payloadTemplate, object->getControllingPlayer()->getDefaultTeam() );
		if( object->getContain() && object->getContain()->isValidContainerFor( payload, true ) )
		{
			object->getContain()->addToContain( payload );
		}
		else
		{
			DEBUG_CRASH( ( "DeliverPayload: PutInContainer %s is full, or not valid for the payload %s!", object->getName().str(), self->m_initialPayload.name.str() ) );
		}
	}
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
UpdateSleepTime MobNexusContain::update()
{
	MobNexusContainModuleData *moduleData = (MobNexusContainModuleData*)getModuleData();

	if( moduleData && moduleData->m_healthRegen )
	{
		ContainModuleInterface *contain = getObject()->getContain();
		if( contain )
		{
			//This MobNexus has a health regeneration value, so go through and heal all inside.
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
ExitDoorType MobNexusContain::reserveDoorForExit( const ThingTemplate* objType, Object *specificObject )
{
	if( specificObject == NULL )
		return DOOR_1;// I can, in general, exit people.

	// This is an override, not an extend.  I will check for game legality for
	// okaying the call to exitObjectViaDoor.
  Object *me = getObject();
 
	// this is present solely for some MobNexuss to override, so that they can land before 
	// allowing people to exit...
	AIUpdateInterface* ai = me->getAIUpdateInterface();
	if (ai && ai->getAiFreeToExit(me) != FREE_TO_EXIT)
		return DOOR_NONE_AVAILABLE;

  // I can always kick people out if I am in the air, I know what I'm doing
  if( me->isUsingAirborneLocomotor() )
   	return DOOR_1;
 
  const Coord3D *myPosition = me->getPosition();
 	if( !specificObject->getAIUpdateInterface() )
	{
		return DOOR_NONE_AVAILABLE;
	}
	const Locomotor *hisLocomotor = specificObject->getAIUpdateInterface()->getCurLocomotor();
  // He can't get to this spot naturally, so I can't force him there.  (amphib MobNexus)
  if( ! TheAI->pathfinder()->validMovementTerrain(me->getLayer(), hisLocomotor, myPosition ) )
   	return DOOR_NONE_AVAILABLE;
 
  return DOOR_1;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void MobNexusContain::unreserveDoorForExit( ExitDoorType exitDoor )
{
	/* nothing */
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
Bool MobNexusContain::tryToEvacuate( Bool exposeStealthedUnits )
{
	Bool exitedAnyone = false;
	ContainedItemsList::const_iterator it = getContainList().begin();
	while( it != getContainList().end() )
	{
		// get the object
		Object *obj = *it;
		// increment the iterator, since removal will pull this guy from the list somewhere else
		// and we might not actually kick anyone so we don't want to loop forever.
		++it;

		ExitDoorType exitDoor = reserveDoorForExit(obj->getTemplate(), obj);
		if(exitDoor != DOOR_NONE_AVAILABLE)
		{
			exitObjectViaDoor( obj, exitDoor );
			exitedAnyone = true;

			if( obj->isKindOf( KINDOF_STEALTH_GARRISON ) && exposeStealthedUnits )
			{
				StealthUpdate* stealth = obj->getStealth();
				if( stealth )
				{
					stealth->markAsDetected();
				}
			}
		}
	}
	return exitedAnyone;
}

// ------------------------------------------------------------------------------------------------
/** CRC - I like the word nexus */
// ------------------------------------------------------------------------------------------------
void MobNexusContain::crc( Xfer *xfer )
{

	// extend base class
	OpenContain::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void MobNexusContain::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	OpenContain::xfer( xfer );

	// extra slots in use
	xfer->xferInt( &m_extraSlotsInUse );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void MobNexusContain::loadPostProcess( void )
{

	// extend base class
	OpenContain::loadPostProcess();

}  // end loadPostProcess
