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

/* $Header: /Commando/Code/Tools/max2w3d/gmtldlg.cpp 18    5/27/98 8:34a Greg_h $ */
/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando / G 3D engine                                       * 
 *                                                                                             * 
 *                    File Name : GMTLDLG.CPP                                                  * 
 *                                                                                             * 
 *                   Programmer : Greg Hjelstrom                                               * 
 *                                                                                             * 
 *                   Start Date : 06/26/97                                                     * 
 *                                                                                             * 
 *                  Last Update : June 26, 1997 [GH]                                           *
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   GameMtlDlg::GameMtlDlg -- constructor                                                     * 
 *   GameMtlDlg::~GameMtlDlg -- destructor!                                                    * 
 *   GameMtlDlg::ClassID -- Returns the ClassID of GameMtl                                     * 
 *   GameMtlDlg::Invalidate -- causes the dialog to be redrawn                                 * 
 *   GameMtlDlg::ReloadDialog -- Updates the values in all of the dialog's controls            * 
 *   GameMtlDlg::SetTime -- Sets the time value, updates the material and the dialog           * 
 *   GameMtlDlg::PanelProc -- Windows Message handler                                          * 
 *   PanelDlgProc -- Windows Proc which thunks into GameMtlDlg::PanelProc                      * 
 *   GameMtlDlg::LoadDialog -- Sets the state of all of the dialog's controls                  * 
 *   GameMtlDlg::UpdateMtlDisplay -- Informs MAX that the material parameters have changed     * 
 *   GameMtlDlg::ActivateDlg -- Activates and deactivates the dialog                           * 
 *   GameMtlDlg::SetThing -- Sets the material to be edited                                    * 
 *   GameMtlDlg::BuildDialog -- Adds the dialog to the material editor                         * 
 *   GameMtlDlg::UpdateTexmapDisplay -- Updates the texture map buttons                        * 
 *   NotesDlgProc -- Dialog Proc which thunks to GameMtlDlg::NotesProc                         * 
 *   GameMtlDlg::NotesProc -- Dialog Proc for the Notes panel                                  * 
 *   HintsDlgProc -- Dialog proc which thunks to GameMtlDlg::HintsProc                         *
 *   GameMtlDlg::HintsProc -- Dialog Proc for the hints panel                                  *
 *   GameMtlDlg::PsxProc -- Dialog proc for the PSX options panel                              *
 *   PsxDlgProc -- Dialog proc which thunks into GameMtlDlg::PsxProc                           *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include <max.h>
#include <gport.h>
#include <hsv.h>
#include <bmmlib.h>

#include "gmtldlg.h"
#include "gamemtl.h"
#include "gamemaps.h"
#include "dllmain.h"
#include "resource.h"
#include "w3d_file.h"



static inline float PcToFrac(int pc) 
{
	return (float)pc/100.0f;	
}

static inline int FracToPc(float f) 
{
	if (f<0.0) return (int)(100.0f*f - .5f);
	else return (int) (100.0f*f + .5f);
}


