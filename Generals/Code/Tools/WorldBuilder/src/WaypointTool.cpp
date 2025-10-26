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

// WaypointTool.cpp
// Texture tiling tool for worldbuilder.
// Author: John Ahlquist, April 2001

#include "StdAfx.h"
#include "resource.h"

#include "WaypointTool.h"
#include "PointerTool.h"
#include "CUndoable.h"
#include "TerrainMaterial.h"
#include "WHeightMapEdit.h"
#include "WorldBuilderDoc.h"
#include "WorldBuilderView.h"
#include "MainFrm.h"
#include "DrawObject.h"
#include "Common/WellKnownKeys.h"

//
// WaypointTool class.
//

Bool WaypointTool::m_isActive = false;


/// Constructor
WaypointTool::WaypointTool(void) :
	Tool(ID_WAYPOINT_TOOL, IDC_WAYPOINT)
{
}

/// Destructor
WaypointTool::~WaypointTool(void)
{
}

/// Clears it's is active flag.
void WaypointTool::deactivate()
{
	m_isActive = false;
}

/// Shows the terrain materials options panel.
void WaypointTool::activate()
{
	CMainFrame::GetMainFrame()->showOptionsDialog(IDD_WAYPOINT_OPTIONS);
	WaypointOptions::update();
	DrawObject::setDoBrushFeedback(false);
	m_isActive = true;
}

// Pick a waypoint.
MapObject *WaypointTool::pickWaypoint(Coord3D loc){
	// Tight check first.
	MapObject *pObj;
	for (pObj = MapObject::getFirstMapObject(); pObj; pObj = pObj->getNext()) {
		if (!pObj->isWaypoint()) {
			continue;
		}
		Coord3D cloc = *pObj->getLocation();
		// Check and see if we are within 1/2 cell size of the center.
		Coord3D cpt = loc;
		cpt.x -= cloc.x;
		cpt.y -= cloc.y;
		cpt.z = 0;
		if (cpt.length() < 0.5f*MAP_XY_FACTOR) {
			return pObj;
		}
	}
	// Loose check
	for (pObj = MapObject::getFirstMapObject(); pObj; pObj = pObj->getNext()) {
		if (!pObj->isWaypoint()) {
			continue;
		}
		Coord3D cloc = *pObj->getLocation();
		// Check and see if we are within 1 & 1/2 cell size of the center.
		Coord3D cpt = loc;
		cpt.x -= cloc.x;
		cpt.y -= cloc.y;
		cpt.z = 0;
		if (cpt.length() < 1.5f*MAP_XY_FACTOR) {
			return pObj;
		}
	}
	return NULL;
}


/// Perform the tool behavior on mouse down.
void WaypointTool::mouseDown(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	if (m != TRACK_L) return;
	m_downWaypointID = 0;
	Coord3D docPt;
	pView->viewToDocCoords(viewPt, &docPt);
	MapObject *pObj = pickWaypoint(docPt);
	if (pObj) {
		pObj->setSelected(true);
		m_downWaypointID = pObj->getWaypointID();
		docPt = *pObj->getLocation();
		WaypointOptions::update();
	}	else {
		pView->snapPoint(&docPt);
		MapObject *pNew = newInstance( MapObject)(docPt, AsciiString("*Waypoints/Waypoint"), 0, 0, NULL, NULL );
		Int id = pDoc->getNextWaypointID();
		AsciiString name = WaypointOptions::GenerateUniqueName(id);
		pNew->setSelected(true);
		pNew->setIsWaypoint();
		pNew->setWaypointID(id);
		pNew->setWaypointName(name);
		pNew->getProperties()->setAsciiString(TheKey_originalOwner, AsciiString("team"));
		AddObjectUndoable *pUndo = new AddObjectUndoable(pDoc, pNew);
		pDoc->AddAndDoUndoable(pUndo);
		REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
		pNew = NULL; // undoable owns it now.
		m_downWaypointID = id;
		WaypointOptions::update();
	}
	m_mouseDownPt = docPt;
}

/// Left button move code.
void WaypointTool::mouseMoved(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	if (m != TRACK_L) return;
	Coord3D docPt;
	pView->viewToDocCoords(viewPt, &docPt);
	MapObject *pObj = pickWaypoint(docPt);
	if (pObj) {
		docPt = *pObj->getLocation();
	} else {
		pView->snapPoint(&docPt);
	}
	DrawObject::setWaypointDragFeedback(m_mouseDownPt, docPt);
	pView->Invalidate();
}

/** Execute the tool on mouse up - Place an object. */
void WaypointTool::mouseUp(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	DrawObject::stopWaypointDragFeedback();
	if (m != TRACK_L) return;

	Coord3D docPt;
	pView->viewToDocCoords(viewPt, &docPt);
	MapObject *pObj;
	PointerTool::clearSelection();
	pObj = pickWaypoint(docPt);
	if (pObj == NULL) {
		pView->snapPoint(&docPt);
		MapObject *pNew = newInstance( MapObject)(docPt, AsciiString("*Waypoints/Waypoint"), 0, 0, NULL, NULL );
		Int id = pDoc->getNextWaypointID();
		AsciiString name;
		name.format("Waypoint %d", id);
		pNew->setSelected(true);
		pNew->setIsWaypoint();
		pNew->setWaypointID(id);
		pNew->setWaypointName(name);
		pNew->getProperties()->setAsciiString(TheKey_originalOwner, AsciiString("team"));
		AddObjectUndoable *pUndo = new AddObjectUndoable(pDoc, pNew);
		pDoc->AddAndDoUndoable(pUndo);
		REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
		pObj = pNew;
		pNew = NULL; // undoable owns it now.
	}
	if (pObj)
	{
		pObj->setSelected(true);
		if (m_downWaypointID && (pObj->getWaypointID() != m_downWaypointID))
		{
			Int waypointID = pObj->getWaypointID();
			// If a link
			if (pDoc->waypointLinkExists(m_downWaypointID, waypointID))
			{
				pDoc->removeWaypointLink(m_downWaypointID, waypointID);
			}
			else if (m_downWaypointID != waypointID)
			{
				pDoc->addWaypointLink(m_downWaypointID, waypointID);
			}
			MapObject *pDown = pDoc->getWaypointByID(m_downWaypointID);
			if (pDown) {
				pDoc->updateLinkedWaypointLabels(pDown);
			}
		}
		WaypointOptions::update();
	}

}

