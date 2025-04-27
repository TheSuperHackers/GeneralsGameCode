/*
**	Command & Conquer Renegade(tm)
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
 *                 Project Name : WW3DView                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/W3DView/Vector3RndCombo.cpp                                                                                                                                                                                                                                                                                                                              $Modtime::                                                             $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "StdAfx.H"
#include "Vector3RndCombo.H"
#include "V3_Rnd.H"

const char * const RANDOMIZER_NAMES[Vector3Randomizer::CLASSID_MAXKNOWN] = 
{
	"Solid Box",
	"Solid Sphere",
	"Hollow Sphere",
	"Solid Cylinder",
};


////////////////////////////////////////////////////////////////////
//
//	Fill_Vector3_Rnd_Combo
//
////////////////////////////////////////////////////////////////////
void
Fill_Vector3_Rnd_Combo (HWND hcombobox)
{
	ASSERT (Vector3Randomizer::CLASSID_MAXKNOWN == (sizeof (RANDOMIZER_NAMES) / sizeof (char *)));

	//
	//	Add all the strings to the combobox
	//
	for (int index = 0; index < Vector3Randomizer::CLASSID_MAXKNOWN; index ++) {
		::SendMessage (hcombobox, CB_ADDSTRING, 0, (LPARAM)RANDOMIZER_NAMES[index]);
	}

	return ;
}


////////////////////////////////////////////////////////////////////
//
//	Combo_Index_From_Vector3_Rnd
//
////////////////////////////////////////////////////////////////////
int
Combo_Index_From_Vector3_Rnd (Vector3Randomizer *randomizer)
{
	int index = 0;
	if (randomizer != NULL) {		
		index = (int)randomizer->Class_ID ();
	}

	// Return the index to the caller
	return index;
}


////////////////////////////////////////////////////////////////////
//
//	Vector3_Rnd_From_Combo_Index
//
////////////////////////////////////////////////////////////////////
Vector3Randomizer *
Vector3_Rnd_From_Combo_Index (int index, float value1, float value2, float value3)
{
	Vector3Randomizer *randomizer = NULL;

	//
	//	What type of randomizer should we create?
	//
	switch (index)
	{
		case Vector3Randomizer::CLASSID_SOLIDBOX:
			randomizer = new Vector3SolidBoxRandomizer (Vector3 (value1, value2, value3));
			break;

		case Vector3Randomizer::CLASSID_SOLIDSPHERE:
			randomizer = new Vector3SolidSphereRandomizer (value1);
			break;

		case Vector3Randomizer::CLASSID_HOLLOWSPHERE:
			randomizer = new Vector3HollowSphereRandomizer (value1);
			break;

		case Vector3Randomizer::CLASSID_SOLIDCYLINDER:
			randomizer = new Vector3SolidCylinderRandomizer (value1, value2);
			break;
	}

	// Return a pointer to the new randomizer
	return randomizer;
}
