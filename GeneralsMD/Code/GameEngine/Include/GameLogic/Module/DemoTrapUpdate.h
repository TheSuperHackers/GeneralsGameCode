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

// FILE: DemoTrapUpdate.cpp //////////////////////////////////////////////////////////////////////////
// Author: Kris Morness, August 2002
// Desc:   Update module to handle demo trap proximity triggering.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/KindOf.h"
#include "GameLogic/Module/UpdateModule.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class DemoTrapUpdateModuleData : public ModuleData
{
public:
	WeaponTemplate *m_detonationWeaponTemplate;
	KindOfMaskType	m_ignoreKindOf;
	WeaponSlotType  m_manualModeWeaponSlot;
	WeaponSlotType  m_detonationWeaponSlot;
	WeaponSlotType  m_proximityModeWeaponSlot;
	Real						m_triggerDetonationRange;
	UnsignedInt			m_scanFrames;
	Bool						m_defaultsToProximityMode;
	Bool						m_friendlyDetonation;
	Bool						m_detonateWhenKilled;

	DemoTrapUpdateModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);

private:

};

//-------------------------------------------------------------------------------------------------
/** The default	update module */
//-------------------------------------------------------------------------------------------------
class DemoTrapUpdate : public UpdateModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( DemoTrapUpdate, "DemoTrapUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( DemoTrapUpdate, DemoTrapUpdateModuleData );

public:

	DemoTrapUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual void onObjectCreated();
	virtual UpdateSleepTime update();

	void detonate();

protected:

	Int m_nextScanFrames;
	Bool m_detonated;
};
