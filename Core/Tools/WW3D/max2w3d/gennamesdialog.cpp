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
 *                     $Archive:: /Commando/Code/Tools/max2w3d/gennamesdialog.cpp             $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 12/06/00 3:24p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   GenNamesDialogClass::GenNamesDialogClass -- Constructor                                   *
 *   GenNamesDialogClass::~GenNamesDialogClass -- Destructor                                   *
 *   GenNamesDialogClass::Get_Options -- Get_Options                                           *
 *   GenNamesDialogClass::Ok_To_Exit -- verifies that the input is valid                       *
 *   GenNamesDialogClass::Toggle_Name_Assignment -- toggles the controls related to name assig *
 *   GenNamesDialogClass::Toggle_Collision_Bits_Assignment -- toggles the controls related to  *
 *   GenNamesDialogClass::Dialog_Proc -- windows dialog proc                                   *
 *   _gen_names_dialog_proc -- dialog proc that "thunks" to the class's dialog proc            *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "gennamesdialog.h"
#include "dllmain.h"
#include "resource.h"


static BOOL CALLBACK _gen_names_dialog_proc(HWND Hwnd,UINT message,WPARAM wParam,LPARAM lParam);


/**********************************************************************************************
**
** GenNamesDialogClass Implementation
**
**********************************************************************************************/


/***********************************************************************************************
 * GenNamesDialogClass::GenNamesDialogClass -- Constructor                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
GenNamesDialogClass::GenNamesDialogClass(Interface * maxinterface) :
	Hwnd(NULL),
	Options(NULL),
	MaxInterface(maxinterface),
	NameIndexSpin(NULL)
{
}


/***********************************************************************************************
 * GenNamesDialogClass::~GenNamesDialogClass -- Destructor                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/10/2000 gth : Created.                                                                 *
 *=============================================================================================*/
GenNamesDialogClass::~GenNamesDialogClass(void)
{
	ReleaseISpinner(NameIndexSpin);
}


/***********************************************************************************************
 * GenNamesDialogClass::Get_Options -- Get_Options                                             *
 *                                                                                             *
 *    presents the dialog to the user and records his selections into the passed-in options    *
 *    structure.                                                                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 * options - pointer to a OptionsStruct to store the users input in.                           *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 * true - user pressed ok and entered valid input                                              *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/10/2000 gth : Created.                                                                 *
 *=============================================================================================*/
bool GenNamesDialogClass::Get_Options(OptionsStruct * options)
{
	Options = options;

	// Put up the options dialog box.
	BOOL result = DialogBoxParam
						(
							AppInstance,
							MAKEINTRESOURCE (IDD_GENERATE_NAMES_DIALOG),
							MaxInterface->GetMAXHWnd(),
							(DLGPROC) _gen_names_dialog_proc,
							(LPARAM) this
						);

	if (result == TRUE) {
		return true;
	} else {
		return false;
	}
}


/***********************************************************************************************
 * GenNamesDialogClass::Ok_To_Exit -- verifies that the input is valid                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 * true - dialog contains valid input                                                          *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/10/2000 gth : Created.                                                                 *
 *=============================================================================================*/
bool GenNamesDialogClass::Ok_To_Exit(void)
{
	if (IsDlgButtonChecked(Hwnd,IDC_ASSIGN_NAMES_CHECK) == BST_CHECKED) {

		// just check that the user entered a name
		char buf[W3D_NAME_LEN];
		GetWindowText(GetDlgItem(Hwnd,IDC_BASE_NAME_EDIT),buf,sizeof(buf));

		if (strlen(buf) == 0) {
			MessageBox(Hwnd,"Please enter a root name to use.\n","Error",MB_OK);
			return false;
		} else {
			return true;
		}
	}

	return true;
}


/***********************************************************************************************
 * GenNamesDialogClass::Toggle_Name_Assignment -- toggles the controls related to name assignm *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/10/2000 gth : Created.                                                                 *
 *=============================================================================================*/
