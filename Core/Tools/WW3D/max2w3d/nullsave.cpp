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
 *                     $Archive:: /Commando/Code/Tools/max2w3d/nullsave.cpp                   $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 8/05/99 3:14p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "nullsave.h"


NullSaveClass::NullSaveClass
(
	char * mesh_name,
	char * container_name,
	Progress_Meter_Class & meter
)
{
	//////////////////////////////////////////////////////////////////////
	// Set up the NullObject description
	//////////////////////////////////////////////////////////////////////
	memset(&NullData,0,sizeof(NullData));

	NullData.Version = W3D_NULL_OBJECT_CURRENT_VERSION;
	if ((container_name != NULL) && (strlen(container_name) > 0)) {
		strcpy(NullData.Name,container_name);
		strcat(NullData.Name,".");
	}
	strcat(NullData.Name,mesh_name);
}



int NullSaveClass::Write_To_File(ChunkSaveClass & csave)
{
	csave.Begin_Chunk(W3D_CHUNK_NULL_OBJECT);
	csave.Write(&NullData,sizeof(NullData));
	csave.End_Chunk();
	return 0;
}



