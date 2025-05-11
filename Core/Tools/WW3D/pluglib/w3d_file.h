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

/* $Header: /Commando/Code/ww3d/w3d_file.h 56    8/12/98 11:16a Greg_h $ */
/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando / G 3D Library                                      * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/ww3d/w3d_file.h                              $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 8/11/98 4:15p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 56                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#ifndef W3D_FILE_H
#define W3D_FILE_H

#include "always.h"

#ifndef BITTYPE_H
#include "BITTYPE.H"
#endif


/********************************************************************************

VERSION NUMBERS:

	Each Major chunk type will contain a "header" as its first
	sub-chunk.  The first member of this header will be a Version
	number formatted so that its major revision number is the
	high two bytes and its minor revision number is the lower two
	bytes.

Version 1.0:

	MESHES -  contained the following chunks:
		W3D_CHUNK_MESH_HEADER,			// header for a mesh
		W3D_CHUNK_VERTICES,				// array of vertices
		W3D_CHUNK_VERTEX_NORMALS,		// array of normals
		W3D_CHUNK_SURRENDER_NORMALS,	// array of surrender normals (one per vertex as req. by surrender)
		W3D_CHUNK_TEXCOORDS,				// array of texture coordinates
		W3D_CHUNK_MATERIALS,				// array of materials
		W3D_CHUNK_TRIANGLES,				// array of triangles
		W3D_CHUNK_SURRENDER_TRIANGLES,// array of surrender format tris	
		W3D_CHUNK_MESH_USER_TEXT,		// Name of owning hierarchy, text from the MAX comment field
		
	HIERARCHY TREES - contained the following chunks:
		W3D_CHUNK_HIERARCHY_HEADER,
		W3D_CHUNK_PIVOTS,
		W3D_CHUNK_PIVOT_FIXUPS,		

	HIERARCHY ANIMATIONS - contained the following chunks:  
		W3D_CHUNK_ANIMATION_HEADER,
		W3D_CHUNK_ANIMATION_CHANNEL,

	MESH CONNECTIONS - (blueprint for a hierarchical model) contained these chunks:

	 
Version 2.0:

	MESHES: 

	- Mesh header now contains the hierarchy model name.  The mesh name will be built
	as <HModelName>.<MeshName> instead of the old convention: <HTreeName>.<Meshname>

	- The material chunk is replaced with a new material structure which contains 
	some information for animating materials.
	
	- Vertex Influences link vertices of a mesh to bones in a hierarchy, this is 
	the information needed for skinning.  
	
	- Damage chunks added.  A damage chunk contains	a new set of materials, a set
	of vertex offsets, and a set of vertex colors.
	
	Added the following chunks:

		W3D_CHUNK_VERTEX_COLORS,		
		W3D_CHUNK_VERTEX_INFLUENCES,	
		W3D_CHUNK_DAMAGE,					
		W3D_CHUNK_DAMAGE_HEADER,
			W3D_CHUNK_DAMAGE_MATERIALS,
			W3D_CHUNK_DAMAGE_VERTICES,
			W3D_CHUNK_DAMAGE_COLORS,
		W3D_CHUNK_MATERIALS2,		

	MESH CONNECTIONS: Hierarchy models can now contain skins and collision meshes
	in addition to the normal meshes.  

		W3D_CHUNK_COLLISION_CONNECTION,		// collision meshes connected to the hierarchy
		W3D_CHUNK_SKIN_CONNECTION,				// skins connected to the hierarchy
		W3D_CHUNK_CONNECTION_AUX_DATA			// extension of the connection header


Dec 12, 1997

	Changed MESH_CONNECTIONS chunks into HMODEL chunks because the name
	mesh connections was becoming more and more inappropriate...  This was only
	a data structure naming change so no-one other than the coders are affected

	Added W3D_CHUNK_LODMODEL.  An LOD Model contains a set of names for
	render objects, each with a specified distance range.


Feb 6, 1998

	Added W3D_CHUNK_SECTMESH and its sub-chunks.  This will be the file
	format for the terrain geometry exported from POV's Atlas tool.  

March 29, 1998 : Version 3.0

	- New material chunk which supports the new features of the 3D engine
	- Modified HTrees to always have a root transform to remove all of the
	  special case -1 bone indexes.
	- Added new mesh types, A mesh can now be categorized as: normal, 
	  aligned, skin, collision, or shadow.

June 22, 1998

	Removed the "SECTMESH" chunks which were never implemented or used.

	Adding a new type of object: The 'Tilemap'.  This simple-sounding object
	is a binary partition tree of tiles where tiles are rectangular regions of
	space.  In each leaf to the tree, a mesh is referenced.  The tile map is 
	made of several chunks:
	
	- W3D_CHUNK_TILEMAP
		- W3D_CHUNK_TILEMAP_HEADER
		- W3D_CHUNK_TILES
			- W3D_CHUNK_MESH
			- W3D_CHUNK_MESH
			...

		- W3D_CHUNK_PARTITION_TREE
			- W3D_CHUNK_PARTITION_TREE_HEADER
			- W3D_CHUNK_PARTITION_TREE_NODES

		- W3D_CHUNK_TILE_INSTANCES
			- W3D_CHUNK_TILE_INSTANCE



********************************************************************************/


