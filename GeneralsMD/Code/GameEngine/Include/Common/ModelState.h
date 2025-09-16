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

// ModelState.h
// Basic data types needed for the game engine. This is an extension of BaseType.h.
// Author: Michael S. Booth, April 2001

#pragma once

#ifndef _ModelState_H_
#define _ModelState_H_

#include "Lib/BaseType.h"
#include "Common/INI.h"
#include "Common/BitFlags.h"
#include "Common/BitFlagsIO.h"

//-------------------------------------------------------------------------------------------------

/**
	THE PROBLEM
	-----------

	-- there are lots of different states. (consider that structures can be: day/night, snow/nosnow,
	powered/not, garrisoned/empty, damaged/not... you do the math.)

	-- some states are mutually exclusive (idle vs. moving), others are not (snow/nosnow, day/night).
	generally, humanoid units have mostly the former, while structure units have mostly the latter.
	The current ModelState system really only supports the mutually-exclusive states well.

	-- we'd rather not have to specify every state in the INI files, BUT we do want to be able
	to intelligently choose the best model for a given state, whether or not a "real" state exists for it.

	-- it would be desirable to have a unified way of representing "ModelState" so that we don't
	have multiple similar-yet-different systems.

	YUCK, WHAT NOW
	--------------

	Let's represent the Model State with two dictinct pieces:

	-- an "ActionState" piece, representing the mutually-exclusive states, which are almost
		always an action of some sort

	-- and a "ConditionState" piece, which is a set of bitflags to indicate the static "condition"
		of the model.

	Note that these are usually set independently in code, but they are lumped together in order to
	determine the actual model to be used.

	(Let's require all objects would be required to have an "Idle" ActionState, which is the
	normal, just-sitting there condition.)

	From a code point of view, this becomes an issue of requesting a certain state, and
	finding the best-fit match for it. So, what are the rules for finding a good match?

	-- Action states must match exactly. If the desired action state is not found, then the
	IDLE state is substituted (but this should generally be considered an error condition).

	-- Condition states choose the match with the closest match among the "Condition" bits in the
	INI file, based on satisfying the most of the "required" conditions and the fewest of the
	"forbidden" conditions.

*/

#define NUM_MODELCONDITION_DOOR_STATES 4

//-------------------------------------------------------------------------------------------------
// IMPORTANT NOTE: you should endeavor to set up states such that the most "normal"
// state is defined by the bit being off. That is, the typical "normal" condition
// has all condition flags set to zero.
enum ModelConditionFlagType CPP_11(: Int)
{
	MODELCONDITION_INVALID = -1,

