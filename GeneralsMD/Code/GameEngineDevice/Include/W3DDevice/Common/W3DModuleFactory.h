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

// FILE: W3DModuleFactory.h ///////////////////////////////////////////////////////////////////////
// Author: Steven Johnson, September 2001
//				 Colin Day, November 2001
// Desc:   W3D game logic class, there shouldn't be a lot of new functionality
//				 in this class, but there are certain things that need to have close
//				 knowledge of each other like ther logical and visual terrain
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/ModuleFactory.h"

//-------------------------------------------------------------------------------------------------
/** W3D specific functionality for the module factory */
//-------------------------------------------------------------------------------------------------
class W3DModuleFactory : public ModuleFactory
{

public:

	virtual void init( void );

};
