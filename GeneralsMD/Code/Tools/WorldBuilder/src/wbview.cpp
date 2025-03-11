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

// wbview.cpp : implementation file
//

#include "StdAfx.h"
#include "resource.h"
#include "CUndoable.h"
#include "CFixTeamOwnerDialog.h"
#include "WorldBuilder.h"
#include "WorldBuilderDoc.h"
#include "wbview.h"
#include "WHeightMapEdit.h"
#include "MainFrm.h"
#include "Common/Debug.h"
#include "Common/ThingTemplate.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "GlobalLightOptions.h"
#include "playerlistdlg.h"
#include "teamsdialog.h"
#include "LayersList.h"

#ifdef _INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif
Bool WbView::m_snapToGrid = false;

/////////////////////////////////////////////////////////////////////////////
// WbView

IMPLEMENT_DYNCREATE(WbView, CView)

WbView::WbView() :
	m_trackingMode(TRACK_NONE),
	m_centerPt(15,40,0),
	m_hysteresis(0),
	m_lockAngle(false),
	m_doLightFeedback(FALSE),
	m_pickConstraint(ES_NONE),
	m_doRulerFeedback(RULER_NONE)
{
	Int showWay = ::AfxGetApp()->GetProfileInt(MAIN_FRAME_SECTION, "ShowWaypoints", 1);
	m_showWaypoints = (showWay!=0);
	Int showPoly = ::AfxGetApp()->GetProfileInt(MAIN_FRAME_SECTION, "ShowPolygonTriggers", 1);
	m_showPolygonTriggers = (showPoly!=0);
	Int showObj = ::AfxGetApp()->GetProfileInt(MAIN_FRAME_SECTION, "ShowObjectIcons", 1);
	m_showObjects = (showObj!=0);
	Int showNames = ::AfxGetApp()->GetProfileInt(MAIN_FRAME_SECTION, "ShowNames", 1);
	m_showNames = (showNames!=0);
	Int snapToGrid = ::AfxGetApp()->GetProfileInt(MAIN_FRAME_SECTION, "SnapToGrid", 0);
	m_snapToGrid = (snapToGrid!=0);
	Int showTerrain = ::AfxGetApp()->GetProfileInt(MAIN_FRAME_SECTION, "ShowTerrain", 1);
	m_showTerrain = (showTerrain!=0);
}

WbView::~WbView()
{
}


BEGIN_MESSAGE_MAP(WbView, CView)
	//{{AFX_MSG_MAP(WbView)
	ON_WM_RBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_SETCURSOR()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_COMMAND(ID_EDIT_DELETE, OnEditDelete)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_COMMAND(ID_VIEW_SNAPTOGRID, OnViewSnaptogrid)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SNAPTOGRID, OnUpdateViewSnaptogrid)
	ON_COMMAND(ID_VIEW_SHOW_OBJECTS, OnViewShowObjects)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_OBJECTS, OnUpdateViewShowObjects)
	ON_COMMAND(ID_EDIT_SELECTDUP, OnEditSelectdup)
	ON_COMMAND(ID_EDIT_SELECTSIMILAR, OnEditSelectsimilar)
	ON_COMMAND(ID_EDIT_REPLACE, OnEditReplace)
	ON_COMMAND(ID_EDIT_SELECTINVALIDTEAM, OnEditSelectinvalidteam)
	ON_COMMAND(ID_OBJECTPROPERTIES_REFLECTSINMIRROR, OnObjectpropertiesReflectsinmirror)
	ON_COMMAND(ID_LOCK_HORIZONTAL, OnLockHorizontal)
	ON_UPDATE_COMMAND_UI(ID_LOCK_HORIZONTAL, OnUpdateLockHorizontal)
	ON_COMMAND(ID_EDIT_GLOBALLIGHTOPTIONS, OnEditGloballightoptions)
	ON_COMMAND(ID_VIEW_SHOWWAYPOINTS, OnViewShowwaypoints)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWWAYPOINTS, OnUpdateViewShowwaypoints)
	ON_COMMAND(ID_VIEW_SHOWPOLYGONTRIGGERS, OnViewShowpolygontriggers)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWPOLYGONTRIGGERS, OnUpdateViewShowpolygontriggers)
	ON_COMMAND(ID_EDIT_PLAYERLIST, OnEditPlayerlist)
	ON_COMMAND(ID_EDIT_WORLDINFO, OnEditWorldinfo)
	ON_COMMAND(ID_EDIT_TEAMLIST, OnEditTeamlist)
	ON_COMMAND(ID_TEAM_EDIT, OnEditTeamlist)
	ON_UPDATE_COMMAND_UI(ID_OBJECTPROPERTIES_REFLECTSINMIRROR, OnUpdateObjectpropertiesReflectsinmirror)
	ON_COMMAND(ID_EDIT_PICKSTRUCTS, OnPickStructures)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PICKSTRUCTS, OnUpdatePickStructures)
	ON_COMMAND(ID_EDIT_PICKINFANTRY, OnPickInfantry)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PICKINFANTRY, OnUpdatePickInfantry)
	ON_COMMAND(ID_EDIT_PICKVEHICLES, OnPickVehicles)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PICKVEHICLES, OnUpdatePickVehicles)
	ON_COMMAND(ID_EDIT_PICKSHRUBBERY, OnPickShrubbery)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PICKSHRUBBERY, OnUpdatePickShrubbery)
	ON_COMMAND(ID_EDIT_PICKMANMADE, OnPickManMade)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PICKMANMADE, OnUpdatePickManMade)
	ON_COMMAND(ID_EDIT_PICKNATURAL, OnPickNatural)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PICKNATURAL, OnUpdatePickNatural)
	ON_COMMAND(ID_EDIT_PICKDEBRIS, OnPickDebris)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PICKDEBRIS, OnUpdatePickDebris)
	ON_COMMAND(ID_EDIT_PICKANYTHING, OnPickAnything)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PICKANYTHING, OnUpdatePickAnything)
	ON_COMMAND(ID_EDIT_PICKWAYPOINTS, OnPickWaypoints)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PICKWAYPOINTS, OnUpdatePickWaypoints)
	ON_COMMAND(ID_EDIT_PICKROADS, OnPickRoads)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PICKROADS, OnUpdatePickRoads)
	ON_COMMAND(ID_EDIT_PICKSOUNDS, OnPickSounds)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PICKSOUNDS, OnUpdatePickSounds)
	ON_COMMAND(ID_VIEW_LABELS, OnShowNames)
	ON_UPDATE_COMMAND_UI(ID_VIEW_LABELS, OnUpdateShowNames)
	ON_COMMAND(ID_VALIDATION_FIXTEAMS, OnValidationFixTeams)
	ON_COMMAND(ID_VIEW_SHOW_TERRAIN, OnShowTerrain)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_TERRAIN, OnUpdateShowTerrain)
	ON_WM_CREATE()
	
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// WbView drawing

