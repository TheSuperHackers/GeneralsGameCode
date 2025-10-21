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

#if !defined(AFX_GENERATEDLG_H__959D0D41_50A5_11D3_B9DA_006097B90D93__INCLUDED_)
#define AFX_GENERATEDLG_H__959D0D41_50A5_11D3_B9DA_006097B90D93__INCLUDED_

#pragma once
// GenerateDlg.h : header file
//

#include "expimp.h"

/////////////////////////////////////////////////////////////////////////////
// CGenerateDlg dialog

class CGenerateDlg : public CDialog
{
	char filename[200];
	GNOPTIONS	options;
	LangID	langids[200];
	int			langindices[200];
	int num_langs;
	CListBox *list;
	CEdit *edit;
	CStatic *filetext;
	CButton *unicode;
	CButton *strfile;
	CButton *useids;
	CButton *usetext;

// Construction
public:
	CGenerateDlg(CWnd* pParent = NULL);   // standard constructor

	char*					FilePrefix	( void )		{ return filename; };
	GNOPTIONS*		Options			( void )		{ return &options; };
	LangID*				Langauges		( void )		{ return langids; };

// Dialog Data
	//{{AFX_DATA(CGenerateDlg)
	enum { IDD = IDD_GENERATE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGenerateDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGenerateDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelectall();
	afx_msg void OnInvert();
	afx_msg void OnChangePrefix();
	afx_msg void OnBabylonstr();
	afx_msg void OnUnicode();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnIds();
	afx_msg void OnOriginal();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GENERATEDLG_H__959D0D41_50A5_11D3_B9DA_006097B90D93__INCLUDED_)