#define W3D_MAKE_VERSION(major,minor)		(((major) << 16) | (minor))
#define W3D_GET_MAJOR_VERSION(ver)			((ver)>>16)
#define W3D_GET_MINOR_VERSION(ver)			((ver) & 0xFFFF)

#define W3D_CURRENT_VERSION W3D_MAKE_VERSION(3,0)



/********************************************************************************

	CHUNK TYPES FOR ALL 3D DATA

	All 3d data is stored in chunks similar to an IFF file. Each
	chunk will be headed by an ID and size field.

	All structures defined in this header file are prefixed with
	W3d to prevent naming conflicts with in-game structures which
	may be slightly different than the on-disk structures.

********************************************************************************/

enum {

	W3D_CHUNK_MESH = 0,					// Mesh definition (.WTM file)
		W3D_CHUNK_MESH_HEADER,			// header for a mesh
		W3D_CHUNK_VERTICES,				// array of vertices
		W3D_CHUNK_VERTEX_NORMALS,		// array of normals
		W3D_CHUNK_SURRENDER_NORMALS,	// array of surrender normals (one per vertex as req. by surrender)
		W3D_CHUNK_TEXCOORDS,				// array of texture coordinates
		W3D_CHUNK_MATERIALS,				// array of materials
		O_W3D_CHUNK_TRIANGLES,			// array of triangles (obsolete)
		O_W3D_CHUNK_QUADRANGLES,		// array of quads (obsolete)
		W3D_CHUNK_SURRENDER_TRIANGLES,// array of surrender format tris	
		O_W3D_CHUNK_POV_TRIANGLES,		// POV format triangles (obsolete)
		O_W3D_CHUNK_POV_QUADRANGLES,	// POV format quads (obsolete)
		W3D_CHUNK_MESH_USER_TEXT,		// Name of owning hierarchy, text from the MAX comment field
		W3D_CHUNK_VERTEX_COLORS,		// Pre-set vertex coloring
		W3D_CHUNK_VERTEX_INFLUENCES,	// Mesh Deformation vertex connections
		W3D_CHUNK_DAMAGE,					// Mesh damage, new set of materials, vertex positions, vertex colors	
			W3D_CHUNK_DAMAGE_HEADER,		// Header for the damage data, tells what is coming
			W3D_CHUNK_DAMAGE_VERTICES,		// Array of modified vertices (W3dMeshDamageVertexStruct's)
			W3D_CHUNK_DAMAGE_COLORS,		// Array of modified vert colors (W3dMeshDamageColorStruct's)
			O_W3D_CHUNK_DAMAGE_MATERIALS,	// (OBSOLETE) Damage materials simply wrapped with MATERIALS3 or higher
			
		W3D_CHUNK_MATERIALS2,			// array of version 2 materials (with animation frame counts)
		
		W3D_CHUNK_MATERIALS3,			// array of version 3 materials (all new surrender features supported)
			W3D_CHUNK_MATERIAL3,					// Each version 3 material wrapped with this chunk ID
				W3D_CHUNK_MATERIAL3_NAME,				// Name of the material (array of chars, null terminated)
				W3D_CHUNK_MATERIAL3_INFO,				// contains a W3dMaterial3Struct, general material info
				W3D_CHUNK_MATERIAL3_DC_MAP,			// wraps the following two chunks, diffuse color texture
					W3D_CHUNK_MAP3_FILENAME,			// filename of the texture
					W3D_CHUNK_MAP3_INFO,					// a W3dMap3Struct
				W3D_CHUNK_MATERIAL3_DI_MAP,			// diffuse illimination map, same format as other maps
				W3D_CHUNK_MATERIAL3_SC_MAP,			// specular color map, same format as other maps
				W3D_CHUNK_MATERIAL3_SI_MAP,			// specular illumination map, same format as other maps

