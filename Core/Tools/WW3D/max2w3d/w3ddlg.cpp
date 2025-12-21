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

/* $Header: /Commando/Code/Tools/max2w3d/w3ddlg.cpp 24    11/07/00 5:40p Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando Tools - W3D export                                  *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/w3ddlg.cpp                     $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 11/07/00 4:39p                                              $*
 *                                                                                             *
 *                    $Revision:: 24                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "w3ddlg.h"
#include "resource.h"
#include "dllmain.h"
#include "w3dexp.h"
#include "util.h"
#include "rawfile.h"
#include <string>
#include "units.h"
#include "presetexportoptionsdialog.h"


#define ENABLE_MESH_OPTIMIZING 0

/*
** Static functions
*/
static BOOL CALLBACK		_options_dialog_proc(HWND Hwnd,UINT message,WPARAM wParam,LPARAM lParam);
static void					_init_ofn(void);

/*
** Static data
*/
static bool					_OfnInited = false;
static OPENFILENAME 		_HierarchyFileOFN;


/***********************************************************************************************
 * W3dOptionsDialogClass::W3dOptionsDialogClass -- constructor for the options dialog object   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/24/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
W3dOptionsDialogClass::W3dOptionsDialogClass(Interface * maxinterface,ExpInterface * exportinterface)
{
	MaxInterface = maxinterface;
	ExportInterface = exportinterface;
	if (!_OfnInited) _init_ofn();
	GotHierarchyFilename = false;

	RangeLowSpin = nullptr;
	RangeHighSpin = nullptr;

	GetMasterUnitInfo(&UnitsType, &UnitsScale);
}

W3dOptionsDialogClass::~W3dOptionsDialogClass(void)
{
	ReleaseISpinner(RangeLowSpin);
	ReleaseISpinner(RangeHighSpin);
}

bool W3dOptionsDialogClass::Get_Export_Options(W3dExportOptionsStruct * options)
{
	Options = options;

	// Put up the options dialog box.
	/*BOOL result = DialogBoxParam
						(
							AppInstance,
							MAKEINTRESOURCE (IDD_W3D_EXPORT_OPTIONS),
							MaxInterface->GetMAXHWnd(),
							(DLGPROC) _options_dialog_proc,
							(LPARAM) this
						);*/

	PresetExportOptionsDialogClass dialog (MaxInterface, MaxInterface->GetMAXHWnd ());

	dialog.Set_Options (Options);
	int result = dialog.Do_Modal ();

	if (result == IDOK) {
		return true;
	} else {
		return false;
	}
}


