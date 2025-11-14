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
 *                     $Archive:: /Commando/Code/Tools/max2w3d/presetexportoptionsdialog.cpp  $*
 *                                                                                             *
 *              Original Author:: Patrick Smith																  *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 11/10/00 2:26p                                              $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "presetexportoptionsdialog.h"
#include "dllmain.h"
#include "resource.h"
#include "w3dexp.h"
#include "animationcompressionsettings.h"


////////////////////////////////////////////////////////////////////////////////////////
//	Constants
////////////////////////////////////////////////////////////////////////////////////////

static const char *BROWSE_FILTER	= "W3D Files (*.W3D)\0*.W3D\0WHT Files (*.WHT)\0*.WHT\0\0";


////////////////////////////////////////////////////////////////////////////////////////
//
//	PresetExportOptionsDialogClass
//
////////////////////////////////////////////////////////////////////////////////////////
PresetExportOptionsDialogClass::PresetExportOptionsDialogClass (Interface *maxinterface, HWND parent_wnd) :
	MaxInterface (maxinterface),
	Options (NULL),
	Wnd (NULL),
	ParentWnd (parent_wnd),
	CurrentPane (-1)
{
	::memset (PaneWnds, 0, sizeof (PaneWnds));
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////
//
//	~PresetExportOptionsDialogClass
//
////////////////////////////////////////////////////////////////////////////////////////
PresetExportOptionsDialogClass::~PresetExportOptionsDialogClass (void)
{
	return ;
}


////////////////////////////////////////////////////////////////////////////////////////
//
//	Do_Modal
//
////////////////////////////////////////////////////////////////////////////////////////
int
PresetExportOptionsDialogClass::Do_Modal (void)
{
	int retval = ::DialogBoxParam (AppInstance, MAKEINTRESOURCE (IDD_W3D_PRESET_EXPORT_OPTIONS),
													ParentWnd, Real_Message_Proc, (LPARAM)this);
	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////
//
//	Real_Message_Proc
//
////////////////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK
PresetExportOptionsDialogClass::Real_Message_Proc
(
	HWND		wnd,
	UINT		message,
	WPARAM	wparam,
	LPARAM	lparam
)
{
	PresetExportOptionsDialogClass *dialog_obj = NULL;

	//
	//	Setup the framework we need so that the instance
	// can process the messages instead of this static callback.
	//
	if (message == WM_INITDIALOG) {
		dialog_obj			= (PresetExportOptionsDialogClass *)lparam;
		dialog_obj->Wnd	= wnd;
		::SetProp (wnd, "DIALOG_OBJ", (HANDLE)dialog_obj);
	} else {
		dialog_obj = (PresetExportOptionsDialogClass *)::GetProp (wnd, "DIALOG_OBJ");
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
//	Settings_Pane_Message_Proc
//
////////////////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK
PresetExportOptionsDialogClass::Settings_Pane_Message_Proc
(
	HWND		wnd,
	UINT		message,
	WPARAM	wparam,
	LPARAM	lparam
)
{
	PresetExportOptionsDialogClass *dialog_obj = NULL;

	//
	//	Setup the framework we need so that the instance
	// can process the messages instead of this static callback.
	//
	if (message == WM_INITDIALOG) {
		dialog_obj = (PresetExportOptionsDialogClass *)lparam;
		::SetProp (wnd, "DIALOG_OBJ", (HANDLE)dialog_obj);
	} else {
		dialog_obj = (PresetExportOptionsDialogClass *)::GetProp (wnd, "DIALOG_OBJ");
	}

	//
	//	Allow the instance to handle the call
	//
	BOOL retval = FALSE;
	if (dialog_obj != NULL) {
		retval = dialog_obj->Pane_Message_Proc (message, wparam, lparam);
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
//	Pane_Message_Proc
//
////////////////////////////////////////////////////////////////////////////////////////
BOOL
PresetExportOptionsDialogClass::Pane_Message_Proc
(
	UINT		message,
	WPARAM	wparam,
	LPARAM	lparam
)
{
	BOOL retval = FALSE;

	switch (message)
	{
		case WM_CUSTEDIT_ENTER:

			switch (wparam)
			{
				case IDC_RANGE_LOW_EDIT:
				{
					//
					//	Update the start frame
					//
					ICustEdit *edit_ctrl	= GetICustEdit ((HWND)lparam);
					if (edit_ctrl != NULL) {
						Options->StartFrame = edit_ctrl->GetInt ();

						//
						//	Bounds check the value
						//
						if (Options->StartFrame > Options->EndFrame) {
							Options->StartFrame = Options->EndFrame;
						}

						Update_Controls ();
					}
				}
				break;

				case IDC_RANGE_HIGH_EDIT:
				{
					//
					//	Update the end frame
					//
					ICustEdit *edit_ctrl	= GetICustEdit ((HWND)lparam);
					if (edit_ctrl != NULL) {
						Options->EndFrame = edit_ctrl->GetInt ();

						//
						//	Bounds check the value
						//
						if (Options->EndFrame < Options->StartFrame) {
							Options->EndFrame = Options->StartFrame;
						}

						Update_Controls ();
					}
				}
				break;
			}

			break;

		case CC_SPINNER_BUTTONUP:
		{
			ISpinnerControl *spin_ctrl = (ISpinnerControl *)lparam;
			if (spin_ctrl != NULL) {

				switch (LOWORD (wparam))
				{
					//
					//	Update the start frame
					//
					case IDC_RANGE_LOW_SPIN:
						Options->StartFrame = spin_ctrl->GetIVal ();

						//
						//	Bounds check the value
						//
						if (Options->StartFrame > Options->EndFrame) {
							Options->StartFrame = Options->EndFrame;
						}
						Update_Controls ();
						break;

					//
					//	Update the end frame
					//
					case IDC_RANGE_HIGH_SPIN:
						Options->EndFrame = spin_ctrl->GetIVal ();

						//
						//	Bounds check the value
						//
						if (Options->EndFrame < Options->StartFrame) {
							Options->EndFrame = Options->StartFrame;
						}

						Update_Controls ();
						break;
				}
			}
		}
		break;

		case WM_COMMAND:
		{
			HWND control_wnd		= reinterpret_cast <HWND> (lparam);
			bool update_controls	= true;

			switch (LOWORD (wparam))
			{
				case IDC_EXPORT_MESH_SMOOTH_CHECK:
					Options->SmoothBetweenMeshes = (SendMessage (control_wnd, BM_GETCHECK, 0, 0L) == 1);
					break;

				case IDC_EXPORT_MESH_AABTREES:
					Options->DisableExportAABTrees = (SendMessage (control_wnd, BM_GETCHECK, 0, 0L) != 1);
					break;

#if 0
				case IDC_EXPORT_MESH_OPTIMIZE:
					Options->EnableOptimizeMeshData = (SendMessage (control_wnd, BM_GETCHECK, 0, 0L) == 1);
					break;
#endif

				case IDC_USE_SKELETON_CHECK:
					Options->LoadHierarchy = (SendMessage (control_wnd, BM_GETCHECK, 0, 0L) == 1);
					break;

				case IDC_COMPRESS_ANIMATION_CHECK:
					Options->CompressAnimation = (SendMessage (control_wnd, BM_GETCHECK, 0, 0L) == 1);
					break;

				case IDC_WHT_BROWSE_BUTTON:
				{
					OPENFILENAME ofn		= { sizeof (OPENFILENAME), 0 };
					ofn.lpstrFilter		= BROWSE_FILTER;
					ofn.nMaxFile			= _MAX_PATH;
					ofn.nMaxFileTitle		= _MAX_FNAME + _MAX_EXT;
					ofn.Flags				= OFN_HIDEREADONLY | OFN_CREATEPROMPT;
					ofn.lpstrDefExt		= "wht";
					ofn.hwndOwner			= Wnd;
					ofn.lpstrFile			= Options->HierarchyFilename;

					if (::GetOpenFileName (&ofn)) {

						//
						// Get the relative path between the current export path
						// and the full file path to the hierarchy file:
						//
						Create_Relative_Path (Options->RelativeHierarchyFilename,
														W3dExportClass::CurrentExportPath,
														ofn.lpstrFile);
					}
				}
				break;

				case IDC_COMPRESSION_SETTINGS:
				{
					//
					//	Display the compression options dialog
					//
					AnimationCompressionSettingsDialogClass dialog (MaxInterface, Wnd);
					dialog.Set_Options (Options);
					dialog.Do_Modal ();
				}
				break;

				case IDC_EXPORT_MESH_MAT_TO_TEXTURE:
					Options->EnableMaterialColorToTextureConversion = (SendMessage (control_wnd, BM_GETCHECK, 0, 0L) == 1);
					break;

				default:
					update_controls = false;
					break;
			}

			if (update_controls) {
				Update_Controls ();
			}
		}
		break;
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////
//
//	Message_Proc
//
////////////////////////////////////////////////////////////////////////////////////////
BOOL
PresetExportOptionsDialogClass::Message_Proc
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
			//	Initialize the controls
			//
			Create_Settings_Panes ();
			Initialize_Controls ();
			Update_Controls ();
			Determine_Preset_Type ();
			retval = TRUE;
		}
		break;

		case WM_COMMAND:
			retval = On_Command (wparam, lparam);
			break;
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////
//
//	On_Command
//
////////////////////////////////////////////////////////////////////////////////////////
BOOL
PresetExportOptionsDialogClass::On_Command (WPARAM wparam, LPARAM lparam)
{
	BOOL retval = FALSE;

	switch (LOWORD (wparam))
	{
		case IDC_HLOD_RADIO:
			Show_Settings_Pane (PANE_HLOD);
			break;

		case IDC_ANIM_HLOD_RADIO:
			Show_Settings_Pane (PANE_ANIM_HLOD);
			break;

		case IDC_ANIM_RADIO:
			Show_Settings_Pane (PANE_ANIM);
			break;

		case IDC_TERRAIN_RADIO:
			Show_Settings_Pane (PANE_TERRAIN);
			break;

		case IDC_SKELETON_RADIO:
			Show_Settings_Pane (PANE_SKELETON);
			break;

		case IDC_MESH_RADIO:
			Show_Settings_Pane (PANE_MESH);
			break;

		case IDCANCEL:
			::memcpy (Options, &OrigOptions, sizeof (OrigOptions));
			EndDialog (Wnd, IDCANCEL);
			break;

		case IDOK:
			Save_Settings ();
			EndDialog (Wnd, IDOK);
			break;
	}

	return retval;
}


////////////////////////////////////////////////////////////////////////////////////////
//
//	Show_Settings_Pane
//
////////////////////////////////////////////////////////////////////////////////////////
void
PresetExportOptionsDialogClass::Show_Settings_Pane (int pane_id)
{
	if (pane_id != CurrentPane) {

		//
		//	Show the new pane and hide the old pane
		//
		::ShowWindow (PaneWnds[pane_id], SW_SHOW);
		if (CurrentPane >= 0) {
			::ShowWindow (PaneWnds[CurrentPane], SW_HIDE);
		}
		CurrentPane = pane_id;
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////
//
//	Create_Settings_Panes
//
////////////////////////////////////////////////////////////////////////////////////////
void
PresetExportOptionsDialogClass::Create_Settings_Panes (void)
{
	PaneWnds[PANE_HLOD] = ::CreateDialogParam (AppInstance, MAKEINTRESOURCE (IDD_EXPORT_PANE_HLOD),
										Wnd, Settings_Pane_Message_Proc, (LPARAM)this);

	PaneWnds[PANE_ANIM_HLOD] = ::CreateDialogParam (AppInstance, MAKEINTRESOURCE (IDD_EXPORT_PANE_ANIMATED_HLOD),
																		Wnd, Settings_Pane_Message_Proc, (LPARAM)this);

	PaneWnds[PANE_ANIM] = ::CreateDialogParam (AppInstance, MAKEINTRESOURCE (IDD_EXPORT_PANE_ANIMATION),
										Wnd, Settings_Pane_Message_Proc, (LPARAM)this);

	PaneWnds[PANE_TERRAIN] = ::CreateDialogParam (AppInstance, MAKEINTRESOURCE (IDD_EXPORT_PANE_TERRAIN),
																	Wnd, Settings_Pane_Message_Proc, (LPARAM)this);

	PaneWnds[PANE_SKELETON] = ::CreateDialogParam (AppInstance, MAKEINTRESOURCE (IDD_EXPORT_PANE_SKELETON),
																		Wnd, Settings_Pane_Message_Proc, (LPARAM)this);

	PaneWnds[PANE_MESH] = ::CreateDialogParam (AppInstance, MAKEINTRESOURCE (IDD_EXPORT_PANE_MESH),
																Wnd, Settings_Pane_Message_Proc, (LPARAM)this);


	//
	//	Get the position and size of the group box the settings panes will be
	// displayed inside
	//
	RECT group_rect = { 0 };
	::GetWindowRect (::GetDlgItem (Wnd, IDC_GROUP_BOX), &group_rect);
	::ScreenToClient (Wnd, (LPPOINT)&group_rect);
	::ScreenToClient (Wnd, ((LPPOINT)&group_rect) + 1);
	int width	= group_rect.right - group_rect.left;
	int height	= group_rect.bottom - group_rect.top;

	//
	//	Loop over all the panes and make sure they are in the proper location
	//
	for (int index = 0; index < PANE_MAX; index ++) {
		HWND pane_wnd = PaneWnds[index];

		//
		//	Get the size of this pane
		//
		RECT rect = { 0 };
		::GetWindowRect (pane_wnd, &rect);

		//
		//	Center the pane inside of the group box
		//
		::SetWindowPos (	pane_wnd, ::GetDlgItem (Wnd, IDC_GROUP_BOX),
								group_rect.left + (width / 2) - ((rect.right - rect.left) / 2),
								group_rect.top + (height / 2) - ((rect.bottom - rect.top) / 2),
								0, 0, SWP_NOSIZE);
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////
//
//	Destroy_Settings_Panes
//
////////////////////////////////////////////////////////////////////////////////////////
void
PresetExportOptionsDialogClass::Destroy_Settings_Panes (void)
{
	//
	//	Loop over all the panes and destroy them
	//
	for (int index = 0; index < PANE_MAX; index ++) {
		::DestroyWindow (PaneWnds[index]);
		PaneWnds[index] = NULL;
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////
//
//	Determine_Preset_Type
//
////////////////////////////////////////////////////////////////////////////////////////
void
PresetExportOptionsDialogClass::Determine_Preset_Type (void)
{
	//
	//	Examine the current options and try to determine which
	// preset best matches this configuration.
	//

	if (Options->EnableTerrainMode) {

		//
		//	Select the terrain UI
		//
		Show_Settings_Pane (PANE_TERRAIN);
		SendDlgItemMessage (Wnd, IDC_TERRAIN_RADIO, BM_SETCHECK, (WPARAM)TRUE, 0L);

	} else if (Options->ExportGeometry == false) {

		if (Options->ExportAnimation == false) {

			//
			//	Select the skeleton UI
			//
			Show_Settings_Pane (PANE_SKELETON);
			SendDlgItemMessage (Wnd, IDC_SKELETON_RADIO, BM_SETCHECK, (WPARAM)TRUE, 0L);
		} else {

			//
			//	Select the anim UI
			//
			Show_Settings_Pane (PANE_ANIM);
			SendDlgItemMessage (Wnd, IDC_ANIM_RADIO, BM_SETCHECK, (WPARAM)TRUE, 0L);
		}

	} else if (Options->ExportHierarchy == false && Options->LoadHierarchy == false) {

		//
		//	Select the mesh UI
		//
		Show_Settings_Pane (PANE_MESH);
		SendDlgItemMessage (Wnd, IDC_MESH_RADIO, BM_SETCHECK, (WPARAM)TRUE, 0L);

	} else if (Options->ExportAnimation == false) {

		//
		//	Select the HLOD UI
		//
		Show_Settings_Pane (PANE_HLOD);
		SendDlgItemMessage (Wnd, IDC_HLOD_RADIO, BM_SETCHECK, (WPARAM)TRUE, 0L);

	} else {

		//
		//	Select the HLOD anim UI
		//
		Show_Settings_Pane (PANE_ANIM_HLOD);
		SendDlgItemMessage (Wnd, IDC_ANIM_HLOD_RADIO, BM_SETCHECK, (WPARAM)TRUE, 0L);
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////
//
//	Initialize_Controls
//
////////////////////////////////////////////////////////////////////////////////////////
void
PresetExportOptionsDialogClass::Initialize_Controls (void)
{
	//
	//	Check the review log file button if necessary
	//
	CheckDlgButton (Wnd, IDC_REVIEW_LOG, Options->ReviewLog);

	//
	// Precompute some of the animation data
	//
	int ticksperframe	= ::GetTicksPerFrame();
	int startframe		= MaxInterface->GetAnimRange ().Start () / ticksperframe;
	int endframe		= MaxInterface->GetAnimRange ().End () / ticksperframe;

	//
	// Clamp the real options the same way the displayed values are clamped
	//
	if (startframe > Options->StartFrame) {
		Options->StartFrame = startframe;
	}
	if (endframe < Options->EndFrame) {
		Options->EndFrame = endframe;
	}

	//
	//	Loop over all the panes and update any of the controls therein
	//
	for (int index = 0; index < PANE_MAX; index ++) {
		HWND pane_wnd = PaneWnds[index];

		//
		//	Are there any animation controls on this pane to initialize?
		//
		if (::GetDlgItem (pane_wnd, IDC_RANGE_LOW_SPIN) != NULL) {

			ISpinnerControl *low_spin	= NULL;
			ISpinnerControl *high_spin	= NULL;

			low_spin = ::SetupIntSpinner (pane_wnd, IDC_RANGE_LOW_SPIN, IDC_RANGE_LOW_EDIT,
															startframe, endframe, 0);

			high_spin = ::SetupIntSpinner (pane_wnd, IDC_RANGE_HIGH_SPIN, IDC_RANGE_HIGH_EDIT,
															startframe, endframe, 0);

			::SetProp (::GetDlgItem (pane_wnd, IDC_RANGE_LOW_SPIN), "ISpinnerControl", (HANDLE)low_spin);
			::SetProp (::GetDlgItem (pane_wnd, IDC_RANGE_HIGH_SPIN), "ISpinnerControl", (HANDLE)high_spin);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////
//
//	Update_Controls
//
////////////////////////////////////////////////////////////////////////////////////////
void
PresetExportOptionsDialogClass::Update_Controls (void)
{
	//
	//	Loop over all the panes and update any of the controls therein
	//
	for (int index = 0; index < PANE_MAX; index ++) {
		HWND pane_wnd = PaneWnds[index];

		//
		//	Handle the check boxes
		//
		CheckDlgButton (pane_wnd, IDC_EXPORT_MESH_SMOOTH_CHECK,	Options->SmoothBetweenMeshes);
		CheckDlgButton (pane_wnd, IDC_EXPORT_MESH_AABTREES,		!Options->DisableExportAABTrees);
#if ENABLE_MESH_OPTIMIZING
		CheckDlgButton (pane_wnd, IDC_EXPORT_MESH_OPTIMIZE,		Options->EnableOptimizeMeshData);
#endif
		CheckDlgButton (pane_wnd, IDC_USE_SKELETON_CHECK,			Options->LoadHierarchy);
		CheckDlgButton (pane_wnd, IDC_COMPRESS_ANIMATION_CHECK,	Options->CompressAnimation);
		CheckDlgButton (pane_wnd, IDC_EXPORT_MESH_MAT_TO_TEXTURE,	Options->EnableMaterialColorToTextureConversion);

		//
		//	Enable/disable the compression settings button
		//
		HWND compress_settings_btn = ::GetDlgItem (pane_wnd, IDC_COMPRESSION_SETTINGS);
		if (compress_settings_btn != NULL) {
			::EnableWindow (compress_settings_btn, Options->CompressAnimation);
		}

		//
		//	Setup the skeleton browse button
		//
		HWND skeleten_browse_btn = ::GetDlgItem (pane_wnd, IDC_WHT_BROWSE_BUTTON);
		if (skeleten_browse_btn != NULL) {

			//
			// Honor the relative path if it is present
			//
			if (Options->RelativeHierarchyFilename[0] != 0) {
				SetWindowText (skeleten_browse_btn, Options->RelativeHierarchyFilename);
				::Create_Full_Path (Options->HierarchyFilename,
											W3dExportClass::CurrentExportPath,
											Options->RelativeHierarchyFilename);

			} else if (Options->HierarchyFilename[0] != 0) {
				SetWindowText (skeleten_browse_btn, Options->HierarchyFilename);
				::Create_Relative_Path (Options->RelativeHierarchyFilename,
													W3dExportClass::CurrentExportPath,
													Options->HierarchyFilename);
			}

			//
			//	Set the enable state of the window
			//
			::EnableWindow (skeleten_browse_btn, Options->LoadHierarchy);
		}

		//
		//	Are there any animation controls on this pane to update?
		//
		HWND low_spin_wnd		= ::GetDlgItem (pane_wnd, IDC_RANGE_LOW_SPIN);
		HWND high_spin_wnd	= ::GetDlgItem (pane_wnd, IDC_RANGE_HIGH_SPIN);
		if (low_spin_wnd != NULL && high_spin_wnd != NULL) {

			//
			//	Peek at the spinner control objects
			//
			ISpinnerControl *low_spin	= NULL;
			ISpinnerControl *high_spin	= NULL;
			low_spin		= (ISpinnerControl *)::GetProp (low_spin_wnd, "ISpinnerControl");
			high_spin	= (ISpinnerControl *)::GetProp (high_spin_wnd, "ISpinnerControl");

			//
			//	Update the spin controls
			//
			low_spin->SetValue (Options->StartFrame, FALSE);
			high_spin->SetValue (Options->EndFrame, FALSE);
		}
	}

	return ;
}


////////////////////////////////////////////////////////////////////////////////////////
//
//	Save_Settings
//
////////////////////////////////////////////////////////////////////////////////////////
void
PresetExportOptionsDialogClass::Save_Settings (void)
{
	//
	//	Force settings that certain preset types need
	//
	if (::IsDlgButtonChecked (Wnd, IDC_TERRAIN_RADIO)) {

		//
		//	Force some settings for the "Renegade Terrain" preset
		//
		Options->ExportHierarchy			= true;
		Options->LoadHierarchy				= false;
		Options->ExportAnimation			= false;
		Options->ExportGeometry				= true;
		Options->CompressAnimation			= false;
		Options->ReduceAnimation			= false;
		Options->EnableTerrainMode			= true;
		Options->DisableExportAABTrees	= false;
		Options->EnableMaterialColorToTextureConversion = false;
	} else if (::IsDlgButtonChecked (Wnd, IDC_SKELETON_RADIO)) {

		//
		//	Force some settings for the "Skeleton" preset
		//
		Options->ExportHierarchy			= true;
		Options->LoadHierarchy				= false;
		Options->ExportAnimation			= false;
		Options->ExportGeometry				= false;
		Options->CompressAnimation			= false;
		Options->ReduceAnimation			= false;
		Options->EnableTerrainMode			= false;
		Options->EnableOptimizeMeshData	= false;
		Options->DisableExportAABTrees	= true;
		Options->SmoothBetweenMeshes		= false;
		Options->EnableTerrainMode			= false;
		Options->EnableMaterialColorToTextureConversion = false;
	} else if (::IsDlgButtonChecked (Wnd, IDC_MESH_RADIO)) {

		//
		//	Force some settings for the "Simple Mesh" preset
		//
		Options->ExportHierarchy			= false;
		Options->LoadHierarchy				= false;
		Options->ExportAnimation			= false;
		Options->ExportGeometry				= true;
		Options->CompressAnimation			= false;
		Options->ReduceAnimation			= false;
		Options->EnableTerrainMode			= false;
		Options->EnableOptimizeMeshData	= false;
		Options->SmoothBetweenMeshes		= false;
		Options->EnableTerrainMode			= false;
	} else if (::IsDlgButtonChecked (Wnd, IDC_HLOD_RADIO)) {

		//
		//	Force some settings for the "Hierarchical Model" preset
		//
		Options->ExportHierarchy			= !Options->LoadHierarchy;
		Options->ExportAnimation			= false;
		Options->ExportGeometry				= true;
		Options->CompressAnimation			= false;
		Options->ReduceAnimation			= false;
		Options->EnableTerrainMode			= false;
	} else if (::IsDlgButtonChecked (Wnd, IDC_ANIM_HLOD_RADIO)) {

		//
		//	Force some settings for the "Hierarchical Animated Model" preset
		//
		Options->ExportHierarchy			= !Options->LoadHierarchy;
		Options->ExportAnimation			= true;
		Options->ExportGeometry				= true;
		Options->EnableTerrainMode			= false;
	} else if (::IsDlgButtonChecked (Wnd, IDC_ANIM_RADIO)) {

		//
		//	Force some settings for the "Pure Animation" preset
		//
		Options->ExportHierarchy			= !Options->LoadHierarchy;
		Options->ExportAnimation			= true;
		Options->ExportGeometry				= false;
		Options->EnableTerrainMode			= false;
		Options->EnableOptimizeMeshData	= false;
		Options->DisableExportAABTrees	= true;
		Options->SmoothBetweenMeshes		= false;
		Options->EnableMaterialColorToTextureConversion = false;
	}

	//
	//	Record the "Review Log File" option
	//
	Options->ReviewLog = (::IsDlgButtonChecked (Wnd, IDC_REVIEW_LOG) == 1);

	//
	//	Check to see if anything has changed
	//
	if (::memcmp (Options, &OrigOptions, sizeof (OrigOptions)) != 0) {
		SetSaveRequiredFlag (true);
	}

	return ;
}

