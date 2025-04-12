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

// FILE: W3DScienceModelDraw.h ////////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, NOVEMBER 2002
// Desc: Draw module just like Model, except it only draws if the local player has the specified science
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef _W3D_SCIENCE_MODEL_DRAW_H_
#define _W3D_SCIENCE_MODEL_DRAW_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "W3DDevice/GameClient/Module/W3DModelDraw.h"

enum ScienceType CPP_11(: Int);

//-------------------------------------------------------------------------------------------------
class W3DScienceModelDrawModuleData : public W3DModelDrawModuleData
{
public:
	ScienceType m_requiredScience; ///< Local player must have this science for me to ever draw

	W3DScienceModelDrawModuleData();
	~W3DScienceModelDrawModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);
};

//-------------------------------------------------------------------------------------------------
class W3DScienceModelDraw : public W3DModelDraw
{

 	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( W3DScienceModelDraw, "W3DScienceModelDraw" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( W3DScienceModelDraw, W3DScienceModelDrawModuleData )
		
public:

	W3DScienceModelDraw( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration
	virtual void doDrawModule(const Matrix3D* transformMtx);///< checks a property on the local player before passing this up

protected:
};

#endif