/***********************************************************************************************
 * W3dOptionsDialogClass::Dialog_Proc -- Handles the windows message for the options dialog    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/24/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
bool W3dOptionsDialogClass::Dialog_Proc
(
	HWND hwnd,
	UINT message,
	WPARAM wParam,
	LPARAM
)
{
	int code = HIWORD(wParam);

	switch (message )	{

		/*******************************************************************
		* WM_INITDIALOG
		*
		* Initialize all of the custom controls for the dialog box
		*
		*******************************************************************/
		case WM_INITDIALOG:

			Dialog_Init();
			return 1;


		/*******************************************************************
		* WM_COMMAND
		*
		*
		*******************************************************************/
		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
				case IDOK:

					if (Dialog_Ok() == FALSE) {
						MessageBox(Hwnd,"You have not supplied a Base Pose hierarchy file!","Error",MB_OK);
						return 1;
					}

					SetCursor(LoadCursor (nullptr, IDC_WAIT));
					EndDialog(Hwnd, 1);
					break;

				case IDCANCEL:
					EndDialog(Hwnd, 0);
					break;

				case IDC_WHT_EXPORT_RADIO:
				case IDC_WHT_LOAD_RADIO:
				case IDC_WHT_NO_EXPORT_RADIO:
					WHT_Export_Radio_Changed();
					break;

				case IDC_WHA_EXPORT_RADIO:
				case IDC_WHA_NO_EXPORT_RADIO:
					WHA_Export_Radio_Changed();
					break;

				case IDC_WTM_EXPORT_RADIO:
				case IDC_WTM_NO_EXPORT_RADIO:
					WTM_Export_Radio_Changed();
					break;

				case IDC_COMPRESS_ANIMATION_CHECK:
					WHA_Compress_Animation_Check_Changed();
					break;

				case IDC_REDUCE_ANIMATION_CHECK:
					WHA_Reduce_Animation_Check_Changed();
					break;

				case IDC_COMPRESS_ANIMATION_FLAVOR_COMBO:
					WHA_Compression_Flavor_Changed();
					break;

				case IDC_WHT_BROWSE_BUTTON:

					// use the open file common dialog to get a hierarchy filename.
					_HierarchyFileOFN.hwndOwner = Hwnd;
					_HierarchyFileOFN.lpstrFileTitle = nullptr;
					_HierarchyFileOFN.lpstrFile = Options->HierarchyFilename;

					if (GetOpenFileName(&_HierarchyFileOFN)) {

						// Get the relative path between the current export path
						// and the full file path to the hierarchy file:
						Create_Relative_Path(Options->RelativeHierarchyFilename,
							W3dExportClass::CurrentExportPath,
							_HierarchyFileOFN.lpstrFile);

						// set window text to the relative path.
						HWND butHwnd = GetDlgItem(hwnd,IDC_WHT_BROWSE_BUTTON);
						if (Options->RelativeHierarchyFilename[0] != 0) {
							SetWindowText(butHwnd, Options->RelativeHierarchyFilename);
						} else {
							SetWindowText(butHwnd, Options->HierarchyFilename);
						}
						GotHierarchyFilename = true;

						SetSaveRequiredFlag(true);
					}

					_HierarchyFileOFN.lpstrFile = nullptr;
					break;
			}
			return 1;

		/*******************************************************************
		* CC_SPINNER_CHANGE
		*
		* Max custom spinner controls
		*
		*******************************************************************/
		case CC_SPINNER_CHANGE:

			switch (LOWORD(wParam))
			{
				case IDC_RANGE_LOW_SPIN:
					if (RangeLowSpin->GetIVal() > RangeHighSpin->GetIVal())
					{
						RangeHighSpin->SetValue (RangeLowSpin->GetIVal(),FALSE);
					}
					break;

				case IDC_RANGE_HIGH_SPIN:
					if (RangeHighSpin->GetIVal() < RangeLowSpin->GetIVal())
					{
						RangeLowSpin->SetValue(RangeHighSpin->GetIVal(),FALSE);
					}
					break;

	 		}

	}
	return 0;
}

