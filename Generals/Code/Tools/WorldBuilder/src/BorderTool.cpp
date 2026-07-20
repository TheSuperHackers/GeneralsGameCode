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

#include "StdAfx.h"
#include "resource.h"

#include "BorderTool.h"
#include "DrawObject.h"
#include "MainFrm.h"
#include "wbview3d.h"
#include "WorldBuilderDoc.h"

const long BOUNDARY_PICK_DISTANCE = 5.0f;

BorderTool::BorderTool() : Tool(ID_BORDERTOOL, IDC_POINTER),
													 m_mouseDown(false),
													 m_addingNewBorder(false),
													 m_modifyBorderNdx(-1)

{ }

BorderTool::~BorderTool()
{

}

void BorderTool::setCursor()
{

}

void BorderTool::activate()
{
	CMainFrame::GetMainFrame()->showOptionsDialog(IDD_NO_OPTIONS);
	DrawObject::setDoBoundaryFeedback(TRUE);
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

		current.x = REAL_TO_INT((new3DPoint.x / MAP_XY_FACTOR) + 0.5f);
		current.y = REAL_TO_INT((new3DPoint.y / MAP_XY_FACTOR) + 0.5f);

		// TheSuperHackers @bugfix ZsoltFeher 07/20/2026 These clamps used to run before the assignments
		// above and were therefore dead code (immediately overwritten), so a newly-added border being
		// dragged towards the origin could be given negative extents. Clamp after assigning from the
		// mouse position instead. (GitHub issue #413)
		if (current.x < 0) {
			current.x = 0;
		}

		if (current.y < 0) {
			current.y = 0;
		}

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

		// TheSuperHackers @bugfix ZsoltFeher 07/20/2026 Boundary 0 is the default/main border, always
		// created spanning the whole map when a new map is made, and other systems (e.g. shroud) rely on
		// at least one non-degenerate boundary existing. Unlike any other boundary, collapsing it to zero
		// width/height by dragging it into the origin permanently breaks the map (always shrouded) with
		// no way to bring it back, since it's the one boundary that can never simply be re-added by the
		// user like any other. Never let it be dragged smaller than 1x1. (GitHub issue #312)
		if (m_modifyBorderNdx == 0) {
			if (currentBorder.x < 1) {
				currentBorder.x = 1;
			}

			if (currentBorder.y < 1) {
				currentBorder.y = 1;
			}
		}

		pDoc->changeBoundary(m_modifyBorderNdx, &currentBorder);
	}
}

void BorderTool::mouseDown(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	if (m != TRACK_L) {
		return;
	}

	static Coord3D zero = {0.0f, 0.0f, 0.0f};

	Coord3D groundPt;
	pView->viewToDocCoords(viewPt, &groundPt);
	if (groundPt.length() < BOUNDARY_PICK_DISTANCE) {
		m_addingNewBorder = true;

		ICoord2D initialBoundary = { 1, 1 };
		pDoc->addBoundary(&initialBoundary);
		return;
	}

	Int motion;
	pDoc->findBoundaryNear(&groundPt, BOUNDARY_PICK_DISTANCE, &m_modifyBorderNdx, &motion);
	if (motion == 0) {
		// modifying the bottom left is not allowed.
		m_modifyBorderNdx = -1;
	} else {
		m_modificationType = (ModificationType) motion;
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

		// TheSuperHackers @bugfix ZsoltFeher 07/20/2026 A newly-added border can also end up degenerate
		// (zero or, before the mouseMoved() clamp-order fix above, negative extents) if it's dragged
		// straight back onto the origin before release -- the same permanently-invisible-and-unpickable
		// outcome the modify-path cleanup below already handles. This is never boundary 0 (the default
		// border is created once by the map constructor, never via this add-new-border path), so no
		// index-0 protection is needed here. (GitHub issue #413)
		Int newNdx = pDoc->getNumBoundaries() - 1;
		if (newNdx >= 0) {
			ICoord2D newBorder;
			pDoc->getBoundary(newNdx, &newBorder);
			if (newBorder.x <= 0 || newBorder.y <= 0) {
				pDoc->removeBoundary(newNdx);
			}
		}
	}

	// TheSuperHackers @bugfix ZsoltFeher 07/20/2026 A boundary is anchored at the origin (0,0), so
	// dragging its opposite corner onto the origin collapses it to zero width or height. That used to
	// leave a degenerate, permanently invisible and unselectable entry in the boundary list forever
	// (findBoundaryNear() already skips zero-sized boundaries, so it could never be picked again).
	// Actually remove it here instead, completing what dragging the corners together was clearly meant
	// to do: get rid of an unwanted boundary. (GitHub issue #413)
	if (m_modifyBorderNdx >= 0) {
		ICoord2D currentBorder;
		pDoc->getBoundary(m_modifyBorderNdx, &currentBorder);
		// TheSuperHackers @bugfix ZsoltFeher 07/20/2026 Never auto-remove boundary 0, the default/main
		// border (see the m_modifyBorderNdx == 0 clamp in mouseMoved() above, which already prevents it
		// from reaching zero width/height by dragging in the first place). (GitHub issue #312)
		if (m_modifyBorderNdx != 0 && (currentBorder.x == 0 || currentBorder.y == 0)) {
			// TheSuperHackers @bugfix ZsoltFeher 07/20/2026 Map scripts can reference a boundary by its
			// raw list index (Parameter::BOUNDARY). Removing anything other than the last boundary shifts
			// every later boundary's index down by one, silently retargeting any script that referenced
			// one of them. Warn the user so they know to double check their scripts; removing the actual
			// last boundary (as the add-new-border path above always does) never shifts anything, so no
			// warning is needed there. (GitHub issue #413)
			if (m_modifyBorderNdx != pDoc->getNumBoundaries() - 1) {
				::AfxMessageBox(_T("Removing this boundary will shift the index of every boundary after it. If any map scripts reference boundaries by number, please double check them."), MB_OK | MB_ICONWARNING);
			}
			pDoc->removeBoundary(m_modifyBorderNdx);
		}
		m_modifyBorderNdx = -1;
	}
}
