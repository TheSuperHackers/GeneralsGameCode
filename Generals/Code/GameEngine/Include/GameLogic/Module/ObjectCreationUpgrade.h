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

// FILE: ObjectCreationUpgrade.h ///////////////////////////////////////////////////////////////////////////
// Author: Matthew D. Campbell, April 2002
// Desc:	 upgrades that spawn OCLs
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/UpgradeModule.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Thing;
class Player;
class ObjectCreationList;

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class ObjectCreationUpgradeModuleData : public UpgradeModuleData
{

public:

	ObjectCreationUpgradeModuleData( void );

	static void buildFieldParse(MultiIniFieldParse& p);

	const ObjectCreationList *m_ocl;			///< the object creation list to make

};

//-------------------------------------------------------------------------------------------------
/** The OCL upgrade module */
//-------------------------------------------------------------------------------------------------
class ObjectCreationUpgrade : public UpgradeModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( ObjectCreationUpgrade, "ObjectCreationUpgrade" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( ObjectCreationUpgrade, ObjectCreationUpgradeModuleData );

public:

	ObjectCreationUpgrade( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype defined by MemoryPoolObject

	void onDelete( void );																///< we have some work to do when this module goes away

protected:

	virtual void upgradeImplementation( void ); ///< Here's the actual work of Upgrading
	virtual Bool isSubObjectsUpgrade() { return false; }

};
