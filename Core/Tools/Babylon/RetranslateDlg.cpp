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

// RetranslateDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "Babylon.h"
#include "RetranslateDlg.h"

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// RetranslateDlg dialog


RetranslateDlg::RetranslateDlg(CWnd* pParent /*=NULL*/)
	: CDialog(RetranslateDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(RetranslateDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void RetranslateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(RetranslateDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(RetranslateDlg, CDialog)
	//{{AFX_MSG_MAP(RetranslateDlg)
	ON_WM_CANCELMODE()
	ON_BN_CLICKED(IDC_MATCH, OnRetranslate)
	ON_BN_CLICKED(IDC_SKIP, OnSkip)
	ON_BN_CLICKED(IDC_NOMATCH, OnNoRetranslate)
	ON_BN_CLICKED(IDCANCEL, OnSkipAll)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// RetranslateDlg message handlers

BOOL RetranslateDlg::OnInitDialog() 
{
	// TODO: Add extra initialization here
	CStatic *text;
	static char buffer[4*1024];


	CDialog::OnInitDialog();

	text = (CStatic *) GetDlgItem ( IDC_NEWTEXT );
	text->SetWindowText ( newtext->GetSB());

	text = (CStatic *) GetDlgItem ( IDC_OLDTEXT );
	text->SetWindowText ( oldtext->GetSB());

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void RetranslateDlg::OnCancelMode() 
{
	CDialog::OnCancelMode();
	
	// TODO: Add your message handler code here
	
}

void RetranslateDlg::OnRetranslate() 
{
	// TODO: Add your control notification handler code here

		
	oldtext->SetRetranslate ( TRUE );
	CDialog::OnOK ();
}

void RetranslateDlg::OnSkip() 
{
	// TODO: Add your control notification handler code here
	
		 EndDialog ( IDSKIP );
}

void RetranslateDlg::OnNoRetranslate() 
{
	// TODO: Add your control notification handler code here
	
	oldtext->SetRetranslate ( FALSE );
	CDialog::OnOK ();
}

void RetranslateDlg::OnSkipAll() 
{
	// TODO: Add your control notification handler code here
	
	CDialog::OnCancel ();
}