		W3D_CHUNK_MESH_HEADER3,					// New improved mesh header
		W3D_CHUNK_TRIANGLES,						// New improved triangles chunk
		W3D_CHUNK_PER_TRI_MATERIALS,			// Multi-Mtl meshes - An array of uint16 material id's

	W3D_CHUNK_HIERARCHY = 0x100,		// hierarchy tree definition (.WHT file)
		W3D_CHUNK_HIERARCHY_HEADER,
		W3D_CHUNK_PIVOTS,
		W3D_CHUNK_PIVOT_FIXUPS,			// only needed by the exporter...

	W3D_CHUNK_ANIMATION = 0x200,		// hierarchy animation data (.WHA file)
		W3D_CHUNK_ANIMATION_HEADER,
		W3D_CHUNK_ANIMATION_CHANNEL,
		W3D_CHUNK_BIT_CHANNEL,			// channel of boolean values (e.g. visibility)

	W3D_CHUNK_HMODEL = 0x300,			// blueprint for a hierarchy model
		W3D_CHUNK_HMODEL_HEADER,		// usually found at end of a .WTM file
		W3D_CHUNK_NODE,					// render objects connected to the hierarchy
		W3D_CHUNK_COLLISION_NODE,		// collision meshes connected to the hierarchy
		W3D_CHUNK_SKIN_NODE,				// skins connected to the hierarchy
		W3D_CHUNK_HMODEL_AUX_DATA,		// extension of the connection header
		W3D_CHUNK_SHADOW_NODE,			// shadow object connected to the hierarchy

	W3D_CHUNK_LODMODEL = 0x400,		// blueprint for an LOD model.  This is simply a
		W3D_CHUNK_LODMODEL_HEADER,		// collection of 'n' render objects, ordered in terms
		W3D_CHUNK_LOD,						// of their expected rendering costs.

	W3D_CHUNK_TILEMAP = 0x600,		// Tile Map definition.
		W3D_CHUNK_TILEMAP_NAME,
		W3D_CHUNK_TILEMAP_HEADER,
		W3D_CHUNK_TILEMAP_TILE_INSTANCES,
		W3D_CHUNK_TILEMAP_PARTITION_TREE,
			W3D_CHUNK_TILEMAP_PARTITION_NODE,

};


struct W3dChunkHeader
{
	uint32		ChunkType;			// Type of chunk (see above enumeration)
	uint32		ChunkSize;			// Size of the chunk, (not including the chunk header)
};


/********************************************************************************

	WTM ( Westwood Triangle Mesh )
		
	Each mesh will be contained within a WTM_CHUNK_MESH within
	this chunk will be the following chunks:

	The header will be the first chunk and it tells general 
	information about the mesh such as how many triangles there
	are, how many vertices, the bounding box, center
	of mass, inertia matrix, etc.  

	The vertex array is an array of Vectors giving the object 
	space location of each vertex

	The normal array is an array of all of the unique vertex
	normal vectors needed by the mesh.  This allows for vertices
	with multiple normals so that we can duplicate the effect
	of the smoothing groups in 3dsMax.

	The surrender normal array is an array of vertex normals which
	correspond 1to1 with the vertices.  This is because the current
	version of surrender can only handle one vertex normal per vertex.
	In this case, the application should skip the normal array chunk
	and read the surrender normal chunk into its SR_VERTs
	
	The texture coord array is all of the unique texture coordinates
	for the mesh.  This allows triangles to share vertices but not
	necessarily share texture coordinates.

	The material array is a list of the names and rgb colors of
	all of the unique materials used by the mesh.  All triangles will
	have a material index into this list.

	The triangle array is all of the triangles which make up the 
	mesh.  Each triangle has 3 indices to its vertices, vertex normals,
	and texture coordinates.  Each also has a material id and the
	coefficients for its plane equation.

	The Surrender Triangle array is all of the triangles in a slightly 
	different format.  Surrender triangles contain their u-v coordinates
	so there is no indirection and no possibility for sharing.  To
	make the importer faster, the triangles will also be stored in this
	format.  The application can read whichever chunk it wants to.

	The mesh user text chunk is a NULL-terminated text buffer.

********************************************************************************/

#define W3D_NAME_LEN	16

/////////////////////////////////////////////////////////////////////////////////////////////
// vector
/////////////////////////////////////////////////////////////////////////////////////////////
struct W3dVectorStruct
{
	float32		X;							// X,Y,Z coordinates
	float32		Y;
	float32		Z;
};

