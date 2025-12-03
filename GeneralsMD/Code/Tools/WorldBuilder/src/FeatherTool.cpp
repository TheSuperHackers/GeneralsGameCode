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

// FeatherTool.cpp
// Texture tiling tool for worldbuilder.
// Author: John Ahlquist, April 2001

#include "StdAfx.h"
#include "resource.h"

#include "FeatherTool.h"
#include "FeatherOptions.h"
#include "CUndoable.h"
#include "DrawObject.h"
#include "MainFrm.h"
#include "WHeightMapEdit.h"
#include "WorldBuilderDoc.h"
#include "WorldBuilderView.h"
#include "BrushTool.h"

/// Feather tool uses a higher rate multiplier than other brush operations
static const Int FEATHER_RATE_MULTIPLIER = 5;

//
// FeatherTool class.
Int FeatherTool::m_feather = 0;
Int FeatherTool::m_rate = 0;
Int FeatherTool::m_radius = 0;

//
/// Constructor
FeatherTool::FeatherTool(void) :
	Tool(ID_FEATHERTOOL, IDC_BRUSH_CROSS)
{
	m_htMapEditCopy = NULL;
	m_htMapFeatherCopy = NULL;
	m_htMapRateCopy = NULL;
}

/// Destructor
FeatherTool::~FeatherTool(void)
{
	REF_PTR_RELEASE(m_htMapEditCopy);
	REF_PTR_RELEASE(m_htMapFeatherCopy);
	REF_PTR_RELEASE(m_htMapRateCopy);
}



/// Shows the brush options panel.
void FeatherTool::activate()
{
	CMainFrame::GetMainFrame()->showOptionsDialog(IDD_FEATHER_OPTIONS);
	DrawObject::setDoBrushFeedback(true);
	DrawObject::setBrushFeedbackParms(false, m_feather, 0);
}

/// Set the brush feather and notify the height options panel of the change.
void FeatherTool::setFeather(Int feather)
{
	if (m_feather != feather) {
		m_feather = feather;
		// notify feather palette options panel
		FeatherOptions::setFeather(m_feather);
		DrawObject::setBrushFeedbackParms(false, m_feather, 0);
	}
};

/// Set the brush feather and notify the height options panel of the change.
void FeatherTool::setRate(Int rate)
{
	if (m_rate != rate) {
		m_rate = rate;
		// notify feather palette options panel
		FeatherOptions::setRate(rate);
	}
};

/// Set the brush feather and notify the height options panel of the change.
void FeatherTool::setRadius(Int radius)
{
	if (m_radius != radius) {
		m_radius = radius;
		// notify feather palette options panel
		FeatherOptions::setRadius(radius);
	}
};

/// Start tool.
/** Setup the tool to start brushing - make a copy of the height map
to edit, another copy because we need it :), and call mouseMovedDown. */
void FeatherTool::mouseDown(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	if (m != TRACK_L) return;

//	WorldHeightMapEdit *pMap = pDoc->GetHeightMap();
	// just in case, release it.
	REF_PTR_RELEASE(m_htMapEditCopy);
	m_htMapEditCopy = pDoc->GetHeightMap()->duplicate();
	m_htMapFeatherCopy = pDoc->GetHeightMap()->duplicate();
	m_htMapRateCopy = pDoc->GetHeightMap()->duplicate();
	Int size = m_htMapRateCopy->getXExtent() * m_htMapRateCopy->getYExtent();
	UnsignedByte *pData = m_htMapRateCopy->getDataPtr();
	Int i;
	for (i=0; i<size; i++) {
		*pData++ = 0;
	}
	m_prevXIndex = -1;
	m_prevYIndex = -1;
	mouseMoved(m, viewPt, pView, pDoc);
}

/// End tool.
/** Finish the tool operation - create a command, pass it to the
doc to execute, and cleanup ref'd objects. */
void FeatherTool::mouseUp(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	if (m != TRACK_L) return;

	WBDocUndoable *pUndo = new WBDocUndoable(pDoc, m_htMapEditCopy);
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	REF_PTR_RELEASE(m_htMapEditCopy);
	REF_PTR_RELEASE(m_htMapFeatherCopy);
	REF_PTR_RELEASE(m_htMapRateCopy);
}

/// Execute the tool.
/** Smooth the height map at the current point. */
void FeatherTool::mouseMoved(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	Coord3D cpt;
	pView->viewToDocCoords(viewPt, &cpt);
	DrawObject::setFeedbackPos(cpt);
	if (m != TRACK_L) return;

	CPoint ndx;
	getCenterIndex(&cpt, m_feather, &ndx, pDoc);

	m_prevXIndex = ndx.x;
	m_prevYIndex = ndx.y;

	Bool redoRate = BrushTool::applyBrushWithRateAccumulation(
		m_htMapEditCopy, m_htMapFeatherCopy, m_htMapRateCopy,
		ndx, m_feather, m_rate * FEATHER_RATE_MULTIPLIER,
		m_radius, FeatherOptions::MIN_RADIUS, FeatherOptions::MAX_RADIUS, pDoc);

	pDoc->updateHeightMap(m_htMapEditCopy, true, BrushTool::makePartialRange(ndx, m_feather));
	
	if (redoRate) {
		BrushTool::resetSmoothingBuffers(m_htMapEditCopy, m_htMapFeatherCopy, m_htMapRateCopy);
	}
}
