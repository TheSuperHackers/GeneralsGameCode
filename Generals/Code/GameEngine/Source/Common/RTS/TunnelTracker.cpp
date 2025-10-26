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

// FILE: TunnelTracker.cpp ///////////////////////////////////////////////////////////
// The part of a Player's brain that holds the communal Passenger list of all tunnels.
// Author: Graham Smallwood, March, 2002

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/GameState.h"
#include "Common/GlobalData.h"
#include "Common/KindOf.h"
#include "Common/TunnelTracker.h"
#include "Common/Xfer.h"

#include "GameClient/ControlBar.h"
#include "GameClient/Drawable.h"

#include "GameLogic/AI.h"
#include "GameLogic/AIPathfind.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"

#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/Module/TunnelContain.h"


// ------------------------------------------------------------------------
TunnelTracker::TunnelTracker()
{
	m_tunnelCount = 0;
	m_containListSize = 0;
	m_curNemesisID = INVALID_ID;
	m_nemesisTimestamp = 0;
	m_framesForFullHeal = 0;
	m_needsFullHealTimeUpdate = false;
}

// ------------------------------------------------------------------------
TunnelTracker::~TunnelTracker()
{
	m_tunnelIDs.clear();
}

// ------------------------------------------------------------------------
void TunnelTracker::iterateContained( ContainIterateFunc func, void *userData, Bool reverse )
{
	if (reverse)
	{
		// note that this has to be smart enough to handle items in the list being deleted
		// via the callback function.
		for(ContainedItemsList::reverse_iterator it = m_containList.rbegin(); it != m_containList.rend(); )
		{
			// save the obj...
			Object* obj = *it;

			// incr the iterator BEFORE calling the func (if the func removes the obj,
			// the iterator becomes invalid)
			++it;

			// call it
			(*func)( obj, userData );
		}
	}
	else
	{
		// note that this has to be smart enough to handle items in the list being deleted
		// via the callback function.
		for(ContainedItemsList::iterator it = m_containList.begin(); it != m_containList.end(); )
		{
			// save the obj...
			Object* obj = *it;

			// incr the iterator BEFORE calling the func (if the func removes the obj,
			// the iterator becomes invalid)
			++it;

			// call it
			(*func)( obj, userData );
		}
	}
}

// ------------------------------------------------------------------------
Int TunnelTracker::getContainMax() const
{
	return TheGlobalData->m_maxTunnelCapacity;
}

// ------------------------------------------------------------------------
void TunnelTracker::swapContainedItemsList(ContainedItemsList& newList)
{
	m_containList.swap(newList);
	m_containListSize = (Int)m_containList.size();
}

// ------------------------------------------------------------------------
void TunnelTracker::updateNemesis(const Object *target)
{
	if (getCurNemesis()==NULL) {
		if (target) {
			if (target->isKindOf(KINDOF_VEHICLE) || target->isKindOf(KINDOF_STRUCTURE) ||
				target->isKindOf(KINDOF_INFANTRY) || target->isKindOf(KINDOF_AIRCRAFT)) {
					m_curNemesisID = target->getID();
					m_nemesisTimestamp = TheGameLogic->getFrame();
			}
		}
	} else if (getCurNemesis()==target) {
		m_nemesisTimestamp = TheGameLogic->getFrame();
	}
}

// ------------------------------------------------------------------------
Object *TunnelTracker::getCurNemesis(void)
{
	if (m_curNemesisID == INVALID_ID) {
		return NULL;
	}
	if (m_nemesisTimestamp + 4*LOGICFRAMES_PER_SECOND < TheGameLogic->getFrame()) {
		m_curNemesisID = INVALID_ID;
		return NULL;
	}
	Object *target = TheGameLogic->findObjectByID(m_curNemesisID);
	if (target) {
		//If the enemy unit is stealthed and not detected, then we can't attack it!
	if( target->testStatus( OBJECT_STATUS_STEALTHED ) &&
			!target->testStatus( OBJECT_STATUS_DETECTED ) )
		{
			target = NULL;
		}
	}
	if (target && target->isEffectivelyDead()) {
		target = NULL;
	}
	if (target == NULL) {
		m_curNemesisID = INVALID_ID;
	}
	return target;
}

