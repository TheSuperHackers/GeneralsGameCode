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

#if !defined(AFX_HIERARCHYPROPPAGE_H__FB40246E_5DFB_11D2_9FC7_00104B791122__INCLUDED_)
#define AFX_HIERARCHYPROPPAGE_H__FB40246E_5DFB_11D2_9FC7_00104B791122__INCLUDED_

#pragma once
// HierarchyPropPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CHierarchyPropPage dialog

class CHierarchyPropPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CHierarchyPropPage)

// Construction
public:
	CHierarchyPropPage (const CString &stringHierarchyName);
	~CHierarchyPropPage ();

// Dialog Data
	//{{AFX_DATA(CHierarchyPropPage)
	enum { IDD = IDD_PROP_PAGE_HIERARCHY };
	CListCtrl	m_subObjectListCtrl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CHierarchyPropPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CHierarchyPropPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnDblclkSubObjectList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

    private:
        // Private constructor that shouldn't be called
        CHierarchyPropPage () {};

        CString m_stringHierarchyName;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HIERARCHYPROPPAGE_H__FB40246E_5DFB_11D2_9FC7_00104B791122__INCLUDED_)
