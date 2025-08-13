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
 *                     $Archive:: /Commando/Code/Tools/max2w3d/floaterdialog.h                $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 10/11/00 11:23a                                             $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#ifndef FLOATERDIALOG_H
#define FLOATERDIALOG_H

#include <windows.h>

class Interface;

/**
** FloaterDialogClass
** This class is designed to be used by modeless dialog boxes.  See w3dutil.cpp for an
** example of how to embed an arbitrary dialog template and dialog proc into a floating
** window.
*/
class FloaterDialogClass
{
public:

	FloaterDialogClass(void);
	~FloaterDialogClass();

	bool	Is_Created(void);
	void	Create(Interface * ip, int child_dialog_id, DLGPROC child_dlg_proc);
	bool	Dialog_Proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM);

private:

	HWND		Hwnd;
	int		ChildDialogTemplateID;
	DLGPROC	ChildDialogProc;

};



#endif //FLOATERDIALOG_H

