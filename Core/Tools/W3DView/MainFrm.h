/*
**	Command & Conquer Renegade(tm)
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

// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__2BB39E2F_5D3A_11D2_9FC6_00104B791122__INCLUDED_)
#define AFX_MAINFRM_H__2BB39E2F_5D3A_11D2_9FC6_00104B791122__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "DataTreeView.h"
#include "Toolbar.h"


#if defined(_MSC_VER) && _MSC_VER < 1300
typedef HTASK HTASK_OR_DWORD;
#else
typedef DWORD HTASK_OR_DWORD;
#endif


class CMainFrame : public CFrameWnd
{
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
protected:
	CSplitterWnd m_wndSplitter;
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef RTS_DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnObjectProperties();
	afx_msg void OnUpdateObjectProperties(CCmdUI* pCmdUI);
	afx_msg void OnLodGenerate();
	afx_msg void OnActivateApp(BOOL bActive, HTASK_OR_DWORD hTask);
	afx_msg void OnFileOpen();
	afx_msg void OnAniSpeed();
	afx_msg void OnAniStop();
	afx_msg void OnAniStart();
	afx_msg void OnAniPause();
	afx_msg void OnCameraBack();
	afx_msg void OnCameraBottom();
	afx_msg void OnCameraFront();
	afx_msg void OnCameraLeft();
	afx_msg void OnCameraReset();
	afx_msg void OnCameraRight();
	afx_msg void OnCameraTop();
	afx_msg void OnObjectRotateZ();
	afx_msg void OnObjectRotateY();
	afx_msg void OnObjectRotateX();
	afx_msg void OnLightAmbient();
	afx_msg void OnLightScene();
	afx_msg void OnBackgroundColor();
	afx_msg void OnBackgroundBMP();
	afx_msg void OnSaveSettings();
	afx_msg void OnLoadSettings();
	afx_msg void OnLODSetSwitch();
	afx_msg void OnLODSave();
	afx_msg void OnLODSaveAll();
	afx_msg void OnBackgroundObject();
	afx_msg void OnUpdateViewAnimationBar(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewObjectBar(CCmdUI* pCmdUI);
	afx_msg void OnViewAnimationBar();
	afx_msg void OnViewObjectBar();
	afx_msg void OnAniStepFwd();
	afx_msg void OnAniStepBkwd();
	afx_msg void OnObjectReset();
	afx_msg void OnCameraAllowRotateX();
	afx_msg void OnCameraAllowRotateY();
	afx_msg void OnCameraAllowRotateZ();
	afx_msg void OnUpdateCameraAllowRotateX(CCmdUI* pCmdUI);
	afx_msg void OnUpdateCameraAllowRotateY(CCmdUI* pCmdUI);
	afx_msg void OnUpdateCameraAllowRotateZ(CCmdUI* pCmdUI);
	afx_msg void OnUpdateObjectRotateX(CCmdUI* pCmdUI);
	afx_msg void OnUpdateObjectRotateY(CCmdUI* pCmdUI);
	afx_msg void OnUpdateObjectRotateZ(CCmdUI* pCmdUI);
	afx_msg void OnDeviceChange();
	afx_msg void OnViewFullscreen();
	afx_msg void OnUpdateViewFullscreen(CCmdUI* pCmdUI);
	afx_msg void OnWindowPosChanging(WINDOWPOS FAR* lpwndpos);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnCreateEmitter();
	afx_msg void OnEditEmitter();
	afx_msg void OnUpdateEditEmitter(CCmdUI* pCmdUI);
	afx_msg void OnSaveEmitter();
	afx_msg void OnUpdateSaveEmitter(CCmdUI* pCmdUI);
	afx_msg void OnBoneAutoAssign();
	afx_msg void OnBoneManagement();
	afx_msg void OnSaveAggregate();
	afx_msg void OnCameraAnimate();
	afx_msg void OnUpdateCameraAnimate(CCmdUI* pCmdUI);
	afx_msg void OnUpdateLodSave(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSaveAggregate(CCmdUI* pCmdUI);
	afx_msg void OnCameraResetOnLoad();
	afx_msg void OnUpdateCameraResetOnLoad(CCmdUI* pCmdUI);
	afx_msg void OnObjectRotateYBack();
	afx_msg void OnObjectRotateZBack();
	afx_msg void OnLightRotateY();
	afx_msg void OnLightRotateYBack();
	afx_msg void OnLightRotateZ();
	afx_msg void OnLightRotateZBack();
	afx_msg void OnDestroy();
	afx_msg void OnDecLight();
	afx_msg void OnIncLight();
	afx_msg void OnDecAmbientLight();
	afx_msg void OnIncAmbientLight();
	afx_msg void OnMakeAggregate();
	afx_msg void OnRenameAggregate();
	afx_msg void OnCrashApp();
	afx_msg void OnLODRecordScreenArea();
	afx_msg void OnLODIncludeNull();
	afx_msg void OnUpdateLODIncludeNull(CCmdUI* pCmdUI);
	afx_msg void OnLodPrevLevel();
	afx_msg void OnUpdateLodPrevLevel(CCmdUI* pCmdUI);
	afx_msg void OnLodNextLevel();
	afx_msg void OnUpdateLodNextLevel(CCmdUI* pCmdUI);
	afx_msg void OnLodAutoswitch();
	afx_msg void OnUpdateLodAutoswitch(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMakeMovie(CCmdUI* pCmdUI);
	afx_msg void OnMakeMovie();
	afx_msg void OnSaveScreenshot();
	afx_msg void OnSlideshowDown();
	afx_msg void OnSlideshowUp();
	afx_msg void OnAdvancedAnim();
	afx_msg void OnUpdateAdvancedAnim(CCmdUI* pCmdUI);
	afx_msg void OnCameraSettings();
	afx_msg void OnCopyScreenSize();
	afx_msg void OnListMissingTextures();
	afx_msg void OnCopyAssets();
	afx_msg void OnUpdateCopyAssets(CCmdUI* pCmdUI);
	afx_msg void OnLightingExpose();
	afx_msg void OnUpdateLightingExpose(CCmdUI* pCmdUI);
	afx_msg void OnTexturePath();
	afx_msg void OnChangeResolution();
	afx_msg void OnCreateSphere();
	afx_msg void OnCreateRing();
	afx_msg void OnUpdateEditPrimitive(CCmdUI* pCmdUI);
	afx_msg void OnEditPrimitive();
	afx_msg void OnExportPrimitive();
	afx_msg void OnUpdateExportPrimitive(CCmdUI* pCmdUI);
	afx_msg void OnKillSceneLight();
	afx_msg void OnPrelitMultipass();
	afx_msg void OnUpdatePrelitMultipass(CCmdUI* pCmdUI);
	afx_msg void OnPrelitMultitex();
	afx_msg void OnUpdatePrelitMultitex(CCmdUI* pCmdUI);
	afx_msg void OnPrelitVertex();
	afx_msg void OnUpdatePrelitVertex(CCmdUI* pCmdUI);
	afx_msg void OnAddToLineup();
	afx_msg void OnUpdateAddToLineup(CCmdUI* pCmdUI);
	afx_msg void OnImportFacialAnims();
	afx_msg void OnUpdateImportFacialAnims(CCmdUI* pCmdUI);
	afx_msg void OnRestrictAnims();
	afx_msg void OnUpdateRestrictAnims(CCmdUI* pCmdUI);
	afx_msg void OnBindSubobjectLod();
	afx_msg void OnUpdateBindSubobjectLod(CCmdUI* pCmdUI);
	afx_msg void OnSetCameraDistance();
	afx_msg void OnObjectAlternateMaterials();
	afx_msg void OnCreateSoundObject();
	afx_msg void OnEditSoundObject();
	afx_msg void OnUpdateEditSoundObject(CCmdUI* pCmdUI);
	afx_msg void OnExportSoundObj();
	afx_msg void OnUpdateExportSoundObj(CCmdUI* pCmdUI);
	afx_msg void OnWireframeMode();
	afx_msg void OnUpdateWireframeMode(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBackgroundFog(CCmdUI* pCmdUI);
	afx_msg void OnBackgroundFog();
	afx_msg void OnUpdateScaleEmitter(CCmdUI* pCmdUI);
	afx_msg void OnScaleEmitter();
	afx_msg void OnUpdateToggleSorting(CCmdUI* pCmdUI);
	afx_msg void OnToggleSorting();
	afx_msg void OnCameraBonePosX();
	afx_msg void OnUpdateCameraBonePosX(CCmdUI* pCmdUI);
	afx_msg void OnViewPatchGapFill();
	afx_msg void OnUpdateViewPatchGapFill(CCmdUI* pCmdUI);
	afx_msg void OnViewSubdivision1();
	afx_msg void OnUpdateViewSubdivision1(CCmdUI* pCmdUI);
	afx_msg void OnViewSubdivision2();
	afx_msg void OnUpdateViewSubdivision2(CCmdUI* pCmdUI);
	afx_msg void OnViewSubdivision3();
	afx_msg void OnUpdateViewSubdivision3(CCmdUI* pCmdUI);
	afx_msg void OnViewSubdivision4();
	afx_msg void OnUpdateViewSubdivision4(CCmdUI* pCmdUI);
	afx_msg void OnViewSubdivision5();
	afx_msg void OnUpdateViewSubdivision5(CCmdUI* pCmdUI);
	afx_msg void OnViewSubdivision6();
	afx_msg void OnUpdateViewSubdivision6(CCmdUI* pCmdUI);
	afx_msg void OnViewSubdivision7();
	afx_msg void OnUpdateViewSubdivision7(CCmdUI* pCmdUI);
	afx_msg void OnViewSubdivision8();
	afx_msg void OnUpdateViewSubdivision8(CCmdUI* pCmdUI);
	afx_msg void OnMungeSortOnLoad();
	afx_msg void OnUpdateMungeSortOnLoad(CCmdUI* pCmdUI);
	afx_msg void OnEnableGammaCorrection();
	afx_msg void OnUpdateEnableGammaCorrection(CCmdUI* pCmdUI);
	afx_msg void OnSetGamma();
	afx_msg void OnEditAnimatedSoundsOptions();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	
	//////////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////////
	CView *GetPane (int iRow, int iCol) const
	{ return (CView *)m_wndSplitter.GetPane (iRow, iCol); }

	void	ShowObjectProperties (void);

	void	OnSelectionChanged (ASSET_TYPE newAssetType);

	void	Update_Frame_Time (DWORD milliseconds);
	void	UpdatePolygonCount (int iPolygons);
	void	Update_Particle_Count (int particles);
	void	UpdateCameraDistance (float cameraDistance);
	void	UpdateFrameCount (int iCurrentFrame, int iTotalFrames, float frame_rate);
	void	RestoreOriginalSize (void);
	void	Select_Device (bool show_dlg = true);

	HMENU	Get_Emitters_List_Menu (void) const { return m_hEmittersSubMenu; }
	void	Update_Emitters_List (void);

protected:
	
	//////////////////////////////////////////////////////////////////////
	//	Protected methods
	//////////////////////////////////////////////////////////////////////
	void	Restore_Window_State (void);

private:

	//////////////////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////////////////
	ASSET_TYPE m_currentAssetType;
	CFancyToolbar m_objectToolbar;
	CFancyToolbar m_animationToolbar;
	BOOL m_bShowAnimationBar;
	RECT m_OrigRect;
	HMENU m_hEmittersSubMenu;
	BOOL m_bInitialized;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__2BB39E2F_5D3A_11D2_9FC6_00104B791122__INCLUDED_)
