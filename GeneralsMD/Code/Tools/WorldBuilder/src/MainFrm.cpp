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

// MainFrm.cpp : implementation of the CMainFrame class
//

#include "StdAfx.h"
#include "MainFrm.h"

#include "Common/GlobalData.h"

#include "DrawObject.h"
#include "LayersList.h"
#include "WHeightMapEdit.h"
#include "wbview3d.h"
#include "WorldBuilder.h"
#include "WorldBuilderDoc.h"
#include "WorldBuilderView.h"
#include "ToastDialog.h"

#include "ScriptDialog.h"
#define ADJUST_VIEW_TIMER 6969
#define COUNTDOWN_TIMER 6910

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_MOVE()
	ON_COMMAND(ID_VIEW_BRUSHFEEDBACK, OnViewBrushfeedback)
	ON_UPDATE_COMMAND_UI(ID_VIEW_BRUSHFEEDBACK, OnUpdateViewBrushfeedback)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_CANCELMODE()
	ON_COMMAND(ID_EDIT_CAMERAOPTIONS, OnEditCameraoptions)
	ON_WM_DROPFILES()  
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

CMainFrame *CMainFrame::TheMainFrame = NULL;

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	TheMainFrame = this;
	m_curOptions = NULL;
	m_hAutoSaveTimer = NULL;
	m_autoSaving = false;
	m_layersList = NULL;
	m_curDialogID = IDD_NO_OPTIONS;
	m_scriptDialog = NULL;
	// DragAcceptFiles(TRUE);
}

void CMainFrame::OnDropFiles(HDROP hDropInfo)
{
    UINT nFiles = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);

    for (UINT i = 0; i < nFiles; i++)
    {
        TCHAR szFile[MAX_PATH];
        DragQueryFile(hDropInfo, i, szFile, MAX_PATH);

        CString path = szFile;
        if (path.Right(4).CompareNoCase(".map") == 0) // accept only .map files
        {
            // Use MFC doc template system to open the map
            AfxGetApp()->OpenDocumentFile(path);
        }
    }

    DragFinish(hDropInfo);
}


