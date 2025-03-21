/*
**	Command & Conquer Generals(tm)
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

/* $Header: /Commando/Code/Tools/max2w3d/w3dutil.h 29    10/26/00 5:59p Greg_h $ */
/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando Tools - W3D export                                  * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Tools/max2w3d/w3dutil.h                      $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 10/26/00 4:40p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 29                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#ifndef W3DUTIL_H
#define W3DUTIL_H

#include <max.h>
#include "utilapi.h"
#include "dllmain.h"
#include "resource.h"
#include "util.h"
#include "w3dappdata.h"

#define W3DUtilityClassID Class_ID(0x3c362c97, 0x5fc73ab0)

ClassDesc * Get_W3D_Utility_Desc(void);



/*
** W3dExportOptionsStruct - This structure is AppData that is attached
** not to an INode, but to the exporter class itself. It stores the
** export settings so that they come up the same next time, which
** facilitates batch exporting (just use the stored settings).
**
** WWScript.dlx uses this structure to tell if a scene depends on
** the HTree exported by another scene.
*/
struct W3dExportOptionsStruct
{
	bool		ExportHierarchy;
	bool		LoadHierarchy;
	bool		ExportAnimation;
	bool		ExportGeometry;	

	// Hierarchy Export options:
	bool		TranslationOnly;
	char		HierarchyFilename[_MAX_PATH];
	char		RelativeHierarchyFilename[_MAX_PATH];	// For storing in MAX file

	// Animation Export options:
	int		StartFrame;
	int		EndFrame;
	
	// Geometry Export options;
	bool		UseVoxelizer;

	// Option to apply smoothing between mesh boundaries
	bool		SmoothBetweenMeshes;

	int		space[10];		// blank space, so compression options default proper

	// More Animation Options
	bool		CompressAnimation;
	bool		ReduceAnimation;
	int		ReduceAnimationPercent;
	int		CompressAnimationFlavor;
	float		CompressAnimationTranslationError;
	float		CompressAnimationRotationError;
	bool		ReviewLog;

	// Option to prevent the exporter from exporting AABTrees with the meshes
	// Defined with the "inverse" sense so that older Max files default to having
	// AABTrees exported with their meshes.
	bool		DisableExportAABTrees;

	// Option to cause the exporter to optimize mesh data.  Defaulting to zero
	// causes older Max files to default to not messing with their mesh data.
	bool		EnableOptimizeMeshData;

	// Option to cause the exporter to ignore the Export_Transform setting for
	// all meshes.  Terrains should have all meshes exported in world space.
	bool		EnableTerrainMode;

	// Option to cause the exporter to generate textures from all materials using
	// only diffuse color (no textures).  All such material colors will be placed
	// into one texture page to improve batch rendering of models.
	bool		EnableMaterialColorToTextureConversion;
};




/*
** Functions to access the W3D AppData of any INode.
** An accessor function for each AppData we define is required.
** Our extensions to the MAXScript language (wwCopyAppData)
** uses these accessors.
*/
W3DAppData0Struct *			GetW3DAppData0 (INode *node);
W3DAppData1Struct *			GetW3DAppData1 (INode *node);
W3DAppData2Struct *			GetW3DAppData2 (INode *node);
W3DDazzleAppDataStruct *	GetW3DDazzleAppData(INode *node);

#endif
