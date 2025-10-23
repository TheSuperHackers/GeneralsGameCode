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

// AssetPropertySheet.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAssetPropertySheet

class CAssetPropertySheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CAssetPropertySheet)

// Construction
public:
    CAssetPropertySheet (int iCaptionID, CPropertyPage *pCPropertyPage, CWnd *pCParentWnd = NULL);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAssetPropertySheet)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CAssetPropertySheet();

	// Generated message map functions
protected:
	//{{AFX_MSG(CAssetPropertySheet)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

    private:
        // Private constructors (shouldn't be called)
	    CAssetPropertySheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0) {}
	    CAssetPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0) {}

        CPropertyPage *m_pCPropertyPage;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.
