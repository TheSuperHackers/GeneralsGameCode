/*
**	Command & Conquer Generals(tm)
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
 *                     $Archive:: /Commando/Code/Tools/max2w3d/GameMtlTextureDlg.cpp          $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 8/14/00 1:47p                                               $*
 *                                                                                             *
 *                    $Revision:: 15                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   GameMtlTextureDlg::GameMtlTextureDlg -- constructor                                       *
 *   GameMtlTextureDlg::~GameMtlTextureDlg -- destructor                                       *
 *   GameMtlTextureDlg::Dialog_Proc -- windows message handler                                 *
 *   GameMtlTextureDlg::ReloadDialog -- reload the contents of all of the controls in this dia *
 *   GameMtlTextureDlg::ActivateDlg -- activate/deactivate this dialog                         *
 *   GameMtlTextureDlg::Enable_Stage -- enable or disable a texture stage                      *
 *   GameMtlTextureDlg::Update_Texture_Buttons -- update the texture buttons text              *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */





#include "GameMtlTextureDlg.h"
#include "gamemtl.h"
#include "dllmain.h"
#include "resource.h"
#include <max.h>
#include <bmmlib.h>
#include <stdmat.h>


/***********************************************************************************************
 * GameMtlTextureDlg::GameMtlTextureDlg -- constructor                                         *
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
GameMtlTextureDlg::GameMtlTextureDlg
(
	HWND				parent, 
	IMtlParams *	imp, 
	GameMtl *		mtl,
	int				pass
) :
	GameMtlFormClass(imp,mtl,pass)
{
	Stage0FramesSpin = NULL;
	Stage1FramesSpin = NULL;
	Stage0RateSpin = NULL;
	Stage1RateSpin = NULL;

	Stage0PublishButton = NULL;
	Stage1PublishButton = NULL;
	Stage0ClampUButton = NULL;
	Stage1ClampUButton = NULL;
	Stage0ClampVButton = NULL;
	Stage1ClampVButton = NULL;
	Stage0NoLODButton = NULL;
	Stage1NoLODButton = NULL;
	Stage0AlphaBitmapButton = NULL;
	Stage1AlphaBitmapButton = NULL;
	Stage0DisplayButton = NULL;
	Stage1DisplayButton = NULL;

	if (mtl->Get_Shader_Type() == GameMtl::STE_PC_SHADER) {
		Create_Form(parent,IDD_GAMEMTL_TEXTURES);
	} else {
		// Use the PS2 dialog.  It is the same but it disables some functions that aren't
		// supported yet.
		Create_Form(parent,IDD_GAMEMTL_PS2_TEXTURES);
	}
}


/***********************************************************************************************
 * GameMtlTextureDlg::~GameMtlTextureDlg -- destructor                                         *
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
GameMtlTextureDlg::~GameMtlTextureDlg()
{
	assert(Stage0FramesSpin && Stage1FramesSpin && Stage0RateSpin && Stage1RateSpin);
	ReleaseISpinner(Stage0FramesSpin);
	ReleaseISpinner(Stage1FramesSpin);
	ReleaseISpinner(Stage0RateSpin);
	ReleaseISpinner(Stage1RateSpin);

	ReleaseICustButton(Stage0PublishButton);
	ReleaseICustButton(Stage1PublishButton);
	ReleaseICustButton(Stage0ClampUButton);
	ReleaseICustButton(Stage1ClampUButton);
	ReleaseICustButton(Stage0ClampVButton);
	ReleaseICustButton(Stage1ClampVButton);
	ReleaseICustButton(Stage0NoLODButton);
	ReleaseICustButton(Stage1NoLODButton);
	ReleaseICustButton(Stage0AlphaBitmapButton);
	ReleaseICustButton(Stage1AlphaBitmapButton);
	ReleaseICustButton(Stage0DisplayButton);
	ReleaseICustButton(Stage1DisplayButton);
}


/***********************************************************************************************
 * GameMtlTextureDlg::Dialog_Proc -- windows message handler                                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/23/98   GTH : Created.                                                                 *
 *   10/6/1999 MLL: Turned off the display button when the texture is turned off.              *
 *=============================================================================================*/