void W3dOptionsDialogClass::Dialog_Init()
{
	CenterWindow(Hwnd, GetParent(Hwnd));
	SetCursor(LoadCursor (nullptr, IDC_ARROW));

	// initialize the export radio buttons
	if (Options->ExportHierarchy) {
		CheckDlgButton(Hwnd,IDC_WHT_EXPORT_RADIO,BST_CHECKED);
		Enable_WHT_Export();
	} else {
		if (Options->LoadHierarchy) {
			CheckDlgButton(Hwnd,IDC_WHT_LOAD_RADIO,BST_CHECKED);
			Enable_WHT_Load();

			// If the relative path is a full path, just erase both paths
			// This case happens with files which were exported using a previous,
			// bugged version of the exporter which did not handle files on
			// different drives correctly.
			if (Is_Full_Path(Options->RelativeHierarchyFilename)) {
				Options->RelativeHierarchyFilename[0] = 0;
				Options->HierarchyFilename[0] = 0;
			}

			// Honor the relative path if it is present
			if (Options->RelativeHierarchyFilename[0] != 0)
			{

				HWND butHwnd = GetDlgItem(Hwnd,IDC_WHT_BROWSE_BUTTON);
				SetWindowText(butHwnd, Options->RelativeHierarchyFilename);
				Create_Full_Path(Options->HierarchyFilename,
					W3dExportClass::CurrentExportPath,
					Options->RelativeHierarchyFilename);
				GotHierarchyFilename = true;

			} else if (Options->HierarchyFilename[0] != 0) {

				HWND butHwnd = GetDlgItem(Hwnd,IDC_WHT_BROWSE_BUTTON);
				SetWindowText(butHwnd, Options->HierarchyFilename);
				GotHierarchyFilename = true;

			}

		} else {
			CheckDlgButton(Hwnd,IDC_WHT_NO_EXPORT_RADIO,BST_CHECKED);
			Disable_WHT_Export();
		}
	}

	if (Options->ExportGeometry) {
		CheckDlgButton(Hwnd,IDC_WTM_EXPORT_RADIO,BST_CHECKED);
		Enable_WTM_Export();
	} else {
		CheckDlgButton(Hwnd,IDC_WTM_NO_EXPORT_RADIO,BST_CHECKED);
		Disable_WTM_Export();
	}

	if (Options->SmoothBetweenMeshes && Options->ExportGeometry) {
		CheckDlgButton(Hwnd,IDC_EXPORT_MESH_SMOOTH_CHECK,BST_CHECKED);
	} else {
		CheckDlgButton(Hwnd,IDC_EXPORT_MESH_SMOOTH_CHECK,BST_UNCHECKED);
	}

	SetCheckBox(Hwnd, IDC_TRANSLATION_ONLY_CHECK, Options->TranslationOnly);
	SetCheckBox(Hwnd, IDC_TERRAIN_MODE_CHECK, Options->EnableTerrainMode);

	// Initialize additional Animation Options

	SetCheckBox(Hwnd, IDC_COMPRESS_ANIMATION_CHECK, Options->CompressAnimation);
	SetCheckBox(Hwnd, IDC_REDUCE_ANIMATION_CHECK	 , Options->ReduceAnimation);
	SetCheckBox(Hwnd, IDC_VIEWLOG_CHECK, Options->ReviewLog);

	char string[128];	// temp string buffer

	sprintf(string, "Current FPS:  %d", GetFrameRate());

	SetDlgItemText(Hwnd, IDC_ANIMATION_FPS_STATIC, string);


  // initialize animation combo/list boxes

	HwndReduce = GetDlgItem(Hwnd, IDC_REDUCE_ANIMATION_COMBO);
	HwndFlavor = GetDlgItem(Hwnd, IDC_COMPRESS_ANIMATION_FLAVOR_COMBO);

	ComboBox_ResetContent(HwndReduce);
	ComboBox_ResetContent(HwndFlavor);

	for (int i=1; i<100; i++) {
		sprintf(string, "%d", i);
		ComboBox_AddString(HwndReduce, string);
	}

	ComboBox_AddString(HwndFlavor, "TimeCoded");
	ComboBox_AddString(HwndFlavor, "Adaptive Delta");


	if ((Options->ReduceAnimationPercent < 1) || (Options->ReduceAnimationPercent > 99)) {
		Options->ReduceAnimationPercent = 50;

	}

	if ((Options->CompressAnimationFlavor < 0) || (Options->CompressAnimationFlavor >= ANIM_FLAVOR_VALID)) {
		Options->CompressAnimationFlavor = 0;
	}

	ComboBox_SetCurSel(HwndReduce, Options->ReduceAnimationPercent-1);
	ComboBox_SetCurSel(HwndFlavor, Options->CompressAnimationFlavor);


	HwndTError = GetDlgItem(Hwnd, IDC_MAX_TRANS_ERROR_EDIT);
	HwndRError = GetDlgItem(Hwnd, IDC_MAX_ROT_ERROR_EDIT);

	sprintf(string, "%f", Options->CompressAnimationTranslationError);
	Edit_SetText(HwndTError, string);

	sprintf(string, "%f", Options->CompressAnimationRotationError);
	Edit_SetText(HwndRError, string);


	// Make sure everything under animations is properly active/inactive

	WHA_Compress_Animation_Check_Changed();

	if (Options->ExportAnimation) {
		CheckDlgButton(Hwnd,IDC_WHA_EXPORT_RADIO,BST_CHECKED);
		Enable_WHA_Export();
	} else {
		CheckDlgButton(Hwnd,IDC_WHA_NO_EXPORT_RADIO,BST_CHECKED);
		Disable_WHA_Export();
	}


	// initialize the frame-range spinners
	int ticksperframe = GetTicksPerFrame();
	int startframe = MaxInterface->GetAnimRange().Start() / ticksperframe;
	int endframe = MaxInterface->GetAnimRange().End() / ticksperframe;

	RangeLowSpin = SetupIntSpinner
	(
		Hwnd,
		IDC_RANGE_LOW_SPIN,
		IDC_RANGE_LOW_EDIT,
		startframe,
		endframe,
		Options->StartFrame
	);

	RangeHighSpin = SetupIntSpinner
	(
		Hwnd,
		IDC_RANGE_HIGH_SPIN,
		IDC_RANGE_HIGH_EDIT,
		startframe,
		endframe,
		Options->EndFrame
	);

	// initialize the 'DisableExportAABTrees' option
	CheckDlgButton(Hwnd,IDC_EXPORT_MESH_AABTREES,!Options->DisableExportAABTrees);

	CheckDlgButton(Hwnd,IDC_EXPORT_MESH_MAT_TO_TEXTURE,Options->EnableMaterialColorToTextureConversion);

#if ENABLE_MESH_OPTIMIZING
	CheckDlgButton(Hwnd,IDC_EXPORT_MESH_OPTIMIZE,Options->EnableOptimizeMeshData);
#endif
}

