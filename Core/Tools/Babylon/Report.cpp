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

// Report.cpp : implementation file
//

#include "StdAfx.h"
#include "Babylon.h"
#include "Report.h"
#include <limits.h>

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CReport dialog


CReport::CReport(CWnd* pParent /*=nullptr*/)
	: CDialog(CReport::IDD, pParent)
{

	options.translations = TRUE;
	options.dialog = TRUE;
	options.limit = 0;
	langids[0] = LANGID_UNKNOWN;
	filename[0] = 0;
	//{{AFX_DATA_INIT(CReport)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CReport::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CReport)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CReport, CDialog)
	//{{AFX_MSG_MAP(CReport)
	ON_BN_CLICKED(IDC_INVERT, OnInvert)
	ON_BN_CLICKED(IDC_SELECTALL, OnSelectall)
	ON_BN_CLICKED(IDC_SHOW_DETAILS, OnShowDetails)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReport message handlers

BOOL CReport::OnInitDialog()
{
	int index;
	LANGINFO	*info;

	limit = (CEdit *) GetDlgItem ( IDC_LIMIT );
	trans_status = (CButton *) GetDlgItem ( IDC_TRANSLATION_STATUS );
	dialog_status = (CButton *) GetDlgItem ( IDC_DIALOG_STATUS );
	show_details = (CButton *) GetDlgItem ( IDC_SHOW_DETAILS );
	ifless = (CButton *) GetDlgItem ( IDC_IFLESS );
	list = (CListBox *) GetDlgItem ( IDC_LANGUAGE );
	items = (CStatic *) GetDlgItem ( IDC_ITEMS );

	CDialog::OnInitDialog();

	// TODO: Add extra initialization here

	trans_status->SetCheck ( options.translations );
	dialog_status->SetCheck ( options.dialog );
	show_details->SetCheck ( 0 );
	ifless->SetCheck ( 1 );
	limit->EnableWindow ( FALSE );
	ifless->EnableWindow ( FALSE );
	items->EnableWindow ( FALSE );
	limit->SetWindowText ( "100" );
	limit->SetLimitText ( 50 );
	options.limit = 100;

	index = 0;
	while ( (info = GetLangInfo ( index )) )
	{
		list->InsertString ( index,  info->name );
		if ( info->langid == CurrentLanguage )
		{
			list->SetSel ( index );
		}

		index++;
	}
	num_langs = index;


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CReport::OnSelectall()
{
	// TODO: Add your control notification handler code here
	 list->SelItemRange ( TRUE, 0, num_langs-1 );
}

void CReport::OnInvert()
{
	// TODO: Add your control notification handler code here
	int index = 0;


	while ( index < num_langs )
	{
		list->SetSel ( index,  !list->GetSel ( index ));
		index++;
	}


}



void CReport::OnShowDetails()
{
	// TODO: Add your control notification handler code here
	if ( show_details->GetCheck () == 0 )
	{
		ifless->EnableWindow ( FALSE );
		limit->EnableWindow ( FALSE );
		items->EnableWindow ( FALSE );
	}
	else
	{
		ifless->EnableWindow ( TRUE );
		limit->EnableWindow ( TRUE );
		items->EnableWindow ( TRUE );
	}
}

void CReport::OnOK()
{
	int count;
	int i;
	char buffer[100];

	count = list->GetSelItems ( num_langs, langindices );

	if ( !count )
	{
		AfxMessageBox ( "No languages selected" );
		return;
	}

// get the filename
	CFileDialog fd ( FALSE , nullptr, "*.txt",  OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR );

	if ( fd.DoModal () != IDOK )
	{
		return;
	}

	strcpy ( filename, fd.GetPathName ());

	num_langs = 0;
	for ( i = 0; i <count; i++ )
	{
		LANGINFO *info;

		if ( info = GetLangInfo ( langindices[i] ))
		{
			langids[num_langs++] = info->langid;
		}
	}

	langids[num_langs] = LANGID_UNKNOWN;

	options.dialog = dialog_status->GetCheck ();
	options.translations = trans_status->GetCheck ();
	limit->GetWindowText( buffer, sizeof(buffer)-1);
	options.limit = atoi ( buffer );

	if ( !show_details->GetCheck () )
	{
		options.limit = 0;
	}
	else if ( !ifless->GetCheck () )
	{
		options.limit = INT_MAX;
	}


	CDialog::OnOK();
}

void CReport::OnCancel()
{
	// TODO: Add extra cleanup here

	CDialog::OnCancel();
}
