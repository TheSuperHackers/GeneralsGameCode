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

// GroveTool.cpp
// Texture tiling tool for worldbuilder.
// Author: John Ahlquist, April 2001

#include "StdAfx.h"
#include "resource.h"

#include "GroveTool.h"
#include "CUndoable.h"
#include "MainFrm.h"
#include "WHeightMapEdit.h"
#include "WorldBuilderDoc.h"
#include "WorldBuilderView.h"
#include "Common/GlobalData.h"
#include "GameLogic/LogicRandomValue.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "Common/Debug.h"
#include "DrawObject.h"
#include "GroveOptions.h"
#include "GameLogic/PolygonTrigger.h"
#include "mapobjectprops.h"

#define DRAG_THRESHOLD	5
#define MAX_TREE_RISE_OVER_RUN		(1.5f)

// surface normal Z must be greater than this to plant a tree
static Real flatTolerance = 0.8f;

static Bool _positionIsTooCliffyForTrees(Coord3D pos);

//-------------------------------------------------------------------------------------------------
/** See if a location is underwater, and what the water height is. */
//-------------------------------------------------------------------------------------------------
Bool localIsUnderwater( Real x, Real y)
{
	ICoord3D iLoc;
	iLoc.x = (floor(x+0.5f));
	iLoc.y = (floor(y+0.5f));
	iLoc.z = 0;
	// Look for water areas.
	for (PolygonTrigger *pTrig=PolygonTrigger::getFirstPolygonTrigger(); pTrig; pTrig = pTrig->getNext()) {
		if (!pTrig->isWaterArea()) {
			continue;
		}
		// See if point is in a water area
		if (pTrig->pointInTrigger(iLoc)) {
			Real wZ = pTrig->getPoint(0)->z;
			// See if the ground height is less than the water level.
			Real curHeight = TheTerrainRenderObject->getHeightMapHeight(x, y, NULL);
			return (curHeight<wZ);
		}
	}
	return false;
}



// plant a tree
void GroveTool::plantTree( Coord3D *pos )
{
	AsciiString treeName;
	int totalValue = TheGroveOptions->getTotalTreePerc();
	int randVal = GameLogicRandomValue(0, totalValue - 1);


	int runningCum = 0;
	for (int i = 1; i <= 5; ++i) {
		runningCum +=  TheGroveOptions->getNumType(i);
		if (randVal < runningCum) {
			if (TheGroveOptions->getTypeName(i).isEmpty()) {
				return;
			}

			treeName = TheGroveOptions->getTypeName(i).str();
			break;
		}
	}

	addObj(pos, treeName);
}

void GroveTool::activate()
{
	CMainFrame::GetMainFrame()->showOptionsDialog(IDD_GROVE_OPTIONS);
	DrawObject::setDoBrushFeedback(false);
}


// plant a shrub
void GroveTool::plantShrub( Coord3D *pos )
{
// TODO: Determine when we can tell something is a shurubbery, and plant it here - jkmcd
//	addObj(pos, AsciiString("Shrub"));
}

void GroveTool::_plantGroveInBox(CPoint tl, CPoint br, WbView* pView)
{

	int numTotalTrees = TheGroveOptions->getNumTrees();

	Coord3D tl3d, tr3d, bl3d;
	pView->viewToDocCoords(tl, &tl3d);
	pView->viewToDocCoords(CPoint(tl.x, br.y), &bl3d);
	pView->viewToDocCoords(CPoint(br.x, tl.y), &tr3d);

	tl3d.z = 0;

	bl3d.x -= tl3d.x;
	bl3d.y -= tl3d.y;
	bl3d.z = 0;

	tr3d.x -= tl3d.x;
	tr3d.y -= tl3d.y;
	tr3d.z = 0;

	for (int i = 0; i < numTotalTrees; ++i) {
		Real trModifier = GameLogicRandomValueReal(0.0f, 1.0f);
		Real blModifier = GameLogicRandomValueReal(0.0f, 1.0f);

		Vector3 tlVec(tl3d.x, tl3d.y, tl3d.z);
		Vector3 trVec(tr3d.x * trModifier, tr3d.y * trModifier, tr3d.z);
		Vector3 blVec(bl3d.x * blModifier, bl3d.y * blModifier, bl3d.z);

		tlVec = tlVec + trVec;
		tlVec = tlVec + blVec;

		Coord3D position;

		position.x = tlVec.X;
		position.y = tlVec.Y;
		position.z = 0;

		// (maybe) don't put trees on steep slopes
		// (maybe) tree must not be in the water
		if (!TheGroveOptions->getCanPlaceInWater() && localIsUnderwater(position.x, position.y)) {
			continue;
		}

		if (!TheGroveOptions->getCanPlaceOnCliffs() && _positionIsTooCliffyForTrees(position)) {
			continue;
		}
		// We've satisfied our conditions. Plant the little bugger
		plantTree(&position);
	}
}

/**
 * Plant a grove of trees, recursively.  Given a "seed location", create child
 * trees nearby, and recursively call the grove function on those children.
 */
