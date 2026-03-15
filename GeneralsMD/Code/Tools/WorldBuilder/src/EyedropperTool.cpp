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

// EyedropperTool.cpp
// Texture tiling tool for worldbuilder.
// Author: John Ahlquist, April 2001

#include "StdAfx.h"
#include "resource.h"

#include "EyedropperTool.h"
#include "TerrainMaterial.h"
#include "WHeightMapEdit.h"
#include "WorldBuilderDoc.h"
#include "WorldBuilderView.h"
#include "MainFrm.h"
#include "DrawObject.h"

//
// EyedropperTool class.
//
/// Constructor
EyedropperTool::EyedropperTool(void) :
	Tool(ID_EYEDROPPER_TOOL, IDC_EYEDROPPER)
{
}

/// Destructor
EyedropperTool::~EyedropperTool(void)
{
}


/// Shows the terrain materials options panel.
void EyedropperTool::activate()
{
	CMainFrame::GetMainFrame()->showOptionsDialog(IDD_TERRAIN_MATERIAL);
	TerrainMaterial::setToolOptions(true);
	DrawObject::setDoBrushFeedback(false);
}

/// Perform the tool behavior on mouse down.
/** Finds the current texture class, and tells the terrain material panel to use it as fg. */
void EyedropperTool::mouseDown(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	if (m != TRACK_L) return;

	Coord3D cpt;
	pView->viewToDocCoords(viewPt, &cpt);

	CPoint ndx;
	if (!pDoc->getCellIndexFromCoord(cpt, &ndx))
		return;

	WorldHeightMapEdit *pMap = pDoc->GetHeightMap();
	Int texClass = pMap->getTextureClass(ndx.x, ndx.y, true);
	TerrainMaterial::setFgTexClass(texClass);
}