/*********************************************************************************************** 
 * GameMtlDlg::GameMtlDlg -- constructor                                                       * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 * 	hwMtlEdit - windows handle of the MAX material editor												  * 
 * 	imp - Interface object for MAX materials and textures												  * 
 * 	m - pointer to a GameMtl to be edited																	  * 
 * 																														  * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   06/26/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
GameMtlDlg::GameMtlDlg(HWND hwMtlEdit, IMtlParams *imp, GameMtl *m) 
{
	HwndEdit = hwMtlEdit;
	HwndPanel = NULL;
	HwndHints = NULL;
	HwndPsx = NULL;
	HwndNotes = NULL;
	HpalOld = NULL;
	
	TheMtl = m; 
	IParams = imp;
	Valid = FALSE;
	IsActive = 0;
	InstCopy = FALSE;
	
	DiffuseSwatch = NULL;
	SpecularSwatch = NULL;

	AmbientCoeffSwatch = NULL;
	DiffuseCoeffSwatch = NULL;
	SpecularCoeffSwatch = NULL;
	EmissiveCoeffSwatch = NULL;

	DCTFramesSpin = NULL;
	DITFramesSpin = NULL;
	SCTFramesSpin = NULL;
	SITFramesSpin = NULL;

	DCTRateSpin = NULL;
	DITRateSpin = NULL;
	SCTRateSpin = NULL;
	SITRateSpin = NULL;

	OpacitySpin = NULL;
	TranslucencySpin = NULL;
	ShininessSpin = NULL;
	FogSpin = NULL;
}

/*********************************************************************************************** 
 * GameMtlDlg::~GameMtlDlg -- destructor!                                                      * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   06/26/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
GameMtlDlg::~GameMtlDlg() 
{
	if (DiffuseSwatch) {
		ReleaseIColorSwatch(DiffuseSwatch);
		DiffuseSwatch = NULL;
	}

	if (SpecularSwatch) {
		ReleaseIColorSwatch(SpecularSwatch);
		SpecularSwatch = NULL;
	}
	
	if (AmbientCoeffSwatch) {
		ReleaseIColorSwatch(AmbientCoeffSwatch);
		AmbientCoeffSwatch = NULL;
	}
	
	if (DiffuseCoeffSwatch) {
		ReleaseIColorSwatch(DiffuseCoeffSwatch);
		DiffuseCoeffSwatch = NULL;
	}

	if (SpecularCoeffSwatch) {
		ReleaseIColorSwatch(SpecularCoeffSwatch);
		SpecularCoeffSwatch = NULL;
	}
	
	if (EmissiveCoeffSwatch) {
		ReleaseIColorSwatch(EmissiveCoeffSwatch);
		EmissiveCoeffSwatch = NULL;
	}

	if (HwndPanel) {
		HDC hdc = GetDC(HwndPanel);
		GetGPort()->RestorePalette(hdc, HpalOld);
		ReleaseDC(HwndPanel,hdc);
	}

	TheMtl->SetFlag(GAMEMTL_ROLLUP1_OPEN,IParams->IsRollupPanelOpen(HwndPanel));
	TheMtl->SetFlag(GAMEMTL_ROLLUP2_OPEN,IParams->IsRollupPanelOpen(HwndPsx));
	TheMtl->SetFlag(GAMEMTL_ROLLUP3_OPEN,IParams->IsRollupPanelOpen(HwndHints));
	TheMtl->SetFlag(GAMEMTL_ROLLUP4_OPEN,IParams->IsRollupPanelOpen(HwndNotes));
	TheMtl->RollScroll = IParams->GetRollupScrollPos();
	TheMtl->SetParamDlg(NULL);

	IParams->UnRegisterDlgWnd(HwndPanel);		
	IParams->DeleteRollupPage(HwndPanel);
	HwndPanel = NULL;

	IParams->UnRegisterDlgWnd(HwndPsx);		
	IParams->DeleteRollupPage(HwndPsx);
	HwndPsx = NULL;

	IParams->UnRegisterDlgWnd(HwndHints);		
	IParams->DeleteRollupPage(HwndHints);
	HwndHints = NULL;

	IParams->UnRegisterDlgWnd(HwndNotes);		
	IParams->DeleteRollupPage(HwndNotes);
	HwndNotes = NULL;
}


/*********************************************************************************************** 
 * GameMtlDlg::ClassID -- Returns the ClassID of GameMtl                                       * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   06/26/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
Class_ID GameMtlDlg::ClassID() 
{
	return GameMaterialClassID;  
}
	
/*********************************************************************************************** 
 * GameMtlDlg::Invalidate -- causes the dialog to be redrawn                                   * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   06/26/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
void GameMtlDlg::Invalidate()
{
	Valid = FALSE;
	InvalidateRect(HwndPanel,NULL,0);
	InvalidateRect(HwndPsx,NULL,0);
	InvalidateRect(HwndHints,NULL,0);
	InvalidateRect(HwndNotes,NULL,0);
}

/*********************************************************************************************** 
 * GameMtlDlg::ReloadDialog -- Updates the values in all of the dialog's controls              * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   06/26/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
void GameMtlDlg::ReloadDialog() 
{
	Interval v;
	TheMtl->Update(IParams->GetTime(),v);
	LoadDialog(FALSE);
}

/*********************************************************************************************** 
 * GameMtlDlg::SetTime -- Sets the time value, updates the material and the dialog             * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   06/26/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
void GameMtlDlg::SetTime(TimeValue t) 
{
	if (t!=CurTime) {
		CurTime = t;
		Interval v;
		TheMtl->Update(IParams->GetTime(),v);
		LoadDialog(TRUE);
	}
}


/*********************************************************************************************** 
 * GameMtlDlg::PanelProc -- Windows Message handler                                            * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   06/26/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
BOOL GameMtlDlg::PanelProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam ) 
{
	int id = LOWORD(wParam);
	int code = HIWORD(wParam);
	int mtype;

	switch (msg) {

		case WM_INITDIALOG:
			{
				HDC theHDC = GetDC(hwndDlg);
				HpalOld = GetGPort()->PlugPalette(theHDC);
				ReleaseDC(hwndDlg,theHDC);

				DiffuseSwatch = GetIColorSwatch(GetDlgItem(hwndDlg, IDC_DIFFUSE_COLOR),TheMtl->GetDiffuse(),Get_String(IDS_DIFFUSE_COLOR));
				SpecularSwatch = GetIColorSwatch(GetDlgItem(hwndDlg, IDC_SPECULAR_COLOR),TheMtl->GetSpecular(),Get_String(IDS_SPECULAR_COLOR));
				
				AmbientCoeffSwatch = GetIColorSwatch(GetDlgItem(hwndDlg, IDC_AMBIENT_COEFF),TheMtl->GetAmbientCoeff(),Get_String(IDS_AMBIENT_COEFF));
				DiffuseCoeffSwatch = GetIColorSwatch(GetDlgItem(hwndDlg, IDC_DIFFUSE_COEFF),TheMtl->GetDiffuseCoeff(),Get_String(IDS_DIFFUSE_COEFF));
				SpecularCoeffSwatch = GetIColorSwatch(GetDlgItem(hwndDlg, IDC_SPECULAR_COEFF),TheMtl->GetSpecularCoeff(),Get_String(IDS_SPECULAR_COEFF));
				EmissiveCoeffSwatch = GetIColorSwatch(GetDlgItem(hwndDlg, IDC_EMISSIVE_COEFF),TheMtl->GetEmissiveCoeff(),Get_String(IDS_EMISSIVE_COEFF));

				DCTFramesSpin = SetupIntSpinner(hwndDlg,IDC_DCT_FRAMES_SPIN,IDC_DCT_FRAMES_EDIT,1,999,TheMtl->DCTFrames);
				DITFramesSpin = SetupIntSpinner(hwndDlg,IDC_DIT_FRAMES_SPIN,IDC_DIT_FRAMES_EDIT,1,999,TheMtl->DITFrames);
				SCTFramesSpin = SetupIntSpinner(hwndDlg,IDC_SCT_FRAMES_SPIN,IDC_SCT_FRAMES_EDIT,1,999,TheMtl->SCTFrames);
				SITFramesSpin = SetupIntSpinner(hwndDlg,IDC_SIT_FRAMES_SPIN,IDC_SIT_FRAMES_EDIT,1,999,TheMtl->SITFrames);

				DCTRateSpin = SetupFloatSpinner(hwndDlg,IDC_DCT_RATE_SPIN,IDC_DCT_RATE_EDIT,0.0f,60.0f,TheMtl->DCTFrameRate,5.0f);
				DITRateSpin = SetupFloatSpinner(hwndDlg,IDC_DIT_RATE_SPIN,IDC_DIT_RATE_EDIT,0.0f,60.0f,TheMtl->DITFrameRate,5.0f);
				SCTRateSpin = SetupFloatSpinner(hwndDlg,IDC_SCT_RATE_SPIN,IDC_SCT_RATE_EDIT,0.0f,60.0f,TheMtl->SCTFrameRate,5.0f);
				SITRateSpin = SetupFloatSpinner(hwndDlg,IDC_SIT_RATE_SPIN,IDC_SIT_RATE_EDIT,0.0f,60.0f,TheMtl->SITFrameRate,5.0f);
				
				OpacitySpin = SetupFloatSpinner(hwndDlg,IDC_OPACITY_SPIN,IDC_OPACITY_EDIT,0.0f,1.0f,TheMtl->GetOpacity(),0.01f);
				TranslucencySpin = SetupFloatSpinner(hwndDlg,IDC_TRANSLUCENCY_SPIN,IDC_TRANSULCENCY_EDIT,0.0f,1.0f,TheMtl->GetTranslucency(),0.01f);
				ShininessSpin = SetupFloatSpinner(hwndDlg,IDC_SHININESS_SPIN,IDC_SHININESS_EDIT,1.0f,1000.0f,TheMtl->GetShininess(),1.0f);
				FogSpin = SetupFloatSpinner(hwndDlg,IDC_FOG_SPIN,IDC_FOG_EDIT,0.0f,1.0f,TheMtl->FogCoeff,0.01f);

				SendDlgItemMessage( hwndDlg, IDC_DCT_MAPPING_COMBO, CB_ADDSTRING, 0, (LPARAM) (LPCTSTR) Get_String(IDS_UV_MAPPING));
				SendDlgItemMessage( hwndDlg, IDC_DCT_MAPPING_COMBO, CB_ADDSTRING, 0, (LPARAM) (LPCTSTR) Get_String(IDS_ENVIRONMENT_MAPPING) );

				SendDlgItemMessage( hwndDlg, IDC_DIT_MAPPING_COMBO, CB_ADDSTRING, 0, (LPARAM) (LPCTSTR) Get_String(IDS_UV_MAPPING));
				SendDlgItemMessage( hwndDlg, IDC_DIT_MAPPING_COMBO, CB_ADDSTRING, 0, (LPARAM) (LPCTSTR) Get_String(IDS_ENVIRONMENT_MAPPING) );
				
				SendDlgItemMessage( hwndDlg, IDC_SCT_MAPPING_COMBO, CB_ADDSTRING, 0, (LPARAM) (LPCTSTR) Get_String(IDS_UV_MAPPING));
				SendDlgItemMessage( hwndDlg, IDC_SCT_MAPPING_COMBO, CB_ADDSTRING, 0, (LPARAM) (LPCTSTR) Get_String(IDS_ENVIRONMENT_MAPPING) );
				
				SendDlgItemMessage( hwndDlg, IDC_SIT_MAPPING_COMBO, CB_ADDSTRING, 0, (LPARAM) (LPCTSTR) Get_String(IDS_UV_MAPPING));
				SendDlgItemMessage( hwndDlg, IDC_SIT_MAPPING_COMBO, CB_ADDSTRING, 0, (LPARAM) (LPCTSTR) Get_String(IDS_ENVIRONMENT_MAPPING) );

				/* Installing a windproc for texmap buttons which will handle drag-n-drop
				HWND hw = GetDlgItem(hwndDlg, texMapID[i]);
				WNDPROC oldp = (WNDPROC)GetWindowLong(hw, GWL_WNDPROC);
				SetWindowLong( hw, GWL_WNDPROC, (LONG)TexSlotWndProc);
				SetWindowLong( hw, GWL_USERDATA, (LONG)oldp);
				*/

				return TRUE;
			}
			break;

		case WM_COMMAND: 
			{
				switch (id) {

					case IDC_DCT_BUTTON:
						{
							BitmapInfo bmi;
							BitmapTex * texture;

							if (TheManager->SelectFileInput(&bmi, HwndEdit)) {
								texture = NewDefaultBitmapTex();
								if (texture) {
									texture->SetMapName((char *)bmi.Name());
									TheMtl->SetSubTexmap(ID_DI,texture);
									UpdateMtlDisplay();
									TheMtl->NotifyChanged();
								}
							}
						}
						break;

					case IDC_DIT_BUTTON:
						{
							BitmapInfo bmi;
							BitmapTex * texture;

							if (TheManager->SelectFileInput(&bmi, HwndEdit)) {
								texture = NewDefaultBitmapTex();
								if (texture) {
									texture->SetMapName((char *)bmi.Name());
									TheMtl->SetSubTexmap(ID_SI,texture);
									UpdateMtlDisplay();
									TheMtl->NotifyChanged();
								}
							}
						}
						break;

					case IDC_SCT_BUTTON:
						{
							BitmapInfo bmi;
							BitmapTex * texture;

							if (TheManager->SelectFileInput(&bmi, HwndEdit)) {
								texture = NewDefaultBitmapTex();
								if (texture) {
									texture->SetMapName((char *)bmi.Name());
									TheMtl->SetSubTexmap(ID_SP,texture);
									UpdateMtlDisplay();
									TheMtl->NotifyChanged();
								}
							}
						}
						break;

					case IDC_SIT_BUTTON:
						{
							BitmapInfo bmi;
							BitmapTex * texture;

							if (TheManager->SelectFileInput(&bmi, HwndEdit)) {
								texture = NewDefaultBitmapTex();
								if (texture) {
									texture->SetMapName((char *)bmi.Name());
									TheMtl->SetSubTexmap(ID_RL,texture);
									UpdateMtlDisplay();
									TheMtl->NotifyChanged();
								}
							}
						}
						break;

					case IDC_MAPON_DCT:
						TheMtl->EnableMap(ID_DI,GetCheckBox(hwndDlg, id));
						if (!GetCheckBox(hwndDlg,id))	TheMtl->SetSubTexmap(ID_DI,NULL);
						UpdateTexmapDisplay(ID_DI);
						UpdateMtlDisplay();
						TheMtl->NotifyChanged();
						break;
					case IDC_MAPON_DIT:
						TheMtl->EnableMap(ID_SI,GetCheckBox(hwndDlg, id));
						if (!GetCheckBox(hwndDlg,id))	TheMtl->SetSubTexmap(ID_SI,NULL);
						UpdateTexmapDisplay(ID_SI);
						UpdateMtlDisplay();
						TheMtl->NotifyChanged();
						break;
					case IDC_MAPON_SCT:
						TheMtl->EnableMap(ID_SP,GetCheckBox(hwndDlg, id));
						if (!GetCheckBox(hwndDlg,id))	TheMtl->SetSubTexmap(ID_SP,NULL);
						UpdateTexmapDisplay(ID_SP);
						UpdateMtlDisplay();
						TheMtl->NotifyChanged();
						break;
					case IDC_MAPON_SIT:
						TheMtl->EnableMap(ID_RL,GetCheckBox(hwndDlg, id));
						if (!GetCheckBox(hwndDlg,id))	TheMtl->SetSubTexmap(ID_RL,NULL);
						UpdateTexmapDisplay(ID_RL);
						UpdateMtlDisplay();
						TheMtl->NotifyChanged();
						break;
					case IDC_USE_ALPHA_CHECK:
						TheMtl->SetAttribute(GAMEMTL_USE_ALPHA,GetCheckBox(hwndDlg,IDC_USE_ALPHA_CHECK));
						UpdateMtlDisplay();
						TheMtl->NotifyChanged();
						break;
					case IDC_USE_SORTING_CHECK:
						TheMtl->SetAttribute(GAMEMTL_USE_SORTING,GetCheckBox(hwndDlg,IDC_USE_SORTING_CHECK));
						break;
					case IDC_DCT_MAPPING_COMBO:
						mtype = SendDlgItemMessage(hwndDlg,IDC_DCT_MAPPING_COMBO,CB_GETCURSEL,0,0);
						TheMtl->DCTMappingType = mtype;
						break;
					case IDC_DIT_MAPPING_COMBO:
						mtype = SendDlgItemMessage(hwndDlg,IDC_DIT_MAPPING_COMBO,CB_GETCURSEL,0,0);
						TheMtl->DITMappingType = mtype;
						break;
					case IDC_SCT_MAPPING_COMBO:
						mtype = SendDlgItemMessage(hwndDlg,IDC_SCT_MAPPING_COMBO,CB_GETCURSEL,0,0);
						TheMtl->SCTMappingType = mtype;
						break;
					case IDC_SIT_MAPPING_COMBO:
						mtype = SendDlgItemMessage(hwndDlg,IDC_SIT_MAPPING_COMBO,CB_GETCURSEL,0,0);
						TheMtl->SITMappingType = mtype;
						break;
					case IDC_VIEWPORT_DISPLAY_CHECK:
						TheMtl->Set_Viewport_Display_Status(GetCheckBox(hwndDlg,IDC_VIEWPORT_DISPLAY_CHECK));
						TheMtl->NotifyChanged();
						UpdateMtlDisplay();				
						break;
				}
			}
			break;

		case CC_COLOR_CHANGE:
			{			
				// just update all of the colors
				TheMtl->Diffuse = DiffuseSwatch->GetColor();
				TheMtl->Specular = SpecularSwatch->GetColor();
				TheMtl->AmbientCoeff = AmbientCoeffSwatch->GetColor();
				TheMtl->DiffuseCoeff = DiffuseCoeffSwatch->GetColor();
				TheMtl->SpecularCoeff = SpecularCoeffSwatch->GetColor();
				TheMtl->EmissiveCoeff = EmissiveCoeffSwatch->GetColor();

				TheMtl->NotifyChanged();
				UpdateMtlDisplay();				
			}			
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			{
				IParams->RollupMouseMessage(hwndDlg,msg,wParam,lParam);
			}
			return FALSE;

		case CC_SPINNER_CHANGE:    
			{
				TheMtl->DCTFrames = DCTFramesSpin->GetIVal();
				TheMtl->DITFrames = DITFramesSpin->GetIVal();
				TheMtl->SCTFrames = SCTFramesSpin->GetIVal();
				TheMtl->SITFrames = SITFramesSpin->GetIVal();
				
				TheMtl->DCTFrameRate = DCTRateSpin->GetFVal();
				TheMtl->DITFrameRate = DITRateSpin->GetFVal();
				TheMtl->SCTFrameRate = SCTRateSpin->GetFVal();
				TheMtl->SITFrameRate = SITRateSpin->GetFVal();

				TheMtl->SetOpacity(OpacitySpin->GetFVal());
				TheMtl->SetTranslucency(TranslucencySpin->GetFVal());
				TheMtl->SetShininess(ShininessSpin->GetFVal());
				TheMtl->FogCoeff = FogSpin->GetFVal();

				TheMtl->NotifyChanged();
				UpdateMtlDisplay();				
			}
			break;

		case CC_SPINNER_BUTTONUP: 
			{
				#if 0
				UpdateMtlDisplay();
				#endif
			}
			break;

		case WM_PAINT: 
			{
				if (!Valid) {
					Valid = TRUE;
					ReloadDialog();
				}
			}
			return FALSE;

		case WM_CLOSE:
			break;       

		case WM_DESTROY: 

			TheMtl->DCTFrames = DCTFramesSpin->GetIVal();
			TheMtl->DITFrames = DITFramesSpin->GetIVal();
			TheMtl->SCTFrames = SCTFramesSpin->GetIVal();
			TheMtl->SITFrames = SITFramesSpin->GetIVal();
	
			TheMtl->DCTFrameRate = DCTRateSpin->GetFVal();
			TheMtl->DITFrameRate = DITRateSpin->GetFVal();
			TheMtl->SCTFrameRate = SCTRateSpin->GetFVal();
			TheMtl->SITFrameRate = SITRateSpin->GetFVal();

			TheMtl->SetOpacity(OpacitySpin->GetFVal());
			TheMtl->SetTranslucency(TranslucencySpin->GetFVal());
			TheMtl->SetShininess(ShininessSpin->GetFVal());
			TheMtl->FogCoeff = FogSpin->GetFVal();

			ReleaseISpinner(DCTFramesSpin);
			ReleaseISpinner(DITFramesSpin);
			ReleaseISpinner(SCTFramesSpin);
			ReleaseISpinner(SITFramesSpin);
			ReleaseISpinner(DCTRateSpin);
			ReleaseISpinner(DITRateSpin);
			ReleaseISpinner(SCTRateSpin);
			ReleaseISpinner(SITRateSpin);
			ReleaseISpinner(OpacitySpin);
			ReleaseISpinner(TranslucencySpin);
			ReleaseISpinner(ShininessSpin);
			ReleaseISpinner(FogSpin);
	
			DCTFramesSpin = DITFramesSpin = SCTFramesSpin = SITFramesSpin = NULL;
			DCTRateSpin = DITRateSpin = SCTRateSpin = SITRateSpin = NULL;
			OpacitySpin = TranslucencySpin = ShininessSpin = FogSpin = NULL;

			break;

  	}

	return FALSE;
}


