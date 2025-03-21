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

const long BOUNDARY_PICK_DISTANCE = 5.0f;

BorderTool::BorderTool() : Tool(ID_BORDERTOOL, IDC_POINTER), 
													 m_mouseDown(false),
													 m_addingNewBorder(false), 
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

	//static Coord3D zero = {0.0f, 0.0f, 0.0f};
	
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
	
	// if bottom left boundary grabbed
	if (motion == 0) 
	{
		// modifying the bottom left is not allowed.
		m_modifyBorderNdx = -1;
	}
	// else if no boundary is near
	else if (motion == -1)
	{
		// add a boundary
		m_addingNewBorder = true;
		
		ICoord2D initialBoundary = { 1, 1 };
		pDoc->addBoundary(&initialBoundary);
	} 
	else
	{
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
	}
}
