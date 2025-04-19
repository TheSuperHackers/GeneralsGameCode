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

#if !defined(AFX_EXPORTDLG_H__DDA81307_4F1A_11D3_B9DA_006097B90D93__INCLUDED_)
#define AFX_EXPORTDLG_H__DDA81307_4F1A_11D3_B9DA_006097B90D93__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExportDlg.h : header file
//

#include "expimp.h"

/////////////////////////////////////////////////////////////////////////////
// CExportDlg dialog

class CExportDlg : public CDialog
{
			LangID	langid;
			char		filename[200];
			TROPTIONS	options;
			int			got_lang;

// Construction
public:

	LangID			Language ( void )			{ return langid; };
	char*				Filename ( void )			{ return filename; };
	TROPTIONS*	Options ( void )			{ return &options; };

	CExportDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CExportDlg)
	enum { IDD = IDD_EXPORT };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExportDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CExportDlg)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeCombolang();
	afx_msg void OnSelendokCombolang();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXPORTDLG_H__DDA81307_4F1A_11D3_B9DA_006097B90D93__INCLUDED_)
