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

// OpenMap.h : header file
//

#define MAP_OPENSAVE_PANEL_SECTION "MapOpenSavePanel"

typedef struct {
	CString filename;
	Bool		browse;
} TOpenMapInfo;

/////////////////////////////////////////////////////////////////////////////
// OpenMap dialog

class OpenMap : public CDialog
{
// Construction
public:
	OpenMap(TOpenMapInfo *pInfo, CWnd* pParent = nullptr);   // standard constructor

// Dialog Data
	//{{AFX_DATA(OpenMap)
	enum { IDD = IDD_OPEN_MAP };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(OpenMap)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	TOpenMapInfo *m_pInfo;
	void populateMapListbox( Bool systemMaps );
	Bool m_usingSystemDir;
protected:

	// Generated message map functions
	//{{AFX_MSG(OpenMap)
	afx_msg void OnBrowse();
	afx_msg void OnSystemMaps();
	afx_msg void OnUserMaps();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnDblclkOpenList();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
