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
 *                    File name : PS2GameMtlShaderDlg.h                                        *
 *                                                                                             *
 *                   Programmer : Mike Lytle                                                   *
 *                                                                                             *
 *                   Start date : 10/12/1999                                                   *
 *                                                                                             *
 *                  Last update : 10/12/1999                                                   *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#ifndef PS2GAMEMTLSHADERDLG_H
#define PS2GAMEMTLSHADERDLG_H

#include <max.h>
#include "GameMtlForm.h"

// This class was taken from GTH's GameMtlShaderDlg.

class GameMtl;
struct PS2ShaderBlendSettingPreset;

class PS2GameMtlShaderDlg : public GameMtlFormClass
{

public:

	PS2GameMtlShaderDlg(HWND parent, IMtlParams * imp, GameMtl * m, int pass);

	virtual BOOL		Dialog_Proc (HWND dlg_wnd, UINT message, WPARAM wparam, LPARAM lparam);

	void					ReloadDialog(void);

	// Pure virtual that must be defined.
	void					ActivateDlg(BOOL onOff) {}

private:

	void					Apply_Preset(int preset_index);
	void					Set_Preset(void);
	bool					CompareShaderToBlendPreset(const PS2ShaderBlendSettingPreset &blend_preset);
	void					Set_Advanced_Defaults(void);
};

#endif
