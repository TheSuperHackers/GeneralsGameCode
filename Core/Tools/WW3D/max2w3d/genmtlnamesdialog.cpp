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
 *                 Project Name : Max2W3D                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/genmtlnamesdialog.cpp          $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 10/10/00 11:12a                                             $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "genmtlnamesdialog.h"
#include "dllmain.h"
#include "resource.h"
#include "WW3D2/w3d_file.h"
#include "max_sdk.h"

static BOOL CALLBACK _gen_mtl_names_dialog_proc(HWND Hwnd,UINT message,WPARAM wParam,LPARAM lParam);


/**********************************************************************************************
**
** GenMtlNamesDialogClass Implementation
**
**********************************************************************************************/


GenMtlNamesDialogClass::GenMtlNamesDialogClass(Interface * maxinterface) :
	Hwnd(nullptr),
	Options(nullptr),
	MaxInterface(maxinterface),
	NameIndexSpin(nullptr)
{
}


GenMtlNamesDialogClass::~GenMtlNamesDialogClass()
{
	ReleaseISpinner(NameIndexSpin);
}


bool GenMtlNamesDialogClass::Get_Options(OptionsStruct * options)
{
	Options = options;

	// Put up the options dialog box.
	BOOL result = DialogBoxParam
						(
							AppInstance,
							MAKEINTRESOURCE (IDD_GENERATE_MTL_NAMES_DIALOG),
							MaxInterface->GetMAXHWnd(),
							(DLGPROC) _gen_mtl_names_dialog_proc,
							(LPARAM) this
						);

	if (result == TRUE) {
		return true;
	} else {
		return false;
	}
}


bool GenMtlNamesDialogClass::Ok_To_Exit()
{
	// just check that the user entered a name
	char buf[W3D_NAME_LEN];
	GetWindowText(GetDlgItem(Hwnd,IDC_BASE_NAME_EDIT),buf,sizeof(buf));

	if (strlen(buf) == 0) {
		MessageBox(Hwnd,"Please enter a root name to use.\n","Error",MB_OK);
		return false;
	} else {
		return true;
	}

	return true;
}


bool GenMtlNamesDialogClass::Dialog_Proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM)
{
	switch (message )	{

		case WM_INITDIALOG:

			NameIndexSpin = SetupIntSpinner
			(
				Hwnd,
				IDC_NAME_INDEX_SPIN,
				IDC_NAME_INDEX_EDIT,
				MIN_NAME_INDEX,MAX_NAME_INDEX,INITIAL_NAME_INDEX
			);

			// limit the edit box characters
			SendDlgItemMessage(Hwnd,IDC_BASE_NAME_EDIT,EM_LIMITTEXT,MAX_ROOT_NAME_LEN,0);

			// set initial name to root of the filename
			char buf[_MAX_FNAME];
			_splitpath(MaxInterface->GetCurFileName(),nullptr,nullptr,buf,nullptr);
			buf[MAX_ROOT_NAME_LEN+1] = 0;
			SetWindowText(GetDlgItem(Hwnd,IDC_BASE_NAME_EDIT),buf);

			// init radio buttons
			CheckDlgButton(Hwnd,IDC_AFFECT_ALL_RADIO,BST_UNCHECKED);
			CheckDlgButton(Hwnd,IDC_AFFECT_SELECTED_RADIO,BST_CHECKED);

			return 1;

		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
				case IDOK:
					if (Ok_To_Exit()) {
						// general options
						Options->OnlyAffectSelected = (IsDlgButtonChecked(Hwnd,IDC_AFFECT_SELECTED_RADIO) == BST_CHECKED);

						// naming options
						Options->NameIndex = NameIndexSpin->GetIVal();
						GetWindowText(GetDlgItem(Hwnd,IDC_BASE_NAME_EDIT),Options->RootName,sizeof(Options->RootName));

						EndDialog(Hwnd, 1);
					}
					break;

				case IDCANCEL:
					EndDialog(Hwnd, 0);
					break;

			}
			return 1;
	}
	return 0;
}


static BOOL CALLBACK _gen_mtl_names_dialog_proc(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam)
{
	static GenMtlNamesDialogClass * dialog = nullptr;

	if (message == WM_INITDIALOG) {
		dialog = (GenMtlNamesDialogClass *)lparam;
		dialog->Hwnd = hwnd;
	}

	if (dialog) {
		return dialog->Dialog_Proc(hwnd, message, wparam, lparam);
	} else {
		return FALSE;
	}
}

