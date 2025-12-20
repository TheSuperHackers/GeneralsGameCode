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

// FILE: RiderChangeContain.cpp //////////////////////////////////////////////////////////////////////
// Author: Kris Morness, May 2003
// Desc:   Contain module for the combat bike (transport that switches units).
///////////////////////////////////////////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#define DEFINE_LOCOMOTORSET_NAMES //Gain access to TheLocomotorSetNames[]

#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/ThingTemplate.h"
#include "Common/ThingFactory.h"
#include "Common/Xfer.h"

#include "GameClient/ControlBar.h"
#include "GameClient/Drawable.h"
#include "GameClient/InGameUI.h"

#include "GameLogic/AI.h"
#include "GameLogic/AIPathfind.h"
#include "GameLogic/ExperienceTracker.h"
#include "GameLogic/Object.h"
#include "GameLogic/Locomotor.h"

#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/Module/PhysicsUpdate.h"
#include "GameLogic/Module/StealthUpdate.h"
#include "GameLogic/Module/RiderChangeContain.h"



// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
RiderChangeContainModuleData::RiderChangeContainModuleData()
{
	m_scuttleFrames = 0;
	m_scuttleState = MODELCONDITION_TOPPLED;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void RiderChangeContainModuleData::parseRiderInfo( INI* ini, void *instance, void *store, const void* /*userData*/ )
{
	RiderInfo* rider = (RiderInfo*)store;
	const char* name = ini->getNextToken();

	//Template name
	rider->m_templateName.format( name );

	//Model condition state
	INI::parseIndexList( ini, instance, &(rider->m_modelConditionFlagType), ModelConditionFlags::getBitNames() );

	//Weaponset
	INI::parseIndexList( ini, instance, &(rider->m_weaponSetFlag), WeaponSetFlags::getBitNames() );

	//Object status
	INI::parseIndexList( ini, instance, &(rider->m_objectStatusType), ObjectStatusMaskType::getBitNames() );

	//Command set override
	name = ini->getNextToken();
	rider->m_commandSet.format( name );

	//Locomotor set type
	rider->m_locomotorSetType = (LocomotorSetType)INI::scanIndexList( ini->getNextToken(), TheLocomotorSetNames );
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void RiderChangeContainModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  TransportContainModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "Rider1",					parseRiderInfo,					NULL, offsetof( RiderChangeContainModuleData, m_riders[0] ) },
		{ "Rider2",					parseRiderInfo,					NULL, offsetof( RiderChangeContainModuleData, m_riders[1] ) },
		{ "Rider3",					parseRiderInfo,					NULL, offsetof( RiderChangeContainModuleData, m_riders[2] ) },
		{ "Rider4",					parseRiderInfo,					NULL, offsetof( RiderChangeContainModuleData, m_riders[3] ) },
		{ "Rider5",					parseRiderInfo,					NULL, offsetof( RiderChangeContainModuleData, m_riders[4] ) },
		{ "Rider6",					parseRiderInfo,					NULL, offsetof( RiderChangeContainModuleData, m_riders[5] ) },
		{ "Rider7",					parseRiderInfo,					NULL, offsetof( RiderChangeContainModuleData, m_riders[6] ) },
		{ "Rider8",					parseRiderInfo,					NULL, offsetof( RiderChangeContainModuleData, m_riders[7] ) },
    { "ScuttleDelay",   INI::parseDurationUnsignedInt,	NULL, offsetof( RiderChangeContainModuleData, m_scuttleFrames ) },
    { "ScuttleStatus",  INI::parseIndexList,		ModelConditionFlags::getBitNames(), offsetof( RiderChangeContainModuleData, m_scuttleState ) },
		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}



// PRIVATE ////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Int RiderChangeContain::getContainMax( void ) const
{
	if (getRiderChangeContainModuleData())
		return getRiderChangeContainModuleData()->m_slotCapacity;

	return 0;
}

