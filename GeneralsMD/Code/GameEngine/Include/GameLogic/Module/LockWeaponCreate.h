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

// FILE: LockWeaponCreate.h //////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, March 2003
// Desc:   Locks the weapon choice to the slot specified on creation
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/CreateModule.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
/** The GrantUpgrade create module */
//-------------------------------------------------------------------------------------------------

class LockWeaponCreateModuleData : public CreateModuleData
{
public:
	WeaponSlotType m_slotToLock; ///< slot to lock

	LockWeaponCreateModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class LockWeaponCreate : public CreateModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( LockWeaponCreate, "LockWeaponCreate" );
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( LockWeaponCreate, LockWeaponCreateModuleData );


public:

	LockWeaponCreate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	/// the create method
	virtual void onCreate( void );
	virtual void onBuildComplete();	///< This is called when you are a finished game object

protected:

};
