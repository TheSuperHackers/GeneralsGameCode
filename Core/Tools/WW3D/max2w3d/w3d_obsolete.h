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
 *                     $Archive:: /Commando/Code/Tools/max2w3d/w3d_obsolete.h                 $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 9/22/00 10:40a                                              $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef W3D_OBSOLETE_H
#define W3D_OBSOLETE_H


/////////////////////////////////////////////////////////////////////////////////////////////
// Obsolete chunk id's
// At one point in time I was using the 'O' prefix to mean obsolete.  Now we just move stuff
// into this file...
/////////////////////////////////////////////////////////////////////////////////////////////
enum
{
		W3D_CHUNK_MESH_HEADER					=0x00000001,	// header for a mesh
		W3D_CHUNK_SURRENDER_NORMALS			=0x00000004,	// array of surrender normals (one per vertex as req. by surrender)
		W3D_CHUNK_TEXCOORDS						=0x00000005,	// array of texture coordinates
		O_W3D_CHUNK_MATERIALS					=0x00000006,	// array of materials
		O_W3D_CHUNK_TRIANGLES					=0x00000007,	// array of triangles
		O_W3D_CHUNK_QUADRANGLES					=0x00000008,	// array of quads
		O_W3D_CHUNK_SURRENDER_TRIANGLES		=0x00000009,	// array of surrender format tris
		O_W3D_CHUNK_POV_TRIANGLES				=0x0000000A,	// POV format triangles
		O_W3D_CHUNK_POV_QUADRANGLES			=0x0000000B,	// POV format quads
		W3D_CHUNK_VERTEX_COLORS					=0x0000000D,	// Pre-set vertex coloring
		W3D_CHUNK_DAMAGE							=0x0000000F,	// Mesh damage, new set of materials, vertex positions, vertex colors
			W3D_CHUNK_DAMAGE_HEADER				=0x00000010,	// Header for the damage data, tells what is coming
			W3D_CHUNK_DAMAGE_VERTICES			=0x00000011,	// Array of modified vertices (W3dMeshDamageVertexStruct's)
			W3D_CHUNK_DAMAGE_COLORS				=0x00000012,	// Array of modified vert colors (W3dMeshDamageColorStruct's)
			W3D_CHUNK_DAMAGE_MATERIALS			=0x00000013,

		O_W3D_CHUNK_MATERIALS2					=0x00000014,	// array of version 2 materials (with animation frame counts)

		W3D_CHUNK_MATERIALS3						=0x00000015,	// array of version 3 materials (all new surrender features supported)
			W3D_CHUNK_MATERIAL3					=0x00000016,	// Each version 3 material wrapped with this chunk ID
				W3D_CHUNK_MATERIAL3_NAME		=0x00000017,	// Name of the material (array of chars, null terminated)
				W3D_CHUNK_MATERIAL3_INFO		=0x00000018,	// contains a W3dMaterial3Struct, general material info
				W3D_CHUNK_MATERIAL3_DC_MAP		=0x00000019,	// wraps the following two chunks, diffuse color texture
					W3D_CHUNK_MAP3_FILENAME		=0x0000001A,	// filename of the texture
					W3D_CHUNK_MAP3_INFO			=0x0000001B,	// a W3dMap3Struct
				W3D_CHUNK_MATERIAL3_DI_MAP		=0x0000001C,	// diffuse illimination map, same format as other maps
				W3D_CHUNK_MATERIAL3_SC_MAP		=0x0000001D,	// specular color map, same format as other maps
				W3D_CHUNK_MATERIAL3_SI_MAP		=0x0000001E,	// specular illumination map, same format as other maps
		W3D_CHUNK_PER_TRI_MATERIALS			=0x00000021,	// Multi-Mtl meshes - An array of uint16 material id's
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



//
// W3dHModelAuxDataStruct.  HModels had this extra chunk defining the counts of individual
// types of objects to be found in the model and some obsolete distance-based LOD settings.
// As changes were made to the ww3d library, all of this became useles so the chunk was
// "retired".
//
struct W3dHModelAuxDataStruct
{
	uint32					Attributes;
	uint32					MeshCount;
	uint32					CollisionCount;
	uint32					SkinCount;
	uint32					ShadowCount;
	uint32					NullCount;
	uint32					FutureCounts[6];

	float32					LODMin;
	float32					LODMax;
	uint32					FutureUse[32];
};


#endif // W3D_OBSOLETE_H
