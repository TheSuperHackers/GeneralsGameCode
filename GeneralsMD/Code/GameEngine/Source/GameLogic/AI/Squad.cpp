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

// FILE: Squad.cpp
/*---------------------------------------------------------------------------*/
/* EA Pacific                                                                */
/* Confidential Information	                                                 */
/* Copyright (C) 2001 - All Rights Reserved                                  */
/* DO NOT DISTRIBUTE                                                         */
/*---------------------------------------------------------------------------*/
/* Project:    RTS3                                                          */
/* File name:  Squad.cpp                                                      */
/* Created:    John K. McDonald, Jr., 4/19/2002                               */
/* Desc:       // @todo                                                      */
/* Revision History:                                                         */
/*		4/19/2002 : Initial creation                                          */
/*---------------------------------------------------------------------------*/
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "GameLogic/Squad.h"

#include "Common/GameState.h"
#include "Common/Team.h"
#include "Common/Xfer.h"

#include "GameLogic/AI.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"


// addObject //////////////////////////////////////////////////////////////////////////////////////
void Squad::addObject(Object *objectToAdd)
{
	if (objectToAdd) {
		m_objectIDs.push_back(objectToAdd->getID());
	}
}

// addObjectID ////////////////////////////////////////////////////////////////////////////////////
void Squad::addObjectID(ObjectID objectID) {
	m_objectIDs.push_back(objectID);
}

// removeObject ///////////////////////////////////////////////////////////////////////////////////
void Squad::removeObject(Object *objectToRemove)
{
	if (objectToRemove) {
		ObjectID objID;
		objID = objectToRemove->getID();
		VecObjectIDIt it = std::find(m_objectIDs.begin(), m_objectIDs.end(), objID);
		if (it != m_objectIDs.end()) {
			m_objectIDs.erase(it);
		}
	}
}

// clearSquad /////////////////////////////////////////////////////////////////////////////////////
void Squad::clearSquad() {
	m_objectIDs.clear();
	m_objectsCached.clear();
}

// getAllObjects //////////////////////////////////////////////////////////////////////////////////
const VecObjectPtr& Squad::getAllObjects(void) // Not a const function cause we clear away dead object here too
{
	// prunes all NULL objects
	m_objectsCached.clear();
	for (VecObjectIDIt it = m_objectIDs.begin(); it != m_objectIDs.end(); ) {
		Object *obj = TheGameLogic->findObjectByID(*it);
		if (obj) {
			m_objectsCached.push_back(obj);
			++it;
		} else {
			it = m_objectIDs.erase(it);
		}
	}

	return m_objectsCached;
}

// getLiveObjects /////////////////////////////////////////////////////////////////////////////////
const VecObjectPtr& Squad::getLiveObjects(void)
{
	// first get all the objects.
	// cheat, since we are a member function, and just use m_objectsCached
	getAllObjects();
	for (VecObjectPtrIt it = m_objectsCached.begin(); it != m_objectsCached.end(); ) {
		if (!(*it)->isSelectable()) {
			it = m_objectsCached.erase(it);
		} else {
			++it;
		}
	}

	return m_objectsCached;
}

// getSizeOfGroup /////////////////////////////////////////////////////////////////////////////////
Int Squad::getSizeOfGroup(void) const
{
	return m_objectIDs.size();
}

// isOnSquad //////////////////////////////////////////////////////////////////////////////////////
Bool Squad::isOnSquad(const Object *objToTest) const
{
	// @todo need a faster way to do this. Perhaps a more efficient data structure?
	ObjectID objID = objToTest->getID();
	for (VecObjectID::const_iterator cit = m_objectIDs.begin(); cit != m_objectIDs.end(); ++cit) {
		if (objID == (*cit)) {
			return true;
		}
	}

	return false;
}

/**
 * There should never be a TeamFromSqaud as Teams are entirely a construct to work with the AI.
 * Since things can only be on one Team at a time, creating a Team from an arbitrary Squad will
 * cause weird, difficult to reproduce bugs. Please don't do it.
 */

// squadFromTeam //////////////////////////////////////////////////////////////////////////////////
void Squad::squadFromTeam(const Team* fromTeam, Bool clearSquadFirst)
{
	if (!fromTeam) {
		return;
	}

	if (clearSquadFirst) {
		m_objectIDs.clear();
	}

	for (DLINK_ITERATOR<Object> iter = fromTeam->iterate_TeamMemberList(); !iter.done(); iter.advance()) {
		Object *obj = iter.cur();
		m_objectIDs.push_back(obj->getID());
	}
}

// squadFromAIGroup ///////////////////////////////////////////////////////////////////////////////
void Squad::squadFromAIGroup(const AIGroup* fromAIGroup, Bool clearSquadFirst)
{
	if (!fromAIGroup) {
		return;
	}

	if (clearSquadFirst) {
		m_objectIDs.clear();
	}

	m_objectIDs = fromAIGroup->getAllIDs();
}

// aiGroupFromSquad ///////////////////////////////////////////////////////////////////////////////
void Squad::aiGroupFromSquad(AIGroup* aiGroupToFill)
{
	if (!aiGroupToFill) {
		return;
	}

	// cheat, since we are a member function, and just use m_objectsCached
	getLiveObjects();
	for (VecObjectPtr::iterator it = m_objectsCached.begin(); it != m_objectsCached.end(); ++it) {
		aiGroupToFill->add((*it));
	}
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void Squad::crc( Xfer *xfer )
{

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void Squad::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// length of object ID list
	UnsignedShort objectCount = m_objectIDs.size();
	xfer->xferUnsignedShort( &objectCount );

	// object id elements
	ObjectID objectID;
	if( xfer->getXferMode() == XFER_SAVE )
	{

		// save each object id
		VecObjectIDIt it;
		for( it = m_objectIDs.begin(); it != m_objectIDs.end(); ++it )
		{

			// save object ID
			objectID = *it;
			xfer->xferObjectID( &objectID );

		}  // end for, it

	}  // end if, save
	else
	{

		// the cached objects list should be empty
		if( m_objectsCached.size() != 0 )
		{

			DEBUG_CRASH(( "Squad::xfer - m_objectsCached should be emtpy, but is not" ));
			throw SC_INVALID_DATA;

		}  // end of

		// read all items
		for( UnsignedShort i = 0; i < objectCount; ++i )
		{

			// read id
			xfer->xferObjectID( &objectID );

			// put on list
			m_objectIDs.push_back( objectID );

		}  // end for, i

	}  // end else, load

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void Squad::loadPostProcess( void )
{

}  // end loadPostProcess