/*********************************************************************************************** 
 * PanelDlgProc -- Windows Proc which thunks into GameMtlDlg::PanelProc                        * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   06/26/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
static BOOL CALLBACK PanelDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	GameMtlDlg *theDlg;

	if (msg==WM_INITDIALOG) {
		theDlg = (GameMtlDlg*)lParam;
		theDlg->HwndPanel = hwndDlg;
		SetWindowLong(hwndDlg, GWL_USERDATA,lParam);
	} else {
		if ((theDlg = (GameMtlDlg *)GetWindowLong(hwndDlg, GWL_USERDATA) ) == NULL) {
			return FALSE; 
		}
	}

	BOOL res;
	theDlg->IsActive = 1;
	res = theDlg->PanelProc(hwndDlg,msg,wParam,lParam);
	theDlg->IsActive = 0;
	return res;
}

/*********************************************************************************************** 
 * GameMtlDlg::NotesProc -- Dialog Proc for the Notes panel                                    * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   06/26/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
BOOL GameMtlDlg::NotesProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	int id = LOWORD(wParam);
	int code = HIWORD(wParam);

	switch (msg) {

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
		{
			IParams->RollupMouseMessage(hwndDlg,msg,wParam,lParam);
			return FALSE;
		}
  	
		case WM_COMMAND: 
		{
			int i = lParam;
		}
		break;
	
	}
	return FALSE;
}


/*********************************************************************************************** 
 * NotesDlgProc -- Dialog Proc which thunks to GameMtlDlg::NotesProc                           * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   06/26/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
static BOOL CALLBACK NotesDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	GameMtlDlg *theDlg;

	if (msg==WM_INITDIALOG) {
		theDlg = (GameMtlDlg*)lParam;
		theDlg->HwndNotes = hwndDlg;
		SetWindowLong(hwndDlg, GWL_USERDATA,lParam);
	} else {
		if ((theDlg = (GameMtlDlg *)GetWindowLong(hwndDlg, GWL_USERDATA) ) == NULL) {
			return FALSE; 
		}
	}

	BOOL res;
	theDlg->IsActive = 1;
	res = theDlg->NotesProc(hwndDlg,msg,wParam,lParam);
	theDlg->IsActive = 0;
	return res;
}


/***********************************************************************************************
 * GameMtlDlg::HintsProc -- Dialog Proc for the hints panel                                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/30/98    GTH : Created.                                                                 *
 *=============================================================================================*/
