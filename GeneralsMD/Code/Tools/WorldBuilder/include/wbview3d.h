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


// WbView3d.h
// Class to encapsulate height map.
// Author: Steven Johnson, Aug 2001

#pragma once

// wbview3d.h : header file
//

#include "Lib/BaseType.h"
#include "rendobj.h"
#include "robjlist.h"
#include "wbview.h"
#include "Common/GameType.h"
#include "Common/GlobalData.h"
#include "Common/ModelState.h"
#include "dx8wrapper.h"

//#include "GameLogic/Module/BodyModule.h" -- Yikes... not necessary to include this! (KM)
enum BodyDamageType CPP_11(: Int); //Ahhhh much better!

class WorldHeightMap;
class LayerClass;
class IntersectionClass;
class W3DAssetManager;
class SkeletonSceneClass;
class CameraClass;
class WBHeightMap;
class LightClass;
class MapObject;
class DrawObject;
class CWorldBuilderView;
class BuildListInfo;
class TransRenderObj;
struct ID3DXFont;

/////////////////////////////////////////////////////////////////////////////
// WbView3d view

class WbView3d : public WbView, public DX8_CleanupHook
{
protected:
	WbView3d();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(WbView3d)

// Attributes
public:

	// DX8_CleanupHook methods
	virtual void ReleaseResources(void);	///< Release all dx8 resources so the device can be reset.
	virtual void ReAcquireResources(void);  ///< Reacquire all resources after device reset.

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(WbView3d)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~WbView3d();
#ifdef RTS_DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(WbView3d)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnViewShowwireframe();
	afx_msg void OnUpdateViewShowwireframe(CCmdUI* pCmdUI);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnViewShowentire3dmap();
	afx_msg void OnUpdateViewShowentire3dmap(CCmdUI* pCmdUI);
	afx_msg void OnViewShowtopdownview();
	afx_msg void OnUpdateViewShowtopdownview(CCmdUI* pCmdUI);
	afx_msg void OnViewShowclouds();
	afx_msg void OnUpdateViewShowclouds(CCmdUI* pCmdUI);
	afx_msg void OnViewShowmacrotexture();
	afx_msg void OnUpdateViewShowmacrotexture(CCmdUI* pCmdUI);
	afx_msg void OnEditSelectmacrotexture();
	afx_msg void OnViewShowshadows();
	afx_msg void OnUpdateViewShowshadows(CCmdUI* pCmdUI);
	afx_msg void OnViewShowSoftWater();
	afx_msg void OnUpdateViewShowSoftWater(CCmdUI* pCmdUI);
	afx_msg void OnViewExtraBlends();
	afx_msg void OnUpdateViewShowExtraBlends(CCmdUI* pCmdUI);
	afx_msg void OnEditShadows();
	afx_msg void OnEditMapSettings();
	afx_msg void OnViewShowimpassableareas();
	afx_msg void OnUpdateViewShowimpassableareas(CCmdUI* pCmdUI);
	afx_msg void OnImpassableAreaOptions();
	afx_msg void OnViewPartialmapsize96x96();
	afx_msg void OnUpdateViewPartialmapsize96x96(CCmdUI* pCmdUI);
	afx_msg void OnViewPartialmapsize192x192();
	afx_msg void OnUpdateViewPartialmapsize192x192(CCmdUI* pCmdUI);
	afx_msg void OnViewPartialmapsize160x160();
	afx_msg void OnUpdateViewPartialmapsize160x160(CCmdUI* pCmdUI);
	afx_msg void OnViewPartialmapsize128x128();
	afx_msg void OnUpdateViewPartialmapsize128x128(CCmdUI* pCmdUI);
	afx_msg void OnViewShowModels();
	afx_msg void OnUpdateViewShowModels(CCmdUI* pCmdUI);
	afx_msg void OnViewBoundingBoxes();
	afx_msg void OnUpdateViewBoundingBoxes(CCmdUI* pCmdUI);
	afx_msg void OnViewSightRanges();
	afx_msg void OnUpdateViewSightRanges(CCmdUI* pCmdUI);
	afx_msg void OnViewWeaponRanges();
	afx_msg void OnUpdateViewWeaponRanges(CCmdUI* pCmdUI);
	afx_msg void OnHighlightTestArt();
	afx_msg void OnUpdateHighlightTestArt(CCmdUI* pCmdUI);
	afx_msg void OnShowLetterbox();
	afx_msg void OnUpdateShowLetterbox(CCmdUI* pCmdUI);
	afx_msg void OnViewLayersList();
	afx_msg void OnUpdateViewLayersList(CCmdUI* pCmdUI);
	afx_msg void OnViewGarrisoned();
	afx_msg void OnUpdateViewGarrisoned(CCmdUI* pCmdUI);
	afx_msg void OnViewShowMapBoundaries();
	afx_msg void OnUpdateViewShowMapBoundaries(CCmdUI* pCmdUI);
	afx_msg void OnViewShowAmbientSounds();
	afx_msg void OnUpdateViewShowAmbientSounds(CCmdUI* pCmdUI);
  afx_msg void OnViewShowSoundCircles();
  afx_msg void OnUpdateViewShowSoundCircles(CCmdUI* pCmdUI);
  //}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	HINSTANCE								m_hInst;
	W3DAssetManager					*m_assetManager;
	SkeletonSceneClass			*m_scene;
	SkeletonSceneClass			*m_overlayScene;
	SkeletonSceneClass			*m_baseBuildScene;
	SkeletonSceneClass			*m_transparentObjectsScene;
	CameraClass							*m_camera;
	WBHeightMap							*m_heightMapRenderObj;