/////////////////////////////////////////////////////////////////////////////////////////////
// quaternion
/////////////////////////////////////////////////////////////////////////////////////////////
struct W3dQuaternionStruct
{
	float32		Q[4];
};

/////////////////////////////////////////////////////////////////////////////////////////////
// texture coordinate
/////////////////////////////////////////////////////////////////////////////////////////////
struct W3dTexCoordStruct
{
	float32		U;							// U,V coordinates
	float32		V;
};

/////////////////////////////////////////////////////////////////////////////////////////////
// rgb color, one byte per channel, padded to an even 4 bytes
/////////////////////////////////////////////////////////////////////////////////////////////
struct W3dRGBStruct
{
	uint8			R;
	uint8			G;
	uint8			B;
	uint8			pad;
};

/////////////////////////////////////////////////////////////////////////////////////////////
// Version 1.0 Material, array of these are found inside the W3D_CHUNK_MATERIALS chunk.
/////////////////////////////////////////////////////////////////////////////////////////////
struct W3dMaterialStruct
{
	char		  	MaterialName[W3D_NAME_LEN];	// name of the material (NULL terminated)
	char	 		PrimaryName[W3D_NAME_LEN];		// primary texture name (NULL terminated)
	char	 		SecondaryName[W3D_NAME_LEN];	// secondary texture name (NULL terminated)
	uint32		RenderFlags;						// Rendering flags
	uint8	 		Red;									// Rgb colors
	uint8	 		Green;
	uint8	 		Blue;
};

/////////////////////////////////////////////////////////////////////////////////////////////
// Version 2.0 Material, array of these are found inside the W3D_CHUNK_MATERIALS2 chunk.
/////////////////////////////////////////////////////////////////////////////////////////////
struct W3dMaterial2Struct
{
	char		  	MaterialName[W3D_NAME_LEN];	// name of the material (NULL terminated)
	char	 		PrimaryName[W3D_NAME_LEN];		// primary texture name (NULL terminated)
	char	 		SecondaryName[W3D_NAME_LEN];	// secondary texture name (NULL terminated)
	uint32		RenderFlags;						// Rendering flags
	uint8	 		Red;									// Rgb colors
	uint8	 		Green;
	uint8	 		Blue;
	uint8			Alpha;								

	uint16		PrimaryNumFrames;					// number of animated frames (if 1, not animated)
	uint16		SecondaryNumFrames;				// number of animated frames (if 1, not animated)

	char			Pad[12];								// expansion room
};


/////////////////////////////////////////////////////////////////////////////////////////////
// MATERIAL ATTRIBUTES (version 3.0 onward)
/////////////////////////////////////////////////////////////////////////////////////////////
// Use alpha enables alpha channels, etc, Use sorting causes display lists using
// this material to be sorted (even with z-buf, translucent materials need to be sorted)
#define		W3DMATERIAL_USE_ALPHA 									0x00000001
#define		W3DMATERIAL_USE_SORTING 								0x00000002

// Hints for render devices that cannot support all features
#define		W3DMATERIAL_HINT_DIT_OVER_DCT 						0x00000010
#define		W3DMATERIAL_HINT_SIT_OVER_SCT 						0x00000020
#define		W3DMATERIAL_HINT_DIT_OVER_DIG 						0x00000040
#define		W3DMATERIAL_HINT_SIT_OVER_SIG 						0x00000080
#define		W3DMATERIAL_HINT_FAST_SPECULAR_AFTER_ALPHA 		0x00000100

// Last byte is for PSX: Translucency type and a lighting disable flag.
#define		W3DMATERIAL_PSX_MASK										0xFF000000
#define		W3DMATERIAL_PSX_TRANS_MASK 							0x07000000
#define		W3DMATERIAL_PSX_TRANS_NONE 							0x00000000
#define		W3DMATERIAL_PSX_TRANS_100 								0x01000000
#define		W3DMATERIAL_PSX_TRANS_50 								0x02000000
#define		W3DMATERIAL_PSX_TRANS_25 								0x03000000
#define		W3DMATERIAL_PSX_TRANS_MINUS_100 						0x04000000
#define		W3DMATERIAL_PSX_NO_RT_LIGHTING 						0x08000000

/////////////////////////////////////////////////////////////////////////////////////////////
// MAPPING TYPES (version 3.0 onward)
/////////////////////////////////////////////////////////////////////////////////////////////
#define		W3DMAPPING_UV												0
#define		W3DMAPPING_ENVIRONMENT									1

