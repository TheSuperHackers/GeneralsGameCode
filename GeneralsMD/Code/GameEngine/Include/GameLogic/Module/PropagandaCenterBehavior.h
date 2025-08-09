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

// FILE: PropagandaCenterBehavior.h ///////////////////////////////////////////////////////////////
// Author: Colin Day, August 2002
// Desc:   Propaganda Center Behavior
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __PROPAGANDA_CENTER_BEHAVIOR_H_
#define __PROPAGANDA_CENTER_BEHAVIOR_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/PrisonBehavior.h"

#ifdef ALLOW_SURRENDER

// ------------------------------------------------------------------------------------------------
typedef std::list< ObjectID > BrainwashedIDList;
typedef BrainwashedIDList::const_iterator BrainwashedIDListContIterator;
typedef BrainwashedIDList::iterator BrainwashedIDListIterator;

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class PropagandaCenterBehaviorModuleData : public PrisonBehaviorModuleData
{

public:

	PropagandaCenterBehaviorModuleData( void );

	static void buildFieldParse( MultiIniFieldParse &p );

	UnsignedInt m_brainwashDuration;			///< how long (in frames) it takes to become brainwashed

};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class PropagandaCenterBehavior : public PrisonBehavior
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( PropagandaCenterBehavior, "PropagandaCenterBehavior" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( PropagandaCenterBehavior, PropagandaCenterBehaviorModuleData )

public:

	PropagandaCenterBehavior( Thing *thing, const ModuleData *moduleData );
	// virtual destructor prototype provided by memory pool object

	// generic module methods
	virtual void onDelete( void );

	// contain methods
	virtual UpdateSleepTime update( void );
	virtual void onRemoving( Object *obj );

protected:

	ObjectID m_brainwashingSubjectID;								///< who we're currently brainwashing
	UnsignedInt m_brainwashingSubjectStartFrame;		///< frame we started brainwashing
	BrainwashedIDList m_brainwashedList;						///< list of objects we've brainwashed

};

#endif

#endif  // end __PROPAGANDA_CENTER_BEHAVIOR_H_
