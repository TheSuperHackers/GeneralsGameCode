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

// EulaDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "WorldBuilder.h"
#include "euladialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// EulaDialog dialog


EulaDialog::EulaDialog(CWnd* pParent /*=NULL*/)
	: CDialog(EulaDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(EulaDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void EulaDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(EulaDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(EulaDialog, CDialog)
	//{{AFX_MSG_MAP(EulaDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// EulaDialog message handlers

BOOL EulaDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CString theText;
	theText.LoadString( IDS_EULA_AGREEMENT1 );
	
	CString concatText;
	concatText.LoadString( IDS_EULA_AGREEMENT2 );
	theText += concatText;
	concatText.LoadString( IDS_EULA_AGREEMENT3 );
	theText += concatText;
	concatText.LoadString( IDS_EULA_AGREEMENT4 );
	theText += concatText;
	concatText.LoadString( IDS_EULA_AGREEMENT5 );
	theText += concatText;
	
	CWnd *theEditDialog = GetDlgItem( IDC_EDIT1 );
	if( theEditDialog )
	{
		theEditDialog->SetWindowText( theText );
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
