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
 *                     $Archive:: /Commando/Code/Tools/max2w3d/presetexportoptionsdialog.h    $*
 *                                                                                             *
 *              Original Author:: Patrick Smith																  *
 *                                                                                             *
 *                      $Author:: Patrick                                                     $*
 *                                                                                             *
 *                     $Modtime:: 10/30/00 2:45p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include <windows.h>
#include <max.h>
#include "w3dutil.h"


////////////////////////////////////////////////////////////////////////////////////////
//
//	PresetExportOptionsDialogClass
//
////////////////////////////////////////////////////////////////////////////////////////
class PresetExportOptionsDialogClass
{
public:

	//////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////
	PresetExportOptionsDialogClass (Interface *maxinterface, HWND parent_wnd = NULL);
	~PresetExportOptionsDialogClass (void);


	//////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////

	void			Set_Options (W3dExportOptionsStruct *options)	{ Options = options; ::memcpy (&OrigOptions, Options, sizeof (OrigOptions)); }
	int			Do_Modal (void);

private:

	//////////////////////////////////////////////////////////////////
	//	Private data types
	//////////////////////////////////////////////////////////////////

	enum
	{
		PANE_HLOD			= 0,
		PANE_ANIM_HLOD,
		PANE_ANIM,
		PANE_TERRAIN,
		PANE_SKELETON,
		PANE_MESH,
		PANE_MAX
	};


	//////////////////////////////////////////////////////////////////
	//	Static methods
	//////////////////////////////////////////////////////////////////
	static BOOL CALLBACK	Real_Message_Proc (HWND wnd, UINT message, WPARAM wparam, LPARAM lparam);
	static BOOL CALLBACK	Settings_Pane_Message_Proc (HWND wnd, UINT message, WPARAM wparam, LPARAM lparam);

	//////////////////////////////////////////////////////////////////
	//	Private methods
	//////////////////////////////////////////////////////////////////
	BOOL			Message_Proc (UINT message, WPARAM wparam, LPARAM lparam);
	BOOL			Pane_Message_Proc (UINT message, WPARAM wparam, LPARAM lparam);
	BOOL			Settings_Message_Proc (UINT message, WPARAM wparam, LPARAM lparam);
	BOOL			On_Command (WPARAM wparam, LPARAM lparam);
	void			Show_Settings_Pane (int pane_id);
	void			Create_Settings_Panes (void);
	void			Destroy_Settings_Panes (void);
	void			Determine_Preset_Type (void);
	void			Initialize_Controls (void);
	void			Update_Controls (void);
	void			Save_Settings (void);

	//////////////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////////////
	W3dExportOptionsStruct *	Options;
	W3dExportOptionsStruct		OrigOptions;
	Interface *						MaxInterface;
	HWND								Wnd;
	HWND								ParentWnd;
	HWND								PaneWnds[PANE_MAX];
	int								CurrentPane;
};
