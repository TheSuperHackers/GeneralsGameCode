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
 *                     $Archive:: /Commando/Code/Tools/max2w3d/AppData.cpp                    $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 9/26/00 4:24p                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


/*
** AppData.cpp - Implementation of some Westwood
** extensions to the MAXScript language.
*/

#include <maxscrpt.h>	// Main MAXScript header
#include <maxobj.h>		// MAX* Wrapper objects
#include <strings.h>		// MAX String class
#include <definsfn.h>	// def_* functions to create static function headers

#include "w3dutil.h"	// W3DAppData*Struct accessor functions!
#include "w3ddesc.h"


/*
** Let MAXScript know we're implementing new built-in functions.
*/
def_visible_primitive(copy_app_data,			"wwCopyAppData");
def_visible_primitive(set_origin_app_data,	"wwSetOriginAppData");
def_visible_primitive(get_hierarchy_file,		"wwGetHierarchyFile");


/*
**
** MAXScript Function:
** wwCopyAppData - Usage: wwCopyAppData to_node from_node
**
** Copies all AppData associated with from_node to to_node.
** This is needed for W3D flags such as Export Geometry
** and Export Transform to get passed on to their
** instances/copies/references.
*/
Value * copy_app_data_cf (Value **arg_list, int count)
{
	// Verify the number and type of the arguments.
	check_arg_count("wwCopyAppData", 2, count);
	type_check(arg_list[0], MAXNode, "Target INode");
	type_check(arg_list[1], MAXNode, "Source INode");

	// Get the INode pointers that were passed in.
	INode *dest_node = arg_list[0]->to_node();
	INode *src_node = arg_list[1]->to_node();

	/*
	** Copy W3DAppData0Struct
	*/
	W3DAppData0Struct *app_data_0 = GetW3DAppData0(src_node);
	if (app_data_0 != NULL) {

		// App Data 0 is now obsolete, not fatal if we don't find one
		W3DAppData0Struct *copy_data_0 = new W3DAppData0Struct;
		if (copy_data_0 == NULL)
			throw RuntimeError("Out of memory.");

		// Copy the app data and give it to the target node.
		*copy_data_0 = *app_data_0;
		dest_node->AddAppDataChunk(W3DUtilityClassID, UTILITY_CLASS_ID, W3D_APPDATA_0,
											sizeof(W3DAppData0Struct), copy_data_0);
	}

	/*
	** Copy W3DAppData1Struct
	*/
	W3DAppData1Struct *app_data_1 = GetW3DAppData1(src_node);
	if (app_data_1 == NULL)
		throw RuntimeError("Unable to retrieve W3DAppData1Struct from object: ", arg_list[1]);

	W3DAppData1Struct *copy_data_1 = new W3DAppData1Struct;
	if (copy_data_1 == NULL)
		throw RuntimeError("Out of memory.");

	// Copy the app data and give it to the target node.
	*copy_data_1 = *app_data_1;
	dest_node->AddAppDataChunk(W3DUtilityClassID, UTILITY_CLASS_ID, W3D_APPDATA_1,
										sizeof(W3DAppData1Struct), copy_data_1);


	/*
	** Copy W3DAppData2Struct
	*/
	W3DAppData2Struct *app_data_2 = GetW3DAppData2(src_node);
	if (app_data_2 == NULL)
		throw RuntimeError("Unable to retrieve W3DAppData1Struct from object: ", arg_list[1]);

	W3DAppData2Struct *copy_data_2 = new W3DAppData2Struct;
	if (copy_data_2 == NULL)
		throw RuntimeError("Out of memory.");

	// Copy the app data and give it to the target node.
	*copy_data_2 = *app_data_2;
	dest_node->AddAppDataChunk(W3DUtilityClassID, UTILITY_CLASS_ID, W3D_APPDATA_2,
										sizeof(W3DAppData2Struct), copy_data_2);

	/*
	** Copy W3DDazzleAppDataStruct if one is present.
	*/
	W3DDazzleAppDataStruct *dazzle_app_data = GetW3DDazzleAppData(src_node);
	if (dazzle_app_data != NULL) {

		W3DDazzleAppDataStruct *copy_dazzle_data = new W3DDazzleAppDataStruct;
		if (copy_dazzle_data == NULL)
			throw RuntimeError("Out of memory.");


		// Copy the app data and give it to the target node.
		*copy_dazzle_data = *dazzle_app_data;
		dest_node->AddAppDataChunk(W3DUtilityClassID, UTILITY_CLASS_ID, W3D_DAZZLE_APPDATA,
											sizeof(W3DDazzleAppDataStruct), copy_dazzle_data);
	}
	
	return &ok;
}



/*
**
** MAXScript Function:
** wwSetOriginAppData - Usage: wwSetOriginAppData origin_node
**
** Sets the AppData associated with the given node to values
** appropriate to an origin. (ie. turn off Export Geometry and
** Export Transform)
*/
Value * set_origin_app_data_cf (Value **arg_list, int count)
{
	// Check the arguments that were passed to this function.
	check_arg_count("wwSetOriginAppData", 1, count);
	type_check(arg_list[0], MAXNode, "Origin INode");

	// Get the INode that we were given.
	INode *origin = arg_list[0]->to_node();

	// Get the node's W3DAppData2Struct, and modify it accordingly.
	W3DAppData2Struct *data = GetW3DAppData2(origin);
	if (data == NULL)
		throw RuntimeError("Unable to retrieve W3DAppData0Struct from object: ", arg_list[0]);

	// Turn off Export Geometry and Export Hierarchy.
	data->Enable_Export_Geometry(false);
	data->Enable_Export_Transform(false);

	return &ok;
}

/*
**
** MAXScript Function:
** wwGetHierarchyFile - Usage: wwGetHierarchyFile()
**
** Returns the relative pathname of the file that will be loaded
** during W3D export to get the hierarchy tree. If no such file
** will be loaded, the return value is the MAXScript undefined.
**
*/
Value * get_hierarchy_file_cf (Value **arg_list, int count)
{
	// Check that we weren't passed any arguments.
	check_arg_count("wwGetHierarchyFile", 0, count);

	// Retrieve the export options from the scene.
	W3dExportOptionsStruct *options = NULL;
	AppDataChunk * appdata = MAXScript_interface->GetScenePointer()->GetAppDataChunk(W3D_EXPORTER_CLASS_ID,SCENE_EXPORT_CLASS_ID,0);
	if (appdata)
		options = (W3dExportOptionsStruct*)(appdata->data);

	// If we didn't get any options, return undefined.
	if (!options)
		return &undefined;

	// Return the relative path to the htree file if it's available.
	// Otherwise, return the absolute path.
	one_typed_value_local(String* htree_file);
	if (options->RelativeHierarchyFilename[0] != 0)
		vl.htree_file = new String(options->RelativeHierarchyFilename);
	else if (options->HierarchyFilename[0] != 0)
		vl.htree_file = new String(options->HierarchyFilename);
	else
	{
		// Neither filename is available, return undefined.
		pop_value_locals();
		return &undefined;
	}
	return_value(vl.htree_file);
}




