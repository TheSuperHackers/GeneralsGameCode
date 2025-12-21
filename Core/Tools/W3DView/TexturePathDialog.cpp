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

// TexturePathDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "W3DView.h"
#include "W3DViewDoc.h"
#include "TexturePathDialog.h"
#include "Utils.h"
#include "DirectoryDialog.h"

#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// TexturePathDialogClass
//
/////////////////////////////////////////////////////////////////////////////
TexturePathDialogClass::TexturePathDialogClass(CWnd* pParent /*=nullptr*/)
	: CDialog(TexturePathDialogClass::IDD, pParent)
{
	//{{AFX_DATA_INIT(TexturePathDialogClass)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// DoDataExchange
//
/////////////////////////////////////////////////////////////////////////////
void
TexturePathDialogClass::DoDataExchange (CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(TexturePathDialogClass)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	return ;
}


BEGIN_MESSAGE_MAP(TexturePathDialogClass, CDialog)
	//{{AFX_MSG_MAP(TexturePathDialogClass)
	ON_BN_CLICKED(IDC_BROWSE1, OnBrowse1)
	ON_BN_CLICKED(IDC_BROWSE2, OnBrowse2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
//
// OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL
TexturePathDialogClass::OnInitDialog (void)
{
	CDialog::OnInitDialog ();

	CW3DViewDoc *doc = ::GetCurrentDocument ();

	SetDlgItemText (IDC_PATH1, doc->Get_Texture_Path1 ());
	SetDlgItemText (IDC_PATH2, doc->Get_Texture_Path2 ());
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnOK
//
/////////////////////////////////////////////////////////////////////////////
void
TexturePathDialogClass::OnOK (void)
{
	CString path1;
	CString path2;
	GetDlgItemText (IDC_PATH1, path1);
	GetDlgItemText (IDC_PATH2, path2);

	CW3DViewDoc *doc = ::GetCurrentDocument ();
	doc->Set_Texture_Path1 (path1);
	doc->Set_Texture_Path2 (path2);

	CDialog::OnOK ();
	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnBrowse1
//
/////////////////////////////////////////////////////////////////////////////
void
TexturePathDialogClass::OnBrowse1 (void)
{
	CString initial_path;
	GetDlgItemText (IDC_PATH1, initial_path);

	CString path;
	if (::Browse_For_Folder (m_hWnd, initial_path, path)) {
		SetDlgItemText (IDC_PATH1, path);
	}

	return ;
}


/////////////////////////////////////////////////////////////////////////////
//
// OnBrowse2
//
/////////////////////////////////////////////////////////////////////////////
void
TexturePathDialogClass::OnBrowse2 (void)
{
	CString initial_path;
	GetDlgItemText (IDC_PATH2, initial_path);

	CString path;
	if (::Browse_For_Folder (m_hWnd, initial_path, path)) {
		SetDlgItemText (IDC_PATH2, path);
	}

	return ;
}