void WbView::OnDraw(CDC* pDC)
{
	DEBUG_ASSERTCRASH((0),("oops"));
}

/////////////////////////////////////////////////////////////////////////////
// WbView diagnostics

#ifdef _DEBUG
void WbView::AssertValid() const
{
	CView::AssertValid();
}

void WbView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// WbView message handlers

// ----------------------------------------------------------------------------
void WbView::mouseDown(TTrackingMode m, CPoint viewPt)
{
	// can happen if you press 2 mouse buttons.  DEBUG_ASSERTCRASH((m_trackingMode==TRACK_NONE),("oops"));
	if (m_trackingMode != TRACK_NONE)
		return;

	SetCapture();
	m_mouseDownPoint = viewPt;
	viewToDocCoords(viewPt, &m_mouseDownDocPoint);
	m_trackingMode = m;
	WbApp()->updateCurTool(m == TRACK_R || m == TRACK_M);
	WbApp()->lockCurTool();
	// If we have a tool, invoke it's mouse down method.
	if (WbApp()->getCurTool()) {
		WbApp()->getCurTool()->mouseDown(m, viewPt, this, WbDoc());
	}
}

// ----------------------------------------------------------------------------
void WbView::mouseMove(TTrackingMode m, CPoint viewPt)
{
	MSG msg;
	while (::PeekMessage(&msg, m_hWnd, WM_MOUSEMOVE, WM_MOUSEMOVE, PM_REMOVE)) {
		viewPt.x = (short)LOWORD(msg.lParam);  // horizontal position of cursor 
		viewPt.y = (short)HIWORD(msg.lParam);  // vertical position of cursor 
		DEBUG_LOG(("Peek mouse %d, %d\n", viewPt.x,  viewPt.y));
	}

	if (m_trackingMode == TRACK_NONE) {
		// Don't lock while the mouse is up.
		m_doLockAngle = false;
	} else {
		m_doLockAngle = m_lockAngle;
	}
	if (m_trackingMode == m) {
		WbApp()->updateCurTool(false);
		if (WbApp()->getCurTool()) {
			WbApp()->getCurTool()->mouseMoved(m, viewPt, this, WbDoc());
		}
	}
	if (CMainFrame::GetMainFrame()->isAutoSaving()) {
		return;
	}

	if (m_doRulerFeedback != RULER_NONE) {
		// If the user is measuring stuff, no need to do the rest of the text.
		CString str;
		if (m_doRulerFeedback == RULER_CIRCLE) {
			str.Format("Diameter (in feet): %f", m_rulerLength * 2.0f);
		} else {
			str.Format("Length (in feet): %f", m_rulerLength);
		}
		CMainFrame::GetMainFrame()->SetMessageText(str);
		return;
	}

	// Generate the status text display with coordinates and height.
	Coord3D cpt;
	viewToDocCoords(viewPt, &cpt);

	WorldHeightMapEdit *pMap = WbDoc()->GetHeightMap();
	MapObject *pObj = MapObject::getFirstMapObject();
	Int totalObjects = 0;
	Int totalWaypoints = 0;
	Int numSelected = 0;
	while(pObj) {
		if (pObj->isSelected()) {
			numSelected++;
		}
		const Int flagsWeDontWant = FLAG_ROAD_FLAGS | FLAG_ROAD_CORNER_ANGLED | FLAG_BRIDGE_FLAGS | FLAG_ROAD_CORNER_TIGHT | FLAG_ROAD_JOIN;
		Int flags = pObj->getFlags();
		if (!(pObj->isWaypoint() || (flags & flagsWeDontWant) != 0))
			++totalObjects;
		else 
			++totalWaypoints;

		pObj = pObj->getNext();
	}

	// See if the cursor is over an object.
	pObj = MapObject::getFirstMapObject();
	while (pObj) {
		if (picked(pObj, cpt)) {
			break;
		}
		pObj = pObj->getNext();
	}
	if (pObj==NULL) {
		pObj = picked3dObjectInView(viewPt);
	}
	Real height = TheTerrainRenderObject->getHeightMapHeight(cpt.x, cpt.y, NULL);
	CString str, str2, str3;
	// If a layer has been activated, display it.
	if (strcmp(AsciiString::TheEmptyString.str(), LayersList::TheActiveLayerName.c_str()) != 0) {
		str.Format("Active Layer: (%s)    %d object(s), ", LayersList::TheActiveLayerName.c_str(), totalObjects);
	} else {
		str.Format("%d object(s), ", totalObjects);
	}
	str2.Format("%d waypoint(s), ", totalWaypoints);
	str3.Format("(%.2f,%.2f), height %.2f", cpt.x, cpt.y, height);
	str += str2;
	str += str3;
	if (numSelected) {
		CString numSel;
		numSel.Format("(%d selected) ", numSelected);
		str += numSel;
	}
	if (pObj) {
		// If we have an object, add the object's name and index to the status.
		const char *pName;
		const char *pFullName = pObj->getName().str();
		Coord3D loc = *pObj->getLocation();
		pName = pFullName;
		while (*pFullName) {
			if (*pFullName == '/') {
				pName = pFullName+1;
			}
			pFullName++;
		}

		Bool exists;
		AsciiString objectID = pObj->getProperties()->getAsciiString(TheKey_uniqueID, &exists);
		CString obj;
		if (exists) {
			obj.Format(" - object %s(%.2f,%.2f)", objectID.str(), loc.x, loc.y);
		} else {
			obj.Format(" - object %s(%.2f,%.2f)", pName, loc.x, loc.y);
		}
		str += obj;
	}
	RGBColor color;
	pMap->getTerrainColorAt(cpt.x, cpt.y, &color);
	CString colorStr;
	colorStr.Format("{r%f,g%f,b%f}", color.red,color.green,color.blue);
	str += colorStr;
	CMainFrame::GetMainFrame()->SetMessageText(str);
}

