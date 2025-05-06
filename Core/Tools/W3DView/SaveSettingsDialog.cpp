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

// SaveSettingsDialog.cpp : implementation file
//

#include "stdafx.h"
#include "W3DView.h"
#include "SaveSettingsDialog.h"
#include "W3DViewDoc.H"
#include "Utils.H"


#ifdef RTS_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSaveSettingsDialog dialog


///////////////////////////////////////////////////////////////
//
//  CSaveSettingsDialog
//
CSaveSettingsDialog::CSaveSettingsDialog (CWnd* pParent /*=NULL*/)
	: CDialog(CSaveSettingsDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSaveSettingsDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
    return ;
}

///////////////////////////////////////////////////////////////
//
//  DoDataExchange
//
void
CSaveSettingsDialog::DoDataExchange (CDataExchange* pDX)
{
	// Allow the base class to process this message
    CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSaveSettingsDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
    return ;
}


BEGIN_MESSAGE_MAP(CSaveSettingsDialog, CDialog)
	//{{AFX_MSG_MAP(CSaveSettingsDialog)
	ON_BN_CLICKED(IDC_BROWSE_BUTTON, OnBrowseButton)
	ON_EN_UPDATE(IDC_FILENAME_EDIT, OnUpdateFilenameEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////
//
//  OnInitDialog
//
BOOL
CSaveSettingsDialog::OnInitDialog (void)
{
	// Allow the base class to process this message
    CDialog::OnInitDialog ();

    // Check everything by default
    SendDlgItemMessage (IDC_LIGHTING_CHECKBOX, BM_SETCHECK, (WPARAM)TRUE);
    SendDlgItemMessage (IDC_BACKGROUND_CHECKBOX, BM_SETCHECK, (WPARAM)TRUE);
    SendDlgItemMessage (IDC_CAMERA_CHECKBOX, BM_SETCHECK, (WPARAM)TRUE);
	
	// Put the default filename into the edit control
    SetDlgItemText (IDC_FILENAME_EDIT, "Default.dat");
    return TRUE;
}

///////////////////////////////////////////////////////////////
//
//  OnBrowseButton
//
void
CSaveSettingsDialog::OnBrowseButton (void) 
{
	 TCHAR szFileName[MAX_PATH];
	 ::GetModuleFileName (NULL, szFileName, sizeof (szFileName));
	 LPTSTR pszPath = ::strrchr (szFileName, '\\');
	 if (pszPath)
	 {
			::SetCurrentDirectory (pszPath);
		  pszPath[0] = 0;
	 }	 


    // Get the current filename
    CString stringCurrentFile;
    GetDlgItemText (IDC_FILENAME_EDIT, stringCurrentFile);
    
    CFileDialog saveFileDialog (FALSE,
                                ".dat",
                                stringCurrentFile,
                                OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
                                "Setting data files(*.dat)|*.dat||",
                                this);

    // Ask the user what filename to save under
    if (saveFileDialog.DoModal () == IDOK)
    {
        // Put the path into the filename edit control
        SetDlgItemText (IDC_FILENAME_EDIT, saveFileDialog.GetPathName ());
    }
        
    return ;
}

///////////////////////////////////////////////////////////////
//
//  OnUpdateFilenameEdit
//
void
CSaveSettingsDialog::OnUpdateFilenameEdit (void) 
{
    // Set the enabled state of the OK button
    // based on the values of the control
    FixOKEnableState ();
	return ;
}

///////////////////////////////////////////////////////////////
//
//  OnUpdateFilenameEdit
//
void
CSaveSettingsDialog::OnOK (void)
{
    // Assume we want to allow the base class to process this message
    BOOL bAllowDefaultProcessing = TRUE;

    // Get a pointer to the doc so we can get at the current scene
    // pointer.
    CW3DViewDoc *pCDoc = ::GetCurrentDocument ();
    if (pCDoc)
    {
        DWORD dwSettingsMask = 0L;

        // Did the user want to save lighting?
        if (SendDlgItemMessage (IDC_LIGHTING_CHECKBOX, BM_GETCHECK))
        {
            dwSettingsMask |= SAVE_SETTINGS_LIGHT;
        }
        
        // Did the user want to save the background?
        if (SendDlgItemMessage (IDC_BACKGROUND_CHECKBOX, BM_GETCHECK))
        {
            dwSettingsMask |= SAVE_SETTINGS_BACK;
        }

        // Did the user want to save camera settings?
        if (SendDlgItemMessage (IDC_CAMERA_CHECKBOX, BM_GETCHECK))
        {
            dwSettingsMask |= SAVE_SETTINGS_CAMERA;
        }

        // Get the current filename
        CString stringCurrentFile;
        GetDlgItemText (IDC_FILENAME_EDIT, stringCurrentFile);

        // Save the settings to the selected file
        bAllowDefaultProcessing = pCDoc->SaveSettings (stringCurrentFile,
                                                       dwSettingsMask);
    }        
    
	if (bAllowDefaultProcessing)
    {
        // Allow the base class to process this message
        CDialog::OnOK ();
    }

    return ;
}

///////////////////////////////////////////////////////////////
//
//  OnCommand
//
BOOL
CSaveSettingsDialog::OnCommand
(
    WPARAM wParam,
    LPARAM lParam
) 
{
    // Did the user check/uncheck one of the checkboxes?
    if ((HIWORD (wParam) == BN_CLICKED) &&
        ((LOWORD (wParam) == IDC_LIGHTING_CHECKBOX) ||
         (LOWORD (wParam) == IDC_BACKGROUND_CHECKBOX) ||
         (LOWORD (wParam) == IDC_CAMERA_CHECKBOX)))
    {
        // Set the enabled state of the OK button
        // based on the values of the control
        FixOKEnableState ();
    }

	// Allow the base class to process this message
    return CDialog::OnCommand (wParam, lParam);
}

///////////////////////////////////////////////////////////////
//
//  FixOKEnableState
//
void
CSaveSettingsDialog::FixOKEnableState (void)
{
    // Determine which (if any) checkboxes are checked
    int iValidSel = 0;
    iValidSel += SendDlgItemMessage (IDC_LIGHTING_CHECKBOX, BM_GETCHECK);
    iValidSel += SendDlgItemMessage (IDC_BACKGROUND_CHECKBOX, BM_GETCHECK);
    iValidSel += SendDlgItemMessage (IDC_CAMERA_CHECKBOX, BM_GETCHECK);

    // Is the dialog in a valid state?
    if ((iValidSel > 0) &&
        (::GetWindowTextLength (::GetDlgItem (m_hWnd, IDC_FILENAME_EDIT)) > 0))
    {
        // Enable the OK button
        ::EnableWindow (::GetDlgItem (m_hWnd, IDOK), TRUE);
    }
    else
    {
        // Disable the OK button
        ::EnableWindow (::GetDlgItem (m_hWnd, IDOK), FALSE);
    }
    
    return ;
}

