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

// TransportAIUpdate.cpp //////////
// Needs to check legality of evacuate, and may move to a place that is better to evacuate at
// Author: Graham Smallwood, July 2002

#pragma once

#include "GameLogic/Module/AIUpdate.h"

//-------------------------------------------------------------------------------------------------
/**
 * Soldier behavior implementation.
 * Override or extend AIUpdate methods to customize the Soldier's behavior.
 */
class TransportAIUpdate : public AIUpdateInterface
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( TransportAIUpdate, "TransportAIUpdate" )
	MAKE_STANDARD_MODULE_MACRO( TransportAIUpdate )

	/*
		IMPORTANT NOTE: if you ever add module data to this, you must have it inherit from
		AIUpdateModuleData to allow locomotors to work correctly. (see SupplyTruckAIUpdate
		for an example.)
	*/

public:

	TransportAIUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration
	virtual AIFreeToExitType getAiFreeToExit(const Object* exiter) const;

protected:

	virtual AIStateMachine* makeStateMachine();

protected:
	virtual void privateAttackObject( Object *victim, Int maxShotsToFire, CommandSourceType cmdSource );///< Extension.  Also tell occupants to attackObject
	virtual void privateAttackPosition( const Coord3D *pos, Int maxShotsToFire, CommandSourceType cmdSource );///< Extension.  Also tell occupants to attackPosition
	virtual void privateForceAttackObject( Object *victim, Int maxShotsToFire, CommandSourceType cmdSource );///< Extension.  Also tell occupants to forceAttackObject
};