// ----------------------------------------------------------------------------
void WbView::mouseUp(TTrackingMode m, CPoint viewPt)
{
	//DEBUG_ASSERTCRASH((m_trackingMode!=TRACK_NONE),("oops"));
	if (m_trackingMode == TRACK_NONE)
		return;

	if (GetCapture() == this) 
	{
		ReleaseCapture();
	}

	// If we were tracking a tool, call its mouseup method.
	if (WbApp()->getCurTool()) {
		WbApp()->getCurTool()->mouseUp(m, viewPt, this, WbDoc());
		// Give warning messages at this point.
		WorldHeightMapEdit *pMap = WbDoc()->GetHeightMap();
		if (pMap->tooManyTextures()) {
			AfxMessageBox(IDS_TOO_MANY_TILES);
		}
		if (pMap->tooManyBlends()) {
			AfxMessageBox(IDS_TOO_MANY_BLENDS);
		}
		pMap->clearStatus();
	}
	WbApp()->unlockCurTool();
	// We don't update the tool while tracking down, wouldn't want the eyedropper
	// to suddenly turn into the flood fill tool, so now we update in case a key
	// was released while tracking.
	WbApp()->updateCurTool(false);
	m_trackingMode = TRACK_NONE;
}

void WbView::OnMouseMove(UINT nFlags, CPoint point) 
{
	mouseMove(m_trackingMode, point);
}

