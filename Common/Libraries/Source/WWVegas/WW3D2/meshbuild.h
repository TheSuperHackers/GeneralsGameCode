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
 *                     $Archive:: /Commando/Code/ww3d2/meshbuild.h                            $*
 *                                                                                             *
 *                   Org Author:: Greg_h                                                       *
 *                                                                                             *
 *                       Author : Kenny Mitchell                                               * 
 *                                                                                             * 
 *                     $Modtime:: 06/27/02 9:23a                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 * 
 * 06/27/02 KM Shader system MAX plugin updates                                       *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef MESHBUILD_H
#define MESHBUILD_H

#include	"always.h"
#include "vector2.h"
#include "vector3.h"
#include "bittype.h"

#include <assert.h>

/*
** WorldInfoClass
**	Abstract base class that defines an interface for 'world information'.
** This class provides the mesh builder with information about the world
** outside of its mesh.
*/
class WorldInfoClass
{
	public:
		WorldInfoClass(void)				{ }
		virtual ~WorldInfoClass(void)	{ }

		// Public methods		
		virtual Vector3	Get_Shared_Vertex_Normal (Vector3 pos, int smgroup) = 0;
		virtual bool		Are_Meshes_Smoothed (void) const { return true; }
};

/*
** MeshBuilderClass
** This class will process meshes for you, splitting all vertices which do not
** share all parameters (such as texture coordinates), sort the polygons by
** material, and put them into strip order.
**
** To "build" a mesh:
** 1. Reset the builder with the number of polys you're going to sumbit
** 2. Enable the vertex channels that you want
** 3. Submit each face in the form of a FaceClass
** 4. Call Build_Mesh
**
** To use the results:
** 1. Call Get_Vertex_Count and Get_Face_Count to get the counts
** 2. Loop through the verts, looking at each one using Get_Vertex
** 3. Loop through the faces, looking at each one using Get_Face
**
** *NOTE*  This class is meant to be relatively self-sufficient.  It is used in a 
** variety of different applications which are built on completely different
** code-bases.  Do not introduce dependencies into this module lightly! :-)
*/
class MeshBuilderClass
{

public:

	enum {
		STATE_ACCEPTING_INPUT = 0,		// mesh builder is accepting input triangles
		STATE_MESH_PROCESSED,			// mesh builder has processed the mesh
	
		MAX_PASSES = 4,					// maximum number of material passes supported
		MAX_STAGES = 8,					// maximum number of texture stages supported in a single pass
	};

	/*
	** Constructor, Destructor
	*/
	MeshBuilderClass(int pass_count=1,int face_count_guess=255,int face_count_growth_rate=64);
	~MeshBuilderClass(void);

	/*
	** VertClass.  The MeshBuilder deals with vertices in this format.  
	*/
	class VertClass
	{
	public:
		VertClass(void)		{ Reset(); } 
		void						Reset(void);

	public:

		Vector3					Position;			// position of the vertex
		Vector3					Normal;				// vertex normal (can be calculated by mesh builder)
		int						SmGroup;				// smoothing group of the face this vertex was submitted with
		int						Id;					// id of the vertex, must match for vert to be welded, ok at zero if you don't care
		int						BoneIndex;			// bone influence if the mesh is a skin

		int						MaxVertColIndex;	// Index into the Max mesh.vertCol array of this vertex.
		
		Vector2					TexCoord[MAX_PASSES][MAX_STAGES];				
		Vector3					DiffuseColor[MAX_PASSES];			// diffuse color
		Vector3					SpecularColor[MAX_PASSES];			// specular color
		Vector3					DiffuseIllumination[MAX_PASSES];	// pre-calced diffuse illum
		float						Alpha[MAX_PASSES];					// alpha
		int						VertexMaterialIndex[MAX_PASSES];	// vertex material index

		int						Attribute0;			// user-set attributes (passed on through...)
		int						Attribute1;			// user-set attributes

		// These values are set up by the mesh builder:

		int						SharedSmGroup;		// smooth bits that were on in all faces that contributed to this final vertex
		int						UniqueIndex;		// used internally!
		int						ShadeIndex;			// used internally!
		VertClass *				NextHash;			// used internally!
	
	};

