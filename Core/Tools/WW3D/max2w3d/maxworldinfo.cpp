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
 *                     $Archive:: /Commando/Code/Tools/max2w3d/maxworldinfo.cpp               $*
 *                                                                                             *
 *              Original Author:: Patrick Smith                                                *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 10/27/00 7:11p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "maxworldinfo.h"
#include "geometryexporttask.h"

/*
** Get_Shared_Vertex_Normal
** Loops through all the other meshes in the world and builds a vertex normal for
** the vertices that share the same space and are part of the same smoothing group.
*/
Vector3 MaxWorldInfoClass::Get_Shared_Vertex_Normal (Vector3 pos, int smgroup)
{
	Point3 normal(0,0,0);
	Point3 world_pos = ExportTrans * Point3(pos.X,pos.Y,pos.Z);

	//
	//	Loop through all the meshes in the world and see which ones contain
	// vertices that share the same space and are part of the same smoothing group.
	//
	for(unsigned int index = 0; index < MeshList.Count(); index ++) {
		GeometryExportTaskClass * task = MeshList[index];
		if (task != CurrentTask) {
			normal += task->Get_Shared_Vertex_Normal(world_pos,smgroup);
		}
	}

	return Vector3(normal.x,normal.y,normal.z);
}
