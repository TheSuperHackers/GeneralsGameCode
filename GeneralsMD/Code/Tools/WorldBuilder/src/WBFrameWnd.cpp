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

// WBFrameWnd.cpp : implementation file
//

#include "StdAfx.h"
#include "WorldBuilder.h"
#include "MainFrm.h"
#include "WBFrameWnd.h"
#include "WorldBuilderDoc.h"
#include "WHeightMapEdit.h"
#include "wbview3d.h"
#include "ToastDialog.h"
/////////////////////////////////////////////////////////////////////////////
// CWBFrameWnd

IMPLEMENT_DYNCREATE(CWBFrameWnd, CFrameWnd)

CWBFrameWnd::CWBFrameWnd()
{
}

CWBFrameWnd::~CWBFrameWnd()
{
}

BOOL CWBFrameWnd::LoadFrame(UINT nIDResource,
				DWORD dwDefaultStyle,
				CWnd* pParentWnd,
				CCreateContext* pContext) {
	//dwDefaultStyle &= ~(WS_SIZEBOX|WS_MAXIMIZEBOX|WS_SYSMENU);

	BOOL ret = CFrameWnd::LoadFrame(nIDResource, dwDefaultStyle, CMainFrame::GetMainFrame(), pContext);
	if (ret) {
		Int top = ::AfxGetApp()->GetProfileInt(TWO_D_WINDOW_SECTION, "Top", 10);
		Int left =::AfxGetApp()->GetProfileInt(TWO_D_WINDOW_SECTION, "Left", 10);
		this->SetWindowPos(NULL, left,
			top, 0, 0,
			SWP_NOZORDER|SWP_NOSIZE);
		if (!m_cellSizeToolBar.Create(this, IDD_CELL_SLIDER, CBRS_LEFT, IDD_CELL_SLIDER))
		{
			DEBUG_CRASH(("Failed to create toolbar\n"));
		}
		EnableDocking(CBRS_ALIGN_ANY);
		m_cellSizeToolBar.SetupSlider();
		m_cellSizeToolBar.EnableDocking(CBRS_ALIGN_ANY);
		DockControlBar(&m_cellSizeToolBar);
	}
	return(ret);
}

void CWBFrameWnd::OnMove(int x, int y) 
{
	CFrameWnd::OnMove(x, y);
	if (this->IsWindowVisible() && !this->IsIconic()) {
		CRect frameRect;
		GetWindowRect(&frameRect);
		::AfxGetApp()->WriteProfileInt(TWO_D_WINDOW_SECTION, "Top", frameRect.top);
		::AfxGetApp()->WriteProfileInt(TWO_D_WINDOW_SECTION, "Left", frameRect.left);
	}
}


BEGIN_MESSAGE_MAP(CWBFrameWnd, CFrameWnd)
	//{{AFX_MSG_MAP(CWBFrameWnd)
	ON_WM_MOVE()
	ON_WM_SIZE()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWBFrameWnd message handlers


/////////////////////////////////////////////////////////////////////////////
// CWB3dFrameWnd

IMPLEMENT_DYNCREATE(CWB3dFrameWnd, CMainFrame)

CWB3dFrameWnd::CWB3dFrameWnd()
{
}

CWB3dFrameWnd::~CWB3dFrameWnd()
{
}


BEGIN_MESSAGE_MAP(CWB3dFrameWnd, CMainFrame)
	//{{AFX_MSG_MAP(CWB3dFrameWnd)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	ON_WM_MOVE()
	ON_WM_SIZE()
	// ON_WM_TIMER() // Bugs out main frame timer beware
	ON_COMMAND(ID_WINDOW_PREVIEW1024X768, OnWindowPreview1024x768)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_PREVIEW1024X768, OnUpdateWindowPreview1024x768)
	ON_COMMAND(ID_WINDOW_PREVIEW640X480, OnWindowPreview640x480)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_PREVIEW640X480, OnUpdateWindowPreview640x480)
	ON_COMMAND(ID_WINDOW_PREVIEW800X600, OnWindowPreview800x600)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_PREVIEW800X600, OnUpdateWindowPreview800x600)
	ON_COMMAND(ID_WINDOW_PREVIEW1280X768, OnWindowPreview1280x768)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_PREVIEW1280X768, OnUpdateWindowPreview1280x768)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWB3dFrameWnd message handlers
