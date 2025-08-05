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

// FILE: W3DPoly.h /////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
// Project:    RTS3
//
// File name:  W3DPoly.h
//
// Created:    Mark Wilczynski, Jan 2002
//
// Desc:       Generic Polgon operations.
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __W3DPOLY_H_
#define __W3DPOLY_H_

#include "vector3.h"
#include "plane.h"
#include "simplevec.h"

//-------------------------------------------------------------------------------------------------
/**VisPolyClass - This class is used to clip a polygon to a plane.  Useful for manually
	* clipping polys to the frustum or other geometry.  Based on internal WW3D2 code. */
//-------------------------------------------------------------------------------------------------

class ClipPolyClass
{
public:
	void Reset(void);
	void Add_Vertex(const Vector3 & point);
	void Clip(const PlaneClass & plane,ClipPolyClass & dest) const;

	SimpleDynVecClass<Vector3> Verts;
};

#endif //__W3DPOLY_H_
