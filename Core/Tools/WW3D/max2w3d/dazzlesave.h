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
 *                     $Archive:: /Commando/Code/Tools/max2w3d/dazzlesave.h                   $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 9/21/00 4:09p                                               $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
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
** DazzleSaveClass - Create a Dazzle definition from an INode.  Basically, we just save
** the transform and the dazzle type that the user has selected.
**
*******************************************************************************************/
class DazzleSaveClass
{
public:

	enum {
		EX_UNKNOWN = 0,	// exception error codes
		EX_CANCEL = 1
	};

	DazzleSaveClass(		char *						mesh_name,
								char *						container_name,
								INode *						inode,
								Matrix3 &					exportspace,
								TimeValue					curtime,
								Progress_Meter_Class &	meter);

	int Write_To_File(ChunkSaveClass & csave);

private:

	char						W3DName[128];
	char						DazzleType[128];

};