/////////////////////////////////////////////////////////////////////////////////////////////
// Version 3.0 Material, A W3D_CHUNK_MATERIALS3 chunk will wrap a bunch of 
// W3D_CHUNK_MATERIAL3 chunks.  Inside each chunk will be a name chunk, an 'info' chunk which
// contains the following struct, and one or more map chunks. a mesh with 2 materials might
// look like:
//
// W3D_CHUNK_MATERIALS3						<-- simply a wrapper around the array of mtls
//		W3D_CHUNK_MATERIAL3					<-- a wrapper around each material
//			W3D_CHUNK_STRING			 		<-- name of the material
//			W3D_CHUNK_MATERIAL3_INFO		<-- standard material properties, a W3dMaterial3Struct
//			W3D_CHUNK_MATERIAL3_DC_MAP		<-- a map, W3dMap3Struct
//				W3D_CHUNK_STRING				<-- filename of the map
//				W3D_CHUNK_MAP_INFO			<-- map parameters
//			W3D_CHUNK_MATERIAL3_SC_MAP        
//				W3D_CHUNK_STRING				<-- filename of the map
//				W3D_CHUNK_MAP_INFO
//		W3D_CHUNK_MATERIAL3
//			W3D_CHUNK_MATERIAL3_NAME
//			W3D_CHUNK_MATERIAL3_INFO
//			W3D_CHUNK_MATERIAL3_SI_MAP
//		
/////////////////////////////////////////////////////////////////////////////////////////////
struct W3dMaterial3Struct
{
	uint32					Attributes;					// flags,hints,etc.
	
	W3dRGBStruct			DiffuseColor;				// diffuse color
	W3dRGBStruct			SpecularColor;				// specular color

	W3dRGBStruct			EmissiveCoefficients;	// emmissive coefficients, default to 0,0,0
	W3dRGBStruct			AmbientCoefficients;		// ambient coefficients, default to 1,1,1
	W3dRGBStruct			DiffuseCoefficients;		// diffuse coeficients, default to 1,1,1
	W3dRGBStruct			SpecularCoefficients;	// specular coefficients, default to 0,0,0

	float32					Shininess;					// how tight the specular highlight will be, 1 - 1000 (default = 1)
	float32					Opacity;						// how opaque the material is, 0.0 = invisible, 1.0 = fully opaque (default = 1)
	float32					Translucency;				// how much light passes through the material. (default = 0)
	float32					FogCoeff;					// effect of fog (0.0=not fogged, 1.0=fogged) (default = 1)
};


/////////////////////////////////////////////////////////////////////////////////////////////
// A map, only occurs as part of a material, will be preceeded by its name.
/////////////////////////////////////////////////////////////////////////////////////////////
struct W3dMap3Struct
{
	uint16					MappingType;				// Mapping type, will be one of the above #defines (e.g. W3DMAPPING_UV)
	uint16					FrameCount;					// Number of frames (1 if not animated)
	float32					FrameRate;					// Frame rate, frames per second in floating point
};


/////////////////////////////////////////////////////////////////////////////////////////////
// A triangle, occurs inside the W3D_CHUNK_SURRENDER_TRIANGLES chunk
/////////////////////////////////////////////////////////////////////////////////////////////
struct W3dSurrenderTriStruct
{
	uint32					Vindex[3];			// vertex, vert normal, and texture coord indexes (all use same index)
	W3dTexCoordStruct		TexCoord[3];		// texture coordinates	(OBSOLETE!!!)
	uint32					MaterialIdx; 		// material index
	W3dVectorStruct		Normal;		 		// Face normal
	uint32					Attributes;			// collision flags, sort method, etc
	W3dRGBStruct			Gouraud[3];			// Pre-set shading values (OBSOLETE!!!)
};


/////////////////////////////////////////////////////////////////////////////////////////////
// A triangle, occurs inside the W3D_CHUNK_TRIANGLES chunk
// This is NEW for Version 3.
/////////////////////////////////////////////////////////////////////////////////////////////
struct W3dTriStruct
{
	uint32					Vindex[3];			// vertex,vnormal,texcoord,color indices
	uint32					Attributes;			// attributes bits
	W3dVectorStruct		Normal;				// plane normal
	float32					Dist;					// plane distance
};


