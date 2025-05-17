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
 *                 Project name : Buccaneer Bay                                                *
 *                                                                                             *
 *                    File name : PS2GameMtlShaderDlg.cpp                                      *
 *                                                                                             *
 *                   Programmer : Mike Lytle                                                   *
 *                                                                                             *
 *                   Start date : 10/12/1999                                                   *
 *                                                                                             *
 *                  Last update : 10/12/1999                                                   *
 *                                                                                             *
 *                  Taken from GTH's GameMtlShaderDlg.cpp												  *	                                                                           
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   PS2GameMtlShaderDlg -- Constructor.                                                       *
 *   PGMSD::Dialog_Proc -- Respond to user selections.                                         *
 *   PGMSD::ReloadDialog -- Setup the dialog box.                                              *
 *   PGMSD::Apply_Preset -- Notify the material of the values for the selected setting.        *
 *   PGMSD::Set_Preset -- Sets the dialog to one of the presets or custom.                     *
 *   PGMSD::CompareShaderToBlendPreset -- Determine if the settings conform to one of the prese*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "PS2GameMtlShaderDlg.h"
#include "GameMtlDlg.h"
#include "gamemtl.h"
#include "resource.h"


#define NUM_PS2_SHADER_BLEND_PRESETS 7

static char * _PS2ShaderBlendSettingPresetNames[NUM_PS2_SHADER_BLEND_PRESETS + 1] = 
{
	"Opaque",
	"Additive",
	"Source Subtracted",
	"Destination Subtracted",
	"Alpha Blend",
	"Alpha Test",
	"Alpha Test and Blend",
	"------ Custom -----"
};

struct PS2ShaderBlendSettingPreset
{
	int	A;
	int	B;
	int	C;
	int	D;
	bool	DepthMask;
	bool	AlphaTest;
};

//	(A - B) * C + D
static const PS2ShaderBlendSettingPreset PS2ShaderBlendSettingPresets[NUM_PS2_SHADER_BLEND_PRESETS] = {
	{PSS_SRC,	PSS_ZERO,	PSS_ONE,			PSS_ZERO,	true,		false},	// Opaque
	{PSS_SRC,	PSS_ZERO,	PSS_ONE,			PSS_DEST,	false,	false},	// Additive
	{PSS_DEST,	PSS_SRC,		PSS_ONE,			PSS_ZERO,	false,	false},	// Src subtracted
	{PSS_SRC,	PSS_DEST,	PSS_ONE,			PSS_ZERO,	false,	false},	// Dest subtracted
	{PSS_SRC,	PSS_DEST,	PSS_SRC_ALPHA, PSS_DEST,	false,	false},	// Alpha blend
	{PSS_SRC,	PSS_ZERO,	PSS_ONE,			PSS_ZERO,	true,		true},	// Alpha test
	{PSS_SRC,	PSS_DEST,	PSS_SRC_ALPHA, PSS_DEST,	true,		true},	// Alpha test & blend
};


/***********************************************************************************************
 * PS2GameMtlShaderDlg -- Constructor.                                                         *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/12/1999MLL: Created.                                                                   *
 *=============================================================================================*/
PS2GameMtlShaderDlg::PS2GameMtlShaderDlg
(
	HWND				parent, 
	IMtlParams *	imp, 
	GameMtl *		mtl,
	int				pass
) : GameMtlFormClass(imp,mtl,pass)
{
	Create_Form(parent, IDD_GAMEMTL_PS2_SHADER);
}


/***********************************************************************************************
 * PGMSD::Dialog_Proc -- Respond to user selections.                                           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/12/1999MLL: Created.                                                                   *
 *=============================================================================================*/
