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

// GameType.h
// Basic data types needed for the game engine. This is an extension of BaseType.h.
// Author: Michael S. Booth, April 2001

#pragma once

#ifndef _GAME_TYPE_H_
#define _GAME_TYPE_H_

#include "Lib/BaseType.h"

// the default size of the world map
#define DEFAULT_WORLD_WIDTH		64
#define DEFAULT_WORLD_HEIGHT	64

/// A unique, generic "identifier" used to access Objects.
enum ObjectID CPP_11(: Int)
{
	INVALID_ID = 0,
	FORCE_OBJECTID_TO_LONG_SIZE = 0x7ffffff
};

/// A unique, generic "identifier" used to access Drawables.
enum DrawableID CPP_11(: Int)
{
	INVALID_DRAWABLE_ID = 0,
	FORCE_DRAWABLEID_TO_LONG_SIZE = 0x7ffffff
};

/// A unique, generic "identifier" used to identify player specified formations.
enum FormationID CPP_11(: Int)
{
	NO_FORMATION_ID = 0,					// Unit is not a member of any formation
	FORCE_FORMATIONID_TO_LONG_SIZE = 0x7ffffff
};

class INI;

//-------------------------------------------------------------------------------------------------
/** The time of day enumeration, keep in sync with TimeOfDayNames[] */
//-------------------------------------------------------------------------------------------------
enum TimeOfDay CPP_11(: Int)
{
	TIME_OF_DAY_INVALID = 0,
	TIME_OF_DAY_FIRST = 1,
	TIME_OF_DAY_MORNING = TIME_OF_DAY_FIRST,
	TIME_OF_DAY_AFTERNOON,
	TIME_OF_DAY_EVENING,
	TIME_OF_DAY_NIGHT,

	TIME_OF_DAY_COUNT					// keep this last
};

extern const char *TimeOfDayNames[];
// defined in Common/GameType.cpp

//-------------------------------------------------------------------------------------------------
enum Weather CPP_11(: Int)
{
	WEATHER_NORMAL = 0,
	WEATHER_SNOWY = 1,

	WEATHER_COUNT					// keep this last
};

extern const char *WeatherNames[];

enum Scorches CPP_11(: Int)
{
	SCORCH_1 = 0,
	SCORCH_2 = 1,
	SCORCH_3 = 2, 
	SCORCH_4 = 3, 
	SHADOW_SCORCH = 4,
/*	SCORCH_6 = 5, 
	SCORCH_7 = 6, 
	SCORCH_8 = 7, 

	CRATER_1 = 8,
	CRATER_2 = 9,
	CRATER_3 = 10,
	CRATER_4 = 11,
	CRATER_5 = 12,
	CRATER_6 = 13,
	CRATER_7 = 14,
	CRATER_8 = 15,

	
	MISC_DECAL_1 = 16,
	MISC_DECAL_2 = 17,
	MISC_DECAL_3 = 18,
	MISC_DECAL_4 = 19,
	MISC_DECAL_5 = 20,
	MISC_DECAL_6 = 21,
	MISC_DECAL_7 = 22,
	MISC_DECAL_8 = 23,

	MISC_DECAL_9 = 24,
	MISC_DECAL_10 = 25,
	MISC_DECAL_11 = 26,
	MISC_DECAL_12 = 27,
	MISC_DECAL_13 = 28,
	MISC_DECAL_14 = 29,
	MISC_DECAL_15 = 30,
	MISC_DECAL_16 = 31,

	MISC_DECAL_17 = 32,
	MISC_DECAL_18 = 33,
	MISC_DECAL_19 = 34,
	MISC_DECAL_20 = 35,
	MISC_DECAL_21 = 36,
	MISC_DECAL_22 = 37,
	MISC_DECAL_23 = 38,
	MISC_DECAL_24 = 39,

	MISC_DECAL_25 = 40,
	MISC_DECAL_26 = 41,
	MISC_DECAL_27 = 42,
	MISC_DECAL_28 = 43,
	MISC_DECAL_29 = 44,
	MISC_DECAL_30 = 45,
	MISC_DECAL_31 = 46,
	MISC_DECAL_32 = 47,

	MISC_DECAL_33 = 48,
	MISC_DECAL_34 = 49,
	MISC_DECAL_35 = 50,
	MISC_DECAL_36 = 51,
	MISC_DECAL_37 = 52,
	MISC_DECAL_38 = 53,
	MISC_DECAL_39 = 54,
	MISC_DECAL_40 = 55,

	MISC_DECAL_41 = 56,
	MISC_DECAL_42 = 57,
	MISC_DECAL_43 = 58,
	MISC_DECAL_44 = 59,
	MISC_DECAL_45 = 60,
	MISC_DECAL_46 = 61,
	MISC_DECAL_47 = 62,
	MISC_DECAL_48 = 63,
*/
	SCORCH_COUNT
};

//-------------------------------------------------------------------------------------------------
enum WeaponSlotType CPP_11(: Int)
{
	PRIMARY_WEAPON = 0,
	SECONDARY_WEAPON,
	TERTIARY_WEAPON,

	WEAPONSLOT_COUNT	// keep last
};

//-------------------------------------------------------------------------------------------------
// Pathfind layers - ground is the first layer, each bridge is another. jba.
// Layer 1 is the ground.
// Layer 2 is the top layer - bridge if one is present, ground otherwise.
// Layer 2 - LAYER_LAST -1 are bridges. 
// Layer_WALL is a special "wall" layer for letting units run aroound on top of a wall
// made of structures.
// Note that the bridges just index in the pathfinder, so you don't actually
// have a LAYER_BRIDGE_1 enum value.
enum PathfindLayerEnum CPP_11(: Int) {LAYER_INVALID = 0, LAYER_GROUND = 1, LAYER_WALL = 15, LAYER_LAST=15};

//-------------------------------------------------------------------------------------------------

#endif // _GAME_TYPE_H_

