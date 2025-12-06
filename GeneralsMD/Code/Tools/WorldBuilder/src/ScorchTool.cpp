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

// ScorchTool.cpp
// Author: Dennis Griffin, April 2002

#include "StdAfx.h"
#include "resource.h"

#include "ScorchTool.h"
#include "PointerTool.h"
#include "CUndoable.h"
#include "WHeightMapEdit.h"
#include "WorldBuilderDoc.h"
#include "WorldBuilderView.h"
#include "ScorchOptions.h"
#include "MainFrm.h"
#include "DrawObject.h"
#include "Common/WellKnownKeys.h"

//
// ScorchTool class.
//

/// Constructor
ScorchTool::ScorchTool(void) :
	Tool(ID_SCORCH_TOOL, IDC_SCORCH)
{
}

/// Destructor
ScorchTool::~ScorchTool(void)
{
}

/// Clears it's is active flag.
void ScorchTool::deactivate()
{
}

/// Shows the terrain materials options panel.
void ScorchTool::activate()
{
	CMainFrame::GetMainFrame()->showOptionsDialog(IDD_SCORCH_OPTIONS);
	ScorchOptions::update();
	DrawObject::setDoBrushFeedback(false);
}

// Pick a scorchmark.
MapObject *ScorchTool::pickScorch(Coord3D loc){
	// Tight check first.
	MapObject *pObj;
	for (pObj = MapObject::getFirstMapObject(); pObj; pObj = pObj->getNext()) {
		if (!pObj->isScorch()) {
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
		if (!pObj->isScorch()) {
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
void ScorchTool::mouseDown(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	if (m != TRACK_L) return;
	Coord3D docPt;
	pView->viewToDocCoords(viewPt, &docPt);
	MapObject *pObj = pickScorch(docPt);
	if (pObj) {
		pObj->setSelected(true);
		docPt = *pObj->getLocation();
		ScorchOptions::update();
	} else {
		pView->snapPoint(&docPt);
		MapObject *pNew = newInstance(MapObject)(docPt, "Scorch", 0, 0, NULL, NULL );
		pNew->getProperties()->setAsciiString(TheKey_originalOwner, NEUTRAL_TEAM_INTERNAL_STR);
		pNew->setSelected(true);
		pNew->setIsScorch();
		pNew->getProperties()->setReal(TheKey_objectRadius, ScorchOptions::getScorchSize());
		pNew->getProperties()->setInt(TheKey_scorchType, ScorchOptions::getScorchType());
		AddObjectUndoable *pUndo = new AddObjectUndoable(pDoc, pNew);
		pDoc->AddAndDoUndoable(pUndo);
		REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
		pNew = NULL; // undoable owns it now.
		ScorchOptions::update();
	}
	m_mouseDownPt = docPt;
}

/// Left button move code.
void ScorchTool::mouseMoved(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	if (m != TRACK_L) return;
	Coord3D docPt;
	pView->viewToDocCoords(viewPt, &docPt);
	MapObject *pObj = pickScorch(docPt);
	if (pObj) {
		docPt = *pObj->getLocation();
	} else {
		pView->snapPoint(&docPt);
	}
	pView->Invalidate();
}

/** Execute the tool on mouse up - Place an object. */
void ScorchTool::mouseUp(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{

}

