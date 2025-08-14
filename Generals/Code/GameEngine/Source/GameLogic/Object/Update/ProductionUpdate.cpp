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

// FILE: ProductionUpdate.cpp /////////////////////////////////////////////////////////////////////
// Author: Colin Day, March 2002
// Desc:   This module allows things to be "constructed" from a building
///////////////////////////////////////////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/BitFlagsIO.h"
#include "Common/BuildAssistant.h"
#include "Common/CRCDebug.h"
#include "Common/GameState.h"
#include "Common/Player.h"
#include "Common/Radar.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "Common/Upgrade.h"
#include "Common/Xfer.h"
#include "Common/XferCRC.h"

#include "GameClient/ControlBar.h"
#include "GameClient/Drawable.h"
#include "GameClient/Eva.h"
#include "GameClient/GameText.h"
#include "GameClient/InGameUI.h"

#include "GameLogic/GameLogic.h"
#include "GameLogic/Module/CreateModule.h"
#include "GameLogic/Module/ParkingPlaceBehavior.h"
#include "GameLogic/Module/ProductionUpdate.h"
#include "GameLogic/Object.h"
#include "GameLogic/ScriptEngine.h"


// PUBLIC /////////////////////////////////////////////////////////////////////////////////////////

static const ModelConditionFlagType theOpeningFlags[DOOR_COUNT_MAX] =
{
	MODELCONDITION_DOOR_1_OPENING,
	MODELCONDITION_DOOR_2_OPENING,
	MODELCONDITION_DOOR_3_OPENING,
	MODELCONDITION_DOOR_4_OPENING
};

static const ModelConditionFlagType theClosingFlags[DOOR_COUNT_MAX] =
{
	MODELCONDITION_DOOR_1_CLOSING,
	MODELCONDITION_DOOR_2_CLOSING,
	MODELCONDITION_DOOR_3_CLOSING,
	MODELCONDITION_DOOR_4_CLOSING
};

static const ModelConditionFlagType theWaitingOpenFlags[DOOR_COUNT_MAX] =
{
	MODELCONDITION_DOOR_1_WAITING_OPEN,
	MODELCONDITION_DOOR_2_WAITING_OPEN,
	MODELCONDITION_DOOR_3_WAITING_OPEN,
	MODELCONDITION_DOOR_4_WAITING_OPEN
};

static const ModelConditionFlagType theWaitingToCloseFlags[DOOR_COUNT_MAX] =
{
	MODELCONDITION_DOOR_1_WAITING_TO_CLOSE,
	MODELCONDITION_DOOR_2_WAITING_TO_CLOSE,
	MODELCONDITION_DOOR_3_WAITING_TO_CLOSE,
	MODELCONDITION_DOOR_4_WAITING_TO_CLOSE
};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ProductionUpdateModuleData::ProductionUpdateModuleData( void )
{
	// someday, might need separate times for each door. but not yet.
	m_numDoorAnimations = 0;
	m_doorOpeningTime = 0;
	m_doorWaitOpenTime = 0;
	m_doorClosingTime = 0;
	m_constructionCompleteDuration = 0;
	m_quantityModifiers.clear();
	m_maxQueueEntries = 9;
	m_disabledTypesToProcess = MAKE_DISABLED_MASK(DISABLED_HELD);
}

