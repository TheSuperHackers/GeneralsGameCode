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

// MatchDlg.h : header file
//

#include "TransDB.h"
#define IDSKIP		100

/////////////////////////////////////////////////////////////////////////////
// CMatchDlg dialog

class CMatchDlg : public CDialog
{
// Construction
public:
	CMatchDlg(CWnd* pParent = nullptr);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMatchDlg)
	enum { IDD = IDD_MATCH };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMatchDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMatchDlg)
	virtual void OnCancel();
	afx_msg void OnNomatch();
	afx_msg void OnMatch();
	virtual BOOL OnInitDialog();
	afx_msg void OnSkip();
	afx_msg void OnSelchangeMatchcombo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern BabylonText *MatchingBabylonText;
extern BabylonText *MatchOriginalText;
extern BabylonLabel *MatchLabel;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