BOOL CWB3dFrameWnd::LoadFrame(UINT nIDResource,
				DWORD dwDefaultStyle,
				CWnd* pParentWnd,
				CCreateContext* pContext) {
	// dwDefaultStyle &= ~(WS_SIZEBOX);

	// m_disableOnSize = true;
	BOOL ret = CMainFrame::LoadFrame(nIDResource, dwDefaultStyle, CMainFrame::GetMainFrame(), pContext);
	// SetTimer(2, 5000, NULL);
	return(ret);
}


void CWB3dFrameWnd::OnMove(int x, int y) 
{
	CFrameWnd::OnMove(x, y);
	if (this->IsWindowVisible() && !this->IsIconic()) {
		CRect frameRect;
		GetWindowRect(&frameRect);
		::AfxGetApp()->WriteProfileInt(MAIN_FRAME_SECTION, "Top", frameRect.top);
		::AfxGetApp()->WriteProfileInt(MAIN_FRAME_SECTION, "Left", frameRect.left);
	}
}

BOOL CWB3dFrameWnd::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN)
    {
        if (pMsg->wParam == VK_ESCAPE)
        {
            ExitFullScreen();
            return TRUE; // handled
        }
    }
    return CFrameWnd::PreTranslateMessage(pMsg);
}

void CWB3dFrameWnd::ExitFullScreen()
{
    // Restore previous window style (border, caption, etc.)
    LONG style = WS_OVERLAPPEDWINDOW;
    ::SetWindowLong(m_hWnd, GWL_STYLE, style);

    // Get usable work area (screen minus taskbar)
    RECT workArea;
    ::SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);

    int borderspace = 100;

    // Calculate desired window size: use work area but leave border space
    int availableWidth  = (workArea.right - workArea.left) - (borderspace * 2);
    int availableHeight = (workArea.bottom - workArea.top) - (borderspace * 2);

    // Option 1: Use full available area minus borderspace
    int width  = availableWidth;
    int height = availableHeight;

    // Option 2: Use a fixed size smaller than available area
    // int width  = min(availableWidth, 1280);
    // int height = min(availableHeight, 720);

    // Center the window in the work area
    int left = workArea.left + ((workArea.right - workArea.left) - width) / 2;
    int top  = workArea.top  + ((workArea.bottom - workArea.top) - height) / 2;

    ::SetWindowPos(m_hWnd, HWND_TOP, left, top, width, height,
                   SWP_SHOWWINDOW | SWP_FRAMECHANGED);
}

/**
 * Adriane [Deathscythe] :  Much better resize option support
 */
void CWB3dFrameWnd::OnSize(UINT nType, int cx, int cy)
{
    CFrameWnd::OnSize(nType, cx, cy);

	switch (nType)
	{
		case SIZE_MAXIMIZED:
		{
			::MessageBeep(MB_OK);

			CToastDialog* pToast = new CToastDialog(_T("Click Escape to exit full screen"), 10000, false);
			pToast->Create(CToastDialog::IDD);
			pToast->ShowWindow(SW_SHOWNOACTIVATE);

			// Get the full screen size, including taskbar
			int screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
			int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);

			// Remove overlapped window styles that may prevent full coverage
			LONG style = ::GetWindowLong(m_hWnd, GWL_STYLE);
			style &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
			::SetWindowLong(m_hWnd, GWL_STYLE, style);

			::SetWindowPos(m_hWnd, HWND_TOP, 0, 0, screenWidth, screenHeight,
						SWP_SHOWWINDOW | SWP_FRAMECHANGED);
			break;
		}
		case SIZE_RESTORED:
		{
			
			// // CRect rect;
			// int top = ::AfxGetApp()->GetProfileInt(OPTIONS_PANEL_SECTION, "Top", 10);
			// int left = ::AfxGetApp()->GetProfileInt(OPTIONS_PANEL_SECTION, "Left", 10);
			// int right = ::AfxGetApp()->GetProfileInt(OPTIONS_PANEL_SECTION, "Right", 800);
			// int bottom = ::AfxGetApp()->GetProfileInt(OPTIONS_PANEL_SECTION, "Bottom", 600);
			// ::SetWindowPos(m_hWnd, HWND_TOP, left, top, right - left, bottom - top,
			// 			SWP_SHOWWINDOW | SWP_FRAMECHANGED);
			// ScheduleAdjustViewAfterResize();
			break;
		}
	}

    if (nType == SIZE_MINIMIZED) return;