void WbView::OnRButtonUp(UINT nFlags, CPoint point) 
{
	mouseUp(TRACK_R, point);
}

void WbView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	mouseDown(TRACK_R, point);
}

void WbView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	mouseUp(TRACK_L, point);
}

void WbView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	mouseDown(TRACK_L, point);
}

void WbView::OnMButtonUp(UINT nFlags, CPoint point) 
{
	mouseUp(TRACK_M, point);
}

void WbView::OnMButtonDown(UINT nFlags, CPoint point) 
{
	mouseDown(TRACK_M, point);
}

//=============================================================================
// WbView::pickedInView
//=============================================================================
/** Returns true if the pixel location picks the object. */
//=============================================================================
TPickedStatus WbView::picked(MapObject *pObj, Coord3D docPt)
{
	Coord3D cloc = *pObj->getLocation();
	if (!m_showObjects && !pObj->isWaypoint()) {
		return PICK_NONE;
	}
	if (!m_showWaypoints && !WaypointTool::isActive() && pObj->isWaypoint()) {
		return PICK_NONE;
	}

	Bool doArrow = pObj->isSelected();
	// Check and see if we are within 1/2 cell size of the center.
	Coord3D cpt = docPt;
	cpt.x -= cloc.x;
	cpt.y -= cloc.y;
	cpt.z = 0;
	if (cpt.length() < 0.5f*MAP_XY_FACTOR+m_hysteresis) {
		return PICK_CENTER;
	}
	if (pObj->getFlag(FLAG_ROAD_FLAGS) ||  pObj->getFlag(FLAG_BRIDGE_FLAGS) || pObj->isWaypoint()) {
		doArrow = false;
	}
	// Check and see if we are within 1 cell size of the center.
	if (doArrow && cpt.length() < 1.5f*MAP_XY_FACTOR+m_hysteresis) {
		return PICK_ARROW;
	}
	return PICK_NONE;
}

//=============================================================================
WorldHeightMapEdit *WbView::getTrackingHeightMap()
{
	WorldHeightMapEdit *pMap = WbDoc()->GetHeightMap();
	// If we are editing with a tool, draw the map being edited, if one exists.
	if (m_trackingMode != TRACK_NONE && WbApp()->getCurTool()) {
		pMap = WbApp()->getCurTool()->getHeightMap();
	}
	// If we aren't editing, or the tool doesn't provide a map, use the current one.
	if (pMap == NULL) {
		pMap = WbDoc()->GetHeightMap();
	}
	return pMap;
}

//=============================================================================
void WbView::constrainCenterPt()
{
	WorldHeightMapEdit *pMap = WbDoc()->GetHeightMap();
	if (pMap==NULL) return;
#if 0
	if (m_centerPt.X >= pMap->getXExtent()) m_centerPt.X = pMap->getXExtent()-1;
	if (m_centerPt.X<0) m_centerPt.X = 0;
	if (m_centerPt.Y >= pMap->getYExtent()) m_centerPt.Y = pMap->getYExtent() - 1;
	if (m_centerPt.Y<0) m_centerPt.Y=0;
#endif
}

//=============================================================================
// WbView::OnSetCursor
//=============================================================================
/** Standard window handler method for updating the cursor. */
//=============================================================================
BOOL WbView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if (nHitTest == HTCLIENT) {
		// If we are tracking in our window, update the tool.
		WbApp()->updateCurTool(false);
		if (WbApp()->getCurTool()) {                       
			// Let the current tool set it's cursor.
			WbApp()->getCurTool()->setCursor();
		} else {
			// Else just use the system arrow cursor.  This shouldn't normally happen.
			::SetCursor(::LoadCursor(NULL, IDC_ARROW));
		}
		return(0);
	}
	// Otherwise let the parent window handle the other cursors like window resize etc.
	return(CView::OnSetCursor(pWnd, nHitTest, message));
}

 
/** Handles the delete menu action. */
void WbView::OnEditDelete() 
{
	if (PolygonTool::isActive() || m_showPolygonTriggers) {
		if (PolygonTool::deleteSelectedPolygon()) {
			return;
		}
	}
	CWorldBuilderDoc* pDoc = WbDoc();
	// create a delete undoable.
	DeleteObjectUndoable *pUndo = new DeleteObjectUndoable(pDoc);
	// Execute it.
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
}

/** Handles the key down event.  Currently, handles delete keys, and checks
for updates to the current tool. */
void WbView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if (nChar == VK_DELETE || nChar == VK_BACK) {	
		OnEditDelete();
	}
	WbApp()->updateCurTool(false);
	OnSetCursor(this,HTCLIENT,0);
}

