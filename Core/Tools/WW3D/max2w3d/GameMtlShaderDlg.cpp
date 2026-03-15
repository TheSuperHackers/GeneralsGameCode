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
 *                     $Archive:: /Commando/Code/Tools/max2w3d/GameMtlShaderDlg.cpp           $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 2/26/99 7:23p                                               $*
 *                                                                                             *
 *                    $Revision:: 13                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   GameMtlShaderDlg::GameMtlShaderDlg -- constructor                                         *
 *   GameMtlShaderDlg::~GameMtlShaderDlg -- destructor                                         *
 *   GameMtlShaderDlg::Dialog_Proc -- windows message handler                                  *
 *   GameMtlShaderDlg::ReloadDialog -- reload the contents of all of the controls              *
 *   GameMtlShaderDlg::ActivateDlg -- activate/deactivate the dialog                           *
 *   GameMtlShaderDlg::Apply_Preset -- apply a preset shader setting                           *
 *   GameMtlShaderDlg::Set_Preset -- determine preset shader setting from game material        *
 *   GameMtlShaderDlg::CompareShaderToBlendPreset -- compare preset to game material shader    *
 *   GameMtlShaderDlg::Set_Advanced_Defaults -- set advanced settings to defaults              *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "GameMtlShaderDlg.h"
#include "GameMtlDlg.h"
#include "gamemtl.h"
#include "resource.h"

/*
** Shader Blend Setting Presets
*/

// Note: the array has NUM_SHADER_BLEND_PRESETS + 1 entries (due to the 'Custom' entry).

#define NUM_SHADER_BLEND_PRESETS 8

static char * _ShaderBlendSettingPresetNames[NUM_SHADER_BLEND_PRESETS + 1] =
{
	"Opaque",
	"Add",
	"Multiply",
	"Multiply and Add",
	"Screen",
	"Alpha Blend",
	"Alpha Test",
	"Alpha Test and Blend",
	"------ Custom -----"
};
struct ShaderBlendSettingPreset
{
	int	SrcBlend;
	int	DestBlend;
	bool	DepthMask;
	bool	AlphaTest;
};
static const ShaderBlendSettingPreset ShaderBlendSettingPresets[NUM_SHADER_BLEND_PRESETS] = {
	{W3DSHADER_SRCBLENDFUNC_ONE,			W3DSHADER_DESTBLENDFUNC_ZERO,						true,  false},	// Opaque
	{W3DSHADER_SRCBLENDFUNC_ONE,			W3DSHADER_DESTBLENDFUNC_ONE,						false, false},	// Add
	{W3DSHADER_SRCBLENDFUNC_ZERO,			W3DSHADER_DESTBLENDFUNC_SRC_COLOR,				false, false},	// Multiply
	{W3DSHADER_SRCBLENDFUNC_ONE,			W3DSHADER_DESTBLENDFUNC_SRC_COLOR,				false, false},	// Multiply and Add
	{W3DSHADER_SRCBLENDFUNC_ONE,			W3DSHADER_DESTBLENDFUNC_ONE_MINUS_SRC_COLOR,	false, false},	// Screen
	{W3DSHADER_SRCBLENDFUNC_SRC_ALPHA,	W3DSHADER_DESTBLENDFUNC_ONE_MINUS_SRC_ALPHA,	false, false},	// Alpha Blend
	{W3DSHADER_SRCBLENDFUNC_ONE,			W3DSHADER_DESTBLENDFUNC_ZERO,						true,  true},	// Alpha Test
	{W3DSHADER_SRCBLENDFUNC_SRC_ALPHA,	W3DSHADER_DESTBLENDFUNC_ONE_MINUS_SRC_ALPHA,	true,  true}	// Alpha Test and Blend
};


