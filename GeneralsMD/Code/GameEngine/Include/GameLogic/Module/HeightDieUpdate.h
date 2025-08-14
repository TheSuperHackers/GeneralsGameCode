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

// FILE: HeightDieUpdate.h ////////////////////////////////////////////////////////////////////////
// Author: Objects that will die when the are a certain height above the terrain or objects
// Desc:   Colin Day, April 2002
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __HEIGHTDIEUPDATE_H_
#define __HEIGHTDIEUPDATE_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/UpdateModule.h"

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class HeightDieUpdateModuleData: public UpdateModuleData
{

public:

	HeightDieUpdateModuleData( void );

	static void buildFieldParse(MultiIniFieldParse& p);

	Real m_targetHeightAboveTerrain;				///< die at this height above terrain
	Bool m_targetHeightIncludesStructures;	///< target height considers terrain AND structure height underneath us
	Bool m_onlyWhenMovingDown;							///< don't detonate unless moving in downward z dir
	Real m_destroyAttachedParticlesAtHeight;  ///< HACK, destroy any attached particle system of object when below this height
	Bool m_snapToGroundOnDeath;							///< snap to the ground when killed
	UnsignedInt m_initialDelay;							///< Don't explode before this time

};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class HeightDieUpdate : public UpdateModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( HeightDieUpdate, "HeightDieUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( HeightDieUpdate, HeightDieUpdateModuleData )

public:

	HeightDieUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual UpdateSleepTime update();

protected:


	Bool m_hasDied;							///< TRUE once we have triggered death
	Bool m_particlesDestroyed;	///< TRUE once we destroy attached systems (so we do it only once)
	Coord3D m_lastPosition;			///< we record our last position for logic that needs to know our direction of travel
	UnsignedInt m_earliestDeathFrame; ///< Earliest we are allowed to think about dying

};

#endif  // end __HEIGHTDIEUPDATE_H_