/** Handles the key up event.  Currently, handles delete keys, and checks
for updates to the current tool. */
void WbView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	WbApp()->updateCurTool(false);
	OnSetCursor(this,HTCLIENT,0);
}

void WbView::OnEditCopy() 
{
	MapObject *pTheCopy = NULL;	
	MapObject *pTmp = NULL;

	MapObject *pObj = MapObject::getFirstMapObject();
	// Note - map segments come in pairs.  So copy both.
	MapObject *pMapObj;
	MapObject *pMapObj2;
	for (pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext()) {
		if (pMapObj->getFlag(FLAG_ROAD_POINT1)) {
			pMapObj2 = pMapObj->getNext();
			DEBUG_ASSERTCRASH(pMapObj2 && pMapObj2->getFlag(FLAG_ROAD_POINT2), ("oops"));
			if (pMapObj2==NULL) break;
			if (!pMapObj2->getFlag(FLAG_ROAD_POINT2)) continue;
			// If one end of a road segment is selected, both are.
			if (pMapObj->isSelected() || pMapObj2->isSelected()) {
				pMapObj->setSelected(true);
				pMapObj2->setSelected(true);
			}
		}
		if (pMapObj->isWaypoint()) {
			pMapObj->setSelected(false);
		}
	}

	pObj = MapObject::getFirstMapObject();
	while (pObj) {
		if (pObj->isSelected()) {
			pTmp = pObj->duplicate();
			pTmp->setNextMap(pTheCopy);
			pTheCopy = pTmp;
		}
		pObj = pObj->getNext();
	}
	WbApp()->setMapObjPasteList(pTheCopy);
	pTheCopy = NULL; // belongs to the app.
}

void WbView::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable();	
}

void WbView::OnEditCut() 
{
	OnEditCopy();
	OnEditDelete();
}

void WbView::OnUpdateEditCut(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable();	
}

void WbView::OnEditPaste() 
{
	CWorldBuilderDoc* pDoc = WbDoc();
	MapObject *pTheCopy = NULL;	
	MapObject *pTmp = NULL;

	/* First, clear the selection. */
	PointerTool::clearSelection();

	MapObject *pObj = WbApp()->getMapObjPasteList();
	while (pObj) {
		pTmp = pObj->duplicate();
		pTmp->setNextMap(pTheCopy);
		pTmp->validate();
		
		pTheCopy = pTmp;
		pTmp->setSelected(true);
		pObj = pObj->getNext();
	}
	AddObjectUndoable *pUndo = new AddObjectUndoable(pDoc, pTheCopy);
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	pTheCopy = NULL; // undoable owns it now.

}

/** Toggles the show objects flag and invals the window. */
void WbView::OnViewShowObjects() 
{
	m_showObjects = !m_showObjects;
	Invalidate(false);
	WbView  *pView = (WbView *)WbDoc()->GetActive2DView();
	if (pView != NULL && pView != this) {
		pView->Invalidate(!m_showObjects);
	}
	::AfxGetApp()->WriteProfileInt(MAIN_FRAME_SECTION, "ShowObjectIcons", m_showObjects?1:0);
}

/** Sets the check in the menu to match the show objects flag. */
void WbView::OnUpdateViewShowObjects(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_showObjects?1:0);
}

void WbView::OnUpdateEditPaste(CCmdUI* pCmdUI) 
{
	MapObject *pTheCopy = WbApp()->getMapObjPasteList();
	pCmdUI->Enable(pTheCopy != NULL);
}

void WbView::OnViewSnaptogrid() 
{
	m_snapToGrid = !m_snapToGrid;
	::AfxGetApp()->WriteProfileInt(MAIN_FRAME_SECTION, "SnapToGrid", m_snapToGrid?1:0);
}

void WbView::OnUpdateViewSnaptogrid(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_snapToGrid?1:0);	
}

void WbView::OnEditSelectdup() 
{
	WorldHeightMapEdit *pMap = WbDoc()->GetHeightMap();
	if (pMap==NULL) return;
	pMap->selectDuplicates();
}
void WbView::OnEditSelectsimilar() 
{
	WorldHeightMapEdit *pMap = WbDoc()->GetHeightMap();
	if (pMap==NULL) return;
	pMap->selectSimilar();
}
void WbView::OnEditSelectinvalidteam() 
{
	WorldHeightMapEdit *pMap = WbDoc()->GetHeightMap();
	if (pMap==NULL) return;
	pMap->selectInvalidTeam();
}

