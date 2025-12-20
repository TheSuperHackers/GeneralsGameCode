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

// FILE: CheckpointUpdate.h /////////////////////////////////////////////////////////////////////////////
// Author: 	Matthew D. Campbell, April 2002
// Desc:  Reacts when an enemy is within range
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/UpdateModule.h"
#include "Common/KindOf.h"


//-------------------------------------------------------------------------------------------------
/** Checkpoint update */
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class CheckpointUpdateModuleData : public UpdateModuleData
{
public:
	UnsignedInt m_enemyScanDelayTime;

	CheckpointUpdateModuleData()
	{
		m_enemyScanDelayTime = LOGICFRAMES_PER_SECOND;
	}

	static void buildFieldParse(MultiIniFieldParse& p)
	{
    UpdateModuleData::buildFieldParse(p);
		static const FieldParse dataFieldParse[] =
		{
			{ "ScanDelayTime",		INI::parseDurationUnsignedInt,		NULL, offsetof( CheckpointUpdateModuleData, m_enemyScanDelayTime ) },
			{ 0, 0, 0, 0 }
		};
    p.add(dataFieldParse);
	}
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class CheckpointUpdate : public UpdateModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( CheckpointUpdate, "CheckpointUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( CheckpointUpdate, CheckpointUpdateModuleData )

public:

	CheckpointUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual UpdateSleepTime update();

protected:
	Bool m_enemyNear;
	Bool m_allyNear;
	Real m_maxMinorRadius;

	UnsignedInt m_enemyScanDelay;
	void checkForAlliesAndEnemies( void );

};
