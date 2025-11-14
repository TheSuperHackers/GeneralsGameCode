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

// AssetPropertySheet.cpp : implementation file
//

#include "StdAfx.h"
#include "W3DView.h"
#include "AssetPropertySheet.h"

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAssetPropertySheet

IMPLEMENT_DYNAMIC(CAssetPropertySheet, CPropertySheet)

//////////////////////////////////////////////////////////////////////////
//
//  CAssetPropertySheet
//
CAssetPropertySheet::CAssetPropertySheet
(
    int iCaptionID,
    CPropertyPage *pCPropertyPage,
    CWnd *pCParentWnd
)
	: m_pCPropertyPage (pCPropertyPage),
      CPropertySheet (iCaptionID, pCParentWnd)
{
    ASSERT (m_pCPropertyPage);

    m_psh.dwFlags |= PSH_NOAPPLYNOW;

    // Add this page to the property sheet
    AddPage (m_pCPropertyPage);
    return ;
}

//////////////////////////////////////////////////////////////////////////
//
//  :~CAssetPropertySheet
//
CAssetPropertySheet::~CAssetPropertySheet ()
{
    return ;
}


BEGIN_MESSAGE_MAP(CAssetPropertySheet, CPropertySheet)
	//{{AFX_MSG_MAP(CAssetPropertySheet)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAssetPropertySheet message handlers
