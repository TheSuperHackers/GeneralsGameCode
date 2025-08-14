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

// FILE: DelayedUpgrade.h /////////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, April 2002
// Desc:	 An Upgrade that broadcasts to all DelayedUpgradeUpdates that maybe they should start
//					counting down to execution
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __DELAYED_UPGRADE_H_
#define __DELAYED_UPGRADE_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/UpgradeModule.h"

//-------------------------------------------------------------------------------------------------
class DelayedUpgradeModuleData : public UpgradeModuleData
{
public:
	UnsignedInt m_delayTime;

	DelayedUpgradeModuleData()
	{
		m_delayTime = 0;
	}

	static void buildFieldParse(MultiIniFieldParse& p)
	{
    UpgradeModuleData::buildFieldParse(p);

		static const FieldParse dataFieldParse[] =
		{
			{ "DelayTime",	INI::parseDurationUnsignedInt,	NULL, offsetof( DelayedUpgradeModuleData, m_delayTime ) },
			{ 0, 0, 0, 0 }
		};
    p.add(dataFieldParse);
	}
};

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Thing;

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class DelayedUpgrade : public UpgradeModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( DelayedUpgrade, "DelayedUpgrade" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( DelayedUpgrade, DelayedUpgradeModuleData );

public:

	DelayedUpgrade( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype defined by MemoryPoolObject

protected:

	virtual void upgradeImplementation( ); ///< Here's the actual work of Upgrading
	virtual Bool isSubObjectsUpgrade() { return false; }

};


#endif

