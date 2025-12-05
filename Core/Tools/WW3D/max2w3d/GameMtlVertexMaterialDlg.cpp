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
 *                     $Archive:: /Commando/Code/Tools/max2w3d/GameMtlVertexMaterialDlg.cpp   $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 7/10/00 3:37p                                               $*
 *                                                                                             *
 *                    $Revision:: 12                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   GameMtlVertexMaterialDlg::GameMtlVertexMaterialDlg -- constructor                         *
 *   GameMtlVertexMaterialDlg::~GameMtlVertexMaterialDlg -- destructor                         *
 *   GameMtlVertexMaterialDlg::Dialog_Proc -- windows message handler                          *
 *   GameMtlVertexMaterialDlg::ReloadDialog -- reload the contents of the controls             *
 *   GameMtlVertexMaterialDlg::ActivateDlg -- activate / deactivate this dialog                *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "GameMtlVertexMaterialDlg.h"
#include "gamemtl.h"
#include "dllmain.h"
#include "resource.h"


/***********************************************************************************************
 * GameMtlVertexMaterialDlg::GameMtlVertexMaterialDlg -- constructor                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/98   GTH : Created.                                                                 *
 *=============================================================================================*/
GameMtlVertexMaterialDlg::GameMtlVertexMaterialDlg
(
	HWND				parent,
	IMtlParams *	imp,
	GameMtl *		mtl,
	int				pass
) :
	GameMtlFormClass(imp,mtl,pass)
{
	AmbientSwatch = nullptr;
	DiffuseSwatch = nullptr;
	SpecularSwatch = nullptr;
	EmissiveSwatch = nullptr;

	OpacitySpin = nullptr;
	TranslucencySpin = nullptr;
	ShininessSpin = nullptr;

	for (int i=0; i<MAX_STAGES; i++) {
		UVChannelSpin[i] = nullptr;
	}

	Create_Form(parent,IDD_GAMEMTL_VERTEX_MATERIAL);
}


/***********************************************************************************************
 * GameMtlVertexMaterialDlg::~GameMtlVertexMaterialDlg -- destructor                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/98   GTH : Created.                                                                 *
 *=============================================================================================*/
GameMtlVertexMaterialDlg::~GameMtlVertexMaterialDlg()
{
	if (AmbientSwatch) {
		ReleaseIColorSwatch(AmbientSwatch);
	}
	if (DiffuseSwatch) {
		ReleaseIColorSwatch(DiffuseSwatch);
	}
	if (SpecularSwatch) {
		ReleaseIColorSwatch(SpecularSwatch);
	}
	if (EmissiveSwatch) {
		ReleaseIColorSwatch(EmissiveSwatch);
	}
	if (OpacitySpin) {
		ReleaseISpinner(OpacitySpin);
	}
	if (TranslucencySpin) {
		ReleaseISpinner(TranslucencySpin);
	}
	if (ShininessSpin) {
		ReleaseISpinner(ShininessSpin);
	}
	for (int i=0; i<MAX_STAGES; i++) {
		if (UVChannelSpin[i] != nullptr) {
			ReleaseISpinner(UVChannelSpin[i]);
		}
	}
}


/***********************************************************************************************
 * GameMtlVertexMaterialDlg::Dialog_Proc -- windows message handler                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/98   GTH : Created.                                                                 *
 *=============================================================================================*/
