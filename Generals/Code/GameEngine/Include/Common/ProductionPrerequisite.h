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

// FILE: ProductionPrerequisite.h //////////////////////////////////////////////////////
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
// File name:  ProductionPrerequisite.h
//
// Created:    Steven Johnson, October 2001
//
//-----------------------------------------------------------------------------

#pragma once

#ifndef __ProductionPrerequisite_H_
#define __ProductionPrerequisite_H_

//-----------------------------------------------------------------------------
//           Includes
//-----------------------------------------------------------------------------
#include "Common/GameMemory.h"
#include "Common/GameCommon.h"
#include "Common/Science.h"
//#include "GameClient/ControlBar.h"

//-----------------------------------------------------------------------------
class ThingTemplate;
class Player;

//-----------------------------------------------------------------------------
class ProductionPrerequisite
{
public:

	ProductionPrerequisite();
	~ProductionPrerequisite();

	/// init to safe default values.
	void init();

	void resetSciences( void ) { m_prereqSciences.clear(); }
	void addSciencePrereq( ScienceType science ) { m_prereqSciences.push_back(science); }

	void resetUnits( void ) { m_prereqUnits.clear(); }
	void addUnitPrereq( AsciiString units, Bool orUnitWithPrevious );
	void addUnitPrereq( const std::vector<AsciiString>& units );

	/// called after all ThingTemplates have been loaded.
	void resolveNames();

	/// returns an asciistring which is a list of all the prerequisites
	/// not satisfied yet
	UnicodeString getRequiresList(const Player *player) const;

	/// return true iff the player satisfies our set of prerequisites
	Bool isSatisfied(const Player *player) const;

	/**
		return the BuildFacilityTemplate, if any.

		if this template needs no build facility, null is returned.

		if the template needs a build facility but the given player doesn't have any in existence,
		null will be returned.

		you may not pass 'null' for player.
	*/
	const ThingTemplate *getExistingBuildFacilityTemplate( const Player *player ) const;

	Int getAllPossibleBuildFacilityTemplates(const ThingTemplate* tmpls[], Int maxtmpls) const;

private:

	enum
	{
		UNIT_OR_WITH_PREV = 0x01	// if set, unit is "or-ed" with prev unit, so that either one's presence satisfies
	};

	struct PrereqUnitRec
	{
		const ThingTemplate*	unit;
		Int										flags;
		AsciiString						name;
	};

	enum { MAX_PREREQ = 32 };
	Int calcNumPrereqUnitsOwned(const Player *player, Int counts[MAX_PREREQ]) const;

	std::vector<PrereqUnitRec>	m_prereqUnits;
	ScienceVec									m_prereqSciences;
};

//-----------------------------------------------------------------------------

#endif
