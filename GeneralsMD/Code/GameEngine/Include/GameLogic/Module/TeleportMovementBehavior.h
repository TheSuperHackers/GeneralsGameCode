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

// FILE: TeleportMovementBehavior.h /////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, July 2002
// Desc:   Behavior that reacts to poison Damage by continuously damaging us further in an Update
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __TeleportMovement_Behavior_H_
#define __TeleportMovement_Behavior_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/BehaviorModule.h"
#include "GameLogic/Module/UpdateModule.h"


//-------------------------------------------------------------------------------------------------
class TeleportMovementBehaviorModuleData : public UpdateModuleData
{
public:

	TeleportMovementBehaviorModuleData();

	static void buildFieldParse(MultiIniFieldParse& p);

	Real m_minDistance;
	Real m_disabledDuration;

private:

};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class TeleportMovementBehavior : public UpdateModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(TeleportMovementBehavior, "TeleportMovementBehavior")
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA(TeleportMovementBehavior, TeleportMovementBehaviorModuleData)

public:
	TeleportMovementBehavior(Thing* thing, const ModuleData* moduleData);

	// UpdateInterface
	virtual UpdateSleepTime update();

	void doTeleport(Coord3D targetPos, Real angle, Real dist);

protected:


private:


};

#endif // __TeleportMovement_Behavior_H_

