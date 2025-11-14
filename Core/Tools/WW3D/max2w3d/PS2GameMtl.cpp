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
 *                    File name : PS2GameMtl.cpp                                               *
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

#include "gamemtl.h"
#include <max.h>
#include <gport.h>
#include <hsv.h>
#include "dllmain.h"
#include "resource.h"
#include "util.h"



/*****************************************************************
*
*		PS2 GameMtl Class Descriptor
*
*****************************************************************/
Class_ID PS2GameMaterialClassID(0x2ed62ad7, 0x50571dfd);

// This adds W3D PS2 choice to the Max material selector.
class PS2GameMaterialClassDesc:public ClassDesc {

public:
	int				IsPublic()					{ return 1; }
	void *			Create(BOOL loading)
	{
		GameMtl *mtl = new GameMtl(loading);
		mtl->Set_Shader_Type(GameMtl::STE_PS2_SHADER);
		return ((void*)mtl);
	}
	const TCHAR *	ClassName()					{ return Get_String(IDS_PS2_GAMEMTL); }
	SClass_ID		SuperClassID()				{ return MATERIAL_CLASS_ID; }
	Class_ID 		ClassID()					{ return PS2GameMaterialClassID; }
	const TCHAR* 	Category()					{ return _T("");  }
};

static PS2GameMaterialClassDesc _PS2GameMaterialCD;

ClassDesc * Get_PS2_Game_Material_Desc() { return &_PS2GameMaterialCD;  }
