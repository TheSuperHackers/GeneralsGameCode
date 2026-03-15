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

// MoundTool.cpp
// Texture tiling tool for worldbuilder.
// Author: John Ahlquist, April 2001


#include "StdAfx.h"
#include "resource.h"

#include "MoundTool.h"
#include "CUndoable.h"
#include "DrawObject.h"
#include "MainFrm.h"
#include "WHeightMapEdit.h"
#include "WorldBuilderDoc.h"
#include "WorldBuilderView.h"
#include "MoundOptions.h"
//
// MoundTool class.
//

Int MoundTool::m_moundHeight=0;
Int MoundTool::m_brushWidth;
Int MoundTool::m_brushFeather;



/// Constructor
MoundTool::MoundTool(void) :
	Tool(ID_BRUSH_ADD_TOOL, IDC_BRUSH_CROSS)
{
	m_htMapEditCopy = nullptr;
	m_htMapSaveCopy = nullptr;
	m_raising = true;
}

/// Destructor
MoundTool::~MoundTool(void)
{
	REF_PTR_RELEASE(m_htMapEditCopy);
	REF_PTR_RELEASE(m_htMapSaveCopy);
}


void MoundTool::setMoundHeight(Int height)
{
	if (m_moundHeight != height) {
		m_moundHeight = height;
		// notify mound options panel
		MoundOptions::setHeight(height);
		DrawObject::setBrushFeedbackParms(false, m_brushWidth, m_brushFeather);
	}
};
/// Set the brush width and notify the height options panel of the change.
void MoundTool::setWidth(Int width)
{
	if (m_brushWidth != width) {
		m_brushWidth = width;
		// notify brush palette options panel
		MoundOptions::setWidth(width);
		DrawObject::setBrushFeedbackParms(false, m_brushWidth, m_brushFeather);
	}
};

/// Set the brush feather and notify the height options panel of the change.
void MoundTool::setFeather(Int feather)
{
	if (m_brushFeather != feather) {
		m_brushFeather = feather;
		// notify height palette options panel
		MoundOptions::setFeather(feather);
		DrawObject::setBrushFeedbackParms(false, m_brushWidth, m_brushFeather);
	}
};


/// Shows the brush options panel.
void MoundTool::activate()
{
	CMainFrame::GetMainFrame()->showOptionsDialog(IDD_MOUND_OPTIONS);
	DrawObject::setDoBrushFeedback(true);
	DrawObject::setBrushFeedbackParms(false, m_brushWidth, m_brushFeather);
}

void MoundTool::mouseDown(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	if (m != TRACK_L) return;

//	WorldHeightMapEdit *pMap = pDoc->GetHeightMap();
	// just in case, release it.
	REF_PTR_RELEASE(m_htMapEditCopy);
	m_htMapEditCopy = pDoc->GetHeightMap()->duplicate();
	m_prevXIndex = -1;
	m_prevYIndex = -1;
	REF_PTR_RELEASE(m_htMapSaveCopy);
	m_htMapSaveCopy = m_htMapEditCopy->duplicate();
	m_lastMoveTime = ::GetTickCount();
	m_lastMoveTime -= MIN_DELAY_TIME + 1; // Make the tool fire the first time.
	mouseMoved(m, viewPt, pView, pDoc);
}

void MoundTool::mouseUp(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	if (m != TRACK_L) return;

	WBDocUndoable *pUndo = new WBDocUndoable(pDoc, m_htMapEditCopy);
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	REF_PTR_RELEASE(m_htMapEditCopy);
	REF_PTR_RELEASE(m_htMapSaveCopy);
}

