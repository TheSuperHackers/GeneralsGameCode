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

#if !defined(AFX_SELECTMACROTEXTURE_H__0AB61BFA_3A67_40CB_A38F_7067F6BA352B__INCLUDED_)
#define AFX_SELECTMACROTEXTURE_H__0AB61BFA_3A67_40CB_A38F_7067F6BA352B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectMacrotexture.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// SelectMacrotexture dialog

class SelectMacrotexture : public CDialog
{
// Construction
public:
	SelectMacrotexture(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(SelectMacrotexture)
	enum { IDD = IDD_MACRO_TEXTURE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SelectMacrotexture)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

protected:
	CTreeCtrl		m_textureTreeView;


// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(SelectMacrotexture)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTMACROTEXTURE_H__0AB61BFA_3A67_40CB_A38F_7067F6BA352B__INCLUDED_)