	RenderObjClass					*m_objectToolTrackingObj;
	Bool										m_showObjToolTrackingObj;
	AsciiString							m_objectToolTrackingModelName;

	Real										m_mouseWheelOffset;
	Real										m_groundLevel;
	Coord3D									m_cameraOffset;
	CPoint									m_actualWinSize;
	Real										m_theta;
	Real										m_cameraAngle;
	Real										m_FXPitch;
	Bool										m_doPitch;
	Real										m_actualHeightAboveGround;	// for camera tool display only
	Vector3									m_cameraSource;							// for camera tool display only
	Vector3									m_cameraTarget;							// for camera tool display only
	Int											m_time;
	Int											m_updateCount;
	UINT										m_timer;
	DrawObject							*m_drawObject;
	RefRenderObjListClass		m_lightList;
	LayerClass							*m_layer;
	LayerClass							*m_buildLayer;
	IntersectionClass				*m_intersector;
	Bool										m_showWireframe;
	Bool										m_ww3dInited;
	Bool										m_needToLoadRoads;
	LightClass							*m_globalLight[MAX_GLOBAL_LIGHTS];
	RenderObjClass						*m_lightFeedbackMesh[MAX_GLOBAL_LIGHTS];

	Real										m_buildRedMultiplier;

	Real										m_curTrackingZ;


	Bool										m_projection; ///< True if top down projection instead of "isometric" perspective.
	Bool										m_showEntireMap; ///< True if drawing entire map instead of cached fast subset.
	Bool										m_showShadows; ///< True if drawing shadows.
	Bool										m_firstPaint;  ///< True if we haven't painted yet.
	Bool										m_showLayersList;	///< Flag whether the layers list is visible or not.
	Bool										m_showMapBoundaries;	///< Flag whether to show all the map boundaries or not
	Bool										m_showAmbientSounds;	///< Flag whether to show all the ambient sounds or not
  Bool										m_showSoundCircles;	///< Flag whether to show the minimum and maximum radii of the ambient sounds attached to the selected object
	Bool										m_showBoundingBoxes;
	Bool										m_showSightRanges;
	Bool										m_showWeaponRanges;
	Bool										m_highlightTestArt;
	Bool										m_showLetterbox;


	ID3DXFont*							m3DFont;
	Int											m_pickPixels;
	Int											m_partialMapSize;

protected:

	UINT getLastDrawTime();
	void init3dScene();
	void initAssets();
	void initWW3D();
  void drawCircle( HDC hdc, const Coord3D & centerPoint, Real radius, COLORREF color );
	void drawLabels(HDC hdc);
	void drawLabels(void);
	void shutdownWW3D();
	void killTheTimer();
	void render();
	void setupCamera();
	void updateHysteresis(void);
	void updateLights();
	void updateScorches();
	void updateTrees();

public:
	virtual Bool viewToDocCoords(CPoint curPt, Coord3D *newPt, Bool constrain=true);
	virtual Bool docToViewCoords(Coord3D curPt, CPoint* newPt);