void MoundTool::mouseMoved(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	Coord3D cpt;
	pView->viewToDocCoords(viewPt, &cpt);
	DrawObject::setFeedbackPos(cpt);
	if (m != TRACK_L) return;

	Int curTime	= ::GetTickCount();
	Int deltaTime = curTime - m_lastMoveTime;
	if (deltaTime < MIN_DELAY_TIME) return;
	m_lastMoveTime = curTime;


	int brushWidth = m_brushWidth;
	int setFeather = m_brushFeather;
	if (setFeather>0) {
		brushWidth += 2*setFeather;
		brushWidth += 2; // for round brush.
	}

	CPoint ndx;
	getCenterIndex(&cpt, m_brushWidth, &ndx, pDoc);

	// do nothing if in the same cell....
	// if (m_prevXIndex == ndx.x && m_prevYIndex == ndx.y) return;

	m_prevXIndex = ndx.x;
	m_prevYIndex = ndx.y;

	int sub = brushWidth/2;
	int add = brushWidth-sub;

	Int htDelta = m_moundHeight;
	if (!m_raising) htDelta = -m_moundHeight;
	// round brush
	Int i, j;
	for (i=ndx.x-sub; i<ndx.x+add; i++) {
		if (i<0 || i>=m_htMapEditCopy->getXExtent()) {
			continue;
		}
		for (j=ndx.y-sub; j<ndx.y+add; j++) {
			if (j<0 || j>=m_htMapEditCopy->getYExtent()) {
				continue;
			}
#if 1
			// New floating point based blending calculation.  jba.
			Real blendFactor;
			blendFactor = calcRoundBlendFactor(ndx, i, j, m_brushWidth, m_brushFeather);
			Int curHeight = m_htMapEditCopy->getHeight(i,j);
			float fNewHeight = (blendFactor*(htDelta+curHeight))+((1.0f-blendFactor)*curHeight);
			Int newHeight = floor(fNewHeight+0.5f);

			// check boundary values
			if (newHeight < m_htMapSaveCopy->getMinHeightValue()) newHeight = m_htMapSaveCopy->getMinHeightValue();
			if (newHeight > m_htMapSaveCopy->getMaxHeightValue()) newHeight = m_htMapSaveCopy->getMaxHeightValue();

			m_htMapEditCopy->setHeight(i, j, newHeight);
			pDoc->invalCell(i, j);
#else
			// Previous integer calculated mounding and blending.
			// If not re-enabled by Dec 2001, delete as obsolete.
			Int xd = abs( (2*(i-(ndx.x-sub)))+1 - brushWidth);
			Int yd = abs( (2*(j-(ndx.y-sub)))+1 - brushWidth);

			float delta = (float)sqrt(xd*xd+yd*yd);
			//Int curHeight = m_htMapSaveCopy->getHeight(i,j);
			Int curHeight = m_htMapEditCopy->getHeight(i,j);
			Int newHeight = curHeight + htDelta;

			// check boundary values
			if (newHeight < m_htMapSaveCopy->getMinHeightValue()) newHeight = m_htMapSaveCopy->getMinHeightValue();
			if (newHeight > m_htMapSaveCopy->getMaxHeightValue()) newHeight = m_htMapSaveCopy->getMaxHeightValue();

			if (delta<m_brushWidth) {
				m_htMapEditCopy->setHeight(i, j, newHeight);
				pDoc->invalCell(i, j);
			} else if (delta<brushWidth+0.7 && setFeather) {
				Int factor = setFeather+1;
				factor *= 2;
				float feather = delta-m_brushWidth;
				float fNewHeight = ((factor-feather)*(htDelta+curHeight)+(feather*curHeight) )/factor;
				newHeight = (Int)(fNewHeight);
				if (newHeight < m_htMapSaveCopy->getMinHeightValue()) newHeight = m_htMapSaveCopy->getMinHeightValue();
				if (newHeight > m_htMapSaveCopy->getMaxHeightValue()) newHeight = m_htMapSaveCopy->getMaxHeightValue();
				if (newHeight > m_htMapEditCopy->getHeight(i,j)) {
					if (htDelta<0) {
						newHeight = m_htMapEditCopy->getHeight(i,j);
					}
				} else {
					if (htDelta>0) {
						newHeight = m_htMapEditCopy->getHeight(i,j);
					}
				}

				m_htMapEditCopy->setHeight(i, j, newHeight);
				pDoc->invalCell(i, j);
			}
#endif
		}
	}

	IRegion2D partialRange;
	partialRange.lo.x = ndx.x - brushWidth;
	partialRange.hi.x = ndx.x + brushWidth;
	partialRange.lo.y = ndx.y - brushWidth;
	partialRange.hi.y = ndx.y + brushWidth;
	pDoc->updateHeightMap(m_htMapEditCopy, true, partialRange);
}

/*************************************************************************
**                             DigTool
***************************************************************************/
/// Constructor
DigTool::DigTool(void)
{
	m_toolID = ID_BRUSH_SUBTRACT_TOOL;
	m_raising = false;  // digging.
}