void GroveTool::plantGrove( Coord3D pos, Coord3D prevDir, Real baseHeight, Int level, CPoint bounds )
{
	Coord3D childPos, normal, dir;
	Real childChance = 1.0f;
	Real heightTolerance = 1.5f*MAP_XY_FACTOR;		// was 4
	Real angle, spread;
	Int numChildren = 2, retry, maxTries = 5;

	if (level == 1)
	{
		// plant a shrub around the outskirts of the grove
		plantShrub( &pos );
	}
	else
	{
		// plant a tree here
		plantTree( &pos );
	}

	// if reached base level, stop
	if (level == 0)
		return;


	// spawn child trees
	for( Int i=0; i<numChildren; i++ )
	{
		if (GameLogicRandomValueReal( 0.0f, 1.0f ) < childChance)
		{
			for( retry=0; retry<maxTries; retry++ )
			{
				angle = GameLogicRandomValueReal( 0.0f, 2.0f * PI );
				spread = GameLogicRandomValueReal( 3.0f*MAP_XY_FACTOR, 6.0f*MAP_XY_FACTOR );
				childPos.x = pos.x + spread * (Real)cos( angle );
				childPos.y = pos.y + spread * (Real)sin( angle );
				childPos.z = 0;
				if (TheTerrainRenderObject) {
					TheTerrainRenderObject->getHeightMapHeight( childPos.x, childPos.y, &normal );
				}

				dir.x = childPos.x - pos.x;
				dir.y = childPos.y - pos.y;

				// don't select a spot too much towards where we came from
				//	(note that if prevDir the zero vector, it passes this test (for the 1st tree))
				// don't put trees on steep slopes
				// dont' plant if height changed too much
				// tree must be on map
				// tree must not be in the water
				if (dir.x * prevDir.x + dir.y * prevDir.y >= 0.0f &&
						normal.z > flatTolerance &&
						fabs(childPos.z - baseHeight) < heightTolerance &&
						childPos.x > 0 && childPos.y > 0 &&
						childPos.x < bounds.x && childPos.y < bounds.y &&
						(!localIsUnderwater(childPos.x, childPos.y)))
					break;
			}

			if (retry < maxTries)
				plantGrove( childPos, dir, baseHeight, level-1, bounds );
		}
	}
}


//
// GroveTool class.
//
/// Constructor
GroveTool::GroveTool(void) :
	Tool(ID_GROVE_TOOL, IDC_GROVE)
{
		m_headMapObj = NULL;
}

/// Destructor
GroveTool::~GroveTool(void)
{
	if (m_headMapObj) {
		deleteInstance(m_headMapObj);
	}
}

/** Execute the tool on mouse down - Place an object. */
void GroveTool::mouseDown(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	if (m != TRACK_L) return;
	m_downPt = viewPt;
	m_dragging = false;
}

/** Execute the tool on mouse up - Place an object. */
void GroveTool::mouseUp(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	if (m != TRACK_L) return;

	if (pView && m_dragging) {
		CRect box;
		box.left = viewPt.x;
		box.top = viewPt.y;
		box.right = m_downPt.x;
		box.bottom = m_downPt.y;
		box.NormalizeRect();
		pView->doRectFeedback(false, box);
		pView->Invalidate();

		_plantGroveInBox(m_downPt, viewPt, pView);
		if (m_headMapObj != NULL) {
			AddObjectUndoable *pUndo = new AddObjectUndoable(pDoc, m_headMapObj);
			pDoc->AddAndDoUndoable(pUndo);
			REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
			m_headMapObj = NULL; // undoable owns it now.
		}
		return;
	}

	Coord3D loc;

	pView->viewToDocCoords(m_downPt, &loc);

	WorldHeightMapEdit *pMap = pDoc->GetHeightMap();
	CPoint bounds;
	bounds.x = pMap->getXExtent()*MAP_XY_FACTOR;
	bounds.y = pMap->getYExtent()*MAP_XY_FACTOR;
//	Real angle = 0;
	Int depth = 3;
	Coord3D zeroDir;

	zeroDir.x = 0.0f;
	zeroDir.y = 0.0f;
	zeroDir.z = 0.0f;
	loc.z = TheTerrainRenderObject ? TheTerrainRenderObject->getHeightMapHeight( loc.x, loc.y, NULL ) : 0;

	// grow tree grove out from here
	plantGrove( loc, zeroDir, loc.z, depth, bounds );
	if (m_headMapObj != NULL) {
		AddObjectUndoable *pUndo = new AddObjectUndoable(pDoc, m_headMapObj);
		pDoc->AddAndDoUndoable(pUndo);
		REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
		m_headMapObj = NULL; // undoable owns it now.
	}
}

void GroveTool::mouseMoved(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	if (m != TRACK_L) {
		return;
	}

	if (abs(viewPt.x - m_downPt.x) > DRAG_THRESHOLD || abs(viewPt.y - m_downPt.y) > DRAG_THRESHOLD) {
		m_dragging = true;
	}

	if (pView && m_dragging) {
		CRect box;
		box.left = viewPt.x;
		box.bottom = viewPt.y;
		box.top = m_downPt.y;
		box.right = m_downPt.x;
		box.NormalizeRect();
		pView->doRectFeedback(true, box);
		pView->Invalidate();
		return;

	}
}

