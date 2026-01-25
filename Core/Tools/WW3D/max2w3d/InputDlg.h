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
 *                     $Archive:: /Commando/Code/Tools/max2w3d/InputDlg.h                     $*
 *                                                                                             *
 *                      $Author:: Andre_a                                                     $*
 *                                                                                             *
 *                     $Modtime:: 5/08/00 1:36p                                               $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "dllmain.h"
#include "resource.h"


/////////////////////////////////////////////////////////////////////////////
// InputDlg dialog - a generic input box for MAXScript

class InputDlg
{
	friend BOOL CALLBACK _thunk_dialog_proc (HWND, UINT, WPARAM, LPARAM);

public:

	// Construction
	InputDlg (HWND hWndParent=nullptr);

	// Methods
	int DoModal (void);		// returns IDOK or IDCANCEL

	void SetCaption (const char *caption);
	void SetLabel (const char *label);
	void SetValue (const char *value);

	// DialogProc
	BOOL CALLBACK DialogProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// Dialog data associated with GUI components.
	char	m_Value[1024];		// edit box
	char	m_Label[512];		// description label
	char	m_Caption[128];	// dialog caption

protected:

	// Dialog data
	enum			{ IDD = IDD_INPUT_DIALOG };
	HWND			m_hWnd;
	HWND			m_hWndParent;

	// Message Handlers
	LRESULT OnInitDialog (WPARAM wParam, LPARAM lParam);
	BOOL OnOK (void);
};
