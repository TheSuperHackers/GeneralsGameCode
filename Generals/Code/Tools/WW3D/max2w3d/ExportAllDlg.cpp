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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : G                                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/ExportAllDlg.cpp               $*
 *                                                                                             *
 *                      $Author:: Andre_a                                                     $*
 *                                                                                             *
 *                     $Modtime:: 10/15/99 4:25p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */



// ExportAllDlg.cpp : implementation file
//

#include "ExportAllDlg.h"
#include <max.h>
#include <assert.h>
#include <shlobj.h>	// SHBrowseForFolder


static BOOL CALLBACK _thunk_dialog_proc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


/////////////////////////////////////////////////////////////////////////////
// ExportAllDlg dialog


ExportAllDlg::ExportAllDlg (Interface *max_interface)
{
	m_Directory[0] = '\0';
	m_Recursive = TRUE;
	m_hWnd = NULL;
	assert(max_interface != NULL);
	m_MaxInterface = max_interface;
}


/////////////////////////////////////////////////////////////////////////////
// ExportAllDlg Methods

int ExportAllDlg::DoModal (void)
{
	// Put up the dialog box.
	BOOL result = DialogBoxParam(AppInstance, MAKEINTRESOURCE(IDD_EXPORT_ALL),
							m_MaxInterface->GetMAXHWnd(), (DLGPROC)_thunk_dialog_proc,
							(LPARAM)this);

	// Return IDOK if the user accepted the new settings.
	return (result == 1) ? IDOK : IDCANCEL;
}

/////////////////////////////////////////////////////////////////////////////
// ExportAllDlg DialogProc

BOOL CALLBACK _thunk_dialog_proc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static ExportAllDlg *dialog = NULL;

	if (uMsg == WM_INITDIALOG)
	{
		dialog = (ExportAllDlg*)lParam;
		dialog->m_hWnd = hWnd;
	}

	if (dialog)
		return dialog->DialogProc(hWnd, uMsg, wParam, lParam);
	else
		return 0;
}

BOOL CALLBACK ExportAllDlg::DialogProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int code = HIWORD(wParam);

	switch (uMsg)
	{

		/*******************************************************************
		* WM_INITDIALOG
		*
		* Initialize all of the custom controls for the dialog box
		*
		*******************************************************************/
		case WM_INITDIALOG:

			OnInitDialog();
			return TRUE;


		/*******************************************************************
		* WM_COMMAND
		*
		*
		*******************************************************************/
		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
				case IDOK:

					if (OnOK() == FALSE)
						return TRUE;

					SetCursor(LoadCursor(NULL, IDC_WAIT));
					EndDialog(m_hWnd, 1);
					break;

				case IDCANCEL:
					EndDialog(m_hWnd, 0);
					break;

				case IDC_BROWSE:
					OnBrowse();
					return FALSE;

			}
			return TRUE;

	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// ExportAllDlg message handlers

void ExportAllDlg::OnInitDialog (void)
{
	CenterWindow(m_hWnd, m_MaxInterface->GetMAXHWnd());
	SetCursor(LoadCursor(NULL, IDC_ARROW));

	// Set the check box state.
	CheckDlgButton(m_hWnd, IDC_RECURSIVE, m_Recursive);

	// Set the default directory.
	HWND edit = GetDlgItem(m_hWnd, IDC_DIRECTORY);
	assert(edit != NULL);
	SetWindowText(edit, m_Directory);
}

void ExportAllDlg::OnBrowse() 
{
	char			folder_name[MAX_PATH];
	BROWSEINFO	bi;
	ZeroMemory(&bi, sizeof(bi));
	bi.hwndOwner = m_hWnd;
	bi.pszDisplayName = folder_name;
	bi.lpszTitle = "Select a folder for export...";
	bi.ulFlags = BIF_RETURNONLYFSDIRS;

	// Browse for a folder.
	LPITEMIDLIST il = SHBrowseForFolder(&bi);
	if (il)
	{
		// Get the path of the folder.
		if (SHGetPathFromIDList(il, folder_name))
		{
			HWND edit = GetDlgItem(m_hWnd, IDC_DIRECTORY);
			assert(edit != NULL);
			SetWindowText(edit, folder_name);
		}
		else
			MessageBox(m_hWnd, "Error getting pathname with SHGetPathFromIDList()",
				"Error", MB_OK | MB_ICONSTOP);
	}
}

BOOL ExportAllDlg::OnOK (void)
{
	// Get the directory chosen by the user. If none is entered,
	// freak on the user.
	char	dir[_MAX_PATH];
	HWND	edit = GetDlgItem(m_hWnd, IDC_DIRECTORY);
	assert(edit != NULL);
	if (GetWindowText(edit, dir, sizeof(dir)) == 0)
	{
		// The edit box is empty, that's not a valid choice.
		MessageBox(m_hWnd, "You must choose a directory to export",
			"Invalid Directory", MB_OK);
		SetFocus(edit);
		return FALSE;
	}

	// TODO: Validate the directory as one that actually exists.

	// Store the values from the dialog in our class members.
	strcpy(m_Directory, dir);
	m_Recursive = (IsDlgButtonChecked(m_hWnd, IDC_RECURSIVE) == BST_CHECKED) ? TRUE : FALSE;

	return TRUE;
}