BOOL GameMtlDlg::HintsProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	int id = LOWORD(wParam);
	int code = HIWORD(wParam);

	switch (msg) {


		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
		{
			IParams->RollupMouseMessage(hwndDlg,msg,wParam,lParam);
			return FALSE;
		}

		case WM_COMMAND: 
		{
			switch(id) 
			{
			case IDC_DIT_OVER_DCT_CHECK:
				TheMtl->SetAttribute(GAMEMTL_DIT_OVER_DCT, GetCheckBox(hwndDlg, IDC_DIT_OVER_DCT_CHECK));			
				break;

			case IDC_SIT_OVER_SCT_CHECK:
				TheMtl->SetAttribute(GAMEMTL_SIT_OVER_SCT, GetCheckBox(hwndDlg, IDC_SIT_OVER_SCT_CHECK));			
				break;
			
			case IDC_DIT_OVER_DIG_CHECK:
				TheMtl->SetAttribute(GAMEMTL_DIT_OVER_DIG, GetCheckBox(hwndDlg, IDC_DIT_OVER_DIG_CHECK));			
				break;
			
			case IDC_SIT_OVER_SIG_CHECK:
				TheMtl->SetAttribute(GAMEMTL_SIT_OVER_SIG, GetCheckBox(hwndDlg, IDC_SIT_OVER_SIG_CHECK));		
				break;

			case IDC_FAST_SPECULAR_AFTER_ALPHA_CHECK:
				TheMtl->SetAttribute(GAMEMTL_FAST_SPECULAR_AFTER_ALPHA, GetCheckBox(hwndDlg, IDC_FAST_SPECULAR_AFTER_ALPHA_CHECK));
				break;
			}
		}
		break;
	}
	return FALSE;
}


