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

// FILE: AnimationSteeringUpdate.h ////////////////////////////////////////////////////////////////////////////
// Author: Kris Morness, April 2002
// Desc:   Uses animation states to handle steering.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __ANIMATION_STEERING_UPDATE_H
#define __ANIMATION_STEERING_UPDATE_H

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/UpdateModule.h"

enum PhysicsTurningType CPP_11(: Int);

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class AnimationSteeringUpdateModuleData : public UpdateModuleData
{

public:

	AnimationSteeringUpdateModuleData( void );

	static void buildFieldParse(MultiIniFieldParse& p)
	{
    UpdateModuleData::buildFieldParse( p );

		static const FieldParse dataFieldParse[] =
		{
			{ "MinTransitionTime", INI::parseDurationUnsignedInt, NULL, offsetof( AnimationSteeringUpdateModuleData, m_transitionFrames ) },
			{ 0, 0, 0, 0 }
		};
    p.add(dataFieldParse);

	}

	UnsignedInt m_transitionFrames;
};

//-------------------------------------------------------------------------------------------------
/** The AnimationSteering Update module */
//-------------------------------------------------------------------------------------------------
class AnimationSteeringUpdate : public UpdateModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( AnimationSteeringUpdate, "AnimationSteeringUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( AnimationSteeringUpdate, AnimationSteeringUpdateModuleData );

public:

	AnimationSteeringUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype defined by MemoryPoolObject

	virtual UpdateSleepTime update( void ); ///< Here's the actual work of Upgrading

protected:

  ModelConditionFlagType m_currentTurnAnim;
	UnsignedInt m_nextTransitionFrame;
};

#endif  // end __ANIMATION_STEERING_UPDATE_H
