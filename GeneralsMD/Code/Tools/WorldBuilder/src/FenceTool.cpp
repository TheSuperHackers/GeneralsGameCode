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

// FenceTool.cpp
// Texture tiling tool for worldbuilder.
// Author: John Ahlquist, April 2001

#include "StdAfx.h" 
#include "resource.h"

#include "FenceTool.h"
#include "CUndoable.h"
#include "DrawObject.h"
#include "MainFrm.h"
#include "wbview3d.h"
#include "WHeightMapEdit.h"
#include "WorldBuilderDoc.h"
#include "WorldBuilderView.h"
#include "Common/WellKnownKeys.h"
//
// FenceTool class.
//
	enum {MAX_OBJECTS = 200};
/// Constructor
FenceTool::FenceTool(void) :
	Tool(ID_FENCE_TOOL, IDC_FENCE),
		m_mapObjectList(NULL),
		m_objectCount(1)
{
		m_curObjectWidth = 27.35f;
		m_curObjectOffset = 0;
}
	
/// Destructor
FenceTool::~FenceTool(void) 
{
	if (m_mapObjectList) {
		MemoryPoolObject::deleteInstance(m_mapObjectList);
	}
	m_mapObjectList = NULL;
}

void FenceTool::updateMapObjectList(Coord3D downPt, Coord3D curPt, WbView* pView, CWorldBuilderDoc *pDoc, Bool checkPlayers)
{
	Bool shiftKey = (0x8000 & ::GetAsyncKeyState(VK_SHIFT))!=0;
	Real angle = ObjectTool::calcAngle(downPt, curPt, pView);
	Coord3D delta;
	Coord3D normalDelta;
	delta.x = curPt.x-downPt.x;
	delta.y = curPt.y-downPt.y;
	delta.z = 0;
	Real length = delta.length();
	Int newCount = floor(length/m_curObjectWidth);
	if (newCount<1) newCount = 1;
	if (newCount > MAX_OBJECTS) newCount = MAX_OBJECTS;
	if (shiftKey) {
		// stretch the fence spacing.	Keep the same count.
		delta.x /= (m_objectCount*m_curObjectWidth);
		delta.y /= (m_objectCount*m_curObjectWidth);
	} else {
		// Use exactly the spacing.
		m_objectCount = newCount;
		delta.normalize();
	}
	normalDelta = delta;
	if (angle==0) {
		normalDelta.x = 1;
		normalDelta.y = 0;
	}
	normalDelta.normalize();

	Int i;
	if (m_mapObjectList == NULL) return;
	MapObject *pCurObj = m_mapObjectList;
	for (i=1; i<m_objectCount; i++) {
		if (pCurObj->getNext() == NULL) {
			pCurObj->setNextMap(ObjectOptions::duplicateCurMapObjectForPlace(&downPt, angle, checkPlayers));
		}
		pCurObj=pCurObj->getNext();
		if (pCurObj == NULL) return;
	}
	WbView3d *p3View = pDoc->GetActive3DView();
	MapObject *pXtraObjects = pCurObj->getNext();
	pCurObj->setNextMap(NULL);
	if (pXtraObjects) {
		p3View->removeFenceListObjects(pXtraObjects);
		MemoryPoolObject::deleteInstance(pXtraObjects);
		pXtraObjects = NULL;
	}

	pCurObj = m_mapObjectList;
	for (i=0; i<m_objectCount; i++) {
		Real factor = m_curObjectWidth*(i);
		Coord3D curLoc = downPt;
		curLoc.x += factor*delta.x;
		curLoc.y += factor*delta.y;
		curLoc.x += m_curObjectOffset*normalDelta.x;
		curLoc.y += m_curObjectOffset*normalDelta.y;
		curLoc.z = MAGIC_GROUND_Z;

		pCurObj->setAngle(angle);
		pCurObj->setLocation(&curLoc);
		pCurObj=pCurObj->getNext();
	}

	p3View->updateFenceListObjects(m_mapObjectList);

}



/// Turn off object tracking.
void FenceTool::deactivate() 
{
	CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
	if (pDoc==NULL) return;
	WbView3d *p3View = pDoc->GetActive3DView();
	p3View->setObjTracking(NULL, m_downPt3d, 0, false);
}
/// Shows the object options panel
void FenceTool::activate() 
{
	CMainFrame::GetMainFrame()->showOptionsDialog(IDD_FENCE_OPTIONS);
	DrawObject::setDoBrushFeedback(false);
	CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
	if (pDoc==NULL) return;
	WbView3d *p3View = pDoc->GetActive3DView();
	p3View->setObjTracking(NULL, m_downPt3d, 0, false);
	FenceOptions::update();
}

/** Execute the tool on mouse down - Place an object. */
void FenceTool::mouseDown(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc) 
{
	if (m != TRACK_L) return;

	Coord3D cpt;
	pView->viewToDocCoords(viewPt, &cpt);
	pView->snapPoint(&cpt);
	m_downPt2d = viewPt;
	m_downPt3d = cpt;
	if (m_mapObjectList) {
		MemoryPoolObject::deleteInstance(m_mapObjectList);
		m_mapObjectList = NULL;
	}
	if (FenceOptions::hasSelectedObject()) {
		FenceOptions::update();
		m_curObjectWidth = FenceOptions::getFenceSpacing();
		m_curObjectOffset = FenceOptions::getFenceOffset();
		m_mapObjectList = ObjectOptions::duplicateCurMapObjectForPlace(&m_downPt3d, 0, false);
		m_objectCount = 1;
		mouseMoved(m, viewPt, pView, pDoc);
	}
}
																			 
/** Tracking - show the object. */
void FenceTool::mouseMoved(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc) 
{
//	Bool justAClick = true;
	Coord3D cpt;
	pView->viewToDocCoords(viewPt, &cpt); // Do constrain.
	WbView3d *p3View = pDoc->GetActive3DView();
	Coord3D loc = cpt;
	pView->snapPoint(&loc);
	Real angle =  0 ;
	if (m == TRACK_L) {	// Mouse is down, so fence.
		p3View->setObjTracking(NULL, loc, angle, false);
		updateMapObjectList(m_downPt3d,loc, pView, pDoc, false);
		return;
	}
	MapObject *pCur = ObjectOptions::getObjectNamed(AsciiString(ObjectOptions::getCurObjectName()));
	p3View->setObjTracking(NULL, m_downPt3d, 0, false);
	loc.z = ObjectOptions::getCurObjectHeight();
	if (pCur && FenceOptions::hasSelectedObject()) { 
		// Display the transparent version of this object.
		m_curObjectOffset = FenceOptions::getFenceOffset();
		loc.x += m_curObjectOffset;
		p3View->setObjTracking(pCur, loc, angle, true);
	} else {
		// Don't display anything. 
		p3View->setObjTracking(NULL, loc, angle, false);
	}
}

/** Execute the tool on mouse up - Place an object. */
void FenceTool::mouseUp(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc) 
{
	if (m != TRACK_L) return;
	Coord3D cpt;
	pView->viewToDocCoords(viewPt, &cpt); // Do constrain.
	Coord3D loc = cpt;
	pView->snapPoint(&loc);
	updateMapObjectList(m_downPt3d, loc, pView, pDoc, true);
	WbView3d *p3View = pDoc->GetActive3DView();
	if (m_mapObjectList) {
		p3View->removeFenceListObjects(m_mapObjectList);
		AddObjectUndoable *pUndo = new AddObjectUndoable(pDoc, m_mapObjectList);
		pDoc->AddAndDoUndoable(pUndo);
		REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
		m_mapObjectList = NULL; // undoable owns it now.
	}
}

