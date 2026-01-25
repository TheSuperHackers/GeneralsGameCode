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

#pragma once

#include "Lib/BaseType.h"
#include "Tool.h"
#include "Common/Debug.h"
#include "Common/ThingSort.h"

class CWorldBuilderDoc;
class BuildListInfo;
// wbview.h : header file
//

enum TPickedStatus {
	PICK_NONE,
	PICK_CENTER,
	PICK_ARROW
};

enum RulerTypeEnum {
	RULER_NONE,
	RULER_LINE,
	RULER_CIRCLE,
};

/////////////////////////////////////////////////////////////////////////////
// WbView view

class WbView : public CView
{
protected:
	WbView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(WbView)

	TTrackingMode					m_trackingMode;
	Vector3								m_centerPt;

	void mouseDown(TTrackingMode m, CPoint viewPt);
	void mouseMove(TTrackingMode m, CPoint viewPt);
	void mouseUp(TTrackingMode m, CPoint viewPt);


	void constrainCenterPt();

	WorldHeightMapEdit *getTrackingHeightMap();

	static Bool					m_snapToGrid;

 	Bool					m_showObjects;			  ///< Flag whether object icons are drawn in the 2d and 3d view.
 	Bool					m_showModels;					///< Flag whether models are drawn in the 2d and 3d view.
 	Bool					m_showNames;					///< Flag whether names are drawn in the 2d and 3d view.
	Bool					m_showGarrisoned;
	Bool					m_showWaypoints;
	Bool					m_showPolygonTriggers;
 	Bool					m_showTerrain;			  ///< Flag whether terrain is rendered or not. (Useful for debugging)

	Real					m_hysteresis;

	Bool		m_lockAngle;			///< Reflects the ui button.
	//Bool		m_lockVertical;				///< Reflects the ui button.
	Bool		m_doLockAngle;			///< True if we are currently locking.
	//Bool		m_doLockVertical;				///< True if we are currently locking.
	CPoint	m_mouseDownPoint;
	Coord3D	m_mouseDownDocPoint;

	// Box feedback.
	RECT										m_feedbackBox;
	Bool										m_doRectFeedback;
	int											m_doRulerFeedback;
	Coord3D									m_rulerPoints[2];
	Real										m_rulerLength;

	// Light direction feedback
	Coord3D m_lightDirection[3];	//direction of all 3 lights
	Bool	m_doLightFeedback;

	EditorSortingType				m_pickConstraint;

	// Attributes
public:

	void doRectFeedback(Bool doFeedback, RECT &rect) {m_feedbackBox=rect;m_doRectFeedback = doFeedback;};
	void doRulerFeedback(int doRulerFeedback) {m_doRulerFeedback = doRulerFeedback;}
	void rulerFeedbackInfo(Coord3D &point1, Coord3D &point2, Real dist);

	void doLightFeedback(Bool doFeedback, Coord3D direction, Int lightIndex) { m_doLightFeedback=doFeedback; if (m_doLightFeedback) m_lightDirection[lightIndex]=direction;}

	virtual Bool viewToDocCoords(CPoint curPt, Coord3D *newPt, Bool constrained=true) { DEBUG_CRASH(("should not call")); newPt->zero(); return false; }
	virtual Bool docToViewCoords(Coord3D curPt, CPoint* newPt) { DEBUG_CRASH(("should not call")); return false; }

	virtual Bool viewToDocCoordZ(CPoint curPt, Coord3D *newPt, Real Z) { return viewToDocCoords(curPt, newPt, false); }

	/// Set the center for display.
	virtual void setCenterInView(Real x, Real y) { }

	/// Update the height map in the 3d window.
	virtual void updateHeightMapInView(WorldHeightMap *htMap, Bool partial, const IRegion2D &partialRange) { }

	/// the doc has changed size; readjust view as necessary.
	virtual void adjustDocSize() { }

	/// Scrolls the window by this amount.
	virtual void scrollInView(Real x, Real y, Bool end) { DEBUG_CRASH(("should not call"));  }

	/// Invalidates an object. Pass null to inval all objects.
	virtual void invalObjectInView(MapObject *pObj) { }

	/// Invalidates the area of one height map cell in the 2d view.
	virtual void invalidateCellInView(int xIndex, int yIndex) { }

	virtual void setDefaultCamera() { }
	virtual void rotateCamera(Real delta) { }
	virtual void pitchCamera(Real delta) { }

	virtual Int getPickPixels(void) {return 4;}
	virtual EditorSortingType GetPickConstraint(void) {return m_pickConstraint;}

public:
	CWorldBuilderDoc *WbDoc() { return ((CWorldBuilderDoc*)GetDocument()); }