	MODELCONDITION_FIRST = 0,

//
// Note: these values are saved in save files, so you MUST NOT REMOVE OR CHANGE
// existing values!
//
	MODELCONDITION_TOPPLED = MODELCONDITION_FIRST,
	MODELCONDITION_FRONTCRUSHED,
	MODELCONDITION_BACKCRUSHED,
	MODELCONDITION_DAMAGED,
	MODELCONDITION_REALLY_DAMAGED,
	MODELCONDITION_RUBBLE,
	MODELCONDITION_SPECIAL_DAMAGED,
	MODELCONDITION_NIGHT,
	MODELCONDITION_SNOW,
	MODELCONDITION_PARACHUTING,
	MODELCONDITION_GARRISONED,
	MODELCONDITION_ENEMYNEAR,
	MODELCONDITION_WEAPONSET_VETERAN,
	MODELCONDITION_WEAPONSET_ELITE,
	MODELCONDITION_WEAPONSET_HERO,
	MODELCONDITION_WEAPONSET_CRATEUPGRADE_ONE,
	MODELCONDITION_WEAPONSET_CRATEUPGRADE_TWO,
	MODELCONDITION_WEAPONSET_PLAYER_UPGRADE,
	MODELCONDITION_DOOR_1_OPENING,
	MODELCONDITION_DOOR_1_CLOSING,
	MODELCONDITION_DOOR_1_WAITING_OPEN,
	MODELCONDITION_DOOR_1_WAITING_TO_CLOSE,
	MODELCONDITION_DOOR_2_OPENING,
	MODELCONDITION_DOOR_2_CLOSING,
	MODELCONDITION_DOOR_2_WAITING_OPEN,
	MODELCONDITION_DOOR_2_WAITING_TO_CLOSE,
	MODELCONDITION_DOOR_3_OPENING,
	MODELCONDITION_DOOR_3_CLOSING,
	MODELCONDITION_DOOR_3_WAITING_OPEN,
	MODELCONDITION_DOOR_3_WAITING_TO_CLOSE,
	MODELCONDITION_DOOR_4_OPENING,
	MODELCONDITION_DOOR_4_CLOSING,
	MODELCONDITION_DOOR_4_WAITING_OPEN,
	MODELCONDITION_DOOR_4_WAITING_TO_CLOSE,
	MODELCONDITION_ATTACKING,		//Simply set when a unit is fighting -- terrorist moving with a target will flail arms like a psycho.
	MODELCONDITION_PREATTACK_A,		//Use for pre-attack animations (like aiming, pulling out a knife, or detonating explosives).
	MODELCONDITION_FIRING_A,
	MODELCONDITION_BETWEEN_FIRING_SHOTS_A,
	MODELCONDITION_RELOADING_A,
	MODELCONDITION_PREATTACK_B,		//Use for pre-attack animations (like aiming, pulling out a knife, or detonating explosives).
	MODELCONDITION_FIRING_B,
	MODELCONDITION_BETWEEN_FIRING_SHOTS_B,
	MODELCONDITION_RELOADING_B,
	MODELCONDITION_PREATTACK_C,		//Use for pre-attack animations (like aiming, pulling out a knife, or detonating explosives).
	MODELCONDITION_FIRING_C,
	MODELCONDITION_BETWEEN_FIRING_SHOTS_C,
	MODELCONDITION_RELOADING_C,
	MODELCONDITION_TURRET_ROTATE,
	MODELCONDITION_POST_COLLAPSE,
	MODELCONDITION_MOVING,
	MODELCONDITION_DYING,
	MODELCONDITION_AWAITING_CONSTRUCTION,
	MODELCONDITION_PARTIALLY_CONSTRUCTED,
	MODELCONDITION_ACTIVELY_BEING_CONSTRUCTED,
	MODELCONDITION_PRONE,
	MODELCONDITION_FREEFALL,
	MODELCONDITION_ACTIVELY_CONSTRUCTING,
	MODELCONDITION_CONSTRUCTION_COMPLETE,
	MODELCONDITION_RADAR_EXTENDING,
	MODELCONDITION_RADAR_UPGRADED,
	MODELCONDITION_PANICKING,	// yes, it's spelled with a "k". look it up.
	MODELCONDITION_AFLAME,
	MODELCONDITION_SMOLDERING,
	MODELCONDITION_BURNED,
	MODELCONDITION_DOCKING,						///< This encloses the whole time you are Entering, Actioning, and Exiting a dock
	MODELCONDITION_DOCKING_BEGINNING,	///< From Enter to Action
	MODELCONDITION_DOCKING_ACTIVE,		///< From Action to Exit
	MODELCONDITION_DOCKING_ENDING,		///< Exit all the way to next enter (use only animations that end with this)
	MODELCONDITION_CARRYING,
	MODELCONDITION_FLOODED,
	MODELCONDITION_LOADED,				// loaded woot! ... like a transport is loaded
	MODELCONDITION_JETAFTERBURNER,// shows "flames" for extra motive force (eg, when taking off)
	MODELCONDITION_JETEXHAUST,		// shows "exhaust" for motive force
	MODELCONDITION_PACKING,				// packs an object
	MODELCONDITION_UNPACKING,			// unpacks an object
	MODELCONDITION_DEPLOYED,			// a deployed object state
	MODELCONDITION_OVER_WATER,		// Units that can go over water want cool effects for doing so
	MODELCONDITION_POWER_PLANT_UPGRADED,	// to show special control rods on the cold fusion plant
	MODELCONDITION_CLIMBING,	//For units climbing up or down cliffs.
	MODELCONDITION_SOLD,					// object is being sold
#ifdef ALLOW_SURRENDER
	MODELCONDITION_SURRENDER,			//When units surrender...
#endif
	MODELCONDITION_RAPPELLING,
	MODELCONDITION_ARMED,					// armed like a mine or bomb is armed (not like a human is armed)
	MODELCONDITION_POWER_PLANT_UPGRADING,	// while special control rods on the cold fusion plant are extending

