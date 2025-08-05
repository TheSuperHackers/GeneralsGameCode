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

// FILE: ProneUpdate.h //////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, March 2002
// Desc:   Update module to encapsulate what it means to be "prone"
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __PRONE_UPDATE_H_
#define __PRONE_UPDATE_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/UpdateModule.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Thing;
class DamageInfo;

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class ProneUpdateModuleData : public ModuleData
{
public:
  Real		m_damageToFramesRatio;      ///< Conversion from damage dealt to number of frames we cower

	ProneUpdateModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);

private:

};

//-------------------------------------------------------------------------------------------------
/** The default	update module */
//-------------------------------------------------------------------------------------------------
class ProneUpdate : public UpdateModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( ProneUpdate, "ProneUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( ProneUpdate, ProneUpdateModuleData );

public:

	ProneUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	void goProne( const DamageInfo *damageInfo );

	virtual UpdateSleepTime update();

protected:

	void startProneEffects();
	void stopProneEffects();

	Int m_proneFrames;
};


#endif