// ------------------------------------------------------------------------
Bool TunnelTracker::isValidContainerFor(const Object* obj, Bool checkCapacity) const
{
	//October 11, 2002 -- Kris : Dustin wants ALL units to be able to use tunnels!
	// srj sez: um, except aircraft.
	if (obj && !obj->isKindOf(KINDOF_AIRCRAFT))
	{
		if (checkCapacity)
		{
			Int containMax = getContainMax();
			Int containCount = getContainCount();
			return ( containCount < containMax );
		}
		else
		{
			return true;
		}
	}
	return false;
}

// ------------------------------------------------------------------------
void TunnelTracker::addToContainList( Object *obj )
{
	m_containList.push_back(obj);
	++m_containListSize;
}

// ------------------------------------------------------------------------
void TunnelTracker::removeFromContain( Object *obj, Bool exposeStealthUnits )
{

	ContainedItemsList::iterator it = std::find(m_containList.begin(), m_containList.end(), obj);
	if (it != m_containList.end())
	{
		// note that this invalidates the iterator!
		m_containList.erase(it);
		--m_containListSize;
	}

}

// ------------------------------------------------------------------------
Bool TunnelTracker::isInContainer( Object *obj )
{
	return (std::find(m_containList.begin(), m_containList.end(), obj) != m_containList.end()) ;
}

// ------------------------------------------------------------------------
void TunnelTracker::onTunnelCreated( const Object *newTunnel )
{
	m_tunnelCount++;
	m_tunnelIDs.push_back( newTunnel->getID() );
	m_needsFullHealTimeUpdate = true;
}

// ------------------------------------------------------------------------
void TunnelTracker::onTunnelDestroyed( const Object *deadTunnel )
{
	m_tunnelCount--;
	m_tunnelIDs.remove( deadTunnel->getID() );
	m_needsFullHealTimeUpdate = true;

	if( m_tunnelCount == 0 )
	{
		// Kill everyone in our contain list.  Cave in!
		iterateContained( destroyObject, NULL, FALSE );
		m_containList.clear();
		m_containListSize = 0;
	}
	else
	{
		Object *validTunnel = TheGameLogic->findObjectByID( m_tunnelIDs.front() );
		// Otherwise, make sure nobody inside remembers the dead tunnel as the one they entered
		// (scripts need to use so there must be something valid here)
		for(ContainedItemsList::iterator it = m_containList.begin(); it != m_containList.end(); )
		{
			Object* obj = *it;
			++it;
			if( obj->getContainedBy() == deadTunnel )
				obj->onContainedBy( validTunnel );
		}
	}
}

// ------------------------------------------------------------------------
void TunnelTracker::destroyObject( Object *obj, void * )
{
	// Now that tunnels consider ContainedBy to be "the tunnel you entered", I need to say goodbye
	// llike other contain types so they don't look us up on their deletion and crash
	obj->onRemovedFrom( obj->getContainedBy() );
	TheGameLogic->destroyObject( obj );
}

// ------------------------------------------------------------------------
	// heal all the objects within the tunnel system using the iterateContained function
#if RETAIL_COMPATIBLE_CRC
void TunnelTracker::healObjects(Real frames)
{
	iterateContained(healObject, &frames, FALSE);
}
#else
void TunnelTracker::healObjects()
{
	if (m_needsFullHealTimeUpdate)
	{
		updateFullHealTime();
		m_needsFullHealTimeUpdate = false;
	}

	iterateContained(healObject, &m_framesForFullHeal, FALSE);
}
#endif

// ------------------------------------------------------------------------
	// heal one object within the tunnel network system
