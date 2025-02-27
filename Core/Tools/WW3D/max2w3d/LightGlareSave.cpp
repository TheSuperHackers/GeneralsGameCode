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
 *                 Project Name : Max2W3D                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/LightGlareSave.cpp             $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 8/06/00 11:21a                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "LightGlareSave.h"
#include "w3d_file.h"
#include "util.h"
#include "w3dappdata.h"
#include "errclass.h"


LightGlareSaveClass::LightGlareSaveClass
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
	// Init the glare info
	//////////////////////////////////////////////////////////////////////
	memset(&GlareData,0,sizeof(GlareData));

	//////////////////////////////////////////////////////////////////////
	// Get the position of the pivot point relative to the given
	// export coordinate system.
	//////////////////////////////////////////////////////////////////////

	// Transform the mesh into the desired coordinate system
	Matrix3 node_matrix = inode->GetObjectTM(curtime);
	Matrix3 offset_matrix = node_matrix * Inverse(exportspace);

	GlareData.Position.X = offset_matrix.GetTrans().x;
	GlareData.Position.Y = offset_matrix.GetTrans().y;
	GlareData.Position.Z = offset_matrix.GetTrans().z;
}



int LightGlareSaveClass::Write_To_File(ChunkSaveClass & csave)
{
	csave.Begin_Chunk(W3D_CHUNK_LIGHTGLARE);
	csave.Begin_Chunk(W3D_CHUNK_LIGHTGLARE_INFO);
	csave.Write(&GlareData,sizeof(GlareData));
	csave.End_Chunk();
	csave.End_Chunk();
	return 0;
}