	//Special model conditions work as following:
	//Something turns it on... but a timer in the object will turn them off after a given
	//amount of time. If you add any more special animations, then you'll need to add the
	//code to turn off the state.
	MODELCONDITION_SPECIAL_CHEERING,	//When units do a victory cheer (or player initiated cheer).

	MODELCONDITION_CONTINUOUS_FIRE_SLOW,
	MODELCONDITION_CONTINUOUS_FIRE_MEAN,
	MODELCONDITION_CONTINUOUS_FIRE_FAST,

	MODELCONDITION_RAISING_FLAG,
	MODELCONDITION_CAPTURED,

	MODELCONDITION_EXPLODED_FLAILING,
	MODELCONDITION_EXPLODED_BOUNCING,
	MODELCONDITION_SPLATTED,

	// this is an easier-to-use variant on the whole FIRING_A deal...
	// these bits are set if firing, reloading, between shots, or preattack.
	MODELCONDITION_USING_WEAPON_A,
	MODELCONDITION_USING_WEAPON_B,
	MODELCONDITION_USING_WEAPON_C,

	MODELCONDITION_PREORDER,

	MODELCONDITION_CENTER_TO_LEFT,
	MODELCONDITION_LEFT_TO_CENTER,
	MODELCONDITION_CENTER_TO_RIGHT,
	MODELCONDITION_RIGHT_TO_CENTER,

	MODELCONDITION_RIDER1,	//Added these for different riders
	MODELCONDITION_RIDER2,
	MODELCONDITION_RIDER3,
	MODELCONDITION_RIDER4,
	MODELCONDITION_RIDER5,
	MODELCONDITION_RIDER6,
	MODELCONDITION_RIDER7,
	MODELCONDITION_RIDER8,

	MODELCONDITION_STUNNED_FLAILING, // Daniel Teh's idea, added by Lorenzen, 5/28/03
	MODELCONDITION_STUNNED,
	MODELCONDITION_SECOND_LIFE,
	MODELCONDITION_JAMMED,	///< Jammed as in missile jammed by ECM
	MODELCONDITION_ARMORSET_CRATEUPGRADE_ONE,
	MODELCONDITION_ARMORSET_CRATEUPGRADE_TWO,

	MODELCONDITION_USER_1,		///< Wildcard flag to use with upgrade modules or other random little things
	MODELCONDITION_USER_2,

	MODELCONDITION_DISGUISED,
//
// Note: these values are saved in save files, so you MUST NOT REMOVE OR CHANGE
// existing values!
//

	MODELCONDITION_COUNT
};

//-------------------------------------------------------------------------------------------------

typedef BitFlags<MODELCONDITION_COUNT> ModelConditionFlags;

#define MAKE_MODELCONDITION_MASK(k) ModelConditionFlags(ModelConditionFlags::kInit, (k))
#define MAKE_MODELCONDITION_MASK2(k,a) ModelConditionFlags(ModelConditionFlags::kInit, (k), (a))
#define MAKE_MODELCONDITION_MASK3(k,a,b) ModelConditionFlags(ModelConditionFlags::kInit, (k), (a), (b))
#define MAKE_MODELCONDITION_MASK4(k,a,b,c) ModelConditionFlags(ModelConditionFlags::kInit, (k), (a), (b), (c))
#define MAKE_MODELCONDITION_MASK5(k,a,b,c,d) ModelConditionFlags(ModelConditionFlags::kInit, (k), (a), (b), (c), (d))
#define MAKE_MODELCONDITION_MASK12(a,b,c,d,e,f,g,h,i,j,k,l) ModelConditionFlags(ModelConditionFlags::kInit, (a), (b), (c), (d), (e), (f), (g), (h), (i), (j), (k), (l))

//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------

#endif // _ModelState_H_

