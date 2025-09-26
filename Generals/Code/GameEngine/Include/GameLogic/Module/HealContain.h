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

// FILE: HealContain.h ////////////////////////////////////////////////////////////////////////////
// Author: Colin Day
// Desc:   Objects that are contained inside a heal contain ... get healed!  oh my!
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __HEALCONTAIN_H_
#define __HEALCONTAIN_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/OpenContain.h"

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class HealContainModuleData : public OpenContainModuleData
{

public:

	HealContainModuleData( void );

	static void buildFieldParse(MultiIniFieldParse& p);

	UnsignedInt m_framesForFullHeal;			///< time (in frames) something becomes fully healed

};

//-------------------------------------------------------------------------------------------------
/** The healing container ... bright white light ahhhhh goes here */
//-------------------------------------------------------------------------------------------------
class HealContain : public OpenContain
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( HealContain, "HealContain" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( HealContain, HealContainModuleData )

public:

	HealContain( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual UpdateSleepTime update();												///< called once per frame
	virtual Bool isHealContain() const { return true; } ///< true when container only contains units while healing (not a transport!)
	virtual Bool isTunnelContain() const { return FALSE; }

protected:

	Bool doHeal( Object *obj, UnsignedInt framesForFullHeal );		///< do the heal on an object

};

#endif  // end __HEALCONTAIN_H_
