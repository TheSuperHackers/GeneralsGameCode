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
 *                     $Archive:: /Commando/Code/Tools/max2w3d/ExportAll.cpp                  $*
 *                                                                                             *
 *                      $Author:: Andre_a                                                     $*
 *                                                                                             *
 *                     $Modtime:: 10/15/99 2:08p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *  wwExportTreeSettings  -- Returns the directory to export, and recursive flag.              *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


/*
** ExportAll.cpp - Implements wwExportTreeSettings, which presents the user with a dialog
** to allow them to choose which directory they want to export, and whether they want to
** export all subdirectories as well. These settings are then passed back so that a script
** can go through the directory (and maybe the subdirectories) and export all .max files
** it finds.
*/


#include "ExportAllDlg.h"

#undef STRICT
#include <maxscrpt.h>
#include <arrays.h>
#include <strings.h>
#include <definsfn.h>


/*
** Let MAXScript know we're implementing new built-in functions.
*/
def_visible_primitive(export_tree_settings, "wwExportTreeSettings");


/***********************************************************************************************
 * export_tree_settings_cf - Returns the directory to export, and recursive flag.              *
 *                                                                                             *
 * wwExportTreeSettings - Usage: wwExportTreeSettings #(<initial path>, <recursive>)           *
 *                                                                                             *
 * INPUT: A MaxScript array containing two values:                                             *
 *		initial_path (string)	- the initial export directory                                   *
 *		recursive (bool)			- the initial recursive setting                                  *
 *                                                                                             *
 * OUTPUT: An array of the same format containing the values the user chose.                   *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/4/1999  AJA : Created.                                                                 *
 *=============================================================================================*/
Value * export_tree_settings_cf (Value **arg_list, int count)
{
	// We want an array as an argument
	check_arg_count("wwExportAll", 1, count);
	type_check(arg_list[0], Array, "Parameter array");

	// Grab the two values out of the array.
	// First value is a string whose value is the initial value for the directory.
	// Second value is a bool, true for a recursive export.
	ExportAllDlg	dlg(MAXScript_interface);
	Array *args = (Array*)(arg_list[0]);
	char *temp = (args->get(1))->to_string();
	int len = strlen(temp);
	if (len < MAX_PATH)
		strcpy(dlg.m_Directory, temp);
	else
	{
		strncpy(dlg.m_Directory, temp, MAX_PATH-1);
		dlg.m_Directory[MAX_PATH-1] = 0;
	}
	dlg.m_Recursive = (args->get(2))->to_bool();

	// Show the dialog to let the user change the settings.
	if (dlg.DoModal() == IDCANCEL)
		return &undefined;

	// Create the array we will return to MaxScript.
	one_typed_value_local(Array *result);
	vl.result = new Array(2);
	vl.result->append(new String(dlg.m_Directory));
	if (dlg.m_Recursive)
		vl.result->append(&true_value);
	else
		vl.result->append(&false_value);

	// Return the new values.
	return_value(vl.result);
}