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

// WaterTool.cpp
// Water area tool for worldbuilder.
// Author: John Ahlquist, April 2001

#include "StdAfx.h" 
#include "resource.h"

#include "WaterTool.h"
#include "CUndoable.h"
#include "PointerTool.h"
#include "TerrainMaterial.h"
#include "WHeightMapEdit.h"
#include "WorldBuilderDoc.h"
#include "WorldBuilderView.h"
#include "MainFrm.h"
#include "DrawObject.h"
#include "GameLogic/PolygonTrigger.h"
#include "Common/GlobalData.h"

//
// WaterTool class.
//
Bool WaterTool::m_water_isActive = false;

/// Constructor
WaterTool::WaterTool(void) 
{
	m_toolID = ID_WATER_TOOL; 
	m_cursorID = IDC_WATER;

	m_currentZ = 0;

}



/// Destructor
WaterTool::~WaterTool(void) 
{
	if (m_poly_plusCursor) {
		::DestroyCursor(m_poly_plusCursor);
	}
	if (m_poly_moveCursor) {
		::DestroyCursor(m_poly_moveCursor);
	}
}

/// Clears it's is active flag.
void WaterTool::deactivate() 
{
	PolygonTool::deactivate();
	m_water_isActive = false;
}

static Bool doIt = false;

/// Shows the no options panel.
void WaterTool::activate() 
{
	CMainFrame::GetMainFrame()->showOptionsDialog(IDD_WATER_OPTIONS);
	m_water_isActive = true;
	PointerTool::clearSelection();
	DrawObject::setDoBrushFeedback(false);
	m_poly_isActive = false;
	m_poly_isAdding = false;
	m_poly_mouseUpMove = false;
	m_poly_mouseUpPlus = false;
	m_currentZ = WaterOptions::getHeight();
	doIt = true;
}


#define WATER_FILL
#define INTENSE_DEBUG

/// Perform the tool behavior on mouse down.
void WaterTool::mouseDown(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc) 
{
	if (m != TRACK_L) return;
	if (WaterOptions::getCreatingWaterAreas()) {
		fillTheArea(m, viewPt, pView, pDoc);
		return;
	}
	Coord3D docPt;
	pView->viewToDocCoords(viewPt, &docPt);
	m_poly_unsnappedMouseDownPt = docPt;
	poly_pickOnMouseDown(viewPt, pView);
	if (!poly_snapToPoly(&docPt)) {
		pView->snapPoint(&docPt);
	}
	docPt.z = m_currentZ;
	m_poly_mouseDownPt = docPt;
	if (m_poly_curSelectedPolygon) {
		DEBUG_ASSERTCRASH(m_poly_curSelectedPolygon->isWaterArea(), ("Should be water."));
		m_currentZ = m_poly_curSelectedPolygon->getPoint(0)->z;
		m_poly_mouseDownPt.z = m_currentZ;
	}
	startMouseDown(m, viewPt, pView, pDoc);
	m_poly_curSelectedPolygon->setWaterArea(true);

}



/** Set the cursor. */
void WaterTool::setCursor(void) 
{
	if (m_poly_mouseUpPlus || (m_poly_isAdding && m_poly_curSelectedPolygon)) {
		if (m_poly_plusCursor == NULL) {
			m_poly_plusCursor = AfxGetApp()->LoadCursor(MAKEINTRESOURCE(IDC_WATER_PLUS));
		}
		::SetCursor(m_poly_plusCursor);
	} else 	if (m_poly_mouseUpMove) {
		if (m_poly_moveCursor == NULL) {
			m_poly_moveCursor = AfxGetApp()->LoadCursor(MAKEINTRESOURCE(IDC_WATER_MOVE));
		}
		::SetCursor(m_poly_moveCursor);
	} else {
		Tool::setCursor();
	}
}



