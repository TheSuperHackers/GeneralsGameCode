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

// HandScrollTool.cpp
// Scrolling tool for worldbuilder.
// Author: John Ahlquist, April 2001

#include "StdAfx.h"
#include "resource.h"

#include "HandScrollTool.h"
#include "CUndoable.h"
#include "MainFrm.h"
#include "WHeightMapEdit.h"
#include "WorldBuilder.h"
#include "WorldBuilderDoc.h"
#include "WorldBuilderView.h"
//
// HandScrollTool class.
//

inline Int IABS(Int x) {	if (x>=0) return x; return -x;};
static const Int MAX_SCROLL = 1000;

/// Constructor
HandScrollTool::HandScrollTool(void) :
	Tool(ID_HAND_SCROLL_TOOL, IDC_HAND_SCROLL)
{
}

/// Shows the terrain materials options panel.
void HandScrollTool::activate()
{
	return; // Hand scroll tool intentionally doesn't change tool panel.
}

/// Destructor
HandScrollTool::~HandScrollTool(void)
{
}

void HandScrollTool::mouseDown(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	if (m != TRACK_L && m != TRACK_R && m != TRACK_M) return;

//	pView->viewToDocCoords(viewPt, &m_prevPt);
	m_prevPt2d = viewPt;
	m_downPt2d = viewPt;
	m_scrolling = false;
	// Save the start point.
	m_mouseDownTime = ::GetTickCount();
}

/// Left button move code.
void HandScrollTool::mouseMoved(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	if (m == TRACK_NONE)
		return;

	if (m == TRACK_M) {

		// camera rotation
		const Real factor = 0.01f;
		Real rot = factor * (viewPt.x - m_prevPt2d.x);
		/*
		if (pView->isDoingPitch())
			pView->pitchCamera(rot);
		else
		*/
			pView->rotateCamera(rot);
		m_prevPt2d = viewPt;

	} else if (m == TRACK_L || m == TRACK_R) {
		if (!m_scrolling) {
			// see if we moved enough to start scrolling.
			if (abs(viewPt.x - m_downPt2d.x) > HYSTERESIS) m_scrolling = true;
			if (abs(viewPt.y - m_downPt2d.y) > HYSTERESIS) m_scrolling = true;
		}
		if (!m_scrolling) {
			return;
		}
		// Scroll dynamically.
		Coord3D prev, cur;
		if (pView->viewToDocCoords(m_prevPt2d, &prev, false) &&
					pView->viewToDocCoords(viewPt, &cur, false))
		{

			Real dx = cur.x - prev.x;
			Real dy = cur.y - prev.y;
			if (IABS(dx)>MAX_SCROLL) {
				dx = 0;
			}
			if (IABS(dy)>MAX_SCROLL) {
				dy = 0;
			}
			dx /= MAP_XY_FACTOR;
			dy /= MAP_XY_FACTOR;
			pView->scrollInView(dx, dy, false);

			m_prevPt2d = viewPt;
		}
	}
}

void HandScrollTool::mouseUp(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	if (m == TRACK_M) {
		// if haven't moved, reset view
		Bool moved = false;
		if (abs(viewPt.x - m_downPt2d.x) > HYSTERESIS) moved = true;
		if (abs(viewPt.y - m_downPt2d.y) > HYSTERESIS) moved = true;

		if (!moved && GetTickCount() - m_mouseDownTime < ::GetDoubleClickTime())
		{
			pView->setDefaultCamera();
		}
	} else if (m == TRACK_L || m == TRACK_R) {
		if (m_scrolling) {
			// Tell the view we are done scrolling.
			pView->scrollInView(0,0,true);
		} else if (m==TRACK_R) {
			// Clicked right.  Deselect & go to pointer.
			WbApp()->selectPointerTool();
		}
	}
}

