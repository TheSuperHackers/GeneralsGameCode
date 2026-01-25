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

// Report.h : header file
//

#include "expimp.h"

/////////////////////////////////////////////////////////////////////////////
// CReport dialog

class CReport : public CDialog
{
	char filename[300];
	RPOPTIONS	options;
	LangID	langids[200];
	int			langindices[200];
	int			num_langs;
	CListBox *list;
	CEdit		*limit;
	CButton *dialog_status;
	CButton *trans_status;
	CButton *show_details;
	CButton *ifless;
	CStatic *items;


// Construction
public:
	CReport(CWnd* pParent = nullptr);   // standard constructor

	char*					Filename	( void )			{ return filename; };
	RPOPTIONS*		Options			( void )		{ return &options; };
	LangID*				Langauges		( void )		{ return langids; };

// Dialog Data
	//{{AFX_DATA(CReport)
	enum { IDD = IDD_CREATE_REPORTS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CReport)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CReport)
	virtual BOOL OnInitDialog();
	afx_msg void OnInvert();
	afx_msg void OnSelectall();
	afx_msg void OnShowDetails();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
