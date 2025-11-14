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
 *                     $Archive:: /Commando/Code/Tools/max2w3d/geometryexportcontext.h        $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 10/19/00 11:32a                                             $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include <max.h>

class ChunkSaveClass;
class MaxWorldInfoClass;
class HierarchySaveClass;
class INodeListClass;
class Progress_Meter_Class;
struct W3dExportOptionsStruct;


/**
** ExportContextClass
** This class encapsulates a bunch of datastructures needed during the geometry export
** process.
** NOTE: The user must plug in a valid ProgressMeter before each export operation.
*/
class GeometryExportContextClass
{
public:
	GeometryExportContextClass(	char * model_name,
											ChunkSaveClass & csave,
											MaxWorldInfoClass & world_info,
											W3dExportOptionsStruct & options,
											HierarchySaveClass * htree,
											INode * origin,
											INodeListClass * origin_list,
											TimeValue curtime,
											unsigned int *materialColors
										) :
		CSave(csave),
		WorldInfo(world_info),
		Options(options),
		CurTime(curtime),
		HTree(htree),
		OriginList(origin_list),
		Origin(origin),
		OriginTransform(1),
		ProgressMeter(NULL),
		materialColors(materialColors),
		numMaterialColors(0),
		numHouseColors(0),
		materialColorTexture(NULL)
	{
		ModelName = strdup(model_name);
		OriginTransform = Origin->GetNodeTM(CurTime);
	}

	~GeometryExportContextClass(void)
	{
		delete[] ModelName;
	}

	char *							ModelName;
	ChunkSaveClass &				CSave;
	MaxWorldInfoClass &			WorldInfo;
	W3dExportOptionsStruct &	Options;
	TimeValue						CurTime;
	HierarchySaveClass *			HTree;
	INodeListClass *				OriginList;

	INode *							Origin;
	Matrix3							OriginTransform;
	Progress_Meter_Class	*		ProgressMeter;
	unsigned int *					materialColors;	///MW: holds all used material colors.
	int								numMaterialColors;	///MW: number of used material colors.
	int								numHouseColors;		///MW: number of used house colors
	char	*						materialColorTexture; //MW: texture to hold material colors
};
