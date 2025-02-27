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

// BrushTool.cpp
// Texture tiling tool for worldbuilder.
// Author: John Ahlquist, April 2001

#include "StdAfx.h"
#include "resource.h"

#include "BrushTool.h"
#include "CUndoable.h"
#include "MainFrm.h"
#include "WHeightMapEdit.h"
#include "WorldBuilderDoc.h"
#include "WorldBuilderView.h"
#include "brushoptions.h"
#include "DrawObject.h"
//
// BrushTool class.
//

Int BrushTool::m_brushWidth;
Int BrushTool::m_brushFeather;
Bool BrushTool::m_brushSquare;
Int BrushTool::m_brushHeight;



/// Constructor
BrushTool::BrushTool(void) :
	Tool(ID_BRUSH_TOOL, IDC_BRUSH_CROSS)
{
	m_htMapEditCopy = NULL;
	m_htMapFeatherCopy = NULL;

	m_brushWidth = 0;
	m_brushFeather = 0;
	m_brushHeight = 0;
	m_brushSquare = false;
}

/// Destructor
BrushTool::~BrushTool(void)
{
	REF_PTR_RELEASE(m_htMapEditCopy);
	REF_PTR_RELEASE(m_htMapFeatherCopy);
}

/// Set the brush height and notify the height options panel of the change.
void BrushTool::setHeight(Int height)
{
	if (m_brushHeight != height) {
		m_brushHeight = height;
		// notify height palette options panel
		BrushOptions::setHeight(height);
	}
};

/// Set the brush width and notify the height options panel of the change.
void BrushTool::setWidth(Int width)
{
	if (m_brushWidth != width) {
		m_brushWidth = width;
		// notify brush palette options panel
		BrushOptions::setWidth(width);
		DrawObject::setBrushFeedbackParms(m_brushSquare, m_brushWidth, m_brushFeather);
	}
};

/// Set the brush feather and notify the height options panel of the change.
void BrushTool::setFeather(Int feather)
{
	if (m_brushFeather != feather) {
		m_brushFeather = feather;
		// notify height palette options panel
		BrushOptions::setFeather(feather);
		DrawObject::setBrushFeedbackParms(m_brushSquare, m_brushWidth, m_brushFeather);
	}
};

/// Shows the brush options panel.
void BrushTool::activate()
{
	CMainFrame::GetMainFrame()->showOptionsDialog(IDD_BRUSH_OPTIONS);
	DrawObject::setDoBrushFeedback(true);
	DrawObject::setBrushFeedbackParms(m_brushSquare, m_brushWidth, m_brushFeather);
}

/// Start tool.
/** Setup the tool to start brushing - make a copy of the height map
to edit, another copy because we need it :), and call mouseMovedDown. */
void BrushTool::mouseDown(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	if (m != TRACK_L) return;

//	WorldHeightMapEdit *pMap = pDoc->GetHeightMap();
	// just in case, release it.
	REF_PTR_RELEASE(m_htMapEditCopy);
	m_htMapEditCopy = pDoc->GetHeightMap()->duplicate();
	m_prevXIndex = -1;
	m_prevYIndex = -1;
	REF_PTR_RELEASE(m_htMapFeatherCopy);
	m_htMapFeatherCopy = m_htMapEditCopy->duplicate();
	mouseMoved(m, viewPt, pView, pDoc);
}

/// End tool.
/** Finish the tool operation - create a command, pass it to the
doc to execute, and cleanup ref'd objects. */
void BrushTool::mouseUp(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	if (m != TRACK_L) return;

	WBDocUndoable *pUndo = new WBDocUndoable(pDoc, m_htMapEditCopy);
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	REF_PTR_RELEASE(m_htMapEditCopy);
	REF_PTR_RELEASE(m_htMapFeatherCopy);
}

/// Execute the tool.
/** Apply the height brush at the current point. */
void BrushTool::mouseMoved(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	Coord3D cpt;
	pView->viewToDocCoords(viewPt, &cpt);
	DrawObject::setFeedbackPos(cpt);

	if (m != TRACK_L) return;

	pView->viewToDocCoords(viewPt, &cpt);

	int brushWidth = m_brushWidth;
	if (m_brushFeather>0) {
		brushWidth += 2*m_brushFeather;
	}
	brushWidth += 2;

	CPoint ndx;
	getCenterIndex(&cpt, m_brushWidth, &ndx, pDoc);

	if (m_prevXIndex == ndx.x && m_prevYIndex == ndx.y) return;

	m_prevXIndex = ndx.x;
	m_prevYIndex = ndx.y;

	int sub = brushWidth/2;
	int add = brushWidth-sub;

	Int i, j;
	for (i=ndx.x-sub; i<ndx.x+add; i++) {
		if (i<0 || i>=m_htMapEditCopy->getXExtent()) {
			continue;
		}
		for (j=ndx.y-sub; j<ndx.y+add; j++) {
			if (j<0 || j>=m_htMapEditCopy->getYExtent()) {
				continue;
			}
			Real blendFactor;
			if (m_brushSquare) {
				blendFactor = calcSquareBlendFactor(ndx, i, j, m_brushWidth, m_brushFeather);
			} else {
				blendFactor = calcRoundBlendFactor(ndx, i, j, m_brushWidth, m_brushFeather);
			}
			Int curHeight = m_htMapFeatherCopy->getHeight(i,j);
			float fNewHeight = blendFactor*m_brushHeight+((1.0f-blendFactor)*curHeight) ;
			Int newHeight = floor(fNewHeight+0.5);
			if (m_brushHeight > curHeight) {
				if (m_htMapEditCopy->getHeight(i,j)>newHeight) {
					newHeight = m_htMapEditCopy->getHeight(i,j);
				}
			} else {
				if (m_htMapEditCopy->getHeight(i,j)<newHeight) {
					newHeight = m_htMapEditCopy->getHeight(i,j);
				}
			}
			m_htMapEditCopy->setHeight(i, j, newHeight);
		}
	}
	IRegion2D partialRange;
	partialRange.lo.x = ndx.x - brushWidth;
	partialRange.hi.x = ndx.x + brushWidth;
	partialRange.lo.y = ndx.y - brushWidth;
	partialRange.hi.y = ndx.y + brushWidth;
	pDoc->updateHeightMap(m_htMapEditCopy, true, partialRange);
}
