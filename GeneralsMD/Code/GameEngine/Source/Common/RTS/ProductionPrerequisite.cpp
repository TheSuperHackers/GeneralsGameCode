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

// FILE: ProductionPrerequisite.cpp /////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//                                                                          
//                       Westwood Studios Pacific.                          
//                                                                          
//                       Confidential Information                           
//                Copyright (C) 2001 - All Rights Reserved                  
//                                                                          
//-----------------------------------------------------------------------------
//
// Project:   RTS3
//
// File name: ProductionPrerequisite.cpp
//
// Created:   Steven Johnson, October 2001
//
// Desc:      @todo
//
//-----------------------------------------------------------------------------

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/ProductionPrerequisite.h"
#include "Common/Player.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "GameLogic/Object.h"
#include "GameClient/Drawable.h"
#include "GameClient/GameText.h"


//-----------------------------------------------------------------------------
ProductionPrerequisite::ProductionPrerequisite() 
{
	init();
}

//-----------------------------------------------------------------------------
ProductionPrerequisite::~ProductionPrerequisite()
{
}

//-----------------------------------------------------------------------------
void ProductionPrerequisite::init()
{
	m_prereqUnits.clear();
	m_prereqSciences.clear();
	
}

//=============================================================================
void ProductionPrerequisite::resolveNames()
{
	for (size_t i = 0; i < m_prereqUnits.size(); i++)
	{

		//
		// note that this will find the template at the "top most" level (not override
		// sub-temlates), which is what we want ... we conceptually only have one
		// template for any given thing, it's only the *data* that is overridden
		//
		if( m_prereqUnits[ i ].name.isNotEmpty() )
		{
			m_prereqUnits[i].unit = TheThingFactory->findTemplate(m_prereqUnits[i].name);	// might be null

 			/** @todo for now removing this assert until we can completely remove
 			the GDF stuff, the problem is that some INI files refer to GDF names, and they
 			aren't yet loaded in the world builder but will all go away later anyway etc */
			DEBUG_ASSERTCRASH(m_prereqUnits[i].unit,("could not find prereq %s\n",m_prereqUnits[i].name.str()));

			m_prereqUnits[i].name.clear(); // we're done with it
		}

	}

}

//-----------------------------------------------------------------------------
Int ProductionPrerequisite::calcNumPrereqUnitsOwned(const Player *player, Int counts[MAX_PREREQ]) const
{
	const ThingTemplate *tmpls[MAX_PREREQ];
	Int cnt = m_prereqUnits.size();
	if (cnt > MAX_PREREQ)
		cnt = MAX_PREREQ;
	for (int i = 0; i < cnt; i++)
		tmpls[i] = m_prereqUnits[i].unit;
	player->countObjectsByThingTemplate(cnt, tmpls, false, counts);
	return cnt;
}

//-----------------------------------------------------------------------------
Int ProductionPrerequisite::getAllPossibleBuildFacilityTemplates(const ThingTemplate* tmpls[], Int maxtmpls) const
{
	Int count = 0;
	for (size_t i = 0; i < m_prereqUnits.size(); i++)
	{
		if (i > 0 && !(m_prereqUnits[i].flags & UNIT_OR_WITH_PREV))
			break;
		if (count >= maxtmpls)
			break;
		tmpls[count++] = m_prereqUnits[i].unit;
	}
	return count;
}

//-----------------------------------------------------------------------------
const ThingTemplate *ProductionPrerequisite::getExistingBuildFacilityTemplate( const Player *player ) const
{
	DEBUG_ASSERTCRASH(player, ("player may not be null"));
	if (m_prereqUnits.size())
	{
		Int ownCount[MAX_PREREQ];
		Int cnt = calcNumPrereqUnitsOwned(player, ownCount);
		for (int i = 0; i < cnt; i++)
		{
			if (i > 0 && !(m_prereqUnits[i].flags & UNIT_OR_WITH_PREV))
				break;
			if (ownCount[i])
				return m_prereqUnits[i].unit;
		} 
	}
	return NULL;
}

