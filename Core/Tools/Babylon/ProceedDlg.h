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

#if !defined(AFX_PROCEEDDLG_H__35C33E24_5AD8_11D3_B9DA_006097B90D93__INCLUDED_)
#define AFX_PROCEEDDLG_H__35C33E24_5AD8_11D3_B9DA_006097B90D93__INCLUDED_

#pragma once
// ProceedDlg.h : header file
//
#define IDALWAYS		101

/////////////////////////////////////////////////////////////////////////////
// ProceedDlg dialog

class ProceedDlg : public CDialog
{
// Construction
	const char *message;
public:
	ProceedDlg(const char *message, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(ProceedDlg)
	enum { IDD = IDD_PROCEED };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ProceedDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ProceedDlg)
	afx_msg void OnYes();
	afx_msg void OnAlways();
	afx_msg void OnNo();
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROCEEDDLG_H__35C33E24_5AD8_11D3_B9DA_006097B90D93__INCLUDED_)
