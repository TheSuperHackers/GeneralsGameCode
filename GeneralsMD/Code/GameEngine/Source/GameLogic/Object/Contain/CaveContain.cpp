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

// FILE: CaveContain.cpp ////////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, July 2002
// Desc:   A version of OpenContain that overrides where the passengers are stored: one of CaveSystem's
//					entries. Changing entry is a script or ini command.  All queries about capacity and 
//					contents are also redirected.  They change sides like Garrison too.
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#include "Common/GameState.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/Team.h"
#include "Common/ThingTemplate.h"
#include "Common/TunnelTracker.h"
#include "Common/Xfer.h"
#include "GameClient/Drawable.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/CaveContain.h"
#include "GameLogic/CaveSystem.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
CaveContain::CaveContain( Thing *thing, const ModuleData* moduleData ) : OpenContain( thing, moduleData )
{
	m_needToRunOnBuildComplete = true;
	m_caveIndex = 0;
	m_originalTeam = NULL;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
CaveContain::~CaveContain()
{
}

void CaveContain::addToContainList( Object *obj )
{
	TunnelTracker *myTracker = TheCaveSystem->getTunnelTrackerForCaveIndex( m_caveIndex );
	myTracker->addToContainList( obj );
}

//-------------------------------------------------------------------------------------------------
/** Remove 'obj' from the m_containList of objects in this module.
	* This will trigger an onRemoving event for the object that this module
	* is a part of and an onRemovedFrom event for the object being removed */
//-------------------------------------------------------------------------------------------------
void CaveContain::removeFromContain( Object *obj, Bool exposeStealthUnits )
{

	// sanity
	if( obj == NULL )
		return;

	//
	// we can only remove this object from the contains list of this module if
	// it is actually contained by this module
	//
	TunnelTracker *myTracker = TheCaveSystem->getTunnelTrackerForCaveIndex( m_caveIndex );

	if( ! myTracker->isInContainer( obj ) )
	{
		return;
	}

	// This must come before the onRemov*, because CaveContain's version has a edge-0 triggered event.
	// If that were to go first, the number would still be 1 at that time.  Noone else cares about 
	// order.
	myTracker->removeFromContain( obj, exposeStealthUnits );

	// trigger an onRemoving event for 'm_object' no longer containing 'itemToRemove->m_object'
	if (getObject()->getContain())
		getObject()->getContain()->onRemoving( obj );
		
	// trigger an onRemovedFrom event for 'remove'
	obj->onRemovedFrom( getObject() );

}

//-------------------------------------------------------------------------------------------------
/** Remove all contained objects from the contained list */
//-------------------------------------------------------------------------------------------------
void CaveContain::removeAllContained( Bool exposeStealthUnits )
{
	TunnelTracker *myTracker = TheCaveSystem->getTunnelTrackerForCaveIndex( m_caveIndex );
	const ContainedItemsList *fullList = myTracker->getContainedItemsList();

	Object *obj;
	ContainedItemsList::const_iterator it;
	it = (*fullList).begin();
	while( it != (*fullList).end() )
	{
		obj = *it;
		it++;
		removeFromContain( obj, exposeStealthUnits );
	}
}

//-------------------------------------------------------------------------------------------------
/** Iterate the contained list and call the callback on each of the objects */
//-------------------------------------------------------------------------------------------------
void CaveContain::iterateContained( ContainIterateFunc func, void *userData, Bool reverse )
{
	TunnelTracker *myTracker = TheCaveSystem->getTunnelTrackerForCaveIndex( m_caveIndex );
	myTracker->iterateContained( func, userData, reverse );
}

//-------------------------------------------------------------------------------------------------
void CaveContain::onContaining( Object *obj, Bool wasSelected )
{
	OpenContain::onContaining( obj, wasSelected );
	// objects inside a building are held
	obj->setDisabled( DISABLED_HELD );

	//
	// the team of the building is now the same as those that have garrisoned it, be sure
	// to save our original team tho so that we can revert back to it when all the 
	// occupants are gone
	//
	recalcApparentControllingPlayer();

}

//-------------------------------------------------------------------------------------------------
void CaveContain::onRemoving( Object *obj ) 
{
	OpenContain::onRemoving(obj);
	// object is no longer held inside a garrisoned building
	obj->clearDisabled( DISABLED_HELD );

	/// place the object in the world at position of the container m_object
	ThePartitionManager->registerObject( obj );
	obj->setPosition( getObject()->getPosition() );
	if( obj->getDrawable() )
	{
		obj->getDrawable()->setDrawableHidden( false );
	}

	doUnloadSound();

	if( getContainCount() == 0 )
	{

		// put us back on our original team
		// (hokey exception: if our team is null, don't bother -- this
		// usually means we are being called during game-teardown and
		// the teams are no longer valid...)
		if (getObject()->getTeam() != NULL)
		{
			changeTeamOnAllConnectedCaves( m_originalTeam, FALSE );
			m_originalTeam = NULL;
		}

		// change the state back from garrisoned
		Drawable *draw = getObject()->getDrawable();
		if( draw )
		{
			draw->clearModelConditionState( MODELCONDITION_GARRISONED );
		}

	}  // end if
}

Bool CaveContain::isValidContainerFor(const Object* obj, Bool checkCapacity) const
{
	TunnelTracker *myTracker = TheCaveSystem->getTunnelTrackerForCaveIndex( m_caveIndex );
	return myTracker->isValidContainerFor( obj, checkCapacity );
}

UnsignedInt CaveContain::getContainCount() const
{
	TunnelTracker *myTracker = TheCaveSystem->getTunnelTrackerForCaveIndex( m_caveIndex );
	return myTracker->getContainCount();
}

Int CaveContain::getContainMax( void ) const 
{ 
	TunnelTracker *myTracker = TheCaveSystem->getTunnelTrackerForCaveIndex( m_caveIndex );
	return myTracker->getContainMax();
}

const ContainedItemsList* CaveContain::getContainedItemsList() const
{
	TunnelTracker *myTracker = TheCaveSystem->getTunnelTrackerForCaveIndex( m_caveIndex );
	return myTracker->getContainedItemsList();
}

//-------------------------------------------------------------------------------------------------
/** The die callback. */
//-------------------------------------------------------------------------------------------------
void CaveContain::onDie( const DamageInfo * damageInfo )
{
	// override the onDie we inherit from OpenContain. no super call.
	if (!getCaveContainModuleData()->m_dieMuxData.isDieApplicable(getObject(), damageInfo))
		return;

	if( getObject()->getStatusBits().test( OBJECT_STATUS_UNDER_CONSTRUCTION ) )
		return;//it never registered itself as a tunnel

	TunnelTracker *myTracker = TheCaveSystem->getTunnelTrackerForCaveIndex( m_caveIndex );

	TheCaveSystem->unregisterCave( m_caveIndex );

	myTracker->onTunnelDestroyed( getObject() );
}  


//-------------------------------------------------------------------------------------------------
void CaveContain::onCreate( void )
{
	m_caveIndex = getCaveContainModuleData()->m_caveIndexData;
}

//-------------------------------------------------------------------------------------------------
void CaveContain::onBuildComplete( void )
{
	if( ! shouldDoOnBuildComplete() )
		return;

	m_needToRunOnBuildComplete = false;

	TheCaveSystem->registerNewCave( m_caveIndex );

	TunnelTracker *myTracker = TheCaveSystem->getTunnelTrackerForCaveIndex( m_caveIndex );

	myTracker->onTunnelCreated( getObject() );
} 

//-------------------------------------------------------------------------------------------------
void CaveContain::tryToSetCaveIndex( Int newIndex )
{
	if( TheCaveSystem->canSwitchIndexToIndex( m_caveIndex, newIndex ) )
	{
		TunnelTracker *myOldTracker = TheCaveSystem->getTunnelTrackerForCaveIndex( m_caveIndex );
		TheCaveSystem->unregisterCave( m_caveIndex );
		myOldTracker->onTunnelDestroyed( getObject() );

		m_caveIndex = newIndex;

		TheCaveSystem->registerNewCave( m_caveIndex );
		TunnelTracker *myNewTracker = TheCaveSystem->getTunnelTrackerForCaveIndex( m_caveIndex );
		myNewTracker->onTunnelCreated( getObject() );
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void CaveContain::recalcApparentControllingPlayer( void )
{
	//Record original team first time through.
	if( m_originalTeam == NULL )
	{
		m_originalTeam = getObject()->getTeam();
	}

	// (hokey trick: if our team is null, nuke originalTeam -- this
	// usually means we are being called during game-teardown and
	// the teams are no longer valid...)
	if (getObject()->getTeam() == NULL)
		m_originalTeam = NULL;

	// This is called from onContaining, so a one is the edge trigger to do capture stuff
	if( getContainCount() == 1 )
	{
		ContainedItemsList::const_iterator it = getContainedItemsList()->begin();
		Object *rider = *it;

		// This also gets called during reset from the PlayerList, so we might not actually have players.
		// Check like the hokey trick mentioned above
		if( rider->getControllingPlayer() )
			changeTeamOnAllConnectedCaves( rider->getControllingPlayer()->getDefaultTeam(), TRUE );

	}
	else if( getContainCount() == 0 )
	{
		// And a 0 is the edge trigger to do uncapture stuff
		changeTeamOnAllConnectedCaves( m_originalTeam, FALSE );
	}

	// Handle the team color that is rendered
	const Player* controller = getApparentControllingPlayer(ThePlayerList->getLocalPlayer());
	if (controller)
	{
		if (TheGlobalData->m_timeOfDay == TIME_OF_DAY_NIGHT)
			getObject()->getDrawable()->setIndicatorColor( controller->getPlayerNightColor() );
		else
			getObject()->getDrawable()->setIndicatorColor( controller->getPlayerColor() );
	}
}

/////////////////////////////////////////////////////////////////////////////////////
static CaveInterface* findCave(Object* obj)
{
	for (BehaviorModule** i = obj->getBehaviorModules(); *i; ++i)
	{
		CaveInterface* c = (*i)->getCaveInterface();
		if (c != NULL)
			return c;
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////
void CaveContain::changeTeamOnAllConnectedCaves( Team *newTeam, Bool setOriginalTeams )
{
	TunnelTracker *myTracker = TheCaveSystem->getTunnelTrackerForCaveIndex( m_caveIndex );
	const std::list<ObjectID> *allCaves = myTracker->getContainerList();
	for( std::list<ObjectID>::const_iterator iter = allCaves->begin(); iter != allCaves->end(); iter++ )
	{
		// For each ID, look it up and change its team.  We all get captured together.
		Object *currentCave = TheGameLogic->findObjectByID( *iter );
		if( currentCave )
		{
			// This is a distributed Garrison in terms of capturing, so when one node 
			// triggers the change, he needs to tell everyone, so anyone can do the un-change.
			CaveInterface *caveModule = findCave(currentCave);
			if( caveModule == NULL )
				continue;
			if( setOriginalTeams )
				caveModule->setOriginalTeam( currentCave->getTeam() );
			else 
				caveModule->setOriginalTeam( NULL );

			// Now do the actual switch for this one.
			
			currentCave->defect( newTeam, 0 );
//			currentCave->setTeam( newTeam );
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////
void CaveContain::setOriginalTeam( Team *oldTeam )
{
	m_originalTeam = oldTeam;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void CaveContain::crc( Xfer *xfer )
{
	
	// extend base class
	OpenContain::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void CaveContain::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	OpenContain::xfer( xfer );

	// need to run on build complete
	xfer->xferBool( &m_needToRunOnBuildComplete );

	// cave index
	xfer->xferInt( &m_caveIndex );

	// original team
	TeamID teamID = m_originalTeam ? m_originalTeam->getID() : TEAM_ID_INVALID;
	xfer->xferUser( &teamID, sizeof( TeamID ) );
	if( xfer->getXferMode() == XFER_LOAD )
	{

		if( teamID != TEAM_ID_INVALID )
		{

			m_originalTeam = TheTeamFactory->findTeamByID( teamID );
			if( m_originalTeam == NULL )
			{

				DEBUG_CRASH(( "CaveContain::xfer - Unable to find original team by id" ));
				throw SC_INVALID_DATA;

			}  // end if

		}  // end if
		else
			m_originalTeam = NULL;

	}  // end if

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void CaveContain::loadPostProcess( void )
{

	// extend base class
	OpenContain::loadPostProcess();

}  // end loadPostProcess
