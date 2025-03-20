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
 *                     $Archive:: /Commando/Code/Tools/max2w3d/GameMtlTextureDlg.h            $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 8/14/00 1:47p                                               $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */




#ifndef GAMEMTLTEXTUREDLG_H
#define GAMEMTLTEXTUREDLG_H

#include <max.h>
#include "GameMtlForm.h"

class GameMtl;

class GameMtlTextureDlg : public GameMtlFormClass
{

public:

	GameMtlTextureDlg(HWND parent, IMtlParams * imp, GameMtl * m, int pass);
	~GameMtlTextureDlg(void);

	virtual BOOL		Dialog_Proc (HWND dlg_wnd, UINT message, WPARAM wparam, LPARAM lparam);
	void					ActivateDlg(BOOL onOff);
	void					ReloadDialog(void);

private:
	
	void					Enable_Stage(int stage,BOOL onoff);
	void					Update_Texture_Buttons(void);

	ISpinnerControl * Stage0FramesSpin;
	ISpinnerControl * Stage1FramesSpin;

	ISpinnerControl * Stage0RateSpin;
	ISpinnerControl * Stage1RateSpin;

	ICustButton *		Stage0PublishButton;
	ICustButton *		Stage1PublishButton;
	ICustButton *		Stage0ClampUButton;
	ICustButton *		Stage1ClampUButton;
	ICustButton *		Stage0ClampVButton;
	ICustButton *		Stage1ClampVButton;
	ICustButton *		Stage0NoLODButton;
	ICustButton *		Stage1NoLODButton;
	ICustButton *		Stage0AlphaBitmapButton;
	ICustButton *		Stage1AlphaBitmapButton;
	ICustButton *		Stage0DisplayButton;
	ICustButton *		Stage1DisplayButton;
};




#endif