/// Left button move code.
void WaterTool::mouseMoved(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc)
{
	Coord3D docPt;
	pView->viewToDocCoords(viewPt, &docPt);
	if (m == TRACK_NONE) {
		PolygonTrigger *pCur = m_poly_curSelectedPolygon;
		Int curPt = m_poly_dragPointNdx;
		m_poly_unsnappedMouseDownPt = docPt;
		poly_pickOnMouseDown(viewPt, pView);
		m_poly_mouseUpPlus = false;
		m_poly_mouseUpMove = false;
		if (m_poly_curSelectedPolygon) {
			if (poly_pickPoint(m_poly_curSelectedPolygon, viewPt, pView) >= 0) {
				m_poly_mouseUpMove = true;
			} else {
				m_poly_mouseUpPlus = true;
			}
		}
		m_poly_curSelectedPolygon = pCur;
		m_poly_dragPointNdx = curPt;
		return;	// setCursor will use the value of m_mouseUpRotate.  jba.
	}

	if (m != TRACK_L) return;
	if (!poly_snapToPoly(&docPt)) {
		pView->snapPoint(&docPt);
	}
	if (m_poly_moveUndoable) {
		ICoord3D iDocPt;
		iDocPt.x = floor(docPt.x+0.5f-m_poly_mouseDownPt.x);
		iDocPt.y = floor(docPt.y+0.5f-m_poly_mouseDownPt.y);
		iDocPt.z = 0;
		m_poly_moveUndoable->SetOffset(iDocPt);
		pView->Invalidate();
		return;
	}
	if (m_poly_dragPointNdx >= 0 && m_poly_curSelectedPolygon) {
		ICoord3D iDocPt;
		iDocPt.x = floor(docPt.x+0.5f);
		iDocPt.y = floor(docPt.y+0.5f);
		iDocPt.z = m_currentZ;
		m_poly_curSelectedPolygon->setPoint(iDocPt, m_poly_dragPointNdx);
		pView->Invalidate();
	}
}

/** Mouse up - not much. */
void WaterTool::mouseUp(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc) 
{
	if (m != TRACK_L) return;
	REF_PTR_RELEASE(m_poly_moveUndoable); // belongs to pDoc now.
}

inline static Real mapZtoHeight(UnsignedByte mapZ) {
	return (mapZ * MAP_HEIGHT_SCALE)+0.01f;
}

