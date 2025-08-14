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
 *                 Project Name : Max2W3D                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/gennamesdialog.h               $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 12/06/00 3:25p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include <max.h>
#include "w3d_file.h"


/**********************************************************************************************
**
** GenNamesDialogClass - Dialog box for the node naming parameters
**
**********************************************************************************************/
class GenNamesDialogClass
{
public:

	GenNamesDialogClass(Interface * maxinterface);
	~GenNamesDialogClass();

	struct OptionsStruct
	{
		OptionsStruct(void) : OnlyAffectSelected(false), NameIndex(0), AssignCollisionBits(false)
		{
			memset(RootName,0,sizeof(RootName));
			memset(PrefixName,0,sizeof(PrefixName));
			memset(SuffixName,0,sizeof(SuffixName));
		}

		// overall options
		bool		AssignNames;
		bool		AssignPrefix;
		bool		AssignSuffix;
		bool		AssignCollisionBits;
		bool		OnlyAffectSelected;

		// name options
		char		RootName[W3D_NAME_LEN];
		char		PrefixName[W3D_NAME_LEN];
		char		SuffixName[W3D_NAME_LEN];
		int		NameIndex;

		// collision bit options
		bool		PhysicalCollision;
		bool		ProjectileCollision;
		bool		VisCollision;
		bool		CameraCollision;
		bool		VehicleCollision;
	};

	bool Get_Options(OptionsStruct * options);
	bool Dialog_Proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM);
	bool Ok_To_Exit(void);
	void Toggle_Collision_Bits_Assignment(void);
	void Toggle_Name_Assignment(void);

private:

	enum
	{
		MIN_NAME_INDEX				= 0,
		MAX_NAME_INDEX				= 999,
		INITIAL_NAME_INDEX		= 0,
		MAX_ROOT_NAME_LEN			= 10,
		MAX_PREFIX_LEN				= 3,
		MAX_SUFFIX_LEN				= 3,
	};

	HWND								Hwnd;

	OptionsStruct *				Options;
	Interface *						MaxInterface;
	ISpinnerControl *				NameIndexSpin;

	friend BOOL CALLBACK			_gen_names_dialog_proc(HWND Hwnd,UINT message,WPARAM wParam,LPARAM lParam);

};

