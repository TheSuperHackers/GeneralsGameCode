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
 *                 Project Name : G                                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/SceneSetup.cpp                 $*
 *                                                                                             *
 *                      $Author:: Andre_a                                                     $*
 *                                                                                             *
 *                     $Modtime:: 10/15/99 3:33p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *  wwSceneSetup  -- Allows the user to select how many LOD and damage models to create.       *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


/*
** SceneSetup.cpp - Implements the "wwSceneSetup" MAXScript function to
** present a nice dialog to the user for getting a number of parameters
** that will governs the number, placement, and type of LOD and Damage
** models created in the scene.
*/


#include "SceneSetupDlg.h"

#undef STRICT
#include <maxscrpt.h>
#include <numbers.h>
#include <arrays.h>
#include <definsfn.h>


/*
** Let MAXScript know we're implementing new built-in functions.
*/
def_visible_primitive(scene_setup, "wwSceneSetup");


/***********************************************************************************************
 * scene_setup_cf - MAXScript function wwSceneSetup                                            *
 *                                                                                             *
 * wwSceneSetup - Usage: wwSceneSetup arg_array                                                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *	The contents of the given array is assumed to be as follows:                                *
 *		lod_count (int)			- the number of LOD models that will be created                  *
 *		lod_offset (float)		- X offset to move the LODs by                                   *
 *		lod_clone (int)			- 1==copy 2==instance 3==reference                               *
 *		damage_count (int)		- the number of damage models that will be created               *
 *		damage_offset (float)	- Y offset to move the LODs by                                   *
 *		damage_clone (int)		- 1==copy 2==instance 3==reference                               *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 * The function returns an array of the new values the user selected in the same format as     *
 * above.
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/27/1999  AJA : Created.                                                                 *
 *=============================================================================================*/
Value * scene_setup_cf (Value **arg_list, int count)
{
	// We don't want any arguments for this function.
	check_arg_count("wwSceneSetup", 1, count);
	type_check(arg_list[0], Array, "Parameter array");

	SceneSetupDlg	dlg(MAXScript_interface);
	one_typed_value_local(Array* result);

	// Read the initial values out of the array.
	Array *args = (Array*)(arg_list[0]);
	dlg.m_LodCount = (args->get(1))->to_int();
	dlg.m_LodOffset = (args->get(2))->to_float();
	dlg.m_LodProc = (args->get(3))->to_int();
	dlg.m_DamageCount = (args->get(4))->to_int();
	dlg.m_DamageOffset = (args->get(5))->to_float();
	dlg.m_DamageProc = (args->get(6))->to_int();

	// Display the dialog
	if (dlg.DoModal() == IDCANCEL)
	{
		pop_value_locals();
		return &undefined;
	}

	// Stuff the values the user chose into the array we'll return.
	vl.result = new Array(6);
	vl.result->append(Integer::intern(dlg.m_LodCount));
	vl.result->append(Float::intern(dlg.m_LodOffset));
	vl.result->append(Integer::intern(dlg.m_LodProc));
	vl.result->append(Integer::intern(dlg.m_DamageCount));
	vl.result->append(Float::intern(dlg.m_DamageOffset));
	vl.result->append(Integer::intern(dlg.m_DamageProc));

	// Return the array of new values.
	return_value(vl.result);
}


