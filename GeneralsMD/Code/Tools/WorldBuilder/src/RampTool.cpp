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

// FILE: RampTool.cpp 
/*---------------------------------------------------------------------------*/
/* EA Pacific                                                                */
/* Confidential Information	                                                 */
/* Copyright (C) 2001 - All Rights Reserved                                  */
/* DO NOT DISTRIBUTE                                                         */
/*---------------------------------------------------------------------------*/
/* Project:    RTS3                                                          */
/* File name:  RampTool.cpp                                                  */
/* Created:    John K. McDonald, Jr., 4/19/2002                              */
/* Desc:       // Ramp tool implementation                                   */
/* Revision History:                                                         */
/*		4/19/2002 : Initial creation                                           */
/*---------------------------------------------------------------------------*/
#include "StdAfx.h"
#include "RampTool.h"

#include "CUndoable.h"
#include "MainFrm.h"
#include "DrawObject.h"
#include "RampOptions.h"
#include "resource.h"
#include "wbview.h"
#include "WHeightMapEdit.h"
#include "WorldBuilder.h"
#include "WorldBuilderDoc.h"

#include "GameClient/Line2D.h"


#include "W3DDevice/GameClient/HeightMap.h"

RampTool::RampTool() : Tool(ID_RAMPTOOL, IDC_RAMP )
{

}

void RampTool::activate()
{
	Tool::activate();
	CMainFrame::GetMainFrame()->showOptionsDialog(IDD_RAMP_OPTIONS);

	mIsMouseDown = false;
}

void RampTool::deactivate()
{
	DrawObject::setDoRampFeedback(false);
	mIsMouseDown = false;
}

Bool RampTool::followsTerrain(void)
{
	return true;
}

void RampTool::mouseMoved(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	if (!((m == TRACK_L) && mIsMouseDown)) {
		if (TheRampOptions->shouldApplyTheRamp()) {
			// Call me now for your free ramp application!
			applyRamp(pDoc);
			return;
		}
	} else if (m == TRACK_L) {
		Coord3D docPt;
		pView->viewToDocCoords(viewPt, &docPt);
		docPt.z = TheTerrainRenderObject->getHeightMapHeight(docPt.x, docPt.y, NULL);
		mEndPoint = docPt;
	}

	drawFeedback(&mEndPoint);
}

void RampTool::mouseDown(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	if (m != TRACK_L) {
		return;
	}

	Coord3D docPt;
	pView->viewToDocCoords(viewPt, &docPt);
	mStartPoint = docPt;
	mStartPoint.z = TheTerrainRenderObject->getHeightMapHeight(mStartPoint.x, mStartPoint.y, NULL);

	mIsMouseDown = true;
}

void RampTool::mouseUp(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{

	if (!((m == TRACK_L) && mIsMouseDown)) {
		return;
	}

	Coord3D docPt;
	pView->viewToDocCoords(viewPt, &docPt);
	mEndPoint = docPt;
	mEndPoint.z = TheTerrainRenderObject->getHeightMapHeight(mEndPoint.x, mEndPoint.y, NULL);

	mIsMouseDown = false;
}

void RampTool::drawFeedback(Coord3D* endPoint)
{
	DrawObject::setDoRampFeedback(true);
	DrawObject::setRampFeedbackParms(&mStartPoint, endPoint, TheRampOptions->getRampWidth());
}

void RampTool::applyRamp(CWorldBuilderDoc* pDoc)
{
	Coord3D bl, tl, br, tr;
	VecHeightMapIndexes indices;

	WorldHeightMapEdit *worldHeightDup = pDoc->GetHeightMap()->duplicate();
	
	Real width = TheRampOptions->getRampWidth();
	
	BuildRectFromSegmentAndWidth(&mStartPoint, &mEndPoint, width,
															 &bl, &tl, &br, &tr);

	if (bl == br || bl == tl) {
		return;
	}

	getAllIndexesIn(&bl, &br, &tl, &tr, 0, pDoc, &indices);
	int indiceCount = indices.size();
	if (indiceCount == 0) {
		return;
	}

	/*
		This part is pretty straightforward. Determine the U value for the shortest segment from the 
		index's actual location to the segments from mStartPoint to mEndPoint, and then apply a 
		linear gradient factor according to the height from the beginning to the end of mStartPoint
		and mEndPoint.
	*/

	for (int i = 0; i < indiceCount; ++i) {
		Coord3D pt;
		pDoc->getCoordFromCellIndex(indices[i], &pt);
		
		Real uVal;
		Coord2D start = { mStartPoint.x, mStartPoint.y };
		Coord2D end = { mEndPoint.x, mEndPoint.y };
		Coord2D pt2D = { pt.x, pt.y };
		
		ShortestDistancePointToSegment2D(&start, &end, &pt2D, NULL, NULL, &uVal);
		Real height = mStartPoint.z + uVal * (mEndPoint.z - mStartPoint.z);
		
		worldHeightDup->setHeight(indices[i].x, indices[i].y, (UnsignedByte) (height / MAP_HEIGHT_SCALE));
	}

	IRegion2D partialRange = {0,0,0,0};
	pDoc->updateHeightMap(worldHeightDup, false, partialRange);

	WBDocUndoable *pUndo = new WBDocUndoable(pDoc, worldHeightDup);
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	REF_PTR_RELEASE(worldHeightDup);


	// Once we've applied the ramp, its no longer a mutable thing, so blow away the feedback
	DrawObject::setDoRampFeedback(false);
}


