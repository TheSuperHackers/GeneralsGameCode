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

// FILE: EnemyNearUpdate.h /////////////////////////////////////////////////////////////////////////////
// Author: 	Matthew D. Campbell, April 2002
// Desc:  Reacts when an enemy is within range
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __EnemyNearUpdate_H_
#define __EnemyNearUpdate_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/UpdateModule.h"
#include "Common/KindOf.h"

//-------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class EnemyNearUpdateModuleData : public UpdateModuleData
{
public:
	UnsignedInt m_enemyScanDelayTime;

	EnemyNearUpdateModuleData()
	{
		m_enemyScanDelayTime = LOGICFRAMES_PER_SECOND;
	}

	static void buildFieldParse(MultiIniFieldParse& p)
	{
    UpdateModuleData::buildFieldParse(p);
		static const FieldParse dataFieldParse[] =
		{
			{ "ScanDelayTime",		INI::parseDurationUnsignedInt,		NULL, offsetof( EnemyNearUpdateModuleData, m_enemyScanDelayTime ) },
			{ 0, 0, 0, 0 }
		};
    p.add(dataFieldParse);
	}
};

//-------------------------------------------------------------------------------------------------
/** EnemyNear update */
//-------------------------------------------------------------------------------------------------
class EnemyNearUpdate : public UpdateModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( EnemyNearUpdate, "EnemyNearUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( EnemyNearUpdate, EnemyNearUpdateModuleData )

public:

	EnemyNearUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual UpdateSleepTime update();

protected:

	UnsignedInt m_enemyScanDelay;
	Bool m_enemyNear;

	void checkForEnemies( void );

};

#endif // end __EnemyNearUpdate_H_