BOOL GameMtlTextureDlg::Dialog_Proc (HWND dlg_wnd, UINT message, WPARAM wparam, LPARAM lparam) 
{ 
	int cursel;
	int id = LOWORD(wparam);
	int code = HIWORD(wparam);

	switch (message) 
	{
		case WM_INITDIALOG:
		{
			Stage0FramesSpin = SetupIntSpinner(	dlg_wnd,
															IDC_STAGE0_FRAMES_SPIN,
															IDC_STAGE0_FRAMES_EDIT,
															1,999,
															TheMtl->Get_Texture_Frame_Count(PassIndex,0) );
			
			Stage0RateSpin = SetupFloatSpinner(	dlg_wnd,
															IDC_STAGE0_RATE_SPIN,
															IDC_STAGE0_RATE_EDIT,
															0.0f,60.0f,
															TheMtl->Get_Texture_Frame_Rate(PassIndex,0),
															1.0f );
			
			Stage1FramesSpin = SetupIntSpinner(	dlg_wnd,
															IDC_STAGE1_FRAMES_SPIN,
															IDC_STAGE1_FRAMES_EDIT,
															1,999,
															TheMtl->Get_Texture_Frame_Count(PassIndex,1) );
			
			Stage1RateSpin = SetupFloatSpinner(	dlg_wnd,
															IDC_STAGE1_RATE_SPIN,
															IDC_STAGE1_RATE_EDIT,
															0.0f,60.0f,
															TheMtl->Get_Texture_Frame_Rate(PassIndex,1),
															1.0f );

			Stage0PublishButton = GetICustButton(GetDlgItem(dlg_wnd, IDC_STAGE0_PUBLISH_BUTTON));
			Stage0PublishButton->SetType(CBT_CHECK);
			Stage0PublishButton->SetHighlightColor(GREEN_WASH);
			Stage0PublishButton->SetCheck(TheMtl->Get_Texture_Publish(PassIndex,0));				
			Stage0PublishButton->SetText(Get_String(IDS_PUBLISH));

			Stage1PublishButton = GetICustButton(GetDlgItem(dlg_wnd, IDC_STAGE1_PUBLISH_BUTTON));
			Stage1PublishButton->SetType(CBT_CHECK);
			Stage1PublishButton->SetHighlightColor(GREEN_WASH);
			Stage1PublishButton->SetCheck(TheMtl->Get_Texture_Publish(PassIndex,1));				
			Stage1PublishButton->SetText(Get_String(IDS_PUBLISH));

			Stage0ClampUButton = GetICustButton(GetDlgItem(dlg_wnd, IDC_STAGE0_CLAMP_U_BUTTON));
			Stage0ClampUButton->SetType(CBT_CHECK);
			Stage0ClampUButton->SetHighlightColor(GREEN_WASH);
			Stage0ClampUButton->SetCheck(TheMtl->Get_Texture_Clamp_U(PassIndex,0));				
			Stage0ClampUButton->SetText(Get_String(IDS_CLAMP_U));

			Stage1ClampUButton = GetICustButton(GetDlgItem(dlg_wnd, IDC_STAGE1_CLAMP_U_BUTTON));
			Stage1ClampUButton->SetType(CBT_CHECK);
			Stage1ClampUButton->SetHighlightColor(GREEN_WASH);
			Stage1ClampUButton->SetCheck(TheMtl->Get_Texture_Clamp_U(PassIndex,1));				
			Stage1ClampUButton->SetText(Get_String(IDS_CLAMP_U));

			Stage0ClampVButton = GetICustButton(GetDlgItem(dlg_wnd, IDC_STAGE0_CLAMP_V_BUTTON));
			Stage0ClampVButton->SetType(CBT_CHECK);
			Stage0ClampVButton->SetHighlightColor(GREEN_WASH);
			Stage0ClampVButton->SetCheck(TheMtl->Get_Texture_Clamp_V(PassIndex,0));				
			Stage0ClampVButton->SetText(Get_String(IDS_CLAMP_V));

			Stage1ClampVButton = GetICustButton(GetDlgItem(dlg_wnd, IDC_STAGE1_CLAMP_V_BUTTON));
			Stage1ClampVButton->SetType(CBT_CHECK);
			Stage1ClampVButton->SetHighlightColor(GREEN_WASH);
			Stage1ClampVButton->SetCheck(TheMtl->Get_Texture_Clamp_V(PassIndex,1));				
			Stage1ClampVButton->SetText(Get_String(IDS_CLAMP_V));

			Stage0NoLODButton = GetICustButton(GetDlgItem(dlg_wnd, IDC_STAGE0_NOLOD_BUTTON));
			Stage0NoLODButton->SetType(CBT_CHECK);
			Stage0NoLODButton->SetHighlightColor(GREEN_WASH);
			Stage0NoLODButton->SetCheck(TheMtl->Get_Texture_No_LOD(PassIndex,0));				
			Stage0NoLODButton->SetText(Get_String(IDS_NO_LOD));

			Stage1NoLODButton = GetICustButton(GetDlgItem(dlg_wnd, IDC_STAGE1_NOLOD_BUTTON));
			Stage1NoLODButton->SetType(CBT_CHECK);
			Stage1NoLODButton->SetHighlightColor(GREEN_WASH);
			Stage1NoLODButton->SetCheck(TheMtl->Get_Texture_No_LOD(PassIndex,0));				
			Stage1NoLODButton->SetText(Get_String(IDS_NO_LOD));
			
			Stage0AlphaBitmapButton = GetICustButton(GetDlgItem(dlg_wnd, IDC_STAGE0_ALPHA_BITMAP_BUTTON));
			Stage0AlphaBitmapButton->SetType(CBT_CHECK);
			Stage0AlphaBitmapButton->SetHighlightColor(GREEN_WASH);
			Stage0AlphaBitmapButton->SetCheck(TheMtl->Get_Texture_Alpha_Bitmap(PassIndex,0));				
			Stage0AlphaBitmapButton->SetText(Get_String(IDS_ALPHA_BITMAP));

			Stage1AlphaBitmapButton = GetICustButton(GetDlgItem(dlg_wnd, IDC_STAGE1_ALPHA_BITMAP_BUTTON));
			Stage1AlphaBitmapButton->SetType(CBT_CHECK);
			Stage1AlphaBitmapButton->SetHighlightColor(GREEN_WASH);
			Stage1AlphaBitmapButton->SetCheck(TheMtl->Get_Texture_Alpha_Bitmap(PassIndex,1));				
			Stage1AlphaBitmapButton->SetText(Get_String(IDS_ALPHA_BITMAP));

			Stage0DisplayButton = GetICustButton(GetDlgItem(dlg_wnd, IDC_STAGE0_DISPLAY_BUTTON));
			Stage0DisplayButton->SetType(CBT_CHECK);
			Stage0DisplayButton->SetHighlightColor(GREEN_WASH);
			Stage0DisplayButton->SetCheck(TheMtl->Get_Texture_Display(PassIndex,0));
			Stage0DisplayButton->SetText(Get_String(IDS_DISPLAY));

			Stage1DisplayButton = GetICustButton(GetDlgItem(dlg_wnd, IDC_STAGE1_DISPLAY_BUTTON));
			Stage1DisplayButton->SetType(CBT_CHECK);
			Stage1DisplayButton->SetHighlightColor(GREEN_WASH);
			Stage1DisplayButton->SetCheck(TheMtl->Get_Texture_Display(PassIndex,1));
			Stage1DisplayButton->SetText(Get_String(IDS_DISPLAY));
			break;
		}

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
		{
			IParams->RollupMouseMessage(dlg_wnd,message,wparam,lparam);
			return FALSE;
		}

		case CC_SPINNER_CHANGE:    
		{
			TheMtl->Set_Texture_Frame_Count(	PassIndex, 0,
														Stage0FramesSpin->GetIVal());
			
			TheMtl->Set_Texture_Frame_Rate(	PassIndex, 0,
														Stage0RateSpin->GetFVal());

			TheMtl->Set_Texture_Frame_Count(	PassIndex, 1,
														Stage1FramesSpin->GetIVal());

			TheMtl->Set_Texture_Frame_Rate(	PassIndex, 1,
														Stage1RateSpin->GetFVal());
			break;
		}

		case CC_SPINNER_BUTTONUP: 
		{
			TheMtl->Notify_Changed();
			break;
		}

		case WM_COMMAND: 
		{
			switch (id) 
			{
				case IDC_STAGE0_BUTTON:
				{
					BitmapInfo bmi;
					BitmapTex * texture;

					if (TheManager->SelectFileInput(&bmi, m_hWnd)) {
						texture = NewDefaultBitmapTex();
						if (texture) {

							BOOL disp = TheMtl->Get_Texture_Display(PassIndex,0);
							if (disp) {
								TheMtl->Set_Texture_Display(PassIndex,0,FALSE);
							}

							texture->SetMapName((char *)bmi.Name());
							int texmap_index = TheMtl->pass_stage_to_texmap_index(PassIndex,0);
							TheMtl->SetSubTexmap(texmap_index,texture);
							Update_Texture_Buttons();
							TheMtl->Notify_Changed();

							if (disp) {
								TheMtl->Set_Texture_Display(PassIndex,0,TRUE);
								TheMtl->Notify_Changed();
							}
						}
					}
					break;
				}
				case IDC_STAGE1_BUTTON:
				{
					BitmapInfo bmi;
					BitmapTex * texture;

					if (TheManager->SelectFileInput(&bmi, m_hWnd)) {
						texture = NewDefaultBitmapTex();
						if (texture) {
							
							BOOL disp = TheMtl->Get_Texture_Display(PassIndex,1);
							if (disp) {
								TheMtl->Set_Texture_Display(PassIndex,1,FALSE);
							}

							texture->SetMapName((char *)bmi.Name());
							int texmap_index = TheMtl->pass_stage_to_texmap_index(PassIndex,1);
							TheMtl->SetSubTexmap(texmap_index,texture);
							Update_Texture_Buttons();
							TheMtl->Notify_Changed();
							
							if (disp) {
								TheMtl->Set_Texture_Display(PassIndex,1,TRUE);
								TheMtl->Notify_Changed();
							}
						}
					}
					break;
				}
				case IDC_STAGE0_ENABLE:
				{
					int checkbox = GetCheckBox(dlg_wnd,IDC_STAGE0_ENABLE);
					Enable_Stage(0,(checkbox == TRUE ? true : false) );

					// If the texture stage is turned off, turn off the Display button so that it won't
					// show up in the viewport.
					if (checkbox == FALSE) {

						TheMtl->Set_Texture_Display(PassIndex, 0, FALSE);
						TheMtl->Notify_Changed();
					}
					break;
				}
				case IDC_STAGE1_ENABLE:
				{
					int checkbox = GetCheckBox(dlg_wnd,IDC_STAGE1_ENABLE);

					Enable_Stage(1,(checkbox == TRUE ? true : false) );

					// If the texture stage is turned off, turn off the Display button so that it won't
					// show up in the viewport.
					if (checkbox == FALSE) {

						TheMtl->Set_Texture_Display(PassIndex, 1, FALSE);
						TheMtl->Notify_Changed();
					}
					break;
				}
				case IDC_STAGE0_PUBLISH_BUTTON:
				{
					TheMtl->Set_Texture_Publish(PassIndex,0,(Stage0PublishButton->IsChecked() ? TRUE : FALSE));
					break;
				}
				case IDC_STAGE1_PUBLISH_BUTTON:
				{
					TheMtl->Set_Texture_Publish(PassIndex,1,(Stage1PublishButton->IsChecked() ? TRUE : FALSE));
					break;
				}

				case IDC_STAGE0_CLAMP_U_BUTTON:
				{
					TheMtl->Set_Texture_Clamp_U(PassIndex,0,(Stage0ClampUButton->IsChecked() ? TRUE : FALSE));
					break;
				}
				case IDC_STAGE1_CLAMP_U_BUTTON:
				{
					TheMtl->Set_Texture_Clamp_U(PassIndex,1,(Stage1ClampUButton->IsChecked() ? TRUE : FALSE));
					break;
				}
				case IDC_STAGE0_CLAMP_V_BUTTON:
				{
					TheMtl->Set_Texture_Clamp_V(PassIndex,0,(Stage0ClampVButton->IsChecked() ? TRUE : FALSE));
					break;
				}
				case IDC_STAGE1_CLAMP_V_BUTTON:
				{
					TheMtl->Set_Texture_Clamp_V(PassIndex,1,(Stage1ClampVButton->IsChecked() ? TRUE : FALSE));
					break;
				}
				case IDC_STAGE0_NOLOD_BUTTON:
				{
					TheMtl->Set_Texture_No_LOD(PassIndex,0,(Stage0NoLODButton->IsChecked() ? TRUE : FALSE));
					break;
				}
				case IDC_STAGE1_NOLOD_BUTTON:
				{
					TheMtl->Set_Texture_No_LOD(PassIndex,1,(Stage1NoLODButton->IsChecked() ? TRUE : FALSE));
					break;
				}
				case IDC_STAGE0_ALPHA_BITMAP_BUTTON:
				{
					TheMtl->Set_Texture_Alpha_Bitmap(PassIndex,0,(Stage0AlphaBitmapButton->IsChecked() ? TRUE : FALSE));
					break;
				}
				case IDC_STAGE1_ALPHA_BITMAP_BUTTON:
				{
					TheMtl->Set_Texture_Alpha_Bitmap(PassIndex,1,(Stage0AlphaBitmapButton->IsChecked() ? TRUE : FALSE));
					break;
				}
				case IDC_STAGE0_DISPLAY_BUTTON:
				{
					TheMtl->Set_Texture_Display(PassIndex,0,(Stage0DisplayButton->IsChecked() ? TRUE : FALSE));
					TheMtl->Notify_Changed();
					break;
				}
				case IDC_STAGE1_DISPLAY_BUTTON:
				{
					TheMtl->Set_Texture_Display(PassIndex,1,(Stage1DisplayButton->IsChecked() ? TRUE : FALSE));
					TheMtl->Notify_Changed();
					break;
				}
				case IDC_STAGE0_ANIM_COMBO:
				{
					if (code == CBN_SELCHANGE) {
						cursel = SendDlgItemMessage(dlg_wnd,IDC_STAGE0_ANIM_COMBO,CB_GETCURSEL,0,0);
						TheMtl->Set_Texture_Anim_Type(PassIndex,0,cursel);
					}
					break;
				}
				case IDC_STAGE1_ANIM_COMBO:
				{
					if (code == CBN_SELCHANGE) {
						cursel = SendDlgItemMessage(dlg_wnd,IDC_STAGE1_ANIM_COMBO,CB_GETCURSEL,0,0);
						TheMtl->Set_Texture_Anim_Type(PassIndex,1,cursel);
					}
					break;
				}
				case IDC_STAGE0_HINT_COMBO:
				{
					if (code == CBN_SELCHANGE) {
						cursel = SendDlgItemMessage(dlg_wnd,IDC_STAGE0_HINT_COMBO,CB_GETCURSEL,0,0);
						TheMtl->Set_Texture_Hint(PassIndex,0,cursel);
					}
					break;
				}
				case IDC_STAGE1_HINT_COMBO:
				{
					if (code == CBN_SELCHANGE) {
						cursel = SendDlgItemMessage(dlg_wnd,IDC_STAGE1_HINT_COMBO,CB_GETCURSEL,0,0);
						TheMtl->Set_Texture_Hint(PassIndex,1,cursel);
					}
					break;
				}

			}
			break;
		}
	}

	return FALSE;
}