//-------------------------------------------------------------------------------------------------
/*static*/ void ProductionUpdateModuleData::parseAppendQuantityModifier( INI* ini, void *instance, void *store, const void* /*userData*/ )
{
	ProductionUpdateModuleData* data = (ProductionUpdateModuleData*)instance;
	const char* name = ini->getNextToken();
	const char* countStr = ini->getNextTokenOrNull();
	Int count = countStr ? INI::scanInt(countStr) : 1;

	QuantityModifier qm;
	qm.m_quantity = count;
	qm.m_templateName.set( name );
	data->m_quantityModifiers.push_back( qm );
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/*static*/ void ProductionUpdateModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  UpdateModuleData::buildFieldParse( p );

	static const FieldParse dataFieldParse[] =
	{
		{ "MaxQueueEntries",	INI::parseInt, NULL, offsetof( ProductionUpdateModuleData, m_maxQueueEntries ) },
		{ "NumDoorAnimations",	INI::parseInt, NULL, offsetof( ProductionUpdateModuleData, m_numDoorAnimations ) },
		{ "DoorOpeningTime", INI::parseDurationUnsignedInt, NULL, offsetof( ProductionUpdateModuleData, m_doorOpeningTime ) },
		{ "DoorWaitOpenTime", INI::parseDurationUnsignedInt, NULL, offsetof( ProductionUpdateModuleData, m_doorWaitOpenTime ) },
		{ "DoorCloseTime", INI::parseDurationUnsignedInt, NULL, offsetof( ProductionUpdateModuleData, m_doorClosingTime ) },
		{ "ConstructionCompleteDuration", INI::parseDurationUnsignedInt, NULL, offsetof( ProductionUpdateModuleData, m_constructionCompleteDuration ) },
		{ "QuantityModifier",	parseAppendQuantityModifier, NULL, offsetof( ProductionUpdateModuleData, m_quantityModifiers ) },
		{ "DisabledTypesToProcess",	DisabledMaskType::parseFromINI, NULL, offsetof( ProductionUpdateModuleData, m_disabledTypesToProcess ) },
		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);

}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ProductionEntry::ProductionEntry( void )
{

	m_type = PRODUCTION_INVALID;
	m_objectToProduce = NULL;
	m_upgradeToResearch = NULL;
	m_productionID = (ProductionID)1;
	m_percentComplete = 0.0f;
	m_framesUnderConstruction = 0;
	m_next = NULL;
	m_prev = NULL;
	//Added By Sadullah Nader
	//Initializations inserted
	m_productionQuantityProduced = 0;
	m_productionQuantityTotal = 0;
	//
}  // end ProductionEntry

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ProductionEntry::~ProductionEntry( void )
{

}  // end ~ProductionEntry

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ProductionUpdate::ProductionUpdate( Thing *thing, const ModuleData* moduleData ) :
									UpdateModule( thing, moduleData )
{

	m_productionQueue = NULL;
	m_productionQueueTail = NULL;
	m_productionCount = 0;
	m_uniqueID = (ProductionID)1;
	for (Int i = 0; i < DOOR_COUNT_MAX; ++i)
	{
		m_doors[i].m_doorOpenedFrame = 0;
		m_doors[i].m_doorWaitOpenFrame = 0;
		m_doors[i].m_doorClosedFrame = 0;
		m_doors[i].m_holdOpen = false;
	}
	m_constructionCompleteFrame = 0;
	m_clearFlags.clear();
	m_setFlags.clear();
	m_flagsDirty = FALSE;

}  // end ProductionUpdate

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ProductionUpdate::~ProductionUpdate( void )
{

	// destroy any queued productions
	ProductionEntry *production;
	while( m_productionQueue )
	{

		production = m_productionQueue;
		removeFromProductionQueue( production );
		// TheSuperHackers @fix Mauller 13/04/2025 Delete instance of production item
		deleteInstance(production);

	}  // end while

}  // end ~ProductionUpdate

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
CanMakeType ProductionUpdate::canQueueUpgrade( const UpgradeTemplate *upgrade ) const
{
	if (m_productionCount >= getProductionUpdateModuleData()->m_maxQueueEntries)
		return CANMAKE_QUEUE_FULL;

	return CANMAKE_OK;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
CanMakeType ProductionUpdate::canQueueCreateUnit( const ThingTemplate *unitType ) const
{
	/// @todo srj -- this is horrible, but the "right" way to do it is to move
	// ProductionUpdate to be part of ParkingPlaceBehavior, which I don't currently
	// have time for...
	ParkingPlaceBehaviorInterface* pp = NULL;
	for (BehaviorModule** i = getObject()->getBehaviorModules(); *i; ++i)
	{
		if ((pp = (*i)->getParkingPlaceBehaviorInterface()) != NULL)
		{
			if (pp->shouldReserveDoorWhenQueued(unitType) && !pp->hasAvailableSpaceFor(unitType))
				return CANMAKE_PARKING_PLACES_FULL;
		}
	}

	if (m_productionCount >= getProductionUpdateModuleData()->m_maxQueueEntries)
		return CANMAKE_QUEUE_FULL;

	return CANMAKE_OK;

}

//-------------------------------------------------------------------------------------------------
/** Queue an upgrade to be produced */
//-------------------------------------------------------------------------------------------------
Bool ProductionUpdate::queueUpgrade( const UpgradeTemplate *upgrade )
{

	// sanity
	if( upgrade == NULL )
		return FALSE;

	// get the player
	Player *player = getObject()->getControllingPlayer();

	// sanity check to make sure we can build this upgrade
	if( upgrade->getUpgradeType() == UPGRADE_TYPE_PLAYER &&
			TheUpgradeCenter->canAffordUpgrade( player, upgrade ) == FALSE )
		return FALSE;
	else if( upgrade->getUpgradeType() == UPGRADE_TYPE_OBJECT &&
					 (getObject()->hasUpgrade( upgrade ) == TRUE ||
					  getObject()->affectedByUpgrade( upgrade ) == FALSE) )
		return FALSE;

	// you cannot queue the production of an upgrade twice in this queue
	if( isUpgradeInQueue( upgrade ) == TRUE )
		return FALSE;

 	// STOP cheaters by making sure they can actually build this
 	if( !getObject()->canProduceUpgrade(upgrade) )
 		return FALSE;

	//
	// you cannot queue a player upgrade production if you are producing one already somewhere else
	// (or that somewhere else could even possibly be here)
	//
	if( upgrade->getUpgradeType() == UPGRADE_TYPE_PLAYER &&
      (player->hasUpgradeComplete( upgrade ) || player->hasUpgradeInProduction( upgrade )) )
		return FALSE;

	if (m_productionCount >= getProductionUpdateModuleData()->m_maxQueueEntries)
	{
		DEBUG_CRASH(("Production Queue is full... how did we get here?"));
		return FALSE;
	}

	// take the cost for the build away from the player
	Money *money = player->getMoney();
	money->withdraw( upgrade->calcCostToBuild( player ) );

	// allocate a new production entry
	ProductionEntry *production = newInstance(ProductionEntry);

	// assing production entry data
	production->m_type = PRODUCTION_UPGRADE;
	production->m_upgradeToResearch = upgrade;
	production->m_productionID = PRODUCTIONID_INVALID;  // not needed for upgrades, you can only have one of
																	 // this type in the queue

	// tie to the end of the production queue
	addToProductionQueue( production );

	// add this upgrade as in progress in the player
	player->addUpgrade( upgrade, UPGRADE_STATUS_IN_PRODUCTION );



	return TRUE;  // queued

}  // end queueUpgrade

//-------------------------------------------------------------------------------------------------
/** Cancel an upgrade being produced here */
//-------------------------------------------------------------------------------------------------
void ProductionUpdate::cancelUpgrade( const UpgradeTemplate *upgrade )
{

	// sanity
	if( upgrade == NULL )
		return;

	// get the player
	Player *player = getObject()->getControllingPlayer();

	// sanity, you can't cancel it if the player isn't actually building one
	if( upgrade->getUpgradeType() == UPGRADE_TYPE_PLAYER && player->hasUpgradeInProduction( upgrade ) == FALSE )
		return;

	//
	// find the production entry for this upgrade in the queue here, there can only be one
	// of this type in the queue
	//
	ProductionEntry *production;
	for( production = m_productionQueue; production; production = production->m_next )
	{

		if( production->m_type == PRODUCTION_UPGRADE &&
				production->m_upgradeToResearch == upgrade )
			break;

	}  // end for

	// sanity, entry not found
	if( production == NULL )
		return;

	// refund money back to the player
	Money *money = player->getMoney();
	money->deposit( production->m_upgradeToResearch->calcCostToBuild( player ) );

	// remove this production from the queue
	removeFromProductionQueue( production );

	// delete production instance
	deleteInstance(production);

	//
	// remove the IN_PRODUCTION status of this upgrade from the player, object upgrades don't
	// have any other IN_PRODUCTION status other than their existence in the build queue
	//
	if( upgrade->getUpgradeType() == UPGRADE_TYPE_PLAYER )
		player->removeUpgrade( upgrade );

}  // end cancelUpgrade

//-------------------------------------------------------------------------------------------------
/** Queue the prodcution of a unit.  Returns TRUE if unit was added to queue, FALSE if it
	* was not */
//-------------------------------------------------------------------------------------------------
Bool ProductionUpdate::queueCreateUnit( const ThingTemplate *unitType, ProductionID productionID )
{
	const ProductionUpdateModuleData *data = getProductionUpdateModuleData();

	// if we can't create the unit do nothing
	if( TheBuildAssistant->canMakeUnit( getObject(), unitType ) != CANMAKE_OK )
		return FALSE;

	ExitDoorType exitDoor = DOOR_NONE_AVAILABLE;

	/// @todo srj -- this is horrible, but the "right" way to do it is to move
	// ProductionUpdate to be part of ParkingPlaceBehavior, which I don't currently
	// have time for...
	ParkingPlaceBehaviorInterface* pp = NULL;
	for (BehaviorModule** i = getObject()->getBehaviorModules(); *i; ++i)
	{
		if ((pp = (*i)->getParkingPlaceBehaviorInterface()) != NULL)
		{
			if (pp->shouldReserveDoorWhenQueued(unitType))
			{
				ExitInterface* exitInterface = getObject()->getObjectExitInterface();
				if (exitInterface)
				{
					exitDoor = exitInterface->reserveDoorForExit(unitType, NULL);
				}
				if (exitDoor == DOOR_NONE_AVAILABLE)
				{
					return FALSE;
				}
				break;
			}
		}
	}

	if (m_productionCount >= getProductionUpdateModuleData()->m_maxQueueEntries)
	{
		DEBUG_CRASH(("Production Queue is full... how did we get here?"));
		return FALSE;
	}

	// take the cost for the build away from the player
	Player *player = getObject()->getControllingPlayer();
	Money *money = player->getMoney();
	money->withdraw( unitType->calcCostToBuild( player ) );

	// allocate a new production entry
	ProductionEntry *production = newInstance(ProductionEntry);

	// Check to see if there is a quantity modifier entry. What this means is if we are building a particular unit
	// and it has a modifier, we build that number of objects instead of just one. A good example is the Chinese
	// barracks building redguards -- they are built 4 at a time, but the user or script only pays for one and
	// builds four for that price!
	production->m_productionQuantityTotal = 1;
	production->m_productionQuantityProduced = 0;
	for( std::vector<QuantityModifier>::const_iterator it = data->m_quantityModifiers.begin(); it != data->m_quantityModifiers.end(); ++it )
  {
		const ThingTemplate* productionTemplate = TheThingFactory->findTemplate( it->m_templateName );
		if( productionTemplate && productionTemplate->isEquivalentTo( unitType ) )
		{
			production->m_productionQuantityTotal = it->m_quantity;
			break;
		}
	}

	// assing production entry data
	production->m_type = PRODUCTION_UNIT;
	production->m_objectToProduce = unitType;
	production->m_productionID = productionID;
	production->m_exitDoor = exitDoor;

	// tie to the end of the production queue
	addToProductionQueue( production );

	return TRUE;  // unit queued

}  // end queueMakeUnit

//-------------------------------------------------------------------------------------------------
/** Cancel the construction of the unit with the matching production ID */
//-------------------------------------------------------------------------------------------------
void ProductionUpdate::cancelUnitCreate( ProductionID productionID )
{

	// search for the production entry in our queue
	ProductionEntry *production;
	for( production = m_productionQueue; production; production = production->m_next )
	{

		// are we at the one we want get rid of it
		if( production->m_productionID == productionID )
		{

			// give the player the cost of the object back
			Player *player = getObject()->getControllingPlayer();
			Money *money = player->getMoney();
			money->deposit( production->m_objectToProduce->calcCostToBuild( player ) );

			// remove from queue list
			removeFromProductionQueue( production );

			// delete the production entry
			deleteInstance(production);

			return;

		}  // end if

	}  // end for

}  // end cancelUnitCreate

//-------------------------------------------------------------------------------------------------
/** Cancel all production of type unitType */
//-------------------------------------------------------------------------------------------------
void ProductionUpdate::cancelAllUnitsOfType( const ThingTemplate *unitType)
{

	// search for the production entry in our queue
	ProductionEntry *production;
	for( production = m_productionQueue; production; )
	{

		// are we at the one we want get rid of it
		if( production->getProductionType() == PRODUCTION_UNIT &&
		    production->getProductionObject() == unitType )
		{
			ProductionEntry *temp = production->m_next;

			// cancel the production
			cancelUnitCreate( production->getProductionID() );

			// advance
			production = temp;

		}  // end if
		else
		{

			// advance
			production = production->m_next;

		}  // end else

	}  // end for

}  // end cancelAllUnitsOfType

//-------------------------------------------------------------------------------------------------
/** Update the door behavior */
//-------------------------------------------------------------------------------------------------
void ProductionUpdate::updateDoors()
{
	const ProductionUpdateModuleData *d = getProductionUpdateModuleData();

	UnsignedInt now = TheGameLogic->getFrame();
	for (Int i = 0; i < DOOR_COUNT_MAX; ++i)
	{
		//
		// if we have an open door, see if enough time has passed for us to close it ... likewise
		// if we were closing a door we will clear the closing condition after an amount of time
		// has passed
		//
		if( m_doors[i].m_doorOpenedFrame )
		{

			if( now - m_doors[i].m_doorOpenedFrame > d->m_doorOpeningTime )
			{

				// set our frame markers for door states
				m_doors[i].m_doorOpenedFrame = 0;
				m_doors[i].m_doorWaitOpenFrame = now;

				// set the flags that will show the difference in the model
				m_clearFlags.set( theOpeningFlags[i], true );
				m_setFlags.set( theOpeningFlags[i], false );

				m_setFlags.set( theWaitingOpenFlags[i] );
				m_flagsDirty = TRUE;

			}  // end if

		}  // end if
		else if( m_doors[i].m_doorWaitOpenFrame )
		{

			if( now - m_doors[i].m_doorWaitOpenFrame > d->m_doorWaitOpenTime && !m_doors[i].m_holdOpen )
			{

				// set our frame marker for closing
				m_doors[i].m_doorWaitOpenFrame = 0;
				m_doors[i].m_doorClosedFrame = now;

				// set the flags that show the difference in the art
				m_clearFlags.set( theWaitingOpenFlags[i], true );
				m_setFlags.set( theWaitingOpenFlags[i], false );

				m_setFlags.set( theClosingFlags[i] );
				m_flagsDirty = TRUE;

			}  // end if

		}  // end if
		else if( m_doors[i].m_doorClosedFrame && !m_doors[i].m_holdOpen )
		{

			if( now - m_doors[i].m_doorClosedFrame > d->m_doorClosingTime )
			{

				// set our frame marker for the closed door frame
				m_doors[i].m_doorClosedFrame = 0;

				// set the flags that will show the difference in the model
				m_clearFlags.set( theClosingFlags[i], true );
				m_setFlags.set( theClosingFlags[i], false );
				m_flagsDirty = TRUE;

			}  // end if

		}  // end else if
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UpdateSleepTime ProductionUpdate::update( void )
{
/// @todo srj use SLEEPY_UPDATE here
	ProductionEntry *production = m_productionQueue;
	const ProductionUpdateModuleData *d = getProductionUpdateModuleData();
	UnsignedInt now = TheGameLogic->getFrame();
	Object *us = getObject();

	// update the door behaviors
	if( d->m_numDoorAnimations > 0 )
		updateDoors();

	//
	// if we're in the construction complete state ... we need to check frame
	// we're in it and get out of it when the time is up
	//
	if( m_constructionCompleteFrame )
	{

		if( now - m_constructionCompleteFrame > d->m_constructionCompleteDuration )
		{

			// set our frame marker to be out of construction complete state
			m_constructionCompleteFrame = 0;

			// set the flags that show the difference in the model
			m_clearFlags.set( MODELCONDITION_CONSTRUCTION_COMPLETE, true );
			m_setFlags.set( MODELCONDITION_CONSTRUCTION_COMPLETE, false );
			m_flagsDirty = TRUE;

		}  // end if

	}  // end if

	// if we have dirty bits that need to be set and cleared, do it here all at once
	if( m_flagsDirty == TRUE )
	{
		Drawable *draw = us->getDrawable();

		if( draw )
			draw->clearAndSetModelConditionFlags( m_clearFlags, m_setFlags );

		// clear the things we just set and turn off our dirty flag
		m_clearFlags.clear();
		m_setFlags.clear();
		m_flagsDirty = FALSE;

	}  // end if

	// if nothing in the queue get outta here
	if( production == NULL )
		return UPDATE_SLEEP_NONE;

	//
	// if we've become OBJECT_STATUS_SOLD, halt all production ... leave things in the
	// queue because if the sell process completes we get money back for them, for now we
	// will be just frozen in time
	// Actually, there will be nothing in the queue since everything gets cancel/refunded
	// at the start of sell, but we still don't want to do anything here.
	//
	if( us->getStatusBits().test( OBJECT_STATUS_SOLD ) )
		return UPDATE_SLEEP_NONE;

	// get the player that is building this thing
	Player *player = us->getControllingPlayer();

	// sanity
	if( player == NULL )
	{

		// remove from queue list
		removeFromProductionQueue( production );

		// delete the production entry
		deleteInstance(production);

		return UPDATE_SLEEP_NONE;

	}  // end if

	//
	// you can disallow types of units on the fly in scripts, so if there is something
	// in the queue that suddenly can't be build then get rid of it
	//
	if( production->getProductionType() == PRODUCTION_UNIT &&
			player->allowedToBuild( production->getProductionObject() ) == FALSE )
	{

		// Don't cancel dozers in the queue.  jba.
		if (!production->getProductionObject()->isKindOf(KINDOF_DOZER))
		{

			cancelUnitCreate(production->getProductionID());
			return UPDATE_SLEEP_NONE;

		}  // end if

	}  // end if

	// increase the frames we've been under production for
	production->m_framesUnderConstruction++;

	// how many total logic frames does it take to produce this unit
	Int totalProductionFrames;
	if( production->m_type == PRODUCTION_UNIT )
		totalProductionFrames = production->m_objectToProduce->calcTimeToBuild( player );
	else
		totalProductionFrames = production->m_upgradeToResearch->calcTimeToBuild( player );

	// figure out our percent complete
	production->m_percentComplete = INT_TO_REAL( production->m_framesUnderConstruction ) /
																	INT_TO_REAL( totalProductionFrames ) *
																	100.0f;

	// if we've reached 100% or more we're done, tada!
	if( production->m_percentComplete >= 100.0f )
	{

		// handle unit production items
		if( production->m_type == PRODUCTION_UNIT )
		{
			Object *creationBuilding = us;

			// create this unit

			//if I've got a ExitModule
			ExitInterface *exitInterface = creationBuilding->getObjectExitInterface();
			if( exitInterface )
			{

				//
				// only exit if the exit interface will let us ... if we can't we'll just keep this
				// unit in the production and keep "overbuilding" it, every frame we will check to see
				// if we can exit and then move onto production of the next one
				//
				Int numberToTry = production->getProductionQuantityRemaining();// this will change midloop, so it can't be the For clause
				for( int i = 0; i < numberToTry; i++ )
				{
					ExitDoorType exitDoor = production->getExitDoor();
					if (exitDoor == DOOR_NONE_AVAILABLE)
					{
						exitDoor = exitInterface->reserveDoorForExit(production->m_objectToProduce, NULL);
						production->setExitDoor(exitDoor);
					}

					if (exitDoor != DOOR_NONE_AVAILABLE)
					{
						// note, could be DOOR_NONE_NEEDED! so door could be null. (srj)
						DoorInfo* door = (exitDoor >= 0 && exitDoor < DOOR_COUNT_MAX) ? &m_doors[exitDoor] : NULL;

						//
						// if the producing structure has a door opening animation we will set the condition
						// bits to open the door (note that we must also take care to remove any condition
						// that had us previously closing a door)
						//
						const ProductionUpdateModuleData *d = getProductionUpdateModuleData();
						if( d->m_numDoorAnimations > 0 && door != NULL )
						{

							// if the door is closed, open it
							if( door->m_doorOpenedFrame == 0 && door->m_doorWaitOpenFrame == 0 && door->m_doorClosedFrame == 0 )
							{

								door->m_doorOpenedFrame = now;
								m_setFlags.set( theOpeningFlags[exitDoor] );
								m_flagsDirty = TRUE;

							}  // end if
							// if the door is waiting-open, keep it there
							else if( door->m_doorWaitOpenFrame != 0 )
							{

								door->m_doorWaitOpenFrame = now;

							}  // end else if
							// if the door is closing, for now, pop it to waiting open
							else if( door->m_doorClosedFrame != 0 )
							{

								door->m_doorWaitOpenFrame = now;

								m_clearFlags.set( theOpeningFlags[exitDoor], true );
								m_clearFlags.set( theClosingFlags[exitDoor], true );
								m_setFlags.set( theOpeningFlags[exitDoor], false );
								m_setFlags.set( theClosingFlags[exitDoor], false );

								m_setFlags.set( theWaitingOpenFlags[exitDoor] );
								m_flagsDirty = TRUE;

							}  // end else

						}  // end if, has door animation

						// we now go into the construction complete condition for a while
						if( m_constructionCompleteFrame == 0 )
						{

							m_constructionCompleteFrame = now;
							m_setFlags.set( MODELCONDITION_CONSTRUCTION_COMPLETE );
							m_flagsDirty = TRUE;

						}  // end if

						//
						// make a new object, note that for production buildings that have door
						// animations we will not make the object until the door has been totally
						// opened
						//
						if( d->m_numDoorAnimations == 0 || door == NULL || door->m_doorWaitOpenFrame != 0 )
						{
							Object *newObj = TheThingFactory->newObject( production->m_objectToProduce,
																	creationBuilding->getControllingPlayer()->getDefaultTeam() );

							newObj->setProducer(creationBuilding);

							// call the exit interface to do the rally point and position stuff
							exitInterface->exitObjectViaDoor( newObj, exitDoor );

							// since we successfully exited via this door, we should NOT call unreserveDoorForExit
							// when we toss this production entry. so set it back to an innocuous value.
							production->setExitDoor(DOOR_NONE_AVAILABLE);

							// Now, play the sound associated with the new object's production.
							AudioEventRTS voiceCreate = *newObj->getTemplate()->getVoiceCreated();
							voiceCreate.setObjectID(newObj->getID());
							TheAudio->addAudioEvent(&voiceCreate);

							// call the onUnitCreated for the player
							creationBuilding->getControllingPlayer()->onUnitCreated( creationBuilding, newObj );

							// onCreates have been called on newObj, and after that the owner was set,
							// so now is the time to call the game side of CreateModules
							for (BehaviorModule** m = newObj->getBehaviorModules(); *m; ++m)
							{
								CreateModuleInterface* create = (*m)->getCreate();
								if (!create)
									continue;
								create->onBuildComplete();
							}

							if( production->getProductionQuantity() == production->getProductionQuantityRemaining() )
							{
								//Call the voice created for the 1st object -- because it's possible to create multiple objects like redguards!
								AudioEventRTS sound = *newObj->getTemplate()->getPerUnitSound( "VoiceCreate" );
								sound.setObjectID( newObj->getID() );
								TheAudio->addAudioEvent( &sound );
							}

							//We created one guy, but we may want to do more so we should stay in this node of production.
							// This is last so the voice check can easily check for "first" guy
							production->oneProductionSuccessful();

						}  // end if, door open or no door animation ... make the object

					}  // end, if we got a door reservation

				} //end of trying to exit all the things we were planning on attempting

				if( production->getProductionQuantityRemaining() == 0 )
				{
					// remove this production entry so we can go on to the next if we are totally finished
					removeFromProductionQueue( production );

					// delete the production entry
					deleteInstance(production);
				}

			}  // end if we found an exit interface
			else
			{

				// there is no exit interface, this is an error
				DEBUG_ASSERTCRASH( 0, ("Cannot create '%s', there is no ExitUpdate interface defined for producer object '%s'",
															production->m_objectToProduce->getName().str(),
															creationBuilding->getTemplate()->getName().str()) );

				// remove this item from the production queue
				removeFromProductionQueue( production );

				// delete the production entry
				deleteInstance(production);

			}  // end else

		}  // end if, production unit
		else if( production->m_type == PRODUCTION_UPGRADE )
		{
			const UpgradeTemplate *upgrade = production->m_upgradeToResearch;

			// we finished an upgrade, lets add that money spent on it to the scorekeeper
			player->getScoreKeeper()->addMoneySpent(upgrade->calcCostToBuild(player));

			// notify the script engine
			TheScriptEngine->notifyOfCompletedUpgrade(
				us->getControllingPlayer()->getPlayerIndex(),
				upgrade->getUpgradeName(),
				us->getID());

			// print a message to the local player
			if( us->isLocallyControlled() )
			{
				UnicodeString msg;
				UnicodeString format = TheGameText->fetch( "UPGRADE:UpgradeComplete" );
				UnicodeString upgradeName = TheGameText->fetch( upgrade->getDisplayNameLabel().str() );

				msg.format( format.str(), upgradeName.str() );
				TheInGameUI->message( msg );

				// upgrades are a more rare event, play a nifty radar event thingie
				TheRadar->createEvent( us->getPosition(), RADAR_EVENT_UPGRADE );

				//Play the sound for the upgrade, because we just built it!
				AudioEventRTS sound = *upgrade->getResearchCompleteSound();
				if( TheAudio->isValidAudioEvent( &sound ) )
				{
					//We have a custom upgrade complete sound.
					sound.setObjectID( us->getID() );
					TheAudio->addAudioEvent( &sound );
				}
				else
				{
					//Use a generic EVA event.
					TheEva->setShouldPlay(EVA_UpgradeComplete);
				}

				//Play any available unit specific sound for upgrade.
				sound = *upgrade->getUnitSpecificSound();
				sound.setObjectID( us->getID() );
				TheAudio->addAudioEvent( &sound );

			}  // end if

			// update the upgrade status in the player or give the upgrade to the object
			if( upgrade->getUpgradeType() == UPGRADE_TYPE_PLAYER )
			{
				player->addUpgrade( upgrade, UPGRADE_STATUS_COMPLETE );
			}
			else
			{
				us->giveUpgrade( upgrade );
			}

			//Also mark the UI dirty -- incase object with upgrade cameo is selected.
			Drawable *draw = TheInGameUI->getFirstSelectedDrawable();
			Object *selectedObject = draw ? draw->getObject() : NULL;
			if( selectedObject )
			{
				const ThingTemplate *thing = selectedObject->getTemplate();
				for( Int i = 0; i < MAX_UPGRADE_CAMEO_UPGRADES; i++ )
				{
					AsciiString upgradeName = thing->getUpgradeCameoName( i );
					const UpgradeTemplate *testUpgrade = TheUpgradeCenter->findUpgrade( upgradeName );
					if( testUpgrade == upgrade )
					{
						//Our selected object has the upgrade
						TheControlBar->markUIDirty();
						break;
					}
				}
			}

			// remove this production entry so we can go on to the next
			removeFromProductionQueue( production );

			// delete the production entry
			deleteInstance(production);

		}  // end else, production upgrade

	}  // end if, production is 100% complete

	return UPDATE_SLEEP_NONE;
}  // end update

//-------------------------------------------------------------------------------------------------
/** Add the production entry to the *END* of the production queue list */
//-------------------------------------------------------------------------------------------------
void ProductionUpdate::addToProductionQueue( ProductionEntry *production )
{

	// check for empty list
	if( m_productionQueue == NULL )
		m_productionQueue = production;

	// make any existing tail pointer now point to us, and we point back to them
	if( m_productionQueueTail )
	{

		m_productionQueueTail->m_next = production;
		production->m_prev = m_productionQueueTail;

	}  // end if

	// this production entry is now the new tail at the end of the list
	m_productionQueueTail = production;

	// we now have one more production item
	++m_productionCount;

	// when something is in the queue ... we are in the actively constructing state
	Drawable *draw = getObject()->getDrawable();
	if( draw )
	{
		ModelConditionFlags condition = draw->getModelConditionFlags();

		if( condition.test( MODELCONDITION_ACTIVELY_CONSTRUCTING ) == FALSE )
		{

			m_setFlags.set( MODELCONDITION_ACTIVELY_CONSTRUCTING );
			m_flagsDirty = TRUE;

		}  // end if

	}  // end if

}  // end addToProductionQueue

//-------------------------------------------------------------------------------------------------
/** Remove the production entry from the production queue list */
//-------------------------------------------------------------------------------------------------
void ProductionUpdate::removeFromProductionQueue( ProductionEntry *production )
{
	if (production->m_type == PRODUCTION_UNIT &&
			production->m_exitDoor != DOOR_NONE_AVAILABLE)
	{
		ExitInterface* exitInterface = getObject()->getObjectExitInterface();
		if (exitInterface)
		{
			exitInterface->unreserveDoorForExit(production->m_exitDoor);
		}
	}

	// detach prev pointer, keep head pointer to the whole queue in tact
	if( production->m_prev )
		production->m_prev->m_next = production->m_next;
	else
		m_productionQueue = production->m_next;

	// detach next pointer, keep tail poitner to the whole queue in tact
	if( production->m_next )
		production->m_next->m_prev = production->m_prev;
	else
		m_productionQueueTail = production->m_prev;

	// we now have one less production item
	--m_productionCount;

	// when we go back to zero things in our queue, we clear the constructing state
	Drawable *draw = getObject()->getDrawable();
	if( draw )
	{
		ModelConditionFlags condition = draw->getModelConditionFlags();

		if( m_productionCount == 0 && condition.test( MODELCONDITION_ACTIVELY_CONSTRUCTING ) == TRUE )
		{

			m_clearFlags.set( MODELCONDITION_ACTIVELY_CONSTRUCTING, true );
			m_setFlags.set( MODELCONDITION_ACTIVELY_CONSTRUCTING, false );
			m_flagsDirty = TRUE;

		}  // end if

	}  // end if

/*
	// Debugging
	UnicodeString msg;
	if( production->getProductionType() == ProductionEntry::PRODUCTION_UNIT )
	{
		msg.format( L"Removed unit '%S'(%d)", production->getProductionObject()->getName().str(),
																					production->getProductionID() );
		TheInGameUI->message( msg );
	}
	else
	{
		msg.format( L"Remove upgrade '%S'", production->getProductionUpgrade()->getName().str() );
		TheInGameUI->message( msg );
	}
*/

}  // end removeFromProductionQueue

//-------------------------------------------------------------------------------------------------
/** Is the upgrade already in the production queue.  Note that you can only have one
	* production entry for any given upgrade in the queue */
//-------------------------------------------------------------------------------------------------
Bool ProductionUpdate::isUpgradeInQueue( const UpgradeTemplate *upgrade ) const
{
	const ProductionEntry *production;

	for( production = firstProduction(); production; production = nextProduction( production ) )
		if( production->getProductionType() == PRODUCTION_UPGRADE &&
				production->getProductionUpgrade() == upgrade )
			return TRUE;

	return FALSE;  // not in queue

}  // end isUpgradeInQueue

// ------------------------------------------------------------------------------------------------
/** count number of units with matching unit type in the production queue */
// ------------------------------------------------------------------------------------------------
UnsignedInt ProductionUpdate::countUnitTypeInQueue( const ThingTemplate *unitType ) const
{
	UnsignedInt count = 0;
	const ProductionEntry *production;

	for( production = firstProduction(); production; production = nextProduction( production ) )
		if( production->getProductionType() == PRODUCTION_UNIT &&
				production->getProductionObject() == unitType )
			count++;

	return count;

}  // end countUnitTypeInQueue

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void ProductionUpdate::onDie( const DamageInfo *damageInfo )
{
	// we need to cancel all of our production on death
	cancelAndRefundAllProduction();
}

// ------------------------------------------------------------------------------------------------
/** Cancel each of the production items in the queue.  By going through the actual cancel
	* methods, the cost of each of those items will be refunded to the player */
// ------------------------------------------------------------------------------------------------
void ProductionUpdate::cancelAndRefundAllProduction( void )
{
  // Empirically, in release the code can loop forever.  So we limit to 100 passes. jba. [8/31/2003]
  const Int productionLimit = 100;// With luck, we never queue up 100 units. [8/31/2003]
  Int i;
  for (i=0; i<productionLimit; i++)
  {
    // iterate through our production queue
    if( m_productionQueue )
    {
      if( m_productionQueue->getProductionType() == PRODUCTION_UNIT )
        cancelUnitCreate( m_productionQueue->getProductionID() );
      else if( m_productionQueue->getProductionType() == PRODUCTION_UPGRADE )
        cancelUpgrade( m_productionQueue->getProductionUpgrade() );
      else
      {
        // unknown production type
        DEBUG_CRASH(( "ProductionUpdate::cancelAndRefundAllProduction - Unknown production type '%d'",
                      m_productionQueue->getProductionType() ));
        return;
      }  // end else
    }  // end if
  }
}  // end cancelAndRefundAllProduction

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void ProductionUpdate::setHoldDoorOpen(ExitDoorType exitDoor, Bool holdIt)
{
	if (exitDoor >= DOOR_1 && exitDoor < DOOR_COUNT_MAX)
	{
		DoorInfo& door = m_doors[exitDoor];
		door.m_holdOpen = holdIt;
		if (holdIt && door.m_doorOpenedFrame == 0 && door.m_doorWaitOpenFrame == 0 && door.m_doorClosedFrame == 0)
		{
			door.m_doorOpenedFrame = TheGameLogic->getFrame();
			m_setFlags.set( theOpeningFlags[exitDoor] );
			m_flagsDirty = TRUE;
		}
	}
}

// ------------------------------------------------------------------------------------------------
/** Helper method to retrieve a production update interface from an object if one is present */
// ------------------------------------------------------------------------------------------------
/*static*/ ProductionUpdateInterface *ProductionUpdate::getProductionUpdateInterfaceFromObject( Object *obj )
{

	// sanity
	if( obj == NULL )
		return NULL;

	BehaviorModule **bmi;
	ProductionUpdateInterface *pui = NULL;
	for( bmi = obj->getBehaviorModules(); *bmi; ++bmi )
	{

		pui = (*bmi)->getProductionUpdateInterface();
		if( pui )
			return pui;

	}  // end for, bmi

	// interface not found
	return NULL;


}  // end getProductionUpdateInterfaceFromObject

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void ProductionUpdate::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void ProductionUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// production queue count
	ProductionEntry *production;
	UnsignedShort productionCount = 0;
	for( production = m_productionQueue; production; production = production->m_next )
		productionCount++;
	xfer->xferUnsignedShort( &productionCount );

	// production queue data
	if( xfer->getXferMode() == XFER_SAVE )
	{
		AsciiString name;

		// write all queue data
		for( production = m_productionQueue; production; production = production->m_next )
		{

			// type
			xfer->xferUser( &production->m_type, sizeof( ProductionType ) );

			// thing/upgrade template name
			if( production->m_type == PRODUCTION_UNIT )
				name = production->m_objectToProduce->getName();
			else
				name = production->m_upgradeToResearch->getUpgradeName();
			xfer->xferAsciiString( &name );

			// production ID
			xfer->xferUser( &production->m_productionID, sizeof( ProductionID ) );

			// percent complete
			xfer->xferReal( &production->m_percentComplete );

			// frames under construction
			xfer->xferInt( &production->m_framesUnderConstruction );

			// production quantity
			xfer->xferInt( &production->m_productionQuantityTotal );

			// production quantity in progress
			xfer->xferInt( &production->m_productionQuantityProduced );

			// exit door
			xfer->xferInt( (Int*)&production->m_exitDoor );

		}  // end for

	}  // end if, save
	else
	{
		AsciiString name;

		// the queue should be emtpy now
		if( m_productionQueue != NULL )
		{

			DEBUG_CRASH(( "ProductionUpdate::xfer - m_productionQueue is not empty, but should be" ));
			throw SC_INVALID_DATA;

		}  // end if

		// read each element
		for( UnsignedShort i = 0; i < productionCount; ++i )
		{

			// allocate new production entry
			production = newInstance(ProductionEntry);

			// tie to list at end
			if( m_productionQueue == NULL )
				m_productionQueue = production;

			// make any existing tail pointer now point to us, and we point back to them
			if( m_productionQueueTail )
			{

				m_productionQueueTail->m_next = production;
				production->m_prev = m_productionQueueTail;

			}  // end if

			// this production entry is now the new tail at the end of the list
			m_productionQueueTail = production;

			// type
			xfer->xferUser( &production->m_type, sizeof( ProductionType ) );

			// thing/upgrade template name
			xfer->xferAsciiString( &name );
			if( production->m_type == PRODUCTION_UNIT )
			{

				production->m_objectToProduce = TheThingFactory->findTemplate( name );
				if( production->m_objectToProduce == NULL )
				{

					DEBUG_CRASH(( "ProductionUpdate::xfer - Cannot find template '%s'", name.str() ));
					throw SC_INVALID_DATA;

				}  // end if

			}  // end if, unit production
			else
			{

				production->m_upgradeToResearch = TheUpgradeCenter->findUpgrade( name );
				if( production->m_upgradeToResearch == NULL )
				{

					DEBUG_CRASH(( "ProductionUpdate::xfer - Cannot find upgrade '%s'", name.str() ));
					throw SC_INVALID_DATA;

				}  // end if

			}  // end else, upgrade production

			// production ID
			xfer->xferUser( &production->m_productionID, sizeof( ProductionID ) );

			// percent complete
			xfer->xferReal( &production->m_percentComplete );

			// frames under construction
			xfer->xferInt( &production->m_framesUnderConstruction );

			// production quantity
			xfer->xferInt( &production->m_productionQuantityTotal );

			// production quantity in progress
			xfer->xferInt( &production->m_productionQuantityProduced );

			// exit door
			xfer->xferInt( (Int*)&production->m_exitDoor );

		}  // end for, i

	}  // end else, load

	// unique id
	xfer->xferUser( &m_uniqueID, sizeof( ProductionID ) );

	// production count
	xfer->xferUnsignedInt( &m_productionCount );

	// construction complete frame
	xfer->xferUnsignedInt( &m_constructionCompleteFrame );

	// door info
	xfer->xferUser( &m_doors, sizeof( DoorInfo ) * DOOR_COUNT_MAX );

	// clear flags
	m_clearFlags.xfer( xfer );

	// set flags
	m_setFlags.xfer( xfer );

	// flags dirty
	xfer->xferBool( &m_flagsDirty );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void ProductionUpdate::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}  // end loadPostProcess
