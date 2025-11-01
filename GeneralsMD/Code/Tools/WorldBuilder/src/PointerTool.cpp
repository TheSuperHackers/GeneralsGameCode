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

// PointerTool.cpp
// Texture tiling tool for worldbuilder.
// Author: John Ahlquist, April 2001

#include "StdAfx.h" 
#include "resource.h"

#include "PointerTool.h"
#include "CUndoable.h"
#include "MainFrm.h"
#include "WHeightMapEdit.h"
#include "WorldBuilderDoc.h"
#include "WorldBuilderView.h"
#include "GameLogic/SidesList.h"
#include "Common/ThingSort.h"
#include "Common/ThingTemplate.h"
#include "GameLogic/PolygonTrigger.h"
#include "wbview3d.h"
#include "ObjectTool.h"
#include "ToastDialog.h"


CString PointerTool::m_lastPointerInfo = _T("");
Bool PointerTool::m_isMouseDown = false;
Bool PointerTool::m_dragSelect = false;
Bool PointerTool::m_pointerIsActive = false;

static Bool g_PointerToolTip = false;

//
// Static helper functions
// This function spiders out and un/picks all Waypoints that have some form of indirect contact with this point
// Has a recursive helper function as well.
//
static void helper_pickAllWaypointsInPath( Int sourceID, CWorldBuilderDoc *pDoc, const Int numWaypointLinks, std::vector<Int>& alreadyTouched );

static void pickAllWaypointsInPath( Int sourceID, Bool select )
{
	std::vector<Int> alreadyTouched;
	CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();

	helper_pickAllWaypointsInPath(sourceID, pDoc, pDoc->getNumWaypointLinks(), alreadyTouched);
	
	// already touched should now be filled with waypointIDs that want to be un/selected
	MapObject *pMapObj = MapObject::getFirstMapObject();
	while (pMapObj) {
		if (pMapObj->isWaypoint()) {
			if (std::find(alreadyTouched.begin(), alreadyTouched.end(), pMapObj->getWaypointID()) != alreadyTouched.end()) {
				pMapObj->setSelected(select);
			}
		}
		pMapObj = pMapObj->getNext();
	}
}

static void helper_pickAllWaypointsInPath( Int sourceID, CWorldBuilderDoc *pDoc, const Int numWaypointLinks, std::vector<Int>& alreadyTouched )
{
	if (std::find(alreadyTouched.begin(), alreadyTouched.end(), sourceID) != alreadyTouched.end() ) {
		return;
	}

	alreadyTouched.push_back(sourceID);
	for (int i = 0; i < numWaypointLinks; ++i) {
		Int way1, way2;
		pDoc->getWaypointLink(i, &way1, &way2);
		if (way1 == sourceID) {
			helper_pickAllWaypointsInPath(way2, pDoc, numWaypointLinks, alreadyTouched);
		}

		if (way2 == sourceID) {
			helper_pickAllWaypointsInPath(way1, pDoc, numWaypointLinks, alreadyTouched);
		}
	}
}

//
// PointerTool class.
//

/// Constructor
PointerTool::PointerTool(void) :
	m_modifyUndoable(NULL),
	m_curObject(NULL),
	m_rotateCursor(NULL),
	m_moveCursor(NULL),
	m_rotateObjectsWithGroup(true),
	m_useFarthestObjectPivot(true)
{
	m_toolID = ID_POINTER_TOOL;
	m_cursorID = IDC_POINTER; 

}
	
/// Destructor
PointerTool::~PointerTool(void) 
{
	REF_PTR_RELEASE(m_modifyUndoable); // belongs to pDoc now.
	if (m_rotateCursor) {
		::DestroyCursor(m_rotateCursor);
	}
	if (m_moveCursor) {
		::DestroyCursor(m_moveCursor);
	}
}


/**
 * Adriane [Deathscythe]
 * Edited the panel logic so it only shows the Map Objects panel when an object is clicked;
 * otherwise, it hides the panel to save screen space.
 */