CMainFrame::~CMainFrame()
{
	if (m_layersList) {
		delete m_layersList;
	}

	if (m_scriptDialog) {
		delete m_scriptDialog;
		m_scriptDialog = NULL;
	}

	SaveBarState("MainFrame");
	TheMainFrame = NULL;
	::AfxGetApp()->WriteProfileInt(MAIN_FRAME_SECTION, "AutoSave", m_autoSave);
	::AfxGetApp()->WriteProfileInt(MAIN_FRAME_SECTION, "AutoSaveIntervalSeconds", m_autoSaveInterval);
    CoUninitialize();
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	adjustWindowSize();
	CRect frameRect;
	GetWindowRect(&frameRect);

	CWnd *pDesk = GetDesktopWindow();
	CRect top;
	pDesk->GetWindowRect(&top);
	top.left += 10;
	top.top += 10;
	top.top = ::AfxGetApp()->GetProfileInt(MAIN_FRAME_SECTION, "Top", top.top);
	top.left =::AfxGetApp()->GetProfileInt(MAIN_FRAME_SECTION, "Left", top.left);
	SetWindowPos(NULL, top.left, top.top, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
	GetWindowRect(&frameRect);
	EnableDocking(CBRS_ALIGN_TOP);

#if 0 // For a floating toolbar.
#define WRAP(btn) m_floatingToolBar.SetButtonStyle( btn, m_floatingToolBar.GetButtonStyle( btn )|TBBS_WRAPPED)
	if (!m_floatingToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_LEFT
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_FIXED ) ||
		!m_floatingToolBar.LoadToolBar(IDR_TOOLBAR2))
		WRAP(1);
	WRAP(4);
	WRAP(6);
	WRAP(9);
	WRAP(11);
	WRAP(14);
	WRAP(16);
#undef WRAP
	CPoint pos(frameRect.left,frameRect.top+60);
	this->FloatControlBar(&m_floatingToolBar, pos, CBRS_ALIGN_LEFT);
	m_floatingToolBar.EnableDocking(CBRS_ALIGN_TOP); 
#endif

	if (!m_wndStatusBar.Create(this) || !m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT)))
	{
		DEBUG_CRASH(("Failed to create status bar\n"));
	}

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_FIXED ) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
 	 m_wndToolBar.EnableDocking(CBRS_ALIGN_TOP);

	frameRect.left = frameRect.right;
	frameRect.top = ::AfxGetApp()->GetProfileInt(OPTIONS_PANEL_SECTION, "Top", frameRect.top);
	frameRect.left =::AfxGetApp()->GetProfileInt(OPTIONS_PANEL_SECTION, "Left", frameRect.left);



	m_brushOptions.Create(IDD_BRUSH_OPTIONS, this);
	m_brushOptions.SetWindowPos(NULL, frameRect.left, frameRect.top,	0, 0, SWP_NOZORDER|SWP_NOSIZE);
	m_brushOptions.GetWindowRect(&frameRect);
	m_optionsPanelWidth = frameRect.Width();
	m_optionsPanelHeight = frameRect.Height();

	m_featherOptions.Create(IDD_FEATHER_OPTIONS, this);
	m_featherOptions.SetWindowPos(NULL, frameRect.left, frameRect.top,	0, 0, SWP_NOZORDER|SWP_NOSIZE);
	m_featherOptions.GetWindowRect(&frameRect);
	if (m_optionsPanelWidth < frameRect.Width()) m_optionsPanelWidth = frameRect.Width();
	if (m_optionsPanelHeight < frameRect.Height()) m_optionsPanelHeight = frameRect.Height();


	m_noOptions.Create(IDD_NO_OPTIONS, this);
	m_noOptions.SetWindowPos(NULL, frameRect.left, frameRect.top,	0, 0, SWP_NOZORDER|SWP_NOSIZE);
	m_noOptions.GetWindowRect(&frameRect);
	if (m_optionsPanelWidth < frameRect.Width()) m_optionsPanelWidth = frameRect.Width();
	if (m_optionsPanelHeight < frameRect.Height()) m_optionsPanelHeight = frameRect.Height();
	
	m_terrainMaterial.Create(IDD_TERRAIN_MATERIAL, this);
	m_terrainMaterial.SetWindowPos(NULL, frameRect.left, frameRect.top,	0, 0, SWP_NOZORDER|SWP_NOSIZE);
	m_terrainMaterial.GetWindowRect(&frameRect);
	if (m_optionsPanelWidth < frameRect.Width()) m_optionsPanelWidth = frameRect.Width();
	if (m_optionsPanelHeight < frameRect.Height()) m_optionsPanelHeight = frameRect.Height();

	m_blendMaterial.Create(IDD_BLEND_MATERIAL, this);
	m_blendMaterial.SetWindowPos(NULL, frameRect.left, frameRect.top,	0, 0, SWP_NOZORDER|SWP_NOSIZE);
	m_blendMaterial.GetWindowRect(&frameRect);
	if (m_optionsPanelWidth < frameRect.Width()) m_optionsPanelWidth = frameRect.Width();
	if (m_optionsPanelHeight < frameRect.Height()) m_optionsPanelHeight = frameRect.Height();

	m_moundOptions.Create(IDD_MOUND_OPTIONS, this);
	m_moundOptions.SetWindowPos(NULL, frameRect.left, frameRect.top,	0, 0, SWP_NOZORDER|SWP_NOSIZE);
	m_moundOptions.GetWindowRect(&frameRect);
	if (m_optionsPanelWidth < frameRect.Width()) m_optionsPanelWidth = frameRect.Width();
	if (m_optionsPanelHeight < frameRect.Height()) m_optionsPanelHeight = frameRect.Height();

	m_rulerOptions.Create(IDD_RULER_OPTIONS, this);
	m_rulerOptions.SetWindowPos(NULL, frameRect.left, frameRect.top,	0, 0, SWP_NOZORDER|SWP_NOSIZE);
	m_rulerOptions.GetWindowRect(&frameRect);
	if (m_optionsPanelWidth < frameRect.Width()) m_optionsPanelWidth = frameRect.Width();
	if (m_optionsPanelHeight < frameRect.Height()) m_optionsPanelHeight = frameRect.Height();

	m_objectOptions.Create(IDD_OBJECT_OPTIONS, this);
	m_objectOptions.SetWindowPos(NULL, frameRect.left, frameRect.top, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
	m_objectOptions.GetWindowRect(&frameRect);
	if (m_optionsPanelWidth < frameRect.Width()) m_optionsPanelWidth = frameRect.Width();
	if (m_optionsPanelHeight < frameRect.Height()) m_optionsPanelHeight = frameRect.Height();

	m_fenceOptions.Create(IDD_FENCE_OPTIONS, this);
	m_fenceOptions.SetWindowPos(NULL, frameRect.left, frameRect.top, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
	m_fenceOptions.GetWindowRect(&frameRect);
	if (m_optionsPanelWidth < frameRect.Width()) m_optionsPanelWidth = frameRect.Width();
	if (m_optionsPanelHeight < frameRect.Height()) m_optionsPanelHeight = frameRect.Height();

	m_mapObjectProps.Create(IDD_MAPOBJECT_PROPS, this);
	m_mapObjectProps.makeMain();
	m_mapObjectProps.SetWindowPos(NULL, frameRect.left, frameRect.top, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
	m_mapObjectProps.GetWindowRect(&frameRect);
	if (m_optionsPanelWidth < frameRect.Width()) m_optionsPanelWidth = frameRect.Width();
	if (m_optionsPanelHeight < frameRect.Height()) m_optionsPanelHeight = frameRect.Height();

	m_roadOptions.Create(IDD_ROAD_OPTIONS, this);
	m_roadOptions.SetWindowPos(NULL, frameRect.left, frameRect.top, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
 	m_roadOptions.GetWindowRect(&frameRect);
	if (m_optionsPanelWidth < frameRect.Width()) m_optionsPanelWidth = frameRect.Width();
	if (m_optionsPanelHeight < frameRect.Height()) m_optionsPanelHeight = frameRect.Height();

	m_waypointOptions.Create(IDD_WAYPOINT_OPTIONS, this);
	m_waypointOptions.SetWindowPos(NULL, frameRect.left, frameRect.top, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
 	m_waypointOptions.GetWindowRect(&frameRect);
	if (m_optionsPanelWidth < frameRect.Width()) m_optionsPanelWidth = frameRect.Width();
	if (m_optionsPanelHeight < frameRect.Height()) m_optionsPanelHeight = frameRect.Height();

	m_waterOptions.Create(IDD_WATER_OPTIONS, this);
	m_waterOptions.SetWindowPos(NULL, frameRect.left, frameRect.top, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
 	m_waterOptions.GetWindowRect(&frameRect);
	if (m_optionsPanelWidth < frameRect.Width()) m_optionsPanelWidth = frameRect.Width();
	if (m_optionsPanelHeight < frameRect.Height()) m_optionsPanelHeight = frameRect.Height();

	m_lightOptions.Create(IDD_LIGHT_OPTIONS, this);
	m_lightOptions.SetWindowPos(NULL, frameRect.left, frameRect.top, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
 	m_lightOptions.GetWindowRect(&frameRect);
	if (m_optionsPanelWidth < frameRect.Width()) m_optionsPanelWidth = frameRect.Width();
	if (m_optionsPanelHeight < frameRect.Height()) m_optionsPanelHeight = frameRect.Height();

	m_meshMoldOptions.Create(IDD_MESHMOLD_OPTIONS, this);
	m_meshMoldOptions.SetWindowPos(NULL, frameRect.left, frameRect.top, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
 	m_meshMoldOptions.GetWindowRect(&frameRect);
	if (m_optionsPanelWidth < frameRect.Width()) m_optionsPanelWidth = frameRect.Width();
	if (m_optionsPanelHeight < frameRect.Height()) m_optionsPanelHeight = frameRect.Height();

	m_buildListOptions.Create(IDD_BUILD_LIST_PANEL, this);
	m_buildListOptions.SetWindowPos(NULL, frameRect.left, frameRect.top, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
 	m_buildListOptions.GetWindowRect(&frameRect);
	if (m_optionsPanelWidth < frameRect.Width()) m_optionsPanelWidth = frameRect.Width();
	if (m_optionsPanelHeight < frameRect.Height()) m_optionsPanelHeight = frameRect.Height();

	m_groveOptions.Create(IDD_GROVE_OPTIONS, this);
	m_groveOptions.makeMain();
	m_groveOptions.SetWindowPos(NULL, frameRect.left, frameRect.top, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
 	m_groveOptions.GetWindowRect(&frameRect);
	if (m_optionsPanelWidth < frameRect.Width()) m_optionsPanelWidth = frameRect.Width();
	if (m_optionsPanelHeight < frameRect.Height()) m_optionsPanelHeight = frameRect.Height();

	m_rampOptions.Create(IDD_RAMP_OPTIONS, this);
	m_rampOptions.SetWindowPos(NULL, frameRect.left, frameRect.top, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
 	m_rampOptions.GetWindowRect(&frameRect);
	if (m_optionsPanelWidth < frameRect.Width()) m_optionsPanelWidth = frameRect.Width();
	if (m_optionsPanelHeight < frameRect.Height()) m_optionsPanelHeight = frameRect.Height();

	m_scorchOptions.Create(IDD_SCORCH_OPTIONS, this);
	m_scorchOptions.SetWindowPos(NULL, frameRect.left, frameRect.top, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
 	m_scorchOptions.GetWindowRect(&frameRect);
	if (m_optionsPanelWidth < frameRect.Width()) m_optionsPanelWidth = frameRect.Width();
	if (m_optionsPanelHeight < frameRect.Height()) m_optionsPanelHeight = frameRect.Height();

	frameRect.top = ::AfxGetApp()->GetProfileInt(GLOBALLIGHT_OPTIONS_PANEL_SECTION, "Top", frameRect.top);
	frameRect.left =::AfxGetApp()->GetProfileInt(GLOBALLIGHT_OPTIONS_PANEL_SECTION, "Left", frameRect.left);

	m_globalLightOptions.Create(IDD_GLOBAL_LIGHT_OPTIONS, this);
	m_globalLightOptions.SetWindowPos(NULL, frameRect.left, frameRect.top, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
 	m_globalLightOptions.GetWindowRect(&frameRect);
	m_globalLightOptionsWidth = frameRect.Width();
	m_globalLightOptionsHeight = frameRect.Height();

	frameRect.top = ::AfxGetApp()->GetProfileInt(CAMERA_OPTIONS_PANEL_SECTION, "Top", frameRect.top);
	frameRect.left =::AfxGetApp()->GetProfileInt(CAMERA_OPTIONS_PANEL_SECTION, "Left", frameRect.left);

	m_cameraOptions.Create(IDD_CAMERA_OPTIONS, this);
	m_cameraOptions.SetWindowPos(NULL, frameRect.left, frameRect.top, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
 	m_cameraOptions.GetWindowRect(&frameRect);
	
	// now, setup the Layers Panel
	m_layersList = new LayersList(LayersList::IDD, this);
	m_layersList->Create(LayersList::IDD, this);
	m_layersList->ShowWindow(::AfxGetApp()->GetProfileInt(MAIN_FRAME_SECTION, "ShowLayersList", 0) ? SW_SHOW : SW_HIDE);
	
	CRect optionsRect;
	m_globalLightOptions.GetWindowRect(&optionsRect);
	m_layersList->SetWindowPos(NULL, optionsRect.left, optionsRect.bottom + 100, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
  
	Int sbf = ::AfxGetApp()->GetProfileInt(MAIN_FRAME_SECTION, "ShowBrushFeedback", 1);
	if (sbf != 0) {
		DrawObject::enableFeedback();
	} else {
		DrawObject::disableFeedback();
	}
	
	Int autoSave = ::AfxGetApp()->GetProfileInt(MAIN_FRAME_SECTION, "AutoSave", 1);
	m_autoSave = autoSave != 0;
	autoSave = ::AfxGetApp()->GetProfileInt(MAIN_FRAME_SECTION, "AutoSaveIntervalSeconds", 120);
	m_autoSaveInterval = autoSave;
	m_hAutoSaveTimer = this->SetTimer(1, m_autoSaveInterval*1000, NULL);
	if (m_autoSave) {
		m_nextAutoSaveTime = CTime::GetCurrentTime() + CTimeSpan(0, 0, 0, m_autoSaveInterval);
		SetTimer(COUNTDOWN_TIMER, 1000, NULL);
	}
#if USE_STREAMING_AUDIO
	StartMusic();
#endif

	DragAcceptFiles(TRUE);
	return 0;
}

void CMainFrame::adjustWindowSize(Bool forcedResolution, Bool dynamicResolution)
{
	DEBUG_LOG(("Adjusting window size"));
	// if (m_disableOnSize){
	// 	Int viewWidth = ::AfxGetApp()->GetProfileInt(MAIN_FRAME_SECTION, "Width", THREE_D_VIEW_WIDTH);
	// 	Int viewHeight = ::AfxGetApp()->GetProfileInt(MAIN_FRAME_SECTION, "Height", THREE_D_VIEW_HEIGHT);
	// 		DEBUG_LOG(("viewWidth: %d, viewHeight: %d\n", viewWidth, viewHeight));
	// 	return;
	// } 

	HWND hDesk = ::GetDesktopWindow();
	CRect top;
	::GetWindowRect(hDesk, &top);
	top.right -= 2*::GetSystemMetrics(SM_CYCAPTION);
	top.bottom -= 3*::GetSystemMetrics(SM_CYCAPTION);

	CRect client, window;
	Int borderX = ::GetSystemMetrics(SM_CXEDGE);
//	Int borderY = ::GetSystemMetrics(SM_CYEDGE);
	Int viewWidth = ::AfxGetApp()->GetProfileInt(MAIN_FRAME_SECTION, "Width", THREE_D_VIEW_WIDTH);
	Int viewHeight = ::AfxGetApp()->GetProfileInt(MAIN_FRAME_SECTION, "Height", THREE_D_VIEW_HEIGHT);
	WbView3d * pView = CWorldBuilderDoc::GetActive3DView();	
	if (pView) {
		pView->GetClientRect(&client);
	}	else {
		GetClientRect(&client);
		client.right -= 2*borderX;
	}
	    // No use to us anymore
		// int widthDelta = client.Width() - (viewWidth);
		// int heightDelta = client.Height() - (viewHeight);
		
		Int newWidth = 0;
		Int newHeight = 0; 
		this->GetWindowRect(window);
		
		/**
		 * Adriane [Deathscythe] 
		 * `forcedResolution` is `true` by default. The check below preserves the old behavior  
		 * (using specific resolution values) while supporting the new one.  
		 * 
		 * If `forcedResolution` is true, we use the provided resolution.  
		 * If `dynamicResolution` is true and `forcedResolution` is false, we use the current window size.  
		 * Otherwise, we fallback to using the specific resolution values.
		 */
		if (forcedResolution) {
			newWidth = viewWidth;
			newHeight = viewHeight;
		} else if (dynamicResolution) {
			newWidth = window.Width();
			newHeight = window.Height();

			// Save the new dynamic resolution -- Make sure its greater than 0 or else if we load a 0 it will crash the wb everytime
			if(newWidth > 0 && newHeight > 0){
				::AfxGetApp()->WriteProfileInt(MAIN_FRAME_SECTION, "Width", newWidth);
				::AfxGetApp()->WriteProfileInt(MAIN_FRAME_SECTION, "Height", newHeight);
			}
		} else {
			newWidth = viewWidth;
			newHeight = viewHeight;
		}


	this->SetWindowPos(NULL, 0,
	0, newWidth, newHeight,
	SWP_NOMOVE|SWP_NOZORDER); // MainFrm.cpp sets the top and left.
	if (pView) {
		pView->reset3dEngineDisplaySize(newWidth, newHeight);
	}
	
	/**  This is responsible for the check icon for the resolution selector -- 
	 * make sure we sent similar values to the ones we have in the menu or else it wont have that check icon
	*/ 
	m_3dViewWidth = newWidth;

	// DEBUG_LOG(("Client Width: %d, Client Height: %d\n", client.Width(), client.Height()));
	// DEBUG_LOG(("widthDelta: %d, heightDelta: %d\n", widthDelta, heightDelta));
	// DEBUG_LOG(("OLD viewWidth: %d, OLD viewHeight: %d\n", viewWidth, viewHeight));
	// DEBUG_LOG(("New Width: %d, New Height: %d\n", newWidth, newHeight));
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	return TRUE;
}

void CMainFrame::ResetWindowPositions(void)
{
	if (m_curOptions == NULL) {
		m_curOptions = &m_brushOptions;
	}
	SetWindowPos(NULL, 20, 20, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
	ShowWindow(SW_SHOW);
	m_curOptions->SetWindowPos(NULL, 40, 40, 0, 0,  SWP_NOSIZE|SWP_NOZORDER);
	m_curOptions->ShowWindow(SW_SHOW);
	CView *pView = CWorldBuilderDoc::GetActive2DView();
	if (pView) {
		CWnd *pParent = pView->GetParentFrame();
		if (pParent) {
			pParent->SetWindowPos(NULL, 60, 60, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
		}
	}
	CPoint pos(20,200);

	this->FloatControlBar(&m_floatingToolBar, pos, CBRS_ALIGN_LEFT);
	m_floatingToolBar.SetWindowPos(NULL, pos.x, pos.y, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
	m_floatingToolBar.ShowWindow(SW_SHOW);
}

void CMainFrame::showOptionsDialog(Int dialogID)
{
	if (dialogID == m_curDialogID && m_curOptions && m_curOptions->IsWindowVisible()) {
		// DEBUG_LOG(("Already showing visible dialog ID: %d\n", dialogID));
		return;
	}

	/**
	 * Adriane [Deathscythe]
	 * Suggested feature -- lets not entertain the huge ass dialog that blocks the app 
	 * just hide that damn thing ..
	 */
	if (dialogID == IDD_NO_OPTIONS) {
		// DEBUG_LOG(("Hiding current options dialog (IDD_NO_OPTIONS triggered).\n"));
		if (m_curOptions) {
			m_curOptions->ShowWindow(SW_HIDE);
			m_curOptions = NULL;
		}
		return;
	}

	CWnd *newOptions = NULL;
	switch(dialogID) {
		case IDD_BRUSH_OPTIONS : newOptions = &m_brushOptions; break;
		case IDD_TERRAIN_MATERIAL: newOptions = &m_terrainMaterial; break;
		case IDD_BLEND_MATERIAL: newOptions = &m_blendMaterial; break;
		case IDD_OBJECT_OPTIONS: newOptions = &m_objectOptions; break;
		case IDD_FENCE_OPTIONS: newOptions = &m_fenceOptions; break;
		case IDD_MAPOBJECT_PROPS: newOptions = &m_mapObjectProps; break;
		case IDD_ROAD_OPTIONS:newOptions  = &m_roadOptions; break;
		case IDD_MOUND_OPTIONS:newOptions  = &m_moundOptions; break;
		case IDD_RULER_OPTIONS:newOptions  = &m_rulerOptions; break;
		case IDD_FEATHER_OPTIONS:newOptions  = &m_featherOptions; break;
		case IDD_MESHMOLD_OPTIONS:newOptions  = &m_meshMoldOptions; break;
		case IDD_WAYPOINT_OPTIONS:newOptions  = &m_waypointOptions; break;
		case IDD_WATER_OPTIONS:newOptions  = &m_waterOptions; break;
		case IDD_LIGHT_OPTIONS:newOptions  = &m_lightOptions; break;		
		case IDD_BUILD_LIST_PANEL:newOptions  = &m_buildListOptions; break;		
		case IDD_GROVE_OPTIONS:newOptions = &m_groveOptions; break;
		case IDD_RAMP_OPTIONS:newOptions = &m_rampOptions; break;
		case IDD_SCORCH_OPTIONS:newOptions = &m_scorchOptions; break;
		case IDD_NO_OPTIONS:newOptions  = &m_noOptions; break;
		default : break;												 
	}																						 
	CRect frameRect;
	if (newOptions && newOptions != m_curOptions) {
		newOptions->GetWindowRect(&frameRect);
		if (m_curOptions) {
			m_curOptions->GetWindowRect(&frameRect);
		}
		/**
		 * Adriane [Deathscythe] -- Bug fix
		 * These panels just wouldn't behave, so I had to force them to use the actual saved position values
		 * as their base location.
		 */
		int top = ::AfxGetApp()->GetProfileInt(OPTIONS_PANEL_SECTION, "Top", 10);
		int left = ::AfxGetApp()->GetProfileInt(OPTIONS_PANEL_SECTION, "Left", 10);
		newOptions->SetWindowPos(m_curOptions, left, top, 
			m_optionsPanelWidth, m_optionsPanelHeight, 
			SWP_NOZORDER | SWP_NOACTIVATE );
		newOptions->ShowWindow(SW_SHOWNA);
		if (m_curOptions) {
			m_curOptions->ShowWindow(SW_HIDE);
		}
		m_curOptions = newOptions;
		m_curDialogID = dialogID;
		// DEBUG_LOG(("Current ID----------:%d\n", m_curDialogID));
	}
}

void CMainFrame::OnEditGloballightoptions() 
{
	m_globalLightOptions.ShowWindow(SW_SHOWNA);
}

void CMainFrame::onEditScripts()
{
	if (m_scriptDialog) {
		// Delete the old one since it is no longer valid.
		delete m_scriptDialog;
	}

	CRect frameRect;
	GetWindowRect(&frameRect);

	frameRect.top = ::AfxGetApp()->GetProfileInt(SCRIPT_DIALOG_SECTION, "Top", frameRect.top);
	frameRect.left =::AfxGetApp()->GetProfileInt(SCRIPT_DIALOG_SECTION, "Left", frameRect.left);
	// Setup the Script Dialog.
	// This needs to be recreated each time so that it will have the current data.
	m_scriptDialog = new ScriptDialog(this);
	m_scriptDialog->Create(IDD_ScriptDialog, this);
	m_scriptDialog->SetWindowPos(NULL, frameRect.left, frameRect.top, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
 	m_scriptDialog->GetWindowRect(&frameRect);
	m_scriptDialog->ShowWindow(SW_SHOWNA);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


#if DEAD
	void CMainFrame::OnEditContouroptions() 
	{
		ContourOptions contourOptsDialog(this);	
		contourOptsDialog.DoModal();
	}
#endif

void CMainFrame::OnMove(int x, int y) 
{
	CFrameWnd::OnMove(x, y);
	if (this->IsWindowVisible() && !this->IsIconic()) {
		CRect frameRect;
		GetWindowRect(&frameRect);
		::AfxGetApp()->WriteProfileInt(MAIN_FRAME_SECTION, "Top", frameRect.top);
		::AfxGetApp()->WriteProfileInt(MAIN_FRAME_SECTION, "Left", frameRect.left);
	}
}

void CMainFrame::OnViewBrushfeedback() 
{
	if (DrawObject::isFeedbackEnabled()) {
		DrawObject::disableFeedback();
		::AfxGetApp()->WriteProfileInt(MAIN_FRAME_SECTION, "ShowBrushFeedback", 0);
	} else {
		DrawObject::enableFeedback();
		::AfxGetApp()->WriteProfileInt(MAIN_FRAME_SECTION, "ShowBrushFeedback", 1);
	}
}

void CMainFrame::OnUpdateViewBrushfeedback(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(DrawObject::isFeedbackEnabled()?1:0);
}

void CMainFrame::OnDestroy() 
{
	if (m_hAutoSaveTimer) {
		KillTimer(m_hAutoSaveTimer);
	}
	m_hAutoSaveTimer = NULL;

	KillTimer(ADJUST_VIEW_TIMER);
	CFrameWnd::OnDestroy();
}

void CMainFrame::ScheduleAdjustViewAfterResize(void) 
{
    KillTimer(ADJUST_VIEW_TIMER);
    SetTimer(ADJUST_VIEW_TIMER, 300, NULL);  // 300ms delay to detect when resizing stops
}

void CMainFrame::OnTimer(UINT nIDEvent) 
{
    if (nIDEvent == ADJUST_VIEW_TIMER)
    {
        KillTimer(ADJUST_VIEW_TIMER);
        adjustWindowSize(false, true);
        return;
    }
	if (nIDEvent == COUNTDOWN_TIMER) // UI update timer
	{
		CTime currentTime = CTime::GetCurrentTime();
		CTimeSpan diff = m_nextAutoSaveTime - currentTime;
		double secondsRemaining = diff.GetTotalSeconds();

		// DEBUG_LOG(("Countdown timer active. Seconds until auto-save: %.0f\n", secondsRemaining));

		CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
        if (pDoc && pDoc->needAutoSave() && secondsRemaining <= 10 && secondsRemaining > 0)
		{
			CString msg;
			msg.Format("Autosaving in %.0f seconds...", secondsRemaining);
			// DEBUG_LOG(("SetMessageText: %s", msg));

			SetMessageText(msg);

			// Play a system sound as a cue (only once at 10 seconds)
			if ((int)secondsRemaining == 10)
			{
				CToastDialog* pToast = new CToastDialog(_T(msg), 10000, false);
				pToast->Create(CToastDialog::IDD);
				pToast->ShowWindow(SW_SHOWNOACTIVATE);
					
				PlaySound((LPCTSTR)SND_ALIAS_SYSTEMASTERISK, NULL, SND_ALIAS_ID | SND_ASYNC);
			}
		}

		// Ensure view refresh
		CView* pView = GetActiveView();
		if (pView) pView->Invalidate(); // force redraw if needed

		return;
	}

    if (nIDEvent == 1) // Auto-save timer
    {
        CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
        if (pDoc && pDoc->needAutoSave()) {
            m_autoSaving = true;

            HCURSOR old = SetCursor(::LoadCursor(0, IDC_WAIT));
            SetMessageText("Auto Saving map...");
            pDoc->autoSave();

            if (old) SetCursor(old);
            SetMessageText("Auto Save Complete.");
            m_autoSaving = false;

            CView* pView = GetActiveView();
            if (pView) pView->Invalidate();
        }

		// 🔁 Set the next expected save time first
		m_nextAutoSaveTime = CTime::GetCurrentTime() + CTimeSpan(0, 0, 0, m_autoSaveInterval);
        return;
    }
}

void CMainFrame::OnEditCameraoptions() 
{
	m_cameraOptions.ShowWindow(SW_SHOWNA);
}

void CMainFrame::handleCameraChange(void)
{
	m_cameraOptions.update();
}