void GenNamesDialogClass::Toggle_Name_Assignment(void)
{
	bool onoff = (IsDlgButtonChecked(Hwnd,IDC_ASSIGN_NAMES_CHECK) == BST_CHECKED);
	EnableWindow(GetDlgItem(Hwnd,IDC_BASE_NAME_EDIT),onoff);
	EnableWindow(GetDlgItem(Hwnd,IDC_LOD_INDEX_EDIT),onoff);
	EnableWindow(GetDlgItem(Hwnd,IDC_LOD_INDEX_SPIN),onoff);
	EnableWindow(GetDlgItem(Hwnd,IDC_NAME_INDEX_EDIT),onoff);
	EnableWindow(GetDlgItem(Hwnd,IDC_NAME_INDEX_SPIN),onoff);
}


/***********************************************************************************************
 * GenNamesDialogClass::Toggle_Collision_Bits_Assignment -- toggles the controls related to co *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/10/2000 gth : Created.                                                                 *
 *=============================================================================================*/
void GenNamesDialogClass::Toggle_Collision_Bits_Assignment(void)
{
	bool onoff = (IsDlgButtonChecked(Hwnd,IDC_ASSIGN_COLLISION_BITS_CHECK) == BST_CHECKED);
	EnableWindow(GetDlgItem(Hwnd,IDC_COLLISION_PHYSICAL),onoff);
	EnableWindow(GetDlgItem(Hwnd,IDC_COLLISION_PROJECTILE),onoff);
	EnableWindow(GetDlgItem(Hwnd,IDC_COLLISION_VIS),onoff);
	EnableWindow(GetDlgItem(Hwnd,IDC_COLLISION_CAMERA),onoff);
	EnableWindow(GetDlgItem(Hwnd,IDC_COLLISION_VEHICLE),onoff);
}


/***********************************************************************************************
 * GenNamesDialogClass::Dialog_Proc -- windows dialog proc                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/10/2000 gth : Created.                                                                 *
 *=============================================================================================*/