/// See if a single obj is selected that has properties.
void PointerTool::checkForPropertiesPanel(void) 
{
	MapObject *theMapObj = WaypointOptions::getSingleSelectedWaypoint();
	PolygonTrigger *theTrigger = WaypointOptions::getSingleSelectedPolygon();
	MapObject *theLightObj = LightOptions::getSingleSelectedLight(); 
	MapObject *theObj = MapObjectProps::getSingleSelectedObject(); 
	if (theMapObj) {
		CMainFrame::GetMainFrame()->showOptionsDialog(IDD_WAYPOINT_OPTIONS);
		WaypointOptions::update();
	} else if (theTrigger) { 
		if (theTrigger->isWaterArea()) {
			CMainFrame::GetMainFrame()->showOptionsDialog(IDD_WATER_OPTIONS);
			WaterOptions::update();
		} else {
			CMainFrame::GetMainFrame()->showOptionsDialog(IDD_WAYPOINT_OPTIONS);
			WaypointOptions::update();
		}
	} else if (theLightObj) {
		CMainFrame::GetMainFrame()->showOptionsDialog(IDD_LIGHT_OPTIONS);
		LightOptions::update();
	} else if (RoadOptions::selectionIsRoadsOnly()) {
		CMainFrame::GetMainFrame()->showOptionsDialog(IDD_ROAD_OPTIONS);
		RoadOptions::updateSelection();
	} else if (theObj) {
		CMainFrame::GetMainFrame()->showOptionsDialog(IDD_MAPOBJECT_PROPS);
		MapObjectProps::update();
		ObjectOptions::selectObject(theObj);
	} else {
		CMainFrame::GetMainFrame()->showOptionsDialog(IDD_NO_OPTIONS);
		// Nothing relevant selected -- hide the current options panel
		// if (CMainFrame::GetMainFrame()->m_curOptions) {
		//     CMainFrame::GetMainFrame()->m_curOptions->ShowWindow(SW_HIDE);
		//     CMainFrame::GetMainFrame()->m_curOptions = NULL;
		// }
	}
}


/// Clear the selection..
void PointerTool::clearSelection(void) ///< Clears the selected objects selected flags.
{
	// Clear selection.
	MapObject *pObj = MapObject::getFirstMapObject();
	while (pObj) {
		if (pObj->isSelected()) {
			pObj->setSelected(false);
		}
		pObj = pObj->getNext();
	}
	// Clear selected build list items.
	Int i;
	for (i=0; i<TheSidesList->getNumSides(); i++) {
		SidesInfo *pSide = TheSidesList->getSideInfo(i); 
		for (BuildListInfo *pBuild = pSide->getBuildList(); pBuild; pBuild = pBuild->getNext()) {
			if (pBuild->isSelected()) {
				pBuild->setSelected(false);
			}
		}
	}
	m_poly_curSelectedPolygon = NULL;
}

/// Activate.
void PointerTool::activate() 
{
	Tool::activate();
	m_mouseUpRotate = false;
	m_mouseUpMove = false;
	m_pointerIsActive = true;
	checkForPropertiesPanel();
	CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
	if (pDoc==NULL) return;
	WbView3d *p3View = pDoc->GetActive3DView();
	p3View->setObjTracking(NULL, m_downPt3d, 0, false);
}

/// deactivate.
void PointerTool::deactivate() 
{
	m_curObject = NULL;
	m_pointerIsActive = false;
	PolygonTool::deactivate();
}

/** Set the cursor. */
void PointerTool::setCursor(void) 
{
	if (m_mouseUpRotate) {
		if (m_rotateCursor == NULL) {
			m_rotateCursor = AfxGetApp()->LoadCursor(MAKEINTRESOURCE(IDC_ROTATE));
		}
		::SetCursor(m_rotateCursor);
	} else 	if (m_mouseUpMove) {
		if (m_moveCursor == NULL) {
			m_moveCursor = AfxGetApp()->LoadCursor(MAKEINTRESOURCE(IDC_MOVE_POINTER));
		}
		::SetCursor(m_moveCursor);
	} else {
		Tool::setCursor();
	}
}