/// Perform the fill water area on mouse down.
void WaterTool::fillTheArea(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc) 
{
	Int waterHeight = m_currentZ;

	Coord3D docPt;
	pView->viewToDocCoords(viewPt, &docPt);

	Int pointLimit = 3000; // In case we get lost.

	PolygonTrigger *pNew = newInstance(PolygonTrigger)(100);
	AsciiString name;
	name.format("Water Area %d", pNew->getID());
	pNew->setTriggerName(name);

	CPoint ndx;
	Int width = 1;
	getCenterIndex(&docPt, width, &ndx, pDoc);
	Int i, j;
	i = ndx.x;
	j = ndx.y;
	
	WorldHeightMapEdit *pMap = pDoc->GetHeightMap(); 
	if (i<0 || i>=pMap->getXExtent()-1) return;
	if (j<0 || j>=pMap->getYExtent()-1) return;

	Int intMapHeight = pMap->getHeight(i, j);
	if (waterHeight<mapZtoHeight(intMapHeight)) {
		return;
	}

	// Move left till we find an edge.
	while (i>0) {
		i--;
		intMapHeight = pMap->getHeight(i, j);	
		if (waterHeight<mapZtoHeight(intMapHeight)) {
			break;
		}
	}

	Bool bottom = true;	 // bottom left is dry, bottom right wet.
	Bool right = false;	 // right top is wet, right bottom dry.
	Bool left = false;	 // left bottom wet, left top dry.
	Bool top = false;		 // top left wet, top right dry.

	Int startI = -1;
	Int startJ = -1;
	Bool firstTime = true;

	// Build area in index coords for now, convert later.
	ICoord3D pt;
	ICoord3D tmpPt;
	Int curPoint = 0;
	while (pointLimit>0) {
		if (curPoint>2 && i==startI && j==startJ) break;
		pointLimit--;
		pt.x = i*MAP_XY_FACTOR;
		pt.y = j*MAP_XY_FACTOR;
		pt.z = waterHeight;
		intMapHeight = pMap->getHeight(i, j);
#ifdef INTENSE_DEBUG
		if (bottom) {
			DEBUG_LOG(("Bottom %d,%d", i, j));
		} else if (left) {
			DEBUG_LOG(("Left %d,%d", i, j));
		} else if (right) {
			DEBUG_LOG(("Right %d,%d", i, j));
		} else if (top) {
			DEBUG_LOG(("Top %d,%d", i, j));
		}
#endif
		if (bottom) {
			bottom = false;
			if (waterHeight<mapZtoHeight(intMapHeight)) {
				DEBUG_ASSERTCRASH(waterHeight>mapZtoHeight(pMap->getHeight(i+1, j)), ("Logic error. jba."));
				Real dx = (mapZtoHeight(pMap->getHeight(i,j))-waterHeight) / (mapZtoHeight(pMap->getHeight(i,j)-pMap->getHeight(i+1,j)));
				pt.x += dx*MAP_XY_FACTOR;
				Bool topLeftDry = (waterHeight<mapZtoHeight(pMap->getHeight(i, j+1)));
				Bool topRightDry = (waterHeight<mapZtoHeight(pMap->getHeight(i+1, j+1)));
				if (topRightDry) {
					i++;
					left = true;
				}	else if (topLeftDry) {
					if (j<pMap->getYExtent()-2) {
						j++;
						bottom = true;
					}	else {
						tmpPt = pt;
						pt.y += MAP_XY_FACTOR;
						tmpPt.x -= pMap->getBorderSize()*MAP_XY_FACTOR;
						tmpPt.y -= pMap->getBorderSize()*MAP_XY_FACTOR;
						pNew->addPoint(tmpPt);
						left = true;
						i++;
					}
				}	else {
					if (i>0) {
						i--;
						right = true;
					} else {
						j++;
						bottom = true;
					}
				}
			} else {
				DEBUG_ASSERTCRASH(i==0, ("Logic error. jba."));
				left = true;
				while (j<pMap->getYExtent()-2 && waterHeight>mapZtoHeight(pMap->getHeight(i, j+1))) {
					j++;
				}
				pt.y = (j)*MAP_XY_FACTOR;
				if (waterHeight>mapZtoHeight(pMap->getHeight(i, j+1))) {
					pt.y = (j+1)*MAP_XY_FACTOR;
				}	else {
					continue;
				}

			}
		} else if (left) {
			left = false;
			if (waterHeight<mapZtoHeight(pMap->getHeight(i, j+1))) {
				DEBUG_ASSERTCRASH(waterHeight>mapZtoHeight(intMapHeight), ("Logic error. jba."));
				Real dy = (waterHeight-mapZtoHeight(pMap->getHeight(i,j))) / (mapZtoHeight(pMap->getHeight(i,j+1)-pMap->getHeight(i,j)));
				pt.y += dy*MAP_XY_FACTOR;
				Bool bottomRightDry = (waterHeight<mapZtoHeight(pMap->getHeight(i+1, j)));
				Bool topRightDry = (waterHeight<mapZtoHeight(pMap->getHeight(i+1, j+1)));
				if (bottomRightDry) {
					j--;
					top = true;
				}	else if (topRightDry) {
					if (i<pMap->getXExtent()-2) {
						i++;
						left = true;
					} else {
						tmpPt = pt;
						pt.x += MAP_XY_FACTOR;
						tmpPt.x -= pMap->getBorderSize()*MAP_XY_FACTOR;
						tmpPt.y -= pMap->getBorderSize()*MAP_XY_FACTOR;
						pNew->addPoint(tmpPt);
						top = true;
						j--;
					}
				}	else {
					if (j<pMap->getYExtent()-1) {
						j++;
						bottom = true;
					} else {
						i++;
						left = true;
					}
				}
			} else {
				pt.y = (j+1)*MAP_XY_FACTOR;
				DEBUG_ASSERTCRASH(j==pMap->getYExtent()-2, ("Logic error. jba."));
				while (i<pMap->getXExtent()-2 && waterHeight>mapZtoHeight(pMap->getHeight(i+1, j+1))) {
					i++;
				}
				top = true;
				pt.x = i*MAP_XY_FACTOR;
				if (waterHeight>mapZtoHeight(pMap->getHeight(i+1, j+1))) {
					pt.x = (i+1)*MAP_XY_FACTOR;
				} else {
					continue;
				}
			}
		} else if (right) {
			right = false;
			pt.x = (i+1)*MAP_XY_FACTOR;
			if (waterHeight<mapZtoHeight(pMap->getHeight(i+1, j))) {
				DEBUG_ASSERTCRASH(waterHeight>mapZtoHeight(pMap->getHeight(i+1,j+1)), ("Logic error. jba."));
				Real dy = (mapZtoHeight(pMap->getHeight(i+1,j))-waterHeight) / (mapZtoHeight(pMap->getHeight(i+1,j)-pMap->getHeight(i+1,j+1)));
				pt.y += dy*MAP_XY_FACTOR;
				Bool bottomLeftDry = (waterHeight<mapZtoHeight(pMap->getHeight(i, j)));
				Bool topLeftDry = (waterHeight<mapZtoHeight(pMap->getHeight(i, j+1)));
				if (topLeftDry) {
					j++;
					bottom = true;
				}	else if (bottomLeftDry) {
					if (i>0) {
						i--;
						right = true;
					}	else {
						tmpPt = pt;
						pt.x -= MAP_XY_FACTOR;
						tmpPt.x -= pMap->getBorderSize()*MAP_XY_FACTOR;
						tmpPt.y -= pMap->getBorderSize()*MAP_XY_FACTOR;
						pNew->addPoint(tmpPt);
						bottom = true;
						j++;
					}
				}	else {
					if (j>0) {
						j--;
						top = true;
					} else {
						i--;
						right = true;
					}
				}
			} else {
				DEBUG_ASSERTCRASH(j==0, ("Logic error. jba."));
				while (i>0 && waterHeight>mapZtoHeight(pMap->getHeight(i, j))) {
					i--;
				}
				bottom = true;
				pt.x = (i)*MAP_XY_FACTOR;
				if (waterHeight<mapZtoHeight(pMap->getHeight(i, j)))	{
					continue;
				}
			}
		} else if (top) {
			top = false;
			pt.y = (j+1)*MAP_XY_FACTOR;
			if (waterHeight<mapZtoHeight(pMap->getHeight(i+1, j+1))) {
				DEBUG_ASSERTCRASH(waterHeight>mapZtoHeight(pMap->getHeight(i, j+1)), ("Logic error. jba."));
				Real dx = (waterHeight-mapZtoHeight(pMap->getHeight(i,j+1))) / (mapZtoHeight(pMap->getHeight(i+1,j+1)-pMap->getHeight(i,j+1)));
				pt.x += dx*MAP_XY_FACTOR;
				Bool bottomLeftDry = (waterHeight<mapZtoHeight(pMap->getHeight(i, j)));
				Bool bottomRightDry = (waterHeight<mapZtoHeight(pMap->getHeight(i+1, j)));
				if (bottomLeftDry) {
					i--;
					right = true;
				}	else if (bottomRightDry) {
					if (j>0) {
						j--;
						top = true;
					}	else {
						tmpPt = pt;
						pt.y -= MAP_XY_FACTOR;
						tmpPt.x -= pMap->getBorderSize()*MAP_XY_FACTOR;
						tmpPt.y -= pMap->getBorderSize()*MAP_XY_FACTOR;
						pNew->addPoint(tmpPt);
						i--;
						right = true;
					}
				}	else {
					if (i<pMap->getXExtent()-2) {
						i++;
						left = true;
					} else {
						j--;
						top = true;
					}
				}
			} else {
				DEBUG_ASSERTCRASH(i==pMap->getXExtent()-2, ("Logic error. jba."));
				while (j>0 && waterHeight>mapZtoHeight(pMap->getHeight(i+1, j))) {
					j--;
				}
				right = true;
				pt.y = (j+1)*MAP_XY_FACTOR;
				if (waterHeight>mapZtoHeight(pMap->getHeight(i, j))) {
					pt.y = (j)*MAP_XY_FACTOR;
				} else {
					continue;
				}
				pt.x = (i+1)*MAP_XY_FACTOR;
			}
		} else { 
			DEBUG_CRASH(("Logic error. jba.")); // shouldn't get here.
		}
		pt.x -= pMap->getBorderSize()*MAP_XY_FACTOR;
		pt.y -= pMap->getBorderSize()*MAP_XY_FACTOR;
		pNew->addPoint(pt);	
		curPoint++;
		if (firstTime) {
			startI = i;
			startJ = j;
			firstTime = false;
		}

	}
	
	if (pNew->getNumPoints()>2) {
		PolygonTrigger *pBetter = adjustSpacing(pNew, WaterOptions::getSpacing());
		deleteInstance(pNew);
		pNew = pBetter;
		pNew->setWaterArea(true);
		AddPolygonUndoable *pUndo = new AddPolygonUndoable(pNew);
		pDoc->AddAndDoUndoable(pUndo);
		REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
		m_poly_curSelectedPolygon = pNew;
		m_poly_dragPointNdx = -1;
		WaterOptions::update();
	}	else {
		deleteInstance(pNew);
	}

}