/////////////////////////////////////////////////////////////////////////////////////////////
// Flags for the Mesh Attributes member
/////////////////////////////////////////////////////////////////////////////////////////////
#define W3D_MESH_FLAG_NONE									0x00000000			// plain ole normal mesh
#define W3D_MESH_FLAG_COLLISION_BOX						0x00000001			// mesh is a collision box (should be 8 verts, should be hidden, etc)
#define W3D_MESH_FLAG_SKIN									0x00000002			// skin mesh 
#define W3D_MESH_FLAG_SHADOW								0x00000004			// intended to be projected as a shadow
#define W3D_MESH_FLAG_ALIGNED								0x00000008			// always aligns with camera

#define W3D_MESH_FLAG_COLLISION_TYPE_MASK				0x00000FF0			// mask for the collision type bits
#define W3D_MESH_FLAG_COLLISION_TYPE_SHIFT						4			// shifting to get to the collision type bits
#define W3D_MESH_FLAG_COLLISION_TYPE_PHYSICAL		0x00000010			// physical collisions
#define W3D_MESH_FLAG_COLLISION_TYPE_PROJECTILE		0x00000020			// projectiles (rays) collide with this

#define W3D_MESH_FLAG_HIDDEN								0x00001000			// this mesh is hidden by default


/////////////////////////////////////////////////////////////////////////////////////////////
// Original (Obsolete) Mesh Header
/////////////////////////////////////////////////////////////////////////////////////////////
struct W3dMeshHeaderStruct
{
	uint32					Version;							// Currently version 0x100
	char						MeshName[W3D_NAME_LEN];		// name of the mesh (Null terminated)
	uint32					Attributes;
	
	//
	// Counts, these can be regarded as an inventory of what is to come in the file.
	//
	uint32					NumTris;				// number of triangles (OBSOLETE!)
	uint32					NumQuads;			// number of quads; (OBSOLETE!)
	uint32					NumSrTris;			// number of triangles

	uint32					NumPovTris;			// (NOT USED)
	uint32					NumPovQuads;		// (NOT USED)
	
	uint32					NumVertices;		// number of unique vertices
	uint32					NumNormals;			// number of unique normals (OBSOLETE!)
	uint32					NumSrNormals;		// number of surrender normals (MUST EQUAL NumVertices or 0)

	uint32					NumTexCoords;		// number of unique texture coords (MUST EQUAL NumVertices or 0)
	uint32					NumMaterials;		// number of unique materials needed

	uint32					NumVertColors;		// number of vertex colors (MUST EQUAL NumVertices or 0)
	uint32					NumVertInfluences;// vertex influences(MUST EQUAL NumVertices or 0)
	uint32					NumDamageStages;	// number of damage offset chunks
	uint32					FutureCounts[5];	// reserve space for future counts (set to zero).

	//
	// LOD controls
	//
	float32					LODMin;				// min LOD distance
	float32					LODMax;				// max LOD distance

	//
	// Collision / rendering quick-rejection
	//
	W3dVectorStruct		Min;					// Min corner of the bounding box
	W3dVectorStruct		Max;					// Max corner of the bounding box
	W3dVectorStruct		SphCenter;			// Center of bounding sphere
	float32					SphRadius;			// Bounding sphere radius

	// 
	// Default transformation
	//
	W3dVectorStruct		Translation;
	float32					Rotation[9];

	// 
	// Physics Properties
	//
	W3dVectorStruct		MassCenter;			// Center of mass in object space
	float32					Inertia[9];			// Inertia tensor (relative to MassCenter)
	float32					Volume;				// volume of the object

	//
	// Name of possible hierarchy this mesh should be attached to
	//
	char						HierarchyTreeName[W3D_NAME_LEN];
	char						HierarchyModelName[W3D_NAME_LEN];
	uint32					FutureUse[24];		// Reserved for future use
};


/////////////////////////////////////////////////////////////////////////////////////////////
// Version 3 Mesh Header, trimmed out some of the junk that was in the
// previous versions.  
/////////////////////////////////////////////////////////////////////////////////////////////
#define W3D_VERTEX_CHANNEL_LOCATION		0x00000001	// object-space location of the vertex
#define W3D_VERTEX_CHANNEL_NORMAL		0x00000002	// object-space normal for the vertex
#define W3D_VERTEX_CHANNEL_TEXCOORD		0x00000004	// texture coordinate
#define W3D_VERTEX_CHANNEL_COLOR			0x00000008	// vertex color
#define W3D_VERTEX_CHANNEL_BONEID		0x00000010	// per-vertex bone id for skins

#define W3D_FACE_CHANNEL_FACE				0x00000001	// basic face info, W3dTriStruct...

struct W3dMeshHeader3Struct
{
	uint32					Version;							
	uint32					Attributes;
	