BOOL GameMtlVertexMaterialDlg::Dialog_Proc(HWND dlg_wnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	int val;
	int id = LOWORD(wparam);
	int code = HIWORD(wparam);

	switch (message)
	{
		case WM_INITDIALOG:
		{
			AmbientSwatch = GetIColorSwatch(GetDlgItem(dlg_wnd, IDC_AMBIENT_COLOR),TheMtl->Get_Ambient(PassIndex,IParams->GetTime()),Get_String(IDS_AMBIENT_COLOR));
			DiffuseSwatch = GetIColorSwatch(GetDlgItem(dlg_wnd, IDC_DIFFUSE_COLOR),TheMtl->Get_Diffuse(PassIndex,IParams->GetTime()),Get_String(IDS_DIFFUSE_COLOR));
			SpecularSwatch = GetIColorSwatch(GetDlgItem(dlg_wnd, IDC_SPECULAR_COLOR),TheMtl->Get_Specular(PassIndex,IParams->GetTime()),Get_String(IDS_SPECULAR_COLOR));
			EmissiveSwatch = GetIColorSwatch(GetDlgItem(dlg_wnd, IDC_EMISSIVE_COLOR),TheMtl->Get_Emissive(PassIndex,IParams->GetTime()),Get_String(IDS_EMISSIVE_COLOR));

			OpacitySpin = SetupFloatSpinner(dlg_wnd,IDC_OPACITY_SPIN,IDC_OPACITY_EDIT,0.0f,1.0f,TheMtl->Get_Opacity(PassIndex,IParams->GetTime()),0.01f);
			TranslucencySpin = SetupFloatSpinner(dlg_wnd,IDC_TRANSLUCENCY_SPIN,IDC_TRANSULCENCY_EDIT,0.0f,1.0f,TheMtl->Get_Translucency(PassIndex,IParams->GetTime()),0.01f);
			ShininessSpin = SetupFloatSpinner(dlg_wnd,IDC_SHININESS_SPIN,IDC_SHININESS_EDIT,1.0f,1000.0f,TheMtl->Get_Shininess(PassIndex,IParams->GetTime()),1.0f);
			UVChannelSpin[0] = SetupIntSpinner(dlg_wnd,IDC_STAGE0UVCHAN_SPIN,IDC_STAGE0UVCHAN_EDIT,1,99,1);
			UVChannelSpin[1] = SetupIntSpinner(dlg_wnd,IDC_STAGE1UVCHAN_SPIN,IDC_STAGE1UVCHAN_EDIT,1,99,1);
			break;
		}

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
		{
			IParams->RollupMouseMessage(dlg_wnd,message,wparam,lparam);
			return FALSE;
		}

		case WM_COMMAND:
		{
			switch (id)
			{
				case IDC_COPY_SPECULAR_DIFFUSE:
					TheMtl->Set_Copy_Specular_To_Diffuse(PassIndex,GetCheckBox(dlg_wnd, IDC_COPY_SPECULAR_DIFFUSE) == TRUE);
					break;

				case IDC_MAPPING0_COMBO:
					if (code == CBN_SELCHANGE) {
						val = SendDlgItemMessage(dlg_wnd,IDC_MAPPING0_COMBO,CB_GETCURSEL,0,0);
						TheMtl->Set_Mapping_Type(PassIndex,0,val);
					}
					break;

				case IDC_MAPPING1_COMBO:
					if (code == CBN_SELCHANGE) {
						val = SendDlgItemMessage(dlg_wnd,IDC_MAPPING1_COMBO,CB_GETCURSEL,0,0);
						TheMtl->Set_Mapping_Type(PassIndex,1,val);
					}
					break;

				case IDC_MAPPING0_ARGS_EDIT:
					switch (code) {
						case EN_SETFOCUS:
							DisableAccelerators();
							break;
						case EN_KILLFOCUS:
							EnableAccelerators();
							break;
						case EN_CHANGE:
							int len = GetWindowTextLength(GetDlgItem(dlg_wnd, IDC_MAPPING0_ARGS_EDIT));
							char *buffer = TheMtl->Get_Mapping_Arg_Buffer(PassIndex, 0, len);
							GetWindowText(GetDlgItem(dlg_wnd, IDC_MAPPING0_ARGS_EDIT), buffer, len + 1);
							break;
					}
					break;

				case IDC_MAPPING1_ARGS_EDIT:
					switch (code) {
						case EN_SETFOCUS:
							DisableAccelerators();
							break;
						case EN_KILLFOCUS:
							EnableAccelerators();
							break;
						case EN_CHANGE:
							int len = GetWindowTextLength(GetDlgItem(dlg_wnd, IDC_MAPPING1_ARGS_EDIT));
							char *buffer = TheMtl->Get_Mapping_Arg_Buffer(PassIndex, 1, len);
							GetWindowText(GetDlgItem(dlg_wnd, IDC_MAPPING1_ARGS_EDIT), buffer, len + 1);
							break;
					}
					break;

				case IDC_NO_TRANS:
					TheMtl->Set_PSX_Translucency(PassIndex,GAMEMTL_PSX_TRANS_NONE);
					break;

				case IDC_100_TRANS:
					TheMtl->Set_PSX_Translucency(PassIndex,GAMEMTL_PSX_TRANS_100);
					break;

				case IDC_50_TRANS:
					TheMtl->Set_PSX_Translucency(PassIndex,GAMEMTL_PSX_TRANS_50);
					break;

				case IDC_25_TRANS:
					TheMtl->Set_PSX_Translucency(PassIndex,GAMEMTL_PSX_TRANS_25);
					break;

				case IDC_MINUS_100_TRANS:
					TheMtl->Set_PSX_Translucency(PassIndex,GAMEMTL_PSX_TRANS_MINUS_100);
					break;

				case IDC_NO_RT_LIGHTING:
					TheMtl->Set_PSX_Lighting(PassIndex,!GetCheckBox(dlg_wnd, IDC_NO_RT_LIGHTING));
					break;
			}
			break;
		}

		case CC_COLOR_CHANGE:
		{
			// just update all of the colors
			TheMtl->Set_Ambient(PassIndex,IParams->GetTime(),AmbientSwatch->GetColor());
			TheMtl->Set_Diffuse(PassIndex,IParams->GetTime(),DiffuseSwatch->GetColor());
			TheMtl->Set_Specular(PassIndex,IParams->GetTime(),SpecularSwatch->GetColor());
			TheMtl->Set_Emissive(PassIndex,IParams->GetTime(),EmissiveSwatch->GetColor());
			TheMtl->Notify_Changed();
			break;
		}

		case CC_SPINNER_CHANGE:
		{
			TheMtl->Set_Shininess(PassIndex,IParams->GetTime(),ShininessSpin->GetFVal());
			TheMtl->Set_Opacity(PassIndex,IParams->GetTime(),OpacitySpin->GetFVal());
			TheMtl->Set_Translucency(PassIndex,IParams->GetTime(),TranslucencySpin->GetFVal());
			for (int i=0; i<MAX_STAGES; i++) {
				TheMtl->Set_Map_Channel(PassIndex,i,UVChannelSpin[i]->GetIVal());
			}
			break;
		}

		case CC_SPINNER_BUTTONUP:
		{
			TheMtl->Notify_Changed();
			break;
		}

	}
	return FALSE;
}