BOOL W3dOptionsDialogClass::Dialog_Ok()
{
	bool changed = false;

	// export options:
	bool export_h = (IsDlgButtonChecked(Hwnd,IDC_WHT_EXPORT_RADIO) == BST_CHECKED);
	changed = changed || (Options->ExportHierarchy != export_h);
	Options->ExportHierarchy = export_h;

	bool load_h = (IsDlgButtonChecked(Hwnd,IDC_WHT_LOAD_RADIO) == BST_CHECKED);
	changed = changed || (Options->LoadHierarchy != load_h);
	Options->LoadHierarchy = load_h;

	bool export_a = (IsDlgButtonChecked(Hwnd,IDC_WHA_EXPORT_RADIO) == BST_CHECKED);
	changed = changed || (Options->ExportAnimation != export_a);
	Options->ExportAnimation = export_a;

	bool export_g = (IsDlgButtonChecked(Hwnd,IDC_WTM_EXPORT_RADIO) == BST_CHECKED);
	changed = changed || (Options->ExportGeometry != export_g);
	Options->ExportGeometry = export_g;

	if (export_g) {
		bool smooth_meshes = (IsDlgButtonChecked(Hwnd,IDC_EXPORT_MESH_SMOOTH_CHECK) == BST_CHECKED);
		changed = changed || (Options->SmoothBetweenMeshes != smooth_meshes);
		Options->SmoothBetweenMeshes = smooth_meshes;

		bool disable_export_aabs = (IsDlgButtonChecked(Hwnd,IDC_EXPORT_MESH_AABTREES) != BST_CHECKED);
		changed = changed || (Options->DisableExportAABTrees != disable_export_aabs);
		Options->DisableExportAABTrees = disable_export_aabs;

		bool convert_materials = (IsDlgButtonChecked(Hwnd,IDC_EXPORT_MESH_MAT_TO_TEXTURE) == BST_CHECKED);
		changed = changed || (Options->EnableMaterialColorToTextureConversion != convert_materials);
		Options->EnableMaterialColorToTextureConversion = convert_materials;

#if ENABLE_MESH_OPTIMIZING
		bool optimize_mesh_data = (IsDlgButtonChecked(Hwnd,IDC_EXPORT_MESH_OPTIMIZE) == BST_CHECKED);
		changed = changed || (Options->EnableOptimizeMeshData != optimize_mesh_data);
		Options->EnableOptimizeMeshData = optimize_mesh_data;
#else
		Options->EnableOptimizeMeshData = false;
#endif

	} else {
		Options->SmoothBetweenMeshes = false;
		Options->DisableExportAABTrees = false;
		Options->EnableOptimizeMeshData = false;
		Options->EnableMaterialColorToTextureConversion = false;
	}

	// Hierarchy Options:
	bool xlation_only = (IsDlgButtonChecked(Hwnd,IDC_TRANSLATION_ONLY_CHECK) == BST_CHECKED);
	changed = changed || (Options->TranslationOnly != xlation_only);
	Options->TranslationOnly = xlation_only;

	bool terrain_mode = (IsDlgButtonChecked(Hwnd,IDC_TERRAIN_MODE_CHECK) == BST_CHECKED);
	changed = changed || (Options->EnableTerrainMode != terrain_mode);
	Options->EnableTerrainMode = terrain_mode;

	if (Options->LoadHierarchy && (Options->ExportAnimation || Options->ExportGeometry)) {
		if (!GotHierarchyFilename) {
			MessageBox(Hwnd,"You have not supplied a Base Pose hierarchy file!","Error",MB_OK);
			if (changed) SetSaveRequiredFlag(true);
			return FALSE;
		}

		RawFileClass file(Options->HierarchyFilename);
		if (!file.Open()) {
			char buf[100+_MAX_FNAME+_MAX_EXT];
			sprintf(buf,"Unable to load hierarchy file: %s\nIf this Max file has been moved, please re-select the hierarchy file.",Options->HierarchyFilename);
			MessageBox(Hwnd,buf,"Error",MB_OK);
			return FALSE;
		}
		file.Close();
	}

	// Animation options:
	int start_frame = RangeLowSpin->GetIVal();
	changed = changed || (Options->StartFrame != start_frame);
	Options->StartFrame = start_frame;

	int end_frame = RangeHighSpin->GetIVal();
	changed = changed || (Options->EndFrame != end_frame);
	Options->EndFrame = end_frame;

	bool compress_animation = (IsDlgButtonChecked(Hwnd, IDC_COMPRESS_ANIMATION_CHECK) == BST_CHECKED);
	changed = changed || (Options->CompressAnimation != compress_animation);
	Options->CompressAnimation = compress_animation;

	bool reduce_animation = (IsDlgButtonChecked(Hwnd, IDC_REDUCE_ANIMATION_CHECK) == BST_CHECKED);
	changed = changed || (Options->ReduceAnimation != reduce_animation);
	Options->ReduceAnimation = reduce_animation;

	int reduce_percent = ComboBox_GetCurSel(HwndReduce) + 1;
	changed = changed || (Options->ReduceAnimationPercent != reduce_percent);
	Options->ReduceAnimationPercent = reduce_percent;

	int flavor = ComboBox_GetCurSel(HwndFlavor);
	changed = changed || (Options->CompressAnimationFlavor != flavor);
	Options->CompressAnimationFlavor = flavor;

	char string[128];
	Edit_GetText(HwndTError, string, 128);
	float trans_error = atof(string);
	changed = changed || (Options->CompressAnimationTranslationError != trans_error);
	Options->CompressAnimationTranslationError = trans_error;

	Edit_GetText(HwndRError, string, 128);
	float rot_error = atof(string);
	changed = changed || (Options->CompressAnimationRotationError != rot_error);
	Options->CompressAnimationRotationError = rot_error;

	bool review_log = (IsDlgButtonChecked(Hwnd, IDC_VIEWLOG_CHECK) == BST_CHECKED);
	changed = changed || (Options->ReviewLog != review_log);
	Options->ReviewLog = review_log;

	// Geometry options:
	//Options->UseVoxelizer = (IsDlgButtonChecked(Hwnd,IDC_VOXELIZER_CHECK) == BST_CHECKED);
	Options->UseVoxelizer = false;

	if (changed) SetSaveRequiredFlag(true);
	return TRUE;
}