void WbView::OnEditReplace() 
{
	WorldHeightMapEdit *pMap = WbDoc()->GetHeightMap();
	if (pMap==NULL) return;

	EditorSortingType sort = ES_NONE;
	for (MapObject* pObj = MapObject::getFirstMapObject(); pObj; pObj = pObj->getNext()) {
		if (pObj->isSelected()) {
			const ThingTemplate* tt = pObj->getThingTemplate();
			if (tt) {
				sort = tt->getEditorSorting();
				break;
			}
		}
	}

	PickUnitDialog dlg;
	if (sort == ES_NONE) {
		for (int i = ES_FIRST; i<ES_NUM_SORTING_TYPES; i++)	{
			dlg.SetAllowableType((EditorSortingType)i);
		}
	} else {
		dlg.SetAllowableType(sort);
	}
	dlg.SetFactionOnly(false);
	if (dlg.DoModal() == IDOK) {
		const ThingTemplate* thing = dlg.getPickedThing();
		if (thing) {
			CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
			ModifyObjectUndoable *pUndo = new ModifyObjectUndoable(pDoc);
			pDoc->AddAndDoUndoable(pUndo);
			pUndo->SetThingTemplate(thing);
			REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
		}
	}
}


/** Shows the selected status of the reflects in mirror flag. */
void WbView::OnUpdateObjectpropertiesReflectsinmirror(CCmdUI* pCmdUI) 
{
	Bool reflects = false;
	Bool multiple = false;
	Bool first = true;

	MapObject *pMapObj;
	for (pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext()) {
		if (!pMapObj->isSelected()) {
			continue;
		}
		if (pMapObj->getFlag(FLAG_DRAWS_IN_MIRROR)) {
			if (!first && !reflects) {
				multiple = true;
			}
			reflects = true;
		} else {
			if (!first && reflects) {
				multiple = true;
			}
		}
		first = false;
	}
	Int val=0;
	if (reflects) val = 1;
	if (multiple) val = 2;
	pCmdUI->SetCheck(val);
}

void WbView::OnObjectpropertiesReflectsinmirror() 
{
	Bool reflects = false;

	MapObject *pMapObj;
	for (pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext()) {
		if (!pMapObj->isSelected()) {
			continue;
		}
		if (pMapObj->getFlag(FLAG_DRAWS_IN_MIRROR)) {
			reflects = true;
		} 
	}

	CWorldBuilderDoc* pDoc = WbDoc();
	ModifyFlagsUndoable *pUndo = new ModifyFlagsUndoable(pDoc, FLAG_DRAWS_IN_MIRROR, !reflects);
	pDoc->AddAndDoUndoable(pUndo);
	REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
}

// This is actually lock angle - used to be horizontal & vertical, now just 1.
void WbView::OnLockHorizontal() 
{
	m_lockAngle = !m_lockAngle;
}

// This is actually lock angle - used to be horizontal & vertical, now just 1.
void WbView::OnUpdateLockHorizontal(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_lockAngle?1:0);
}

// Obsolete. Delete Jan15,2002 if nobody complains about it being missing.  jba.
void WbView::OnLockVertical() 
{
//	m_lockVertical = !m_lockVertical;
//	if (m_lockVertical) {
//		m_lockHorizontal = false;
//	}
}

// Obsolete. Delete Jan15,2002 if nobody complains about it being missing.  jba.
void WbView::OnUpdateLockVertical(CCmdUI* pCmdUI) 
{
//	pCmdUI->SetCheck(m_lockVertical?1:0);
}

void WbView::OnEditGloballightoptions() 
{
	CMainFrame::GetMainFrame()->OnEditGloballightoptions();

//	GlobalLightOptions globalLightDialog(this);	
//	globalLightDialog.DoModal();
//	Coord3D lightRay;
//	lightRay.x=0.0f;lightRay.y=0.0f;lightRay.z=-1.0f;	//default light above terrain.
//	doLightFeedback(false,lightRay,0);	//turn off the light direction indicator
}

void WbView::OnViewShowwaypoints() 
{
	m_showWaypoints = !m_showWaypoints;
	::AfxGetApp()->WriteProfileInt(MAIN_FRAME_SECTION, "ShowWaypoints", m_showWaypoints?1:0);
	PointerTool::clearSelection();
}

void WbView::OnUpdateViewShowwaypoints(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_showWaypoints?1:0);
}

void WbView::OnViewShowpolygontriggers() 
{
	m_showPolygonTriggers = !m_showPolygonTriggers;
	::AfxGetApp()->WriteProfileInt(MAIN_FRAME_SECTION, "ShowPolygonTriggers", m_showPolygonTriggers?1:0);
}

void WbView::OnUpdateViewShowpolygontriggers(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_showPolygonTriggers?1:0);
}

