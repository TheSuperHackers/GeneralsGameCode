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

// FILE: CostModifierUpgrade.h /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//                                                                          
//                       Electronic Arts Pacific.                          
//                                                                          
//                       Confidential Information                           
//                Copyright (C) 2002 - All Rights Reserved                  
//                                                                          
//-----------------------------------------------------------------------------
//
//	created:	June 2025
//
//	Filename: 	UnitProductionBonusUpgrade.h
//
//	author:		Andi W
// 
//	purpose:	
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __UNIT_PRODUCTION_BONUS_UPGRADE_H_
#define __UNIT_PRODUCTION_BONUS_UPGRADE_H_

//-----------------------------------------------------------------------------
#include "GameLogic/Module/UpgradeModule.h"

//-----------------------------------------------------------------------------
class Thing;
class Player;


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class UnitProductionBonusUpgradeModuleData : public UpgradeModuleData
{

public:

	UnitProductionBonusUpgradeModuleData(void);

	static void buildFieldParse(MultiIniFieldParse& p);

	std::vector<AsciiString> m_templateNames;
	Real m_costPercentage;
	Real m_timePercentage;
	// Bool m_isOneShot;
};

//-------------------------------------------------------------------------------------------------
/** The OCL upgrade module */
//-------------------------------------------------------------------------------------------------
class UnitProductionBonusUpgrade : public UpgradeModule
{

	    MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(UnitProductionBonusUpgrade, "UnitProductionBonusUpgrade")
		MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA(UnitProductionBonusUpgrade, UnitProductionBonusUpgradeModuleData);

public:

	UnitProductionBonusUpgrade(Thing* thing, const ModuleData* moduleData);
	// virtual destructor prototype defined by MemoryPoolObject

	// virtual void onDelete(void);																///< we have some work to do when this module goes away
	// virtual void onCapture(Player* oldOwner, Player* newOwner);

protected:

	virtual void upgradeImplementation(void); ///< Here's the actual work of Upgrading
	virtual Bool isSubObjectsUpgrade() { return false; }

};

#endif // __UNIT_PRODUCTION_BONUS_UPGRADE_H_