void W3dOptionsDialogClass::Enable_WHT_Export(void)
{
	EnableWindow(GetDlgItem(Hwnd,IDC_TRANSLATION_ONLY_CHECK),TRUE);
	EnableWindow(GetDlgItem(Hwnd,IDC_TERRAIN_MODE_CHECK),TRUE);
	EnableWindow(GetDlgItem(Hwnd,IDC_WHT_BROWSE_BUTTON),FALSE);
	EnableWindow(GetDlgItem(Hwnd,IDC_WHA_EXPORT_RADIO),TRUE);
}

void W3dOptionsDialogClass::Enable_WHT_Load(void)
{
	EnableWindow(GetDlgItem(Hwnd,IDC_TRANSLATION_ONLY_CHECK),FALSE);
	EnableWindow(GetDlgItem(Hwnd,IDC_TERRAIN_MODE_CHECK),FALSE);
	EnableWindow(GetDlgItem(Hwnd,IDC_WHT_BROWSE_BUTTON),TRUE);
	EnableWindow(GetDlgItem(Hwnd,IDC_WHA_EXPORT_RADIO),TRUE);
}

void W3dOptionsDialogClass::Disable_WHT_Export(void)
{
	// since there will be no hierarchy tree, disable animation
	CheckDlgButton(Hwnd,IDC_WHA_EXPORT_RADIO,BST_UNCHECKED);
	CheckDlgButton(Hwnd,IDC_WHA_NO_EXPORT_RADIO,BST_CHECKED);
	Disable_WHA_Export();

	EnableWindow(GetDlgItem(Hwnd,IDC_TRANSLATION_ONLY_CHECK),FALSE);
	EnableWindow(GetDlgItem(Hwnd,IDC_TERRAIN_MODE_CHECK),FALSE);
	EnableWindow(GetDlgItem(Hwnd,IDC_WHT_BROWSE_BUTTON),FALSE);
	EnableWindow(GetDlgItem(Hwnd,IDC_WHA_EXPORT_RADIO),FALSE);
}

