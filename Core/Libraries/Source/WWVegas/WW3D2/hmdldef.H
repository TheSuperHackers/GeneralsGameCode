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
 *                 Project Name : WW3D                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/hmdldef.h                              $*
 *                                                                                             *
 *                       Author:: Greg_h                                                       *
 *                                                                                             *
 *                     $Modtime:: 1/08/01 10:04a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef HMDLDEF_H
#define HMDLDEF_H

#include "always.h"
#include "w3d_file.h"

class FileClass;
class ChunkLoadClass;
class ChunkSaveClass;
class SnapPointsClass;

/*
** Each render object to be attached into the hierarchy model
** will be defined by an instance of the following structure.
** The HmdlNodeDefStruct associates a named render object with
** an indexed pivot/bone in the hierarchy tree.
*/
struct HmdlNodeDefStruct
{
	char	RenderObjName[2*W3D_NAME_LEN];
	int	PivotID;
};


/*
**	HModelDefClass
**
**	"Hierarchy Model Definition Class"
**	This class serves as a blueprint for creating HierarchyModelClasses.  
**	The asset manager stores these objects internally and uses them to 
**	create instances of HierarchyModels for the user.
*/
class HModelDefClass
{

public:
	
	enum {
		OK,
		LOAD_ERROR
	};

	HModelDefClass(void);
	~HModelDefClass(void);

	int				Load_W3D(ChunkLoadClass & cload);
	const char *	Get_Name(void) const { return Name; }

private:

	char							Name[2*W3D_NAME_LEN];			// <basepose>.<modelname>
	char							ModelName[W3D_NAME_LEN];		// name of the model
	char							BasePoseName[W3D_NAME_LEN];	// name of the base pose (hierarchy tree)

	int							SubObjectCount;
	HmdlNodeDefStruct *		SubObjects;
	SnapPointsClass *			SnapPoints;

	void Free(void);
	bool read_header(ChunkLoadClass & cload);
	bool read_connection(ChunkLoadClass & cload,HmdlNodeDefStruct * con,bool pre30);
	bool read_mesh_connection(ChunkLoadClass & cload,int idx,bool pre30);
	bool read_collision_connection(ChunkLoadClass & cload,int idx,bool pre30);
	bool read_skin_connection(ChunkLoadClass & cload,int idx,bool pre30);

	// HModelClass requires intimate knowlege of us
	friend class HModelClass;
	friend class HLodClass;
};

#endif 
