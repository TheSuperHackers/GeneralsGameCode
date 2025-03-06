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

// GenerateDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "noxstring.h"
#include "GenerateDlg.h"
#include "direct.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGenerateDlg dialog


CGenerateDlg::CGenerateDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGenerateDlg::IDD, pParent)
{
	options.format = GN_UNICODE;
	options.untranslated = GN_USEIDS;
	langids[0] = LANGID_UNKNOWN;
	filename[0] = 0;
	//{{AFX_DATA_INIT(CGenerateDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CGenerateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGenerateDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGenerateDlg, CDialog)
	//{{AFX_MSG_MAP(CGenerateDlg)
	ON_BN_CLICKED(IDC_SELECTALL, OnSelectall)
	ON_BN_CLICKED(IDC_INVERT, OnInvert)
	ON_EN_CHANGE(IDC_PREFIX, OnChangePrefix)
	ON_BN_CLICKED(IDC_NOXSTR, OnNoxstr)
	ON_BN_CLICKED(IDC_UNICODE, OnUnicode)
	ON_BN_CLICKED(IDC_IDS, OnIds)
	ON_BN_CLICKED(IDC_ORIGINAL, OnOriginal)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGenerateDlg message handlers

BOOL CGenerateDlg::OnInitDialog() 
{
	int index;
	LANGINFO	*info;

	edit = (CEdit *) GetDlgItem ( IDC_PREFIX );
	unicode = (CButton *) GetDlgItem ( IDC_UNICODE );
	strfile = (CButton *) GetDlgItem ( IDC_NOXSTR );
	useids = (CButton *) GetDlgItem ( IDC_IDS );
	usetext = (CButton *) GetDlgItem ( IDC_ORIGINAL );
	list = (CListBox *) GetDlgItem ( IDC_LANGUAGE );
	filetext = ( CStatic *) GetDlgItem ( IDC_FILENAME );

	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

	unicode->SetCheck ( 1 );
	useids->SetCheck ( 1 );
	edit->SetWindowText ( "Generals" );
	edit->SetLimitText ( 5 );

	OnChangePrefix ();


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

void CGenerateDlg::OnSelectall() 
{
	// TODO: Add your control notification handler code here
	 list->SelItemRange ( TRUE, 0, num_langs-1 );
}

void CGenerateDlg::OnInvert() 
{
	// TODO: Add your control notification handler code here
	int index = 0;


	while ( index < num_langs )
	{
		list->SetSel ( index,  !list->GetSel ( index ));
		index++;
	}


}

void CGenerateDlg::OnChangePrefix() 
{
	char buffer[30];

	edit->GetWindowText ( buffer, 6 );

	if ( options.format == GN_NOXSTR )
	{
		strcat ( buffer, "_{xx}.str" );
	}
	else
	{
		strcat ( buffer, "_{xx}.csf" );
	}

	filetext->SetWindowText ( buffer );
	
}

void CGenerateDlg::OnNoxstr() 
{
	// TODO: Add your control notification handler code here
	options.format = GN_NOXSTR;
	OnChangePrefix ();
	unicode->SetCheck ( 0 );

	
}

void CGenerateDlg::OnUnicode() 
{
	// TODO: Add your control notification handler code here
	options.format = GN_UNICODE;
	OnChangePrefix ();
	strfile->SetCheck ( 0 );
	
}

void CGenerateDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CGenerateDlg::OnOK() 
{
	char buffer[30];
	int count;
	int i;
	// TODO: Add extra validation here

	edit->GetWindowText ( buffer, sizeof ( filename) -1 );	
	_getcwd ( filename, sizeof (filename ) -1 );
	strcat ( filename, "\\" );
	strcat ( filename, buffer );

	count = list->GetSelItems ( num_langs, langindices );

	if ( !count )
	{
		AfxMessageBox ( "No languages selected" );
		return;
	}

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

	
	CDialog::OnOK();
}

void CGenerateDlg::OnIds() 
{
	options.untranslated = GN_USEIDS;
	usetext->SetCheck ( 0 );
	
}

void CGenerateDlg::OnOriginal() 
{
	options.untranslated = GN_USEORIGINAL;
	useids->SetCheck ( 0 );
}
