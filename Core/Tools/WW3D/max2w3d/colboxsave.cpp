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
 *                     $Archive:: /Commando/Code/Tools/max2w3d/colboxsave.cpp                 $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 12/06/00 4:06p                                              $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "colboxsave.h"
#include "w3d_file.h"
#include "util.h"
#include "w3dappdata.h"
#include "errclass.h"


CollisionBoxSaveClass::CollisionBoxSaveClass
(
	char * mesh_name,
	char * container_name,
	INode * inode,
	Matrix3 & exportspace,
	TimeValue curtime,
	Progress_Meter_Class & meter
)
{
	//////////////////////////////////////////////////////////////////////
	// wrestle the mesh out of 3dsMAX
	//////////////////////////////////////////////////////////////////////
	Object *       obj = inode->EvalWorldState(curtime).obj;
	TriObject *    tri = (TriObject *)obj->ConvertToType(curtime, triObjectClassID);
	Mesh           mesh = tri->mesh;
	DWORD				wirecolor = inode->GetWireColor();

	if (mesh.getNumVerts() == 0) {
		throw ErrorClass("Mesh %s has no vertices!\n",mesh_name);
	}

	//////////////////////////////////////////////////////////////////////
	// Generate the AABox or OBBox data.
	//////////////////////////////////////////////////////////////////////
	memset(&BoxData,0,sizeof(BoxData));

	BoxData.Version = W3D_BOX_CURRENT_VERSION;
	if ((container_name != nullptr) && (strlen(container_name) > 0)) {
		strcpy(BoxData.Name,container_name);
		strcat(BoxData.Name,".");
	}
	strcat(BoxData.Name,mesh_name);

	BoxData.Attributes = 0;
	if (Is_Collision_AABox(inode)) {
		BoxData.Attributes |= W3D_BOX_ATTRIBUTE_ALIGNED;
	} else {
		BoxData.Attributes |= W3D_BOX_ATTRIBUTE_ORIENTED;
	}
	if (Is_Physical_Collision(inode)) {
		BoxData.Attributes |= W3D_BOX_ATTRIBTUE_COLLISION_TYPE_PHYSICAL;
	}
	if (Is_Projectile_Collision(inode)) {
		BoxData.Attributes |= W3D_BOX_ATTRIBTUE_COLLISION_TYPE_PROJECTILE;
	}
	if (Is_Vis_Collision(inode)) {
		BoxData.Attributes |= W3D_BOX_ATTRIBTUE_COLLISION_TYPE_VIS;
	}
	if (Is_Camera_Collision(inode)) {
		BoxData.Attributes |= W3D_BOX_ATTRIBTUE_COLLISION_TYPE_CAMERA;
	}
	if (Is_Vehicle_Collision(inode)) {
		BoxData.Attributes |= W3D_BOX_ATTRIBTUE_COLLISION_TYPE_VEHICLE;
	}

	BoxData.Color.R = GetRValue(wirecolor);
	BoxData.Color.G = GetGValue(wirecolor);
	BoxData.Color.B = GetBValue(wirecolor);

	// if this is an axis-aligned box, then use the world coord system
	if (Is_Collision_AABox(inode)) {
		exportspace.NoRot();
	}

	// Transform the mesh into the desired coordinate system
	Matrix3 node_matrix = inode->GetObjectTM(curtime);
	Matrix3 offset_matrix = node_matrix * Inverse(exportspace);
	int ivert;

	for (ivert = 0; ivert < mesh.getNumVerts (); ++ivert) {
		mesh.verts[ivert] = mesh.verts[ivert] * offset_matrix;
	}

	// Find the center and extent of the box.
	Point3 min_point = mesh.verts[0];
	Point3 max_point = mesh.verts[1];

	for (ivert=0; ivert < mesh.getNumVerts(); ++ivert) {
		if (mesh.verts[ivert].x < min_point.x) min_point.x = mesh.verts[ivert].x;
		if (mesh.verts[ivert].y < min_point.y) min_point.y = mesh.verts[ivert].y;
		if (mesh.verts[ivert].z < min_point.z) min_point.z = mesh.verts[ivert].z;

		if (mesh.verts[ivert].x > max_point.x) max_point.x = mesh.verts[ivert].x;
		if (mesh.verts[ivert].y > max_point.y) max_point.y = mesh.verts[ivert].y;
		if (mesh.verts[ivert].z > max_point.z) max_point.z = mesh.verts[ivert].z;
	}

	Point3 center = (max_point + min_point) / 2.0f;
	Point3 extent = (max_point - min_point) / 2.0f;

	BoxData.Center.X = center.x;
	BoxData.Center.Y = center.y;
	BoxData.Center.Z = center.z;

	BoxData.Extent.X = extent.x;
	BoxData.Extent.Y = extent.y;
	BoxData.Extent.Z = extent.z;
}



int CollisionBoxSaveClass::Write_To_File(ChunkSaveClass & csave)
{
	csave.Begin_Chunk(W3D_CHUNK_BOX);
	csave.Write(&BoxData,sizeof(BoxData));
	csave.End_Chunk();
	return 0;
}



