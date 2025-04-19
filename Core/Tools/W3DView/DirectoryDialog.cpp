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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : W3DView                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/W3DView/DirectoryDialog.cpp            $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/25/00 2:56p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "stdafx.h"
#include "directorydialog.h"
#include "utils.h"


////////////////////////////////////////////////////////////////////////////
//
//	Browse_For_Folder_Hook_Proc
//
////////////////////////////////////////////////////////////////////////////
UINT CALLBACK Browse_For_Folder_Hook_Proc
(
	HWND		hdlg,
	UINT		message,
	WPARAM	wparam,
	LPARAM	lparam
)
{
	//
	//	If the user clicked OK, then stuff a dummy filename
	// into the control so the dialog will close...
	//
	if (	message == WM_COMMAND &&
			LOWORD (wparam) == IDOK &&
			HIWORD (wparam) == BN_CLICKED)
	{
		::SetDlgItemText (hdlg, 1152, "xxx.xxx");
	}

	return FALSE;
}


////////////////////////////////////////////////////////////////////////////
//
//	Browse_For_Folder
//
////////////////////////////////////////////////////////////////////////////
bool
Browse_For_Folder (HWND parent_wnd, LPCTSTR initial_path, CString &path)
{
	bool retval = false;

	OPENFILENAME openfilename	= { sizeof (OPENFILENAME), 0 };
	TCHAR filename[MAX_PATH]	= { 0 };

	openfilename.lpstrInitialDir	= initial_path;
	openfilename.hwndOwner			= parent_wnd;
	openfilename.hInstance			= ::AfxGetResourceHandle ();
	openfilename.lpstrFilter		= _T("\0\0");
	openfilename.lpstrFile			= filename;
	openfilename.nMaxFile			= sizeof (filename);
	openfilename.lpstrTitle			= _T("Choose Directory");
	openfilename.lpfnHook			= Browse_For_Folder_Hook_Proc;
	openfilename.lpTemplateName	= MAKEINTRESOURCE (1536);
	openfilename.Flags				= OFN_HIDEREADONLY | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK | OFN_LONGNAMES;

	//
	//	Display the modified 'file-open' dialog.
	//
	if (::GetOpenFileName (&openfilename) == IDOK) {
		path		= ::Strip_Filename_From_Path (filename);
		retval	= true;
	}	

	return retval;
}
