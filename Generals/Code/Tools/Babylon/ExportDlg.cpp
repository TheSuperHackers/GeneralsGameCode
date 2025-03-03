/*
**	Command & Conquer Generals(tm)
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

// ExportDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "noxstring.h"
#include "ExportDlg.h"
#include "direct.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static int max_index;

/////////////////////////////////////////////////////////////////////////////
// CExportDlg dialog


CExportDlg::CExportDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CExportDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExportDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CExportDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExportDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CExportDlg, CDialog)
	//{{AFX_MSG_MAP(CExportDlg)
	ON_CBN_SELCHANGE(IDC_COMBOLANG, OnSelchangeCombolang)
	ON_CBN_SELENDOK(IDC_COMBOLANG, OnSelendokCombolang)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExportDlg message handlers

void CExportDlg::OnOK() 
{
	char buffer[100];
	char *ptr;
	// TODO: Add extra validation here
	CEdit *edit = (CEdit *) GetDlgItem ( IDC_FILENAME );
	CButton *all = (CButton *) GetDlgItem ( IDC_RADIOALL );
	CButton *button;
	CButton *sample = (CButton *) GetDlgItem ( IDC_RADIOSAMPLE );
	CButton *dialog = (CButton *) GetDlgItem ( IDC_RADIODIALOG );
	CButton *nondialog = (CButton *) GetDlgItem ( IDC_RADIONONDIALOG );
	CButton *unverified = (CButton *) GetDlgItem ( IDC_RADIOUNVERIFIED );
	CButton *missing = (CButton *) GetDlgItem ( IDC_RADIOMISSING );
	CButton *unsent = (CButton *) GetDlgItem ( IDC_RADIOUNSENT );

	edit->GetWindowText ( buffer, sizeof ( filename) -1 );	
	_getcwd ( filename, sizeof (filename ) -1 );
	strcat ( filename, "\\" );
	if ( ( ptr = strchr ( buffer, '.' )))
	{
			*ptr = 0;
	}
	strcat ( filename, buffer );
	if ( all->GetCheck ())
	{
		options.filter = TR_ALL;
	}
	else if ( dialog->GetCheck ())
	{
		options.filter = TR_DIALOG;
	}
	else if ( nondialog->GetCheck ())
	{
		options.filter = TR_NONDIALOG;
	}
	else if ( sample->GetCheck ())
	{
		options.filter = TR_SAMPLE;
	}
	else if ( unverified->GetCheck ())
	{
		options.filter = TR_UNVERIFIED;
	}
	else if ( missing->GetCheck ())
	{
		options.filter = TR_MISSING_DIALOG;
	}
	else if ( unsent->GetCheck ())
	{
		options.filter = TR_UNSENT;
	}
	else
	{
		options.filter = TR_CHANGES;
	}

	options.include_comments = FALSE;
	button = (CButton *) GetDlgItem ( IDC_CHECKTRANS );
	options.include_translations = button->GetCheck ();
	
	CDialog::OnOK();
}

void CExportDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	langid = LANGID_UNKNOWN;
	CDialog::OnCancel();
}

BOOL CExportDlg::OnInitDialog() 
{
	int index;
	int lang_index;
	LANGINFO	*info;
	CComboBox *combo;
	CEdit *edit = (CEdit *) GetDlgItem ( IDC_FILENAME );
	CButton *button = (CButton *) GetDlgItem ( IDC_RADIOCHANGES );


	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	combo = (CComboBox *) GetDlgItem ( IDC_COMBOLANG );

	combo->SetItemDataPtr ( 0, NULL );

	options.filter = TR_CHANGES;
	options.include_comments = FALSE;
	options.include_translations = FALSE;
	langid = LANGID_UNKNOWN;
	filename[0] = 0;
	button->SetCheck ( 1 );



	index = 0;
	lang_index = 0;
	got_lang = FALSE;
	while ( (info = GetLangInfo ( lang_index )) )
	{
		if ( TRUE )//info->langid != LANGID_US )
		{
			combo->InsertString ( index, info->name );
			combo->SetItemDataPtr ( index, info );
			if ( info->langid == CurrentLanguage )
			{
				combo->SetCurSel ( index );
				got_lang = TRUE;
			}
			index++;
		}

		lang_index++; 
	}
	max_index = index;

	if ( !got_lang )
	{
		combo->InsertString ( 0, "Select language" );
		combo->SetCurSel ( 0 );
		max_index++;
	}

	edit->SetLimitText ( 8 );
	OnSelchangeCombolang ();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CExportDlg::OnSelchangeCombolang() 
{
	// TODO: Add your control notification handler code here
	LANGINFO *info = NULL;
	int index;
	CButton *export = (CButton *) GetDlgItem ( IDOK );
	CComboBox *combo = (CComboBox *) GetDlgItem ( IDC_COMBOLANG );
	CEdit *edit = (CEdit *) GetDlgItem ( IDC_FILENAME );

	index = combo->GetCurSel ();

	if ( index >= 0  && index < max_index )
	{
		info = (LANGINFO *) combo->GetItemDataPtr ( index );
	}
	
	if ( info )
	{
		char buffer[10];
		edit->EnableWindow ( TRUE );
		sprintf ( buffer, "Generals_%s", info->initials );
		edit->SetWindowText ( buffer );
		export->EnableWindow ( TRUE );
		langid = info->langid;
		if ( !got_lang )
		{
			combo->DeleteString ( 0 );
			max_index--;
			got_lang = TRUE;
		}
	}
	else
	{
		edit->SetWindowText ("");
		edit->EnableWindow ( FALSE );
		export->EnableWindow ( FALSE );
		langid = LANGID_UNKNOWN;
	}

}

void CExportDlg::OnSelendokCombolang() 
{
	// TODO: Add your control notification handler code here
	int i = 0;
}