BOOL PS2GameMtlShaderDlg::Dialog_Proc (HWND dlg_wnd, UINT message, WPARAM wparam, LPARAM lparam) 
{ 
	int cursel;
	int i;
	int id = LOWORD(wparam);
	int code = HIWORD(wparam);
	
	switch (message) 
	{

		case WM_INITDIALOG:
			for(i = 0; i <= NUM_PS2_SHADER_BLEND_PRESETS; i++) {
				SendDlgItemMessage(dlg_wnd,IDC_PS2_PRESET_COMBO,CB_ADDSTRING,0,(LONG)_PS2ShaderBlendSettingPresetNames[i]);
			}
			SendDlgItemMessage(dlg_wnd,IDC_PS2_PRESET_COMBO,CB_SETCURSEL,0,0);
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
						// Shared by both shaders.
						case IDC_DEPTHCOMPARE_COMBO:
							cursel = SendDlgItemMessage(dlg_wnd,IDC_DEPTHCOMPARE_COMBO,CB_GETCURSEL,0,0);
							TheMtl->Set_Depth_Compare(PassIndex,cursel);
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

						// Playstation 2 specific.
						case IDC_PS2_PRESET_COMBO:
							cursel = SendDlgItemMessage(dlg_wnd,IDC_PS2_PRESET_COMBO,CB_GETCURSEL,0,0);
							Apply_Preset(cursel);
							break;
						case IDC_A_COMBO:
							cursel = SendDlgItemMessage(dlg_wnd,IDC_A_COMBO,CB_GETCURSEL,0,0);
							TheMtl->Set_PS2_Shader_Param_A(PassIndex,cursel);
							TheMtl->Notify_Changed();
							Set_Preset();
							break;
						case IDC_B_COMBO:
							cursel = SendDlgItemMessage(dlg_wnd,IDC_B_COMBO,CB_GETCURSEL,0,0);
							TheMtl->Set_PS2_Shader_Param_B(PassIndex,cursel);
							TheMtl->Notify_Changed();
							Set_Preset();
							break;
						case IDC_C_COMBO:
							cursel = SendDlgItemMessage(dlg_wnd,IDC_C_COMBO,CB_GETCURSEL,0,0);
							TheMtl->Set_PS2_Shader_Param_C(PassIndex,cursel);
							TheMtl->Notify_Changed();
							Set_Preset();
							break;
						case IDC_D_COMBO:
							cursel = SendDlgItemMessage(dlg_wnd,IDC_D_COMBO,CB_GETCURSEL,0,0);
							TheMtl->Set_PS2_Shader_Param_D(PassIndex,cursel);
							TheMtl->Notify_Changed();
							Set_Preset();
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
 * PGMSD::ReloadDialog -- Setup the dialog box.                                                *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/12/1999MLL: Created.                                                                   *
 *=============================================================================================*/
void PS2GameMtlShaderDlg::ReloadDialog(void)
{
	DebugPrint("GameMtlShaderDlg::ReloadDialog\n");
	SendDlgItemMessage(m_hWnd, IDC_PRIGRADIENT_COMBO, CB_SETCURSEL, TheMtl->Get_Pri_Gradient(PassIndex), 0 );
	SendDlgItemMessage(m_hWnd, IDC_SECGRADIENT_COMBO, CB_SETCURSEL, TheMtl->Get_Sec_Gradient(PassIndex), 0 );
	SendDlgItemMessage(m_hWnd, IDC_DEPTHCOMPARE_COMBO, CB_SETCURSEL, TheMtl->Get_Depth_Compare(PassIndex), 0 );
	SendDlgItemMessage(m_hWnd, IDC_DETAILCOLOR_COMBO, CB_SETCURSEL, TheMtl->Get_Detail_Color_Func(PassIndex), 0 );
	SendDlgItemMessage(m_hWnd, IDC_DETAILALPHA_COMBO, CB_SETCURSEL, TheMtl->Get_Detail_Alpha_Func(PassIndex), 0 );
	SendDlgItemMessage(m_hWnd, IDC_A_COMBO, CB_SETCURSEL, TheMtl->Get_PS2_Shader_Param_A(PassIndex), 0);
	SendDlgItemMessage(m_hWnd, IDC_B_COMBO, CB_SETCURSEL, TheMtl->Get_PS2_Shader_Param_B(PassIndex), 0);
	SendDlgItemMessage(m_hWnd, IDC_C_COMBO, CB_SETCURSEL, TheMtl->Get_PS2_Shader_Param_C(PassIndex), 0);
	SendDlgItemMessage(m_hWnd, IDC_D_COMBO, CB_SETCURSEL, TheMtl->Get_PS2_Shader_Param_D(PassIndex), 0);

	Set_Preset();
	
	SetCheckBox(m_hWnd,IDC_DEPTHMASK_CHECK, TheMtl->Get_Depth_Mask(PassIndex));
	SetCheckBox(m_hWnd,IDC_ALPHATEST_CHECK, TheMtl->Get_Alpha_Test(PassIndex));
}


/***********************************************************************************************
 * PGMSD::Apply_Preset -- Notify the material of the values for the selected setting.          *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/12/1999MLL: Created.                                                                   *
 *=============================================================================================*/
void PS2GameMtlShaderDlg::Apply_Preset(int preset_index)
{
	if (preset_index < 0 || preset_index >= NUM_PS2_SHADER_BLEND_PRESETS) return;

	const PS2ShaderBlendSettingPreset &preset = PS2ShaderBlendSettingPresets[preset_index];

	int depth_mask = preset.DepthMask ? W3DSHADER_DEPTHMASK_WRITE_ENABLE : W3DSHADER_DEPTHMASK_WRITE_DISABLE;
	TheMtl->Set_Depth_Mask(PassIndex, depth_mask);

	int alpha_test = preset.AlphaTest ? W3DSHADER_ALPHATEST_ENABLE : W3DSHADER_ALPHATEST_DISABLE;
	TheMtl->Set_Alpha_Test(PassIndex, alpha_test);

	TheMtl->Set_PS2_Shader_Param_A(PassIndex, preset.A);
	TheMtl->Set_PS2_Shader_Param_B(PassIndex, preset.B);
	TheMtl->Set_PS2_Shader_Param_C(PassIndex, preset.C);
	TheMtl->Set_PS2_Shader_Param_D(PassIndex, preset.D);

	TheMtl->Notify_Changed();
	ReloadDialog();

	if (TheMtl->Compute_PC_Shader_From_PS2_Shader(PassIndex))
		SetWindowText(GetDlgItem(m_hWnd,IDC_COMPATIBLE), "  Compatible");
	else
		SetWindowText(GetDlgItem(m_hWnd,IDC_COMPATIBLE), "  NOT Compatible");

}

/***********************************************************************************************
 * PGMSD::Set_Preset -- Sets the dialog to one of the presets or custom.                       *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/12/1999MLL: Created.                                                                   *
 *=============================================================================================*/
void PS2GameMtlShaderDlg::Set_Preset(void)
{
	for (int i = 0; i < NUM_PS2_SHADER_BLEND_PRESETS; i++) {
		if (CompareShaderToBlendPreset(PS2ShaderBlendSettingPresets[i])) break;
	}
	SendDlgItemMessage(m_hWnd, IDC_PS2_PRESET_COMBO, CB_SETCURSEL, i, 0);

	if (TheMtl->Compute_PC_Shader_From_PS2_Shader(PassIndex))
		SetWindowText(GetDlgItem(m_hWnd,IDC_COMPATIBLE), "  Compatible");
	else
		SetWindowText(GetDlgItem(m_hWnd,IDC_COMPATIBLE), "  NOT Compatible");
}

/***********************************************************************************************
 * PGMSD::CompareShaderToBlendPreset -- Determine if the settings conform to one of the presets*
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/12/1999MLL: Created.                                                                   *
 *=============================================================================================*/
bool PS2GameMtlShaderDlg::CompareShaderToBlendPreset(const PS2ShaderBlendSettingPreset &blend_preset)
{
	bool depthmask = TheMtl->Get_Depth_Mask(PassIndex) != W3DSHADER_DEPTHMASK_WRITE_DISABLE;
	if (depthmask != blend_preset.DepthMask) return false;
	bool alphatest = TheMtl->Get_Alpha_Test(PassIndex) != W3DSHADER_ALPHATEST_DISABLE;
	if (alphatest != blend_preset.AlphaTest) return false;

	if (TheMtl->Get_PS2_Shader_Param_A(PassIndex) != blend_preset.A) return false;
	if (TheMtl->Get_PS2_Shader_Param_B(PassIndex) != blend_preset.B) return false;
	if (TheMtl->Get_PS2_Shader_Param_C(PassIndex) != blend_preset.C) return false;
	if (TheMtl->Get_PS2_Shader_Param_D(PassIndex) != blend_preset.D) return false;

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
void PS2GameMtlShaderDlg::Set_Advanced_Defaults(void)
{
	TheMtl->Set_Pri_Gradient(PassIndex, PSS_PRIGRADIENT_MODULATE);
	TheMtl->Set_Depth_Compare(PassIndex, PSS_DEPTHCOMPARE_PASS_LEQUAL);
	TheMtl->Set_Detail_Color_Func(PassIndex, W3DSHADER_DETAILCOLORFUNC_DEFAULT);
	TheMtl->Set_Detail_Alpha_Func(PassIndex, W3DSHADER_DETAILALPHAFUNC_DEFAULT);

	TheMtl->Notify_Changed();
	ReloadDialog();
}