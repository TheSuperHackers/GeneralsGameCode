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
 *                     $Archive:: /Commando/Code/Tools/max2w3d/genlodextensiondialog.h        $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 10/10/00 11:14a                                             $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef GENLODEXTENSIONDIALOG_H
#define GENLODEXTENSIONDIALOG_H

#include <windows.h>

class Interface;
class ISpinnerControl;

/**********************************************************************************************
**
** GenLodExtensionDialogClass - Dialog box for the LOD extension naming parameters
**
**********************************************************************************************/
class GenLodExtensionDialogClass
{
public:

	GenLodExtensionDialogClass(Interface * maxinterface);
	~GenLodExtensionDialogClass();
	
	struct OptionsStruct
	{
		OptionsStruct(void) : LodIndex(0)
		{ 
		}
		
		// name options
		int		LodIndex;
	};

	bool Get_Options(OptionsStruct * options);
	bool Dialog_Proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM);
		
private:

	enum 
	{
		MIN_LOD_INDEX				= 0,
		MAX_LOD_INDEX				= 99,
		INITIAL_LOD_INDEX			= 0,
	};

	HWND								Hwnd;

	OptionsStruct *				Options;
	Interface *						MaxInterface;
	ISpinnerControl *				LodIndexSpin;

	friend BOOL CALLBACK			_gen_lod_ext_dialog_proc(HWND Hwnd,UINT message,WPARAM wParam,LPARAM lParam);

};




#endif //GENLODEXTENSIONDIALOG_H