Bool PointerTool::allowPick(MapObject* pMapObj, WbView* pView)
{
	EditorSortingType sort = ES_NONE;
	if (!pMapObj) {
		return false;
	} 

	const ThingTemplate *tt = pMapObj->getThingTemplate();
	if (tt && tt->getEditorSorting() == ES_AUDIO) {
		if (pView->GetPickConstraint() == ES_NONE || pView->GetPickConstraint() == ES_AUDIO) {
			return true;
		}
	}
	
	// Early reject roads if showRoads = false
	if (pMapObj->getFlag(FLAG_ROAD_FLAGS) && !pView->getShowRoads()) {
		return false;
	}

	// Early reject if models are hidden or object is invisible
	if ((tt && !pView->getShowModels()) || (pMapObj->getFlags() & FLAG_DONT_RENDER)) {
		return false;
	}

	// === Adriane [ Deathscythe ] NEW LOGIC: reject bridge points depending on constraint ===
	if (pView->GetPickConstraint() != ES_NONE && pView->GetPickConstraint() != ES_ROAD) {
		if (pMapObj->getFlag(FLAG_BRIDGE_POINT1) || pMapObj->getFlag(FLAG_BRIDGE_POINT2)) {
			return false;
		}
	}

	// === Adriane [ Deathscythe ] NEW LOGIC: reject Scorchmarks ===
	if (pView->GetPickConstraint() != ES_NONE && pView->GetPickConstraint() != ES_DEBRIS) {
		if (pMapObj->isScorch()) {
			return false;
		}
	}

	// Apply pick constraint type checks
	if (pView->GetPickConstraint() != ES_NONE) {
		if (tt) {
			if (!pView->getShowModels()) {
				return false;
			}
			sort = tt->getEditorSorting();
		} else {
			if (pMapObj->isWaypoint()) {
				sort = ES_WAYPOINT;
			}
			if (pMapObj->getFlag(FLAG_ROAD_FLAGS)) {
				if(!pView->getShowRoads()){
					return false;
				} else {
					sort = ES_ROAD;
				}
			}
		}
		if (sort != ES_NONE && sort != pView->GetPickConstraint()) {
			return false;
		}
	}

	return true;
}

