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

// ObjectTool.cpp
// Texture tiling tool for worldbuilder.
// Author: John Ahlquist, April 2001

#include "StdAfx.h" 
#include "resource.h"

#include "ObjectTool.h"
#include "CUndoable.h"
#include "DrawObject.h"
#include "MainFrm.h"
#include "wbview3d.h"
#include "WHeightMapEdit.h"
#include "WorldBuilderDoc.h"
#include "WorldBuilderView.h"
#include "Common/ThingTemplate.h"
#include "Common/WellKnownKeys.h"
//
// ObjectTool class.
//
	enum {HYSTERESIS = 3};
/// Constructor
ObjectTool::ObjectTool(void) :
	Tool(ID_PLACE_OBJECT_TOOL, IDC_PLACE_OBJECT) 
{
}
	
/// Destructor
ObjectTool::~ObjectTool(void) 
{
}

Real ObjectTool::calcAngle(Coord3D downPt, Coord3D curPt, WbView* pView)
{
	enum {HYSTERESIS = 3};
	double dx = curPt.x - downPt.x;
	double dy = curPt.y - downPt.y;
	double dist = sqrt(dx*dx+dy*dy);
	double angle = 0;
	if (dist < 0.1) // check for div-by-zero.
	{
		angle = 0;
	} 
	else if (abs(dx) > abs(dy)) 
	{
		angle = acos(	(double)dx / dist);
		if (dy<0) angle = -angle;
	} 
	else 
	{
		angle = asin(	((double)dy) / dist);
		if (dx<0) angle = PI-angle;
	}
	if (angle > PI) angle -= 2*PI;
#ifdef _DEBUG
	CString buf;
	buf.Format("Angle %f rad, %d degrees\n", angle, (int)(angle*180/PI));
	::OutputDebugString(buf);
#endif
	return((Real)angle);
}



/// Turn off object tracking.
void ObjectTool::deactivate() 
{
	CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
	if (pDoc==NULL) return;
	WbView3d *p3View = pDoc->GetActive3DView();
	p3View->setObjTracking(NULL, m_downPt3d, 0, false);
}
/// Shows the object options panel
void ObjectTool::activate() 
{
	CMainFrame::GetMainFrame()->showOptionsDialog(IDD_OBJECT_OPTIONS);
	DrawObject::setDoBrushFeedback(false);
	CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
	if (pDoc==NULL) return;
	WbView3d *p3View = pDoc->GetActive3DView();
	p3View->setObjTracking(NULL, m_downPt3d, 0, false);
}

/** Execute the tool on mouse down - Place an object. */
void ObjectTool::mouseDown(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc) 
{
	if (m != TRACK_L) return;

	Coord3D cpt;
	pView->viewToDocCoords(viewPt, &cpt);
	m_downPt2d = viewPt;
	m_downPt3d = cpt;
}

/** Tracking - show the object. */
void ObjectTool::mouseMoved(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc) 
{
	Bool justAClick = true;
	Coord3D cpt;
	pView->viewToDocCoords(viewPt, &cpt, false); // Don't constrain.
	Coord3D loc = cpt;
	pView->snapPoint(&loc);
	if (m == TRACK_L) {	// Mouse is down, so preview the angle if > hysteresis.
		// always check hysteresis in view coords.
		justAClick = (abs(viewPt.x - m_downPt2d.x)<HYSTERESIS || abs(viewPt.x - m_downPt2d.x)<HYSTERESIS);
		loc = m_downPt3d;
	}
	MapObject *pCur = ObjectOptions::getObjectNamed(AsciiString(ObjectOptions::getCurObjectName()));
	Real angle = justAClick ? 0 : calcAngle(loc, cpt, pView);
	if (justAClick && pCur && pCur->getThingTemplate()) {
		angle = pCur->getThingTemplate()->getPlacementViewAngle();
	}
	WbView3d *p3View = pDoc->GetActive3DView();
	p3View->setObjTracking(NULL, m_downPt3d, 0, false);
	loc.z = ObjectOptions::getCurObjectHeight();
	if (pCur) { 
		// Display the transparent version of this object.
		p3View->setObjTracking(pCur, loc, angle, true);
	} else {
		// Don't display anything. 
		p3View->setObjTracking(NULL, loc, angle, false);
	}
}

/** Execute the tool on mouse up - Place an object. */
void ObjectTool::mouseUp(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc) 
{
	if (m != TRACK_L) return;

	// always check hysteresis in view coords.
	Bool justAClick = (abs(viewPt.x - m_downPt2d.x)<HYSTERESIS || abs(viewPt.x - m_downPt2d.x)<HYSTERESIS);

	Coord3D cpt;
	pView->viewToDocCoords(viewPt, &cpt, false); // Don't constrain.

	Coord3D loc = m_downPt3d;
	pView->snapPoint(&loc);
	loc.z = ObjectOptions::getCurObjectHeight();
	Real angle = justAClick ? 0 : calcAngle(loc, cpt, pView);
	MapObject *pNew = ObjectOptions::duplicateCurMapObjectForPlace(&loc, angle, true);
	if (pNew) {
		if (justAClick && pNew->getThingTemplate()) {
			angle = pNew->getThingTemplate()->getPlacementViewAngle();
			pNew->setAngle(angle);
		}
		AddObjectUndoable *pUndo = new AddObjectUndoable(pDoc, pNew);
		pDoc->AddAndDoUndoable(pUndo);
		REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
		pNew = NULL; // undoable owns it now.
	}
}