	/*
	** FaceClass.  The MeshBuilder deals faces in this format.  When inputing faces, set the
	** top half of the struct and the builder will fill in the bottom (vertex indices, normal,
	** and distance).
	*/
	class FaceClass
	{
	public:
		FaceClass(void)		{ Reset(); }
		void						Reset(void);									// reset this face
		
	public:
		VertClass				Verts[3];										// array of 3 verts
		int						SmGroup;											// smoothing group
		int						Index;											// user-set index of the face
		int						Attributes;										// user-set attributes
		int						TextureIndex[MAX_PASSES][MAX_STAGES];	// texture to use for each pass
		int						ShaderIndex[MAX_PASSES];					// shader for each pass
		uint32					SurfaceType;									// surface type identifier

		int						AddIndex;			// set by builder: index of addition
		int						VertIdx[3];			// set by builder: "optimized" vertex indices
		Vector3					Normal;		 		// set by builder: Face normal
		float32					Dist;			 		// set by builder: Plane distance
	
		void						Compute_Plane(void);
		bool						operator != (const FaceClass & that)		{ return !(*this == that); }
		bool						operator == (const FaceClass & /*that*/)	{ return false; }
		bool						Is_Degenerate(void);

		friend class MeshBuilderClass;
	};
	
	/*
	** To "build" a mesh:
	** 1. Reset the builder with the approximate number of polys you're going to sumbit, etc.
	** 3. Submit each face in the form of a FaceClass, set only the fields you need (leave others at default)
	** 4. Call Build_Mesh
	*/ 
	void							Reset(int pass_count,int face_count_guess,int face_count_growth_rate);
	int							Add_Face(const FaceClass & face);
	void							Build_Mesh(bool compute_normals);

	/*
	** Optional controls: 
	** If one of your passes has more textures than another, you may wish to do the
	** stripping and sorting based on that channel.  By default, everything will 
	** be stripped and sorted based on pass 0, stage 0
	** Sort_Vertices can be used to order the vertices arbitrarily.  I use it to
	** sort them according to the bone they are attached for skin meshes.
	*/
	void							Set_Polygon_Ordering_Channel(int pass,int texstage);
	
	/*
	** To use the results:
	** 1. Call Get_Vertex_Count and Get_Face_Count to get the counts
	** 2. Loop through the verts, looking at each one using Get_Vertex
	** 3. Loop through the faces, looking at each one using Get_Face
	*/
	int							Get_Pass_Count(void) const;
	int							Get_Vertex_Count(void) const;
	int							Get_Face_Count(void) const;
	const VertClass &			Get_Vertex(int index) const;
	const FaceClass &			Get_Face(int index) const;

	/*
	** Access to the Vertices and Faces for modifications.  This is used by
	** the max plugin when generating a skin mesh for example.  Be careful
	** what you do to them.  (I haven't thought through all of the
	** possible things you might do to mess up my nice clean mesh...).
	*/
	VertClass &					Get_Vertex(int index);
	FaceClass &					Get_Face(int index);

	/*
	** Bounding volume information about the mesh.  These functions can compute
	** various types of bounding volumes for the mesh you just processed...
	*/
	void							Compute_Bounding_Box(Vector3 * set_min,Vector3 * set_max);
	void							Compute_Bounding_Sphere(Vector3 * set_center,float * set_radius);

	// For per-pixel lighting KM
	void							Compute_Tangent_Basis();

	/*
	** World information managment.  Used to give the mesh builder information
	** about the world outside of its mesh.
	*/
	WorldInfoClass *			Peek_World_Info(void) const						{ return WorldInfo; }
	void							Set_World_Info(WorldInfoClass *world_info)	{ WorldInfo = world_info; }
	
	/*
	** Mesh Stats, mainly lots of flags for whether this mesh has various 
	** channels of information.
	*/
	struct MeshStatsStruct
	{
		void		Reset(void);

		bool		HasTexture[MAX_PASSES][MAX_STAGES];				// has at least one texture in given pass/stage
		bool		HasShader[MAX_PASSES];								// has at least one shader in given pass
		bool		HasVertexMaterial[MAX_PASSES];					// has at least one vert material in given pass

		bool		HasPerPolyTexture[MAX_PASSES][MAX_STAGES];	// has 2+ textures in given pass/stage
		bool		HasPerPolyShader[MAX_PASSES];						// has 2+ shaders in given pass 
		bool		HasPerVertexMaterial[MAX_PASSES];				// has 2+ vertex materials in given pass