/***********************************************************************************************
 * HintsDlgProc -- Dialog proc which thunks to GameMtlDlg::HintsProc                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/30/98    GTH : Created.                                                                 *
 *=============================================================================================*/
static BOOL CALLBACK HintsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	GameMtlDlg *theDlg;

	if (msg==WM_INITDIALOG) {
		theDlg = (GameMtlDlg*)lParam;
		theDlg->HwndHints = hwndDlg;
		SetWindowLong(hwndDlg, GWL_USERDATA,lParam);
	} else {
		if ((theDlg = (GameMtlDlg *)GetWindowLong(hwndDlg, GWL_USERDATA) ) == NULL) {
			return FALSE; 
		}
	}

	BOOL res;
	theDlg->IsActive = 1;
	res = theDlg->HintsProc(hwndDlg,msg,wParam,lParam);
	theDlg->IsActive = 0;
	return res;
}


/***********************************************************************************************
 * GameMtlDlg::PsxProc -- Dialog proc for the PSX options panel                                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/31/98    GTH : Created.                                                                 *
 *=============================================================================================*/
BOOL GameMtlDlg::PsxProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	int id = LOWORD(wParam);
	int code = HIWORD(wParam);

	switch (msg) {

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
		{
			IParams->RollupMouseMessage(hwndDlg,msg,wParam,lParam);
			return FALSE;
		}

		case WM_COMMAND: 
		{
			switch(id) 
			{
			case IDC_NO_TRANS:
				TheMtl->SetMaskedAttribute(GAMEMTL_PSX_TRANS_MASK,0);
				break;

			case IDC_100_TRANS:
				TheMtl->SetMaskedAttribute(GAMEMTL_PSX_TRANS_MASK,GAMEMTL_PSX_100_TRANS);
				break;
			
			case IDC_50_TRANS:
				TheMtl->SetMaskedAttribute(GAMEMTL_PSX_TRANS_MASK,GAMEMTL_PSX_50_TRANS);
				break;
			
			case IDC_25_TRANS:
				TheMtl->SetMaskedAttribute(GAMEMTL_PSX_TRANS_MASK,GAMEMTL_PSX_25_TRANS);
				break;

			case IDC_MINUS_100_TRANS:
				TheMtl->SetMaskedAttribute(GAMEMTL_PSX_TRANS_MASK,GAMEMTL_PSX_MINUS_100_TRANS);
				break;
			
			case IDC_NO_RT_LIGHTING:
				TheMtl->SetAttribute(GAMEMTL_PSX_NO_RT_LIGHTING, GetCheckBox(hwndDlg, IDC_NO_RT_LIGHTING));
				break;
			}
		}
		break;
	}
	return FALSE;
}


