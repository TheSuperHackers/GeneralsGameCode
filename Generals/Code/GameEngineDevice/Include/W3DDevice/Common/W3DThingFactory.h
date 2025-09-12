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

// FILE: W3DThingFactory.h ////////////////////////////////////////////////////////////////////////
// Author: Colin Day, February 2002
// Desc:   Device dependent thing factory access, for things like post processing the
//				 Thing database where we might want to look at device dependent stuff like
//				 model info and such
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __W3DTHINGFACTORY_H_
#define __W3DTHINGFACTORY_H_

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "Common/ThingFactory.h"

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class W3DThingFactory : public ThingFactory
{

public:

	W3DThingFactory( void );
	virtual ~W3DThingFactory( void );
};

#endif // __W3DTHINGFACTORY_H_

