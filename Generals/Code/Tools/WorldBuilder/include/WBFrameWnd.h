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

#pragma once

// WBFrameWnd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CWBFrameWnd frame

class CWBFrameWnd : public CFrameWnd
{
	DECLARE_DYNCREATE(CWBFrameWnd)
protected:
	CWBFrameWnd();           // protected constructor used by dynamic creation

// Attributes
public:

// Operations
public:

// Overrides
	//{{AFX_VIRTUAL(CWBFrameWnd)
	public:
	virtual BOOL LoadFrame(UINT nIDResource,
				DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE,
				CWnd* pParentWnd = NULL,
				CCreateContext* pContext = NULL);
	// ClassWizard generated virtual function overrides
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CWBFrameWnd();

	// Generated message map functions
	//{{AFX_MSG(CWBFrameWnd)
	afx_msg void OnMove(int x, int y);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
protected:
	CellSizeToolBar		m_cellSizeToolBar;

};

class CWB3dFrameWnd : public CMainFrame
{
	DECLARE_DYNCREATE(CWB3dFrameWnd)
protected:
	CWB3dFrameWnd();           // protected constructor used by dynamic creation
// Operations
public:
// Overrides
	virtual BOOL LoadFrame(UINT nIDResource,
				DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE,
				CWnd* pParentWnd = NULL,
				CCreateContext* pContext = NULL);
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWB3dFrameWnd)
	public:
	//}}AFX_VIRTUAL
// Implementation
protected:
	virtual ~CWB3dFrameWnd();
	// Generated message map functions
	//{{AFX_MSG(CWB3dFrameWnd)
	afx_msg void OnMove(int x, int y);
	afx_msg void OnWindowPreview1024x768();
	afx_msg void OnUpdateWindowPreview1024x768(CCmdUI* pCmdUI);
	afx_msg void OnWindowPreview640x480();
	afx_msg void OnUpdateWindowPreview640x480(CCmdUI* pCmdUI);
	afx_msg void OnWindowPreview800x600();
	afx_msg void OnUpdateWindowPreview800x600(CCmdUI* pCmdUI);
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
