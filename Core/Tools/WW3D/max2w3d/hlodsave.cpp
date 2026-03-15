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
 *                 Project Name : Renegade / G                                                 *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/hlodsave.cpp                   $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 11/07/00 5:24p                                              $*
 *                                                                                             *
 *                    $Revision:: 9                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *    -- The constructor builds the whole HLOD tree in a                                       *
 *    -- Destructor blows away the dynamic memory we used.                                     *
 *    -- Method called when saving to a W3D file. Saves the chunks                             *
 *    -- Write the header                                                                      *
 *    -- Writes each LOD                                                                       *
 *    -- Writes the mesh to bone connectivity info for each mesh in an LOD.                    *
 *   HLodSaveClass::save_aggregate_array -- save the aggregates (if any)                       *
 *   HLodSaveClass::save_proxy_array -- save the array of proxies (if any)                     *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "hlodsave.h"
#include "meshcon.h"
#include "errclass.h"
#include "util.h"
#include "w3dappdata.h"
#include "wwmath.h"	// NO_MAX_SCREEN_SIZE
#include "exportlog.h"



/* Behold, the applicable snippets of code from w3d_file.h that are applicable to this module!

	W3D_CHUNK_HLOD										=0x00000700,	// description of an HLod object (see HLodClass)
		W3D_CHUNK_HLOD_HEADER,											// general information such as name and version
		W3D_CHUNK_HLOD_LOD_ARRAY,										// wrapper around the array of objects for each level of detail
			W3D_CHUNK_HLOD_LOD_ARRAY_HEADER,							// info on the objects in this level of detail array
			W3D_CHUNK_HLOD_SUB_OBJECT,									// an object in this level of detail array

struct W3dHLodHeaderStruct
{
	uint32					Version;
	uint32					LodCount;
	char						Name[W3D_NAME_LEN];
	char						HierarchyName[W3D_NAME_LEN];		// name of the hierarchy tree to use (\0 if none)
};

struct W3dHLodArrayHeaderStruct
{
	uint32					ModelCount;
	float32					MaxScreenSize;		// if model is bigger than this, switch to higher lod.
};

struct W3dHLodSubObjectStruct
{
	uint32					BoneIndex;
	char						Name[W3D_NAME_LEN*2];
};
*/


/***********************************************************************************************
 * HLodSaveClass -- The constructor builds the whole HLOD tree in a form suitable for saving   *
 *  to a W3D file.                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/14/1999  AJA : Created.                                                                 *
 *=============================================================================================*/
HLodSaveClass::HLodSaveClass (MeshConnectionsClass **connections, int lod_count, TimeValue CurTime,
										char *name, const char *htree_name, Progress_Meter_Class &meter,
										INodeListClass *origin_list)
:	lod_array(nullptr)
{
	/*
	** Fill in the W3dHLodHeaderStruct
	*/
	header.Version = W3D_CURRENT_HLOD_VERSION;
	header.LodCount = lod_count;
	Set_W3D_Name(header.Name, name);
	Set_W3D_Name(header.HierarchyName, htree_name);
	ExportLog::printf("\nExporting HLOD object: %s\n",header.Name);
	ExportLog::printf(" lod count: %d\n",header.LodCount);

	/*
	** Create the array of stuff for each LOD.
	*/
	lod_array = new HLodArrayEntry[lod_count];
	if (!lod_array)
		throw ErrorClass("Out Of Memory!");

	int i;
	for (i = 0; i < lod_count; i++)
	{

		ExportLog::printf(" Exporting LOD Array %d\n",i);

		INode *origin = connections[i]->Get_Origin();
		int sub_obj_count = connections[i]->Get_Sub_Object_Count();
		lod_array[i].Allocate_Sub_Objects(sub_obj_count);
		lod_array[i].header.ModelCount = sub_obj_count;

		float screen_size = NO_MAX_SCREEN_SIZE;
		if (origin)
			origin->GetUserPropFloat("MaxScreenSize", screen_size);
		lod_array[i].header.MaxScreenSize = screen_size;

		/*
		** Create the info per mesh in this LOD.
		*/
		int j;
		W3dHLodSubObjectStruct *sub_obj = lod_array[i].sub_obj;
		ExportLog::printf(" sub-object count: %d\n",sub_obj_count);
		for (j = 0; j < sub_obj_count; j++)
		{
			char	*mesh_name;
			int	bone_index;
			INode	*mesh_node;
			if (!connections[i]->Get_Sub_Object_Data(j, &mesh_name, &bone_index, &mesh_node))
				throw ErrorClass("Model %s  is missing connection data!", name);

			strcpy(sub_obj[j].Name, mesh_name);
			sub_obj[j].BoneIndex = bone_index;

			ExportLog::printf("  Sub Object: %s Bone: %d\n",mesh_name,bone_index);
		}
	}

	/*
	** Copy aggregates from the Top-Level LOD
	*/
	int agg_count = connections[lod_count-1]->Get_Aggregate_Count();
	aggregate_array.Allocate_Sub_Objects(agg_count);
	aggregate_array.header.ModelCount = agg_count;
	aggregate_array.header.MaxScreenSize = 0.0f;

	ExportLog::printf(" Exporting Aggregates:\n");
	ExportLog::printf(" aggregate count: %d\n",agg_count);

	for (i=0; i<agg_count; i++) {

		char	*mesh_name;
		int	bone_index;
		INode	*mesh_node;
		connections[lod_count-1]->Get_Aggregate_Data(i, &mesh_name, &bone_index, &mesh_node);

		W3dHLodSubObjectStruct & sub_obj = aggregate_array.sub_obj[i];
		strcpy(sub_obj.Name, mesh_name);
		sub_obj.BoneIndex = bone_index;

		ExportLog::printf("  Aggregate object: %s Bone: %d\n",mesh_name,bone_index);

	}

	/*
	** Copy the proxy objects from the Top-Level LOD
	*/
	int proxy_count = connections[lod_count-1]->Get_Proxy_Count();
	proxy_array.Allocate_Sub_Objects(proxy_count);
	proxy_array.header.ModelCount = proxy_count;
	proxy_array.header.MaxScreenSize = 0.0f;

	ExportLog::printf(" Exporting Proxies\n");
	ExportLog::printf(" proxy count: %d\n",proxy_count);
	for (i=0; i<proxy_count; i++) {

		char	*mesh_name;
		int	bone_index;
		INode	*mesh_node;
		connections[lod_count-1]->Get_Proxy_Data(i, &mesh_name, &bone_index, &mesh_node);

		W3dHLodSubObjectStruct & sub_obj = proxy_array.sub_obj[i];
		strcpy(sub_obj.Name, mesh_name);
		sub_obj.BoneIndex = bone_index;

		ExportLog::printf("  Proxy object: %s Bone: %d\n",mesh_name,bone_index);
	}

}


