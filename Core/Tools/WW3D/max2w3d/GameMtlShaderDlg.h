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
 *                     $Archive:: /Commando/Code/Tools/max2w3d/GameMtlShaderDlg.h             $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 2/26/99 7:00p                                               $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */



#ifndef GAMEMTLSHADERDLG_H
#define GAMEMTLSHADERDLG_H

#include <max.h>
#include "GameMtlForm.h"


class GameMtl;
struct ShaderBlendSettingPreset;

class GameMtlShaderDlg : public GameMtlFormClass
{

public:

	GameMtlShaderDlg(HWND parent, IMtlParams * imp, GameMtl * m, int pass);
	~GameMtlShaderDlg();

	virtual BOOL		Dialog_Proc (HWND dlg_wnd, UINT message, WPARAM wparam, LPARAM lparam);

	void					ActivateDlg(BOOL onOff);
	void					ReloadDialog(void);

private:

	void					Apply_Preset(int preset_index);
	void					Set_Preset(void);
	bool					CompareShaderToBlendPreset(const ShaderBlendSettingPreset &blend_preset);
	void					Set_Advanced_Defaults(void);
};

#endif