// DEBUG_LOG(("Ignored resize? %s\n", m_disableOnSize ? "Yes" : "No"));
// 	if (m_disableOnSize) return; 
    // m_newWidth = cx;
    // m_newHeight = cy;

	// Kill any existing timer and start a new one
	ScheduleAdjustViewAfterResize();
    // KillTimer(1);
    // SetTimer(1, 300, NULL);  // 300ms delay to detect when resizing stops
	// DEBUG_LOG(("OnSize Width: %d\n", m_newWidth));
	// DEBUG_LOG(("OnSize Height: %d\n", m_newHeight));
}

void CWB3dFrameWnd::OnTimer(UINT nIDEvent)
{
	// if (nIDEvent == 2)
	// {
	// 	KillTimer(2);
	// 	m_disableOnSize = false;
	// 	DEBUG_LOG(("Initialization Finished!!\n"));
	// 	return;
	// }
    // if (nIDEvent == 1 && !m_disableOnSize) // Our resizing timer
	if (nIDEvent == 1) // Our resizing timer
    {
        KillTimer(1);  // Stop the timer
        // Apply new size and save it
        // if (m_newWidth > 0 && m_newHeight > 0)
        // {
            // ::AfxGetApp()->WriteProfileInt(MAIN_FRAME_SECTION, "Width", m_newWidth);
            // ::AfxGetApp()->WriteProfileInt(MAIN_FRAME_SECTION, "Height", m_newHeight);
            adjustWindowSize(false, true);
			DEBUG_LOG(("Size Adjusted!!\n"));
        // }
    }
    CFrameWnd::OnTimer(nIDEvent);
}

void CWB3dFrameWnd::OnWindowPreview1280x768() 
{
	if (m_3dViewWidth == 1280) return;
	::AfxGetApp()->WriteProfileInt(MAIN_FRAME_SECTION, "Width", 1280);
	::AfxGetApp()->WriteProfileInt(MAIN_FRAME_SECTION, "Height", 768);
	adjustWindowSize(true, false);
}

void CWB3dFrameWnd::OnUpdateWindowPreview1280x768(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_3dViewWidth==1280?1:0);
}

/**
 * Adriane [Deathscythe] :  End of code
 */
void CWB3dFrameWnd::OnWindowPreview1024x768() 
{
	if (m_3dViewWidth == 1024) return;
	::AfxGetApp()->WriteProfileInt(MAIN_FRAME_SECTION, "Width", 1024);
	::AfxGetApp()->WriteProfileInt(MAIN_FRAME_SECTION, "Height", 768);
	adjustWindowSize(true, false);
}

void CWB3dFrameWnd::OnUpdateWindowPreview1024x768(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_3dViewWidth==1024?1:0);
}

void CWB3dFrameWnd::OnWindowPreview640x480() 
{
	if (m_3dViewWidth == 640) return;
	::AfxGetApp()->WriteProfileInt(MAIN_FRAME_SECTION, "Width", 640);
	::AfxGetApp()->WriteProfileInt(MAIN_FRAME_SECTION, "Height", 480);
	adjustWindowSize(true, false);
}

void CWB3dFrameWnd::OnUpdateWindowPreview640x480(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_3dViewWidth==640?1:0);
}

void CWB3dFrameWnd::OnWindowPreview800x600() 
{
	if (m_3dViewWidth == 800) return;
	::AfxGetApp()->WriteProfileInt(MAIN_FRAME_SECTION, "Width", 800);
	::AfxGetApp()->WriteProfileInt(MAIN_FRAME_SECTION, "Height", 600);
	adjustWindowSize(true, false);
}

void CWB3dFrameWnd::OnUpdateWindowPreview800x600(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_3dViewWidth==800?1:0);
}