/** Execute the tool on mouse down - Pick an object. */
void PointerTool::mouseDown(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc) 
{
	if (m != TRACK_L) return;

	Coord3D cpt;
	pView->viewToDocCoords(viewPt, &cpt);
	Coord3D loc;

	m_downPt2d = viewPt;
	m_downPt3d = cpt;
	pView->snapPoint(&m_downPt3d);
	m_moving = false;
	m_rotating = false;
	m_dragSelect = false;
	Bool shiftKey = (0x8000 & ::GetAsyncKeyState(VK_SHIFT))!=0;
	Bool ctrlKey = (0x8000 & ::GetAsyncKeyState(VK_CONTROL))!=0;

	m_doPolyTool = false;
	if (pView->GetPickConstraint() == ES_NONE || pView->GetPickConstraint() == ES_WAYPOINT) {
		// If polygon triggers are visible, see if we clicked on one.
		if (pView->isPolygonTriggerVisible()) {
			m_poly_unsnappedMouseDownPt = cpt;
			poly_pickOnMouseDown(viewPt, pView);
			if (m_poly_curSelectedPolygon) {
				// picked on one.
				if (!poly_snapToPoly(&cpt)) {
					pView->snapPoint(&cpt);
				}
				m_poly_mouseDownPt = cpt;
				m_poly_justPicked = true; // Makes poly tool move instead of inserting.
				m_doPolyTool = true;
				PolygonTool::startMouseDown(m, viewPt, pView, pDoc);
				return;
			}
			m_poly_curSelectedPolygon = NULL;
			m_poly_dragPointNdx = -1;
		}
	}



//	WorldHeightMapEdit *pMap = pDoc->GetHeightMap();
	m_curObject = NULL;
	MapObject *pObj = MapObject::getFirstMapObject();
	MapObject *p3DObj = pView->picked3dObjectInView(viewPt);
	MapObject *pClosestPicked = NULL;
	if (allowPick(p3DObj, pView)) {
		pClosestPicked = p3DObj;
	}
	Real pickDistSqr = 10000*MAP_XY_FACTOR;
	pickDistSqr *= pickDistSqr;

	// Find the closest pick.
	for (pObj = MapObject::getFirstMapObject(); pObj; pObj = pObj->getNext()) {
		if (!allowPick(pObj, pView)) {
			continue;
		}
		Bool picked = (pView->picked(pObj, cpt) != PICK_NONE);
		if (picked) {
			loc = *pObj->getLocation();
			Real dx = m_downPt3d.x-loc.x;
			Real dy = m_downPt3d.y-loc.y;
			Real distSqr = dx*dx+dy*dy;
			if (distSqr < pickDistSqr) {
				pClosestPicked = pObj;
				pickDistSqr = distSqr;
			}
		}
	}

	Bool anySelected = (pClosestPicked!=NULL);
	if (shiftKey) {
		if (pClosestPicked && pClosestPicked->isSelected()) {
			pClosestPicked->setSelected(false);
			if (ctrlKey && pClosestPicked->isWaypoint()) {
				pickAllWaypointsInPath(pClosestPicked->getWaypointID(), false);
			}
		} else if (pClosestPicked) {
			pClosestPicked->setSelected(true);
			if (ctrlKey && pClosestPicked->isWaypoint()) {
				pickAllWaypointsInPath(pClosestPicked->getWaypointID(), true);
			}
		}
	} else if (pClosestPicked && pClosestPicked->isSelected()) {
		// We picked a selected object
			m_curObject = pClosestPicked;
	} else {
		clearSelection();
		if (pClosestPicked) {
			pClosestPicked->setSelected(true);
			if (ctrlKey && pClosestPicked->isWaypoint()) {
				pickAllWaypointsInPath(pClosestPicked->getWaypointID(), true);
			}

		}
	}

	// Grab both ends of a road.
	if ((pView->GetPickConstraint() == ES_NONE || pView->GetPickConstraint() == ES_ROAD) && pView->getShowRoads()) {
		if (!shiftKey && pClosestPicked && (pClosestPicked->getFlags()&FLAG_ROAD_FLAGS) ) {
			for (pObj = MapObject::getFirstMapObject(); pObj; pObj = pObj->getNext()) {
				if (pObj->getFlags()&FLAG_ROAD_FLAGS) {
					loc = *pObj->getLocation();
					Real dx = pClosestPicked->getLocation()->x - loc.x;
					Real dy = pClosestPicked->getLocation()->y - loc.y;
					Real dist = sqrt(dx*dx+dy*dy);
					if (dist < MAP_XY_FACTOR/100) {
						pObj->setSelected(true);
					}
				}
			}
		}
	}

	if (anySelected) {
		if (m_curObject) {
			// See if we are picking on the arrow. 
			if (pView->picked(m_curObject, cpt) == PICK_ARROW) {
				m_rotating = true;

				if(!g_PointerToolTip){
					CToastDialog* pToast = new CToastDialog(
					_T("Hold Ctrl to rotate as a group.\nSee Edit tab for rotation options."),
					20000, true);
					pToast->Create(CToastDialog::IDD);
					pToast->ShowWindow(SW_SHOWNOACTIVATE);
					g_PointerToolTip = true;
				}
			}
		}	else {
			pObj = MapObject::getFirstMapObject();
			while (pObj) {
				if (pObj->isSelected()) {
					m_curObject = pObj;
					break;
				}
				pObj = pObj->getNext();
			}
		}
		if (m_curObject) {
			// adjust the starting point so if we are snapping, the object snaps as well.
			loc = *m_curObject->getLocation();
			Coord3D snapLoc = loc;
			pView->snapPoint(&snapLoc);
			m_downPt3d.x += (loc.x-snapLoc.x);
			m_downPt3d.y += (loc.y-snapLoc.y);
		}
	}	else {
		m_dragSelect = true;
	}

	m_isMouseDown = true;
}

