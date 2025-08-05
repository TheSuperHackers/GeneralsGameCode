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

// FILE: ClientUpdateModule.h /////////////////////////////////////////////////////////////////////////////////
// Author:
// Desc:
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __ClientUpdateModule_H_
#define __ClientUpdateModule_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>

#include "Common/Module.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////

// TYPES //////////////////////////////////////////////////////////////////////////////////////////

typedef ModuleData ClientUpdateModuleData;

//-------------------------------------------------------------------------------------------------
/** DRAWABLE CLIENT UPDATE MODULE base class */
//-------------------------------------------------------------------------------------------------
class ClientUpdateModule : public DrawableModule
{

	MEMORY_POOL_GLUE_ABC( ClientUpdateModule )

public:

	ClientUpdateModule( Thing *thing, const ModuleData* moduleData );
	static ModuleType getModuleType() { return MODULETYPE_CLIENT_UPDATE; }
	static Int getInterfaceMask() { return MODULEINTERFACE_CLIENT_UPDATE; }

	virtual void clientUpdate() = 0;

};
inline ClientUpdateModule::ClientUpdateModule( Thing *thing, const ModuleData* moduleData ) : DrawableModule( thing, moduleData ) { }
inline ClientUpdateModule::~ClientUpdateModule() { }
//-------------------------------------------------------------------------------------------------


#endif // __ClientUpdateModule_H_

