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
 *                 Project Name : Max2W3d                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/animationcompressionsettings.cpp $*
 *                                                                                             *
 *              Original Author:: Patrick Smith																  *
 *                                                                                             *
 *                      $Author:: Patrick                                                     $*
 *                                                                                             *
 *                     $Modtime:: 10/30/00 1:57p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "animationcompressionsettings.h"
#include "dllmain.h"
#include "resource.h"
#include "w3dexp.h"


////////////////////////////////////////////////////////////////////////////////////////
//
//	AnimationCompressionSettingsDialogClass
//
////////////////////////////////////////////////////////////////////////////////////////
AnimationCompressionSettingsDialogClass::AnimationCompressionSettingsDialogClass (Interface *maxinterface, HWND parent_wnd) :
	MaxInterface (maxinterface),
	Options (NULL),
	Wnd (NULL),
	ParentWnd (parent_wnd)
{
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////
//
//	~AnimationCompressionSettingsDialogClass
//
////////////////////////////////////////////////////////////////////////////////////////
AnimationCompressionSettingsDialogClass::~AnimationCompressionSettingsDialogClass (void)
{
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////
//
//	Do_Modal
//
////////////////////////////////////////////////////////////////////////////////////////
int
AnimationCompressionSettingsDialogClass::Do_Modal (void)
{
	int retval = ::DialogBoxParam (AppInstance, MAKEINTRESOURCE (IDD_ANIMATION_COMPRESSION),
													ParentWnd, Real_Message_Proc, (LPARAM)this);
	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////
//
//	Real_Message_Proc
//
////////////////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK
AnimationCompressionSettingsDialogClass::Real_Message_Proc
(
	HWND		wnd,
	UINT		message,
	WPARAM	wparam,
	LPARAM	lparam
)
{
	AnimationCompressionSettingsDialogClass *dialog_obj = NULL;

	//
	//	Setup the framework we need so that the instance
	// can process the messages instead of this static callback.
	//
	if (message == WM_INITDIALOG) {
		dialog_obj			= (AnimationCompressionSettingsDialogClass *)lparam;
		dialog_obj->Wnd	= wnd;
		::SetProp (wnd, "DIALOG_OBJ", (HANDLE)dialog_obj);
	} else {
		dialog_obj = (AnimationCompressionSettingsDialogClass *)::GetProp (wnd, "DIALOG_OBJ");
	}

	//
	//	Allow the instance to handle the call
	//
	BOOL retval = FALSE;
	if (dialog_obj != NULL) {
		retval = dialog_obj->Message_Proc (message, wparam, lparam);
	}

	//
	//	Cleanup the framework
	//
	if (message == WM_DESTROY) {
		::RemoveProp (wnd, "DIALOG_OBJ");
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////
//
//	Message_Proc
//
////////////////////////////////////////////////////////////////////////////////////////
BOOL
AnimationCompressionSettingsDialogClass::Message_Proc
(
	UINT		message,
	WPARAM	wparam,
	LPARAM	lparam
)
{
	BOOL retval = FALSE;

	switch (message)
	{
		case WM_INITDIALOG:
		{
			//
			//	Center the dialog
			//
			RECT parent_rect = { 0 };
			RECT rect = { 0 };
			::GetWindowRect (ParentWnd, &parent_rect);
			::GetWindowRect (Wnd, &rect);
			int width	= parent_rect.right - parent_rect.left;
			int height	= parent_rect.bottom - parent_rect.top;
			::SetWindowPos (	Wnd, NULL,
									parent_rect.left + (width / 2) - ((rect.right - rect.left) / 2),
									parent_rect.top + (height / 2) - ((rect.bottom - rect.top) / 2),
									0, 0, SWP_NOZORDER | SWP_NOSIZE);
			
			//
			//	Initialize the dialog controls
			//
			Initialize_Controls ();
		}
		break;

		case WM_COMMAND:
		{
			switch (LOWORD (wparam))
			{
				case IDCANCEL:
					EndDialog (Wnd, IDCANCEL);
					break;

				case IDOK:
					Save_Settings ();
					EndDialog (Wnd, IDOK);
					break;
			}
		}
		break;
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////
//
//	Initialize_Controls
//
////////////////////////////////////////////////////////////////////////////////////////
void
AnimationCompressionSettingsDialogClass::Initialize_Controls (void)
{
	SetCheckBox (Wnd, IDC_REDUCE_ANIMATION_CHECK, Options->ReduceAnimation);
	char string[128] = { 0 };

	//
	//	Populate the reduction percent combo box
	//
	HWND percent_combo = ::GetDlgItem (Wnd, IDC_REDUCE_ANIMATION_COMBO);
	for (int index = 1; index < 100; index ++) {
		sprintf (string, "%d", index);
		ComboBox_AddString (percent_combo, string);
	}

	//
	//	Populate the animation type combo box
	//
	HWND flavor_combo = ::GetDlgItem (Wnd, IDC_COMPRESS_ANIMATION_FLAVOR_COMBO);
	ComboBox_AddString (flavor_combo, "TimeCoded");
	ComboBox_AddString (flavor_combo, "Adaptive Delta");
	
	//
	//	Bounds check the parameters
	//
	if ((Options->ReduceAnimationPercent < 1) || (Options->ReduceAnimationPercent > 99)) {
		Options->ReduceAnimationPercent = 50;

	}

	if ((Options->CompressAnimationFlavor < 0) || (Options->CompressAnimationFlavor >= ANIM_FLAVOR_VALID)) {
		Options->CompressAnimationFlavor = 0;
	}

	//
	//	Select the correct entries in the combo boxes
	//
	ComboBox_SetCurSel (percent_combo, Options->ReduceAnimationPercent - 1);
	ComboBox_SetCurSel (flavor_combo, Options->CompressAnimationFlavor);

	//
	//	Fill in the error fields
	//
	::sprintf (string, "%f", Options->CompressAnimationTranslationError);
	::SetDlgItemText (Wnd, IDC_MAX_TRANS_ERROR_EDIT, string);

	::sprintf (string, "%f", Options->CompressAnimationRotationError);
	::SetDlgItemText (Wnd, IDC_MAX_ROT_ERROR_EDIT, string);
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////
//
//	Save_Settings
//
////////////////////////////////////////////////////////////////////////////////////////
void
AnimationCompressionSettingsDialogClass::Save_Settings (void)
{
	//
	//	Read the compression type setting
	//
	int flavor = ComboBox_GetCurSel (::GetDlgItem (Wnd, IDC_COMPRESS_ANIMATION_FLAVOR_COMBO));
	Options->CompressAnimationFlavor = flavor;

	//
	//	Determine whether or not we want to force reduction
	//
	Options->ReduceAnimation = (IsDlgButtonChecked (Wnd, IDC_REDUCE_ANIMATION_CHECK) == 1);

	//
	//	Read the reduction percent setting
	//
	int reduce_percent = ComboBox_GetCurSel (::GetDlgItem (Wnd, IDC_REDUCE_ANIMATION_COMBO)) + 1;
	Options->ReduceAnimationPercent = reduce_percent;

	//
	//	Read the amount of compression error we'll allow in the translational component
	//
	char string[128];
	::GetDlgItemText (Wnd, IDC_MAX_TRANS_ERROR_EDIT, string, sizeof (string));
	float trans_error = ::atof (string);
	Options->CompressAnimationTranslationError = trans_error;

	//
	//	Read the amount of compression error we'll allow in the rotational component
	//
	::GetDlgItemText (Wnd, IDC_MAX_ROT_ERROR_EDIT, string, sizeof (string));
	float rot_error = ::atof (string);
	Options->CompressAnimationRotationError = rot_error;
	return ;
}