void W3dOptionsDialogClass::Enable_WHA_Export(void)
{
	EnableWindow(GetDlgItem(Hwnd,IDC_RANGE_LOW_EDIT),TRUE);
	EnableWindow(GetDlgItem(Hwnd,IDC_RANGE_LOW_SPIN),TRUE);
	EnableWindow(GetDlgItem(Hwnd,IDC_RANGE_HIGH_EDIT),TRUE);
	EnableWindow(GetDlgItem(Hwnd,IDC_RANGE_HIGH_SPIN),TRUE);
	EnableWindow(GetDlgItem(Hwnd,IDC_VIEWLOG_CHECK), TRUE);

	EnableWindow(GetDlgItem(Hwnd,IDC_COMPRESS_ANIMATION_CHECK),TRUE);
	if (IsDlgButtonChecked(Hwnd, IDC_COMPRESS_ANIMATION_CHECK) == BST_CHECKED) {
		Enable_CompressAnimationOptions_Export();

	}
}

void W3dOptionsDialogClass::Disable_WHA_Export(void)
{
	EnableWindow(GetDlgItem(Hwnd,IDC_RANGE_LOW_EDIT),FALSE);
	EnableWindow(GetDlgItem(Hwnd,IDC_RANGE_LOW_SPIN),FALSE);
	EnableWindow(GetDlgItem(Hwnd,IDC_RANGE_HIGH_EDIT),FALSE);
	EnableWindow(GetDlgItem(Hwnd,IDC_RANGE_HIGH_SPIN),FALSE);
	EnableWindow(GetDlgItem(Hwnd,IDC_VIEWLOG_CHECK), FALSE);

	EnableWindow(GetDlgItem(Hwnd,IDC_COMPRESS_ANIMATION_CHECK),FALSE);
	Disable_CompressAnimationOptions_Export();
}

void W3dOptionsDialogClass::Enable_ReduceAnimationOptions_Export(void)
{
	EnableWindow(GetDlgItem(Hwnd,IDC_REDUCE_ANIMATION_COMBO), TRUE);
}

void W3dOptionsDialogClass::Disable_ReduceAnimationOptions_Export(void)
{
	EnableWindow(GetDlgItem(Hwnd,IDC_REDUCE_ANIMATION_COMBO), FALSE);
}