	void snapPoint(Coord3D *thePt) {if (m_snapToGrid || m_lockAngle) {thePt->x = MAP_XY_FACTOR*floor(thePt->x/MAP_XY_FACTOR+0.5); thePt->y = MAP_XY_FACTOR*floor(thePt->y/MAP_XY_FACTOR+0.5);};};

	virtual TPickedStatus picked(MapObject *pObj, Coord3D docPt);
	virtual MapObject *picked3dObjectInView(CPoint viewPt) {return nullptr;};
	virtual BuildListInfo *pickedBuildObjectInView(CPoint viewPt) {return nullptr;};

	Bool isPolygonTriggerVisible(void) {return m_showPolygonTriggers;};
	Bool isWaypointVisible(void) {return m_showWaypoints;};
	Bool isNamesVisible(void) {return m_showNames;};
	void setShowModels(Bool show) {m_showModels = show;}
	Bool getShowModels(void) { return m_showModels;}
	Bool getShowTerrain(void) { return m_showTerrain;}

	void setShowGarrisoned(Bool show) {m_showGarrisoned = show;}
	Bool getShowGarrisoned(void) { return m_showGarrisoned;}

	virtual Bool isDoingPitch( void ) { return false; }

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(WbView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~WbView();
#ifdef RTS_DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(WbView)
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnEditDelete();
	afx_msg void OnEditCopy();
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnEditCut();
	afx_msg void OnUpdateEditCut(CCmdUI* pCmdUI);
	afx_msg void OnEditPaste();
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	afx_msg void OnViewSnaptogrid();
	afx_msg void OnUpdateViewSnaptogrid(CCmdUI* pCmdUI);
	afx_msg void OnViewShowObjects();
	afx_msg void OnUpdateViewShowObjects(CCmdUI* pCmdUI);
	afx_msg void OnEditSelectdup();
	afx_msg void OnEditSelectsimilar();
	afx_msg void OnEditSelectinvalidteam();
	afx_msg void OnEditReplace();
	afx_msg void OnObjectpropertiesReflectsinmirror();
	afx_msg void OnLockHorizontal();
	afx_msg void OnUpdateLockHorizontal(CCmdUI* pCmdUI);
	afx_msg void OnLockVertical();
	afx_msg void OnUpdateLockVertical(CCmdUI* pCmdUI);
	afx_msg void OnEditGloballightoptions();
	afx_msg void OnViewShowwaypoints();
	afx_msg void OnUpdateViewShowwaypoints(CCmdUI* pCmdUI);
	afx_msg void OnViewShowpolygontriggers();
	afx_msg void OnUpdateViewShowpolygontriggers(CCmdUI* pCmdUI);
	afx_msg void OnEditPlayerlist();
	afx_msg void OnEditWorldinfo();
	afx_msg void OnEditTeamlist();
	afx_msg void OnUpdateObjectpropertiesReflectsinmirror(CCmdUI* pCmdUI);
	afx_msg void OnPickStructures();
	afx_msg void OnUpdatePickStructures(CCmdUI* pCmdUI);
	afx_msg void OnPickInfantry();
	afx_msg void OnUpdatePickInfantry(CCmdUI* pCmdUI);
	afx_msg void OnPickVehicles();
	afx_msg void OnUpdatePickVehicles(CCmdUI* pCmdUI);
	afx_msg void OnPickShrubbery();
	afx_msg void OnUpdatePickShrubbery(CCmdUI* pCmdUI);
	afx_msg void OnPickManMade();
	afx_msg void OnUpdatePickManMade(CCmdUI* pCmdUI);
	afx_msg void OnPickNatural();
	afx_msg void OnUpdatePickNatural(CCmdUI* pCmdUI);
	afx_msg void OnPickDebris();
	afx_msg void OnUpdatePickDebris(CCmdUI* pCmdUI);
	afx_msg void OnPickAnything();
	afx_msg void OnUpdatePickAnything(CCmdUI* pCmdUI);
	afx_msg void OnPickWaypoints();
	afx_msg void OnUpdatePickWaypoints(CCmdUI* pCmdUI);
	afx_msg void OnPickRoads();
	afx_msg void OnUpdatePickRoads(CCmdUI* pCmdUI);
	afx_msg void OnPickSounds();
	afx_msg void OnUpdatePickSounds(CCmdUI* pCmdUI);
	afx_msg void OnShowNames();
	afx_msg void OnUpdateShowNames(CCmdUI* pCmdUI);
	afx_msg void OnValidationFixTeams();
	afx_msg void OnShowTerrain();
	afx_msg void OnUpdateShowTerrain(CCmdUI* pCmdUI);
	afx_msg int OnCreate(LPCREATESTRUCT lpcs);


	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