// PUBLIC /////////////////////////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
RiderChangeContain::RiderChangeContain( Thing *thing, const ModuleData *moduleData ) :
								 TransportContain( thing, moduleData )
{
	m_extraSlotsInUse = 0;
	m_frameExitNotBusy = 0;
	m_containing = FALSE;
	m_scuttledOnFrame = 0;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
RiderChangeContain::~RiderChangeContain( void )
{

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/**
	can this container contain this kind of object?
	and, if checkCapacity is TRUE, does this container have enough space left to hold the given unit?
*/
Bool RiderChangeContain::isValidContainerFor(const Object* rider, Bool checkCapacity) const
{
	//Don't check capacity because our rider will kick the other rider out!
	if( TransportContain::isValidContainerFor( rider, FALSE ) )
	{
		if( m_scuttledOnFrame != 0 )
		{
			//Scuttled... too late!
			return FALSE;
		}

		//We can enter this bike... but now we need to extend the base functionality by limiting
		//which infantry can enter.
		const RiderChangeContainModuleData *data = getRiderChangeContainModuleData();
		for( int i = 0; i < MAX_RIDERS; i++ )
		{
			const ThingTemplate *thing = TheThingFactory->findTemplate( data->m_riders[ i ].m_templateName );
			if( thing && thing->isEquivalentTo( rider->getTemplate() ) )
			{
				//We found a valid rider, so return success.
				return TRUE;
			}
		}
	}
	return FALSE;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void RiderChangeContain::onContaining( Object *rider, Bool wasSelected )
{
	Object *obj = getObject();
	m_containing = TRUE;
	//Remove our existing rider
	if( m_payloadCreated )
	{
		obj->getAI()->aiEvacuateInstantly( TRUE, CMD_FROM_AI );
	}

	//If the rider is currently selected, transfer selection to the container and preserve other units
	//that may be already selected. Note that containing the rider will automatically cause it to be
	//deselected, so all we have to do is select the container (if not already selected)!
	Drawable *containDraw = getObject()->getDrawable();
	if( containDraw && wasSelected && !containDraw->isSelected() )
	{
		//Create the selection message
		GameMessage *teamMsg = TheMessageStream->appendMessage( GameMessage::MSG_CREATE_SELECTED_GROUP );
		teamMsg->appendBooleanArgument( FALSE );// not creating new team so pass false
		teamMsg->appendObjectIDArgument( getObject()->getID() );
		TheInGameUI->selectDrawable( containDraw );
		TheInGameUI->setDisplayedMaxWarning( FALSE );
	}

	//Find the rider in the list and set the appropriate model condition
	const RiderChangeContainModuleData *data = getRiderChangeContainModuleData();
	for( int i = 0; i < MAX_RIDERS; i++ )
	{
		const ThingTemplate *thing = TheThingFactory->findTemplate( data->m_riders[ i ].m_templateName );
		if( thing && thing->isEquivalentTo( rider->getTemplate() ) )
		{

			//This is our rider, so set the correct model condition.
			obj->setModelConditionState( data->m_riders[ i ].m_modelConditionFlagType );

			//Also set the correct weaponset flag
			obj->setWeaponSetFlag( data->m_riders[ i ].m_weaponSetFlag );

			//Also set the object status
			obj->setStatus( MAKE_OBJECT_STATUS_MASK( data->m_riders[ i ].m_objectStatusType ) );

			//Set the new commandset override
			obj->setCommandSetStringOverride( data->m_riders[ i ].m_commandSet );
			TheControlBar->markUIDirty();	// Refresh the UI in case we are selected

			//Change the locomotor.
			AIUpdateInterface* ai = obj->getAI();
			if( ai )
			{
				ai->chooseLocomotorSet( data->m_riders[ i ].m_locomotorSetType );
			}

			if( obj->getStatusBits().test( OBJECT_STATUS_STEALTHED ) )
			{
				StealthUpdate* stealth = obj->getStealth();
				if( stealth )
				{
					stealth->markAsDetected();
				}
			}

			//Transfer experience from the rider to the bike.
			ExperienceTracker *riderTracker = rider->getExperienceTracker();
			ExperienceTracker *bikeTracker = obj->getExperienceTracker();
#if !RETAIL_COMPATIBLE_CRC
			// TheSuperHackers @bugfix Stubbjax 15/12/2025 Copy trainable flag from the rider to prevent
			// Workers and other untrainable riders from ranking up via the bike's experience tracker.
			bikeTracker->setTrainable(riderTracker->isTrainable());
#endif
			bikeTracker->setVeterancyLevel( riderTracker->getVeterancyLevel(), FALSE );
			riderTracker->setExperienceAndLevel( 0, FALSE );

			break;
		}
	}

	//Extend base class
	TransportContain::onContaining( rider, wasSelected );

	m_containing = FALSE;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void RiderChangeContain::onRemoving( Object *rider )
{
	Object *bike = getObject();
	//Note if the bike dies, the rider dies too.
	if( bike->isEffectivelyDead() )
	{
		TheGameLogic->destroyObject( rider );
		return;
	}

	if( m_payloadCreated )
	{
		//Extend base class
		TransportContain::onRemoving( rider );
	}

	//Find the rider in the list and clear various data.
	const RiderChangeContainModuleData *data = getRiderChangeContainModuleData();
	for( int i = 0; i < MAX_RIDERS; i++ )
	{
		const ThingTemplate *thing = TheThingFactory->findTemplate( data->m_riders[ i ].m_templateName );
		if( thing && thing->isEquivalentTo( rider->getTemplate() ) )
		{
			//This is our rider, so clear the current model condition.
			bike->clearModelConditionFlags( MAKE_MODELCONDITION_MASK2( data->m_riders[ i ].m_modelConditionFlagType, MODELCONDITION_DOOR_1_CLOSING ) );

			//Also clear the current weaponset flag
			bike->clearWeaponSetFlag( data->m_riders[ i ].m_weaponSetFlag );

			//Also clear the object status
			bike->clearStatus( MAKE_OBJECT_STATUS_MASK( data->m_riders[ i ].m_objectStatusType ) );

			if( rider->getControllingPlayer() != NULL )
			{
				//Wow, completely unforseeable game teardown order crash.  SetVeterancyLevel results in a call to player
				//about upgrade masks.  So if we have a null player, it is game teardown, so don't worry about transfering exp.

				//Transfer experience from the bike to the rider.
				ExperienceTracker *riderTracker = rider->getExperienceTracker();
				ExperienceTracker *bikeTracker = bike->getExperienceTracker();
				bikeTracker->resetTrainable();
				riderTracker->setVeterancyLevel( bikeTracker->getVeterancyLevel(), FALSE );
				bikeTracker->setExperienceAndLevel( 0, FALSE );
			}

			break;
		}
	}

	//If we're not replacing the rider, then if the cycle is selected, transfer selection
	//to the rider getting off (because the bike is gonna blow).
	if( !m_containing )
	{
		Drawable *containDraw = bike->getDrawable();
		Drawable *riderDraw = rider->getDrawable();
		if( containDraw && riderDraw )
		{
			//Create the selection message for the rider if it's ours and SELECTED!
			if( bike->getControllingPlayer() == ThePlayerList->getLocalPlayer() && containDraw->isSelected() )
			{
				GameMessage *teamMsg = TheMessageStream->appendMessage( GameMessage::MSG_CREATE_SELECTED_GROUP );
				teamMsg->appendBooleanArgument( FALSE );// not creating new team so pass false
				teamMsg->appendObjectIDArgument( rider->getID() );
				TheInGameUI->selectDrawable( riderDraw );
				TheInGameUI->setDisplayedMaxWarning( FALSE );

				//Create the de-selection message for the container
				teamMsg = TheMessageStream->appendMessage( GameMessage::MSG_REMOVE_FROM_SELECTED_GROUP );
				teamMsg->appendObjectIDArgument( bike->getID() );
				TheInGameUI->deselectDrawable( containDraw );
			}

			//Finally, scuttle the bike so nobody else can use it! <Design Spec>
			m_scuttledOnFrame = TheGameLogic->getFrame();
			bike->setStatus( MAKE_OBJECT_STATUS_MASK( OBJECT_STATUS_UNSELECTABLE ) );
			bike->setModelConditionState( data->m_scuttleState );
			if( !bike->getAI()->isMoving() )
			{
				bike->setStatus( MAKE_OBJECT_STATUS_MASK( OBJECT_STATUS_IMMOBILE ) );
			}
		}
	}
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void RiderChangeContain::createPayload()
{
	// extend base class
	TransportContain::createPayload();
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
UpdateSleepTime RiderChangeContain::update()
{
	if( m_scuttledOnFrame != 0 )
	{
		//Bike in the process of getting scuttled.
		const RiderChangeContainModuleData *data = getRiderChangeContainModuleData();
		UnsignedInt now = TheGameLogic->getFrame();
		if( m_scuttledOnFrame + data->m_scuttleFrames <= now )
		{
			//We have scuttled the bike (at least as far as tipping it over via scuttle animation. Now
			//kill the bike in a way that will cause it to sink into the ground without any real destruction.
			getObject()->kill( DAMAGE_UNRESISTABLE, DEATH_TOPPLED ); //Sneaky, eh? Toppled heheh.
		}
	}
	// extend base class
	return TransportContain::update(); //extend
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void RiderChangeContain::unreserveDoorForExit( ExitDoorType exitDoor )
{
	/* nothing */
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void RiderChangeContain::killRidersWhoAreNotFreeToExit()
{
	// extend base class
	TransportContain::killRidersWhoAreNotFreeToExit();
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
Bool RiderChangeContain::isSpecificRiderFreeToExit(Object* specificObject)
{
	// extend base class
	return TransportContain::isSpecificRiderFreeToExit( specificObject );
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
ExitDoorType RiderChangeContain::reserveDoorForExit( const ThingTemplate* objType, Object *specificObject )
{
	// extend base class
	return TransportContain::reserveDoorForExit( objType, specificObject );
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
Bool RiderChangeContain::isExitBusy() const	///< Contain style exiters are getting the ability to space out exits, so ask this before reserveDoor as a kind of no-commitment check.
{
	// extend base class
	return FALSE; //return TransportContain::isExitBusy();
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void RiderChangeContain::onCapture( Player *oldOwner, Player *newOwner )
{
	// extend base class
	TransportContain::onCapture( oldOwner, newOwner );
}

//-------------------------------------------------------------------------------------------------
Bool RiderChangeContain::getContainerPipsToShow(Int& numTotal, Int& numFull)
{
	//Don't show any pips for motorcycles as they always have one rider unless dead!
	numTotal = 0;
	numFull = 0;
	return false;
}

//-------------------------------------------------------------------------------------------------
const Object *RiderChangeContain::friend_getRider() const
{
 	if( m_containListSize > 0 ) // Yes, this does assume that infantry never ride double on the bike
 		return m_containList.front();

	return NULL;
}




// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void RiderChangeContain::crc( Xfer *xfer )
{

	// extend base class
	TransportContain::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void RiderChangeContain::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	TransportContain::xfer( xfer );

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
void RiderChangeContain::loadPostProcess( void )
{

	// extend base class
	TransportContain::loadPostProcess();

}
