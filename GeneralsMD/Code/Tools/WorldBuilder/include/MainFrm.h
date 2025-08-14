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

// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__371EC7AB_29D3_11D5_8CE0_00010297BBAC__INCLUDED_)
#define AFX_MAINFRM_H__371EC7AB_29D3_11D5_8CE0_00010297BBAC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Lib/BaseType.h"
#include "MyToolbar.h"
#include "brushoptions.h"
#include "FeatherOptions.h"
#include "CellWidth.h"
#include "TerrainMaterial.h"
#include "BlendMaterial.h"
#include "MoundOptions.h"
#include "ObjectOptions.h"
#include "FenceOptions.h"
#include "RoadOptions.h"
#include "ContourOptions.h"
#include "MeshMoldOptions.h"
#include "WaypointOptions.h"
#include "WaterOptions.h"
#include "LightOptions.h"
#include "mapobjectprops.h"
#include "GroveOptions.h"
#include "RampOptions.h"
#include "GlobalLightOptions.h"
#include "CameraOptions.h"
#include "ScorchOptions.h"
#include "BuildList.h"
#include "RulerOptions.h"

#define TWO_D_WINDOW_SECTION "TwoDWindow"
#define MAIN_FRAME_SECTION "MainFrame"

class LayersList;
class ScriptDialog;

class CMainFrame : public CFrameWnd
{
  DECLARE_DYNAMIC(CMainFrame)

public:
	CMainFrame();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef RTS_DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	static CMainFrame *GetMainFrame() { return TheMainFrame; }

	void showOptionsDialog(Int dialogID);
	void OnEditGloballightoptions();
	void ResetWindowPositions(void);
	void adjustWindowSize(void);
	Bool isAutoSaving(void) {return m_autoSaving;};
	void handleCameraChange(void);
	void onEditScripts();

protected:  // control bar embedded members
	CStatusBar					m_wndStatusBar;
	CToolBar						m_wndToolBar;
	CToolBar						m_floatingToolBar;
	BrushOptions				m_brushOptions;
	TerrainMaterial			m_terrainMaterial;
	BlendMaterial				m_blendMaterial;
	ObjectOptions				m_objectOptions;
	FenceOptions				m_fenceOptions;
	MapObjectProps			m_mapObjectProps;
	MoundOptions				m_moundOptions;
	RoadOptions					m_roadOptions;
	FeatherOptions			m_featherOptions;
	MeshMoldOptions			m_meshMoldOptions;
	WaypointOptions			m_waypointOptions;
	WaterOptions				m_waterOptions;
	LightOptions				m_lightOptions;
	BuildList						m_buildListOptions;
	GroveOptions				m_groveOptions;
	RampOptions					m_rampOptions;
	ScorchOptions				m_scorchOptions;
	COptionsPanel				m_noOptions;
	GlobalLightOptions	m_globalLightOptions;
	CameraOptions				m_cameraOptions;
	LayersList*					m_layersList;
	ScriptDialog*				m_scriptDialog;
	RulerOptions				m_rulerOptions;

	CWnd							*m_curOptions;
	Int								m_curOptionsX;
	Int								m_curOptionsY;
	Int								m_optionsPanelWidth;
	Int								m_optionsPanelHeight;
	Int								m_globalLightOptionsWidth;
	Int								m_globalLightOptionsHeight;

	Int								m_3dViewWidth;

	Bool							m_autoSaving;  ///< True if we are autosaving.
	UINT							m_hAutoSaveTimer;  ///< Timer that triggers for autosave.
	Bool							m_autoSave;    ///< If true, then do autosaves.
	Int								m_autoSaveInterval;  ///< Time between autosaves in seconds.

	static CMainFrame *TheMainFrame;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnViewBrushfeedback();
	afx_msg void OnUpdateViewBrushfeedback(CCmdUI* pCmdUI);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnEditCameraoptions();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__371EC7AB_29D3_11D5_8CE0_00010297BBAC__INCLUDED_)