bool GenNamesDialogClass::Dialog_Proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM)
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

			// limit the base name edit box to 10 characters
			SendDlgItemMessage(Hwnd,IDC_BASE_NAME_EDIT,EM_LIMITTEXT,MAX_ROOT_NAME_LEN,0);

			// limit the suffix and prefix edit boxes
			SendDlgItemMessage(Hwnd,IDC_PREFIX_EDIT,EM_LIMITTEXT,MAX_PREFIX_LEN,0);
			SendDlgItemMessage(Hwnd,IDC_SUFFIX_EDIT,EM_LIMITTEXT,MAX_SUFFIX_LEN,0);

			// set initial name to root of the filename
			char buf[_MAX_FNAME];
			_splitpath(MaxInterface->GetCurFileName(),NULL,NULL,buf,NULL);
			buf[MAX_ROOT_NAME_LEN+1] = 0;
			SetWindowText(GetDlgItem(Hwnd,IDC_BASE_NAME_EDIT),buf);

			// init radio buttons
			CheckDlgButton(Hwnd,IDC_AFFECT_ALL_RADIO,BST_UNCHECKED);
			CheckDlgButton(Hwnd,IDC_AFFECT_SELECTED_RADIO,BST_CHECKED);

			// init the check boxes
			CheckDlgButton(Hwnd,IDC_ASSIGN_NAMES_CHECK,BST_CHECKED);
			CheckDlgButton(Hwnd,IDC_ASSIGN_PREFIX_CHECK,BST_UNCHECKED);
			CheckDlgButton(Hwnd,IDC_ASSIGN_SUFFIX_CHECK,BST_UNCHECKED);
			CheckDlgButton(Hwnd,IDC_ASSIGN_COLLISION_BITS_CHECK,BST_CHECKED);
			CheckDlgButton(Hwnd,IDC_COLLISION_PHYSICAL,BST_CHECKED);
			CheckDlgButton(Hwnd,IDC_COLLISION_PROJECTILE,BST_CHECKED);
			CheckDlgButton(Hwnd,IDC_COLLISION_VIS,BST_CHECKED);
			CheckDlgButton(Hwnd,IDC_COLLISION_CAMERA,BST_CHECKED);
			CheckDlgButton(Hwnd,IDC_COLLISION_VEHICLE,BST_CHECKED);
			return 1;

		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
				case IDOK:
					if (Ok_To_Exit()) {
						// general options
						Options->AssignNames = (IsDlgButtonChecked(Hwnd,IDC_ASSIGN_NAMES_CHECK) == BST_CHECKED);
						Options->AssignPrefix = (IsDlgButtonChecked(Hwnd,IDC_ASSIGN_PREFIX_CHECK) == BST_CHECKED);
						Options->AssignSuffix = (IsDlgButtonChecked(Hwnd,IDC_ASSIGN_SUFFIX_CHECK) == BST_CHECKED);
						Options->AssignCollisionBits = (IsDlgButtonChecked(Hwnd,IDC_ASSIGN_COLLISION_BITS_CHECK) == BST_CHECKED);
						Options->OnlyAffectSelected = (IsDlgButtonChecked(Hwnd,IDC_AFFECT_SELECTED_RADIO) == BST_CHECKED);

						// naming options
						Options->NameIndex = NameIndexSpin->GetIVal();
						GetWindowText(GetDlgItem(Hwnd,IDC_BASE_NAME_EDIT),Options->RootName,sizeof(Options->RootName));
						GetWindowText(GetDlgItem(Hwnd,IDC_PREFIX_EDIT),Options->PrefixName,sizeof(Options->PrefixName));
						GetWindowText(GetDlgItem(Hwnd,IDC_SUFFIX_EDIT),Options->SuffixName,sizeof(Options->SuffixName));

						// collision options
						Options->PhysicalCollision = (IsDlgButtonChecked(Hwnd,IDC_COLLISION_PHYSICAL) == BST_CHECKED);
						Options->ProjectileCollision = (IsDlgButtonChecked(Hwnd,IDC_COLLISION_PROJECTILE) == BST_CHECKED);
						Options->VisCollision = (IsDlgButtonChecked(Hwnd,IDC_COLLISION_VIS) == BST_CHECKED);
						Options->CameraCollision = (IsDlgButtonChecked(Hwnd,IDC_COLLISION_CAMERA) == BST_CHECKED);
						Options->VehicleCollision = (IsDlgButtonChecked(Hwnd,IDC_COLLISION_VEHICLE) == BST_CHECKED);
						EndDialog(Hwnd, 1);
					}
					break;

				case IDCANCEL:
					EndDialog(Hwnd, 0);
					break;

				case IDC_ASSIGN_NAMES_CHECK:
					Toggle_Name_Assignment();
					break;

				case IDC_ASSIGN_PREFIX_CHECK:
					EnableWindow(GetDlgItem(Hwnd,IDC_PREFIX_EDIT),(IsDlgButtonChecked(Hwnd,IDC_ASSIGN_SUFFIX_CHECK) == BST_CHECKED));
					break;

				case IDC_ASSIGN_SUFFIX_CHECK:
					EnableWindow(GetDlgItem(Hwnd,IDC_SUFFIX_EDIT),(IsDlgButtonChecked(Hwnd,IDC_ASSIGN_SUFFIX_CHECK) == BST_CHECKED));
					break;

				case IDC_ASSIGN_COLLISION_BITS_CHECK:
					Toggle_Collision_Bits_Assignment();
					break;
			}
			return 1;
	}
	return 0;
}


/***********************************************************************************************
 * _gen_names_dialog_proc -- dialog proc that "thunks" to the class's dialog proc              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/10/2000 gth : Created.                                                                 *
 *=============================================================================================*/
static BOOL CALLBACK _gen_names_dialog_proc(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam)
{
	static GenNamesDialogClass * dialog = NULL;

	if (message == WM_INITDIALOG) {
		dialog = (GenNamesDialogClass *)lparam;
		dialog->Hwnd = hwnd;
	}

	if (dialog) {
		return dialog->Dialog_Proc(hwnd, message, wparam, lparam);
	} else {
		return FALSE;
	}
}


