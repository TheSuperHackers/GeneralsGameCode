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

// FILE: ObjectRepulsorHelper.h ////////////////////////////////////////////////////////////////////////
// Author: Steven Johnson, December 202
// Desc:   Object helper - Repulsor
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __ObjectRepulsorHelper_H_
#define __ObjectRepulsorHelper_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/ObjectHelper.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class ObjectRepulsorHelperModuleData : public ModuleData
{

};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class ObjectRepulsorHelper : public ObjectHelper
{

	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( ObjectRepulsorHelper, ObjectRepulsorHelperModuleData )
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(ObjectRepulsorHelper, "ObjectRepulsorHelper" )

public:

	ObjectRepulsorHelper( Thing *thing, const ModuleData *modData ) : ObjectHelper( thing, modData ) { }
	// virtual destructor prototype provided by memory pool object

	virtual UpdateSleepTime update();

};


#endif  // end __ObjectRepulsorHelper_H_