bool m_groupRotationInit = false;
float m_startGroupAngle = 0.0f;

std::map<MapObject*, Coord3D> m_originalPositions;
std::map<MapObject*, Real> m_originalAngles;
std::vector<MapObject*> m_tempDeselectedRoads;
void PointerTool::mouseMoved(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc) {
    Coord3D cpt;
    pView->viewToDocCoords(viewPt, &cpt, false);
    Bool KEY = (0x8000 & ::GetAsyncKeyState(VK_CONTROL)) != 0;
	m_rotateObjectsWithGroup = ::AfxGetApp()->GetProfileInt("MainFrame", "ToggleObjectRotationWithGroup", 1);
	m_useFarthestObjectPivot = ::AfxGetApp()->GetProfileInt("MainFrame", "TogglePivotFarthest", 1);

	// ::MessageBeep(MB_OK); // BEEP BOOP
    if (m == TRACK_NONE) {
        MapObject *pObj = MapObject::getFirstMapObject();
        m_mouseUpRotate = false;
        m_mouseUpMove = false;
        while (pObj) {
            if (allowPick(pObj, pView)) {
                TPickedStatus stat = pView->picked(pObj, cpt);
                if (stat == PICK_ARROW) { m_mouseUpRotate = true; break; }
                if (stat == PICK_CENTER) { m_mouseUpMove = true; break; }
            }
            pObj = pObj->getNext();
        }
        if (!m_mouseUpRotate) {
            pObj = pView->picked3dObjectInView(viewPt);
            if (allowPick(pObj, pView)) m_mouseUpMove = true;
        }
        if (pView->isPolygonTriggerVisible() && pickPolygon(cpt, viewPt, pView)) {
            if (pView->GetPickConstraint() == ES_NONE || pView->GetPickConstraint() == ES_WAYPOINT) {
                m_mouseUpMove = true;
                m_mouseUpRotate = false;
            }
        }

		// Call this ridiculously expensive redraw function every time the mouse moves
		// Greatly smooths the framerate when just moving the mouse around, with or without any selection
		// At the cost of your GPU usage 
		pView->Invalidate();
		pDoc->updateAllViews();
        return;
    }

    if (m != TRACK_L) return;

    if (m_doPolyTool) {
        PolygonTool::mouseMoved(m, viewPt, pView, pDoc);
        return;
    }

    if (m_dragSelect) {
        CRect box;
        box.left = viewPt.x;
        box.bottom = viewPt.y;
        box.top = m_downPt2d.y;
        box.right = m_downPt2d.x;
        box.NormalizeRect();
        pView->doRectFeedback(true, box);
        pView->Invalidate();
		pDoc->updateAllViews();
        return;
    }

    if (m_curObject == NULL) return;
    pView->viewToDocCoords(viewPt, &cpt, !m_rotating);

    if (!m_moving) {
        Int dx = viewPt.x - m_downPt2d.x;
        Int dy = viewPt.y - m_downPt2d.y;
        if (abs(dx) > HYSTERESIS || abs(dy) > HYSTERESIS) {
            m_moving = true;
            m_modifyUndoable = new ModifyObjectUndoable(pDoc);

            // Calculate group pivot when starting movement in group rotate mode
            if (m_rotating && KEY) {
                Coord3D pivot = {0, 0, 0};

                if (m_useFarthestObjectPivot) {
                    // --- Farthest pivot mode ---
                    float maxDistSq = -1.0f;
                    MapObject *farthestObj = NULL;
                    MapObject *obj = MapObject::getFirstMapObject();
                    while (obj) {
                        if (obj->isSelected()) {
                            Coord3D loc = *obj->getLocation();
                            float dx = loc.x - m_downPt3d.x;
                            float dy = loc.y - m_downPt3d.y;
                            float distSq = dx * dx + dy * dy;
                            if (distSq > maxDistSq) {
                                maxDistSq = distSq;
                                farthestObj = obj;
                            }
                        }
                        obj = obj->getNext();
                    }
                    if (farthestObj) {
                        pivot = *farthestObj->getLocation();
                    }
                } else {
                    // --- Default average pivot mode ---
                    int count = 0;
                    MapObject *obj = MapObject::getFirstMapObject();
                    while (obj) {
                        if (obj->isSelected()) {
                            pivot.x += obj->getLocation()->x;
                            pivot.y += obj->getLocation()->y;
                            pivot.z += obj->getLocation()->z;
                            count++;
                        }
                        obj = obj->getNext();
                    }
                    if (count > 0) {
                        pivot.x /= count;
                        pivot.y /= count;
                        pivot.z /= count;
                    }
                }

                m_groupPivot = pivot;
                m_groupRotationInit = false;
            }
        }
    }
    if (!m_moving || !m_modifyUndoable) return;

    MapObject *curMapObj = MapObject::getFirstMapObject();
    while (curMapObj) {
        if (curMapObj->isSelected()) {
            pView->invalObjectInView(curMapObj);
        }
        curMapObj = curMapObj->getNext();
    }

    CString text;
    if (m_rotating) {
        if (KEY) {
            // --- FIXED GROUP ROTATION MODE ---
            Coord3D pivot = m_groupPivot;

            // On first move, store initial angle & original positions
			if (!m_groupRotationInit) {
				pView->snapPoint(&cpt);
				if (pView->isLockedAngle()) {
					m_startGroupAngle = ObjectTool::calcAngleSnapped(pivot, cpt, pView);
				} else {
					m_startGroupAngle = ObjectTool::calcAngle(pivot, cpt, pView);
				}

				m_originalPositions.clear();
				m_originalAngles.clear();
				MapObject* obj = MapObject::getFirstMapObject();
				while (obj) {
					if (obj->isSelected()) {
						m_originalPositions[obj] = *obj->getLocation();
						m_originalAngles[obj] = obj->getAngle();
					}
					obj = obj->getNext();
				}
				m_groupRotationInit = true;
			}

            // Calculate delta rotation
            pView->snapPoint(&cpt);
            float currentAngle = pView->isLockedAngle()
                ? ObjectTool::calcAngleSnapped(pivot, cpt, pView)
                : ObjectTool::calcAngle(pivot, cpt, pView);
            float deltaAngle = currentAngle - m_startGroupAngle;

            // Rotate all selected objects from original positions
            MapObject* obj = MapObject::getFirstMapObject();
            while (obj) {
                if (obj->isSelected()) {
                    Coord3D origLoc = m_originalPositions[obj];
                    float relX = origLoc.x - pivot.x;
                    float relY = origLoc.y - pivot.y;
                    float newX = relX * cos(deltaAngle) - relY * sin(deltaAngle);
                    float newY = relX * sin(deltaAngle) + relY * cos(deltaAngle);
                    origLoc.x = pivot.x + newX;
                    origLoc.y = pivot.y + newY;
                    obj->setLocation(&origLoc);

                    // Rotate the object itself if toggle is on
                    if (m_rotateObjectsWithGroup) {
                        obj->setAngle(m_originalAngles[obj] + deltaAngle);
                    }
                }
                obj = obj->getNext();
            }

            // UI angle display
            float angleDeg = pView->isLockedAngle()
                ? ObjectTool::getAngleDegreesSnapped15(pivot, cpt, pView)
                : ObjectTool::getAngleDegrees360(pivot, cpt, pView);
            text.Format(_T("Group Angle: %.2f"), angleDeg);
            m_lastPointerInfo = text;
        } else {
            // --- NORMAL ROTATION ---
			// // Deselect roads temporarily
			// m_tempDeselectedRoads.clear();
			// MapObject* obj = MapObject::getFirstMapObject();
			// while (obj) {
			// 	if (obj->isSelected() && obj->getFlag(FLAG_ROAD_FLAGS)) {
			// 		obj->setSelected(false);
			// 		m_tempDeselectedRoads.push_back(obj);
			// 	}
			// 	obj = obj->getNext();
			// }
			
            Coord3D center = *m_curObject->getLocation();
            float angleDeg;
            pView->snapPoint(&cpt);
            if (pView->isLockedAngle()) {
                m_modifyUndoable->RotateTo(ObjectTool::calcAngleSnapped(center, cpt, pView));
                angleDeg = ObjectTool::getAngleDegreesSnapped15(center, cpt, pView);
            } else {
                m_modifyUndoable->RotateTo(ObjectTool::calcAngle(center, cpt, pView));
                angleDeg = ObjectTool::getAngleDegrees360(center, cpt, pView);
            }
            text.Format(_T("Angle: %.2f"), angleDeg);
            m_lastPointerInfo = text;
        }
    } else {
        // --- MOVEMENT ---
        pView->snapPoint(&cpt);
        Real xOffset = (cpt.x - m_downPt3d.x);
        Real yOffset = (cpt.y - m_downPt3d.y);
        m_modifyUndoable->SetOffset(xOffset, yOffset);
        Coord3D center = *m_curObject->getLocation();
        text.Format(_T("X: %.2f\nY: %.2f"), center.x, center.y);
        m_lastPointerInfo = text;
    }

    curMapObj = MapObject::getFirstMapObject();
    while (curMapObj) {
        if (curMapObj->isSelected()) {
            pView->invalObjectInView(curMapObj);
        }
        curMapObj = curMapObj->getNext();
    }


	pView->Invalidate();  
    pDoc->updateAllViews();
}

