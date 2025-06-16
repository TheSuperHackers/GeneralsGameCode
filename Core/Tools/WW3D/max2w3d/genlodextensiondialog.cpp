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
 *                     $Archive:: /Commando/Code/Tools/max2w3d/genlodextensiondialog.cpp      $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 10/10/00 11:14a                                             $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   GenLodExtensionDialogClass::GenLodExtensionDialogClass -- Constructor                     *
 *   GenLodExtensionDialogClass::~GenLodExtensionDialogClass -- Destructor                     *
 *   GenLodExtensionDialogClass::Get_Options -- Presents the dialog, gets user input           *
 *   GenLodExtensionDialogClass::Dialog_Proc -- Windows message handling                       *
 *   _gen_lod_ext_dialog_proc -- windows dialog proc                                           *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "genlodextensiondialog.h"
#include "dllmain.h"
#include "resource.h"
#include <max.h>


/**********************************************************************************************
**
** GenLodExtensionDialogClass Implementation
**
**********************************************************************************************/


/***********************************************************************************************
 * GenLodExtensionDialogClass::GenLodExtensionDialogClass -- Constructor                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
GenLodExtensionDialogClass::GenLodExtensionDialogClass(Interface * maxinterface) :
	Hwnd(NULL),
	Options(NULL),
	MaxInterface(maxinterface),
	LodIndexSpin(NULL)
{
}


/***********************************************************************************************
 * GenLodExtensionDialogClass::~GenLodExtensionDialogClass -- Destructor                       *
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
GenLodExtensionDialogClass::~GenLodExtensionDialogClass(void)
{
	ReleaseISpinner(LodIndexSpin);
}


/***********************************************************************************************
 * GenLodExtensionDialogClass::Get_Options -- Presents the dialog, gets user input             *
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
bool GenLodExtensionDialogClass::Get_Options(OptionsStruct * options)
{
	Options = options;

	// Put up the options dialog box.
	BOOL result = DialogBoxParam
						(
							AppInstance,
							MAKEINTRESOURCE (IDD_GENERATE_LOD_EXTENSION_DIALOG),
							MaxInterface->GetMAXHWnd(),
							(DLGPROC) _gen_lod_ext_dialog_proc,
							(LPARAM) this
						);

	if (result == TRUE) {
		return true;
	} else {
		return false;
	}
}


/***********************************************************************************************
 * GenLodExtensionDialogClass::Dialog_Proc -- Windows message handling                         *
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
bool GenLodExtensionDialogClass::Dialog_Proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM)
{
	switch (message )	{

		case WM_INITDIALOG:

			// Setup the LOD spinner control.
			LodIndexSpin = SetupIntSpinner
			(
				Hwnd,
				IDC_LOD_INDEX_SPIN,
				IDC_LOD_INDEX_EDIT,
				MIN_LOD_INDEX,MAX_LOD_INDEX,INITIAL_LOD_INDEX
			);
			
			return 1;

		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
				case IDOK:
					Options->LodIndex = LodIndexSpin->GetIVal();
					EndDialog(Hwnd, 1);
					break;

				case IDCANCEL:
					EndDialog(Hwnd, 0);
					break;
			}
			return 1;
	}
	return 0;
}


/***********************************************************************************************
 * _gen_lod_ext_dialog_proc -- windows dialog proc                                             *
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
static BOOL CALLBACK _gen_lod_ext_dialog_proc(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam)
{
	static GenLodExtensionDialogClass * dialog = NULL;

	if (message == WM_INITDIALOG) {
		dialog = (GenLodExtensionDialogClass *)lparam;
		dialog->Hwnd = hwnd;
	}

	if (dialog) {
		return dialog->Dialog_Proc(hwnd, message, wparam, lparam);
	} else {
		return FALSE;
	}
}