	virtual void updateHeightMapInView(WorldHeightMap *htMap, Bool partial, const IRegion2D &partialRange);

	/// Invalidates an object. Pass null to inval all objects.
	virtual void invalObjectInView(MapObject *pObj);

	// find the best model for an object
	AsciiString getBestModelName(const ThingTemplate* tt, const ModelConditionFlags& c);

	/// Invalidates an build list object.
	void invalBuildListItemInView(BuildListInfo *pBuild);

	/// Invalidates the area of one height map cell in the 2d view.
	virtual void invalidateCellInView(int xIndex, int yIndex);

	/// Scrolls the window by this amount.
	virtual void scrollInView(Real x, Real y, Bool end);

	virtual void setDefaultCamera();
	virtual void rotateCamera(Real delta);
	virtual void pitchCamera(Real delta);
	void setCameraPitch(Real absolutePitch);
	Real getCameraPitch(void);
	Real getCurrentZoom(void); //WST 10/17/2002
	Real getHeightAboveGround(void) { return m_actualHeightAboveGround; }
	Vector3 getCameraSource(void) { return m_cameraSource; }
	Vector3 getCameraTarget(void) { return m_cameraTarget; }
	Real getCameraAngle(void) { return m_cameraAngle; }
	CPoint getActualWinSize(void) {return m_actualWinSize;}

	virtual MapObject *picked3dObjectInView(CPoint viewPt);
	virtual BuildListInfo *pickedBuildObjectInView(CPoint viewPt);

	void removeFenceListObjects(MapObject *pObject);
	void updateFenceListObjects(MapObject *pObject);

	/// Removes all render objects.  Call when swithing to a new map.
	void resetRenderObjects();

	void stepTimeOfDay(void);

	void reset3dEngineDisplaySize(Int width, Int height); ///< Closes & reinitializes w3d.
	void setLighting(const GlobalData::TerrainLighting *tl, Int whichLighting, Int whichLight=0);

	DrawObject *getDrawObject(void) {return m_drawObject;};

	AsciiString getModelNameAndScale(MapObject *pMapObj, Real *scale, BodyDamageType curDamageState);

	virtual Int getPickPixels(void) {return m_pickPixels;}
	virtual Bool viewToDocCoordZ(CPoint curPt, Coord3D *newPt, Real Z);
public:

//	void init(CWorldBuilderView *pMainView, HINSTANCE hInstance, CWnd* parent);
	void redraw();

	virtual void setCenterInView(Real x, Real y);

	Bool getShowTerrain();
	Bool getShowWireframe();

	void setObjTracking(MapObject *pMapObj, Coord3D pos, Real angle, Bool show);
	void setViewLayersList(Bool showLayersList) { m_showLayersList = showLayersList; }

	Bool getShowMapBoundaryFeedback(void) const { return m_showMapBoundaries; }
	Bool getShowAmbientSoundsFeedback(void) const { return m_showAmbientSounds; }

	void togglePitchAndRotation( void ) { m_doPitch = !m_doPitch; }
	virtual Bool isDoingPitch( void ) { return m_doPitch; }
	void setShowBoundingBoxes(Bool toggle) {m_showBoundingBoxes = toggle;}
	Bool getShowBoundingBoxes(void) { return m_showBoundingBoxes;}
	void setShowSightRanges(Bool toggle) {m_showSightRanges = toggle;}
	Bool getShowSightRanges(void) { return m_showSightRanges;}
	void setShowWeaponRanges(Bool toggle) {m_showWeaponRanges = toggle;}
	Bool getShowWeaponRanges(void) { return m_showWeaponRanges;}
	void setHighlightTestArt(Bool toggle) {m_highlightTestArt = toggle;}
	Bool getHighlightTestArt(void) { return m_highlightTestArt;}
	void setShowLetterbox(Bool toggle) {m_showLetterbox = toggle;}
	Bool getShowLetterbox(void) { return m_showLetterbox;}
};

inline UINT WbView3d::getLastDrawTime() { return m_time; }
inline Bool WbView3d::getShowWireframe() { return m_showWireframe; }


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