void TunnelTracker::healObject( Object *obj, void *frames)
{

	//get the number of frames to heal
#if RETAIL_COMPATIBLE_CRC
	Real *framesForFullHeal = (Real*)frames;
#else
	UnsignedInt* framesForFullHeal = (UnsignedInt*)frames;
#endif

	// setup the healing damageInfo structure with all but the amount
	DamageInfo healInfo;
	healInfo.in.m_damageType = DAMAGE_HEALING;
	healInfo.in.m_deathType = DEATH_NONE;
	//healInfo.in.m_sourceID = getObject()->getID();

	// get body module of the thing to heal
	BodyModuleInterface *body = obj->getBodyModule();

	// if we've been in here long enough ... set our health to max
	if( TheGameLogic->getFrame() - obj->getContainedByFrame() >= *framesForFullHeal )
	{

		// set the amount to max just to be sure we're at the top
		healInfo.in.m_amount = body->getMaxHealth();

		// set max health
		body->attemptHealing( &healInfo );

	}
	else
	{
		//
		// given the *whole* time it would take to heal this object, lets pretend that the
		// object is at zero health ... and give it a sliver of health as if it were at 0 health
		// and would be fully healed at 'framesForFullHeal'
		//
		healInfo.in.m_amount = body->getMaxHealth() / *framesForFullHeal;

		// do the healing
		body->attemptHealing( &healInfo );

	}
}

void TunnelTracker::updateFullHealTime()
{
	UnsignedInt minFrames = ~0u;

	for (std::list<ObjectID>::const_iterator it = m_tunnelIDs.begin(); it != m_tunnelIDs.end(); ++it)
	{
		const Object* tunnelObj = TheGameLogic->findObjectByID(*it);
		if (!tunnelObj)
			continue;

		const ContainModuleInterface* contain = tunnelObj->getContain();
		DEBUG_ASSERTCRASH(contain != NULL, ("Contain module is NULL"));

		if (!contain->isTunnelContain())
			continue;

		const TunnelContain* tunnelContain = static_cast<const TunnelContain*>(contain);
		const UnsignedInt framesForFullHeal = tunnelContain->getFullTimeForHeal();
		if (framesForFullHeal < minFrames)
			minFrames = framesForFullHeal;
	}

	m_framesForFullHeal = minFrames;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void TunnelTracker::crc( Xfer *xfer )
{

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void TunnelTracker::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// tunnel object id list
	xfer->xferSTLObjectIDList( &m_tunnelIDs );

	// contain list count
	xfer->xferInt( &m_containListSize );

	// contain list data
	ObjectID objectID;
	if( xfer->getXferMode() == XFER_SAVE )
	{
		ContainedItemsList::const_iterator it;

		for( it = m_containList.begin(); it != m_containList.end(); ++it )
		{

			objectID = (*it)->getID();
			xfer->xferObjectID( &objectID );

		}

	}
	else
	{

		for( UnsignedShort i = 0; i < m_containListSize; ++i )
		{

			xfer->xferObjectID( &objectID );
			m_xferContainList.push_back( objectID );

		}

		m_needsFullHealTimeUpdate = true;
	}

	// tunnel count
	xfer->xferUnsignedInt( &m_tunnelCount );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void TunnelTracker::loadPostProcess( void )
{

	// sanity, the contain list should be empty until we post process the id list
	if( m_containList.size() != 0 )
	{

		DEBUG_CRASH(( "TunnelTracker::loadPostProcess - m_containList should be empty but is not" ));
		throw SC_INVALID_DATA;

	}

	// translate each object ids on the xferContainList into real object pointers in the contain list
	Object *obj;
	std::list< ObjectID >::const_iterator it;
	for( it = m_xferContainList.begin(); it != m_xferContainList.end(); ++it )
	{

		obj = TheGameLogic->findObjectByID( *it );
		if( obj == NULL )
		{

			DEBUG_CRASH(( "TunnelTracker::loadPostProcess - Unable to find object ID '%d'", *it ));
			throw SC_INVALID_DATA;

		}

		// push on the back of the contain list
		m_containList.push_back( obj );

		// Crap.  This is in OpenContain as a fix, but not here.
		{
			// remove object from its group (if any)
			obj->leaveGroup();

			// remove rider from partition manager
			ThePartitionManager->unRegisterObject( obj );

			// hide the drawable associated with rider
			if( obj->getDrawable() )
				obj->getDrawable()->setDrawableHidden( true );

			// remove object from pathfind map
			if( TheAI )
				TheAI->pathfinder()->removeObjectFromPathfindMap( obj );

		}
	}

	// we're done with the xfer contain list now
	m_xferContainList.clear();

}
