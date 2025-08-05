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
 *                     $Archive:: /Commando/Code/Tools/max2w3d/genmtlnamesdialog.h            $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 10/10/00 11:12a                                             $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef GENMTLNAMESDIALOG_H
#define GENMTLNAMESDIALOG_H

#include <windows.h>

class Interface;
class ISpinnerControl;

/**********************************************************************************************
**
** GenMtlNamesDialogClass - Dialog box for the material naming parameters
**
**********************************************************************************************/
class GenMtlNamesDialogClass
{
public:

	GenMtlNamesDialogClass(Interface * maxinterface);
	~GenMtlNamesDialogClass();

	enum
	{
		MAX_MATERIAL_NAME_LEN	= 32,
		MIN_NAME_INDEX				= 0,
		MAX_NAME_INDEX				= 999,
		INITIAL_NAME_INDEX		= 0,
		MAX_ROOT_NAME_LEN			= 28,
	};

	struct OptionsStruct
	{
		OptionsStruct(void) : OnlyAffectSelected(true), NameIndex(0)
		{
			memset(RootName,0,sizeof(RootName));
		}

		// overall options
		bool		OnlyAffectSelected;

		// name options
		char		RootName[MAX_MATERIAL_NAME_LEN];
		int		NameIndex;
	};

	bool Get_Options(OptionsStruct * options);
	bool Dialog_Proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM);
	bool Ok_To_Exit(void);

private:

	HWND								Hwnd;

	OptionsStruct *				Options;
	Interface *						MaxInterface;
	ISpinnerControl *				NameIndexSpin;

	friend BOOL CALLBACK _gen_mtl_names_dialog_proc(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam);

};


#endif //GENMTLNAMESDIALOG_H