/***********************************************************************************************
 * ~HLodSaveClass -- Destructor blows away the dynamic memory we used.                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/14/1999  AJA : Created.                                                                 *
 *=============================================================================================*/
HLodSaveClass::~HLodSaveClass (void)
{
	if (lod_array)
	{
		delete []lod_array;
		lod_array = nullptr;
	}
}


/***********************************************************************************************
 * HLodSaveClass::Save -- Method called when saving to a W3D file. Saves the chunks that       *
 *  define a HLOD.                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/14/1999  AJA : Created.                                                                 *
 *=============================================================================================*/
bool HLodSaveClass::Save(ChunkSaveClass &csave)
{
	if (!lod_array)
		return false;

	if (!csave.Begin_Chunk(W3D_CHUNK_HLOD))
		return false;

	if (!save_header(csave))
		return false;

	if (!save_lod_arrays(csave))
		return false;

	if (!save_aggregate_array(csave))
		return false;

	if (!save_proxy_array(csave))
		return false;

	if (!csave.End_Chunk())
		return false;

	return true;
}


/***********************************************************************************************
 * HLodSaveClass::save_header -- Write the header                                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/14/1999  AJA : Created.                                                                 *
 *=============================================================================================*/
bool HLodSaveClass::save_header (ChunkSaveClass &csave)
{
	if (!csave.Begin_Chunk(W3D_CHUNK_HLOD_HEADER))
		return false;

	if (csave.Write(&header, sizeof(header)) != sizeof(header))
		return false;

	if (!csave.End_Chunk())
		return false;

	return true;
}


/***********************************************************************************************
 * HLodSaveClass::save_lod_arrays -- Writes each LOD                                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/14/1999  AJA : Created.                                                                 *
 *=============================================================================================*/
bool HLodSaveClass::save_lod_arrays(ChunkSaveClass &csave)
{
	for (int i = 0; i < header.LodCount; i++)
	{
		if (!csave.Begin_Chunk(W3D_CHUNK_HLOD_LOD_ARRAY))
			return false;

		if (!save_sub_object_array(csave, lod_array[i]))
			return false;

		if (!csave.End_Chunk())
			return false;
	}

	return true;
}



/***********************************************************************************************
 * HLodSaveClass::save_aggregate_array -- save the aggregates (if any)                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/25/2000 gth : Created.                                                                 *
 *=============================================================================================*/
bool HLodSaveClass::save_aggregate_array(ChunkSaveClass & csave)
{
	if (aggregate_array.num_sub_objects > 0) {
		if (!csave.Begin_Chunk(W3D_CHUNK_HLOD_AGGREGATE_ARRAY))
			return false;

		if (!save_sub_object_array(csave, aggregate_array))
			return false;

		if (!csave.End_Chunk())
			return false;
	}
	return true;
}


/***********************************************************************************************
 * HLodSaveClass::save_proxy_array -- save the array of proxies (if any)                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/27/2000 gth : Created.                                                                 *
 *=============================================================================================*/
bool HLodSaveClass::save_proxy_array(ChunkSaveClass & csave)
{
	if (proxy_array.num_sub_objects > 0) {
		if (!csave.Begin_Chunk(W3D_CHUNK_HLOD_PROXY_ARRAY))
			return false;

		if (!save_sub_object_array(csave, proxy_array))
			return false;

		if (!csave.End_Chunk())
			return false;
	}
	return true;
}


/***********************************************************************************************
 * HLodSaveClass::save_sub_object_array -- Writes the mesh to bone connectivity info for each  *
 * mesh in an LOD.                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/14/1999  AJA : Created.                                                                 *
 *=============================================================================================*/
bool HLodSaveClass::save_sub_object_array(ChunkSaveClass &csave, const HLodArrayEntry & array)
{
	if (!csave.Begin_Chunk(W3D_CHUNK_HLOD_SUB_OBJECT_ARRAY_HEADER))
		return false;

	if (csave.Write(&(array.header), sizeof(array.header)) != sizeof(array.header))
		return false;

	if (!csave.End_Chunk())
		return false;

	for (int j = 0; j < array.num_sub_objects; j++)
	{
		if (!csave.Begin_Chunk(W3D_CHUNK_HLOD_SUB_OBJECT))
			return false;

		if (csave.Write(&(array.sub_obj[j]), sizeof(array.sub_obj[j])) != sizeof(array.sub_obj[j]))
			return false;

		if (!csave.End_Chunk())
			return false;
	}

	return true;
}