		bool		HasDiffuseColor[MAX_PASSES];						// has diffuse colors in given pass
		bool		HasSpecularColor[MAX_PASSES];						// has specular colors in given pass
		bool		HasDiffuseIllumination[MAX_PASSES];				// has diffuse illum in given pass

		bool		HasTexCoords[MAX_PASSES][MAX_STAGES];			// has texture coords in given pass

		int		UVSplitCount;											// how many vertices were split due solely to UV discontinuities
		int		StripCount;												// number of strips that were created
		int		MaxStripLength;										// longest strip created
		float		AvgStripLength;										// average strip length
	};

	const MeshStatsStruct & Get_Mesh_Stats(void) const;

private:

	void							Free(void);
	void							Compute_Mesh_Stats(void);
	void							Optimize_Mesh(bool compute_normals);
	void							Strip_Optimize_Mesh(void);
	void							Remove_Degenerate_Faces(void);
	void							Compute_Face_Normals(void);
	bool							Verify_Face_Normals(void);
	void							Compute_Vertex_Normals(void);
	void							Grow_Face_Array(void);
	void							Sort_Vertices(void);

	/*
	** Winged edge stuff is used by the strip optimize function
	*/
	struct WingedEdgeStruct
	{
		int						MaterialIdx;
		WingedEdgeStruct *	Next;
		int						Vertex[2];
		int						Poly[2];
	};

	struct WingedEdgePolyStruct
	{
		WingedEdgeStruct *	Edge[3];
	};


	int							State;					// is the builder accepting input or already processed the mesh.
	int							PassCount;				// number of render passes for this mesh
	int							FaceCount;				// number of faces
	FaceClass *					Faces;					// array of faces
	int							InputVertCount;		// number of input verts;
	int							VertCount;				// number of verts;
	VertClass *					Verts;					// array of verts;
	int							CurFace;					// current face being added

	WorldInfoClass	*			WorldInfo;				// obj containing info about other meshes in the world

	MeshStatsStruct			Stats;					// internally useful junk about the mesh being processed.
	int							PolyOrderPass;			// order the polys using the texture indices in this pass
	int							PolyOrderStage;		// order the polys using the texture indices in this stage

	int							AllocFaceCount;		// number of faces allocated
	int							AllocFaceGrowth;		// growth rate of face array
};

inline void	MeshBuilderClass::Set_Polygon_Ordering_Channel(int pass,int texstage)
{
	assert(pass >= 0);
	assert(pass < MAX_PASSES);
	assert(texstage >= 0);
	assert(texstage < MAX_STAGES);

	PolyOrderPass = pass;
	PolyOrderStage = texstage;
}

inline int MeshBuilderClass::Get_Pass_Count(void) const
{
	return PassCount;
}

inline int MeshBuilderClass::Get_Vertex_Count(void) const
{
	assert(State == STATE_MESH_PROCESSED);
	return VertCount;
}

inline int MeshBuilderClass::Get_Face_Count(void) const
{
	assert(State == STATE_MESH_PROCESSED);
	return FaceCount;
}

inline const MeshBuilderClass::VertClass & MeshBuilderClass::Get_Vertex(int index) const
{
	assert(State == STATE_MESH_PROCESSED);
	assert(index >= 0);
	assert(index < VertCount);
	return Verts[index];	
}

inline const MeshBuilderClass::FaceClass & MeshBuilderClass::Get_Face(int index) const
{
	assert(State == STATE_MESH_PROCESSED);
	assert(index >= 0);
	assert(index < FaceCount);
	return Faces[index];
}

inline MeshBuilderClass::VertClass & MeshBuilderClass::Get_Vertex(int index)
{
	assert(State == STATE_MESH_PROCESSED);
	assert(index >= 0);
	assert(index < VertCount);
	return Verts[index];	
}

inline MeshBuilderClass::FaceClass & MeshBuilderClass::Get_Face(int index)
{
	assert(State == STATE_MESH_PROCESSED);
	assert(index >= 0);
	assert(index < FaceCount);
	return Faces[index];
}

inline const MeshBuilderClass::MeshStatsStruct & MeshBuilderClass::Get_Mesh_Stats(void) const
{
	assert(State == STATE_MESH_PROCESSED);
	return Stats;
}

#endif
