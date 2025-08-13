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

// FILE: Handicap.h ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
// Project:    RTS3
//
// File name:  Handicap.h
//
// Created:    Steven Johnson, October 2001
//
// Desc:			 @todo
//
//-----------------------------------------------------------------------------

#pragma once

#ifndef _HANDICAP_H_
#define _HANDICAP_H_

#include "Lib/BaseType.h"

// ----------------------------------------------------------------------------------------------
class Object;
class Dict;
class ThingTemplate;

// ----------------------------------------------------------------------------------------------
/**
	Handicap encapsulates the sets of modifiers to abilities used to balance
	the game and give different abilities to different Players.
	Conceptually, it's a large set of coefficients (typically, but not necessarily,
	in the range of 0.0...1.0).

	Usage example (conceptual):

	Real armorCoef = this->m_handicap->getHandicap(ARMOR, this);
	Real actualArmor = this->getArmorValue() * armorCoef;
*/
class Handicap
{
public:

	// NOTE: if you ever add/remove/change the enums in HandicapType,
	// you must also modify initFromDict()!
	enum HandicapType
	{
		BUILDCOST,
		BUILDTIME,

		HANDICAP_TYPE_COUNT
	};

	Handicap();

	/// reset all handicaps to a default value.
	void init();

	/**
		initialize from the fields in the Dict. Note that this does NOT call init()
		internally, so only those fields that are present in the dict will be set.
		if you want to ensure all fields are something reasonable, you should call init()
		prior to calling this.
	*/
	void readFromDict(const Dict* d);

	/**
		return the multiplier for the given Handicap type on the given Object.
		The Object's type (unit, building, etc.) will generally be examined
		to determine what value to return.
	*/
	Real getHandicap(HandicapType t, const ThingTemplate *tmpl) const;

protected:
private:

	// NOTE: if you ever add/remove/change the enums in ThingType,
	// you must also modify initFromDict()!
	enum ThingType
	{
		GENERIC,		// if a thing is nothing else, it's generic.
		BUILDINGS,

		THING_TYPE_COUNT
	};

	Real m_handicaps[HANDICAP_TYPE_COUNT][THING_TYPE_COUNT];

	static ThingType getBestThingType(const ThingTemplate *tmpl);
};

#endif // _HANDICAP_H_
