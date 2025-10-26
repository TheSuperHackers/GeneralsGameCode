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

// FindDialog.cpp : implementation file
//

#include "stdafx.h"
#include "wdump.h"
#include "FindDialog.h"

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Static data.
char FindDialog::_FindString [MAX_FIND_STRING_LENGTH + 1] = "";
bool FindDialog::_Found;

/////////////////////////////////////////////////////////////////////////////
// FindDialog dialog


FindDialog::FindDialog(CWnd* pParent /*=NULL*/)
	: CDialog(FindDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(FindDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void FindDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FindDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(FindDialog, CDialog)
	//{{AFX_MSG_MAP(FindDialog)
	ON_EN_CHANGE(IDC_FIND_STRING, OnChangeFindString)
	ON_EN_UPDATE(IDC_FIND_STRING, OnUpdateFindString)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// FindDialog message handlers

BOOL FindDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	((CEdit*) GetDlgItem (IDC_FIND_STRING))->SetLimitText (MAX_FIND_STRING_LENGTH);
	GetDlgItem (IDC_FIND_STRING)->SetWindowText (_FindString);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void FindDialog::OnChangeFindString()
{
	GetDlgItem (IDC_FIND_STRING)->GetWindowText (_FindString, MAX_FIND_STRING_LENGTH);
}

void FindDialog::OnUpdateFindString()
{
	GetDlgItem (IDOK)->EnableWindow (GetDlgItem (IDC_FIND_STRING)->GetWindowTextLength() > 0);
}
