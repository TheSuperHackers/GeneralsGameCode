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

// FILE: W3DDependencyModelDraw.h ////////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, October 2002
// Desc: Draw module just like Model, except it can't draw unless somebody else explicitly says to, since they
// have to draw first.
//
// This draw module can be used in a general case (although I don't see why), m_attachToDrawableBoneInContainer
// is for the one present and main reason to use this module.  Our transport needs to tell us it is okay to
// draw after he draws.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "W3DDevice/GameClient/Module/W3DModelDraw.h"

//-------------------------------------------------------------------------------------------------
class W3DDependencyModelDrawModuleData : public W3DModelDrawModuleData
{
public:
	AsciiString	m_attachToDrawableBoneInContainer;// Not just a different draw module, this bone is in our container

	W3DDependencyModelDrawModuleData();
	~W3DDependencyModelDrawModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);
};

//-------------------------------------------------------------------------------------------------
class W3DDependencyModelDraw : public W3DModelDraw
{

 	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( W3DDependencyModelDraw, "W3DDependencyModelDraw" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( W3DDependencyModelDraw, W3DDependencyModelDrawModuleData )

public:

	W3DDependencyModelDraw( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration
	virtual void doDrawModule(const Matrix3D* transformMtx);
	virtual void notifyDrawModuleDependencyCleared( );///< if you were waiting for something before you drew, it's ready now
	virtual void adjustTransformMtx(Matrix3D& mtx) const;

protected:
	Bool m_dependencyCleared; // The thing we depend on will clear this, and we will relatch it after we draw.
};