//-----------------------------------------------------------------------------
Bool ProductionPrerequisite::isSatisfied(const Player *player) const
{
	Int i;

	if (!player)
		return false;

	// gotta have all the prereq sciences.
	for (i = 0; i < m_prereqSciences.size(); i++)
	{
		if (!player->hasScience(m_prereqSciences[i]))
			return false;
	}

	// the player must have at least one instance of each prereq unit.
	Int ownCount[MAX_PREREQ];
	Int cnt = calcNumPrereqUnitsOwned(player, ownCount);

	// fix up the "or" cases. (start at 1!)
	for (i = 1; i < cnt; i++)
	{
		if (m_prereqUnits[i].flags & UNIT_OR_WITH_PREV)
		{
			ownCount[i] += ownCount[i-1];	// lump 'em together for prereq purposes
			ownCount[i-1] = -1;						// flag for "ignore me"
		}
	}

	for (i = 0; i < cnt; i++)
	{
		if (ownCount[i] == -1)	// the magic "ignore me" flag
			continue;	
		if (ownCount[i] == 0)		// everything not ignored, is required
			return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------------------
/** Add a unit prerequisite, if 'orWithPrevious' is set then this unit is said
	* to be an alternate prereq to the previously added unit, otherwise this becomes
	* a new 'block' and is required in ADDDITION to other entries. 
	* Return FALSE if no space left to add unit */
//-------------------------------------------------------------------------------------------------
void ProductionPrerequisite::addUnitPrereq( AsciiString unit, Bool orUnitWithPrevious )
{
	PrereqUnitRec info;
	info.name = unit;
	info.flags = orUnitWithPrevious ? UNIT_OR_WITH_PREV : 0;
	info.unit = NULL;
	m_prereqUnits.push_back(info);

}  // end addUnitPrereq

//-------------------------------------------------------------------------------------------------
/** Add a unit prerequisite, if 'orWithPrevious' is set then this unit is said
	* to be an alternate prereq to the previously added unit, otherwise this becomes
	* a new 'block' and is required in ADDDITION to other entries. 
	* Return FALSE if no space left to add unit */
//-------------------------------------------------------------------------------------------------
void ProductionPrerequisite::addUnitPrereq( const std::vector<AsciiString>& units )
{
	Bool orWithPrevious = false;
	for (size_t i = 0; i < units.size(); ++i)
	{
		addUnitPrereq(units[i], orWithPrevious);
		orWithPrevious = true;
	}

}  // end addUnitPrereq

//-------------------------------------------------------------------------------------------------
// returns an asciistring which is a list of all the prerequisites
// not satisfied yet
UnicodeString ProductionPrerequisite::getRequiresList(const Player *player) const
{

	// if player is invalid, return empty string
	if (!player)
		return UnicodeString::TheEmptyString;

	UnicodeString requiresList = UnicodeString::TheEmptyString;
		
	// check the prerequired units
	Int ownCount[MAX_PREREQ];
	Int cnt = calcNumPrereqUnitsOwned(player, ownCount);
	Int i;

	Bool orRequirements[MAX_PREREQ];
	//Added for fix below in getRequiresList
	//By Sadullah Nader
	//Initializes the OR_WITH_PREV structures
	for (i = 0; i < MAX_PREREQ; i++)
	{
		orRequirements[i] = FALSE;
	}
	//
	// account for the "or" unit cases, start for loop at 1
	for (i = 1; i < cnt; i++)
	{
		if (m_prereqUnits[i].flags & UNIT_OR_WITH_PREV)
		{
			orRequirements[i] = TRUE;     // set the flag for this unit to be "ored" with previous
			ownCount[i] += ownCount[i-1];	// lump 'em together for prereq purposes
			ownCount[i-1] = -1;						// flag for "ignore me"
		}
	}
	
	// check to see if anything is required
	const ThingTemplate *unit;
	UnicodeString unitName;
	Bool firstRequirement = true;
	for (i = 0; i < cnt; i++)
	{
		// we have an unfulfilled requirement
		if (ownCount[i] == 0) {
			
			if(orRequirements[i])
			{
				unit = m_prereqUnits[i-1].unit;
				unitName = unit->getDisplayName();
				unitName.concat( L" " );
				unitName.concat(TheGameText->fetch("CONTROLBAR:OrRequirement", NULL));
				unitName.concat( L" " );
				requiresList.concat(unitName);
			}

			// get the requirement and then its name
			unit = m_prereqUnits[i].unit;
			unitName = unit->getDisplayName();

			// gets command button, and then modifies unitName
			//CommandButton *cmdButton = TheControlBar->findCommandButton(unit->getName());
			//if (cmdButton)
				//unitName.translate(TheGameText->fetch(cmdButton->m_textLabel.str()));

			// format name appropriately with 'returns' if necessary
			if (firstRequirement)
				firstRequirement = false;
			else
				unitName.concat(L"\n");

			// add it to the list
			requiresList.concat(unitName);
		}
	}

	Bool hasSciences = TRUE;
	// gotta have all the prereq sciences.
	for (i = 0; i < m_prereqSciences.size(); i++)
	{
		if (!player->hasScience(m_prereqSciences[i]))
			hasSciences = FALSE;
	}

	if (hasSciences == FALSE) {
		if (firstRequirement) {
			firstRequirement = false;
		} else {
			unitName.concat(L"\n");
		}
		requiresList.concat(TheGameText->fetch("CONTROLBAR:GeneralsPromotion", NULL));
	}

	// return final list
	return requiresList;
}