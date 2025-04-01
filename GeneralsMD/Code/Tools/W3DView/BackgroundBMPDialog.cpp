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

// BackgroundBMPDialog.cpp : implementation file
//

#include "stdafx.h"
#include "w3dview.h"
#include "BackgroundBMPDialog.h"
#include "W3DViewDoc.H"
#include "Utils.H"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBackgroundBMPDialog dialog

/////////////////////////////////////////////////////////////
//
//  CBackgroundBMPDialog
//
CBackgroundBMPDialog::CBackgroundBMPDialog (CWnd* pParent /*=NULL*/)
	: CDialog(CBackgroundBMPDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBackgroundBMPDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
    return ;
}

/////////////////////////////////////////////////////////////
//
//  DoDataExchange
//
void
CBackgroundBMPDialog::DoDataExchange (CDataExchange* pDX)
{
	// Allow the base class to process this message
    CDialog::DoDataExchange (pDX);

	//{{AFX_DATA_MAP(CBackgroundBMPDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
    return ;
}


BEGIN_MESSAGE_MAP(CBackgroundBMPDialog, CDialog)
	//{{AFX_MSG_MAP(CBackgroundBMPDialog)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBackgroundBMPDialog message handlers

/////////////////////////////////////////////////////////////
//
//  OnInitDialog
//
BOOL
CBackgroundBMPDialog::OnInitDialog (void)
{
	// Allow the base class to process this message
    CDialog::OnInitDialog ();

    // Center the dialog around the data tree view instead
    // of the direct center of the screen
    ::CenterDialogAroundTreeView (m_hWnd);

    // Gett a pointer to the current document
    CW3DViewDoc *pCDoc = ::GetCurrentDocument ();
    if (pCDoc)
    {
        // Set the initial filename in the edit control
        SetDlgItemText (IDC_FILENAME_EDIT, pCDoc->GetBackgroundBMP ());
    }

	return TRUE;
}

/////////////////////////////////////////////////////////////
//
//  OnOK
//
void
CBackgroundBMPDialog::OnOK (void) 
{
    // Gett a pointer to the current document
    CW3DViewDoc *pCDoc = ::GetCurrentDocument ();
    if (pCDoc)
    {
        CString stringBackgroundBMPName;
        
        // Get the filename the user entered
        if (GetDlgItemText (IDC_FILENAME_EDIT, stringBackgroundBMPName) > 0)
        {
            // Ask the doc to create a new background from this BMP
            pCDoc->SetBackgroundBMP (stringBackgroundBMPName);
        }
        else
        {
            // Ask the doc to clear any existing background BMP
            pCDoc->SetBackgroundBMP (NULL);
        }
    }
	
	// Allow the base class to process this message
    CDialog::OnOK ();
    return ;
}

/////////////////////////////////////////////////////////////
//
//  OnBrowse
//
void
CBackgroundBMPDialog::OnBrowse (void)
{
    // Get a pointer to the current document
    CW3DViewDoc *pCDoc = ::GetCurrentDocument ();
    if (pCDoc)
    {
        CFileDialog openFileDialog (TRUE,
                                    ".tga",
                                    pCDoc->GetBackgroundBMP (),
                                    OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
                                    "Targa files (*.tga)|*.tga||",
                                    this);

        // Ask the user what Targa file they wish to load
        if (openFileDialog.DoModal () == IDOK)
        {
            // Set the text of the filename edit control
            SetDlgItemText (IDC_FILENAME_EDIT, openFileDialog.GetPathName ());
        }
    }

    return ;
}