void GroveTool::addObj(Coord3D *pos, AsciiString name)
{
	MapObject *pCur = ObjectOptions::getObjectNamed(name);
	DEBUG_ASSERTCRASH(pCur!=NULL, ("oops"));
	if (!pCur) return;
	Coord3D theLoc = *pos;
	theLoc.z = 0;
	Real angle = GameLogicRandomValueReal( 0.0f, 2.0f * PI );
	MapObject *pNew = newInstance( MapObject)(theLoc, pCur->getName(), angle, 0, NULL, pCur->getThingTemplate() );
	pNew->getProperties()->setAsciiString(TheKey_originalOwner, NEUTRAL_TEAM_INTERNAL_STR);
	pNew->setNextMap(m_headMapObj);
	m_headMapObj = pNew;
}


#define SQRT_2	(1.41421356f)
static Bool _positionIsTooCliffyForTrees(Coord3D pos)
{
	Coord3D otherPos;

	otherPos = pos;
	otherPos.x += MAP_XY_FACTOR;
	otherPos.z = TheTerrainRenderObject->getHeightMapHeight(otherPos.x, otherPos.y, NULL);

	if (((pos.z / otherPos.z / 1) > MAX_TREE_RISE_OVER_RUN) ||
		  ((otherPos.z / pos.z / 1) > MAX_TREE_RISE_OVER_RUN)) {
		return true;
	}

	otherPos = pos;
	otherPos.y += MAP_XY_FACTOR;
	otherPos.z = TheTerrainRenderObject->getHeightMapHeight(otherPos.x, otherPos.y, NULL);

	if (((pos.z / otherPos.z / 1) > MAX_TREE_RISE_OVER_RUN) ||
			((otherPos.z / pos.z / 1) > MAX_TREE_RISE_OVER_RUN)) {
		return true;
	}

	otherPos = pos;
	otherPos.y -= MAP_XY_FACTOR;
	otherPos.z = TheTerrainRenderObject->getHeightMapHeight(otherPos.x, otherPos.y, NULL);

	if (((pos.z / otherPos.z / 1) > MAX_TREE_RISE_OVER_RUN) ||
			((otherPos.z / pos.z / 1) > MAX_TREE_RISE_OVER_RUN)) {
		return true;
	}

	otherPos = pos;
	otherPos.x -= MAP_XY_FACTOR;
	otherPos.z = TheTerrainRenderObject->getHeightMapHeight(otherPos.x, otherPos.y, NULL);

	if (((pos.z / otherPos.z / 1) > MAX_TREE_RISE_OVER_RUN) ||
			((otherPos.z / pos.z / 1) > MAX_TREE_RISE_OVER_RUN)) {
		return true;
	}

	otherPos = pos;
	otherPos.x += MAP_XY_FACTOR;
	otherPos.y += MAP_XY_FACTOR;
	otherPos.z = TheTerrainRenderObject->getHeightMapHeight(otherPos.x, otherPos.y, NULL);

	if (((pos.z / otherPos.z / SQRT_2) > MAX_TREE_RISE_OVER_RUN) ||
			((otherPos.z / pos.z / SQRT_2) > MAX_TREE_RISE_OVER_RUN)) {
		return true;
	}

	otherPos = pos;
	otherPos.x += MAP_XY_FACTOR;
	otherPos.y -= MAP_XY_FACTOR;
	otherPos.z = TheTerrainRenderObject->getHeightMapHeight(otherPos.x, otherPos.y, NULL);

	if (((pos.z / otherPos.z / SQRT_2) > MAX_TREE_RISE_OVER_RUN) ||
			((otherPos.z / pos.z / SQRT_2) > MAX_TREE_RISE_OVER_RUN)) {
		return true;
	}

	otherPos = pos;
	otherPos.x -= MAP_XY_FACTOR;
	otherPos.y -= MAP_XY_FACTOR;
	otherPos.z = TheTerrainRenderObject->getHeightMapHeight(otherPos.x, otherPos.y, NULL);

	if (((pos.z / otherPos.z / SQRT_2) > MAX_TREE_RISE_OVER_RUN) ||
			((otherPos.z / pos.z / SQRT_2) > MAX_TREE_RISE_OVER_RUN)) {
		return true;
	}

	otherPos = pos;
	otherPos.x -= MAP_XY_FACTOR;
	otherPos.y += MAP_XY_FACTOR;
	otherPos.z = TheTerrainRenderObject->getHeightMapHeight(otherPos.x, otherPos.y, NULL);

	if (((pos.z / otherPos.z / SQRT_2) > MAX_TREE_RISE_OVER_RUN) ||
			((otherPos.z / pos.z / SQRT_2) > MAX_TREE_RISE_OVER_RUN)) {
		return true;
	}

	return false;
}
#undef SQRT_2
