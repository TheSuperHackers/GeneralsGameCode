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

// BuildListTool.cpp
// Texture tiling tool for worldbuilder.
// Author: John Ahlquist, April 2001

#include "StdAfx.h"
#include "resource.h"

#include "BuildListTool.h"
#include "CUndoable.h"
#include "DrawObject.h"
#include "MainFrm.h"
#include "ObjectTool.h"
#include "PointerTool.h"
#include "PickUnitDialog.h"
#include "wbview3d.h"
#include "WHeightMapEdit.h"
#include "WorldBuilderDoc.h"
#include "WorldBuilderView.h"
#include "GameLogic/SidesList.h"
//
// BuildListTool class.
//

Bool BuildListTool::m_isActive = false;
PickUnitDialog* BuildListTool::m_static_pickBuildingDlg = NULL;

/// Constructor
BuildListTool::BuildListTool(void) :
	Tool(ID_BUILD_LIST_TOOL, IDC_BUILD_LIST_TOOL),
	m_rotateCursor(NULL),
	m_pointerCursor(NULL),
	m_moveCursor(NULL),
	m_created(false)
{
	m_curObject = NULL;
}

/// Destructor
BuildListTool::~BuildListTool(void)
{
}

void BuildListTool::createWindow(void)
{
	CRect frameRect;
	frameRect.top = ::AfxGetApp()->GetProfileInt(BUILD_PICK_PANEL_SECTION, "Top", 0);
	frameRect.left =::AfxGetApp()->GetProfileInt(BUILD_PICK_PANEL_SECTION, "Left", 0);

	m_pickBuildingDlg.SetAllowableType(ES_STRUCTURE);
	m_pickBuildingDlg.SetFactionOnly(true);
	m_pickBuildingDlg.Create(IDD_PICKUNIT, CMainFrame::GetMainFrame());
	m_pickBuildingDlg.SetupAsPanel();
	m_pickBuildingDlg.SetWindowPos(NULL, frameRect.left, frameRect.top, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
	m_static_pickBuildingDlg = &m_pickBuildingDlg;
	m_created = true;
}

Bool BuildListTool::isDoingAdd(void)
{
	if (!m_created) {
		return false;
	}
	if (!m_pickBuildingDlg.IsWindowVisible()) {
		return false;
	}
	if (m_pickBuildingDlg.getPickedUnit() == AsciiString::TheEmptyString) {
		return false;
	}
	return true;
}

/// Shows the object options panel.
void BuildListTool::addBuilding()
{
	//PickUnitDialog dlg;
	//dlg.SetAllowableType(ES_STRUCTURE);
	//dlg.SetFactionOnly(true);
	//if (dlg.DoModal() == IDOK) {
	//}
	//CMainFrame::GetMainFrame()->showOptionsDialog(IDD_OBJECT_OPTIONS);
	m_static_pickBuildingDlg->ShowWindow(SW_SHOWNA);
}

/// Shows the object options panel.
void BuildListTool::activate()
{
	Bool wasActive = m_isActive;
	m_isActive = true;
	CMainFrame::GetMainFrame()->showOptionsDialog(IDD_BUILD_LIST_PANEL);
	DrawObject::setDoBrushFeedback(false);
	BuildList::update();
	WbView3d *p3View = CWorldBuilderDoc::GetActive3DView();
	if (!wasActive)
	{
		p3View->resetRenderObjects();
		p3View->invalObjectInView(NULL);
	}
	if (!m_created) {
		createWindow();
	}
	m_pickBuildingDlg.ShowWindow(SW_SHOWNA);
}

/// Clears the isActive flag.
void BuildListTool::deactivate()
{
	m_isActive = false;
	WbView3d *p3View = CWorldBuilderDoc::GetActive3DView();
	Coord3D loc;
	loc.x=loc.y=loc.z=0;
	p3View->setObjTracking(NULL, loc, 0, false);	// Turn off object cursor tracking.
	p3View->resetRenderObjects();
	p3View->invalObjectInView(NULL);
	m_pickBuildingDlg.ShowWindow(SW_HIDE);
}

/** Set the cursor. */
void BuildListTool::setCursor(void)
{
	if (isDoingAdd()) {
		Tool::setCursor();	// Default cursor is the adding cursor
	} else 	if (m_mouseUpMove) {
		if (m_moveCursor == NULL) {
			m_moveCursor = AfxGetApp()->LoadCursor(MAKEINTRESOURCE(IDC_BUILD_MOVE));
		}
		::SetCursor(m_moveCursor);
	} else 	if (m_mouseUpRotate) {
		if (m_rotateCursor == NULL) {
			m_rotateCursor = AfxGetApp()->LoadCursor(MAKEINTRESOURCE(IDC_BUILD_ROTATE));
		}
		::SetCursor(m_rotateCursor);
	} else {
		if (m_pointerCursor == NULL) {
			m_pointerCursor = AfxGetApp()->LoadCursor(MAKEINTRESOURCE(IDC_BUILD_POINTER));
		}
		::SetCursor(m_pointerCursor);
	}
}

/** Execute the tool on mouse down - Place an object. */
void BuildListTool::mouseDown(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	if (m != TRACK_L) return;
	m_moving = false;
	m_rotating = false;
	Coord3D cpt;
	pView->viewToDocCoords(viewPt, &cpt);
	m_downPt2d = viewPt;
	m_downPt3d = cpt;
	if (isDoingAdd()) return;
	BuildListInfo *pInfo = pView->pickedBuildObjectInView(viewPt);
	if (pInfo) {
		m_curObject = pInfo;
		Coord3D center = *pInfo->getLocation();
		center.x -= m_downPt3d.x;
		center.y -= m_downPt3d.y;
		center.z = 0;
		Real len = center.length();
		// Check and see if we are within 1 cell size of the center.
		if (pInfo->isSelected() && len>0.5f*MAP_XY_FACTOR && len < 1.5f*MAP_XY_FACTOR) {
			m_rotating = true;
		}
		m_moving = true;
		m_prevPt3d = m_downPt3d;
		pView->snapPoint(&m_prevPt3d);
		BuildList::setSelectedBuildList(pInfo);
	} else {
		PointerTool::clearSelection();
	}
}

void BuildListTool::mouseMoved(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	Coord3D cpt;
	pView->viewToDocCoords(viewPt, &cpt, false);

	WbView3d *p3View = pDoc->GetActive3DView();
	if (isDoingAdd()) {
		Coord3D loc = cpt;
		MapObject *pCur = ObjectOptions::getObjectNamed(m_pickBuildingDlg.getPickedUnit());
		loc.z = 0;
		if (pCur) {
			// Display the transparent version of this object.
			p3View->setObjTracking(pCur, loc, 0, true);
		} else {
			// Don't display anything.
			p3View->setObjTracking(NULL, loc, 0, false);
		}
		return;
	}
	p3View->setObjTracking(NULL, cpt, 0, false);

	if (m == TRACK_NONE) {
		// See if the cursor is over an object.
		BuildListInfo *pInfo = pView->pickedBuildObjectInView(viewPt);
		m_mouseUpMove	= false;
		m_mouseUpRotate = false;
		if (pInfo) {
			Coord3D center = *pInfo->getLocation();
			center.x -= cpt.x;
			center.y -= cpt.y;
			center.z = 0;
			Real len = center.length();
			// Check and see if we are within 1 cell size of the center.
			if (pInfo->isSelected() && len>0.5f*MAP_XY_FACTOR && len < 1.5f*MAP_XY_FACTOR) {
				m_mouseUpRotate = true;
			}	else {
				m_mouseUpMove = true;
			}
		}
		return;	// setCursor will use the value of m_mouseUpRotate.  jba.
	}

	if (m != TRACK_L) return;
	if (!m_moving || !m_curObject) return;

	Coord3D loc = *m_curObject->getLocation();
	if (m_rotating) {
		Real angle = ObjectTool::calcAngle(m_downPt3d, cpt, pView);
		m_curObject->setAngle(angle);
	} else {
		pView->snapPoint(&cpt);
		Real xOffset = (cpt.x-m_prevPt3d.x);
		Real yOffset = (cpt.y-m_prevPt3d.y);
		loc.x += xOffset;
		loc.y += yOffset;
		m_curObject->setLocation(loc);
	}
	p3View->invalBuildListItemInView(m_curObject);
	m_prevPt3d = cpt;

}



/** Execute the tool on mouse up - Place an object. */
void BuildListTool::mouseUp(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	if (m != TRACK_L) return;
	if (!isDoingAdd()) {
		// Update the cursor.
		mouseMoved(TRACK_NONE, viewPt, pView, pDoc);
		setCursor();
		return;
	}
	// always check hysteresis in view coords.
	enum {HYSTERESIS = 3};
	Bool justAClick = (abs(viewPt.x - m_downPt2d.x)<HYSTERESIS || abs(viewPt.x - m_downPt2d.x)<HYSTERESIS);

	Coord3D cpt;
	pView->viewToDocCoords(viewPt, &cpt, false); // Don't constrain.

	Coord3D loc = m_downPt3d;
	pView->snapPoint(&loc);
	loc.z = ObjectOptions::getCurObjectHeight();
	Real angle = justAClick ? 0 : ObjectTool::calcAngle(loc, cpt, pView);
	if (!m_pickBuildingDlg.getPickedUnit().isEmpty()) {
		BuildList::addBuilding(loc, angle, m_pickBuildingDlg.getPickedUnit());
	}
	//CMainFrame::GetMainFrame()->showOptionsDialog(IDD_BUILD_LIST_PANEL);
}