	char						MeshName[W3D_NAME_LEN];		
	char						HierarchyModelName[W3D_NAME_LEN];

	//
	// Counts, these can be regarded as an inventory of what is to come in the file.
	//
	uint32					NumTris;				// number of triangles
	uint32					NumVertices;		// number of unique vertices
	uint32					NumMaterials;		// number of unique materials
	uint32					NumDamageStages;	// number of damage offset chunks
	uint32					FutureCounts[3];	// future counts

	uint32					VertexChannels;	// bits for presence of types of per-vertex info
	uint32					FaceChannels;		// bits for presence of types of per-face info
	
	//
	// Bounding volumes
	//
	W3dVectorStruct		Min;					// Min corner of the bounding box
	W3dVectorStruct		Max;					// Max corner of the bounding box
	W3dVectorStruct		SphCenter;			// Center of bounding sphere
	float32					SphRadius;			// Bounding sphere radius

};

//
// Vertex Influences.  For "skins" each vertex can be associated with a
// different bone.
// 
struct W3dVertInfStruct
{
	uint16					BoneIdx;
	uint8						Pad[6];
};

//
// Mesh Damage.  This can include a new set of materials for the mesh,
// new positions for certain vertices in the mesh, and new vertex
// colors for certain vertices.
//
struct W3dMeshDamageStruct
{
	uint32					NumDamageMaterials;	// number of materials to replace
	uint32					NumDamageVerts;		// number of vertices to replace
	uint32					NumDamageColors;		// number of vertex colors to replace
	uint32					DamageIndex;			// what index is this damage chunk assigned to
	uint32					FutureUse[4];	
};

struct W3dMeshDamageVertexStruct
{
	uint32				VertexIndex;
	W3dVectorStruct	NewVertex;
};

struct W3dMeshDamageColorStruct
{
	uint32				VertexIndex;
	W3dRGBStruct		NewColor;
};


/********************************************************************************

	WHT ( Westwood Hierarchy Tree )

	A hierarchy tree defines a set of coordinate systems which are connected
	hierarchically.  The header defines the name, number of pivots, etc.  
	The pivots chunk will contain a W3dPivotStructs for each node in the
	tree.  
	
	The W3dPivotFixupStruct contains a transform for each MAX coordinate
	system and our version of that same coordinate system (bone).  It is 
	needed when the user exports the base pose using "Translation Only".
	These are the matrices which go from the MAX rotated coordinate systems
	to a system which is unrotated in the base pose.  These transformations
	are needed when exporting a hierarchy animation with the given hierarchy
	tree file.

	Another explanation of these kludgy "fixup" matrices:

	What are the "fixup" matrices?  These are the transforms which
	were applied to the base pose when the user wanted to force the
	base pose to use only matrices with certain properties.  For 
	example, if we wanted the base pose to use translations only,
	the fixup transform for each node is a transform which when
	multiplied by the real node's world transform, yeilds a pure
	translation matrix.  Fixup matrices are used in the mesh
	exporter since all vertices must be transformed by their inverses
	in order to make things work.  They also show up in the animation
	exporter because they are needed to make the animation work with
	the new base pose.

********************************************************************************/

struct W3dHierarchyStruct
{
	uint32					Version;
	char						Name[W3D_NAME_LEN];	// Name of the hierarchy
	uint32					NumPivots;				
	W3dVectorStruct		Center;					
};

struct W3dPivotStruct
{
	char						Name[W3D_NAME_LEN];	// Name of the node (UR_ARM, LR_LEG, TORSO, etc)
	uint32					ParentIdx;				// 0xffffffff = root pivot; no parent
	W3dVectorStruct		Translation;			// translation to pivot point
	W3dVectorStruct		EulerAngles;			// orientation of the pivot point
	W3dQuaternionStruct	Rotation;				// orientation of the pivot point
};

struct W3dPivotFixupStruct
{
	float32					TM[4][3];				// this is a direct dump of a MAX 3x4 matrix
};


/********************************************************************************

	WHA (Westwood Hierarchy Animation)

	A Hierarchy Animation is a set of data defining deltas from the base 
	position of a hierarchy tree.  Translation and Rotation channels can be
	attached to any node of the hierarchy tree which the animation is 
	associated with.

********************************************************************************/

struct W3dAnimHeaderStruct
{
	uint32					Version;
	char						Name[W3D_NAME_LEN];				
	char						HierarchyName[W3D_NAME_LEN];
	uint32					NumFrames;
	uint32					FrameRate;
};