void W3dOptionsDialogClass::Enable_CompressAnimationOptions_Export(void)
{
	EnableWindow(GetDlgItem(Hwnd,IDC_COMPRESS_ANIMATION_FLAVOR_COMBO), TRUE);
	EnableWindow(GetDlgItem(Hwnd,IDC_MAX_TRANS_ERROR_EDIT), TRUE);
	EnableWindow(GetDlgItem(Hwnd,IDC_MAX_ROT_ERROR_EDIT), TRUE);
	EnableWindow(GetDlgItem(Hwnd,IDC_REDUCE_ANIMATION_CHECK), TRUE);
	WHA_Reduce_Animation_Check_Changed();
	WHA_Compression_Flavor_Changed();
}

void W3dOptionsDialogClass::Disable_CompressAnimationOptions_Export(void)
{
	EnableWindow(GetDlgItem(Hwnd,IDC_COMPRESS_ANIMATION_FLAVOR_COMBO), FALSE);
	EnableWindow(GetDlgItem(Hwnd,IDC_MAX_TRANS_ERROR_EDIT), FALSE);
	EnableWindow(GetDlgItem(Hwnd,IDC_MAX_ROT_ERROR_EDIT), FALSE);
	EnableWindow(GetDlgItem(Hwnd,IDC_REDUCE_ANIMATION_CHECK), FALSE);
	EnableWindow(GetDlgItem(Hwnd,IDC_REDUCE_ANIMATION_COMBO), FALSE);
}

void W3dOptionsDialogClass::Enable_WTM_Export(void)
{
	::EnableWindow(::GetDlgItem(Hwnd,IDC_EXPORT_MESH_SMOOTH_CHECK),TRUE);
	::EnableWindow(::GetDlgItem(Hwnd,IDC_EXPORT_MESH_AABTREES),TRUE);
#if ENABLE_MESH_OPTIMIZING
	::EnableWindow(::GetDlgItem(Hwnd,IDC_EXPORT_MESH_OPTIMIZE),TRUE);
#endif
}

void W3dOptionsDialogClass::Disable_WTM_Export(void)
{
	::EnableWindow(::GetDlgItem(Hwnd,IDC_EXPORT_MESH_SMOOTH_CHECK),FALSE);
	::EnableWindow(::GetDlgItem(Hwnd,IDC_EXPORT_MESH_AABTREES),FALSE);
#if ENABLE_MESH_OPTIMIZING
	::EnableWindow(::GetDlgItem(Hwnd,IDC_EXPORT_MESH_OPTIMIZE),FALSE);
#endif
}

void W3dOptionsDialogClass::WHT_Export_Radio_Changed(void)
{
	if (IsDlgButtonChecked(Hwnd, IDC_WHT_EXPORT_RADIO) == BST_CHECKED) {
		Enable_WHT_Export();
	} else if (IsDlgButtonChecked(Hwnd,IDC_WHT_LOAD_RADIO) == BST_CHECKED) {
		Enable_WHT_Load();
	} else if (IsDlgButtonChecked(Hwnd,IDC_WHT_NO_EXPORT_RADIO) == BST_CHECKED) {
		Disable_WHT_Export();
	}
}

void W3dOptionsDialogClass::WHA_Export_Radio_Changed(void)
{
	if (IsDlgButtonChecked(Hwnd, IDC_WHA_EXPORT_RADIO) == BST_CHECKED) {
		Enable_WHA_Export();
	} else if (IsDlgButtonChecked(Hwnd,IDC_WHA_NO_EXPORT_RADIO) == BST_CHECKED) {
		Disable_WHA_Export();
	}
}

void W3dOptionsDialogClass::WTM_Export_Radio_Changed(void)
{
	if (IsDlgButtonChecked(Hwnd, IDC_WTM_EXPORT_RADIO) == BST_CHECKED) {
		Enable_WTM_Export();
	} else if (IsDlgButtonChecked(Hwnd,IDC_WTM_NO_EXPORT_RADIO) == BST_CHECKED) {
		Disable_WTM_Export();
	}
}


void W3dOptionsDialogClass::WHA_Compress_Animation_Check_Changed(void)
{
	if (IsDlgButtonChecked(Hwnd, IDC_COMPRESS_ANIMATION_CHECK) == BST_CHECKED) {
		Enable_CompressAnimationOptions_Export();
	} else {
		Disable_CompressAnimationOptions_Export();
	}
}

