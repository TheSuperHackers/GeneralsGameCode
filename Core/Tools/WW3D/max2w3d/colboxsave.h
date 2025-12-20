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
 *                     $Archive:: /Commando/Code/Tools/max2w3d/colboxsave.h                   $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 3/16/99 8:37a                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include <max.h>
#include "w3d_file.h"
#include "chunkio.h"
#include "PROGRESS.h"


/*******************************************************************************************
**
** CollisionBoxSaveClass - Create an AABox or an OBBox from a Max mesh (typically the
**	artist should use a 'box' to generate this. In any case, we're just using the bounding
** box).
**
*******************************************************************************************/
class CollisionBoxSaveClass
{
public:

	enum {
		EX_UNKNOWN = 0,	// exception error codes
		EX_CANCEL = 1
	};

	CollisionBoxSaveClass(	char *						mesh_name,
									char *						container_name,
									INode *						inode,
									Matrix3 &					exportspace,
									TimeValue					curtime,
									Progress_Meter_Class &	meter);

	int Write_To_File(ChunkSaveClass & csave);

private:

	W3dBoxStruct						BoxData;				// contains same information as the W3dOBBoxStruct

};
