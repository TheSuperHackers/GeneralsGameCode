/*
**	Command & Conquer Renegade(tm)
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

#include "resource.h"


/////////////////////////////////////////////////////////////////////////////
//
// ResolutionDialogClass
//
/////////////////////////////////////////////////////////////////////////////
class ResolutionDialogClass : public CDialog
{
// Construction
public:
	ResolutionDialogClass(CWnd* pParent = nullptr);   // standard constructor

// Dialog Data
	//{{AFX_DATA(ResolutionDialogClass)
	enum { IDD = IDD_RESOLUTION };
	CListCtrl	m_ListCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ResolutionDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ResolutionDialogClass)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnDblclkResolutionListCtrl(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

/*public:

	///////////////////////////////////////////////////////////////////
	//	Public methods
	///////////////////////////////////////////////////////////////////
	int		Get_Width (void);
	int		Get_Height (void);
	int		Get_BPP (void);

private:

	///////////////////////////////////////////////////////////////////
	//	Private member data
	///////////////////////////////////////////////////////////////////
	int m_Width;
	int m_Height;
	int m_BPP;*/
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
