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

// RoadTool.cpp
// Texture tiling tool for worldbuilder.
// Author: John Ahlquist, April 2001

#include "StdAfx.h"
#include "resource.h"

#include "RoadTool.h"
#include "CUndoable.h"
#include "DrawObject.h"
#include "PointerTool.h"
#include "MainFrm.h"
#include "WHeightMapEdit.h"
#include "WorldBuilderDoc.h"
#include "WorldBuilderView.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "Common/WellKnownKeys.h"
#include "Common/ThingTemplate.h"
#include "Common/ThingFactory.h"
#include "GameClient/Line2D.h"
//
// RoadTool class.
//
/// Constructor
RoadTool::RoadTool(void) :
	Tool(ID_ROAD_TOOL, IDC_ROAD)
{
	m_mapObj = NULL;
}

/// Destructor
RoadTool::~RoadTool(void)
{
	m_mapObj = NULL;
}
//-----------------------------------------------------------------------------
//         Public Functions
//-----------------------------------------------------------------------------

MapObject* RoadTool::findSegment(const Coord3D *pLoc, Coord3D *outLoc)
{
	for (MapObject* pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext())
	{
		if (pMapObj->getFlag(FLAG_ROAD_POINT1))
		{
			MapObject* pMapObj2 = pMapObj->getNext();
			if (pMapObj2==NULL)
				break;
			if (!pMapObj2->getFlag(FLAG_ROAD_POINT2))
				continue;
			Coord2D start, end, loc, snapLoc;
			start.x = pMapObj->getLocation()->x;
			start.y = pMapObj->getLocation()->y;
			end.x = pMapObj2->getLocation()->x;
			end.y = pMapObj2->getLocation()->y;
			loc.x = pLoc->x;
			loc.y = pLoc->y;
			Real dist;
			Real u;

			ShortestDistancePointToSegment2D(&start, &end, &loc, NULL, &snapLoc, &u);
			if (u < 0 || u > 1) {
				continue;
			}
			Coord2D segment;
			segment.x = loc.x - snapLoc.x;
			segment.y = loc.y - snapLoc.y;
			dist = segment.length();

			if (dist < ROAD_SNAP_DISTANCE*MAP_XY_FACTOR)
			{
				outLoc->x = snapLoc.x;
				outLoc->y = snapLoc.y;
				outLoc->z = MAGIC_GROUND_Z;
				return(pMapObj);
			}
		}
	}
	return NULL;
}

//=============================================================================
// RoadTool::snap
//=============================================================================
/** Snaps pLoc to another road endpoint if close enough. */
//=============================================================================
Bool RoadTool::snap(Coord3D *pLoc, Bool skipFirst)
{
	MapObject *pMapObj;
	MapObject *pMapObj2;
	Real snapDist = ROAD_SNAP_DISTANCE*MAP_XY_FACTOR;
	Coord3D newLoc = *pLoc;
	Bool snapped = false;

	for (pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext()) {
		if (skipFirst) {
			skipFirst = false;
			continue;
		}
		if (pMapObj->getFlag(FLAG_ROAD_POINT1)) {
			pMapObj2 = pMapObj->getNext();
			if (pMapObj2==NULL) break;
			if (!pMapObj2->getFlag(FLAG_ROAD_POINT2)) continue;
			Vector2 dist;
			if (!pMapObj->isSelected()) {
				dist.Set(pMapObj->getLocation()->x - pLoc->x, pMapObj->getLocation()->y - pLoc->y);
				if (dist.Length() < snapDist) {
					newLoc = *pMapObj->getLocation();
					snapDist = dist.Length();
					snapped = true;
				}
			}
			if (!pMapObj2->isSelected()) {
				dist.Set(pMapObj2->getLocation()->x - pLoc->x, pMapObj2->getLocation()->y - pLoc->y);
				if (dist.Length() < snapDist) {
					newLoc = *pMapObj2->getLocation();
					snapDist = dist.Length();
					snapped = true;
				}
			}
		}
	}
	newLoc.z = MAGIC_GROUND_Z; // roads always snap to terrain.
	if (snapped) {
		*pLoc = newLoc;
	}
	return snapped;
}


/// Shows the road options panel.
void RoadTool::activate()
{
	PointerTool::clearSelection();
	CMainFrame::GetMainFrame()->showOptionsDialog(IDD_ROAD_OPTIONS);
	RoadOptions::updateSelection();
	DrawObject::setDoBrushFeedback(false);
}

