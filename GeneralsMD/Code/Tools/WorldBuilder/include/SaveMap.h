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

#if !defined(AFX_SAVEMAP_H__BCE93FF1_DA04_4F63_A483_D89BF1F49E32__INCLUDED_)
#define AFX_SAVEMAP_H__BCE93FF1_DA04_4F63_A483_D89BF1F49E32__INCLUDED_

#pragma once
// SaveMap.h : header file
//

#define MAP_OPENSAVE_PANEL_SECTION "MapOpenSavePanel"

typedef struct {
	CString filename;
	Bool		browse;
	Bool		usingSystemDir;
} TSaveMapInfo;

/////////////////////////////////////////////////////////////////////////////
// SaveMap dialog

class SaveMap : public CDialog
{
// Construction
public:
	SaveMap(TSaveMapInfo *pInfo, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(SaveMap)
	enum { IDD = IDD_SAVE_MAP };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SaveMap)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	TSaveMapInfo *m_pInfo;
	void populateMapListbox( Bool systemMaps );
	Bool m_usingSystemDir;
protected:

	// Generated message map functions
	//{{AFX_MSG(SaveMap)
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnBrowse();
	afx_msg void OnSystemMaps();
	afx_msg void OnUserMaps();
	afx_msg void OnSelchangeSaveList();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SAVEMAP_H__BCE93FF1_DA04_4F63_A483_D89BF1F49E32__INCLUDED_)
