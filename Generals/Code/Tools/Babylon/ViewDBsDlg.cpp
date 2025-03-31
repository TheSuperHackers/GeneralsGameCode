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

// ViewDBsDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "noxstring.h"
#include "noxstringDlg.h"
#include "VIEWDBSII.h"
#include "TransDB.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int ViewChanges = FALSE;

/////////////////////////////////////////////////////////////////////////////
// CViewDBsDlg dialog


VIEWDBSII::VIEWDBSII(CWnd* pParent /*=NULL*/)
	: CDialog(VIEWDBSII::IDD, pParent)
{
	//{{AFX_DATA_INIT(CViewDBsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void VIEWDBSII::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CViewDBsDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(VIEWDBSII, CDialog)
	//{{AFX_MSG_MAP(CViewDBsDlg)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewDBsDlg message handlers

static int label_count;
static void progress_cb ( void )
{
	label_count++;
	if ( MainDLG )
	{
		MainDLG->SetProgress ( label_count );
	}
}

HTREEITEM VIEWDBSII::create_full_view ( void )
{
	CTreeCtrl *tc = ( CTreeCtrl *) GetDlgItem ( IDC_TREEVIEW );
	HTREEITEM	root;
	TransDB	*db;
	int count = 0;
	const char *title = "Building database view...";

	MainDLG->Log ("");
	MainDLG->Status ( title );

	root = tc->InsertItem ( "DBs" );


	db = FirstTransDB ( );

	while ( db )
	{
		count += db->NumLabels ();
		count += db->NumObsolete ();
		db = db->Next ();
	}

	MainDLG->InitProgress ( count );

	label_count = 0;

	db = FirstTransDB ( );

	while ( db )
	{
		char buffer[100];

		sprintf ( buffer, "%s%s", title, db->Name());
		MainDLG->Status ( buffer, FALSE );
		db->AddToTree ( tc, root, FALSE, progress_cb );
		db = db->Next ();
	}

	MainDLG->Log ("OK", SAME_LINE );
	tc->Expand ( root, TVE_EXPAND );
	return root;
}

HTREEITEM VIEWDBSII::create_changes_view ( void )
{
	CTreeCtrl *tc = ( CTreeCtrl *) GetDlgItem ( IDC_TREEVIEW );
	HTREEITEM	root;
	TransDB	*db;
	int count = 0;
	const char *title = "Building changes view...";

	MainDLG->Log ("");
	MainDLG->Status ( title );



	db = FirstTransDB ( );

	while ( db )
	{
		if ( db->IsChanged ())
		{
			count += db->NumLabels ();
			count += db->NumObsolete ();
		}
		db = db->Next ();
	}

	if ( MainDLG )
	{
		MainDLG->InitProgress ( count );
	}

	if ( count )
	{
		root = tc->InsertItem ( "Changes" );
	}
	else
	{
		root = tc->InsertItem ( "No Changes" );
	}

	label_count = 0;

	db = FirstTransDB ( );

	while ( db )
	{
		char buffer[100];

		if ( db->IsChanged ())
		{
			sprintf ( buffer, "%s%s", title, db->Name());
			MainDLG->Status ( buffer, FALSE );
			db->AddToTree ( tc, root, TRUE, progress_cb );
		}
		db = db->Next ();
	}

	if ( MainDLG )
	{
		MainDLG->ProgressComplete ( );
		MainDLG->Log ("OK", SAME_LINE );
		MainDLG->Ready ();
	}

	tc->Expand ( root, TVE_EXPAND );
	return root;
}

BOOL VIEWDBSII::OnInitDialog() 
{
	HTREEITEM root;
	CDialog::OnInitDialog();

	// TODO: Add extra initialization here

	if ( !ViewChanges )
	{
		root = create_full_view ();
	}
	else
	{
		root = create_changes_view ();
	}


	MainDLG->Ready();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void VIEWDBSII::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	
	CTreeCtrl *tc = ( CTreeCtrl *) GetDlgItem ( IDC_TREEVIEW );
	HTREEITEM root = tc->GetRootItem ();
	tc->Expand ( root, TVE_COLLAPSE );
	tc->DeleteAllItems ();
	CDialog::OnClose();
}

