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
 *                     $Archive:: /Commando/Code/Tools/max2w3d/GameMtlVertexMaterialDlg.h     $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 5/30/00 12:08p                                              $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */



#ifndef GAMEMTLVERTEXMATERIALDLG_H
#define GAMEMTLVERTEXMATERIALDLG_H

#include <max.h>
#include "GameMtlForm.h"

class GameMtl;

class GameMtlVertexMaterialDlg : public GameMtlFormClass
{

public:

	GameMtlVertexMaterialDlg(HWND parent, IMtlParams * imp, GameMtl * m, int pass);
	~GameMtlVertexMaterialDlg();

	virtual BOOL		Dialog_Proc (HWND dlg_wnd, UINT message, WPARAM wparam, LPARAM lparam);

	void					ActivateDlg(BOOL onoff);
	void					ReloadDialog(void);

private:

	enum { MAX_STAGES = 2 };

	IColorSwatch *		AmbientSwatch;
	IColorSwatch *		DiffuseSwatch;
	IColorSwatch *		SpecularSwatch;
	IColorSwatch *		EmissiveSwatch;

	ISpinnerControl * OpacitySpin;
	ISpinnerControl * TranslucencySpin;
	ISpinnerControl * ShininessSpin;
	ISpinnerControl * UVChannelSpin[MAX_STAGES];
};


#endif