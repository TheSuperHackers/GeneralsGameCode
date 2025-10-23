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

/* $Header: /Commando/Code/Tools/max2w3d/meshsave.h 44    10/30/00 1:12p Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando / G                                                 *
 *                                                                                             *
 *                    File Name : MESHSAVE.H                                                   *
 *                                                                                             *
 *                   Programmer : Greg Hjelstrom                                               *
 *                                                                                             *
 *                   Start Date : 06/10/97                                                     *
 *                                                                                             *
 *                  Last Update : June 10, 1997 [GH]                                           *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "rawfile.h"	// have to include this before Max.h
#include <max.h>
#include "BITTYPE.H"
#include "w3d_file.h"
#include "chunkio.h"
#include "PROGRESS.H"
#include "nodelist.h"
#include "util.h"
#include "w3dmtl.h"
#include "meshbuild.h"
#include "MeshDeformSave.h"
#include "w3dappdata.h"

class HierarchySaveClass;
class MeshConnectionsClass;
class MeshSaveClass;
class SkinDataClass;


/*******************************************************************************************
**
** VertStruct
**
*******************************************************************************************/
struct VertStruct
{
	Point3					Vertex;
	Point3					Normal;
	Point2					TexCoord;
	Color						Color;

	uint32					MaxVertIdx;			// index of the MAX vertex that this vert came from
	uint32					MaxFaceIdx;			// index of the MAX face that this vert came from
	VertStruct *			Next;					// used by the hash table...
};

/*******************************************************************************************
**
** FaceStruct
**
*******************************************************************************************/
struct FaceStruct
{
	uint32					MaxVidx[3];			// original 3ds-MAX vertex index (for smoothing computations)
	uint32					OurVidx[3];			// vertex, vertex normal, and texture coord indices
	uint32					MaterialIdx; 		// material index
	uint32					SmGroup;				// smoothing group (not really needed, normals pre-calced)
	Point3					Normal;		 		// Face normal
	float32					Dist;			 		// Plane distance
	uint32					Attributes;			// collision flags, sort method, etc
};

/*******************************************************************************************
**
** MeshSaveClass - this is the big one, create meshes and skins from a MAX mesh.
**
*******************************************************************************************/
class MeshSaveClass
{
public:

	enum {
		EX_UNKNOWN = 0,	// exception error codes
		EX_CANCEL = 1
	};

	MeshSaveClass(
			char *						mesh_name,
			char *						container_name,
			INode *						inode,
			const Mesh *				input_mesh,
			Matrix3 &					exportspace,
			W3DAppData2Struct	&		exportoptions,
			HierarchySaveClass *		htree,
			TimeValue					curtime,
			Progress_Meter_Class &	meter,
			unsigned int *				materialColors,
			int							&numMaterialColors,
			int							&numHouseColors,
			char *						materialColorTexture,
			WorldInfoClass *			world_info = NULL
			);

	~MeshSaveClass(void);

	int Write_To_File(ChunkSaveClass & csave,bool export_aabtrees = false);

private:

	INode *								MaxINode;
	W3DAppData2Struct &				ExportOptions;

	W3dMeshHeader3Struct				Header;
	W3dMaterialDescClass 			MaterialDesc;
	MeshBuilderClass					Builder;
	MeshDeformSaveClass				DeformSave;
	TimeValue							CurTime;
	Matrix3								ExportSpace;
	Matrix3								MeshToExportSpace;
	Matrix3								PivotSpace;
	HierarchySaveClass *				HTree;
	char *								UserText;
	bool									MeshInverted;				// this flag indicates that the transform is inverting this mesh
	W3dVertInfStruct *				VertInfluences;

	int *									MaterialRemapTable;		// reindexes mtl_idx after un-used mtls are removed

	// Flag set if the mesh uses a PS2 material.
	int									PS2Material;

private:

	// Use a MeshBuilderClass to process the mesh
	void Build_Mesh(Mesh & mesh, Mtl *node_mtl, unsigned int *materialColors, int &numMaterialColors, int &numHouseColors);

	// compute properties for the mesh
	void compute_bounding_volumes(void);
	void compute_physical_constants(INode * inode,Progress_Meter_Class & meter,bool voxelize);

	// create the materials
	int  scan_used_materials(Mesh & mesh, Mtl * nodemtl);
	void create_materials(Mtl * nodemtl,DWORD wirecolor, char *materialColorTexture);
	void fix_diffuse_materials(bool isHouseColor);

	// count number of used solid materials (no texture)
	int getNumSolidMaterials(Mtl * nodemtl);

	// creating damage stages
	void add_damage_stage(MeshSaveClass * damage_mesh);

	// methods used in building the wtm file
	int write_header(ChunkSaveClass & csave);
	int write_user_text(ChunkSaveClass & csave);

	int write_verts(ChunkSaveClass & csave);
	int write_vert_normals(ChunkSaveClass & csave);
	int write_vert_influences(ChunkSaveClass & csave);
	int write_vert_shade_indices(ChunkSaveClass & csave);

	int write_triangles(ChunkSaveClass & csave);

	int write_material_info(ChunkSaveClass & csave);
	int write_shaders(ChunkSaveClass & csave);
	int write_vertex_materials(ChunkSaveClass & csave);
	int write_textures(ChunkSaveClass & csave);

	int write_pass(ChunkSaveClass & csave,int pass);
	int write_vertex_material_ids(ChunkSaveClass & csave,int pass);
	int write_shader_ids(ChunkSaveClass & csave,int pass);
	int write_dcg(ChunkSaveClass & csave,int pass);

	int write_texture_stage(ChunkSaveClass & csave,int pass,int stage);
	int write_texture_ids(ChunkSaveClass & csave,int pass,int stage);
	int write_texture_coords(ChunkSaveClass & csave,int pass,int stage);

	int write_aabtree(ChunkSaveClass & csave);

	// transforms mesh so that it uses the desired coordinate system
	void prep_mesh(Mesh & mesh,Matrix3 & objoff);

	// inverse deform the mesh so that its ready to be used as a skin!
	void get_skin_modifier_objects(SkinDataClass ** skin_data_ptr,SkinWSMObjectClass ** skin_obj_ptr);
	int  get_htree_bone_index_for_inode(INode * node);
	void inv_deform_mesh(void);

	// get rendering settings for the materials
	void customize_materials(void);

	// Write the ps2 shaders and approximate them as close as possible in the W3D shaders.
	int write_ps2_shaders(ChunkSaveClass & csave);

	// Make the PC shader emulate the PS2 shader.
	void setup_PC_shaders_from_PS2_shaders();

	friend class DamageClass;
};