enum 
{
	ANIM_CHANNEL_X = 0,
	ANIM_CHANNEL_Y,
	ANIM_CHANNEL_Z,
	ANIM_CHANNEL_XR,
	ANIM_CHANNEL_YR,
	ANIM_CHANNEL_ZR,
	ANIM_CHANNEL_Q
};

struct W3dAnimChannelStruct
{
	uint16					FirstFrame;			
	uint16					LastFrame;			
	uint16					VectorLen;			// length of each vector in this channel
	uint16					Flags;				// channel type.
	uint16					Pivot;				// pivot affected by this channel
	uint16					pad;
	float32					Data[1];				// will be (LastFrame - FirstFrame + 1) * VectorLen long
};

enum 
{
	BIT_CHANNEL_VIS = 0,							// turn meshes on and off depending on anim frame.
};

struct W3dBitChannelStruct
{
	uint16					FirstFrame;			// all frames outside "First" and "Last" are assumed = DefaultVal
	uint16					LastFrame;			
	uint16					Flags;				// channel type.
	uint16					Pivot;				// pivot affected by this channel
	uint8						DefaultVal;			// default state when outside valid range.
	uint8						Data[1];				// will be (LastFrame - FirstFrame + 1) / 8 long
};

/********************************************************************************

	(HModel - Hiearchy Model)

	A Hierarchy Model is a set of render objects which should be attached to 
	bones in a hierarchy tree.  There can be multiple objects per node
	in the tree.  Or there may be no objects attached to a particular bone.

********************************************************************************/

struct W3dHModelHeaderStruct
{
	uint32					Version;
	char						Name[W3D_NAME_LEN];				// Name of this connection set (NULL terminated)
	char						HierarchyName[W3D_NAME_LEN];	// Name of hierarchy associated with these connections (NULL terminated)
	uint16					NumConnections;				
};

struct W3dHModelAuxDataStruct
{
	uint32					Attributes;
	uint32					MeshCount;
	uint32					CollisionCount;
	uint32					SkinCount;
	uint32					ShadowCount;
	uint32					FutureCounts[7];

	float32					LODMin;
	float32					LODMax;
	uint32					FutureUse[32];	
};

struct W3dHModelNodeStruct
{
	// Note: the full name of the Render object is expected to be: <HModelName>.<RenderObjName>
	char						RenderObjName[W3D_NAME_LEN];
	uint16					PivotIdx;
};


/********************************************************************************

	(LODModel - Level-Of-Detail Model)

	An LOD Model is a set of render objects which are interchangeable and
	designed to be different resolution versions of the same object.

********************************************************************************/

struct W3dLODModelHeaderStruct
{
	uint32					Version;
	char						Name[W3D_NAME_LEN];				// Name of this LOD Model
	uint16					NumLODs;				
};

struct W3dLODStruct 
{
	char						RenderObjName[2*W3D_NAME_LEN];
	float32					LODMin;								// "artist" inspired switching distances
	float32					LODMax;
};


/********************************************************************************

	TileMap



********************************************************************************/

struct W3dTileMapHeaderStruct
{
	uint32				Version;
	uint32				TileInstanceCount;
};

#define W3D_TILE_ROTATION_0			0x00
#define W3D_TILE_ROTATION_90			0x01
#define W3D_TILE_ROTATION_180			0x02
#define W3D_TILE_ROTATION_270			0x03

struct W3dTileInstanceStruct
{
	char					RenderObjName[2*W3D_NAME_LEN];
	uint32				Rotation;
	W3dVectorStruct	Position;
};

#define W3D_TILEMAP_PARTITION_FLAGS_PARTITION		0x0001
#define W3D_TILEMAP_PARTITION_FLAGS_LEAF 				0x0002
#define W3D_TILEMAP_PARTITION_FLAGS_XNORMAL			0x0004
#define W3D_TILEMAP_PARTITION_FLAGS_YNORMAL			0x0008
#define W3D_TILEMAP_PARTITION_FLAGS_ZNORMAL			0x0010
#define W3D_TILEMAP_PARTITION_FLAGS_FRONT_CHILD		0x0020
#define W3D_TILEMAP_PARTITION_FLAGS_BACK_CHILD		0x0040

struct W3dTileMapPartitionStruct
{
	uint16				Flags;						// type of node, type of plane, flags for presence of children.
	uint16				InstanceIndex;				// if Type==LEAF, this field will store the tile instance index.
	float32				Dist;							// distance along plane axis.
	W3dVectorStruct	Min;							// min corner of the bounding box
	W3dVectorStruct	Max;							// max corner of the bounding box
};



#endif