/***********************************************************************************************
 * GameMtlShaderDlg::GameMtlShaderDlg -- constructor                                           *
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
GameMtlShaderDlg::GameMtlShaderDlg
(
	HWND				parent,
	IMtlParams *	imp,
	GameMtl *		mtl,
	int				pass
) :
	GameMtlFormClass(imp,mtl,pass)
{
	Create_Form(parent,IDD_GAMEMTL_SHADER);
}


/***********************************************************************************************
 * GameMtlShaderDlg::~GameMtlShaderDlg -- destructor                                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
GameMtlShaderDlg::~GameMtlShaderDlg()
{
}


/***********************************************************************************************
 * GameMtlShaderDlg::Dialog_Proc -- windows message handler                                    *
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
BOOL GameMtlShaderDlg::Dialog_Proc (HWND dlg_wnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	int cursel;
	int i;
	int id = LOWORD(wparam);
	int code = HIWORD(wparam);

	switch (message)
	{

		case WM_INITDIALOG:
			for(i = 0; i <= NUM_SHADER_BLEND_PRESETS; i++) {
				SendDlgItemMessage(dlg_wnd,IDC_PRESET_COMBO,CB_ADDSTRING,0,(LONG)_ShaderBlendSettingPresetNames[i]);
			}
			SendDlgItemMessage(dlg_wnd,IDC_PRESET_COMBO,CB_SETCURSEL,0,0);
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			{
				IParams->RollupMouseMessage(dlg_wnd,message,wparam,lparam);
			}
			return FALSE;

		case WM_COMMAND:
			{
				if (code == CBN_SELCHANGE) {

					switch (id)
					{
						case IDC_DEPTHCOMPARE_COMBO:
							cursel = SendDlgItemMessage(dlg_wnd,IDC_DEPTHCOMPARE_COMBO,CB_GETCURSEL,0,0);
							TheMtl->Set_Depth_Compare(PassIndex,cursel);
							break;
						case IDC_DESTBLEND_COMBO:
							cursel = SendDlgItemMessage(dlg_wnd,IDC_DESTBLEND_COMBO,CB_GETCURSEL,0,0);
							TheMtl->Set_Dest_Blend(PassIndex,cursel);
							TheMtl->Notify_Changed();
							Set_Preset();
							break;
						case IDC_PRIGRADIENT_COMBO:
							cursel = SendDlgItemMessage(dlg_wnd,IDC_PRIGRADIENT_COMBO,CB_GETCURSEL,0,0);
							TheMtl->Set_Pri_Gradient(PassIndex,cursel);
							TheMtl->Notify_Changed();
							break;
						case IDC_SECGRADIENT_COMBO:
							cursel = SendDlgItemMessage(dlg_wnd,IDC_SECGRADIENT_COMBO,CB_GETCURSEL,0,0);
							TheMtl->Set_Sec_Gradient(PassIndex,cursel);
							TheMtl->Notify_Changed();
							break;
						case IDC_SRCBLEND_COMBO:
							cursel = SendDlgItemMessage(dlg_wnd,IDC_SRCBLEND_COMBO,CB_GETCURSEL,0,0);
							TheMtl->Set_Src_Blend(PassIndex,cursel);
							TheMtl->Notify_Changed();
							Set_Preset();
							break;
						case IDC_DETAILCOLOR_COMBO:
							cursel = SendDlgItemMessage(dlg_wnd,IDC_DETAILCOLOR_COMBO,CB_GETCURSEL,0,0);
							TheMtl->Set_Detail_Color_Func(PassIndex,cursel);
							TheMtl->Notify_Changed();
							break;
						case IDC_DETAILALPHA_COMBO:
							cursel = SendDlgItemMessage(dlg_wnd,IDC_DETAILALPHA_COMBO,CB_GETCURSEL,0,0);
							TheMtl->Set_Detail_Alpha_Func(PassIndex,cursel);
							TheMtl->Notify_Changed();
							break;
						case IDC_PRESET_COMBO:
							cursel = SendDlgItemMessage(dlg_wnd,IDC_PRESET_COMBO,CB_GETCURSEL,0,0);
							Apply_Preset(cursel);
							break;
					}

				} else {

					switch(id) {

						case IDC_DEPTHMASK_CHECK:
							if (SendDlgItemMessage(dlg_wnd,IDC_DEPTHMASK_CHECK,BM_GETCHECK,0,0)) {
								TheMtl->Set_Depth_Mask(PassIndex,W3DSHADER_DEPTHMASK_WRITE_ENABLE);
							} else {
								TheMtl->Set_Depth_Mask(PassIndex,W3DSHADER_DEPTHMASK_WRITE_DISABLE);
							}
							Set_Preset();
							break;

						case IDC_ALPHATEST_CHECK:
							if (SendDlgItemMessage(dlg_wnd,IDC_ALPHATEST_CHECK,BM_GETCHECK,0,0)) {
								TheMtl->Set_Alpha_Test(PassIndex,W3DSHADER_ALPHATEST_ENABLE);
							} else {
								TheMtl->Set_Alpha_Test(PassIndex,W3DSHADER_ALPHATEST_DISABLE);
							}
							Set_Preset();
							break;

						case IDC_SHADER_DEFAULTS_BUTTON:
							Set_Advanced_Defaults();
							break;
					}
				}
			}
	}

	return FALSE;
}


/***********************************************************************************************
 * GameMtlShaderDlg::ReloadDialog -- reload the contents of all of the controls                *
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
void GameMtlShaderDlg::ReloadDialog(void)
{
	DebugPrint("GameMtlShaderDlg::ReloadDialog\n");
	SendDlgItemMessage(m_hWnd, IDC_DESTBLEND_COMBO, CB_SETCURSEL, TheMtl->Get_Dest_Blend(PassIndex), 0 );
	SendDlgItemMessage(m_hWnd, IDC_SRCBLEND_COMBO, CB_SETCURSEL, TheMtl->Get_Src_Blend(PassIndex), 0 );
	SendDlgItemMessage(m_hWnd, IDC_PRIGRADIENT_COMBO, CB_SETCURSEL, TheMtl->Get_Pri_Gradient(PassIndex), 0 );
	SendDlgItemMessage(m_hWnd, IDC_SECGRADIENT_COMBO, CB_SETCURSEL, TheMtl->Get_Sec_Gradient(PassIndex), 0 );
	SendDlgItemMessage(m_hWnd, IDC_DEPTHCOMPARE_COMBO, CB_SETCURSEL, TheMtl->Get_Depth_Compare(PassIndex), 0 );
	SendDlgItemMessage(m_hWnd, IDC_DETAILCOLOR_COMBO, CB_SETCURSEL, TheMtl->Get_Detail_Color_Func(PassIndex), 0 );
	SendDlgItemMessage(m_hWnd, IDC_DETAILALPHA_COMBO, CB_SETCURSEL, TheMtl->Get_Detail_Alpha_Func(PassIndex), 0 );
	Set_Preset();

	SetCheckBox(m_hWnd,IDC_DEPTHMASK_CHECK, TheMtl->Get_Depth_Mask(PassIndex));
	SetCheckBox(m_hWnd,IDC_ALPHATEST_CHECK, TheMtl->Get_Alpha_Test(PassIndex));
}


/***********************************************************************************************
 * GameMtlShaderDlg::ActivateDlg -- activate/deactivate the dialog                             *
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
void GameMtlShaderDlg::ActivateDlg(BOOL onoff)
{
	// shader has no color swatches which need to be activated...
}


/***********************************************************************************************
 * GameMtlShaderDlg::Apply_Preset -- apply a preset shader setting                             *
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
void GameMtlShaderDlg::Apply_Preset(int preset_index)
{
	if (preset_index < 0 || preset_index >= NUM_SHADER_BLEND_PRESETS) return;

	const ShaderBlendSettingPreset &preset = ShaderBlendSettingPresets[preset_index];

	TheMtl->Set_Src_Blend(PassIndex, preset.SrcBlend);

	TheMtl->Set_Dest_Blend(PassIndex, preset.DestBlend);

	int depth_mask = preset.DepthMask ? W3DSHADER_DEPTHMASK_WRITE_ENABLE : W3DSHADER_DEPTHMASK_WRITE_DISABLE;
	TheMtl->Set_Depth_Mask(PassIndex, depth_mask);

	int alpha_test = preset.AlphaTest ? W3DSHADER_ALPHATEST_ENABLE : W3DSHADER_ALPHATEST_DISABLE;
	TheMtl->Set_Alpha_Test(PassIndex, alpha_test);

	TheMtl->Notify_Changed();
	ReloadDialog();
}


/***********************************************************************************************
 * GameMtlShaderDlg::Set_Preset -- determine preset shader setting from game material          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/26/99   NH : Created.                                                                  *
 *=============================================================================================*/
