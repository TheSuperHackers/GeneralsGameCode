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

#include "StdAfx.h"
#include "resource.h"

#include "BorderTool.h"
#include "DrawObject.h"
#include "MainFrm.h"
#include "wbview3d.h"
#include "WorldBuilderDoc.h"
#include "CUndoable.h"

const long BOUNDARY_PICK_DISTANCE = 5.0f;
static Bool g_messagePopped = false;

BorderTool::BorderTool() : Tool(ID_BORDERTOOL, IDC_POINTER), 
													 m_mouseDown(false),
													 m_addingNewBorder(false), 
													 m_lastBoundaryPosValid(false), 
													 m_modifyBorderNdx(-1)

{ }

BorderTool::~BorderTool()
{

}

void BorderTool::setCursor(void)
{

}

void BorderTool::activate()
{
	CMainFrame::GetMainFrame()->showOptionsDialog(IDD_NO_OPTIONS);
	DrawObject::setDoBoundaryFeedback(TRUE);

	// Check for overlapping borders
	bool overlapFound = false;
	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	if (pDoc) {
		const int count = pDoc->getNumBoundaries();
		for (int i = 0; i < count; ++i) {
			ICoord2D a;
			pDoc->getBoundary(i, &a);
			for (int j = i + 1; j < count; ++j) {
				ICoord2D b;
				pDoc->getBoundary(j, &b);
				if (a.x == b.x && a.y == b.y) {
					overlapFound = true;
					break;
				}
			}
			if (overlapFound)
				break;
		}
	}

	if (overlapFound) {
		AfxMessageBox(
			"Warning: Two or more map boundaries are overlapping. This may cause unexpected behavior.",
			MB_ICONWARNING | MB_OK
		);
	}

	if (!g_messagePopped) {
		AfxMessageBox(
			"Warning: The Border Tool has been in beta since release build 0.8. Please back up your map before using it.\n\n"
			"The WorldBuilder 3.0 fan update introduces an experimental undo system, "
			"which is not supported in the original version. This allows you to undo mistakenly placed borders.\n\n"
			"Note: The undo system shares the same stack as the main editor. "
			"If you perform additional actions and exceed the undo limit (maximum 15), "
			"you may lose the ability to undo border changes. For this reason, we recommend using this tool only when your map is at least 80% complete.",
			MB_ICONWARNING | MB_OK
		);
		g_messagePopped = true;
	}
}

void BorderTool::deactivate()
{
	WbView3d *p3View = CWorldBuilderDoc::GetActive3DView();
	DrawObject::setDoBoundaryFeedback(p3View->getShowMapBoundaryFeedback());
}

void BorderTool::mouseMoved(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	if (m != TRACK_L) {
		return;
	}

	if (m_addingNewBorder) {
		Int count = pDoc->getNumBoundaries();
		ICoord2D current;
		pDoc->getBoundary(count - 1, &current);
		Coord3D new3DPoint;
		pView->viewToDocCoords(viewPt, &new3DPoint, false);

		if (current.x < 0) {
			current.x = 0;
		}

		if (current.y < 0) {
			current.y = 0;
		}

		current.x = REAL_TO_INT((new3DPoint.x / MAP_XY_FACTOR) + 0.5f);
		current.y = REAL_TO_INT((new3DPoint.y / MAP_XY_FACTOR) + 0.5f);

		m_lastBoundaryPos = current; // save last position
		m_lastBoundaryPosValid = true;
		pDoc->changeBoundary(count - 1, &current);
		return;
	}

	if (m_modifyBorderNdx >= 0) {
		ICoord2D currentBorder;
		pDoc->getBoundary(m_modifyBorderNdx, &currentBorder);

		Coord3D new3DPoint;
		pView->viewToDocCoords(viewPt, &new3DPoint, false);

		switch (m_modificationType)
		{
			case MOD_TYPE_INVALID: m_modifyBorderNdx = -1; return;
			case MOD_TYPE_UP:	
				currentBorder.y = REAL_TO_INT((new3DPoint.y / MAP_XY_FACTOR) + 0.5f); 
				break;
			case MOD_TYPE_RIGHT: 
				currentBorder.x = REAL_TO_INT((new3DPoint.x / MAP_XY_FACTOR) + 0.5f); 
				break;
			case MOD_TYPE_FREE: 
				currentBorder.x = REAL_TO_INT((new3DPoint.x / MAP_XY_FACTOR) + 0.5f); 
				currentBorder.y = REAL_TO_INT((new3DPoint.y / MAP_XY_FACTOR) + 0.5f); 
				break;
		}

		if (currentBorder.x < 0) {
			currentBorder.x = 0;
		}

		if (currentBorder.y < 0) {
			currentBorder.y = 0;
		}


		pDoc->changeBoundary(m_modifyBorderNdx, &currentBorder);
	}
}

void BorderTool::mouseDown(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	if (m != TRACK_L) {
		return;
	}

	Coord3D groundPt;
	pView->viewToDocCoords(viewPt, &groundPt);

	// Disallow adding new boundaries if we've reached the limit
	const int MAX_BOUNDARIES = 8;
	int boundaryCount = pDoc->getNumBoundaries();
	if (boundaryCount >= MAX_BOUNDARIES) {
		// Only allow modifying existing boundaries
		Int motion;
		pDoc->findBoundaryNear(&groundPt, BOUNDARY_PICK_DISTANCE, &m_modifyBorderNdx, &motion);
		if (motion == 0 || motion == -1) {
			m_modifyBorderNdx = -1; // can't modify bottom-left or no boundary near
		} else {
			m_modificationType = (ModificationType)motion;
		}
		return;
	}

	if (groundPt.length() < BOUNDARY_PICK_DISTANCE) {
		m_addingNewBorder = true;

		if (m_lastBoundaryPosValid) {
			AddBoundaryUndoable *pUndo = new AddBoundaryUndoable(pDoc, &m_lastBoundaryPos);
			pDoc->AddAndDoUndoable(pUndo);
			REF_PTR_RELEASE(pUndo);
		} else {
			ICoord2D initialBoundary = { 1, 1 };
			AddBoundaryUndoable *pUndo = new AddBoundaryUndoable(pDoc, &initialBoundary);
			pDoc->AddAndDoUndoable(pUndo);
			REF_PTR_RELEASE(pUndo);
		}
		return;
	}

	Int motion;
	pDoc->findBoundaryNear(&groundPt, BOUNDARY_PICK_DISTANCE, &m_modifyBorderNdx, &motion);
	
	if (motion == 0) {
		m_modifyBorderNdx = -1;
	} else if (motion == -1) {
		// Add only if boundary count is below max (already checked above, but safety)
		if (boundaryCount < MAX_BOUNDARIES) {
			m_addingNewBorder = true;
			if (m_lastBoundaryPosValid) {
				AddBoundaryUndoable *pUndo = new AddBoundaryUndoable(pDoc, &m_lastBoundaryPos);
				pDoc->AddAndDoUndoable(pUndo);
				REF_PTR_RELEASE(pUndo);
			} else {
				ICoord2D initialBoundary = { 1, 1 };
				AddBoundaryUndoable *pUndo = new AddBoundaryUndoable(pDoc, &initialBoundary);
				pDoc->AddAndDoUndoable(pUndo);
				REF_PTR_RELEASE(pUndo);
			}
		}
	} else {
		m_modificationType = (ModificationType)motion;
	}
}

void BorderTool::mouseUp(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	if (m != TRACK_L) {
		return;
	}

	if (m_addingNewBorder) {
		m_addingNewBorder = false;
		// Do the undoable on the last border
	}
}
