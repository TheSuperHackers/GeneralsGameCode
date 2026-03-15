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

// BlendEdgeTool.cpp
// Texture tiling tool for worldbuilder.
// Author: John Ahlquist, April 2001

#include "StdAfx.h"
#include "resource.h"

#include "BlendEdgeTool.h"
#include "CUndoable.h"
#include "MainFrm.h"
#include "WHeightMapEdit.h"
#include "WorldBuilderDoc.h"
#include "WorldBuilderView.h"
//
// BlendEdgeTool class.
//
/// Constructor
BlendEdgeTool::BlendEdgeTool(void) :
	Tool(ID_BLEND_EDGE_TOOL, IDC_BLEND_EDGE)
{
}

/// Destructor
BlendEdgeTool::~BlendEdgeTool(void)
{
}


/** Execute the tool on mouse down - Place an object. */
void BlendEdgeTool::mouseDown(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	Coord3D cpt;
	pView->viewToDocCoords(viewPt, &cpt);

	m_downPt = cpt;
}

/** Execute the tool on mouse up - Blend an edge.  Left mouse blends the from
texture to the to texture based on your drag. Right mouse blends the
foreground texture in the texture panel on top of the destination tile for
custom blends. */
void BlendEdgeTool::mouseUp(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	CPoint from, to;

	Coord3D cpt;
	// Don't constrain this tool.
	pView->viewToDocCoords(viewPt, &cpt, false);

	if (!pDoc->getCellIndexFromCoord(m_downPt, &from) || !pDoc->getCellIndexFromCoord(cpt, &to)) {
		return;
	}

	if (from.x == to.x && from.y == to.y) {
		return;
	}

//	WorldHeightMapEdit *pMap = pDoc->GetHeightMap();
	WorldHeightMapEdit *htMapEditCopy = pDoc->GetHeightMap()->duplicate();
	if (m == TRACK_L)
		htMapEditCopy->blendTile(to.x, to.y, from.x, from.y, -1, -1);   // does all the work.
	else
		htMapEditCopy->blendTile(to.x, to.y, from.x, from.y, TerrainMaterial::getFgTexClass(), -1);   // does all the work.
	IRegion2D partialRange = {to.x, to.y, to.x+1, to.y+1};
	pDoc->updateHeightMap(htMapEditCopy, true, partialRange);	//update the render object with new tile and/or height data.
	WBDocUndoable *pUndo = new WBDocUndoable(pDoc, htMapEditCopy);
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.

	REF_PTR_RELEASE(htMapEditCopy);

}