void WbView::OnEditPlayerlist() 
{
	PlayerListDlg dlg;
	dlg.DoModal();
}

void WbView::OnEditWorldinfo() 
{
	// TODO jkmcd: are we going to ever use this? If so, implement it.
#if 0
	Dict *d = MapObject::getWorldDict();
	Dict dcopy = *d;
	MapObjectProps editor(&dcopy, "Edit World Info", NULL);
	if (editor.DoModal() == IDOK) 
	{
		CWorldBuilderDoc* pDoc = WbDoc();
		DictItemUndoable *pUndo = new DictItemUndoable(d, dcopy, NAMEKEY_INVALID);
		pDoc->AddAndDoUndoable(pUndo);
		REF_PTR_RELEASE(pUndo); // belongs to pDoc now.
	}
#endif
}

void WbView::OnPickStructures() 
{
	m_pickConstraint = ES_STRUCTURE;
}

void WbView::OnUpdatePickStructures(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck((m_pickConstraint == ES_STRUCTURE)?1:0);
}

void WbView::OnPickInfantry() 
{
	m_pickConstraint = ES_INFANTRY;
}

void WbView::OnUpdatePickInfantry(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck((m_pickConstraint == ES_INFANTRY)?1:0);
}

void WbView::OnPickVehicles() 
{
	m_pickConstraint = ES_VEHICLE;
}

void WbView::OnUpdatePickVehicles(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck((m_pickConstraint == ES_VEHICLE)?1:0);
}

void WbView::OnPickShrubbery() 
{
	m_pickConstraint = ES_SHRUBBERY;
}

void WbView::OnUpdatePickShrubbery(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck((m_pickConstraint == ES_SHRUBBERY)?1:0);
}

void WbView::OnPickManMade() 
{
	m_pickConstraint = ES_MISC_MAN_MADE;
}

void WbView::OnUpdatePickManMade(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck((m_pickConstraint == ES_MISC_MAN_MADE)?1:0);
}

void WbView::OnPickNatural() 
{
	m_pickConstraint = ES_MISC_NATURAL;
}

void WbView::OnUpdatePickNatural(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck((m_pickConstraint == ES_MISC_NATURAL)?1:0);
}

void WbView::OnPickDebris() 
{
	m_pickConstraint = ES_DEBRIS;
}

void WbView::OnUpdatePickDebris(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck((m_pickConstraint == ES_DEBRIS)?1:0);
}

void WbView::OnPickAnything() 
{
	m_pickConstraint = ES_NONE;
}

void WbView::OnUpdatePickAnything(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck((m_pickConstraint == ES_NONE)?1:0);
}

void WbView::OnPickWaypoints() 
{
	m_pickConstraint = ES_WAYPOINT;
}

void WbView::OnUpdatePickWaypoints(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck((m_pickConstraint == ES_WAYPOINT)?1:0);
}

void WbView::OnPickRoads() 
{
	m_pickConstraint = ES_ROAD;
}

void WbView::OnUpdatePickRoads(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck((m_pickConstraint == ES_ROAD)?1:0);
}

void WbView::OnPickSounds()
{
	m_pickConstraint = ES_AUDIO;
}

void WbView::OnUpdatePickSounds(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck((m_pickConstraint == ES_AUDIO) ? 1 : 0);
}

void WbView::OnShowNames() 
{
	m_showNames = m_showNames ? false : true;
	Invalidate(false);
	WbView  *pView = (WbView *)WbDoc()->GetActive2DView();
	if (pView != NULL && pView != this) {
		pView->Invalidate(false);
	}

	::AfxGetApp()->WriteProfileInt(MAIN_FRAME_SECTION, "ShowNames", m_showNames?1:0);
}

void WbView::OnUpdateShowNames(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_showNames ? 1 : 0);
}

