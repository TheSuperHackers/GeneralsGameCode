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

// TileTool.cpp
// Texture tiling tool for worldbuilder.
// Author: John Ahlquist, April 2001

#include "StdAfx.h"
#include "resource.h"

#include "TileTool.h"
#include "CUndoable.h"
#include "MainFrm.h"
#include "WHeightMapEdit.h"
#include "WorldBuilderDoc.h"
#include "WorldBuilderView.h"
#include "BrushTool.h"
#include "DrawObject.h"
//
// TileTool class.
//


/// Constructor
TileTool::TileTool(void) :
	Tool(ID_TILE_TOOL, IDC_TILE_CURSOR)
{
	m_htMapEditCopy = NULL;
}

/// Destructor
TileTool::~TileTool(void)
{
	REF_PTR_RELEASE(m_htMapEditCopy);
}

/// Shows the terrain materials options panel.
void TileTool::activate()
{
	CMainFrame::GetMainFrame()->showOptionsDialog(IDD_TERRAIN_MATERIAL);
	TerrainMaterial::setToolOptions(true);
	DrawObject::setDoBrushFeedback(true);
	DrawObject::setBrushFeedbackParms(true, 1, 0);
}


/// Common mouse down code for left and right clicks.
void TileTool::mouseDown(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	if (m == TRACK_L)
		m_textureClassToDraw = TerrainMaterial::getFgTexClass();
	else if (m == TRACK_R)
		m_textureClassToDraw = TerrainMaterial::getBgTexClass();
	else
		return;

//	WorldHeightMapEdit *pMap = pDoc->GetHeightMap();
	// just in case, release it.
	REF_PTR_RELEASE(m_htMapEditCopy);
	m_htMapEditCopy = pDoc->GetHeightMap()->duplicate();
	m_prevXIndex = -1;
	m_prevYIndex = -1;
	m_prevViewPt = viewPt;
	mouseMoved(m, viewPt, pView, pDoc);
}

/// Common mouse up code for left and right clicks.
void TileTool::mouseUp(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	if (m != TRACK_L && m != TRACK_R) return;

	m_htMapEditCopy->optimizeTiles(); // force to optimize tileset
	IRegion2D partialRange = {0,0,0,0};
	pDoc->updateHeightMap(m_htMapEditCopy, false, partialRange);

	WBDocUndoable *pUndo = new WBDocUndoable(pDoc, m_htMapEditCopy);
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	REF_PTR_RELEASE(m_htMapEditCopy);
}

/// Common mouse moved code for left and right clicks.
void TileTool::mouseMoved(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	Bool didAnything = false;
	Bool fullUpdate = false;
	Coord3D cpt;

	pView->viewToDocCoords(viewPt, &cpt);
	DrawObject::setFeedbackPos(cpt);
	if (m != TRACK_L && m != TRACK_R) return;
	Int dx = m_prevViewPt.x - viewPt.x;
	Int dy = m_prevViewPt.y - viewPt.y;
	Int count = sqrt(dx*dx + dy*dy);
	Int k;

	Int totalMinX = m_htMapEditCopy->getXExtent();
	Int totalMinY = m_htMapEditCopy->getYExtent();
	Int totalMaxX = 0;
	Int totalMaxY = 0;

	count /= 2;
	if (count<1) count = 1;
	for (k=0; k<=count; k++) {
		CPoint curViewPt;
		curViewPt.x = viewPt.x + ((count-k)*dx)/count;
		curViewPt.y = viewPt.y + ((count-k)*dy)/count;

		if (k==0) {
			DEBUG_ASSERTCRASH(curViewPt.x == m_prevViewPt.x, ("Bad x"));
			DEBUG_ASSERTCRASH(curViewPt.y == m_prevViewPt.y, ("Bad y"));
		}
		if (k==count) {
			DEBUG_ASSERTCRASH(curViewPt.x == viewPt.x, ("Bad x"));
			DEBUG_ASSERTCRASH(curViewPt.y == viewPt.y, ("Bad y"));
		}
		CPoint ndx;
		Int width = getWidth();
		pView->viewToDocCoords(curViewPt, &cpt);
		getCenterIndex(&cpt, width, &ndx, pDoc);
		if (m_prevXIndex == ndx.x && m_prevYIndex == ndx.y) continue;

		m_prevXIndex = ndx.x;
		m_prevYIndex = ndx.y;

		Int i, j;
		Int minX = ndx.x - (width)/2;
		Int minY = ndx.y - (width)/2;
		for (i=minX; i<minX+width; i++) {
			if (i<0 || i>=m_htMapEditCopy->getXExtent()) continue;
			for (j=minY; j<minY+width; j++) {
				if (j<0 || j>=m_htMapEditCopy->getYExtent()) continue;
				if (TerrainMaterial::isPaintingPathingInfo()) {
					m_htMapEditCopy->setCliff(i, j, !TerrainMaterial::isPaintingPassable());
				} else {
					if (m_htMapEditCopy->setTileNdx(i, j, m_textureClassToDraw, width==1)) {
						fullUpdate = true;
					}
				}
				didAnything = true;
				if (totalMinX>i) totalMinX = i;
				if (totalMinY>j) totalMinY = j;
				if (totalMaxX<i) totalMaxX = i;
				if (totalMaxY<j) totalMaxY = j;
			}
		}
	}
	if (didAnything) {
		IRegion2D partialRange;
		partialRange.lo.x = totalMinX;
		partialRange.hi.x = totalMaxX+1;
		partialRange.lo.y = totalMinY;
		partialRange.hi.y = totalMaxY+1;
		pDoc->updateHeightMap(m_htMapEditCopy, !fullUpdate, partialRange);
	}
	pView->UpdateWindow();
	m_prevViewPt = viewPt;
}

/*************************************************************************
**                             BigTileTool
***************************************************************************/
Int BigTileTool::m_currentWidth = 0;

/// Constructor
BigTileTool::BigTileTool(void)
{
	m_toolID = ID_BIG_TILE_TOOL;
}

/// Shows the terrain materials options panel.
void BigTileTool::setWidth(Int width)
{
	m_currentWidth = width;
	DrawObject::setBrushFeedbackParms(true, m_currentWidth, 0);
}

/// Shows the terrain materials options panel.
void BigTileTool::activate()
{
	CMainFrame::GetMainFrame()->showOptionsDialog(IDD_TERRAIN_MATERIAL);
	TerrainMaterial::setToolOptions(false);
	TerrainMaterial::setWidth(m_currentWidth);
	DrawObject::setDoBrushFeedback(true);
	DrawObject::setBrushFeedbackParms(true, m_currentWidth, 0);
}
