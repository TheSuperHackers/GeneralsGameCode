/*
**	Command & Conquer Generals(tm)
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
 *                     $Archive:: /Commando/Code/Tools/max2w3d/Utility.cpp                    $*
 *                                                                                             *
 *                      $Author:: Andre_a                                                     $*
 *                                                                                             *
 *                     $Modtime:: 5/08/00 11:51a                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *  wwGetAbsolutePath  -- Returns the absolute path based on a relative one.                   *
 *  wwInputBox  -- Retrive a string from the user in a nice friendly manner.                   *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


/*
** Utility.cpp - Implementation of a some convenient features the WIN32 API provides
** that the MAXScript API doesn't.
*/


#include <maxscrpt.h>
#include <arrays.h>
#include <strings.h>
#include <definsfn.h>

#include "util.h"
#include "InputDlg.h"


/*
** Let MAXScript know we're implementing new built-in functions.
*/
def_visible_primitive(get_absolute_path,	"wwGetAbsolutePath");
def_visible_primitive(input_box,				"wwInputBox");


/***********************************************************************************************
 * get_absolute_path_cf -- Returns the absolute path based on a relative one.                  *
 *                                                                                             *
 * wwGetAbsolutePath - Usage: wwGetAbsolutePath <relative_path> <context>                      *
 *                                                                                             *
 * INPUT: A string containing a relative path, and its context.                                *
 *                                                                                             *
 * OUTPUT: The equivalent absolute path.                                                       *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/4/1999  AJA : Created.                                                                 *
 *=============================================================================================*/
Value * get_absolute_path_cf (Value **arg_list, int count)
{
	// We want an array as an argument
	check_arg_count("wwGetAbsolutePath", 2, count);
	type_check(arg_list[0], String, "Relative path");
	type_check(arg_list[1], String, "Context");

	// Grab the arguments out of the array.
	char absolute[MAX_PATH];
	char *relative = arg_list[0]->to_string();
	char *context  = arg_list[1]->to_string();

	// Turn the relative path into an absolute one.
	Create_Full_Path(absolute, context, relative);

	// Return the absolute path.
	one_typed_value_local(String *abs);
	vl.abs = new String(absolute);
	return_value(vl.abs);
}


/***********************************************************************************************
 * input_box_cf -- Retrive a string from the user in a nice friendly manner.                   *
 *                                                                                             *
 * wwInputBox - Usage: wwInputBox [caption:'caption'] [label:'label'] [value:'value']          *
 *                                                                                             *
 * INPUT: Three optional named arguments. If they are not given values, defaults are used.     *
 *                                                                                             *
 * OUTPUT: Returns the string entered by the user.                                             *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
Value * input_box_cf (Value **arg_list, int count)
{
	// Create the input box (but don't show it yet).
	InputDlg input_box(MAXScript_interface->GetMAXHWnd());
	Value *param = NULL;

	// Check the 'caption' parameter.
	param = key_arg(caption);
	if (param && param != &unsupplied)
		input_box.SetCaption(param->to_string());

	// Check the 'label' parameter.
	param = key_arg(label);
	if (param && param != &unsupplied)
		input_box.SetLabel(param->to_string());

	// Check the 'value' parameter.
	param = key_arg(value);
	if (param && param != &unsupplied)
		input_box.SetValue(param->to_string());

	// Show the dialog and let the user enter a value.
	if (input_box.DoModal() == IDCANCEL)
		return &undefined;

	// Return the value the user entered.
	one_typed_value_local(String *val);
	vl.val = new String(input_box.m_Value);
	return_value(vl.val);
}

