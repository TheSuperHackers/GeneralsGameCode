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
 *                     $Archive:: /Commando/Code/Tools/max2w3d/ExportAllDlg.h                 $*
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

#pragma once

#include "dllmain.h"
#include "resource.h"


class Interface;

/////////////////////////////////////////////////////////////////////////////
// ExportAllDlg dialog

class ExportAllDlg
{
public:

	// Construction
	ExportAllDlg (Interface *max_interface);

	// Methods
	int DoModal (void);

	// DialogProc
	BOOL CALLBACK DialogProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// Dialog data associated with GUI components.
	char	m_Directory[_MAX_PATH];		// edit box
	BOOL	m_Recursive;					// check box

	// Dialog data
	enum			{ IDD = IDD_EXPORT_ALL };
	HWND			m_hWnd;
	Interface	*m_MaxInterface;

protected:

	// Message Handlers
	void OnInitDialog (void);
	void OnBrowse (void);
	BOOL OnOK (void);		// TRUE if ok to close dialog
};
