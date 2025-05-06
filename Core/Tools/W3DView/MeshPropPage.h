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

#if !defined(AFX_MESHPROPPAGE_H__FB40246F_5DFB_11D2_9FC7_00104B791122__INCLUDED_)
#define AFX_MESHPROPPAGE_H__FB40246F_5DFB_11D2_9FC7_00104B791122__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// MeshPropPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMeshPropPage dialog

class CMeshPropPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CMeshPropPage)

// Construction
public:	
    CMeshPropPage (const CString &stringMeshName);
	~CMeshPropPage ();

// Dialog Data
	//{{AFX_DATA(CMeshPropPage)
	enum { IDD = IDD_PROP_PAGE_MESH };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CMeshPropPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CMeshPropPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

    private:
        CMeshPropPage () {};

        CString m_stringMeshName;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MESHPROPPAGE_H__FB40246F_5DFB_11D2_9FC7_00104B791122__INCLUDED_)
