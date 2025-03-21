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

// VIEWDBSII.cpp : implementation file
//

#include "StdAfx.h"
#include "Babylon.h"
#include "VIEWDBSII.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// VIEWDBSII dialog


VIEWDBSII::VIEWDBSII(CWnd* pParent /*=NULL*/)
	: CDialog(VIEWDBSII::IDD, pParent)
{
	//{{AFX_DATA_INIT(VIEWDBSII)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void VIEWDBSII::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(VIEWDBSII)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(VIEWDBSII, CDialog)
	//{{AFX_MSG_MAP(VIEWDBSII)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// VIEWDBSII message handlers
