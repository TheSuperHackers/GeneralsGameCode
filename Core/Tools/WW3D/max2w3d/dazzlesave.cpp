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
 *                     $Archive:: /Commando/Code/Tools/max2w3d/dazzlesave.cpp                 $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 9/21/00 4:14p                                               $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "dazzlesave.h"
#include "w3d_file.h"
#include "util.h"
#include "w3dappdata.h"
#include "errclass.h"


DazzleSaveClass::DazzleSaveClass
(
	char * mesh_name,
	char * container_name,
	INode * inode,
	Matrix3 & exportspace,
	TimeValue curtime,
	Progress_Meter_Class & meter
)
{
	assert(mesh_name != NULL);
	assert(container_name != NULL);

	/*
	** Set up the render object name
	*/
	memset(&W3DName,0,sizeof(W3DName));
	if ((container_name != NULL) && (strlen(container_name) > 0)) {
		strcpy(W3DName,container_name);
		strcat(W3DName,".");
	}
	strcat(W3DName,mesh_name);

	/*
	** Dig the dazzle-type out of the appropriate App-Data chunk on
	** the INode.
	*/
	W3DDazzleAppDataStruct * dazzle_data = W3DDazzleAppDataStruct::Get_App_Data(inode);
	strncpy(DazzleType,dazzle_data->DazzleType,sizeof(DazzleType));
}



int DazzleSaveClass::Write_To_File(ChunkSaveClass & csave)
{
	csave.Begin_Chunk(W3D_CHUNK_DAZZLE);

	csave.Begin_Chunk(W3D_CHUNK_DAZZLE_NAME);
	csave.Write(W3DName,strlen(W3DName) + 1);
	csave.End_Chunk();

	csave.Begin_Chunk(W3D_CHUNK_DAZZLE_TYPENAME);
	csave.Write(DazzleType,strlen(DazzleType) + 1);
	csave.End_Chunk();

	csave.End_Chunk();
	return 0;
}



