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

/* $Header: /Commando/Code/Tools/max2w3d/meshcon.cpp 34    10/27/00 4:12p Greg_h $ */
/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando / G Math Library                                    * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Tools/max2w3d/meshcon.cpp                    $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 10/27/00 10:39a                                             $* 
 *                                                                                             * 
 *                    $Revision:: 34                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   MeshConnectionsClass::MeshConnectionsClass -- Constructor                                 * 
 *   MeshConnectionsClass::~MeshConnectionsClass -- Destructor                                 * 
 *   MeshConnectionsClass::Get_Aggregate_Data -- name and bone for the given aggregate         *
 *   MeshConnectionsClass::Get_Proxy_Data -- name and transform for the specified proxy object *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "meshcon.h"
#include "util.h"
#include "SnapPoints.h"
#include "w3dappdata.h"
#include "geometryexporttask.h"
#include "geometryexportcontext.h"


/*********************************************************************************************** 
 * MeshConnectionsClass::MeshConnectionsClass -- Constructor                                   * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   10/19/2000 gth : New version which uses the GeometryExportTasks                           *
 *=============================================================================================*/
MeshConnectionsClass::MeshConnectionsClass
(	
	DynamicVectorClass<GeometryExportTaskClass *> sub_object_list,
	GeometryExportContextClass & context 
) :
	CurTime(context.CurTime),
	Origin(context.Origin)
{
	unsigned int i;
	assert(Origin != NULL);

	/*
	** Set the name, count the sub-objects and aggregates
	*/
	Set_W3D_Name(Name,context.ModelName);
	
	/*
	** For each sub-object, record the bone it is attached to and its name
	*/
	int count = sub_object_list.Count();
	for (i=0; i<count; i++) {

		ConnectionStruct con;
		sub_object_list[i]->Get_Full_Name(con.ObjectName,sizeof(con.ObjectName));
		con.BoneIndex = sub_object_list[i]->Get_Bone_Index();
		con.MeshINode = sub_object_list[i]->Get_Object_Node();

		if (sub_object_list[i]->Is_Aggregate()) {
			Aggregates.Add(con);
		} else if (sub_object_list[i]->Is_Proxy()) {
			ProxyObjects.Add(con);
		} else {
			SubObjects.Add(con);
		}
	}
}


/*********************************************************************************************** 
 * MeshConnectionsClass::~MeshConnectionsClass -- Destructor                                   * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   07/24/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
MeshConnectionsClass::~MeshConnectionsClass(void)
{
}


/***********************************************************************************************
 * MeshConnectionsClass::Get_Sub_Object_Data -- Returns the name and bone index for a given obj*
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
bool MeshConnectionsClass::Get_Sub_Object_Data (int mesh_idx, char **out_name, int *out_boneindex,
														INode **out_inode)
{
	if (mesh_idx >= SubObjects.Count()) return false;

	if (out_name)
		*out_name = SubObjects[mesh_idx].ObjectName;
	if (out_boneindex)
		*out_boneindex = SubObjects[mesh_idx].BoneIndex;
	if (out_inode)
		*out_inode = SubObjects[mesh_idx].MeshINode;

	return true;
}


/***********************************************************************************************
 * MeshConnectionsClass::Get_Aggregate_Data -- name and bone for the given aggregate           *
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
bool MeshConnectionsClass::Get_Aggregate_Data(int mesh_idx, char **out_name, int *out_boneindex,
														INode **out_inode)
{
	if (mesh_idx >= Aggregates.Count()) return false;

	if (out_name)
		*out_name = Aggregates[mesh_idx].ObjectName;
	if (out_boneindex)
		*out_boneindex = Aggregates[mesh_idx].BoneIndex;
	if (out_inode)
		*out_inode = Aggregates[mesh_idx].MeshINode;

	return true;
}


/***********************************************************************************************
 * MeshConnectionsClass::Get_Proxy_Data -- name and transform for the specified proxy object   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/26/2000 gth : Created.                                                                 *
 *=============================================================================================*/
bool MeshConnectionsClass::Get_Proxy_Data(int index, char **out_name, int *out_boneindex,
														INode **out_inode)
{
	if (index >= ProxyObjects.Count()) return false;

	if (out_name)
		*out_name = ProxyObjects[index].ObjectName;
	if (out_boneindex)
		*out_boneindex = ProxyObjects[index].BoneIndex;
	if (out_inode)
		*out_inode = ProxyObjects[index].MeshINode;

	return true;
}
