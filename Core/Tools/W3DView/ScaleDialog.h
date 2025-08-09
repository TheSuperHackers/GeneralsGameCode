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

#if !defined(AFX_SCALEDIALOG_H__E86BBE8B_F527_11D3_A08F_00104B791122__INCLUDED_)
#define AFX_SCALEDIALOG_H__E86BBE8B_F527_11D3_A08F_00104B791122__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
//
// ScaleDialogClass
//
/////////////////////////////////////////////////////////////////////////////
class ScaleDialogClass : public CDialog
{
// Construction
public:
	ScaleDialogClass (float scale, CWnd* pParent=NULL, const char *prompt_string="");

// Dialog Data
	//{{AFX_DATA(ScaleDialogClass)
	enum { IDD = IDD_SCALE_KEY };
	CSpinButtonCtrl	m_ScaleSpin;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ScaleDialogClass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ScaleDialogClass)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	/////////////////////////////////////////////////////////////
	//	Public methods
	/////////////////////////////////////////////////////////////
	float					Get_Scale (void) const { return m_Scale; }

private:

	/////////////////////////////////////////////////////////////
	//	Private member data
	/////////////////////////////////////////////////////////////
	float					m_Scale;
	CString				m_Prompt;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCALEDIALOG_H__E86BBE8B_F527_11D3_A08F_00104B791122__INCLUDED_)
