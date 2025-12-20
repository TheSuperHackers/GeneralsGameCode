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

// MeshMoldTool.h
// Terrain shaping tools for worldbuilder.
// Author: John Ahlquist, Oct. 2001

#pragma once

#include "Tool.h"
class WorldHeightMapEdit;
/*************************************************************************/
/**                             MeshMoldTool
	 Does the Mesh Mold tool operation.
***************************************************************************/
///  Height brush tool.
class MeshMoldTool : public Tool
{
protected:

	CPoint	m_prevViewPt;	 ///< Previous tracking location in view coords.
	Bool		m_offsettingZ; ///< True if we are raising or lowering the mesh along z axis.
	Coord3D m_prevDocPt;	 ///< Previous tracking location in doc coords.
	static Coord3D m_toolPos;     ///< Current location of the mesh on the terrain.
	static Bool		m_tracking;    ///< True if we are tracking a mesh on screen.
	static WorldHeightMapEdit *m_htMapEditCopy; ///< ref counted.

public:
	MeshMoldTool(void);
	~MeshMoldTool(void);
protected:
	static void applyMesh(CWorldBuilderDoc *pDoc);  ///< Apply the mesh to copy of terrain.

public:
	static void apply(CWorldBuilderDoc *pDoc);  ///< Apply the mesh to the terrain & execute undoable.

public:  //Tool methods.
	virtual void mouseDown(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual void mouseUp(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual void mouseMoved(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual WorldHeightMapEdit *getHeightMap(void) {return m_htMapEditCopy;};
	virtual void activate(); ///< Become the current tool.
	virtual void deactivate(); ///< Become the current tool.
	virtual Bool followsTerrain(void) {return false;};

public:	// Methods specific to MeshMoldTool.
	static void updateMeshLocation(Bool changePreview);

};
