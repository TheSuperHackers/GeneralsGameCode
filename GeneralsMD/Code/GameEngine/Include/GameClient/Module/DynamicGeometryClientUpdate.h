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

// FILE: DynamicGeometryClientUpdate.h ////////////////////////////////////////////////////////////////////
// Author: Andi W, August 2025
// Desc:  Simple Alpha/Scale transition
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __DynamicGeometryClientUpdate_H_
#define __DynamicGeometryClientUpdate_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/ClientUpdateModule.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class DynamicGeometryClientUpdateModuleData : public ClientUpdateModuleData
{
public:
	Real m_scaleInitial;
	Real m_scaleMidpoint;
	Real m_scaleFinal;

	Real m_alphaInitial;
	Real m_alphaMidpoint;
	Real m_alphaFinal;

	UnsignedInt m_totalFrames;
	UnsignedInt m_midpointFrames;

	UnsignedInt m_interpolationType;

	DynamicGeometryClientUpdateModuleData();
	~DynamicGeometryClientUpdateModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);
};
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class DynamicGeometryClientUpdate : public ClientUpdateModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( DynamicGeometryClientUpdate, "DynamicGeometryClientUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( DynamicGeometryClientUpdate, DynamicGeometryClientUpdateModuleData);

public:

	DynamicGeometryClientUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	/// the client update callback
	virtual void clientUpdate( void );

	void setScaleMultiplier( Real value ) { m_overrideScale = value; }
	void setAlphaMultiplier( Real value ) { m_overrideAlpha = value; }

protected:

	UnsignedInt m_startFrame;
	Real m_overrideScale;
	Real m_overrideAlpha;
};

#endif // __DynamicGeometryClientUpdate_H_

