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

// FILE: GrantScienceUpgrade.h /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Electronic Arts Los Angeles
//
//                       Confidential Information
//                Copyright (C) 2003 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
//	Created:	  August 2, 2003
//
//	Filename: 	GrantScienceUpgrade.cpp
//
//	Author:		  Kris Morness
//
//	Purpose:	  Grants specified science once requirements met (typically an upgrade).
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameLogic/Module/UpgradeModule.h"

//-----------------------------------------------------------------------------
class GrantScienceUpgradeModuleData : public UpgradeModuleData
{
public:
	AsciiString m_grantScienceName;

	GrantScienceUpgradeModuleData(){}

	static void buildFieldParse(MultiIniFieldParse& p);
};

//-----------------------------------------------------------------------------
class GrantScienceUpgrade : public UpgradeModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( GrantScienceUpgrade, "GrantScienceUpgrade" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( GrantScienceUpgrade, GrantScienceUpgradeModuleData );

public:

	GrantScienceUpgrade( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype defined by MemoryPoolObject

protected:
	virtual void upgradeImplementation( ); ///< Here's the actual work of Upgrading
	virtual Bool isSubObjectsUpgrade() { return false; }

private:

	ScienceType m_scienceType;

};