/***********************************************************************************************
 * PsxDlgProc -- Dialog proc which thunks into GameMtlDlg::PsxProc                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/31/98    GTH : Created.                                                                 *
 *=============================================================================================*/
static BOOL CALLBACK PsxDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	GameMtlDlg *theDlg;

	if (msg==WM_INITDIALOG) {
		theDlg = (GameMtlDlg*)lParam;
		theDlg->HwndPsx = hwndDlg;
		SetWindowLong(hwndDlg, GWL_USERDATA,lParam);
	} else {
		if ((theDlg = (GameMtlDlg *)GetWindowLong(hwndDlg, GWL_USERDATA) ) == NULL) {
			return FALSE; 
		}
	}

	BOOL res;
	theDlg->IsActive = 1;
	res = theDlg->PsxProc(hwndDlg,msg,wParam,lParam);
	theDlg->IsActive = 0;
	return res;
}


/*********************************************************************************************** 
 * GameMtlDlg::LoadDialog -- Sets the state of all of the dialog's controls                    * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   06/26/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
void  GameMtlDlg::LoadDialog(BOOL draw) 
{

	/*
	** Set the state of the entire panel based on the current material.
	*/
	if (TheMtl && HwndPanel) {
	
		/*
		** Init all of the color swatches
		*/		
		if (DiffuseSwatch) {
			DiffuseSwatch->InitColor(TheMtl->GetDiffuse());
		}
		if (SpecularSwatch) {
			SpecularSwatch->InitColor(TheMtl->GetSpecular());
		}
		if (AmbientCoeffSwatch) {
			AmbientCoeffSwatch->InitColor(TheMtl->GetAmbientCoeff());
		}
		if (DiffuseCoeffSwatch) {
			DiffuseCoeffSwatch->InitColor(TheMtl->GetDiffuseCoeff());
		}
		if (SpecularCoeffSwatch) {
			SpecularCoeffSwatch->InitColor(TheMtl->GetSpecularCoeff());
		}
		if (EmissiveCoeffSwatch) {
			EmissiveCoeffSwatch->InitColor(TheMtl->GetEmissiveCoeff());
		}

		/*
		** Checkboxes
		*/
		SetCheckBox(HwndPanel,IDC_USE_ALPHA_CHECK, TheMtl->GetAttribute(GAMEMTL_USE_ALPHA));
		SetCheckBox(HwndPanel,IDC_USE_SORTING_CHECK, TheMtl->GetAttribute(GAMEMTL_USE_SORTING));
		SetCheckBox(HwndPanel,IDC_VIEWPORT_DISPLAY_CHECK,TheMtl->Get_Viewport_Display_Status());

		/*
		** Texture maps enable checks.
		*/
		SetCheckBox(HwndPanel,IDC_MAPON_DCT, TheMtl->SubTexmapOn(ID_DI));
		SetCheckBox(HwndPanel,IDC_MAPON_DIT, TheMtl->SubTexmapOn(ID_SI)); 
		SetCheckBox(HwndPanel,IDC_MAPON_SCT, TheMtl->SubTexmapOn(ID_SP));
		SetCheckBox(HwndPanel,IDC_MAPON_SIT, TheMtl->SubTexmapOn(ID_RL));

		/*
		** Mapping types
		*/
		SendDlgItemMessage( HwndPanel, IDC_DCT_MAPPING_COMBO, CB_SETCURSEL, TheMtl->DCTMappingType, 0 );
		SendDlgItemMessage( HwndPanel, IDC_DIT_MAPPING_COMBO, CB_SETCURSEL, TheMtl->DITMappingType, 0 );
		SendDlgItemMessage( HwndPanel, IDC_SCT_MAPPING_COMBO, CB_SETCURSEL, TheMtl->SCTMappingType, 0 );
		SendDlgItemMessage( HwndPanel, IDC_SIT_MAPPING_COMBO, CB_SETCURSEL, TheMtl->SITMappingType, 0 );

		/*
		** Texture animation parameters
		*/
		DCTFramesSpin->SetValue(TheMtl->DCTFrames,FALSE);
		DITFramesSpin->SetValue(TheMtl->DITFrames,FALSE);
		SCTFramesSpin->SetValue(TheMtl->SCTFrames,FALSE);
		SITFramesSpin->SetValue(TheMtl->SITFrames,FALSE);

		DCTRateSpin->SetValue(TheMtl->DCTFrameRate,FALSE);
		DITRateSpin->SetValue(TheMtl->DITFrameRate,FALSE);
		SCTRateSpin->SetValue(TheMtl->SCTFrameRate,FALSE);
		SITRateSpin->SetValue(TheMtl->SITFrameRate,FALSE);

		/*
		** Opacity, translucency, etc
		*/
		OpacitySpin->SetValue(TheMtl->Opacity,FALSE);
		TranslucencySpin->SetValue(TheMtl->Translucency,FALSE);
		ShininessSpin->SetValue(TheMtl->Shininess,FALSE);
		FogSpin->SetValue(TheMtl->FogCoeff,FALSE);

		/*
		** Init the Psx flags state
		*/
		SetCheckBox(HwndPsx,IDC_NO_RT_LIGHTING, TheMtl->GetAttribute(GAMEMTL_PSX_NO_RT_LIGHTING));
		
		SetCheckBox(HwndPsx,IDC_NO_TRANS, false);
		SetCheckBox(HwndPsx,IDC_100_TRANS, false);
		SetCheckBox(HwndPsx,IDC_50_TRANS, false);
		SetCheckBox(HwndPsx,IDC_25_TRANS, false);
		SetCheckBox(HwndPsx,IDC_MINUS_100_TRANS, false);

		switch (TheMtl->GetMaskedAttribute(GAMEMTL_PSX_TRANS_MASK)) {
			case 0:
				SetCheckBox(HwndPsx,IDC_NO_TRANS,true);
				break;
			case GAMEMTL_PSX_100_TRANS:
				SetCheckBox(HwndPsx,IDC_100_TRANS,true);
				break;
			case GAMEMTL_PSX_50_TRANS:
				SetCheckBox(HwndPsx,IDC_50_TRANS,true);
				break;
			case GAMEMTL_PSX_25_TRANS:
				SetCheckBox(HwndPsx,IDC_25_TRANS,true);
				break;
			case GAMEMTL_PSX_MINUS_100_TRANS:
				SetCheckBox(HwndPsx,IDC_MINUS_100_TRANS,true);
				break;
		}


		/*
		** Init the Hints state
		*/
		SetCheckBox(HwndHints,IDC_DIT_OVER_DCT_CHECK, TheMtl->GetAttribute(GAMEMTL_DIT_OVER_DCT));
		SetCheckBox(HwndHints,IDC_SIT_OVER_SCT_CHECK, TheMtl->GetAttribute(GAMEMTL_SIT_OVER_SCT));
		SetCheckBox(HwndHints,IDC_DIT_OVER_DIG_CHECK, TheMtl->GetAttribute(GAMEMTL_DIT_OVER_DIG));
		SetCheckBox(HwndHints,IDC_SIT_OVER_SIG_CHECK, TheMtl->GetAttribute(GAMEMTL_SIT_OVER_SIG));
		SetCheckBox(HwndHints,IDC_FAST_SPECULAR_AFTER_ALPHA_CHECK, TheMtl->GetAttribute(GAMEMTL_FAST_SPECULAR_AFTER_ALPHA));

		/*
		** Init the texmaps state
		*/
		for (int i=0; i<NTEXMAPS; i++) {
			UpdateTexmapDisplay(i);
		}
	}
}