/***********************************************************************************************
 * GameMtlVertexMaterialDlg::ReloadDialog -- reload the contents of the controls               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/98   GTH : Created.                                                                 *
 *=============================================================================================*/
void GameMtlVertexMaterialDlg::ReloadDialog(void)
{
	// Vertex Material Controls
	DebugPrint("GameMtlVertexMaterialDlg::ReloadDialog\n");
	assert(AmbientSwatch && DiffuseSwatch && SpecularSwatch && EmissiveSwatch);
	assert(ShininessSpin && OpacitySpin && TranslucencySpin);

	AmbientSwatch->InitColor(TheMtl->Get_Ambient(PassIndex,IParams->GetTime()));
	DiffuseSwatch->InitColor(TheMtl->Get_Diffuse(PassIndex,IParams->GetTime()));
	SpecularSwatch->InitColor(TheMtl->Get_Specular(PassIndex,IParams->GetTime()));
	EmissiveSwatch->InitColor(TheMtl->Get_Emissive(PassIndex,IParams->GetTime()));

	ShininessSpin->SetValue(TheMtl->Get_Shininess(PassIndex,IParams->GetTime()),FALSE);
	OpacitySpin->SetValue(TheMtl->Get_Opacity(PassIndex,IParams->GetTime()),FALSE);
	TranslucencySpin->SetValue(TheMtl->Get_Translucency(PassIndex,IParams->GetTime()),FALSE);
	for (int i=0; i<MAX_STAGES; i++) {
		UVChannelSpin[i]->SetValue(TheMtl->Get_Map_Channel(PassIndex,i),FALSE);
	}

	SetCheckBox(m_hWnd,IDC_COPY_SPECULAR_DIFFUSE, TheMtl->Get_Copy_Specular_To_Diffuse(PassIndex));

	SendDlgItemMessage(	m_hWnd,
								IDC_MAPPING0_COMBO,
								CB_SETCURSEL,
								TheMtl->Get_Mapping_Type(PassIndex, 0),
								0 );

	SendDlgItemMessage(	m_hWnd,
								IDC_MAPPING1_COMBO,
								CB_SETCURSEL,
								TheMtl->Get_Mapping_Type(PassIndex, 1),
								0 );

	// PSX Controls
	SetCheckBox(m_hWnd,IDC_NO_RT_LIGHTING, !TheMtl->Get_PSX_Lighting(PassIndex));
	SetCheckBox(m_hWnd,IDC_NO_TRANS, false);
	SetCheckBox(m_hWnd,IDC_100_TRANS, false);
	SetCheckBox(m_hWnd,IDC_50_TRANS, false);
	SetCheckBox(m_hWnd,IDC_25_TRANS, false);
	SetCheckBox(m_hWnd,IDC_MINUS_100_TRANS, false);

	switch (TheMtl->Get_PSX_Translucency(PassIndex)) {
		case 0:
			SetCheckBox(m_hWnd,IDC_NO_TRANS,true);
			break;
		case GAMEMTL_PSX_TRANS_100:
			SetCheckBox(m_hWnd,IDC_100_TRANS,true);
			break;
		case GAMEMTL_PSX_TRANS_50:
			SetCheckBox(m_hWnd,IDC_50_TRANS,true);
			break;
		case GAMEMTL_PSX_TRANS_25:
			SetCheckBox(m_hWnd,IDC_25_TRANS,true);
			break;
		case GAMEMTL_PSX_TRANS_MINUS_100:
			SetCheckBox(m_hWnd,IDC_MINUS_100_TRANS,true);
			break;
	}

	// Reload contents of mapper args edit box:
	char *buffer = TheMtl->Get_Mapping_Arg_Buffer(PassIndex, 0);
	SetWindowText(GetDlgItem(m_hWnd, IDC_MAPPING0_ARGS_EDIT), buffer);
	buffer = TheMtl->Get_Mapping_Arg_Buffer(PassIndex, 1);
	SetWindowText(GetDlgItem(m_hWnd, IDC_MAPPING1_ARGS_EDIT), buffer);
}


/***********************************************************************************************
 * GameMtlVertexMaterialDlg::ActivateDlg -- activate / deactivate this dialog                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/98   GTH : Created.                                                                 *
 *=============================================================================================*/
void GameMtlVertexMaterialDlg::ActivateDlg(BOOL onoff)
{
	if (AmbientSwatch) {
		AmbientSwatch->Activate(onoff);
	}
	if (DiffuseSwatch) {
		DiffuseSwatch->Activate(onoff);
	}
	if (SpecularSwatch) {
		SpecularSwatch->Activate(onoff);
	}
	if (EmissiveSwatch) {
		EmissiveSwatch->Activate(onoff);
	}
}
