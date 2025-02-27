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
 *                     $Archive:: /Commando/Code/Tools/max2w3d/animationcompressionsettings.h $*
 *                                                                                             *
 *              Original Author:: Patrick Smith																  *
 *                                                                                             *
 *                      $Author:: Patrick                                                     $*
 *                                                                                             *
 *                     $Modtime:: 10/30/00 1:57p                                              $*
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
//	AnimationCompressionSettingsDialogClass
//
////////////////////////////////////////////////////////////////////////////////////////
class AnimationCompressionSettingsDialogClass
{
public:

	//////////////////////////////////////////////////////////////////
	//	Public constructors/destructors
	//////////////////////////////////////////////////////////////////
	AnimationCompressionSettingsDialogClass (Interface *maxinterface, HWND parent_wnd = NULL);
	~AnimationCompressionSettingsDialogClass (void);


	//////////////////////////////////////////////////////////////////
	//	Public methods
	//////////////////////////////////////////////////////////////////

	void			Set_Options (W3dExportOptionsStruct *options)	{ Options = options; }
	int			Do_Modal (void);

private:

	//////////////////////////////////////////////////////////////////
	//	Static methods
	//////////////////////////////////////////////////////////////////
	static BOOL CALLBACK	Real_Message_Proc (HWND wnd, UINT message, WPARAM wparam, LPARAM lparam);

	//////////////////////////////////////////////////////////////////
	//	Private methods
	//////////////////////////////////////////////////////////////////
	BOOL			Message_Proc (UINT message, WPARAM wparam, LPARAM lparam);
	void			Initialize_Controls (void);
	void			Save_Settings (void);

	//////////////////////////////////////////////////////////////////
	//	Private member data
	//////////////////////////////////////////////////////////////////
	W3dExportOptionsStruct *	Options;
	Interface *						MaxInterface;
	HWND								Wnd;
	HWND								ParentWnd;
};
