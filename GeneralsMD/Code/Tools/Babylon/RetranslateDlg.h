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

#if !defined(AFX_RETRANSLATEDLG_H__19E25401_0ECB_11D4_B9DB_006097B90D93__INCLUDED_)
#define AFX_RETRANSLATEDLG_H__19E25401_0ECB_11D4_B9DB_006097B90D93__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RetranslateDlg.h : header file
//

#include "TransDB.h"
#define IDSKIP		100
/////////////////////////////////////////////////////////////////////////////
// RetranslateDlg dialog

class RetranslateDlg : public CDialog
{
// Construction
public:

	BabylonText *newtext;
	BabylonText *oldtext;

	RetranslateDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(RetranslateDlg)
	enum { IDD = IDD_RETRANSLATE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(RetranslateDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(RetranslateDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnCancelMode();
	afx_msg void OnRetranslate();
	afx_msg void OnSkip();
	afx_msg void OnNoRetranslate();
	afx_msg void OnSkipAll();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RETRANSLATEDLG_H__19E25401_0ECB_11D4_B9DB_006097B90D93__INCLUDED_)