void W3dOptionsDialogClass::WHA_Reduce_Animation_Check_Changed(void)
{
	if (IsDlgButtonChecked(Hwnd, IDC_REDUCE_ANIMATION_CHECK) == BST_CHECKED) {
		Enable_ReduceAnimationOptions_Export();
	} else {
		Disable_ReduceAnimationOptions_Export();
	}
}

void W3dOptionsDialogClass::WHA_Compression_Flavor_Changed()
{
	int flavor = ComboBox_GetCurSel(HwndFlavor);

	switch (flavor) {


		case ANIM_FLAVOR_TIMECODED: {
			WHA_Reduce_Animation_Check_Changed();
			EnableWindow(GetDlgItem(Hwnd,IDC_REDUCE_ANIMATION_CHECK), TRUE);
			EnableWindow(GetDlgItem(Hwnd,IDC_MAX_TRANS_ERROR_EDIT), TRUE);
			EnableWindow(GetDlgItem(Hwnd,IDC_MAX_ROT_ERROR_EDIT), TRUE);

			break;
		}

		case ANIM_FLAVOR_ADAPTIVE_DELTA: {
			// Disable Reduce animation controls
			Disable_ReduceAnimationOptions_Export();
			EnableWindow(GetDlgItem(Hwnd,IDC_REDUCE_ANIMATION_CHECK), FALSE);
			EnableWindow(GetDlgItem(Hwnd,IDC_MAX_TRANS_ERROR_EDIT), FALSE);
			EnableWindow(GetDlgItem(Hwnd,IDC_MAX_ROT_ERROR_EDIT), FALSE);

			break;
		}


		default:
			assert(0);  // invalid compressed flavor setting
			break;

	}

}



/***********************************************************************************************
 * _options_dialog_proc -- thunks into the Options dialog class's windows message handler      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/24/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
BOOL CALLBACK _options_dialog_proc
(
	HWND hwnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam
)
{
	static W3dOptionsDialogClass * optdialog = nullptr;

	if (message == WM_INITDIALOG) {
		optdialog = (W3dOptionsDialogClass *) lParam;
		optdialog->Hwnd = hwnd;
	}

	if (optdialog) {
		return optdialog->Dialog_Proc(hwnd, message, wParam, lParam);
	} else {
		return FALSE;
	}
}


/***********************************************************************************************
 * _init_ofn -- initialize the OpenFilename struct.                                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/24/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
void _init_ofn(void)
{
	static char _szhierarchyfilter[] = "W3D Files (*.W3D)\0*.W3D\0WHT Files (*.WHT)\0*.WHT\0\0";

	_HierarchyFileOFN.lStructSize = sizeof(OPENFILENAME);
	_HierarchyFileOFN.hwndOwner = nullptr;
	_HierarchyFileOFN.hInstance = nullptr;
	_HierarchyFileOFN.lpstrFilter = _szhierarchyfilter;
	_HierarchyFileOFN.lpstrCustomFilter = nullptr;
	_HierarchyFileOFN.nMaxCustFilter = 0;
	_HierarchyFileOFN.nFilterIndex = 0;
	_HierarchyFileOFN.lpstrFile = nullptr;
	_HierarchyFileOFN.nMaxFile = _MAX_PATH;
	_HierarchyFileOFN.lpstrFileTitle = nullptr;
	_HierarchyFileOFN.nMaxFileTitle = _MAX_FNAME + _MAX_EXT;
	_HierarchyFileOFN.lpstrInitialDir = nullptr;
	_HierarchyFileOFN.lpstrTitle = nullptr;
	_HierarchyFileOFN.Flags = OFN_HIDEREADONLY | OFN_CREATEPROMPT;
	_HierarchyFileOFN.nFileOffset = 0;
	_HierarchyFileOFN.nFileExtension = 0;
	_HierarchyFileOFN.lpstrDefExt = "wht";
	_HierarchyFileOFN.lCustData = 0L;
	_HierarchyFileOFN.lpfnHook = nullptr;
	_HierarchyFileOFN.lpTemplateName = nullptr;

	_OfnInited = true;
}