/** Execute the tool on mouse up - if modifying, do the modify, 
else update the selection. */
void PointerTool::mouseUp(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc) 
{

	// if (!m_tempDeselectedRoads.empty()) {
	// 	for (std::vector<MapObject*>::iterator it = m_tempDeselectedRoads.begin();
	// 		it != m_tempDeselectedRoads.end();
	// 		++it)
	// 	{
	// 		(*it)->setSelected(true);
	// 	}
	// 	m_tempDeselectedRoads.clear();
	// }

	if (m != TRACK_L) return;

	if (m_doPolyTool) {
		m_doPolyTool = false;
		PolygonTool::mouseUp(m, viewPt, pView, pDoc);
		checkForPropertiesPanel();
		return;
	}

	Coord3D cpt;
	pView->viewToDocCoords(viewPt, &cpt);

	if (m_curObject && m_moving) {
		pDoc->AddAndDoUndoable(m_modifyUndoable);
		REF_PTR_RELEASE(m_modifyUndoable); // belongs to pDoc now.
	}	else if (m_dragSelect) {
		CRect box;
		box.left = viewPt.x;
		box.top = viewPt.y;
		box.right = m_downPt2d.x;
		box.bottom = m_downPt2d.y;
		box.NormalizeRect();
		pView->doRectFeedback(false, box);
		pView->Invalidate();

		MapObject *pObj;
		for (pObj = MapObject::getFirstMapObject(); pObj; pObj = pObj->getNext()) {
			// Don't pick on invisible waypoints
			if (pObj->isWaypoint() && !pView->isWaypointVisible()) {
				continue;
			}
			if (!allowPick(pObj, pView)) {
				continue;
			}
			Bool picked;
			Coord3D loc = *pObj->getLocation();
			CPoint viewPt;
			if (pView->docToViewCoords(loc, &viewPt)){
				picked = (viewPt.x>=box.left && viewPt.x<=box.right && viewPt.y>=box.top && viewPt.y<=box.bottom) ;
				if (picked) {
					if ((0x8000 && ::GetAsyncKeyState(VK_SHIFT))) {
						// !pObj->isSelected() is the original value -- its a bit annoying so we use true always (Adriane[Deathscythe])
						pObj->setSelected(true);
					}	else {
						pObj->setSelected(true);
					}
					pDoc->invalObject(pObj);
				}
			}
		}

	} 

	m_isMouseDown = false;
	checkForPropertiesPanel();
}

