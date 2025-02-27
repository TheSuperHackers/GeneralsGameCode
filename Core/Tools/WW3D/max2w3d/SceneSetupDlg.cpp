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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : G                                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/SceneSetupDlg.cpp              $*
 *                                                                                             *
 *                      $Author:: Andre_a                                                     $*
 *                                                                                             *
 *                     $Modtime:: 10/15/99 3:37p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


// SceneSetupDlg.cpp : implementation file
//

#include "SceneSetupDlg.h"
#include <max.h>
#include <assert.h>

static BOOL CALLBACK _thunk_dialog_proc (HWND hWnd, UINT uMsg, WPARAM wAparam, LPARAM lParam);



/////////////////////////////////////////////////////////////////////////////
// SceneSetupDlg dialog


SceneSetupDlg::SceneSetupDlg(Interface *max_interface)
{
	m_DamageCount = 0;
	m_DamageOffset = -100.0f;
	m_LodCount = 0;
	m_LodOffset = -100.0f;
	m_DamageProc = 3;
	m_LodProc = 3;
	m_hWnd = NULL;
	m_MaxInterface = max_interface;
	assert(max_interface != NULL);
}


/////////////////////////////////////////////////////////////////////////////
// SceneSetupDlg Protected Methods

void SceneSetupDlg::SetEditInt (int control_id, int value)
{
	char buf[64];
	sprintf(buf, "%d", value);
	HWND edit = GetDlgItem(m_hWnd, control_id);
	assert(edit != NULL);
	SetWindowText(edit, buf);
}

void SceneSetupDlg::SetEditFloat (int control_id, float value)
{
	char buf[64];
	sprintf(buf, "%.0f", value);
	HWND edit = GetDlgItem(m_hWnd, control_id);
	assert(edit != NULL);
	SetWindowText(edit, buf);
}

int SceneSetupDlg::GetEditInt (int control_id)
{
	char buf[64];
	HWND edit = GetDlgItem(m_hWnd, control_id);
	assert(edit != NULL);
	GetWindowText(edit, buf, sizeof(buf));
	int value = 0;
	sscanf(buf, "%d", &value);
	return value;
}

float SceneSetupDlg::GetEditFloat (int control_id)
{
	char buf[64];
	HWND edit = GetDlgItem(m_hWnd, control_id);
	assert(edit != NULL);
	GetWindowText(edit, buf, sizeof(buf));
	float value = 0;
	sscanf(buf, "%f", &value);
	return value;
}

bool SceneSetupDlg::ValidateEditFloat (int control_id)
{
	char buf[64];
	HWND edit = GetDlgItem(m_hWnd, control_id);
	assert(edit != NULL);
	GetWindowText(edit, buf, sizeof(buf));
	float value = 0;
	if (sscanf(buf, "%f", &value) == 1)
		return true;
	else
		return false;
}

/////////////////////////////////////////////////////////////////////////////
// SceneSetupDlg Public Methods

int SceneSetupDlg::DoModal (void)
{
	// Put up the dialog box.
	BOOL result = DialogBoxParam(AppInstance, MAKEINTRESOURCE(IDD_SCENE_SETUP),
							m_MaxInterface->GetMAXHWnd(), (DLGPROC)_thunk_dialog_proc,
							(LPARAM)this);

	// Return IDOK if the user accepted the new settings.
	return (result == 1) ? IDOK : IDCANCEL;
}

/////////////////////////////////////////////////////////////////////////////
// SceneSetupDlg DialogProc

BOOL CALLBACK _thunk_dialog_proc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static SceneSetupDlg *dialog = NULL;

	if (uMsg == WM_INITDIALOG)
	{
		dialog = (SceneSetupDlg*)lParam;
		dialog->m_hWnd = hWnd;
	}

	if (dialog)
		return dialog->DialogProc(hWnd, uMsg, wParam, lParam);
	else
		return 0;
}

BOOL CALLBACK SceneSetupDlg::DialogProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

			}
			return TRUE;

	}

	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// SceneSetupDlg message handlers

void SceneSetupDlg::OnInitDialog()
{
	CenterWindow(m_hWnd, m_MaxInterface->GetMAXHWnd());
	SetCursor(LoadCursor(NULL, IDC_ARROW));

	// Select the appropriate radio buttons.
	switch (m_LodProc)
	{
	case 1:
		CheckDlgButton(m_hWnd, IDC_LOD_AS_COPY, BST_CHECKED);
		break;
	case 2:
		CheckDlgButton(m_hWnd, IDC_LOD_AS_INSTANCE, BST_CHECKED);
		break;
	case 3:
		CheckDlgButton(m_hWnd, IDC_LOD_AS_REFERENCE, BST_CHECKED);
		break;
	}
	switch (m_DamageProc)
	{
	case 1:
		CheckDlgButton(m_hWnd, IDC_DAMAGE_AS_COPY, BST_CHECKED);
		break;
	case 2:
		CheckDlgButton(m_hWnd, IDC_DAMAGE_AS_INSTANCE, BST_CHECKED);
		break;
	case 3:
		CheckDlgButton(m_hWnd, IDC_DAMAGE_AS_REFERENCE, BST_CHECKED);
		break;
	}

	// Set the text for the edit boxes.
	SetEditInt(IDC_LOD_COUNT, m_LodCount);
	SetEditFloat(IDC_LOD_OFFSET, m_LodOffset);
	SetEditInt(IDC_DAMAGE_COUNT, m_DamageCount);
	SetEditFloat(IDC_DAMAGE_OFFSET, m_DamageOffset);
}

BOOL SceneSetupDlg::OnOK()
{
	if (!ValidateEditFloat(IDC_LOD_OFFSET))
	{
		MessageBox(m_hWnd, "You must enter a valid number for the LOD Offset.",
			"Not a Number", MB_OK);
		SetFocus(GetDlgItem(m_hWnd, IDC_LOD_OFFSET));
		return FALSE;
	}
	if (!ValidateEditFloat(IDC_DAMAGE_OFFSET))
	{
		MessageBox(m_hWnd, "You must enter a valid number for the Damage Offset.",
			"Not a Number", MB_OK);
		SetFocus(GetDlgItem(m_hWnd, IDC_DAMAGE_OFFSET));
		return FALSE;
	}

	// Get the clone procedure the user wants to use.
	if (IsDlgButtonChecked(m_hWnd, IDC_LOD_AS_COPY))
		m_LodProc = 1;
	else if (IsDlgButtonChecked(m_hWnd, IDC_LOD_AS_INSTANCE))
		m_LodProc = 2;
	else if (IsDlgButtonChecked(m_hWnd, IDC_LOD_AS_REFERENCE))
		m_LodProc = 3;
	if (IsDlgButtonChecked(m_hWnd, IDC_DAMAGE_AS_COPY))
		m_DamageProc = 1;
	else if (IsDlgButtonChecked(m_hWnd, IDC_DAMAGE_AS_INSTANCE))
		m_DamageProc = 2;
	else if (IsDlgButtonChecked(m_hWnd, IDC_DAMAGE_AS_REFERENCE))
		m_DamageProc = 3;

	// Get the other values the user selected.
	m_LodCount = GetEditInt(IDC_LOD_COUNT);
	m_LodOffset = GetEditFloat(IDC_LOD_OFFSET);
	m_DamageCount = GetEditInt(IDC_DAMAGE_COUNT);
	m_DamageOffset = GetEditFloat(IDC_DAMAGE_OFFSET);

	return TRUE;
}