void WbView::OnValidationFixTeams()
{
	Bool anyFixes = false;
	Int i;
	// Check for duplicate teams.
	for (i = 0; i < TheSidesList->getNumTeams(); ++i) 
	{
		Dict *d = TheSidesList->getTeamInfo(i)->getDict();

		AsciiString tname = d->getAsciiString(TheKey_teamName);
		Int j;
		for (j=0; j<i; j++) {
			Dict *prevd = TheSidesList->getTeamInfo(j)->getDict();
			if (prevd->getAsciiString(TheKey_teamName).compare(tname)==0) {
				anyFixes = true;
				CString msg;
				msg.Format(IDS_DUPLICATE_TEAM_REMOVED, tname.str());
				AfxMessageBox(msg, MB_OK);
				TheSidesList->removeTeam(i);
				i--;
				break;
			}
		}
	}
	
	// Check for teams with invalid owners.
	for (i = 0; i < TheSidesList->getNumTeams(); ++i) 
	{
		Dict *d = TheSidesList->getTeamInfo(i)->getDict();
		AsciiString oname = d->getAsciiString(TheKey_teamOwner);
		AsciiString tname = d->getAsciiString(TheKey_teamName);
		SidesInfo* pSide = TheSidesList->findSideInfo(oname);
		Bool found = pSide!=NULL;
		if (!found) {
				CString msg;
				msg.Format(IDS_PLAYERLESS_TEAM_REMOVED, tname.str(), oname.str());
				AfxMessageBox(msg, MB_OK);
				anyFixes = true;
				TheSidesList->removeTeam(i);
				i--;
		}
	}

	// Check for objects with invalid teams. [8/8/2003]
	MapObject *pMapObj;
	for (pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext())
	{
		// there is no validation code for these items as of yet.
		if (pMapObj->isScorch() || pMapObj->isWaypoint() || pMapObj->isLight() || pMapObj->getFlag(FLAG_ROAD_FLAGS) || pMapObj->getFlag(FLAG_BRIDGE_FLAGS))
		{
			continue;
		}

		if (pMapObj->getThingTemplate()==NULL) {
			continue; // Objects that don't have templates don't need teams. [8/8/2003]
		}
		// at this point, only objects with models and teams should be left to process

		AsciiString name = pMapObj->getName();
		AsciiString tmplName = pMapObj->getThingTemplate()->getName();

		// the following code verifies and fixes the team name, player name, and faction linkages
		Bool teamExists;
		AsciiString teamName = pMapObj->getProperties()->getAsciiString(TheKey_originalOwner, &teamExists);
		if (teamExists) {
			TeamsInfo *teamInfo = TheSidesList->findTeamInfo(teamName);
			if (teamInfo) {
				AsciiString teamOwner = teamInfo->getDict()->getAsciiString(TheKey_teamOwner);
				SidesInfo* pSide = TheSidesList->findSideInfo(teamOwner);
				if (!pSide) {
					teamExists = false;
					DEBUG_LOG(("Side '%s' could not be found in sides list!\n", teamOwner.str()));
				}
			} else {
				// Couldn't find team. [8/8/2003]
				teamExists = false;
			}
		} else {
			// Object doesn't even have a team name at all.  bad. jba. [8/8/2003]
			teamExists = false; 
		}
		if (!teamExists) {
			// Query the user for a player, and stick it on the default team. [8/8/2003]
			AsciiString warning;
			warning.format("Object '%s' named '%s' on team '%s' - team doesn't exist.  Select player for object...", 
				tmplName.str(), name.str(), teamName.str());

			anyFixes = true;
			::AfxMessageBox(warning.str(), MB_OK);	
			TeamsInfo ti;	 
			CFixTeamOwnerDialog fix(&ti, TheSidesList);
			if (fix.DoModal() == IDOK) {
				if (fix.pickedValidTeam()) {
					AsciiString team;
					team.set("team");
					team.concat(fix.getSelectedOwner());
					if (TheSidesList->findTeamInfo(team)==NULL) {
						team.set("team"); // neutral.
					}
					pMapObj->getProperties()->setAsciiString(TheKey_originalOwner,  team);
				}
			}
		}
	}
	

	if (anyFixes) {
		// Show a message indicating success.
		AfxMessageBox(IDS_TEAMS_FIXED, MB_OK|MB_ICONWARNING);
	} else {
		AfxMessageBox(IDS_NO_PROBLEMS, MB_OK);
	}
}

void WbView::OnShowTerrain()
{
	m_showTerrain = !m_showTerrain;
	Invalidate(false);
	WbView  *pView = (WbView *)WbDoc()->GetActive2DView();
	if (pView != NULL && pView != this) {
		pView->Invalidate(false);
	}

	::AfxGetApp()->WriteProfileInt(MAIN_FRAME_SECTION, "ShowTerrain", m_showTerrain ? 1 : 0);
}

void WbView::OnUpdateShowTerrain(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_showTerrain ? 1 : 0);
}


void WbView::OnEditTeamlist() 
{
	CTeamsDialog dlg;
	dlg.DoModal();
}

// A sleazy hack for a sleazy hack.
extern HWND ApplicationHWnd;
int WbView::OnCreate(LPCREATESTRUCT lpcs)
{
	ApplicationHWnd = m_hWnd;
	return CView::OnCreate(lpcs);
}

void WbView::rulerFeedbackInfo(Coord3D &point1, Coord3D &point2, Real dist) 
{
	m_rulerPoints[0] = point1;
	m_rulerPoints[1] = point2; 
	m_rulerLength = dist;
}
