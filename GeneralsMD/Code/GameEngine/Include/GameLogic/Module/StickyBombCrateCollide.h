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

///////////////////////////////////////////////////////////////////////////////////////////////////
//	
// FILE: StickyBombCrateCollide.h 
// Author: Kris Morness, June 2003
// Desc:   A crate (actually a saboteur - mobile crate) that resets ALL command center general powers
//	
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef STICKY_BOMB_CRATE_COLLIDE_H_
#define STICKY_BOMB_CRATE_COLLIDE_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/Module.h"
#include "GameLogic/Module/CrateCollide.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Thing;

//-------------------------------------------------------------------------------------------------
class StickyBombCrateCollideModuleData : public CrateCollideModuleData
{
public:

	Bool m_needsTarget;  ///< Need valid AI target (e.g. ordered to enter a building/unit, or projectile target)
	Bool m_allowMultiCollide;  ///< if we are a crate, allow spawning multiple sticky bombs, or just single use
	Bool m_showInfiltrationEvent;   ///< show an infiltration map event for the target player
	AsciiString m_stickyBombObjectName;  ///< the object to create
	Real m_triggerChance;  ///< chance to create the bomb when triggering the module

	StickyBombCrateCollideModuleData()
	{
		m_needsTarget = FALSE;
		m_allowMultiCollide = FALSE;
		m_showInfiltrationEvent = FALSE;
		m_stickyBombObjectName = AsciiString::TheEmptyString;
		m_triggerChance = 1.0;
	}

	static void buildFieldParse(MultiIniFieldParse& p) 
	{
		CrateCollideModuleData::buildFieldParse(p);

		static const FieldParse dataFieldParse[] = 
		{
			{ "NeedsTarget", INI::parseBool, NULL, offsetof(StickyBombCrateCollideModuleData, m_needsTarget) },
			{ "AllowMultiCollide", INI::parseBool, NULL, offsetof(StickyBombCrateCollideModuleData, m_allowMultiCollide) },
			{ "ShowInfiltrationEvent", INI::parseBool, NULL, offsetof(StickyBombCrateCollideModuleData, m_showInfiltrationEvent) },
			{ "StickyBombObject", INI::parseAsciiString, NULL, offsetof(StickyBombCrateCollideModuleData, m_stickyBombObjectName) },
			{ "ChanceToTriggerPercent", INI::parsePercentToReal, NULL, offsetof(StickyBombCrateCollideModuleData, m_triggerChance) },
			{ 0, 0, 0, 0 }
		};
		p.add( dataFieldParse );
	}

};

//-------------------------------------------------------------------------------------------------
class StickyBombCrateCollide : public CrateCollide
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( StickyBombCrateCollide, "StickyBombCrateCollide" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( StickyBombCrateCollide, StickyBombCrateCollideModuleData );

public:

	StickyBombCrateCollide( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

protected:

	/// This allows specific vetoes to certain types of crates and their data
	// virtual Bool isValidToExecute( const Object *other ) const;

	/// This is the game logic execution function that all real CrateCollides will implement
	virtual Bool executeCrateBehavior( Object *other );
	
	/// This would allow entering buildings? Lets disable it for now
	// virtual Bool isSabotageBuildingCrateCollide() const { return TRUE; }

private:
	Bool m_hasCollided;

};

#endif