void GameMtlShaderDlg::Set_Preset(void)
{
	for (int i = 0; i < NUM_SHADER_BLEND_PRESETS; i++) {
		if (CompareShaderToBlendPreset(ShaderBlendSettingPresets[i])) break;
	}
	SendDlgItemMessage(m_hWnd, IDC_PRESET_COMBO, CB_SETCURSEL, i, 0);
}


/***********************************************************************************************
 * GameMtlShaderDlg::CompareShaderToBlendPreset -- compare preset to game material shader      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/26/99   NH : Created.                                                                  *
 *=============================================================================================*/
bool GameMtlShaderDlg::CompareShaderToBlendPreset(const ShaderBlendSettingPreset &blend_preset)
{
	if (TheMtl->Get_Src_Blend(PassIndex) != blend_preset.SrcBlend) return false;
	if (TheMtl->Get_Dest_Blend(PassIndex) != blend_preset.DestBlend) return false;
	bool depthmask = TheMtl->Get_Depth_Mask(PassIndex) != W3DSHADER_DEPTHMASK_WRITE_DISABLE;
	if (depthmask != blend_preset.DepthMask) return false;
	bool alphatest = TheMtl->Get_Alpha_Test(PassIndex) != W3DSHADER_ALPHATEST_DISABLE;
	if (alphatest != blend_preset.AlphaTest) return false;
	return true;
}


/***********************************************************************************************
 * GameMtlShaderDlg::Set_Advanced_Defaults -- set advanced settings to defaults                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/26/99   NH : Created.                                                                  *
 *=============================================================================================*/
void GameMtlShaderDlg::Set_Advanced_Defaults(void)
{
	TheMtl->Set_Pri_Gradient(PassIndex, W3DSHADER_PRIGRADIENT_DEFAULT);
	TheMtl->Set_Sec_Gradient(PassIndex, W3DSHADER_SECGRADIENT_DEFAULT);
	TheMtl->Set_Depth_Compare(PassIndex, W3DSHADER_DEPTHCOMPARE_DEFAULT);
	TheMtl->Set_Detail_Color_Func(PassIndex, W3DSHADER_DETAILCOLORFUNC_DEFAULT);
	TheMtl->Set_Detail_Alpha_Func(PassIndex, W3DSHADER_DETAILALPHAFUNC_DEFAULT);

	TheMtl->Notify_Changed();
	ReloadDialog();
}
