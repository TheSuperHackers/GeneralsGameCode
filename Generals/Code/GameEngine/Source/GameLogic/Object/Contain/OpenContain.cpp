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

// FILE: OpenContain.cpp //////////////////////////////////////////////////////////////////////////
// Author: Colin Day, November 2001
// Desc:   The OpenContain ContainModule allows objects to be contained inside of other
//				 objects.  There is a set of functionality that will be common to 
//				 all container modules that provides the actual containment
//				 implementations, those implementations are found here
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/BitFlagsIO.h"
#include "Common/GameAudio.h"
#include "Common/GameState.h"
#include "Common/Module.h"
#include "Common/Player.h"
#include "Common/RandomValue.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"

#include "GameClient/Drawable.h"
#include "GameClient/InGameUI.h"
#include "GameClient/ControlBar.h"

#include "GameLogic/AIPathfind.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Module/OpenContain.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/PhysicsUpdate.h"
#include "GameLogic/Module/StealthUpdate.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/Weapon.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
OpenContainModuleData::OpenContainModuleData( void )
{

	m_containMax = CONTAIN_MAX_UNKNOWN;  // means we don't care, infinite, unassigned, whatever
	m_passengersAllowedToFire = FALSE;
	m_passengersInTurret = FALSE;
	m_numberOfExitPaths = 1;
	m_damagePercentageToUnits = 0;
	m_doorOpenTime = 1;
	m_allowInsideKindOf.clear(); m_allowInsideKindOf.flip();		// everything is allowed
	m_forbidInsideKindOf.clear();	// nothing is forbidden
 	m_allowAlliesInside = TRUE;
 	m_allowEnemiesInside = TRUE;
 	m_allowNeutralInside = TRUE;
}  // end OpenContainModuleData

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
/*static*/ void OpenContainModuleData::buildFieldParse(MultiIniFieldParse& p) 
{
  UpdateModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] = 
	{
		{ "ContainMax",								INI::parseInt, NULL, offsetof( OpenContainModuleData, m_containMax ) },
		{ "EnterSound",								INI::parseAudioEventRTS,		NULL, offsetof( OpenContainModuleData, m_enterSound ) },
		{ "ExitSound",								INI::parseAudioEventRTS,		NULL, offsetof( OpenContainModuleData, m_exitSound ) },
		{ "DamagePercentToUnits",			INI::parsePercentToReal,		NULL, offsetof( OpenContainModuleData, m_damagePercentageToUnits ) },
		{ "AllowInsideKindOf",				KindOfMaskType::parseFromINI, NULL, offsetof( OpenContainModuleData, m_allowInsideKindOf ) },
		{ "ForbidInsideKindOf",				KindOfMaskType::parseFromINI, NULL, offsetof( OpenContainModuleData, m_forbidInsideKindOf ) },
		{ "PassengersAllowedToFire",	INI::parseBool, NULL, offsetof( OpenContainModuleData, m_passengersAllowedToFire ) },
		{ "PassengersInTurret",				INI::parseBool, NULL, offsetof( OpenContainModuleData, m_passengersInTurret ) },
		{ "NumberOfExitPaths",				INI::parseInt, NULL, offsetof( OpenContainModuleData, m_numberOfExitPaths ) },
		{ "DoorOpenTime",							INI::parseDurationUnsignedInt, NULL, offsetof( OpenContainModuleData, m_doorOpenTime ) },
 		{ "AllowAlliesInside",				INI::parseBool,	NULL, offsetof( OpenContainModuleData, m_allowAlliesInside ) },
 		{ "AllowEnemiesInside",				INI::parseBool,	NULL, offsetof( OpenContainModuleData, m_allowEnemiesInside ) },
 		{ "AllowNeutralInside",				INI::parseBool,	NULL, offsetof( OpenContainModuleData, m_allowNeutralInside ) },
		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
	p.add(DieMuxData::getFieldParse(), offsetof( OpenContainModuleData, m_dieMuxData ));

}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
OpenContain::OpenContain( Thing *thing, const ModuleData* moduleData ) : UpdateModule( thing, moduleData )
{

	// initialize our lists
	m_containList.clear();
	m_objectEnterExitInfo.clear();
	m_playerEnteredMask = 0;
	m_lastUnloadSoundFrame = 0;
	m_lastLoadSoundFrame = 0;
	m_containListSize = 0;
	m_stealthUnitsContained = 0;
	m_doorCloseCountdown = 0;

	//Added By Sadullah Nader
	//Initializations inserted
	m_rallyPoint.zero();
	m_rallyPointExists = FALSE;
	//
	m_conditionState.clear();
	m_firePointStart = -1;
	m_firePointNext = 0;
	m_firePointSize = 0;
	m_noFirePointsInArt = false;
	m_whichExitPath = 1;
	m_loadSoundsEnabled = TRUE;

	for( Int i = 0; i < MAX_FIRE_POINTS; i++ )
	{		
		m_firePoints[ i ].Make_Identity();
	}  // end for i

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
Int OpenContain::getContainMax( void ) const
{
	const OpenContainModuleData *modData = getOpenContainModuleData();

	return modData->m_containMax;

}  // end getContainMax

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
OpenContain::~OpenContain()
{

	// sanity, the system should be cleaning these up itself if all is going well
	DEBUG_ASSERTCRASH( m_containList.empty(), 
										 ("OpenContain %s: destroying a container that still has items in it!\n", 
										  getObject()->getTemplate()->getName().str() ) );

	// sanity
	DEBUG_ASSERTCRASH( m_xferContainIDList.empty(),
										 ("OpenContain %s: m_xferContainIDList is not empty but should be\n", 
											getObject()->getTemplate()->getName().str() ) );

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
// our object changed position... react as appropriate.
void OpenContain::containReactToTransformChange()
{
	// Our transform changed, which means our bones moved, and we keep people positioned on bones.
	redeployOccupants();
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UpdateSleepTime OpenContain::update( void )
{
	m_playerEnteredMask = 0;

	// we need to monitor changes in the art and position
	monitorConditionChanges();

	if( m_doorCloseCountdown )
	{
		/// @todo srj -- for now, OpenContain assumes at most one door
		--m_doorCloseCountdown;
		if( m_doorCloseCountdown == 0 )
			getObject()->clearAndSetModelConditionState( MODELCONDITION_DOOR_1_OPENING, MODELCONDITION_DOOR_1_CLOSING );
	}

	if (!m_objectEnterExitInfo.empty())
		pruneDeadWanters();

	return UPDATE_SLEEP_NONE;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void OpenContain::addOrRemoveObjFromWorld(Object* obj, Bool add) 
{ 

	//
	// it's expensive to add and remove structures from the pathfinder, but we really shouldn't
	// be doing it anyway as it doesn't make much sense to contain a structure, but we'll 
	// check for it here and print a warning
	//
	if( obj->isKindOf( KINDOF_STRUCTURE ) )
		DEBUG_LOG(( "WARNING: Containing/Removing structures like '%s' is potentially a very expensive and slow operation\n",
								obj->getTemplate()->getName().str() ));


	if (add)
	{
		ThePartitionManager->registerObject( obj );

		if( obj->getDrawable() )
		{
			obj->getDrawable()->setDrawableHidden( false );
		}

		// add object to pathfind map
		TheAI->pathfinder()->addObjectToPathfindMap( obj );

	}
	else
	{
		// remove object from its group (if any)
		obj->leaveGroup();

		// remove rider from partition manager
		ThePartitionManager->unRegisterObject( obj );

		// hide the drawable associated with rider
		if( obj->getDrawable() )
			obj->getDrawable()->setDrawableHidden( true );

		// remove object from pathfind map
		TheAI->pathfinder()->removeObjectFromPathfindMap( obj );

	}

	// if we're a non-enclosing container (eg, parachute), and we're inside an 
	// enclosing container (eg, b52), we need to be able to show/hide the contained riders...
	if( obj->getContain() )
	{
		const ContainedItemsList* items = obj->getContain()->getContainedItemsList();
		if (items)
		{
			for(ContainedItemsList::const_iterator it = items->begin(); it != items->end(); ++it)
			{
				if( !obj->getContain()->isEnclosingContainerFor(*it) )
					addOrRemoveObjFromWorld(*it, add);
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------
/** Add 'rider' to the m_containList of objects in this module.
	* This will trigger an onContaining event for the object that this module
	* is a part of and an onContainedBy event for the object being contained */
//-------------------------------------------------------------------------------------------------
void OpenContain::addToContain( Object *rider )
{

	// sanity
	if( rider == NULL )
		return;

#if defined(RTS_DEBUG) || defined(RTS_INTERNAL)
	if( !isValidContainerFor( rider, false ) )
	{
		Object *reportObject = rider;
		if( rider->getContain() )
		{
			//Report the first thing inside it!
			const ContainedItemsList *items = rider->getContain()->getContainedItemsList();
			if( items )
			{
				reportObject = *items->begin();
			}
		}
		DEBUG_CRASH( ("OpenContain::addToContain() - Object %s not valid for container %s!", reportObject?reportObject->getTemplate()->getName().str():"NULL", getObject()->getTemplate()->getName().str() ) );
	}
#endif

	// this object cannot be contained by this module if it is already contained in something
	if( rider->getContainedBy() != NULL )
	{

		DEBUG_LOG(( "'%s' is trying to contain '%s', but '%s' is already contained by '%s'\n",
								getObject()->getTemplate()->getName().str(),
								rider->getTemplate()->getName().str(),
								rider->getTemplate()->getName().str(),
								rider->getContainedBy()->getTemplate()->getName().str() ));
		return;

	}

	// Which list to physically add to needs to be overridable
	addToContainList(rider);

	m_playerEnteredMask = rider->getControllingPlayer()->getPlayerMask();

	if (isEnclosingContainerFor( rider ))
	{
		addOrRemoveObjFromWorld(rider, false);
	}

	// ensure our contents are positions correctly.
	redeployOccupants();

	// trigger an onContaining event for the object that just "ate" something
	if( getObject()->getContain() )
	{
		getObject()->getContain()->onContaining( rider );
	}

	// trigger an onContainedBy event for the object that just got "eaten" by us
	rider->onContainedBy( getObject() );

	doLoadSound();

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void OpenContain::addToContainList( Object *rider )
{
	m_containList.push_back(rider);
	m_containListSize++;
	if( rider->isKindOf( KINDOF_STEALTH_GARRISON ) )
	{
		m_stealthUnitsContained++;
	}
}

//-------------------------------------------------------------------------------------------------
/** Remove 'rider' from the m_containList of objects in this module.
	* This will trigger an onRemoving event for the object that this module
	* is a part of and an onRemovedFrom event for the object being removed */
//-------------------------------------------------------------------------------------------------
void OpenContain::removeFromContain( Object *rider, Bool exposeStealthUnits )
{

	// sanity
	if( rider == NULL )
		return;

	//
	// we can only remove this object from the contains list of this module if
	// it is actually contained by this module
	//
	Object *containedBy = rider->getContainedBy();
	if( containedBy != getObject() )
	{

		DEBUG_LOG(( "'%s' is trying to un-contain '%s', but '%s' is really contained by '%s'\n",
								getObject()->getTemplate()->getName().str(),
								rider->getTemplate()->getName().str(),
								rider->getTemplate()->getName().str(),
								containedBy ? containedBy->getTemplate()->getName().str() : "Nothing" ));
		return;

	}

	ContainedItemsList::iterator it = std::find(m_containList.begin(), m_containList.end(), rider);
	if (it != m_containList.end())
	{
		// note that this invalidates the iterator!
		removeFromContainViaIterator( it, exposeStealthUnits );
	}	

}

//-------------------------------------------------------------------------------------------------
/** Remove all contained objects from the contained list */
//-------------------------------------------------------------------------------------------------
void OpenContain::removeAllContained( Bool exposeStealthUnits )
{
	ContainedItemsList::iterator it;

 	while ((it = m_containList.begin()) != m_containList.end())
	{

 		// note that this invalidates the iterator!
 		removeFromContainViaIterator( it, exposeStealthUnits );

	}  // end while

}  // end removeAllContained

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void OpenContain::doLoadSound()
{
	//
	// play a sound for loading someone into a building
	//
	if( m_loadSoundsEnabled )
	{
		UnsignedInt now = TheGameLogic->getFrame();
		if( now != m_lastLoadSoundFrame )
		{
			if (getOpenContainModuleData())
			{
				AudioEventRTS enterSound(getOpenContainModuleData()->m_enterSound);
				enterSound.setObjectID(getObject()->getID());
				TheAudio->addAudioEvent(&enterSound);
			}
			// save this frame as the last time we did this sound
			m_lastLoadSoundFrame = now;
		}
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void OpenContain::doUnloadSound()
{
	//
	// play a sound for unloading someone from a building ... but don't play more
	// than one per frame (we can do meta unload all commands)
	//
	UnsignedInt now = TheGameLogic->getFrame();
	if( now != m_lastUnloadSoundFrame )
	{
		if (getOpenContainModuleData())
		{
			AudioEventRTS exitSound(getOpenContainModuleData()->m_exitSound);
			exitSound.setObjectID(getObject()->getID());

			TheAudio->addAudioEvent(&exitSound);
		}
		// save this frame as the last time we did this sound
		m_lastUnloadSoundFrame = now;
	}
}

//-------------------------------------------------------------------------------------------------
/** Iterate the contained list and call the callback on each of the objects */
//-------------------------------------------------------------------------------------------------
void OpenContain::iterateContained( ContainIterateFunc func, void *userData, Bool reverse )
{
	if (reverse)
	{
		// note that this has to be smart enough to handle items in the list being deleted
		// via the callback function.
		for(ContainedItemsList::reverse_iterator it = m_containList.rbegin(); it != m_containList.rend(); )
		{
			// save the rider...
			Object* rider = *it;
			
			// incr the iterator BEFORE calling the func (if the func removes the rider,
			// the iterator becomes invalid)
			++it;
			
			// call it
			(*func)( rider, userData );
		}
	}
	else
	{
		// note that this has to be smart enough to handle items in the list being deleted
		// via the callback function.
		for(ContainedItemsList::iterator it = m_containList.begin(); it != m_containList.end(); )
		{
			// save the rider...
			Object* rider = *it;
			
			// incr the iterator BEFORE calling the func (if the func removes the rider,
			// the iterator becomes invalid)
			++it;
			
			// call it
			(*func)( rider, userData );
		}
	}
}

//-------------------------------------------------------------------------------------------------
struct DropData
{
	Real minRadius;
	Real maxRadius;
	Object *container;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
/** Remove an object from the containment of this module given the item
	* to remove and trigger the proper callback events */
//-------------------------------------------------------------------------------------------------
void OpenContain::removeFromContainViaIterator( ContainedItemsList::iterator it, Bool exposeStealthUnits )
{

/*
	#ifdef RTS_DEBUG
		TheInGameUI->message( UnicodeString( L"'%S(%d)' no longer contains '%S(%d)'" ), 
													getObject()->getTemplate()->getName().str(),
													getObject()->getID(),
													itemToRemove->m_object->getTemplate()->getName().str(),
													itemToRemove->m_object->getID() );
	#endif
*/
	Object *rider = *it;

	// remove item from list
	m_containList.erase(it);
	m_containListSize--;
	if( rider->isKindOf( KINDOF_STEALTH_GARRISON ) )
	{
		m_stealthUnitsContained--;
		if( exposeStealthUnits )
		{
			StealthUpdate* stealth = rider->getStealth();
			if( stealth )
			{
				stealth->markAsDetected();
			}
		}
	}


	if (isEnclosingContainerFor( rider ))
	{
		addOrRemoveObjFromWorld(rider, true);
	}

	/// place the object in the world at position of the container m_object
	rider->setPosition( getObject()->getPosition() );
	rider->setLayer( getObject()->getLayer() );

	doUnloadSound();

	// trigger an onRemoving event for 'm_object' no longer containing 'itemToRemove->m_object'
	DEBUG_ASSERTCRASH(getObject()->getContain() == this, ("hmm, wrong container"));
	if( getObject()->getContain() )
	{
		getObject()->getContain()->onRemoving( rider );
	}
		
	// trigger an onRemovedFrom event for 'remove'
	DEBUG_ASSERTCRASH(getObject()->getContain() == this, ("hmm, wrong container 2"));
	rider->onRemovedFrom( getObject() );

}

//-------------------------------------------------------------------------------------------------
void OpenContain::scatterToNearbyPosition(Object* rider)
{
	Object *theContainer = getObject();

	//
	// for now we will just set the position of the object that is being removed from us
	// at a random angle away from our center out some distance
	//
	
	//
	// pick an angle that is in the view of the current camera position so that
	// the thing will come out "toward" the player and they can see it
	// NOPE, can't do that ... all players screen angles will be different, unless
	// we maintain the angle of each players screen in the player structure or something
	//
	Real angle = GameLogicRandomValueReal( 0.0f, 2.0f * PI );
//	angle = TheTacticalView->getAngle();
//	angle -= GameLogicRandomValueReal( PI / 3.0f, 2.0f * (PI / 3.0F) );

	Real minRadius = theContainer->getGeometryInfo().getBoundingCircleRadius();
	Real maxRadius = minRadius + minRadius / 2.0f;
	const Coord3D *containerPos = theContainer->getPosition();
	Real dist = GameLogicRandomValueReal( minRadius, maxRadius );

	Coord3D pos;
	pos.x = dist * Cos( angle ) + containerPos->x;
	pos.y = dist * Sin( angle ) + containerPos->y;
	pos.z = TheTerrainLogic->getLayerHeight( pos.x, pos.y, theContainer->getLayer() );

	// set orientation
	rider->setOrientation( angle );

	AIUpdateInterface *ai = rider->getAI();
	if( ai )
	{
		// set position of the object at center of building and move them toward pos
		rider->setPosition( theContainer->getPosition() );
		ai->ignoreObstacle(theContainer);
		ai->aiMoveToPosition( &pos, CMD_FROM_AI );

	}  // end if
	else
	{

		// no ai, just set position at the target pos
		rider->setPosition( &pos );

	}  // end else
}

//-------------------------------------------------------------------------------------------------
void OpenContain::onContaining( Object * /*rider*/ )
{
	// Play audio
	if( m_loadSoundsEnabled )
	{
		AudioEventRTS enterSound = *getObject()->getTemplate()->getSoundEnter();
		enterSound.setObjectID(getObject()->getID());
		TheAudio->addAudioEvent(&enterSound);
	}
}

//-------------------------------------------------------------------------------------------------
void OpenContain::onRemoving( Object *rider) 
{
	// Play audio
	AudioEventRTS exitSound = *getObject()->getTemplate()->getSoundExit();
	exitSound.setObjectID(getObject()->getID());
	TheAudio->addAudioEvent(&exitSound);

	if (rider) {
		// This is a misnomer, but it makes it clearer for the user.
		AudioEventRTS fallingSound = *rider->getTemplate()->getSoundFalling();
		fallingSound.setObjectID(rider->getID());
		TheAudio->addAudioEvent(&fallingSound);
	}
}

//-------------------------------------------------------------------------------------------------
Real OpenContain::getContainedItemsMass() const
{
	/// @todo srj -- may want to cache this information.
	Real mass = 0;
	for(ContainedItemsList::const_iterator it = m_containList.begin(); it != m_containList.end(); ++it)
	{
		PhysicsBehavior* phys = (*it)->getPhysics();
		if (phys)
			mass += phys->getMass();
	}
	return mass;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void OpenContain::onCollide( Object *other, const Coord3D *loc, const Coord3D *normal )
{
	// colliding with nothing? we don't care.
	if( other == NULL )
		return;

	// ok, step two: only contain stuff that wants us to contain it.
	// (it would suck to have accidental collisions contain things...)

	// must be an AI object....
	AIUpdateInterface *ai = other->getAI();
	if (ai == NULL)
		return;

	// ...and must be trying to enter our object.
	// (huh huh, he said "enter")
	if (ai->getEnterTarget() != getObject())
		return;

	// last-minute change: don't allow units from multiple (different) players to occupy the same
	// unit. so eject everyone else if they aren't controlled by the same player. (srj)
	for( ContainedItemsList::iterator it = m_containList.begin(); it != m_containList.end(); )
	{
		// save the rider...
		Object* rider = *it;
		
		// incr the iterator BEFORE calling the func (if the func removes the rider,
		// the iterator becomes invalid)
		++it;
		
		// call it
		if( rider->getControllingPlayer() != other->getControllingPlayer() )
		{
			if( rider->getAI() )
			{
				if( rider->isKindOf( KINDOF_STEALTH_GARRISON ) )
				{
					// aiExit is needed to walk away from the building well, but it doesn't take the Unstealth flag
					StealthUpdate* stealth = rider->getStealth();
					if( stealth )
					{
						stealth->markAsDetected();
					}
				}
				rider->getAI()->aiExit( getObject(), CMD_FROM_AI );
			}
			else
				removeFromContain( rider, TRUE );
		}
	}


	// finally, we must have space to contain it.
	if( !isValidContainerFor( other, TRUE ) )
		return;

	addToContain(other);
} 

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void OpenContain::onDelete( void )	///< Last possible moment cleanup
{
	// This uses my literal list, and not the gettor, because we don't want to get redirected some place fancy.
	for(ContainedItemsList::iterator it = m_containList.begin(); it != m_containList.end(); )
	{
		Object* rider = *it;
		++it;
		TheGameLogic->destroyObject( rider );
	}
}

//-------------------------------------------------------------------------------------------------
/** The die callback. */
//-------------------------------------------------------------------------------------------------
void OpenContain::onDie( const DamageInfo * damageInfo )
{
	if (!getOpenContainModuleData()->m_dieMuxData.isDieApplicable(getObject(), damageInfo))
		return;

	//Check to see if we are going to inflict damage on contained units.
	if( getOpenContainModuleData()->m_damagePercentageToUnits > 0 )
	{
		//Cycle through the units and apply damage to them!
		processDamageToContained();
	}

	killRidersWhoAreNotFreeToExit();

	// Leaving this commented out to show it can't work.  We are about to die, so they will have zero 
	// chance to hit an exitState::Update.  At least we would clean them up in onDelete.
//	orderAllPassengersToExit( CMD_FROM_AI );
	removeAllContained();
}  

// ------------------------------------------------------------------------------------------------
/** Check to see if we are a valid container for 'obj' */
// ------------------------------------------------------------------------------------------------
Bool OpenContain::isValidContainerFor(const Object* obj, Bool checkCapacity) const
{
	const Object *us = getObject();
	const OpenContainModuleData *modData = getOpenContainModuleData();

	// if we have any kind of masks set then we must make that check
	if (obj->isAnyKindOf( modData->m_allowInsideKindOf ) == FALSE ||
			obj->isAnyKindOf( modData->m_forbidInsideKindOf ) == TRUE)
	{
		return false;
	}

 	//
 	// check relationship, note that this behavior is defined as the relation between
 	// 'obj' and the container 'us', and not the reverse
 	//
 	Bool relationshipRestricted = FALSE;
 	Relationship r = obj->getRelationship( us );
 	switch( r )
 	{
 		case ALLIES:
 			if( modData->m_allowAlliesInside == FALSE )
 				relationshipRestricted = TRUE;
 			break;
 
 		case ENEMIES:
 			if( modData->m_allowEnemiesInside == FALSE )
 				relationshipRestricted = TRUE;
 			break;
 
 		case NEUTRAL:
 			if( modData->m_allowNeutralInside == FALSE )
 				relationshipRestricted = TRUE;
 			break;
 
 		default:
 			DEBUG_CRASH(( "isValidContainerFor: Undefined relationship (%d) between '%s' and '%s'",
 										r, getObject()->getTemplate()->getName().str(),
 										obj->getTemplate()->getName().str() ));
 			return FALSE;
 
 	}  // end switch
 	if( relationshipRestricted == TRUE )
 		return FALSE;
 
	// all is well
	return true;

}  // end isValidContainerFor

// ------------------------------------------------------------------------------------------------
/**
	This new call is going to replace all game callings of removeFromContain.
	Consider rFC a 'system' call, while this is a 'game' call.  rFC will pull
	from the contained list and just plop the guy in the world, this function
	will then override that location with cool game stuff.

	Exit and Evacuate can fail, removeAllContain and removeFromContain cannot.
*/
void OpenContain::exitObjectViaDoor( Object *exitObj, ExitDoorType exitDoor )
{
	DEBUG_ASSERTCRASH(exitDoor == DOOR_1, ("multiple exit doors not supported here"));

	removeFromContain( exitObj );

	Object *me = getObject();

	m_doorCloseCountdown = getOpenContainModuleData()->m_doorOpenTime;
	if (m_doorCloseCountdown)
	{
		// srj sez: only diddle the doors if this countdown is nonzero. 
		// this allows us to prevent this module from messing with the doors
		// at all, which is required for DeliverPayloadAIUpdate.
		/// @todo srj -- for now, OpenContain assumes at most one door
		me->clearAndSetModelConditionState( MODELCONDITION_DOOR_1_CLOSING, MODELCONDITION_DOOR_1_OPENING );
	}

	Int numberExits = getOpenContainModuleData()->m_numberOfExitPaths;
	if( numberExits > 0 )
	{
		// We have ExitStart/End specified in art to use when we kick people out.  If >1, then
		// we have many and we need to decide which one to use.
		AsciiString startBone("ExitStart");
		AsciiString endBone("ExitEnd");
		Coord3D startPosition;
		Coord3D endPosition;
		if( numberExits > 1 )
		{
			char suffix[8];
			itoa(m_whichExitPath, suffix, 10);
			if( m_whichExitPath < 10 )
			{
				startBone.concat('0');
				endBone.concat('0');
			}
			
			m_whichExitPath = (m_whichExitPath % numberExits) + 1;// To cycle from 1 to n

			startBone.concat(suffix);
			endBone.concat(suffix);
		}
		me->getSingleLogicalBonePosition( startBone.str(), &startPosition, NULL );
		me->getSingleLogicalBonePosition( endBone.str(), &endPosition, NULL );

		//startPosition.x = startPosition.y = 0;
		Real exitAngle = me->getOrientation();
		PhysicsBehavior *physics = exitObj->getPhysics();
		// Can fall problem - When units exit from planes, they are airborne for a while as they drop.
		// Airborne units dont' do pathfinding, and this is fine.
		// However, amphibious transports unload 3 feet off the ground.  This is enough to trigger the 
		// airborne flag, and they don't pathfind.  So they all unload at the same exact point, and life
		// is ugly.  So temporarily, we turn off the allowToFall flag, so they aren't considered airborne, 
		// and their final destination (adjustToPossibleDestination) considers them as pathfinding, so they
		// don't stack up.  Then we turn the flag back, so if they are parachuting, they can fly down
		//  on their parachute.  jba.
		Bool canFall = false;
		if (physics) 
		{
			canFall = physics->getAllowToFall();
		}
		exitObj->setPosition( &startPosition );
		exitObj->setOrientation( exitAngle );
		
		// Per JohnA: We need to set our layer to match our transports layer, or we'll try to pick a spot
		// on the ground.
		exitObj->setLayer( me->getLayer() );
		///< @todo This really should be automatically wrapped up in an activation sequence	for objects in general
		// tell the AI about it
		AIUpdateInterface *ai = exitObj->getAI();
		AIUpdateInterface *myAi = me->getAI();
		TheAI->pathfinder()->addObjectToPathfindMap( exitObj );
		if (ai) 
		{
			if (myAi && myAi->isIdle() && me->isKindOf(KINDOF_VEHICLE)) {
				TheAI->pathfinder()->removeUnitFromPathfindMap(me);
				TheAI->pathfinder()->updatePos(me, me->getPosition());
				TheAI->pathfinder()->updateGoal(me, me->getPosition(), TheTerrainLogic->getLayerForDestination(me->getPosition()));
			}
			ai->ignoreObstacle(NULL);
			// The units often come out at the same position, and need to ignore collisions briefly
			// as they move out.  jba.
			ai->setIgnoreCollisionTime(LOGICFRAMES_PER_SECOND);
			TheAI->pathfinder()->adjustToPossibleDestination(exitObj, ai->getLocomotorSet(), &endPosition);
		}
		std::vector<Coord3D> exitPath;
		exitPath.push_back(endPosition);
		exitPath.push_back(endPosition); // Do it twice, in case units stack up due to brief flying.  jba.
		if (m_rallyPointExists) {
			exitPath.push_back(m_rallyPoint);
		}

		if( ai )
		{
			if (physics) 
			{
				physics->setAllowToFall(false);
			}

			ai->aiFollowPath( &exitPath, me, CMD_FROM_AI );

			TheAI->pathfinder()->updateGoal(exitObj, &endPosition, TheTerrainLogic->getLayerForDestination(&endPosition));
		}
		if (physics) 
		{
			physics->setAllowToFall(canFall);
		}
	}
	else
	{
		///< @todo This really should be automatically wrapped up in an activation sequence	for objects in general
		// tell the AI about it
		TheAI->pathfinder()->addObjectToPathfindMap( exitObj );
	}
}
// ------------------------------------------------------------------------------------------------
/**
	This call is used to evacuate units from a tunnel network via a persistent state (guard tunnel networ).
	The main difference between this and exitObjectViaDoor is that this exit doesn't change the current
	ai state, so the guard state doesn't get blown away. jba.
*/
void OpenContain::exitObjectInAHurry( Object *exitObj )
{
	removeFromContain( exitObj );

	Object *me = getObject();

	m_doorCloseCountdown = getOpenContainModuleData()->m_doorOpenTime;
	if (m_doorCloseCountdown)
	{
		// srj sez: only diddle the doors if this countdown is nonzero. 
		// this allows us to prevent this module from messing with the doors
		// at all, which is required for DeliverPayloadAIUpdate.
		/// @todo srj -- for now, OpenContain assumes at most one door
		me->clearAndSetModelConditionState( MODELCONDITION_DOOR_1_CLOSING, MODELCONDITION_DOOR_1_OPENING );
	}

	Int numberExits = getOpenContainModuleData()->m_numberOfExitPaths;
	if( numberExits > 0 )
	{
		// We have ExitStart/End specified in art to use when we kick people out.  If >1, then
		// we have many and we need to decide which one to use.
		AsciiString startBone("ExitStart");
		AsciiString endBone("ExitEnd");
		Coord3D startPosition;
		Coord3D endPosition;
		if( numberExits > 1 )
		{
			char suffix[8];
			itoa(m_whichExitPath, suffix, 10);
			if( m_whichExitPath < 10 )
			{
				startBone.concat('0');
				endBone.concat('0');
			}
			
			m_whichExitPath = (m_whichExitPath % numberExits) + 1;// To cycle from 1 to n

			startBone.concat(suffix);
			endBone.concat(suffix);
		}
		me->getSingleLogicalBonePosition( startBone.str(), &startPosition, NULL );
		me->getSingleLogicalBonePosition( endBone.str(), &endPosition, NULL );

		//startPosition.x = startPosition.y = 0;
		Real exitAngle = me->getOrientation();
		exitObj->setPosition( &startPosition );
		exitObj->setOrientation( exitAngle );
		
		// Per JohnA: We need to set our layer to match our transports layer, or we'll try to pick a spot
		// on the ground.
		exitObj->setLayer( me->getLayer() );
		std::vector<Coord3D> exitPath;
		exitPath.push_back(endPosition);
		AIUpdateInterface *ai = exitObj->getAI();
		AIUpdateInterface *myAi = me->getAI();
		TheAI->pathfinder()->addObjectToPathfindMap( exitObj );
		if (ai) 
		{
			if (myAi && myAi->isIdle() && me->isKindOf(KINDOF_VEHICLE)) {
				TheAI->pathfinder()->removeUnitFromPathfindMap(me);
				TheAI->pathfinder()->updatePos(me, me->getPosition());
				TheAI->pathfinder()->updateGoal(me, me->getPosition(), TheTerrainLogic->getLayerForDestination(me->getPosition()));
			}
			ai->ignoreObstacle(NULL);
			// The units often come out at the same position, and need to ignore collisions briefly
			// as they move out.  jba.
			ai->setIgnoreCollisionTime(LOGICFRAMES_PER_SECOND);
			TheAI->pathfinder()->adjustToPossibleDestination(exitObj, ai->getLocomotorSet(), &endPosition);
		}
		exitPath.push_back(endPosition);
		if (m_rallyPointExists) {
			exitPath.push_back(m_rallyPoint);
		}

		if( ai )
		{
			ai->doQuickExit( &exitPath );

			TheAI->pathfinder()->updateGoal(exitObj, &endPosition, TheTerrainLogic->getLayerForDestination(&endPosition));
		}
	}
	else
	{
		///< @todo This really should be automatically wrapped up in an activation sequence	for objects in general
		// tell the AI about it
		TheAI->pathfinder()->addObjectToPathfindMap( exitObj );
	}
}


//-------------------------------------------------------------------------------------------------
Bool OpenContain::isPassengerAllowedToFire() const
{
	const OpenContainModuleData *modData = getOpenContainModuleData();
	if( ! modData->m_passengersAllowedToFire )
		return FALSE;// Just no, no matter what.

	// If we are ourselves contained, our passengers need to check with them if they get past us
	if( getObject()->getContainedBy() != NULL )
		return getObject()->getContainedBy()->getContain()->isPassengerAllowedToFire();

	return TRUE;// We say yes, and we are not inside something.
}

//-------------------------------------------------------------------------------------------------
/** Check to see if our internal knowledge of the artwork matches the current state of
	* the model being displayed.  When changes occur (from changing the time of day or going
	* to a new damage state, we will want to redeploy all our occupants to be at new fire
	* points that are reflected in the new artwork */
//-------------------------------------------------------------------------------------------------
void OpenContain::monitorConditionChanges( void )
{
	Drawable *draw = getObject()->getDrawable();
	Bool stateChanged = false;
	ModelConditionFlags currCondition;

	if( draw )
	{
		currCondition = draw->getModelConditionFlags();
		if(currCondition != m_conditionState )
			stateChanged = TRUE;
	}

	// on a state change we should redeploy our occupants
	if( stateChanged )
	{
		// just yank all objects from the fire points and redeploy
		redeployOccupants();
		// record this as our current and up to date state
		m_conditionState = currCondition;
	}

}  // end monitorConditionChanges

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void OpenContain::redeployOccupants( void )
{

	//
	// because the state has changed, we will must give the deploy logic the opportunity
	// to look for a new set of bones in the art ... so if we had them flagged as not
	// having any before (for optimization reasons) we want to check at least one more
	// time again
	//
	m_noFirePointsInArt = false;
	m_firePointStart = -1;
	m_firePointNext = 0;
	m_firePointSize = 0;				// 0 here will cause a re-lookup in the new art

	//
	// redeploy the occupants, note that we're starting at the last occupant and going
	// backwards to the start ... that reflects the order in which they actually went
	// into the building since all list building constructs at the head
	//
	for (ContainedItemsList::const_reverse_iterator it = getContainList().rbegin(); it != getContainList().rend(); ++it)
	{
		putObjAtNextFirePoint( *it );
	}

}  // end redeployOccupants

//-------------------------------------------------------------------------------------------------
/** Place the object at the 3D position of the next fire point to use */
//-------------------------------------------------------------------------------------------------
void OpenContain::putObjAtNextFirePoint( Object *obj )
{

	//
	// first, if we need to load the 3D point data from the art do so, if we've already 
	// determined there is no firepoints in the current art we will avoid searching for
	// it all the time
	//
	if( m_firePointSize == 0 && m_noFirePointsInArt == false )
	{

		m_firePointSize = getObject()->getMultiLogicalBonePosition("FIREPOINT", MAX_FIRE_POINTS, NULL, m_firePoints, TRUE );

		//
		// if there is still no firepoints in the art, we'll set a flag so that we don't
		// ever go through the art stuff again
		//
		if( m_firePointSize == 0 )
			m_noFirePointsInArt = TRUE;

	}  // end if

	//
	// if there are no fire points in the art we just put the object at the center
	// of the object
	//
	if( m_noFirePointsInArt == TRUE )
	{
		const Coord3D *pos = getObject()->getPosition();

		obj->setPosition( pos );
		return;

	}  // end if

	// get the position
	Matrix3D matrix;
	if( getOpenContainModuleData()->m_passengersInTurret )
	{
		// If our passengers are in our turret, we need to recompute the Matrix.
		AsciiString firepoint("FIREPOINT");
		char suffix[8];
		itoa( m_firePointNext + 1, suffix, 10 );//+1 from bone names starting at 1, not zero like my array
		if( m_firePointNext < 10 )
		{
			firepoint.concat('0');
		}
		firepoint.concat(suffix);

		getObject()->getSingleLogicalBonePositionOnTurret(TURRET_MAIN, firepoint.str(), NULL, &matrix );
	}
	else
	{
		matrix = m_firePoints[ m_firePointNext ];
	}

	Vector3 vectorPos = matrix.Get_Translation();
	Coord3D pos;
	pos.set( vectorPos.X, vectorPos.Y, vectorPos.Z );

	// set the object position
	if( isEnclosingContainerFor( obj ) )
		obj->setPosition( &pos );
	else
		obj->setTransformMatrix( &matrix );//Only do everything if it matters

	// increment the next firepoint to use ... make sure to wrap if we need to
	m_firePointNext++;
	if( m_firePointNext >= m_firePointSize )
		m_firePointNext = 0;

}  // end putObjAtNextFirePoint

//-------------------------------------------------------------------------------------------------
/**
	this is used for containers that must do something to allow people to enter or exit...
	eg, land (for Chinook), open door (whatever)... it's called with wants=WANTS_TO_ENTER
	when something is in the enter state, and wants=WANTS_NOTHING when the unit has
	either entered, or given up...
*/
void OpenContain::onObjectWantsToEnterOrExit(Object* obj, ObjectEnterExitType wants)
{
	if (obj == NULL)
		return;

	ObjectID id = obj->getID();
	if (wants == WANTS_NEITHER)
	{
		ObjectEnterExitMap::iterator it = m_objectEnterExitInfo.find(id);
		if (it != m_objectEnterExitInfo.end())
			m_objectEnterExitInfo.erase(it);
	}
	else
	{
		m_objectEnterExitInfo[id] = wants;
	}
}

//-------------------------------------------------------------------------------------------------
void OpenContain::pruneDeadWanters()
{
	for (ObjectEnterExitMap::iterator it = m_objectEnterExitInfo.begin(); it != m_objectEnterExitInfo.end(); /*++it*/)
	{
		ObjectID id = (*it).first;
		Object* obj = TheGameLogic->findObjectByID(id);
		if (obj == NULL || obj->isEffectivelyDead())
		{
			ObjectEnterExitMap::iterator tmp = it;
			++it;
			m_objectEnterExitInfo.erase(tmp);
		}
		else
		{
			++it;
		}
	}
}

//-------------------------------------------------------------------------------------------------
void OpenContain::markAllPassengersDetected( )
{
	for( ContainedItemsList::iterator it = m_containList.begin(); it != m_containList.end(); )
	{
		// save the rider...
		Object* rider = *it;
		
		// incr the iterator BEFORE calling the func (if the func removes the rider,
		// the iterator becomes invalid)
		++it;
		
		// call it
		if( rider->isKindOf( KINDOF_STEALTH_GARRISON ) )
		{
			StealthUpdate* stealth = rider->getStealth();
			if( stealth )
			{
				stealth->markAsDetected();
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------
void OpenContain::onSelling()
{
	// An OpenContain tells everyone to leave.
	orderAllPassengersToExit(CMD_FROM_AI);
}

//-------------------------------------------------------------------------------------------------
void OpenContain::orderAllPassengersToExit( CommandSourceType commandSource )
{
	for( ContainedItemsList::const_iterator it = getContainedItemsList()->begin(); it != getContainedItemsList()->end(); )
	{
		// save the rider...
		Object* rider = *it;
		
		// incr the iterator BEFORE calling the func (if the func removes the rider,
		// the iterator becomes invalid)
		++it;
		
		// call it
		if( rider->getAI() )
			rider->getAI()->aiExit( getObject(), commandSource );
	}
}

//-------------------------------------------------------------------------------------------------
void OpenContain::processDamageToContained()
{
#if RETAIL_COMPATIBLE_CRC

	const ContainedItemsList* items = getContainedItemsList();
	if( items )
	{
		ContainedItemsList::const_iterator it = items->begin();
		const size_t listSize = items->size();

		while( it != items->end() )
		{
			Object *object = *it++;

			//Calculate the damage to be inflicted on each unit.
			Real damage = object->getBodyModule()->getMaxHealth() * getOpenContainModuleData()->m_damagePercentageToUnits;

			DamageInfo damageInfo;
			damageInfo.in.m_damageType = DAMAGE_UNRESISTABLE;
			damageInfo.in.m_deathType = DEATH_BURNED;
			damageInfo.in.m_sourceID = getObject()->getID();
			damageInfo.in.m_amount = damage;
			object->attemptDamage( &damageInfo );

			if( !object->isEffectivelyDead() && getOpenContainModuleData()->m_damagePercentageToUnits == 1.0f )
				object->kill(); // in case we are carrying flame proof troops we have been asked to kill

			// TheSuperHackers @info Calls to Object::attemptDamage and Object::kill will not remove
			// the occupant from the host container straight away. Instead it will be removed when the
			// Object deletion is finalized in a Game Logic update. This will lead to strange behavior
			// where the occupant will be removed after death with a delay. This behavior cannot be
			// changed without breaking retail compatibility.

			// TheSuperHackers @bugfix xezon 05/06/2025 Stop iterating when the list was cleared.
			// This scenario can happen if the killed occupant(s) apply deadly damage on death
			// to the host container, which then attempts to remove all remaining occupants
			// on the death of the host container. This is reproducible by destroying a
			// GLA Battle Bus with at least 2 half damaged GLA Terrorists inside.
			if (listSize != items->size())
			{
				DEBUG_ASSERTCRASH( listSize == 0, ("List is expected empty\n") );
				break;
			}
		}
	}

#else

	// TheSuperHackers @bugfix xezon 05/06/2025 Temporarily empty the m_containList
	// to prevent a potential child call to catastrophically modify the m_containList.
	// This scenario can happen if the killed occupant(s) apply deadly damage on death
	// to the host container, which then attempts to remove all remaining occupants
	// on the death of the host container. This is reproducible by destroying a
	// GLA Battle Bus with at least 2 half damaged GLA Terrorists inside.

	// Caveat: While the m_containList is empty, it will not be possible to apply damage
	// on death of a unit to another unit in the host container. If this functionality
	// is desired, then this implementation needs to be revisited.

	ContainedItemsList list;
	m_containList.swap(list);
	m_containListSize = 0;

	ContainedItemsList::iterator it = list.begin();

	while ( it != list.end() )
	{
		Object *object = *it;

		DEBUG_ASSERTCRASH( object, ("Contain list must not contain NULL element\n") );

		// Calculate the damage to be inflicted on each unit.
		Real damage = object->getBodyModule()->getMaxHealth() * percentDamage;

		DamageInfo damageInfo;
		damageInfo.in.m_damageType = DAMAGE_UNRESISTABLE;
		damageInfo.in.m_deathType = data->m_isBurnedDeathToUnits ? DEATH_BURNED : DEATH_NORMAL;
		damageInfo.in.m_sourceID = getObject()->getID();
		damageInfo.in.m_amount = damage;
		object->attemptDamage( &damageInfo );

		if( !object->isEffectivelyDead() && percentDamage == 1.0f )
			object->kill(); // in case we are carrying flame proof troops we have been asked to kill

		if ( object->isEffectivelyDead() )
		{
			onRemoving( object );
			object->onRemovedFrom( getObject() );
			it = list.erase( it );
		}
		else
		{
			++it;
		}
	}

	// Swap the list back where it belongs.
	m_containList.swap(list);
	m_containListSize = (UnsignedInt)m_containList.size();

#endif // RETAIL_COMPATIBLE_CRC
}

//-------------------------------------------------------------------------------------------------
Bool OpenContain::isEnclosingContainerFor( const Object * ) const
{
	return TRUE; 
}

//-------------------------------------------------------------------------------------------------
Bool OpenContain::hasObjectsWantingToEnterOrExit() const
{
	return !m_objectEnterExitInfo.empty();
}


//-------------------------------------------------------------------------------------------------
void OpenContain::setRallyPoint( const Coord3D *pos )
{
	m_rallyPoint = *pos;
	m_rallyPointExists = true;
}

//-------------------------------------------------------------------------------------------------
const Coord3D *OpenContain::getRallyPoint( void ) const
{
	if (m_rallyPointExists)
		return &m_rallyPoint;

	return NULL;
}

//-------------------------------------------------------------------------------------------------
Bool OpenContain::getNaturalRallyPoint( Coord3D& rallyPoint, Bool offset )  const
{
	Int numberExits = getOpenContainModuleData()->m_numberOfExitPaths;
	if( numberExits > 0 )
	{
		AsciiString endBone("ExitEnd");
		if( numberExits > 1 )
		{
				endBone.concat("01");
		}
		getObject()->getSingleLogicalBonePosition( endBone.str(), &rallyPoint, NULL );
	}
	else
	{
		rallyPoint = *getObject()->getPosition();
	}
	return TRUE;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void OpenContain::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void OpenContain::xfer( Xfer *xfer )
{

	// version 
	const XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// contain list data
	ObjectID objectID;
	if( xfer->getXferMode() == XFER_SAVE )
	{

		// contain list size
		xfer->xferUnsignedInt( &m_containListSize );

		for( ContainedItemsList::const_iterator it = m_containList.begin(); it != m_containList.end(); ++it )
		{

			objectID = (*it)->getID();
			xfer->xferObjectID( &objectID );

		}  // end for, it

	}  // end if, save
	else
	{

		// the containment list should be emtpy at this time
		if( m_containList.empty() == FALSE )
		{
#if 1
			// srj sez: yeah, it SHOULD be, but a few rogue things come into existence (via their ctor) preloaded
			// with stuff (eg, "CabooseFullOfTerrorists"). so just nuke 'em here. sigh...
			m_containListSize = 0;
			for (ContainedItemsList::iterator it = m_containList.begin(); it != m_containList.end(); /*++it*/ )
			{
				// clear as go along, otherwise the obj will try to remove itself, munging things
				Object* tmp = *it;
				it = m_containList.erase(it);
				TheGameLogic->destroyObject(tmp);
			}
			m_containList.clear();
#else
			DEBUG_CRASH(( "OpenContain::xfer - Contain list should be empty before load but is not\n" ));
			throw SC_INVALID_DATA;
#endif

		}  // end if

		// contain list size
		xfer->xferUnsignedInt( &m_containListSize );

		// read all contained items
		for( UnsignedInt i = 0; i < m_containListSize; ++i )
		{

			// read id
			xfer->xferObjectID( &objectID );

			// put on list for later processing once objects are loaded
			m_xferContainIDList.push_back( objectID );

		}  // end for, i

	}  // end else, load

	// player entered mask
	xfer->xferUser( &m_playerEnteredMask, sizeof( PlayerMaskType ) );

	// last unload sound frame
	xfer->xferUnsignedInt( &m_lastUnloadSoundFrame );

	// last load sound frame
	xfer->xferUnsignedInt( &m_lastLoadSoundFrame );

	// stealth units contained
	xfer->xferUnsignedInt( &m_stealthUnitsContained );

	// door close countdown
	xfer->xferUnsignedInt( &m_doorCloseCountdown );

	// conditionstate
	m_conditionState.xfer( xfer );

	// fire points
	xfer->xferUser( &m_firePoints, sizeof( Matrix3D ) * MAX_FIRE_POINTS );

	// fire point start
	xfer->xferInt( &m_firePointStart );

	// fire point next
	xfer->xferInt( &m_firePointNext );

	// fire point size
	xfer->xferInt( &m_firePointSize );

	// no fire points in art
	xfer->xferBool( &m_noFirePointsInArt );

	// rally point
	xfer->xferCoord3D( &m_rallyPoint );

	// rally point exists
	xfer->xferBool( &m_rallyPointExists );

	// enter exit map info
	UnsignedShort enterExitCount = m_objectEnterExitInfo.size();
	xfer->xferUnsignedShort( &enterExitCount );
	ObjectEnterExitType enterExitType;
	if( xfer->getXferMode() == XFER_SAVE )
	{

		ObjectEnterExitMap::const_iterator it;
		for( it = m_objectEnterExitInfo.begin(); it != m_objectEnterExitInfo.end(); ++it )
		{

			// object id
			objectID = (*it).first;
			xfer->xferObjectID( &objectID );

			// enter exit type
			enterExitType = (*it).second;
			xfer->xferUser( &enterExitType, sizeof( ObjectEnterExitType ) );

		}  // end for, it

	}  // end if, save
	else
	{
		
		// the map should be emtpy now
		if( m_objectEnterExitInfo.empty() == FALSE )
		{

			DEBUG_CRASH(( "OpenContain::xfer - m_objectEnterExitInfo should be empty, but is not\n" ));
			throw SC_INVALID_DATA;

		}  // end if

		// read all data items
		for( UnsignedShort i = 0; i < enterExitCount; ++i )
		{

			// object id
			xfer->xferObjectID( &objectID );

			// enter exit type
			xfer->xferUser( &enterExitType, sizeof( ObjectEnterExitType ) );

			// assign
			m_objectEnterExitInfo[ objectID ] = enterExitType;

		}  // end for, i

	}  // end else, load
	
	// which exit path
	xfer->xferInt( &m_whichExitPath );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void OpenContain::loadPostProcess( void )
{
	Object *us = getObject();

	// extend base class
	UpdateModule::loadPostProcess();

	// the containment list should be emtpy at this time
	if( m_containList.empty() == FALSE )
	{

		DEBUG_CRASH(( "OpenContain::loadPostProcess - Contain list should be empty before load but is not\n" ));
		throw SC_INVALID_DATA;

	}  // end if

	// turn the contained id list into actual object pointers in the contain list
	Object *obj;
	std::list<ObjectID>::const_iterator idIt;
	for( idIt = m_xferContainIDList.begin(); idIt != m_xferContainIDList.end(); ++idIt )
	{

		// find this object
		obj = TheGameLogic->findObjectByID( *idIt );

		// sanity
		if( obj == NULL )
		{

			DEBUG_CRASH(( "OpenContain::loadPostProcess - Unable to find object to put on contain list\n" ));
			throw SC_INVALID_DATA;

		}  // end if

		// put object on list
		m_containList.push_back( obj );

		// remove this object from the world if we need to
		if( isEnclosingContainerFor( obj ) )
			addOrRemoveObjFromWorld( obj, FALSE );

		// record in the object who we are contained by
		obj->friend_setContainedBy( us );

	}  // end for, idIt

	// sanity
	DEBUG_ASSERTCRASH( m_containListSize == m_containList.size(),
										 ("OpenContain::loadPostProcess - contain list count mismatch\n") );

	// clear the list as we don't need it anymore
	m_xferContainIDList.clear();

}  // end loadPostProcess
