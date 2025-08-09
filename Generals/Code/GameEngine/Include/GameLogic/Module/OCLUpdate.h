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

// FILE: OCLUpdate.h /////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, August2002
// Desc:   Update Spits out an OCL on a timer
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __OCL_UPDATE_H_
#define __OCL_UPDATE_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/UpdateModule.h"

class ObjectCreationList;

//-------------------------------------------------------------------------------------------------
class OCLUpdateModuleData : public UpdateModuleData
{
public:
	const ObjectCreationList *m_ocl;
	UnsignedInt m_minDelay;
	UnsignedInt m_maxDelay;
	Bool m_isCreateAtEdge; ///< Otherwise, it is created on top of myself

	OCLUpdateModuleData();

	static void buildFieldParse(MultiIniFieldParse& p);

private:

};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class OCLUpdate : public UpdateModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( OCLUpdate, "OCLUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( OCLUpdate, OCLUpdateModuleData )

public:

	OCLUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual UpdateSleepTime update();

	Real getCountdownPercent() const; ///< goes from 0% to 100%
	UnsignedInt getRemainingFrames() const; ///< For feedback display

protected:

	UnsignedInt m_nextCreationFrame;
	UnsignedInt m_timerStartedFrame;

	Bool shouldCreate();
	void setNextCreationFrame();

};

#endif