/***********************************************************************************************
 * GameMtlTextureDlg::ReloadDialog -- reload the contents of all of the controls in this dialo *
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
void GameMtlTextureDlg::ReloadDialog(void)
{
	DebugPrint("GameMtlTextureDlg::ReloadDialog\n");
	assert(Stage0FramesSpin && Stage1FramesSpin && Stage0RateSpin && Stage1RateSpin);
	Stage0FramesSpin->SetValue(TheMtl->Get_Texture_Frame_Count(PassIndex,0),FALSE);
	Stage1FramesSpin->SetValue(TheMtl->Get_Texture_Frame_Count(PassIndex,1),FALSE);
	Stage0RateSpin->SetValue(TheMtl->Get_Texture_Frame_Rate(PassIndex,0),FALSE);
	Stage1RateSpin->SetValue(TheMtl->Get_Texture_Frame_Rate(PassIndex,1),FALSE);
	
	SendDlgItemMessage(	m_hWnd, 
								IDC_STAGE0_ANIM_COMBO, 
								CB_SETCURSEL, 
								TheMtl->Get_Texture_Anim_Type(PassIndex,0), 0 );

	SendDlgItemMessage(	m_hWnd, 
								IDC_STAGE1_ANIM_COMBO, 
								CB_SETCURSEL, 
								TheMtl->Get_Texture_Anim_Type(PassIndex,1), 0 );

	SendDlgItemMessage(	m_hWnd, 
								IDC_STAGE0_HINT_COMBO, 
								CB_SETCURSEL, 
								TheMtl->Get_Texture_Hint(PassIndex,0), 0 );

	SendDlgItemMessage(	m_hWnd, 
								IDC_STAGE1_HINT_COMBO, 
								CB_SETCURSEL, 
								TheMtl->Get_Texture_Hint(PassIndex,1), 0 );

	SetCheckBox(m_hWnd,IDC_STAGE0_ENABLE, TheMtl->Get_Texture_Enable(PassIndex,0));
	SetCheckBox(m_hWnd,IDC_STAGE1_ENABLE, TheMtl->Get_Texture_Enable(PassIndex,1));

	Stage0PublishButton->SetCheck(TheMtl->Get_Texture_Publish(PassIndex,0));				
	Stage1PublishButton->SetCheck(TheMtl->Get_Texture_Publish(PassIndex,1));				
	Stage0ClampUButton->SetCheck(TheMtl->Get_Texture_Clamp_U(PassIndex,0));				
	Stage1ClampUButton->SetCheck(TheMtl->Get_Texture_Clamp_U(PassIndex,1));				
	Stage0ClampVButton->SetCheck(TheMtl->Get_Texture_Clamp_V(PassIndex,0));				
	Stage1ClampVButton->SetCheck(TheMtl->Get_Texture_Clamp_V(PassIndex,1));				
	Stage0NoLODButton->SetCheck(TheMtl->Get_Texture_No_LOD(PassIndex,0));				
	Stage1NoLODButton->SetCheck(TheMtl->Get_Texture_No_LOD(PassIndex,1));				
	Stage0AlphaBitmapButton->SetCheck(TheMtl->Get_Texture_Alpha_Bitmap(PassIndex,0));				
	Stage1AlphaBitmapButton->SetCheck(TheMtl->Get_Texture_Alpha_Bitmap(PassIndex,1));				
	Stage0DisplayButton->SetCheck(TheMtl->Get_Texture_Display(PassIndex,0));
	Stage1DisplayButton->SetCheck(TheMtl->Get_Texture_Display(PassIndex,1));

	Update_Texture_Buttons();

	Enable_Stage(0,TheMtl->Get_Texture_Enable(PassIndex,0));
	Enable_Stage(1,TheMtl->Get_Texture_Enable(PassIndex,1));
}


/***********************************************************************************************
 * GameMtlTextureDlg::ActivateDlg -- activate/deactivate this dialog                           *
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
void GameMtlTextureDlg::ActivateDlg(BOOL onOff)
{
	// no color swatches to activate.
}


/***********************************************************************************************
 * GameMtlTextureDlg::Enable_Stage -- enable or disable a texture stage                        *
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
void GameMtlTextureDlg::Enable_Stage(int stage,BOOL onoff)
{
	assert((stage >= 0) && (stage < W3dMaterialClass::MAX_STAGES));
	TheMtl->Set_Texture_Enable(PassIndex,stage,(onoff == TRUE ? true : false));
		
	if (stage == 0) {
		
		EnableWindow(GetDlgItem(m_hWnd,IDC_STAGE0_BUTTON),onoff);

		// Turn these off if it is a playstation 2 shader.
		// These aren't supported yet.
		if (TheMtl->Get_Shader_Type() == GameMtl::STE_PC_SHADER) {
			EnableWindow(GetDlgItem(m_hWnd,IDC_STAGE0_RATE_SPIN), onoff);
			EnableWindow(GetDlgItem(m_hWnd,IDC_STAGE0_RATE_EDIT), onoff);
			EnableWindow(GetDlgItem(m_hWnd,IDC_STAGE0_FRAMES_SPIN), onoff);
			EnableWindow(GetDlgItem(m_hWnd,IDC_STAGE0_FRAMES_EDIT), onoff);
			EnableWindow(GetDlgItem(m_hWnd,IDC_STAGE0_ANIM_COMBO), onoff);
		} else {
			EnableWindow(GetDlgItem(m_hWnd,IDC_STAGE0_RATE_SPIN), FALSE);
			EnableWindow(GetDlgItem(m_hWnd,IDC_STAGE0_RATE_EDIT), FALSE);
			EnableWindow(GetDlgItem(m_hWnd,IDC_STAGE0_FRAMES_SPIN), FALSE);
			EnableWindow(GetDlgItem(m_hWnd,IDC_STAGE0_FRAMES_EDIT), FALSE);
			EnableWindow(GetDlgItem(m_hWnd,IDC_STAGE0_ANIM_COMBO), FALSE);
		}

		EnableWindow(GetDlgItem(m_hWnd,IDC_STAGE0_PUBLISH_BUTTON),onoff);
		EnableWindow(GetDlgItem(m_hWnd,IDC_STAGE0_CLAMP_U_BUTTON),onoff);
		EnableWindow(GetDlgItem(m_hWnd,IDC_STAGE0_CLAMP_V_BUTTON),onoff);
		EnableWindow(GetDlgItem(m_hWnd,IDC_STAGE0_NOLOD_BUTTON),onoff);
		EnableWindow(GetDlgItem(m_hWnd,IDC_STAGE0_ALPHA_BITMAP_BUTTON),onoff);
		EnableWindow(GetDlgItem(m_hWnd,IDC_STAGE0_DISPLAY_BUTTON),onoff);
		EnableWindow(GetDlgItem(m_hWnd,IDC_STAGE0_HINT_COMBO),onoff);
	  		
	} else {

		EnableWindow(GetDlgItem(m_hWnd,IDC_STAGE1_BUTTON),onoff);
		EnableWindow(GetDlgItem(m_hWnd,IDC_STAGE1_RATE_SPIN),onoff);
		EnableWindow(GetDlgItem(m_hWnd,IDC_STAGE1_RATE_EDIT),onoff);
		EnableWindow(GetDlgItem(m_hWnd,IDC_STAGE1_FRAMES_SPIN),onoff);
		EnableWindow(GetDlgItem(m_hWnd,IDC_STAGE1_FRAMES_EDIT),onoff);
		EnableWindow(GetDlgItem(m_hWnd,IDC_STAGE1_PUBLISH_BUTTON),onoff);
		EnableWindow(GetDlgItem(m_hWnd,IDC_STAGE1_CLAMP_U_BUTTON),onoff);
		EnableWindow(GetDlgItem(m_hWnd,IDC_STAGE1_CLAMP_V_BUTTON),onoff);
		EnableWindow(GetDlgItem(m_hWnd,IDC_STAGE1_NOLOD_BUTTON),onoff);
		EnableWindow(GetDlgItem(m_hWnd,IDC_STAGE1_ALPHA_BITMAP_BUTTON),onoff);
		EnableWindow(GetDlgItem(m_hWnd,IDC_STAGE1_DISPLAY_BUTTON),onoff);
		EnableWindow(GetDlgItem(m_hWnd,IDC_STAGE1_ANIM_COMBO),onoff);
		EnableWindow(GetDlgItem(m_hWnd,IDC_STAGE1_HINT_COMBO),onoff);

	}
}


/***********************************************************************************************
 * GameMtlTextureDlg::Update_Texture_Buttons -- update the texture buttons text                *
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
void GameMtlTextureDlg::Update_Texture_Buttons(void)
{
	Texmap * texmap;
	texmap = TheMtl->Get_Texture(PassIndex,0);
	TSTR filename;

	if (texmap) {
		SplitPathFile(texmap->GetFullName(),NULL,&filename);
		SetDlgItemText(m_hWnd, IDC_STAGE0_BUTTON,filename);
	} else {
		SetDlgItemText(m_hWnd, IDC_STAGE0_BUTTON,Get_String(IDS_NONE));
	}

	texmap = TheMtl->Get_Texture(PassIndex,1);
	if (texmap) {
		SplitPathFile(texmap->GetFullName(),NULL,&filename);
		SetDlgItemText(m_hWnd, IDC_STAGE1_BUTTON,filename);
	} else {
		SetDlgItemText(m_hWnd, IDC_STAGE1_BUTTON,Get_String(IDS_NONE));
	}
}
