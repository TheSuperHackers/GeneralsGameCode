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

// FILE: Sqaud.h
/*---------------------------------------------------------------------------*/
/* EA Pacific                                                                */
/* Confidential Information	                                                 */
/* Copyright (C) 2001 - All Rights Reserved                                  */
/* DO NOT DISTRIBUTE                                                         */
/*---------------------------------------------------------------------------*/
/* Project:    RTS3                                                          */
/* File name:  Sqaud.h                                                      */
/* Created:    John K. McDonald, Jr., 4/19/2002                               */
/* Desc:       // @todo                                                      */
/* Revision History:                                                         */
/*		4/19/2002 : Initial creation                                          */
/*---------------------------------------------------------------------------*/

#pragma once
#ifndef _H_SQAUD_
#define _H_SQAUD_

// INCLUDES ///////////////////////////////////////////////////////////////////
#include "Common/Snapshot.h"
#include "Common/GameMemory.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// FORWARD DECLARATIONS ///////////////////////////////////////////////////////
class AIGroup;
class Object;
class Team;


// TYPE DEFINES ///////////////////////////////////////////////////////////////
typedef std::vector<ObjectID> VecObjectID;
typedef VecObjectID::iterator VecObjectIDIt;

typedef std::vector<Object*> VecObjectPtr;
typedef VecObjectPtr::iterator VecObjectPtrIt;

class Squad : public MemoryPoolObject, public Snapshot
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(Squad, "Squad")

protected:
	// snapshot methods
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void );

	VecObjectID m_objectIDs;

	// this one is the last requested object stuff. Its used so that we can return a
	// const& rather than making a copy and placing it on the stack.
	VecObjectPtr m_objectsCached;

public:
	void addObject(Object *objectToAdd);							// add an object
	void addObjectID(ObjectID objectID);							// add an object ID
	void removeObject(Object *objectToRemove);				// remove an object
	void clearSquad();																// remove all objects from this squad.
	const VecObjectPtr& getAllObjects(void);					// get all objects on the list that haven't been deleted
	const VecObjectPtr& getLiveObjects(void);					// get all objects that pass "isEffectivelyDead" test
	Int getSizeOfGroup(void) const;										// get the current number of objects, including dead objects
	Bool isOnSquad(const Object *objToTest) const;		// returns true if the object is on this squad, otherwise false

	// convenience function to fill this squad with members of a team
	// There SHOULD NOT be a TeamFromSquad Function. See comments in Squad.cpp for details
	void squadFromTeam(const Team* fromTeam, Bool clearSquadFirst);

	// convenience function to create a squad from an AIGroup, and an AIGroup from a team.
	// When creating the AIGroup from the Squad, the old AIGroup affiliations are broken.
	void squadFromAIGroup(const AIGroup* fromAIGroup, Bool clearSquadFirst);
	void aiGroupFromSquad(AIGroup* aiGroupToFill);
};
EMPTY_DTOR(Squad)

#endif /* _H_SQAUD_ */
