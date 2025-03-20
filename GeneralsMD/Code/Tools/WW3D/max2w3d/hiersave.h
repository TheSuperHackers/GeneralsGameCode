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

/* $Header: /Commando/Code/Tools/max2w3d/hiersave.h 29    10/26/00 5:59p Greg_h $ */
/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando / G 3D Engine                                       * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Tools/max2w3d/hiersave.h                     $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 10/26/00 5:09p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 29                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef HIERSAVE_H
#define HIERSAVE_H

#include "always.h"

#include <max.h>
#include <stdio.h>

#ifndef W3D_FILE_H
#include "w3d_file.h"
#endif

#ifndef PROGRESS_H
#include "PROGRESS.H"
#endif

#ifndef CHUNKIO_H
#include "chunkio.h"
#endif

#ifndef NODELIST_H
#include "nodelist.h"
#endif

#ifndef VECTOR_H
#include "Vector.H"
#endif



struct HierarchyNodeStruct
{
	INode *					MaxNode;
	W3dPivotStruct			Pivot;
	W3dPivotFixupStruct	Fixup;

	bool operator == (const HierarchyNodeStruct & that) { return false; }
	bool operator != (const HierarchyNodeStruct & that) { return !(*this == that); }
};


class HierarchySaveClass
{

public:

	enum {
		MATRIX_FIXUP_NONE = 0,
		MATRIX_FIXUP_TRANS = 1,
		MATRIX_FIXUP_TRANS_ROT = 2
	};

	HierarchySaveClass();
	
	HierarchySaveClass(
					INode *						root,
					TimeValue					time,
					Progress_Meter_Class &	treemeter,
					char *						hname,
					int							fixup_type = MATRIX_FIXUP_NONE,
					HierarchySaveClass *		fixuptree = NULL);

	HierarchySaveClass(
					INodeListClass *			rootlist,
					TimeValue					time,
					Progress_Meter_Class &	treemeter,
					char *						hname,
					int							fixup_type = MATRIX_FIXUP_NONE,
					HierarchySaveClass *		fixuptree = NULL,
					const Matrix3 &			origin_offset = Matrix3(1));

	~HierarchySaveClass();

	bool				Save(ChunkSaveClass & csave);
	bool				Load(ChunkLoadClass & cload);
	int				Num_Nodes(void) const { return CurNode; }
	const char *	Get_Name(void) const;
	const char *	Get_Node_Name(int node) const;
	
	// get ahold of the max inode
	INode *			Get_Node(int node) const;

	// Returns the node's transform from object to world space
	Matrix3			Get_Node_Transform(int node) const;

	// Returns the node's transform relative to its parent
	Matrix3			Get_Node_Relative_Transform(int node) const { return get_relative_transform(node); }

	// Get the fixup matrix for the given pivot (always applied to the *relative* transform)
	Matrix3			Get_Fixup_Transform(int node) const;

	// Finds a node by name
	int				Find_Named_Node(const char * name) const;

	// Get the coordinate system to use when exporting the given INode.  Note that this
	// function takes into account the multiple skeletons present when exporting LOD models.
	void				Get_Export_Coordinate_System(INode * node,int * set_bone_index,INode ** set_bone_node,Matrix3 * set_transform);

	// Turning on terrian mode will cause all HTrees to force all normal meshes to be
	// attached to the RootTransform regardless of the status of their 'Export_Transform' flag
	static void		Enable_Terrain_Optimization(bool onoff)	{ TerrainModeEnabled = onoff; }

private:

	enum { MAX_PIVOTS = 4096, DEFAULT_NODE_ARRAY_SIZE = 512, NODE_ARRAY_GROWTH_SIZE = 32 };

	TimeValue				CurTime;									
	W3dHierarchyStruct	HierarchyHeader;
	DynamicVectorClass<HierarchyNodeStruct> Node;
	int						CurNode;
	int						FixupType;
	Matrix3					OriginOffsetTransform;				// this transform makes a node relative to the origin
	HierarchySaveClass *	FixupTree;

	static bool				TerrainModeEnabled;

	void	add_tree(INode * node,int pidx);
	int	add_node(INode * node,int pidx);

	bool	save_header(ChunkSaveClass & csave);
	bool	save_pivots(ChunkSaveClass & csave);
	bool	save_fixups(ChunkSaveClass & csave);

	bool	load_header(ChunkLoadClass & cload);
	bool	load_pivots(ChunkLoadClass & cload);
	bool	load_fixups(ChunkLoadClass & cload);

	Matrix3	get_relative_transform(int nodeidx) const;
	Matrix3	fixup_matrix(const Matrix3 & src) const;
	void	 	Free(void);
};

#endif /*HIERSAVE_H*/