/*********************************************************************************************** 
 * GameMtlDlg::UpdateMtlDisplay -- Informs MAX that the material parameters have changed       * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   06/26/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
void GameMtlDlg::UpdateMtlDisplay() 
{
	IParams->MtlChanged();
}


/*********************************************************************************************** 
 * GameMtlDlg::ActivateDlg -- Activates and deactivates the dialog                             * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   06/26/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
void GameMtlDlg::ActivateDlg(BOOL onOff)
{
	if (DiffuseSwatch) {
		DiffuseSwatch->Activate(onOff);
	}
	if (SpecularSwatch) {
		SpecularSwatch->Activate(onOff);
	}
	if (AmbientCoeffSwatch) {
		AmbientCoeffSwatch->Activate(onOff);
	}
	if (DiffuseCoeffSwatch) {
		DiffuseCoeffSwatch->Activate(onOff);
	}
	if (SpecularCoeffSwatch) {
		SpecularCoeffSwatch->Activate(onOff);
	}
	if (EmissiveCoeffSwatch) {
		EmissiveCoeffSwatch->Activate(onOff);
	}
}

/*********************************************************************************************** 
 * GameMtlDlg::SetThing -- Sets the material to be edited                                      * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   06/26/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
void GameMtlDlg::SetThing(ReferenceTarget *m) 
{
	assert (m->SuperClassID()==MATERIAL_CLASS_ID);
	assert (m->ClassID()==GameMaterialClassID);

	if (TheMtl) {
		TheMtl->ParamPanel = NULL;
	}

	TheMtl = (GameMtl *)m;
	
	if (TheMtl)	{
		TheMtl->ParamPanel = this;
	}

	LoadDialog(TRUE);

	if (HwndPanel && DiffuseSwatch) {
		DiffuseSwatch->InitColor(TheMtl->GetDiffuse());
	}
	if (HwndPanel && SpecularSwatch) {
		SpecularSwatch->InitColor(TheMtl->GetSpecular());
	}
	if (HwndPanel && AmbientCoeffSwatch) {
		AmbientCoeffSwatch->InitColor(TheMtl->GetAmbientCoeff());
	}
	if (HwndPanel && DiffuseCoeffSwatch) {
		DiffuseCoeffSwatch->InitColor(TheMtl->GetDiffuseCoeff());
	}
	if (HwndPanel && SpecularCoeffSwatch) {
		SpecularCoeffSwatch->InitColor(TheMtl->GetSpecularCoeff());
	}
	if (HwndPanel && EmissiveCoeffSwatch) {
		EmissiveCoeffSwatch->InitColor(TheMtl->GetEmissiveCoeff());
	}
}

/*********************************************************************************************** 
 * GameMtlDlg::BuildDialog -- Adds the dialog to the material editor                           * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   06/26/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
void GameMtlDlg::BuildDialog() 
{
	if ((TheMtl->Flags&(GAMEMTL_ROLLUP_FLAGS))==0) { 
		TheMtl->SetFlag(GAMEMTL_ROLLUP1_OPEN,TRUE);
	}

	HwndPanel = IParams->AddRollupPage( 
		AppInstance,	 
		MAKEINTRESOURCE(IDD_GAMEMTL_PANEL),
		PanelDlgProc, 
		Get_String(IDS_PARAMETERS), 
		(LPARAM)this,
		(TheMtl->GetFlag(GAMEMTL_ROLLUP1_OPEN) ? 0:APPENDROLL_CLOSED)
	);		

	HwndPsx = IParams->AddRollupPage(
		AppInstance,
		MAKEINTRESOURCE(IDD_GAMEMTL_PSX_PANEL),
		PsxDlgProc, 
		Get_String(IDS_PSX_OPTIONS), 
		(LPARAM)this,
		(TheMtl->GetFlag(GAMEMTL_ROLLUP2_OPEN) ? 0:APPENDROLL_CLOSED)
	);

	HwndHints = IParams->AddRollupPage(
		AppInstance,
		MAKEINTRESOURCE(IDD_GAMEMTL_HINTS_PANEL),
		HintsDlgProc, 
		Get_String(IDS_MATERIAL_HINTS), 
		(LPARAM)this,
		(TheMtl->GetFlag(GAMEMTL_ROLLUP3_OPEN) ? 0:APPENDROLL_CLOSED)
	);

	HwndNotes = IParams->AddRollupPage(
		AppInstance,
		MAKEINTRESOURCE(IDD_MATERIAL_NOTES_PANEL),
		NotesDlgProc, 
		Get_String(IDS_NOTES), 
		(LPARAM)this,
		(TheMtl->GetFlag(GAMEMTL_ROLLUP4_OPEN) ? 0:APPENDROLL_CLOSED)
	);

	IParams->SetRollupScrollPos(TheMtl->RollScroll);
}

/*********************************************************************************************** 
 * GameMtlDlg::UpdateTexmapDisplay -- Updates the texture map buttons                          * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   06/26/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
void GameMtlDlg::UpdateTexmapDisplay(int i) 
{
	TSTR nm = Get_String(IDS_NONE);
	Texmap *texmap = (*TheMtl->Maps)[i].Map;
	if (texmap) nm = texmap->GetFullName();

	// Diffuse Map -> Surrender Diffuse Color Channel
	if (i == ID_DI) {
		SetCheckBox(HwndPanel, IDC_MAPON_DCT, TheMtl->IsMapEnabled(i));
		SetDlgItemText(HwndPanel, IDC_DCT_BUTTON, nm.data());
	}

	// Self Illumination Map -> Surrender Diffuse Illumination Channel
	if (i == ID_SI) {
		SetCheckBox(HwndPanel, IDC_MAPON_DIT, TheMtl->IsMapEnabled(i));
		SetDlgItemText(HwndPanel, IDC_DIT_BUTTON, nm.data());
	}

	// Specular Map -> Surrender Specular Color Channel
	if (i == ID_SP) {
		SetCheckBox(HwndPanel, IDC_MAPON_SCT, TheMtl->IsMapEnabled(i));
		SetDlgItemText(HwndPanel, IDC_SCT_BUTTON, nm.data());
	}

	// Reflection Map -> Surrender Specular Illumination Channel
	if (i == ID_RL) {
		SetCheckBox(HwndPanel, IDC_MAPON_SIT, TheMtl->IsMapEnabled(i));
		SetDlgItemText(HwndPanel, IDC_SIT_BUTTON, nm.data());
	}

}