/// Adjust the spacing.
PolygonTrigger * WaterTool::adjustSpacing(PolygonTrigger *trigger, Real spacing)
{
	PolygonTrigger *pNew = newInstance(PolygonTrigger)(trigger->getNumPoints());
//	Real endLen=0;
	Int i;
	Real curSpacingLen = spacing;
	ICoord3D pt = *trigger->getPoint(0);
	pNew->addPoint(pt);
	Bool didCurPoint = true;
	for (i=0; i<trigger->getNumPoints()-1; i++) {
		ICoord3D curPt = *trigger->getPoint(i);
		ICoord3D nextPt = *trigger->getPoint(i+1);
		Real dx = nextPt.x-curPt.x;
		Real dy = nextPt.y-curPt.y;
		Real curLen = sqrt(dx*dx+dy*dy);
		if (curLen > 4*MAP_XY_FACTOR && curLen>2*spacing) {
			if (!didCurPoint) pNew->addPoint(curPt);
			else pNew->setPoint(curPt, pNew->getNumPoints()-1);
			pNew->addPoint(nextPt);
			didCurPoint = true;
			curSpacingLen = spacing;
		} else if (curSpacingLen>curLen) {
			curSpacingLen -= curLen;
		} else {
			while (curLen >= curSpacingLen) {
				// cur len > curSpacingLen.
				Real factor = curSpacingLen/curLen;
				curPt.x += dx*factor;
				curPt.y += dy*factor;
				pNew->addPoint(curPt);
				didCurPoint = false;
				dx = nextPt.x-curPt.x;
				dy = nextPt.y-curPt.y;
				curLen -= curSpacingLen;
				curSpacingLen = spacing;
			}
			curSpacingLen -= curLen;
 			if ((curLen)<MAP_XY_FACTOR/2) {
				didCurPoint = true;
			}
		}
	}
	pNew->setTriggerName(trigger->getTriggerName());
	return pNew;
}
