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

// FILE: RadarUpdate.h ////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, April 2002
// Desc:   Updating a radar on an object
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/UpdateModule.h"

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class RadarUpdateModuleData : public UpdateModuleData
{

public:

	RadarUpdateModuleData( void );

	static void buildFieldParse(MultiIniFieldParse& p)
	{
    UpdateModuleData::buildFieldParse( p );

		static const FieldParse dataFieldParse[] =
		{

			{ "RadarExtendTime", INI::parseDurationReal, nullptr, offsetof( RadarUpdateModuleData, m_radarExtendTime ) },
			{ 0, 0, 0, 0 }
		};
    p.add(dataFieldParse);

	}

  Real m_radarExtendTime;  ///< in frames, time it takes the radar tower to build

};

//-------------------------------------------------------------------------------------------------
/** The Radar Update module */
//-------------------------------------------------------------------------------------------------
class RadarUpdate : public UpdateModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( RadarUpdate, "RadarUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( RadarUpdate, RadarUpdateModuleData );

public:

	RadarUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype defined by MemoryPoolObject

	void extendRadar( void );									 ///< extend the radar from this object
	Bool isRadarActive() { return m_radarActive; }

	virtual UpdateSleepTime update( void ); ///< Here's the actual work of Upgrading

protected:

	UnsignedInt m_extendDoneFrame;						 ///< radar will be done extending on this frame
	Bool m_extendComplete;										 ///< TRUE when extend is all done
	Bool m_radarActive;												 ///< TRUE when radar is actually online and generating radar information.

};