/** Execute the tool on mouse up - Place a road segment. */
void RoadTool::mouseDown(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	if (m != TRACK_L) return;

	Coord3D cpt;
	pView->viewToDocCoords(viewPt, &cpt);

	Coord3D loc1, loc2, loc3;
	loc1 = cpt;
	if (!pDoc->getCellPositionFromCoord(cpt, &loc1))
		return;

	Bool isBridge = RoadOptions::isBridge();

	if (isBridge) {
		Bool isLandmark = false;
		const ThingTemplate *tt = TheThingFactory->findTemplate(RoadOptions::getCurRoadName());
		if (tt) {
			isLandmark = tt->isBridge();
		}
		if (isLandmark) {
			MapObject *pNew1 = newInstance(MapObject)(loc1, RoadOptions::getCurRoadName(), 0.0f, 0, NULL, tt );
			pNew1->getProperties()->setAsciiString(TheKey_originalOwner, NEUTRAL_TEAM_INTERNAL_STR);
			AddObjectUndoable *pUndo = new AddObjectUndoable(pDoc, pNew1);
			pNew1->setSelected(true);
			pDoc->AddAndDoUndoable(pUndo);
			REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
			m_mapObj = NULL;
			return;
		}
	}


	Bool snapped = false;
	Bool divideSegment = false;
	MapObject* pickedSegment = NULL;
	if (!isBridge) {
		snapped = snap(&loc1, false);
		if (!snapped) {
			pickedSegment = findSegment(&loc1, &loc2);
			if (pickedSegment) {
				snapped = true;
				divideSegment = true;
				loc1 = *pickedSegment->getLocation();
				loc3 = *pickedSegment->getNext()->getLocation();
			}
		}
	}
	if (!snapped) {
		pView->snapPoint(&loc1);
	}

	if (!divideSegment) {
		loc2 = loc1;
	}

	loc1.z = MAGIC_GROUND_Z; // roads stick to terrain anyway.
	loc2.z = MAGIC_GROUND_Z;
	loc3.z = MAGIC_GROUND_Z;
	AsciiString roadName;
	if (divideSegment) {
		roadName = pickedSegment->getName();
	} else {
		roadName = RoadOptions::getCurRoadName();
	}

	MapObject *pNew1 = newInstance(MapObject)(loc1, roadName, 0.0f, 0, NULL, NULL );
	MapObject *pNew2 = newInstance(MapObject)(loc2, roadName, 0.0f, 0, NULL, NULL );
	MapObject *pNew3 = NULL;
	MapObject *pNew4 = NULL;
	if (divideSegment) {
		pNew3 = newInstance(MapObject)(loc2, roadName, 0.0f, 0, NULL, NULL );
		pNew4 = newInstance(MapObject)(loc3, roadName, 0.0f, 0, NULL, NULL );
	}

	pNew1->setColor(RGB(255,255,0));	// make road endpoints yellow.
	pNew2->setColor(RGB(255,255,0));	// make road endpoints yellow.
	if (divideSegment) {
		pNew3->setColor(RGB(255,255,0));	// make road endpoints yellow.
		pNew4->setColor(RGB(255,255,0));	// make road endpoints yellow.
	}

	if (RoadOptions::isBridge()) {
		pNew1->setFlag(FLAG_BRIDGE_POINT1);
		pNew2->setFlag(FLAG_BRIDGE_POINT2);
	} else {
		pNew1->setFlag(FLAG_ROAD_POINT1);
		pNew2->setFlag(FLAG_ROAD_POINT2);
		if (divideSegment) {
			pNew3->setFlag(FLAG_ROAD_POINT1);
			pNew4->setFlag(FLAG_ROAD_POINT2);
		}
	}

	if (divideSegment) {
		pNew1->setFlag(pickedSegment->getFlags());
		pNew4->setFlag(pickedSegment->getNext()->getFlags());
	} else {
		if (RoadOptions::isAngled()) {
			pNew1->setFlag(FLAG_ROAD_CORNER_ANGLED);
			pNew2->setFlag(FLAG_ROAD_CORNER_ANGLED);
		} else if (RoadOptions::isTightCurve()) {
			pNew1->setFlag(FLAG_ROAD_CORNER_TIGHT);
			pNew2->setFlag(FLAG_ROAD_CORNER_TIGHT);
		}
	}

	// Roads belong to the neutral player. :)
	pNew1->getProperties()->setAsciiString(TheKey_originalOwner, NEUTRAL_TEAM_INTERNAL_STR);
	pNew2->getProperties()->setAsciiString(TheKey_originalOwner, NEUTRAL_TEAM_INTERNAL_STR);
	pNew1->setNextMap(pNew2);
	m_mapObj = pNew2;

	if (divideSegment) {
		// Roads belong to the neutral player. :)
		pNew3->getProperties()->setAsciiString(TheKey_originalOwner, NEUTRAL_TEAM_INTERNAL_STR);
		pNew4->getProperties()->setAsciiString(TheKey_originalOwner, NEUTRAL_TEAM_INTERNAL_STR);
		pNew3->setNextMap(pNew4);
	}

	if (divideSegment) {
		PointerTool::clearSelection();
		pickedSegment->setSelected(true);
		DeleteObjectUndoable *pDeleteUndo = new DeleteObjectUndoable(pDoc);
		pDoc->AddAndDoUndoable(pDeleteUndo);
		REF_PTR_RELEASE(pDeleteUndo); // belongs to pDoc now.
	}
	// Clear the selection.
	PointerTool::clearSelection();

	AddObjectUndoable *pUndo = new AddObjectUndoable(pDoc, pNew1);
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	m_mapObj->setSelected(true);

	if (divideSegment) {
		AddObjectUndoable *pUndo = new AddObjectUndoable(pDoc, pNew3);
		pDoc->AddAndDoUndoable(pUndo);
		REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
		m_mapObj = NULL;
		PointerTool::clearSelection();
		pNew2->setSelected(true);
		pNew3->setSelected(true);
	}

}
/** Move the end of the road segment. */
void RoadTool::mouseMoved(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	if (m != TRACK_L) return;

	Coord3D loc1 ;
	if (m_mapObj == NULL) {
		return;
	}

	Coord3D cpt;
	pView->viewToDocCoords(viewPt, &cpt);

	loc1.x=loc1.y=loc1.z=0;
	if (!pDoc->getCellPositionFromCoord(cpt, &loc1))
		return;

	Bool isBridge = RoadOptions::isBridge();
	loc1.z = MAGIC_GROUND_Z;
	Bool snapped = false;
	if (!isBridge) {
		snapped = snap(&loc1, false);
	}
	if (!snapped) {
		pView->snapPoint(&loc1);
	}
	pDoc->invalObject(m_mapObj);
	m_mapObj->setLocation(&loc1);
	pDoc->invalObject(m_mapObj);
}

void RoadTool::mouseUp(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	if (m != TRACK_L) return;
	m_mapObj = NULL;
}

