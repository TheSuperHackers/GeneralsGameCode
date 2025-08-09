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

// FILE: FloatUpdate.h ////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, May 2002
// Desc:   Floting on water update
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __FLOATUPDATE_H_
#define __FLOATUPDATE_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/UpdateModule.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
struct FieldParse;

//-------------------------------------------------------------------------------------------------
class FloatUpdateModuleData: public UpdateModuleData
{

public:

	FloatUpdateModuleData( void );

	static void buildFieldParse(MultiIniFieldParse& p);

	Bool m_enabled;							///< enabled

};

//-------------------------------------------------------------------------------------------------
class FloatUpdate : public UpdateModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( FloatUpdate, "FloatUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( FloatUpdate, FloatUpdateModuleData )

public:

	FloatUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	void setEnabled( Bool enabled ) { m_enabled = enabled; }  ///< enable/disable floating

	virtual UpdateSleepTime update();	///< Deciding whether or not to make new guys

protected:


	Bool m_enabled;			///< enabled

};

#endif  // end __FLOATUPDATE_H_
