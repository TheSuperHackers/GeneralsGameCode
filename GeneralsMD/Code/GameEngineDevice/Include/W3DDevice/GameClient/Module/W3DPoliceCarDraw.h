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

// FILE: W3DPoliceCarDraw.h ///////////////////////////////////////////////////////////////////////
// Author: 
// Desc:   
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __W3DPOLICECARDRAW_H_
#define __W3DPOLICECARDRAW_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/DrawModule.h"
#include "W3DDevice/GameClient/Module/W3DTruckDraw.h"
#include "W3DDevice/GameClient/W3DDynamicLight.h"
#include "WW3D2/line3d.h"

//-------------------------------------------------------------------------------------------------
/** W3D police car draw */
//-------------------------------------------------------------------------------------------------
class W3DPoliceCarDraw : public W3DTruckDraw
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( W3DPoliceCarDraw, "W3DPoliceCarDraw" )
	MAKE_STANDARD_MODULE_MACRO( W3DPoliceCarDraw )

public:

	W3DPoliceCarDraw( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual void doDrawModule(const Matrix3D* transformMtx);

protected:

	/// create the dynamic light for the search light
	W3DDynamicLight *createDynamicLight( void );

	W3DDynamicLight *m_light;  ///< light for the POLICECAR
	Real					m_curFrame;

};

#endif // __W3DPOLICECARDRAW_H_

