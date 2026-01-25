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

/* $Header: /Commando/Code/Tools/max2w3d/w3d_file.h 19    9/12/01 2:08p Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando / G 3D Library                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/w3d_file.h                     $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 9/12/01 2:04p                                               $*
 *                                                                                             *
 *                    $Revision:: 19                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "always.h"
#include "bittype.h"
#include "iostruct.h"
#include <limits.h>

/********************************************************************************

NAMING CONVENTIONS:

	Typical render object name is 15 characters + NULL
	Meshes have 31 + NULL character name formed from the concatenation of the "container"
		model name and the mesh's name:  "ContainerName.MeshName"
	Animations have 31 + NULL character names formed from the concatenation of the Hierarchy tree
		name with the animation name: "AnimationName.HierarchyName"
	Textures have unlimited name length.
	Typically you can determine which 'W3D' file a render object came from by looking
		at its name.  If the name contains a '.' then the filename is the string before
		the '.' and if not, then the render object name is the name of the file.

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

October 19, 1998

	Created the w3d_obsolete.h header file and moved everything that I could into
	it.  This header was getting so messy that even I couldn't understand it so
	hopefully this helps a little...

	Updating the mesh format as part of the conversion to Surrender1.40.  Some
	of the new features in this mesh format are:
	- per pass, per stage U-V coordinate arrays
	- per pass vertex diffuse color arrays
	- per pass vertex specular color arrays
	- per pass vertex pre-calced diffuse illumination arrays

	In addition, the way you describe the materials for a mesh has *completely*
	changed.  The new system separates the concepts of VertexMaterial, Texture
	and Shader.  A VertexMaterial defines the parameters which control the
	gradient calculations for vertices.  Textures you know about.  Shaders
	define how the gradients (diffuse and specular) are combined with the texture
	and the frame buffer.  In addition, a mesh can have several passes; each
	pass having VertexMaterials, Textures and Shaders
	- new chunk to describe a shader, contains a W3dShaderStruct
	- new chunk to describe a vertex material, contains a W3dVertexMaterialStruct
	- textures use the old 'Map3' chunk still
	- new chunk to describe a "pass" for the mesh
	- new chunk to tie up all of these vertex materials, shaders, and textures
	- new chunks for specifying per-poly, per-pass arrays of texture, shader, and
	  vertex material indices.

	The culling system changed a bit requiring some re-working of the "Tilemap"
	so I have removed the tilemap chunks.  At this point in time, all of the AABTree
	building is being done at run-time.  Once we get the editor a little farther
	along, we'll define some new chunks for this stuff.

	At this point in time, meshes look like the following.  I've placed an asterisk
	next to the new chunk types.

	W3D_CHUNK_MESH
		W3D_CHUNK_MESH_HEADER3
		W3D_CHUNK_MESH_USER_TEXT
		W3D_CHUNK_VERTICES
		W3D_CHUNK_VERTEX_NORMALS
		W3D_CHUNK_VERTEX_INFLUENCES
		W3D_CHUNK_TRIANGLES
*		W3D_CHUNK_VERTEX_SHADE_INDICES

*		W3D_CHUNK_MATERIAL_INFO					// how many passes, vertex mtls, shaders, and textures...
*		W3D_CHUNK_SHADERS							// array of W3dShaderStruct's
*		W3D_CHUNK_VERTEX_MATERIALS
*			W3D_CHUNK_VERTEX_MATERIAL
*				W3D_CHUNK_VERTEX_MATERIAL_NAME
*				W3D_CHUNK_VERTEX_MATERIAL_INFO
			...

*		W3D_CHUNK_TEXTURES
*			W3D_CHUNK_TEXTURE
*				W3D_CHUNK_TEXTURE_NAME
*				W3D_CHUNK_TEXTURE_INFO
			...

*		W3D_CHUNK_MATERIAL_PASS
*			W3D_CHUNK_VERTEX_MATERIAL_IDS
*			W3D_CHUNK_SHADER_IDS

*			W3D_CHUNK_DCG
*			W3D_CHUNK_DIG
*			W3D_CHUNK_SCG

*			W3D_CHUNK_TEXTURE_STAGE
*				W3D_CHUNK_TEXTURE_IDS
*				W3D_CHUNK_STAGE_TEXCOORDS


	Added a Collection chunk type.  When we export a bunch of independent meshes
	(turning off the hierarchy and animation options) this chunk will be added
	on to the end of the file.  It lists by name each render object that was
	defined in the file.  Presumably the run-time asset manager will be able
	to give you a "collection" render object which will be named the same as
	the w3d file that it came from and will contain the name or an actual instance
	of each of the meshes.  This is a feature that was added for the Commando
	level editor.

	Added the W3D_CHUNK_POINTS chunk.  This is used to implement "snap points"
	for the level editor.  It is just an array of points that were found in
	the max scene (helper object->point).  We make these points co-incide in
	the level editor to snap objects together.  This chunk can occur inside a
	mesh, hmodel, or collection chunk.  When it does, the points should simply
	be associated with the model being defined.

August 5, 1999

	Adding Null Object exporting

March 28, 2000

	Adding Merge objects to collections.  We already have "Proxy" objects which ask
	the level editor to instatiate an object at a given transform.  Now we will
	have "Merge" objects which mean a copy of the named model should be merged in
	with this model.  This is used in the case of building interiors.  We can create
	a building interior and lightmap it as a separate collection, then "merge" it
	with the level collection (multiple times even) and the tool will pre-transform
	the meshes and add them to the collection.

March 31, 2000

  Changed W3D_CHUNK_MERGE_NODE to W3D_CHUNK_TRANSFORM_NODE. Same data - slightly
  different application. Now the information will indicate how a w3d asset with the
  same name as that in the node should pre-transform itself relative to the object
  that contains the transform node.

April 07, 2000

  Added W3D_CHUNK_LIGHTSCAPE, W3D_CHUNK_LIGHTSCAPE_LIGHT and W3D_CHUNK_LIGHT_TRANSFORM.
  These chunks form part of the output of new Westwood light file type (.wlt).

July 10, 2000

  Added W3D_CHUNK_VERTEX_MAPPER_ARGS1 and renamed W3D_CHUNK_VERTEX_MAPPER_ARGS to
  W3D_CHUNK_VERTEX_MAPPER_ARGS0. These are the mapper args for the first and second
  texture stages. The choice of mapper for the second stage has been added to the
  'Attributes' field in W3dVertexMaterialStruct.

August 5, 2000

  Added W3D_CHUNK_LIGHT_GLARE and its sub-chunks.  Light glares are going to
  be a new 'geometry type' which simply define points where light glare effects
  should be placed.  The application will supply a callback to the WW3D code
  which indicates the visibilty of any light glares in the view frustum.

June 5, 2001

  (gth) Adding line rendering options to particle systems today.  This involves a
  new line-properties chunk and a RenderMode variable added to the InfoV2 struct.

TheSuperHackers @forwardport:
April 5, 2025

  Added W3D_CHUNK_FX_SHADERS and subchunks, which were added in BFME2. These
  chunks are used to define usage of a specific shader and pass params.

April 21, 2025

	Added W3D_CHUNK_VERTEX_TANGENT and W3D_CHUNK_VERTEX_BINORMAL chunks. These are used to define
	normal mapping information for a mesh. They were added in BFME2.

********************************************************************************/


#define W3D_MAKE_VERSION(major,minor)		(((major) << 16) | (minor))
#define W3D_GET_MAJOR_VERSION(ver)			((ver)>>16)
#define W3D_GET_MINOR_VERSION(ver)			((ver) & 0xFFFF)

#define W3D_NAME_LEN	16



/********************************************************************************

	CHUNK TYPES FOR ALL 3D DATA

	All 3d data is stored in chunks similar to an IFF file. Each
	chunk will be headed by an ID and size field.

	All structures defined in this header file are prefixed with
	W3d to prevent naming conflicts with in-game structures which
	may be slightly different than the on-disk structures.

	Oct 19,1998: Moved obsolete chunk id's to w3d_obsolete.h, added many
	new chunk types.

********************************************************************************/

enum {

	W3D_CHUNK_MESH											=0x00000000,	// Mesh definition
		W3D_CHUNK_VERTICES								=0x00000002,	// array of vertices (array of W3dVectorStruct's)
		W3D_CHUNK_VERTEX_NORMALS						=0x00000003,	// array of normals (array of W3dVectorStruct's)
		W3D_CHUNK_MESH_USER_TEXT						=0x0000000C,	// Text from the MAX comment field (Null terminated string)
		W3D_CHUNK_VERTEX_INFLUENCES					=0x0000000E,	// Mesh Deformation vertex connections (array of W3dVertInfStruct's)
		W3D_CHUNK_MESH_HEADER3							=0x0000001F,	//	mesh header contains general info about the mesh. (W3dMeshHeader3Struct)
		W3D_CHUNK_TRIANGLES								=0x00000020,	// New improved triangles chunk (array of W3dTriangleStruct's)
		W3D_CHUNK_VERTEX_SHADE_INDICES				=0x00000022,	// shade indexes for each vertex (array of uint32's)

		W3D_CHUNK_PRELIT_UNLIT							=0x00000023,	// optional unlit material chunk wrapper
		W3D_CHUNK_PRELIT_VERTEX							=0x00000024,	// optional vertex-lit material chunk wrapper
		W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_PASS		=0x00000025,	// optional lightmapped multi-pass material chunk wrapper
		W3D_CHUNK_PRELIT_LIGHTMAP_MULTI_TEXTURE	=0x00000026,	// optional lightmapped multi-texture material chunk wrapper

			W3D_CHUNK_MATERIAL_INFO						=0x00000028,	// materials information, pass count, etc (contains W3dMaterialInfoStruct)

			W3D_CHUNK_SHADERS								=0x00000029,	// shaders (array of W3dShaderStruct's)

			W3D_CHUNK_VERTEX_MATERIALS					=0x0000002A,	// wraps the vertex materials
				W3D_CHUNK_VERTEX_MATERIAL				=0x0000002B,
					W3D_CHUNK_VERTEX_MATERIAL_NAME	=0x0000002C,	// vertex material name (NULL-terminated string)
					W3D_CHUNK_VERTEX_MATERIAL_INFO	=0x0000002D,	// W3dVertexMaterialStruct
					W3D_CHUNK_VERTEX_MAPPER_ARGS0		=0x0000002E,	// Null-terminated string
					W3D_CHUNK_VERTEX_MAPPER_ARGS1		=0x0000002F,	// Null-terminated string

			W3D_CHUNK_TEXTURES							=0x00000030,	// wraps all of the texture info
				W3D_CHUNK_TEXTURE							=0x00000031,	// wraps a texture definition
					W3D_CHUNK_TEXTURE_NAME				=0x00000032,	// texture filename (NULL-terminated string)
					W3D_CHUNK_TEXTURE_INFO				=0x00000033,	// optional W3dTextureInfoStruct

			W3D_CHUNK_MATERIAL_PASS						=0x00000038,	// wraps the information for a single material pass
				W3D_CHUNK_VERTEX_MATERIAL_IDS			=0x00000039,	// single or per-vertex array of uint32 vertex material indices (check chunk size)
				W3D_CHUNK_SHADER_IDS						=0x0000003A,	// single or per-tri array of uint32 shader indices (check chunk size)
				W3D_CHUNK_DCG								=0x0000003B,	// per-vertex diffuse color values (array of W3dRGBAStruct's)
				W3D_CHUNK_DIG								=0x0000003C,	// per-vertex diffuse illumination values (array of W3dRGBStruct's)
				W3D_CHUNK_SCG								=0x0000003E,	// per-vertex specular color values (array of W3dRGBStruct's)
				W3D_CHUNK_FXSHADER_IDS						=0x0000003F,	// single or per-tri array of uint32 fx shader indices (check chunk size)

				W3D_CHUNK_TEXTURE_STAGE					=0x00000048,	// wrapper around a texture stage.
					W3D_CHUNK_TEXTURE_IDS				=0x00000049,	// single or per-tri array of uint32 texture indices (check chunk size)
					W3D_CHUNK_STAGE_TEXCOORDS			=0x0000004A,	// per-vertex texture coordinates (array of W3dTexCoordStruct's)
					W3D_CHUNK_PER_FACE_TEXCOORD_IDS	=0x0000004B,	// indices to W3D_CHUNK_STAGE_TEXCOORDS, (array of Vector3i)

		W3D_CHUNK_FX_SHADERS 					=0x00000050,	// define an array of shaders to be used in the mesh
			W3D_CHUNK_FX_SHADER					=0x00000051,	// a single shader entry
				W3D_CHUNK_FX_SHADER_INFO		=0x00000052,	// information about the shader to be used (W3dFXShaderInfoStruct)
				W3D_CHUNK_FX_SHADER_CONSTANT	=0x00000053,	// contains a constant name and value for the shader

		W3D_CHUNK_DEFORM									=0x00000058,	// mesh deform or 'damage' information.
			W3D_CHUNK_DEFORM_SET							=0x00000059,	// set of deform information
				W3D_CHUNK_DEFORM_KEYFRAME				=0x0000005A,	// a keyframe of deform information in the set
					W3D_CHUNK_DEFORM_DATA				=0x0000005B,	// deform information about a single vertex

		W3D_CHUNK_PS2_SHADERS							=0x00000080,	// Shader info specific to the Playstation 2.

		W3D_CHUNK_VERTEX_TANGENTS = 0x00000060,         // array of tangents (array of W3dVectorStruct's)
		W3D_CHUNK_VERTEX_BINORMALS = 0x00000061,         // array of binormals (array of W3dVectorStruct's)

		W3D_CHUNK_AABTREE									=0x00000090,	// Axis-Aligned Box Tree for hierarchical polygon culling
			W3D_CHUNK_AABTREE_HEADER,										// catalog of the contents of the AABTree
			W3D_CHUNK_AABTREE_POLYINDICES,								// array of uint32 polygon indices with count=mesh.PolyCount
			W3D_CHUNK_AABTREE_NODES,										// array of W3dMeshAABTreeNode's with count=aabheader.NodeCount

	W3D_CHUNK_HIERARCHY									=0x00000100,	// hierarchy tree definition
		W3D_CHUNK_HIERARCHY_HEADER,
		W3D_CHUNK_PIVOTS,
		W3D_CHUNK_PIVOT_FIXUPS,												// only needed by the exporter...

	W3D_CHUNK_ANIMATION									=0x00000200,	// hierarchy animation data
		W3D_CHUNK_ANIMATION_HEADER,
		W3D_CHUNK_ANIMATION_CHANNEL,										// channel of vectors
		W3D_CHUNK_BIT_CHANNEL,												// channel of boolean values (e.g. visibility)

	W3D_CHUNK_COMPRESSED_ANIMATION					=0x00000280,	// compressed hierarchy animation data
		W3D_CHUNK_COMPRESSED_ANIMATION_HEADER,							// describes playback rate, number of frames, and type of compression
		W3D_CHUNK_COMPRESSED_ANIMATION_CHANNEL,						// compressed channel, format dependent on type of compression
		W3D_CHUNK_COMPRESSED_BIT_CHANNEL,								// compressed bit stream channel, format dependent on type of compression

	W3D_CHUNK_MORPH_ANIMATION							=0x000002C0,	// hierarchy morphing animation data (morphs between poses, for facial animation)
		W3D_CHUNK_MORPHANIM_HEADER,										// W3dMorphAnimHeaderStruct describes playback rate, number of frames, and type of compression
		W3D_CHUNK_MORPHANIM_CHANNEL,										// wrapper for a channel
			W3D_CHUNK_MORPHANIM_POSENAME,									// name of the other anim which contains the poses for this morph channel
			W3D_CHUNK_MORPHANIM_KEYDATA,									// morph key data for this channel
		W3D_CHUNK_MORPHANIM_PIVOTCHANNELDATA,							// uin32 per pivot in the htree, indicating which channel controls the pivot

	W3D_CHUNK_HMODEL										=0x00000300,	// blueprint for a hierarchy model
		W3D_CHUNK_HMODEL_HEADER,											// Header for the hierarchy model
		W3D_CHUNK_NODE,														// render objects connected to the hierarchy
		W3D_CHUNK_COLLISION_NODE,											// collision meshes connected to the hierarchy
		W3D_CHUNK_SKIN_NODE,													// skins connected to the hierarchy
		OBSOLETE_W3D_CHUNK_HMODEL_AUX_DATA,								// extension of the hierarchy model header
		OBSOLETE_W3D_CHUNK_SHADOW_NODE,									// shadow object connected to the hierarchy

	W3D_CHUNK_LODMODEL								=0x00000400,		// blueprint for an LOD model.  This is simply a
		W3D_CHUNK_LODMODEL_HEADER,											// collection of 'n' render objects, ordered in terms
		W3D_CHUNK_LOD,															// of their expected rendering costs. (highest is first)

	W3D_CHUNK_COLLECTION								=0x00000420,		// collection of render object names
		W3D_CHUNK_COLLECTION_HEADER,										// general info regarding the collection
		W3D_CHUNK_COLLECTION_OBJ_NAME,									// contains a string which is the name of a render object
		W3D_CHUNK_PLACEHOLDER,												// contains information about a 'dummy' object that will be instanced later
		W3D_CHUNK_TRANSFORM_NODE,											// contains the filename of another w3d file that should be transformed by this node

	W3D_CHUNK_POINTS									=0x00000440,		// array of W3dVectorStruct's.  May appear in meshes, hmodels, lodmodels, or collections.

	W3D_CHUNK_LIGHT									=0x00000460,		// description of a light
		W3D_CHUNK_LIGHT_INFO,												// generic light parameters
		W3D_CHUNK_SPOT_LIGHT_INFO,											// extra spot light parameters
		W3D_CHUNK_NEAR_ATTENUATION,										// optional near attenuation parameters
		W3D_CHUNK_FAR_ATTENUATION,											// optional far attenuation parameters

	W3D_CHUNK_EMITTER									=0x00000500,		// description of a particle emitter
		W3D_CHUNK_EMITTER_HEADER,											// general information such as name and version
		W3D_CHUNK_EMITTER_USER_DATA,										// user-defined data that specific loaders can switch on
		W3D_CHUNK_EMITTER_INFO,												// generic particle emitter definition
		W3D_CHUNK_EMITTER_INFOV2,											// generic particle emitter definition (version 2.0)
		W3D_CHUNK_EMITTER_PROPS,											// Key-frameable properties
		OBSOLETE_W3D_CHUNK_EMITTER_COLOR_KEYFRAME,					// structure defining a single color keyframe
		OBSOLETE_W3D_CHUNK_EMITTER_OPACITY_KEYFRAME,					// structure defining a single opacity keyframe
		OBSOLETE_W3D_CHUNK_EMITTER_SIZE_KEYFRAME,						// structure defining a single size keyframe
		W3D_CHUNK_EMITTER_LINE_PROPERTIES,								// line properties, used by line rendering mode
		W3D_CHUNK_EMITTER_ROTATION_KEYFRAMES,							// rotation keys for the particles
		W3D_CHUNK_EMITTER_FRAME_KEYFRAMES,								// frame keys (u-v based frame animation)
		W3D_CHUNK_EMITTER_BLUR_TIME_KEYFRAMES,						// length of tail for line groups
		W3D_CHUNK_EMITTER_EXTRA_INFO,										// Extra info for the emitter

	W3D_CHUNK_AGGREGATE								=0x00000600,		// description of an aggregate object
		W3D_CHUNK_AGGREGATE_HEADER,										// general information such as name and version
			W3D_CHUNK_AGGREGATE_INFO,										// references to 'contained' models
		W3D_CHUNK_TEXTURE_REPLACER_INFO,									// information about which meshes need textures replaced
		W3D_CHUNK_AGGREGATE_CLASS_INFO,									// information about the original class that created this aggregate

	W3D_CHUNK_HLOD										=0x00000700,		// description of an HLod object (see HLodClass)
		W3D_CHUNK_HLOD_HEADER,												// general information such as name and version
		W3D_CHUNK_HLOD_LOD_ARRAY,											// wrapper around the array of objects for each level of detail
			W3D_CHUNK_HLOD_SUB_OBJECT_ARRAY_HEADER,					// info on the objects in this level of detail array
			W3D_CHUNK_HLOD_SUB_OBJECT,										// an object in this level of detail array
		W3D_CHUNK_HLOD_AGGREGATE_ARRAY,									// array of aggregates, contains W3D_CHUNK_SUB_OBJECT_ARRAY_HEADER and W3D_CHUNK_SUB_OBJECT_ARRAY
		W3D_CHUNK_HLOD_PROXY_ARRAY,										// array of proxies, used for application-defined purposes, provides a name and a bone.

	W3D_CHUNK_BOX										=0x00000740,		// defines an collision box render object (W3dBoxStruct)
	W3D_CHUNK_SPHERE,
	W3D_CHUNK_RING,

	W3D_CHUNK_NULL_OBJECT							=0x00000750,		// defines a NULL object (W3dNullObjectStruct)

	W3D_CHUNK_LIGHTSCAPE								=0x00000800,		// wrapper for lights created with Lightscape.
		W3D_CHUNK_LIGHTSCAPE_LIGHT,										// definition of a light created with Lightscape.
			W3D_CHUNK_LIGHT_TRANSFORM,										// position and orientation (defined as right-handed 4x3 matrix transform W3dLightTransformStruct).

	W3D_CHUNK_DAZZLE									=0x00000900,		// wrapper for a glare object.  Creates halos and flare lines seen around a bright light source
		W3D_CHUNK_DAZZLE_NAME,												// null-terminated string, name of the dazzle (typical w3d object naming: "container.object")
		W3D_CHUNK_DAZZLE_TYPENAME,											// null-terminated string, type of dazzle (from dazzle.ini)

	W3D_CHUNK_SOUNDROBJ								=0x00000A00,		// description of a sound render object
		W3D_CHUNK_SOUNDROBJ_HEADER,										// general information such as name and version
		W3D_CHUNK_SOUNDROBJ_DEFINITION,									// chunk containing the definition of the sound that is to play

	W3D_CHUNK_SHDMESH									=0x00000B00,		// "Shader mesh" Mesh with multiple sub-meshes that use the scaleable shader system
		W3D_CHUNK_SHDMESH_NAME,
		W3D_CHUNK_SHDMESH_HEADER,
		W3D_CHUNK_SHDMESH_USER_TEXT,										// Text from the MAX comment field (Null terminated string)

		W3D_CHUNK_SHDSUBMESH							=0x00000B20,		// wrapper around an individual sub-mesh.
			W3D_CHUNK_SHDSUBMESH_HEADER,

			W3D_CHUNK_SHDSUBMESH_SHADER			=0x00000B40,		// wrapper around a shader
				W3D_CHUNK_SHDSUBMESH_SHADER_CLASSID,
				W3D_CHUNK_SHDSUBMESH_SHADER_DEF,

 			W3D_CHUNK_SHDSUBMESH_VERTICES,								// array of vertices (array of W3dVectorStruct's)
			W3D_CHUNK_SHDSUBMESH_VERTEX_NORMALS,						// array of normals (array of W3dVectorStruct's)
			W3D_CHUNK_SHDSUBMESH_TRIANGLES,								// array of 16bit int triplets (vertex indices for each triangle)
			W3D_CHUNK_SHDSUBMESH_VERTEX_SHADE_INDICES,				// shade indexes for each vertex (array of uint32's)

			W3D_CHUNK_SHDSUBMESH_UV0,										// per-vertex texture coordinates (array of W3dTexCoordStruct's)
			W3D_CHUNK_SHDSUBMESH_UV1,										// per-vertex texture coordinates (array of W3dTexCoordStruct's)

			W3D_CHUNK_SHDSUBMESH_TANGENT_BASIS_S,						// per-vertex tangent basis S vectors
			W3D_CHUNK_SHDSUBMESH_TANGENT_BASIS_T,						// per-vertex tangent basis T vectors
			W3D_CHUNK_SHDSUBMESH_TANGENT_BASIS_SxT,					// per-vertex tangent basis SxT vectors

			W3D_CHUNK_SHDSUBMESH_VERTEX_COLOR,							// per-vertex color
			W3D_CHUNK_SHDSUBMESH_VERTEX_INFLUENCES,					// byte-per-vertex, WWSkin support
};



struct W3dChunkHeader
{
	uint32		ChunkType;			// Type of chunk (see above enumeration)
	uint32		ChunkSize;			// Size of the chunk, (not including the chunk header)
};



/////////////////////////////////////////////////////////////////////////////////////////////
// vector
/////////////////////////////////////////////////////////////////////////////////////////////
typedef IOVector3Struct			W3dVectorStruct;

/////////////////////////////////////////////////////////////////////////////////////////////
// quaternion
/////////////////////////////////////////////////////////////////////////////////////////////
typedef IOQuaternionStruct		W3dQuaternionStruct;

/////////////////////////////////////////////////////////////////////////////////////////////
// texture coordinate
/////////////////////////////////////////////////////////////////////////////////////////////
struct W3dTexCoordStruct
{
	bool operator == (W3dTexCoordStruct t)
	{
		return ((U == t.U) && (V == t.V));
	}

	float32		U;					  	// U,V coordinates
	float32		V;
};

/////////////////////////////////////////////////////////////////////////////////////////////
// rgb color, one byte per channel, padded to an even 4 bytes
/////////////////////////////////////////////////////////////////////////////////////////////
struct W3dRGBStruct
{
	W3dRGBStruct () {}
	W3dRGBStruct (uint8 r, uint8 g, uint8 b)
	{
		R = r;
		G = g;
		B = b;
	}

	void Set (uint8 r, uint8 g, uint8 b)
	{
		R = r;
		G = g;
		B = b;
	}

	void Set (float r, float g, float b)
	{
		R = (unsigned char) MIN ((float) UCHAR_MAX, MAX (0.0f, r) * ((float) (UCHAR_MAX + 1)));
		G = (unsigned char) MIN ((float) UCHAR_MAX, MAX (0.0f, g) * ((float) (UCHAR_MAX + 1)));
		B = (unsigned char) MIN ((float) UCHAR_MAX, MAX (0.0f, b) * ((float) (UCHAR_MAX + 1)));
	}

	bool operator == (W3dRGBStruct c)
	{
		return ((R == c.R) && (G == c.G) && (B == c.B));
	}

	bool operator != (W3dRGBStruct c)
	{
		return (!(*this == c));
	}

	W3dRGBStruct operator += (W3dRGBStruct c)
	{
		R = MIN (((unsigned) R) + ((unsigned) c.R), (unsigned) UCHAR_MAX);
		G = MIN (((unsigned) G) + ((unsigned) c.G), (unsigned) UCHAR_MAX);
		B = MIN (((unsigned) B) + ((unsigned) c.B), (unsigned) UCHAR_MAX);
		return (*this);
	}

	W3dRGBStruct operator *= (W3dRGBStruct c)
	{
		R = (((unsigned) R) * ((unsigned) c.R)) / ((unsigned) UCHAR_MAX);
		G = (((unsigned) G) * ((unsigned) c.G)) / ((unsigned) UCHAR_MAX);
		B = (((unsigned) B) * ((unsigned) c.B)) / ((unsigned) UCHAR_MAX);
		return (*this);
	}

	unsigned Get_Color()
	{
		return (R<<24)|(G<<16)|(B<<8);
	}

	uint8			R;
	uint8			G;
	uint8			B;
	uint8			pad;
};

struct W3dRGBAStruct
{
	uint8			R;
	uint8			G;
	uint8			B;
	uint8			A;
};


/////////////////////////////////////////////////////////////////////////////////////////////
// MATERIALS
//
// Surrender 1.40 significantly changed the way that materials are described.  To
// accommodate this, the w3d file format has changed since there are new features and
// optimizations that we want to take advangage of.
//
// The VertexMaterial defines parameters which control the calculation of the primary
// and secondary gradients.  The shader defines how those gradients are combined with
// the texel and the frame buffer contents.
//
/////////////////////////////////////////////////////////////////////////////////////////////
struct W3dMaterialInfoStruct
{
	uint32		PassCount;				// how many material passes this render object uses
	uint32		VertexMaterialCount;	// how many vertex materials are used
	uint32		ShaderCount;			// how many shaders are used
	uint32		TextureCount;			// how many textures are used
};


#define		W3DVERTMAT_USE_DEPTH_CUE								0x00000001
#define		W3DVERTMAT_ARGB_EMISSIVE_ONLY							0x00000002
#define		W3DVERTMAT_COPY_SPECULAR_TO_DIFFUSE					0x00000004
#define		W3DVERTMAT_DEPTH_CUE_TO_ALPHA							0x00000008

#define		W3DVERTMAT_STAGE0_MAPPING_MASK						0x00FF0000
#define		W3DVERTMAT_STAGE0_MAPPING_UV							0x00000000
#define		W3DVERTMAT_STAGE0_MAPPING_ENVIRONMENT				0x00010000
#define		W3DVERTMAT_STAGE0_MAPPING_CHEAP_ENVIRONMENT		0x00020000
#define		W3DVERTMAT_STAGE0_MAPPING_SCREEN						0x00030000
#define		W3DVERTMAT_STAGE0_MAPPING_LINEAR_OFFSET			0x00040000
#define		W3DVERTMAT_STAGE0_MAPPING_SILHOUETTE				0x00050000
#define		W3DVERTMAT_STAGE0_MAPPING_SCALE						0x00060000
#define		W3DVERTMAT_STAGE0_MAPPING_GRID						0x00070000
#define		W3DVERTMAT_STAGE0_MAPPING_ROTATE						0x00080000
#define		W3DVERTMAT_STAGE0_MAPPING_SINE_LINEAR_OFFSET		0x00090000
#define		W3DVERTMAT_STAGE0_MAPPING_STEP_LINEAR_OFFSET		0x000A0000
#define		W3DVERTMAT_STAGE0_MAPPING_ZIGZAG_LINEAR_OFFSET	0x000B0000
#define		W3DVERTMAT_STAGE0_MAPPING_WS_CLASSIC_ENV			0x000C0000
#define		W3DVERTMAT_STAGE0_MAPPING_WS_ENVIRONMENT			0x000D0000
#define		W3DVERTMAT_STAGE0_MAPPING_GRID_CLASSIC_ENV		0x000E0000
#define		W3DVERTMAT_STAGE0_MAPPING_GRID_ENVIRONMENT		0x000F0000
#define		W3DVERTMAT_STAGE0_MAPPING_RANDOM						0x00100000
#define		W3DVERTMAT_STAGE0_MAPPING_EDGE						0x00110000
#define		W3DVERTMAT_STAGE0_MAPPING_BUMPENV					0x00120000
#define		W3DVERTMAT_STAGE0_MAPPING_GRID_WS_CLASSIC_ENV	0x00130000
#define		W3DVERTMAT_STAGE0_MAPPING_GRID_WS_ENVIRONMENT	0x00140000

#define		W3DVERTMAT_STAGE1_MAPPING_MASK						0x0000FF00
#define		W3DVERTMAT_STAGE1_MAPPING_UV							0x00000000
#define		W3DVERTMAT_STAGE1_MAPPING_ENVIRONMENT				0x00000100
#define		W3DVERTMAT_STAGE1_MAPPING_CHEAP_ENVIRONMENT		0x00000200
#define		W3DVERTMAT_STAGE1_MAPPING_SCREEN						0x00000300
#define		W3DVERTMAT_STAGE1_MAPPING_LINEAR_OFFSET			0x00000400
#define		W3DVERTMAT_STAGE1_MAPPING_SILHOUETTE				0x00000500
#define		W3DVERTMAT_STAGE1_MAPPING_SCALE						0x00000600
#define		W3DVERTMAT_STAGE1_MAPPING_GRID						0x00000700
#define		W3DVERTMAT_STAGE1_MAPPING_ROTATE						0x00000800
#define		W3DVERTMAT_STAGE1_MAPPING_SINE_LINEAR_OFFSET		0x00000900
#define		W3DVERTMAT_STAGE1_MAPPING_STEP_LINEAR_OFFSET		0x00000A00
#define		W3DVERTMAT_STAGE1_MAPPING_ZIGZAG_LINEAR_OFFSET	0x00000B00
#define		W3DVERTMAT_STAGE1_MAPPING_WS_CLASSIC_ENV			0x00000C00
#define		W3DVERTMAT_STAGE1_MAPPING_WS_ENVIRONMENT			0x00000D00
#define		W3DVERTMAT_STAGE1_MAPPING_GRID_CLASSIC_ENV		0x00000E00
#define		W3DVERTMAT_STAGE1_MAPPING_GRID_ENVIRONMENT		0x00000F00
#define		W3DVERTMAT_STAGE1_MAPPING_RANDOM						0x00001000
#define		W3DVERTMAT_STAGE1_MAPPING_EDGE						0x00001100
#define		W3DVERTMAT_STAGE1_MAPPING_BUMPENV					0x00001200
#define		W3DVERTMAT_STAGE1_MAPPING_GRID_WS_CLASSIC_ENV	0x00001300
#define		W3DVERTMAT_STAGE1_MAPPING_GRID_WS_ENVIRONMENT	0x00001400

#define		W3DVERTMAT_PSX_MASK										0xFF000000
#define		W3DVERTMAT_PSX_TRANS_MASK 								0x07000000
#define		W3DVERTMAT_PSX_TRANS_NONE 								0x00000000
#define		W3DVERTMAT_PSX_TRANS_100 								0x01000000
#define		W3DVERTMAT_PSX_TRANS_50 								0x02000000
#define		W3DVERTMAT_PSX_TRANS_25 								0x03000000
#define		W3DVERTMAT_PSX_TRANS_MINUS_100 						0x04000000
#define		W3DVERTMAT_PSX_NO_RT_LIGHTING 						0x08000000

struct W3dVertexMaterialStruct
{
	W3dVertexMaterialStruct(void) {}

	bool operator == (W3dVertexMaterialStruct vm)
	{
		return (	  Attributes   == vm.Attributes
				  && Ambient	   == vm.Ambient
				  && Diffuse	   == vm.Diffuse
				  && Specular	   == vm.Specular
				  && Emissive	   == vm.Emissive
				  && Shininess	   == vm.Shininess
				  && Opacity	   == vm.Opacity
				  && Translucency == vm.Translucency);
	}

	bool operator != (W3dVertexMaterialStruct vm)
	{
		return (!(*this == vm));
	}

	uint32					Attributes;					// bitfield for the flags defined above
	W3dRGBStruct			Ambient;
	W3dRGBStruct			Diffuse;
	W3dRGBStruct			Specular;
	W3dRGBStruct			Emissive;
	float32					Shininess;					// how tight the specular highlight will be, 1 - 1000 (default = 1)
	float32					Opacity;						// how opaque the material is, 0.0 = invisible, 1.0 = fully opaque (default = 1)
	float32					Translucency;				// how much light passes through the material. (default = 0)
};

inline void W3d_Vertex_Material_Reset(W3dVertexMaterialStruct * vmat)
{
	vmat->Attributes = 0;
	vmat->Ambient.R = vmat->Ambient.G = vmat->Ambient.B = 255;
	vmat->Diffuse.R = vmat->Diffuse.G = vmat->Diffuse.B = 255;
	vmat->Specular.R = vmat->Specular.G = vmat->Specular.B = 0;
	vmat->Emissive.R = vmat->Emissive.G = vmat->Emissive.B = 0;
	vmat->Shininess = 1.0f;
	vmat->Opacity = 1.0f;
	vmat->Translucency = 0.0f;
}


// W3dShaderStruct bits.  These control every setting in the shader.  Use the helper functions
// to set them and test them more easily.
enum
{
	W3DSHADER_DEPTHCOMPARE_PASS_NEVER = 0,			// pass never (i.e. always fail depth comparison test)
	W3DSHADER_DEPTHCOMPARE_PASS_LESS,				// pass if incoming less than stored
	W3DSHADER_DEPTHCOMPARE_PASS_EQUAL,				// pass if incoming equal to stored
	W3DSHADER_DEPTHCOMPARE_PASS_LEQUAL,				// pass if incoming less than or equal to stored (default)
	W3DSHADER_DEPTHCOMPARE_PASS_GREATER,			// pass if incoming greater than stored
	W3DSHADER_DEPTHCOMPARE_PASS_NOTEQUAL,			// pass if incoming not equal to stored
	W3DSHADER_DEPTHCOMPARE_PASS_GEQUAL,				// pass if incoming greater than or equal to stored
	W3DSHADER_DEPTHCOMPARE_PASS_ALWAYS,				// pass always
	W3DSHADER_DEPTHCOMPARE_PASS_MAX,					// end of enumeration

	W3DSHADER_DEPTHMASK_WRITE_DISABLE = 0,			// disable depth buffer writes
	W3DSHADER_DEPTHMASK_WRITE_ENABLE,				// enable depth buffer writes		(default)
	W3DSHADER_DEPTHMASK_WRITE_MAX,					// end of enumeration

	W3DSHADER_ALPHATEST_DISABLE = 0,					// disable alpha testing (default)
	W3DSHADER_ALPHATEST_ENABLE,						// enable alpha testing
	W3DSHADER_ALPHATEST_MAX,							// end of enumeration

  	W3DSHADER_DESTBLENDFUNC_ZERO = 0,				// destination pixel doesn't affect blending (default)
  	W3DSHADER_DESTBLENDFUNC_ONE,						// destination pixel added unmodified
 	W3DSHADER_DESTBLENDFUNC_SRC_COLOR,				// destination pixel multiplied by fragment RGB components
 	W3DSHADER_DESTBLENDFUNC_ONE_MINUS_SRC_COLOR, // destination pixel multiplied by one minus (i.e. inverse) fragment RGB components
 	W3DSHADER_DESTBLENDFUNC_SRC_ALPHA,				// destination pixel multiplied by fragment alpha component
 	W3DSHADER_DESTBLENDFUNC_ONE_MINUS_SRC_ALPHA,	// destination pixel multiplied by fragment inverse alpha
 	W3DSHADER_DESTBLENDFUNC_SRC_COLOR_PREFOG,		// destination pixel multiplied by fragment RGB components prior to fogging
	W3DSHADER_DESTBLENDFUNC_MAX,						// end of enumeration

	W3DSHADER_PRIGRADIENT_DISABLE = 0,				// disable primary gradient (same as OpenGL 'decal' texture blend)
	W3DSHADER_PRIGRADIENT_MODULATE,					// modulate fragment ARGB by gradient ARGB (default)
	W3DSHADER_PRIGRADIENT_ADD,							// add gradient RGB to fragment RGB, copy gradient A to fragment A
	W3DSHADER_PRIGRADIENT_BUMPENVMAP,				// environment-mapped bump mapping
	W3DSHADER_PRIGRADIENT_BUMPENVMAPLUMINANCE,	// environment-mapped bump mapping with luminance control
	W3DSHADER_PRIGRADIENT_MODULATE2X,				// modulate fragment ARGB by gradient ARGB and multiply RGB by 2
	W3DSHADER_PRIGRADIENT_MAX,							// end of enumeration

	W3DSHADER_SECGRADIENT_DISABLE = 0,				// don't draw secondary gradient (default)
	W3DSHADER_SECGRADIENT_ENABLE,						// add secondary gradient RGB to fragment RGB
	W3DSHADER_SECGRADIENT_MAX,							// end of enumeration

  	W3DSHADER_SRCBLENDFUNC_ZERO = 0,					// fragment not added to color buffer
  	W3DSHADER_SRCBLENDFUNC_ONE,						// fragment added unmodified to color buffer (default)
 	W3DSHADER_SRCBLENDFUNC_SRC_ALPHA,				// fragment RGB components multiplied by fragment A
 	W3DSHADER_SRCBLENDFUNC_ONE_MINUS_SRC_ALPHA,	// fragment RGB components multiplied by fragment inverse (one minus) A
	W3DSHADER_SRCBLENDFUNC_MAX,						// end of enumeration

	W3DSHADER_TEXTURING_DISABLE = 0,					// no texturing (treat fragment initial color as 1,1,1,1) (default)
	W3DSHADER_TEXTURING_ENABLE,						// enable texturing
	W3DSHADER_TEXTURING_MAX,							// end of enumeration

	W3DSHADER_DETAILCOLORFUNC_DISABLE = 0,			// local (default)
	W3DSHADER_DETAILCOLORFUNC_DETAIL,				// other
	W3DSHADER_DETAILCOLORFUNC_SCALE,					// local * other
	W3DSHADER_DETAILCOLORFUNC_INVSCALE,				// ~(~local * ~other) = local + (1-local)*other
	W3DSHADER_DETAILCOLORFUNC_ADD,					// local + other
	W3DSHADER_DETAILCOLORFUNC_SUB,					// local - other
	W3DSHADER_DETAILCOLORFUNC_SUBR,					// other - local
	W3DSHADER_DETAILCOLORFUNC_BLEND,					// (localAlpha)*local + (~localAlpha)*other
	W3DSHADER_DETAILCOLORFUNC_DETAILBLEND,			// (otherAlpha)*local + (~otherAlpha)*other
	W3DSHADER_DETAILCOLORFUNC_ADDSIGNED,			// (local + other - 0.5)
	W3DSHADER_DETAILCOLORFUNC_ADDSIGNED2X,			// (local + other - 0.5) * 2
	W3DSHADER_DETAILCOLORFUNC_SCALE2X,				// local * other * 2
	W3DSHADER_DETAILCOLORFUNC_MODALPHAADDCOLOR,	// local + localAlpha * other
	W3DSHADER_DETAILCOLORFUNC_MAX,					// end of enumeration

	W3DSHADER_DETAILALPHAFUNC_DISABLE = 0,			// local (default)
	W3DSHADER_DETAILALPHAFUNC_DETAIL,				// other
	W3DSHADER_DETAILALPHAFUNC_SCALE,					// local * other
	W3DSHADER_DETAILALPHAFUNC_INVSCALE,				// ~(~local * ~other) = local + (1-local)*other
	W3DSHADER_DETAILALPHAFUNC_MAX,					// end of enumeration

	W3DSHADER_DEPTHCOMPARE_DEFAULT = W3DSHADER_DEPTHCOMPARE_PASS_LEQUAL,
	W3DSHADER_DEPTHMASK_DEFAULT = W3DSHADER_DEPTHMASK_WRITE_ENABLE,
	W3DSHADER_ALPHATEST_DEFAULT = W3DSHADER_ALPHATEST_DISABLE,
	W3DSHADER_DESTBLENDFUNC_DEFAULT = W3DSHADER_DESTBLENDFUNC_ZERO,
	W3DSHADER_PRIGRADIENT_DEFAULT = W3DSHADER_PRIGRADIENT_MODULATE,
	W3DSHADER_SECGRADIENT_DEFAULT = W3DSHADER_SECGRADIENT_DISABLE,
	W3DSHADER_SRCBLENDFUNC_DEFAULT = W3DSHADER_SRCBLENDFUNC_ONE,
	W3DSHADER_TEXTURING_DEFAULT = W3DSHADER_TEXTURING_DISABLE,
	W3DSHADER_DETAILCOLORFUNC_DEFAULT = W3DSHADER_DETAILCOLORFUNC_DISABLE,
	W3DSHADER_DETAILALPHAFUNC_DEFAULT = W3DSHADER_DETAILALPHAFUNC_DISABLE,
};

enum PS2_SHADER_SETTINGS {
	PSS_SRC = 0,
	PSS_DEST,
	PSS_ZERO,

	PSS_SRC_ALPHA = 0,
	PSS_DEST_ALPHA,
	PSS_ONE,

	// From combo box.  To match the PC default gradient.
	PSS_PRIGRADIENT_DECAL = 0,
	PSS_PRIGRADIENT_MODULATE,
	PSS_PRIGRADIENT_HIGHLIGHT,
	PSS_PRIGRADIENT_HIGHLIGHT2,

	// Actual PS2 numbers.
	PSS_PS2_PRIGRADIENT_MODULATE = 0,
	PSS_PS2_PRIGRADIENT_DECAL,
	PSS_PS2_PRIGRADIENT_HIGHLIGHT,
	PSS_PS2_PRIGRADIENT_HIGHLIGHT2,


	PSS_DEPTHCOMPARE_PASS_NEVER = 0,
	PSS_DEPTHCOMPARE_PASS_LESS,
	PSS_DEPTHCOMPARE_PASS_ALWAYS,
	PSS_DEPTHCOMPARE_PASS_LEQUAL,
};

struct W3dShaderStruct
{
	W3dShaderStruct(void) {}

	uint8						DepthCompare;
	uint8						DepthMask;
	uint8						ColorMask;		// now obsolete and ignored
	uint8						DestBlend;
	uint8						FogFunc;			// now obsolete and ignored
	uint8						PriGradient;
	uint8						SecGradient;
	uint8						SrcBlend;
	uint8						Texturing;
	uint8						DetailColorFunc;
	uint8						DetailAlphaFunc;
	uint8						ShaderPreset;	// now obsolete and ignored
	uint8						AlphaTest;
	uint8						PostDetailColorFunc;
	uint8						PostDetailAlphaFunc;
	uint8						pad[1];

	// Required by DynamicVectorClass...
	bool			operator == (const W3dShaderStruct & that)
	{
		return (
			(DepthCompare == that.DepthCompare) &&
			(DepthMask == that.DepthMask) &&
			/*(ColorMask == that.ColorMask) &&       obsolete*/
			(DestBlend == that.DestBlend) &&
			/*(FogFunc == that.FogFunc) &&           obsolete*/
			(PriGradient == that.PriGradient) &&
			(SecGradient == that.SecGradient) &&
			(SrcBlend == that.SrcBlend) &&
			(Texturing == that.Texturing) &&
			(DetailColorFunc == that.DetailColorFunc) &&
			(DetailAlphaFunc == that.DetailAlphaFunc) &&
			/*(ShaderPreset == that.ShaderPreset) &&  obsolete */
			(AlphaTest == that.AlphaTest) );
	}

	bool			operator != (const W3dShaderStruct & that)
	{
		return !(*this == that);
	}
};

struct W3dPS2ShaderStruct
{
	uint8						DepthCompare;
	uint8						DepthMask;
	uint8						PriGradient;
	uint8						Texturing;
	uint8						AlphaTest;
	uint8						AParam;
	uint8						BParam;
	uint8						CParam;
	uint8						DParam;
	uint8						pad[3];
};

struct W3dFXShaderInfoStruct
{
	char	ShaderName[W3D_NAME_LEN * 2];
	uint8	Technique;
	uint8	Pad[3];
};

typedef enum
{
	CONSTANT_TYPE_TEXTURE = 1,
	CONSTANT_TYPE_FLOAT1 = 2,
	CONSTANT_TYPE_FLOAT2 = 3,
	CONSTANT_TYPE_FLOAT3 = 4,
	CONSTANT_TYPE_FLOAT4 = 5,
	CONSTANT_TYPE_INT = 6,
	CONSTANT_TYPE_BOOL = 7
} W3D_FX_SHADER_CONSTANT_TYPES;

inline void W3d_Shader_Reset(W3dShaderStruct * s)									{
																										s->DepthCompare = W3DSHADER_DEPTHCOMPARE_PASS_LEQUAL;
																										s->DepthMask = W3DSHADER_DEPTHMASK_WRITE_ENABLE;
																										s->ColorMask = 0;	// obsolete
																										s->DestBlend = W3DSHADER_DESTBLENDFUNC_ZERO;
																										s->FogFunc = 0;	// obsolete
																										s->PriGradient = W3DSHADER_PRIGRADIENT_MODULATE;
																										s->SecGradient = W3DSHADER_SECGRADIENT_DISABLE;
																										s->SrcBlend = W3DSHADER_SRCBLENDFUNC_ONE;
																										s->Texturing = W3DSHADER_TEXTURING_DISABLE;
																										s->DetailColorFunc = W3DSHADER_DETAILCOLORFUNC_DISABLE;
																										s->DetailAlphaFunc = W3DSHADER_DETAILALPHAFUNC_DISABLE;
																										s->ShaderPreset = 0;	// obsolete
																										s->AlphaTest = W3DSHADER_ALPHATEST_DISABLE;
																										s->PostDetailColorFunc = W3DSHADER_DETAILCOLORFUNC_DISABLE;
																										s->PostDetailAlphaFunc = W3DSHADER_DETAILALPHAFUNC_DISABLE;
																										s->pad[0] = 0;
																									}

inline void W3d_Shader_Set_Depth_Compare(W3dShaderStruct * s,int val)			 { s->DepthCompare = val; }
inline void W3d_Shader_Set_Depth_Mask(W3dShaderStruct * s,int val)				 { s->DepthMask = val; }
inline void W3d_Shader_Set_Dest_Blend_Func(W3dShaderStruct * s,int val)			 { s->DestBlend = val; }
inline void W3d_Shader_Set_Pri_Gradient(W3dShaderStruct * s,int val)				 { s->PriGradient = val; }
inline void W3d_Shader_Set_Sec_Gradient(W3dShaderStruct * s,int val)				 { s->SecGradient = val; }
inline void W3d_Shader_Set_Src_Blend_Func(W3dShaderStruct * s,int val)			 { s->SrcBlend = val; }
inline void W3d_Shader_Set_Texturing(W3dShaderStruct * s,int val)					 { s->Texturing = val; }
inline void W3d_Shader_Set_Detail_Color_Func(W3dShaderStruct * s,int val)		 { s->DetailColorFunc = val; }
inline void W3d_Shader_Set_Detail_Alpha_Func(W3dShaderStruct * s,int val)		 { s->DetailAlphaFunc = val; }
inline void W3d_Shader_Set_Alpha_Test(W3dShaderStruct * s,int val)				 { s->AlphaTest = val; }
inline void W3d_Shader_Set_Post_Detail_Color_Func(W3dShaderStruct * s,int val) { s->PostDetailColorFunc = val; }
inline void W3d_Shader_Set_Post_Detail_Alpha_Func(W3dShaderStruct * s,int val) { s->PostDetailAlphaFunc = val; }

// These functions use the existing W3dShaderStruct unused members to store PS2 specific parameters.
// At mesh save time, if a PS2 material was used, a separate PS2 material chunk is saved out.
// If W3dShaderStruct is changed this should still work because it is only used for the PS2 stuff.
inline void W3d_Shader_Set_PS2_Param_A(W3dShaderStruct *s, int val) {s->ColorMask = val;}
inline void W3d_Shader_Set_PS2_Param_B(W3dShaderStruct *s, int val) {s->FogFunc = val;}
inline void W3d_Shader_Set_PS2_Param_C(W3dShaderStruct *s, int val) {s->ShaderPreset = val;}
inline void W3d_Shader_Set_PS2_Param_D(W3dShaderStruct *s, int val) {s->pad[0] = val;}

inline int W3d_Shader_Get_PS2_Param_A(const W3dShaderStruct *s) {return (s->ColorMask);}
inline int W3d_Shader_Get_PS2_Param_B(const W3dShaderStruct *s) {return (s->FogFunc);}
inline int W3d_Shader_Get_PS2_Param_C(const W3dShaderStruct *s) {return (s->ShaderPreset);}
inline int W3d_Shader_Get_PS2_Param_D(const W3dShaderStruct *s) {return (s->pad[0]);}

inline int W3d_Shader_Get_PS2_Param_A(const W3dPS2ShaderStruct *s) {return (s->AParam);}
inline int W3d_Shader_Get_PS2_Param_B(const W3dPS2ShaderStruct *s) {return (s->BParam);}
inline int W3d_Shader_Get_PS2_Param_C(const W3dPS2ShaderStruct *s) {return (s->CParam);}
inline int W3d_Shader_Get_PS2_Param_D(const W3dPS2ShaderStruct *s) {return (s->DParam);}

inline int W3d_Shader_Get_Depth_Compare(const W3dPS2ShaderStruct * s)				 { return s->DepthCompare; }
inline int W3d_Shader_Get_Depth_Mask(const W3dPS2ShaderStruct * s)					 { return s->DepthMask; }
inline int W3d_Shader_Get_Pri_Gradient(const W3dPS2ShaderStruct * s)					 { return s->PriGradient; }
inline int W3d_Shader_Get_Texturing(const W3dPS2ShaderStruct * s)						 { return s->Texturing; }
inline int W3d_Shader_Get_Alpha_Test(const W3dPS2ShaderStruct * s)					 { return s->AlphaTest; }


inline int W3d_Shader_Get_Depth_Compare(const W3dShaderStruct * s)				 { return s->DepthCompare; }
inline int W3d_Shader_Get_Depth_Mask(const W3dShaderStruct * s)					 { return s->DepthMask; }
inline int W3d_Shader_Get_Dest_Blend_Func(const W3dShaderStruct * s)				 { return s->DestBlend; }
inline int W3d_Shader_Get_Pri_Gradient(const W3dShaderStruct * s)					 { return s->PriGradient; }
inline int W3d_Shader_Get_Sec_Gradient(const W3dShaderStruct * s)					 { return s->SecGradient; }
inline int W3d_Shader_Get_Src_Blend_Func(const W3dShaderStruct * s)				 { return s->SrcBlend; }
inline int W3d_Shader_Get_Texturing(const W3dShaderStruct * s)						 { return s->Texturing; }
inline int W3d_Shader_Get_Detail_Color_Func(const W3dShaderStruct * s)			 { return s->DetailColorFunc; }
inline int W3d_Shader_Get_Detail_Alpha_Func(const W3dShaderStruct * s)			 { return s->DetailAlphaFunc; }
inline int W3d_Shader_Get_Alpha_Test(const W3dShaderStruct * s)					 { return s->AlphaTest; }
inline int W3d_Shader_Get_Post_Detail_Color_Func(const W3dShaderStruct * s)	 { return s->PostDetailColorFunc; }
inline int W3d_Shader_Get_Post_Detail_Alpha_Func(const W3dShaderStruct * s)	 { return s->PostDetailAlphaFunc; }



/////////////////////////////////////////////////////////////////////////////////////////////
// Texture Animation parameters
// May occur inside a texture chunk if its needed
/////////////////////////////////////////////////////////////////////////////////////////////
#define W3DTEXTURE_PUBLISH					0x0001		// this texture should be "published" (indirected so its changeable in code)
#define W3DTEXTURE_RESIZE_OBSOLETE		0x0002		// this texture should be resizeable (OBSOLETE!!!)
#define W3DTEXTURE_NO_LOD					0x0004		// this texture should not have any LOD (mipmapping or resizing)
#define W3DTEXTURE_CLAMP_U					0x0008		// this texture should be clamped on U
#define W3DTEXTURE_CLAMP_V					0x0010		// this texture should be clamped on V
#define W3DTEXTURE_ALPHA_BITMAP			0x0020		// this texture's alpha channel should be collapsed to one bit

// Specify desired no. of mip-levels to be generated.
#define W3DTEXTURE_MIP_LEVELS_MASK		0x00c0
#define W3DTEXTURE_MIP_LEVELS_ALL		0x0000		// generate all mip-levels
#define W3DTEXTURE_MIP_LEVELS_2			0x0040		// generate up to 2 mip-levels (NOTE: use W3DTEXTURE_NO_LOD to generate just 1 mip-level)
#define W3DTEXTURE_MIP_LEVELS_3			0x0080		// generate up to 3 mip-levels
#define W3DTEXTURE_MIP_LEVELS_4			0x00c0		// generate up to 4 mip-levels

// Hints to describe the intended use of the various passes / stages
// This will go into the high byte of Attributes.
#define W3DTEXTURE_HINT_SHIFT				8				// number of bits to shift up
#define W3DTEXTURE_HINT_MASK				0x0000ff00	// mask for shifted hint value

#define W3DTEXTURE_HINT_BASE				0x0000		// base texture
#define W3DTEXTURE_HINT_EMISSIVE			0x0100		// emissive map
#define W3DTEXTURE_HINT_ENVIRONMENT		0x0200		// environment/reflection map
#define W3DTEXTURE_HINT_SHINY_MASK		0x0300		// shinyness mask map

#define W3DTEXTURE_TYPE_MASK				0x1000
#define W3DTEXTURE_TYPE_COLORMAP			0x0000		// Color map.
#define W3DTEXTURE_TYPE_BUMPMAP			0x1000		// Grayscale heightmap (to be converted to bumpmap).

// Animation types
#define W3DTEXTURE_ANIM_LOOP				0x0000
#define W3DTEXTURE_ANIM_PINGPONG			0x0001
#define W3DTEXTURE_ANIM_ONCE				0x0002
#define W3DTEXTURE_ANIM_MANUAL			0x0003

struct W3dTextureInfoStruct
{
	W3dTextureInfoStruct(void)	{}
	uint16					Attributes;					// flags for this texture
	uint16					AnimType;					// animation logic
	uint32					FrameCount;					// Number of frames (1 if not animated)
	float32					FrameRate;					// Frame rate, frames per second in floating point
};


/////////////////////////////////////////////////////////////////////////////////////////////
// A triangle, occurs inside the W3D_CHUNK_TRIANGLES chunk
// This was introduced with version 3.0 of the file format
/////////////////////////////////////////////////////////////////////////////////////////////
struct W3dTriStruct
{
	uint32					Vindex[3];			// vertex,vnormal,texcoord,color indices
	uint32					Attributes;			// attributes bits
	W3dVectorStruct		Normal;				// plane normal
	float32					Dist;					// plane distance
};

/////////////////////////////////////////////////////////////////////////////////////////////
// Flags for the Triangle Attributes member
/////////////////////////////////////////////////////////////////////////////////////////////
typedef enum
{
	SURFACE_TYPE_LIGHT_METAL = 0,
	SURFACE_TYPE_HEAVY_METAL,
	SURFACE_TYPE_WATER,
	SURFACE_TYPE_SAND,
	SURFACE_TYPE_DIRT,
	SURFACE_TYPE_MUD,
	SURFACE_TYPE_GRASS,
	SURFACE_TYPE_WOOD,
	SURFACE_TYPE_CONCRETE,
	SURFACE_TYPE_FLESH,
	SURFACE_TYPE_ROCK,
	SURFACE_TYPE_SNOW,
	SURFACE_TYPE_ICE,
	SURFACE_TYPE_DEFAULT,
	SURFACE_TYPE_GLASS,
	SURFACE_TYPE_CLOTH,
	SURFACE_TYPE_TIBERIUM_FIELD,
	SURFACE_TYPE_FOLIAGE_PERMEABLE,
	SURFACE_TYPE_GLASS_PERMEABLE,
	SURFACE_TYPE_ICE_PERMEABLE,
	SURFACE_TYPE_CLOTH_PERMEABLE,
	SURFACE_TYPE_ELECTRICAL,
	SURFACE_TYPE_FLAMMABLE,
	SURFACE_TYPE_STEAM,
	SURFACE_TYPE_ELECTRICAL_PERMEABLE,
	SURFACE_TYPE_FLAMMABLE_PERMEABLE,
	SURFACE_TYPE_STEAM_PERMEABLE,
	SURFACE_TYPE_WATER_PERMEABLE,
	SURFACE_TYPE_TIBERIUM_WATER,
	SURFACE_TYPE_TIBERIUM_WATER_PERMEABLE,
	SURFACE_TYPE_UNDERWATER_DIRT,
	SURFACE_TYPE_UNDERWATER_TIBERIUM_DIRT,

	SURFACE_TYPE_MAX			// NOTE: if you add a surface type, add it to the SurfaceEffects.INI file!
} W3D_SURFACE_TYPES;

const char * const SURFACE_TYPE_STRINGS[SURFACE_TYPE_MAX] =
{
	"Light Metal",
	"Heavy Metal",
	"Water",
	"Sand",
	"Dirt",
	"Mud",
	"Grass",
	"Wood",
	"Concrete",
	"Flesh",
	"Rock",
	"Snow",
	"Ice",
	"Default",
	"Glass",
	"Cloth",
	"Tiberium Field",
	"Foliage Permeable",
	"Glass Permeable",
	"Ice Permeable",
	"Cloth Permeable",
	"Electrical",
	"Electrical Permeable",
	"Flammable",
	"Flammable Permeable",
	"Steam",
	"Steam Permeable",
	"Water Permeable",
	"Tiberium Water",
	"Tiberium Water Permeable",
	"Underwater Dirt",
	"Underwater Tiberium Dirt",
};

/////////////////////////////////////////////////////////////////////////////////////////////
// Flags for the Mesh Attributes member
/////////////////////////////////////////////////////////////////////////////////////////////
#define W3D_MESH_FLAG_NONE										0x00000000		// plain ole normal mesh
#define W3D_MESH_FLAG_COLLISION_BOX							0x00000001		// (obsolete as of 4.1) mesh is a collision box (should be 8 verts, should be hidden, etc)
#define W3D_MESH_FLAG_SKIN										0x00000002		// (obsolete as of 4.1) skin mesh
#define W3D_MESH_FLAG_SHADOW									0x00000004		// (obsolete as of 4.1) intended to be projected as a shadow
#define W3D_MESH_FLAG_ALIGNED									0x00000008		// (obsolete as of 4.1) always aligns with camera

#define W3D_MESH_FLAG_COLLISION_TYPE_MASK					0x00000FF0		// mask for the collision type bits
#define W3D_MESH_FLAG_COLLISION_TYPE_SHIFT							4		// shifting to get to the collision type bits
#define W3D_MESH_FLAG_COLLISION_TYPE_PHYSICAL			0x00000010		// physical collisions
#define W3D_MESH_FLAG_COLLISION_TYPE_PROJECTILE			0x00000020		// projectiles (rays) collide with this
#define W3D_MESH_FLAG_COLLISION_TYPE_VIS					0x00000040		// vis rays collide with this mesh
#define W3D_MESH_FLAG_COLLISION_TYPE_CAMERA				0x00000080		// camera rays/boxes collide with this mesh
#define W3D_MESH_FLAG_COLLISION_TYPE_VEHICLE				0x00000100		// vehicles collide with this mesh (and with physical collision meshes)

#define W3D_MESH_FLAG_HIDDEN									0x00001000		// this mesh is hidden by default
#define W3D_MESH_FLAG_TWO_SIDED								0x00002000		// render both sides of this mesh
#define OBSOLETE_W3D_MESH_FLAG_LIGHTMAPPED				0x00004000		// obsolete lightmapped mesh
																							// NOTE: retained for backwards compatibility - use W3D_MESH_FLAG_PRELIT_* instead.
#define W3D_MESH_FLAG_CAST_SHADOW							0x00008000		// this mesh casts shadows

#define W3D_MESH_FLAG_GEOMETRY_TYPE_MASK					0x00FF0000		// (introduced with 4.1)
#define W3D_MESH_FLAG_GEOMETRY_TYPE_NORMAL				0x00000000		// (4.1+) normal mesh geometry
#define W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_ALIGNED		0x00010000		// (4.1+) camera aligned mesh
#define W3D_MESH_FLAG_GEOMETRY_TYPE_SKIN					0x00020000		// (4.1+) skin mesh
#define OBSOLETE_W3D_MESH_FLAG_GEOMETRY_TYPE_SHADOW	0x00030000		// (4.1+) shadow mesh OBSOLETE!
#define W3D_MESH_FLAG_GEOMETRY_TYPE_AABOX					0x00040000		// (4.1+) aabox OBSOLETE!
#define W3D_MESH_FLAG_GEOMETRY_TYPE_OBBOX					0x00050000		// (4.1+) obbox OBSOLETE!
#define W3D_MESH_FLAG_GEOMETRY_TYPE_CAMERA_ORIENTED	0x00060000		// (4.1+) camera oriented mesh (points _towards_ camera)

#define W3D_MESH_FLAG_PRELIT_MASK							0x0F000000		// (4.2+)
#define W3D_MESH_FLAG_PRELIT_UNLIT							0x01000000		// mesh contains an unlit material chunk wrapper
#define W3D_MESH_FLAG_PRELIT_VERTEX							0x02000000		// mesh contains a precalculated vertex-lit material chunk wrapper
#define W3D_MESH_FLAG_PRELIT_LIGHTMAP_MULTI_PASS		0x04000000		// mesh contains a precalculated multi-pass lightmapped material chunk wrapper
#define W3D_MESH_FLAG_PRELIT_LIGHTMAP_MULTI_TEXTURE	0x08000000		// mesh contains a precalculated multi-texture lightmapped material chunk wrapper

#define W3D_MESH_FLAG_SHATTERABLE							0x10000000		// this mesh is shatterable.
#define W3D_MESH_FLAG_NPATCHABLE								0x20000000		// it is ok to NPatch this mesh

/********************************************************************************

	Meshes

	Version 3 Mesh Header, trimmed out some of the junk that was in the
	previous versions.

********************************************************************************/

#define W3D_CURRENT_MESH_VERSION		W3D_MAKE_VERSION(4,2)

#define W3D_VERTEX_CHANNEL_LOCATION		0x00000001	// object-space location of the vertex
#define W3D_VERTEX_CHANNEL_NORMAL		0x00000002	// object-space normal for the vertex
#define W3D_VERTEX_CHANNEL_TEXCOORD		0x00000004	// texture coordinate
#define W3D_VERTEX_CHANNEL_COLOR			0x00000008	// vertex color
#define W3D_VERTEX_CHANNEL_BONEID		0x00000010	// per-vertex bone id for skins

#define W3D_FACE_CHANNEL_FACE				0x00000001	// basic face info, W3dTriStruct...

// boundary values for W3dMeshHeaderStruct::SortLevel
#define SORT_LEVEL_NONE						0
#define MAX_SORT_LEVEL						32
#define SORT_LEVEL_BIN1						10
#define SORT_LEVEL_BIN2						15
#define SORT_LEVEL_BIN3						20

struct W3dMeshHeader3Struct
{
	uint32					Version;
	uint32					Attributes;

	char						MeshName[W3D_NAME_LEN];
	char						ContainerName[W3D_NAME_LEN];

	//
	// Counts, these can be regarded as an inventory of what is to come in the file.
	//
	uint32					NumTris;				// number of triangles
	uint32					NumVertices;		// number of unique vertices
	uint32					NumMaterials;		// number of unique materials
	uint32					NumDamageStages;	// number of damage offset chunks
	sint32					SortLevel;			// static sorting level of this mesh
	uint32					PrelitVersion;		// mesh generated by this version of Lightmap Tool
	uint32					FutureCounts[1];	// future counts

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
// Deform information.  Each mesh can have sets of keyframes of
//	deform info associated with it.
//
struct W3dMeshDeform
{
	uint32					SetCount;
	uint32					AlphaPasses;
	uint32					reserved[3];
};

//
// Deform set information.  Each set is made up of a series
// of keyframes.
//
struct W3dDeformSetInfo
{
	uint32					KeyframeCount;
	uint32					flags;
	uint32					reserved[1];
};

#define W3D_DEFORM_SET_MANUAL_DEFORM	0x00000001	// set is isn't applied during sphere or point tests.

//
// Deform keyframe information.  Each keyframe is made up of
// a set of per-vert deform data.
//
struct W3dDeformKeyframeInfo
{
	float32					DeformPercent;
	uint32					DataCount;
	uint32					reserved[2];
};

//
// Deform data.  Contains deform information about a vertex
// in the mesh.
//
struct W3dDeformData
{
	uint32					VertexIndex;
	W3dVectorStruct		Position;
	W3dRGBAStruct			Color;
	uint32					reserved[2];
};

//
// AABTree header.  Each mesh can have an associated Axis-Aligned-Bounding-Box tree
// which is used for collision detection and certain rendering algorithms (like
// texture projection.
//
struct W3dMeshAABTreeHeader
{
	uint32					NodeCount;
	uint32					PolyCount;
	uint32					Padding[6];
};

//
// AABTree Node.  This is a node in the AABTree.
// If the MSB of FrontOrPoly0 is 1, then the node is a leaf and contains Poly0 and PolyCount
// else, the node is not a leaf and contains indices to its front and back children.  This matches
// the format used by AABTreeClass in WW3D.
//
struct W3dMeshAABTreeNode
{
	W3dVectorStruct		Min;						// min corner of the box
	W3dVectorStruct		Max;						// max corner of the box
	uint32					FrontOrPoly0;			// index of the front child or poly0 (if MSB is set, then leaf and poly0 is valid)
	uint32					BackOrPolyCount;		// index of the back child or polycount
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

#define W3D_CURRENT_HTREE_VERSION		W3D_MAKE_VERSION(4,1)

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
	uint32					ParentIdx;					// 0xffffffff = root pivot; no parent
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

#define W3D_CURRENT_HANIM_VERSION				W3D_MAKE_VERSION(4,1)
#define W3D_CURRENT_COMPRESSED_HANIM_VERSION	W3D_MAKE_VERSION(0,1)
#define W3D_CURRENT_MORPH_HANIM_VERSION		W3D_MAKE_VERSION(0,1)

struct W3dAnimHeaderStruct
{
	uint32					Version;
	char						Name[W3D_NAME_LEN];
	char						HierarchyName[W3D_NAME_LEN];
	uint32					NumFrames;
	uint32					FrameRate;

};

struct W3dCompressedAnimHeaderStruct
{
	uint32					Version;
	char						Name[W3D_NAME_LEN];
	char						HierarchyName[W3D_NAME_LEN];
	uint32					NumFrames;
	uint16					FrameRate;
	uint16					Flavor;
};


enum
{
	ANIM_CHANNEL_X = 0,
	ANIM_CHANNEL_Y,
	ANIM_CHANNEL_Z,
	ANIM_CHANNEL_XR,
	ANIM_CHANNEL_YR,
	ANIM_CHANNEL_ZR,
	ANIM_CHANNEL_Q,

	ANIM_CHANNEL_TIMECODED_X,
	ANIM_CHANNEL_TIMECODED_Y,
	ANIM_CHANNEL_TIMECODED_Z,
	ANIM_CHANNEL_TIMECODED_Q,

	ANIM_CHANNEL_ADAPTIVEDELTA_X,
	ANIM_CHANNEL_ADAPTIVEDELTA_Y,
	ANIM_CHANNEL_ADAPTIVEDELTA_Z,
	ANIM_CHANNEL_ADAPTIVEDELTA_Q,
};

//
// Flavor Enumerations
//
enum
{
    ANIM_FLAVOR_TIMECODED = 0,
    ANIM_FLAVOR_ADAPTIVE_DELTA,

	 ANIM_FLAVOR_VALID
};

// Begin Classic Structures

struct W3dAnimChannelStruct
{
	uint16					FirstFrame;
	uint16					LastFrame;
	uint16					VectorLen;			// length of each vector in this channel
	uint16					Flags;					// channel type.
	uint16					Pivot;					// pivot affected by this channel
	uint16					pad;
	float32					Data[1];				// will be (LastFrame - FirstFrame + 1) * VectorLen long
};

enum
{
	BIT_CHANNEL_VIS = 0,							// turn meshes on and off depending on anim frame.
	BIT_CHANNEL_TIMECODED_VIS,
};

struct W3dBitChannelStruct
{
	uint16					FirstFrame;			// all frames outside "First" and "Last" are assumed = DefaultVal
	uint16					LastFrame;
	uint16					Flags;					// channel type.
	uint16					Pivot;					// pivot affected by this channel
	uint8						DefaultVal;			// default state when outside valid range.
	uint8						Data[1];				// will be (LastFrame - FirstFrame + 1) / 8 long
};

// End Classic Structures
// Begin Time Coded Structures

// A time code is a uint32 that prefixes each vector
// the MSB is used to indicate a binary (non interpolated) movement

#define W3D_TIMECODED_BINARY_MOVEMENT_FLAG  0x80000000

struct W3dTimeCodedAnimChannelStruct
{
	uint32					NumTimeCodes;		// number of time coded entries
	uint16					Pivot;				// pivot affected by this channel
	uint8						VectorLen;			// length of each vector in this channel
	uint8						Flags;				// channel type.
	uint32					Data[1];				// will be (NumTimeCodes * ((VectorLen * sizeof(uint32)) + sizeof(uint32)))
};

// The bit channel is encoded right into the MSB of each time code
#define W3D_TIMECODED_BIT_MASK	0x80000000

struct W3dTimeCodedBitChannelStruct
{
	uint32					NumTimeCodes;  		// number of time coded entries
	uint16					Pivot;					// pivot affected by this channel
	uint8						Flags;					// channel type.
	uint8						DefaultVal;				// default state when outside valid range.
	uint32					Data[1];					// will be (NumTimeCodes * sizeof(uint32))
};

// End Time Coded Structures
// Begin AdaptiveDelta Structures
struct W3dAdaptiveDeltaAnimChannelStruct
{
	uint32					NumFrames;			// number of frames of animation
	uint16					Pivot;				// pivot effected by this channel
	uint8						VectorLen;			// num Channels
	uint8						Flags;				// channel type
	float						Scale;				// Filter Table Scale

	uint32					Data[1];				// OpCode Data Stream

};
// End AdaptiveDelta Structures

/********************************************************************************

	HMorphAnimClass

	This is an animation format which describes morphs between poses in another
	animation.  It is used for Renegade's facial animation system.  There is
	a normal anim which defines the pose for each phoneme and then a "Morph Anim"
	which defines the transitions between phonemes over time.  In addition there
	is the concept of multiple morph channels in a morph anim.  Each "channel"
	controls a set of pivots in the skeleton and has its own set of morph keys
	and poses.  This lets us have one set of poses for expressions and another
	for phonemes (a bone is only moved in one or the other anims though)

	The chunks used to describe a "morph" anim are as follows:

	W3D_CHUNK_MORPH_ANIMATION							=0x000002C0,	// wrapper for the entire anim
		W3D_CHUNK_MORPHANIM_HEADER,										// W3dMorphAnimHeaderStruct describes playback rate, number of frames, and type of compression
		W3D_CHUNK_MORPHANIM_CHANNEL,										// wrapper for a channel
			W3D_CHUNK_MORPHANIM_POSENAME,									// name of the other anim which contains the poses for this morph channel
			W3D_CHUNK_MORPHANIM_KEYDATA,									// array of W3dMorphAnimKeyStruct's (use chunk length to determine how many)
		W3D_CHUNK_MORPHANIM_PIVOTCHANNELDATA,							// uin32 per pivot in the htree, indicating which channel controls the pivot


********************************************************************************/
struct W3dMorphAnimHeaderStruct
{
	uint32					Version;
	char						Name[W3D_NAME_LEN];
	char						HierarchyName[W3D_NAME_LEN];
	uint32					FrameCount;
	float32					FrameRate;
	uint32					ChannelCount;
};

struct W3dMorphAnimKeyStruct
{
	uint32					MorphFrame;
	uint32					PoseFrame;
};



/********************************************************************************

	HModel - Hiearchical Model

	A Hierarchy Model is a set of render objects which should be attached to
	bones in a hierarchy tree.  There can be multiple objects per node
	in the tree.  Or there may be no objects attached to a particular bone.

	(gth) 09/22/2000 - Simplified the HModel file format.  The W3DHModelAuxDataStruct
	was un-needed and moved to w3d_obsolete.h.  The safe way to parse previous
	and current HModel formats is this:
	- Read in the header from W3D_CHUNK_HMODEL_HEADER
	- Allocate space for 'NumConnections' nodes that will follow
	- Read in the rest of the chunks
		- Create a sub-object for W3D_CHUNK_NODE, W3D_CHUNK_COLLISION_NODE, or
		  W3D_CHUNK_SKIN_NODE.
		- Skip the OBSOLETE_W3D_CHUNK_HMODEL_AUX_DATA and OBSOLETE_W3D_CHUNK_SHADOW_NODE

********************************************************************************/

#define W3D_CURRENT_HMODEL_VERSION				W3D_MAKE_VERSION(4,2)

struct W3dHModelHeaderStruct
{
	uint32					Version;
	char						Name[W3D_NAME_LEN];				// Name of this model
	char						HierarchyName[W3D_NAME_LEN];	// Name of the hierarchy tree this model uses
	uint16					NumConnections;
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

	Collection

	A collection chunk is generated when the user exports a bunch of meshes.
	The collection will be named with the root name of the w3d file and will
	contain a string chunk for the name of each render object in the collection.
	A collection may also contain a "Snap Points" chunk.

	W3D_CHUNK_COLLECTION
		W3D_CHUNK_COLLECTION_HEADER
		W3D_CHUNK_SNAP_POINTS
		W3D_CHUNK_COLLECTION_OBJ_NAME
		W3D_CHUNK_COLLECTION_OBJ_NAME
		W3D_CHUNK_COLLECTION_OBJ_NAME
		...

********************************************************************************/

#define W3D_CURRENT_COLLECTION_VERSION		W3D_MAKE_VERSION(4,2)

struct W3dCollectionHeaderStruct
{
	uint32		Version;
	char			Name[W3D_NAME_LEN];
	uint32		RenderObjectCount;
	uint32		pad[2];
};


/*
** Placeholder chunks.  Also known as "PROXIES".  These are used by the Renegade
** level editor to instruct the editor to instance a particular named object
*/
#define W3D_CURRENT_PLACEHOLDER_VERSION	W3D_MAKE_VERSION(1,0)

//
//	Note:  This structure is follwed directly by an array of char's 'name_len' in length
// which specify the name of the placeholder object in our Commando-level editor.
//
struct W3dPlaceholderStruct
{
	uint32		version;
	float32		transform[4][3];				// this is a direct dump of a MAX 3x4 matrix
	uint32		name_len;
};


/*
** Transform chunks.  These chunks refer to other W3D files which should be transformed by
** this file.  This feature is used to allow user to (for example) lightmap the interior
** of a building once and then just transform that into all of our levels that use it.
*/
#define W3D_CURRENT_TRANSFORM_VERSION			W3D_MAKE_VERSION(1,0)

//
//	Note:  This structure is followed directly by an array of char's 'name_len' in length
// which specifies the name of the file to apply the transform to.
//
struct W3dTransformNodeStruct
{
	uint32		version;
	float32		transform[4][3];				// this is a direct dump of a MAX 3x4 matrix
	uint32		name_len;
};



/********************************************************************************

	Lights

	The following structs are used to define lights in the w3d file.  Currently
	we have point lights, directional lights, and spot lights.

********************************************************************************/

#define W3D_CURRENT_LIGHT_VERSION			W3D_MAKE_VERSION(1,0)

#define W3D_LIGHT_ATTRIBUTE_TYPE_MASK						0x000000FF
#define W3D_LIGHT_ATTRIBUTE_POINT							0x00000001
#define W3D_LIGHT_ATTRIBUTE_DIRECTIONAL					0x00000002
#define W3D_LIGHT_ATTRIBUTE_SPOT								0x00000003
#define W3D_LIGHT_ATTRIBUTE_CAST_SHADOWS					0x00000100

struct W3dLightStruct
{
	uint32				Attributes;
	uint32				Unused; // Old exclusion bit deprecated
	W3dRGBStruct		Ambient;
	W3dRGBStruct		Diffuse;
	W3dRGBStruct		Specular;
	float32				Intensity;
};

struct W3dSpotLightStruct
{
	W3dVectorStruct	SpotDirection;
	float32				SpotAngle;
	float32				SpotExponent;
};

struct W3dLightAttenuationStruct
{
	float32				Start;
	float32				End;
};

struct W3dLightTransformStruct
{
	float32 Transform [3][4];
};


/********************************************************************************

	Particle emitters

	The following structs are used to define emitters in the w3d file.

********************************************************************************/

#define W3D_CURRENT_EMITTER_VERSION				0x00020000

//
//	This enum contains valid defines for the Type field
// of the W3dEmitterUserInfoStruct.  The programmer
// can add entries here that their specific loader
// can switch on to determine what type the emitter is.
//
// NOTE:  Please add a string the the EMITTER_TYPE_NAMES
// array when you add an entry to the enum.
//
enum
{
	EMITTER_TYPEID_DEFAULT = 0,
	EMITTER_TYPEID_COUNT
};

//
// The definition of this string array can be found in
// Part_Ldr.cpp.  Please update this array accordingly.
//
extern const char *EMITTER_TYPE_NAMES[EMITTER_TYPEID_COUNT];

struct W3dEmitterHeaderStruct
{
	uint32				Version;
	char					Name[W3D_NAME_LEN];
};

struct W3dEmitterUserInfoStruct
{
	uint32				Type;							// One of the EMITTER_TYPEID_ enum's defined above
	uint32				SizeofStringParam;		// Size (in bytes) of the following string data
	char					StringParam[1];			// Array of bytes.  Where "count = SizeofStringParam"
};

struct W3dEmitterInfoStruct
{
	char					TextureFilename[260];
	float32				StartSize;
	float32				EndSize;
	float32				Lifetime;
	float32				EmissionRate;
	float32				MaxEmissions;
	float32				VelocityRandom;
	float32				PositionRandom;
	float32				FadeTime;
	float32				Gravity;
	float32				Elasticity;
	W3dVectorStruct	Velocity;
	W3dVectorStruct	Acceleration;
	W3dRGBAStruct		StartColor;
	W3dRGBAStruct		EndColor;
};

struct W3dEmitterExtraInfoStruct
{
	float32				FutureStartTime; // Whether to start a particle in the future or not
	uint32				Padding[9];
};

struct W3dVolumeRandomizerStruct
{
	uint32				ClassID;
	float32				Value1;
	float32				Value2;
	float32				Value3;
	uint32				reserved[4];
};

#define W3D_EMITTER_RENDER_MODE_TRI_PARTICLES		0
#define W3D_EMITTER_RENDER_MODE_QUAD_PARTICLES		1
#define W3D_EMITTER_RENDER_MODE_LINE					2
#define W3D_EMITTER_RENDER_MODE_LINEGRP_TETRA		3
#define W3D_EMITTER_RENDER_MODE_LINEGRP_PRISM		4

#define W3D_EMITTER_FRAME_MODE_1x1						0
#define W3D_EMITTER_FRAME_MODE_2x2						1
#define W3D_EMITTER_FRAME_MODE_4x4						2
#define W3D_EMITTER_FRAME_MODE_8x8						3
#define W3D_EMITTER_FRAME_MODE_16x16					4

struct W3dEmitterInfoStructV2
{
	uint32							BurstSize;
	W3dVolumeRandomizerStruct	CreationVolume;
	W3dVolumeRandomizerStruct	VelRandom;
	float32							OutwardVel;
	float32							VelInherit;
	W3dShaderStruct				Shader;
	uint32							RenderMode;		// render as particles or lines?
	uint32							FrameMode;		// chop the texture into a grid of smaller squares?
	uint32							reserved[6];
};

// W3D_CHUNK_EMITTER_PROPS
// Contains a W3dEmitterPropertyStruct followed by a number of color keyframes,
// opacity keyframes, and size keyframes

struct W3dEmitterPropertyStruct
{
	uint32				ColorKeyframes;
	uint32				OpacityKeyframes;
	uint32				SizeKeyframes;
	W3dRGBAStruct		ColorRandom;
	float32				OpacityRandom;
	float32				SizeRandom;
	uint32				reserved[4];
};


struct W3dEmitterColorKeyframeStruct
{
	float32				Time;
	W3dRGBAStruct		Color;
};

struct W3dEmitterOpacityKeyframeStruct
{
	float32				Time;
	float32				Opacity;
};

struct W3dEmitterSizeKeyframeStruct
{
	float32				Time;
	float32				Size;
};

// W3D_CHUNK_EMITTER_ROTATION_KEYFRAMES
// Contains a W3dEmitterRotationHeaderStruct followed by a number of
// rotational velocity keyframes.
struct W3dEmitterRotationHeaderStruct
{
	uint32				KeyframeCount;
	float32				Random;					// random initial rotational velocity (rotations/sec)
	float32				OrientationRandom;	// random initial orientation (rotations, 1.0=360deg)
	uint32				Reserved[1];
};

struct W3dEmitterRotationKeyframeStruct
{
	float32				Time;
	float32				Rotation;				// rotational velocity in rotations/sec
};

// W3D_CHUNK_EMITTER_FRAME_KEYFRAMES
// Contains a W3dEmitterFrameHeaderStruct followed by a number of
// frame keyframes (sub-texture indexing)
struct W3dEmitterFrameHeaderStruct
{
	uint32				KeyframeCount;
	float32				Random;
	uint32				Reserved[2];
};

struct W3dEmitterFrameKeyframeStruct
{
	float32				Time;
	float32				Frame;
};

// W3D_CHUNK_EMITTER_BLUR_TIME_KEYFRAMES
// Contains a W3dEmitterFrameHeaderStruct followed by a number of
// frame keyframes (sub-texture indexing)
struct W3dEmitterBlurTimeHeaderStruct
{
	uint32				KeyframeCount;
	float32				Random;
	uint32				Reserved[1];
};

struct W3dEmitterBlurTimeKeyframeStruct
{
	float32				Time;
	float32				BlurTime;
};


// W3D_CHUNK_EMITTER_LINE_PROPERTIES
// Contains a W3dEmitterLinePropertiesStruct.
// Emiter Line Flags (used in the Flags field of W3dEmitterLinePropertiesStruct):
#define W3D_ELINE_MERGE_INTERSECTIONS 				0x00000001	// Merge intersections
#define W3D_ELINE_FREEZE_RANDOM						0x00000002	// Freeze random (note: offsets are in camera space)
#define W3D_ELINE_DISABLE_SORTING					0x00000004	// Disable sorting (even if shader has alpha-blending)
#define W3D_ELINE_END_CAPS 							0x00000008	// Draw end caps on the line
#define W3D_ELINE_TEXTURE_MAP_MODE_MASK 			0xFF000000	// Must cover all possible TextureMapMode values

#define W3D_ELINE_TEXTURE_MAP_MODE_OFFSET 		24				// By how many bits do I need to shift the texture mapping mode?
#define W3D_ELINE_UNIFORM_WIDTH_TEXTURE_MAP		0x00000000	// Entire line uses one row of texture (constant V)
#define W3D_ELINE_UNIFORM_LENGTH_TEXTURE_MAP 	0x00000001	// Entire line uses one row of texture stretched length-wise
#define W3D_ELINE_TILED_TEXTURE_MAP					0x00000002	// Tiled continuously over line

#define W3D_ELINE_DEFAULT_BITS	(W3D_ELINE_MERGE_INTERSECTIONS | (W3D_ELINE_UNIFORM_WIDTH_TEXTURE_MAP << W3D_ELINE_TEXTURE_MAP_MODE_OFFSET))


struct W3dEmitterLinePropertiesStruct
{
	uint32							Flags;
	uint32							SubdivisionLevel;
	float32							NoiseAmplitude;
	float32							MergeAbortFactor;
	float32							TextureTileFactor;
	float32							UPerSec;
	float32							VPerSec;
	uint32							Reserved[9];
};


/********************************************************************************

	Aggregate objects

	The following structs are used to define aggregates in the w3d file.  An
	'aggregate' is simply a 'shell' that contains references to a hierarchy
	model and subobjects to attach to its bones.

********************************************************************************/

#define W3D_CURRENT_AGGREGATE_VERSION			0x00010003
const int MESH_PATH_ENTRIES						= 15;
const int MESH_PATH_ENTRY_LEN						= (W3D_NAME_LEN * 2);

struct W3dAggregateHeaderStruct
{
	uint32				Version;
	char					Name[W3D_NAME_LEN];
};

struct W3dAggregateInfoStruct
{
	char					BaseModelName[W3D_NAME_LEN*2];
	uint32				SubobjectCount;
};

struct W3dAggregateSubobjectStruct
{
	char					SubobjectName[W3D_NAME_LEN*2];
	char					BoneName[W3D_NAME_LEN*2];
};

//
// Structures for version 1.1 and newer
//
struct W3dTextureReplacerHeaderStruct
{
	uint32				ReplacedTexturesCount;
};

struct W3dTextureReplacerStruct
{
	char						MeshPath[MESH_PATH_ENTRIES][MESH_PATH_ENTRY_LEN];
	char						BonePath[MESH_PATH_ENTRIES][MESH_PATH_ENTRY_LEN];
	char						OldTextureName[260];
	char						NewTextureName[260];
	W3dTextureInfoStruct	TextureParams;
};

//
// Flags used in the W3dAggregateMiscInfo structure
//
const int W3D_AGGREGATE_FORCE_SUB_OBJ_LOD		= 0x00000001;

//
// Structures for version 1.2 and newer
//
struct W3dAggregateMiscInfo
{
	uint32				OriginalClassID;
	uint32				Flags;
	uint32				reserved[3];
};


/********************************************************************************

	HLod (Hierarchical LOD Model)

	This is a hierarchical model which has multiple arrays of models which can
	be switched for LOD purposes.

	Relevant Chunks:
	----------------
	W3D_CHUNK_HLOD										=0x00000700,		// description of an HLod object (see HLodClass)
		W3D_CHUNK_HLOD_HEADER,												// general information such as name and version
		W3D_CHUNK_HLOD_LOD_ARRAY,											// wrapper around the array of objects for each level of detail
			W3D_CHUNK_HLOD_SUB_OBJECT_ARRAY_HEADER,					// info on the objects in this level of detail array
			W3D_CHUNK_HLOD_SUB_OBJECT,										// an object in this level of detail array
		W3D_CHUNK_HLOD_AGGREGATE_ARRAY,									// array of aggregates, contains W3D_CHUNK_SUB_OBJECT_ARRAY_HEADER and W3D_CHUNK_SUB_OBJECT_ARRAY
		W3D_CHUNK_HLOD_PROXY_ARRAY,										// array of proxies, used for application-defined purposes

	An HLOD is the basic hierarchical model format used by W3D.  It references
	an HTree for its hierarchical structure and animation data and several arrays
	of sub-objects; one for each LOD in the model.  In addition, it can contain
	an array of "aggregates" which are references to external W3D objects to
	be automatically attached into it.  And it can have a list of "proxy" objects
	which can be used for application purposes such as instantiating game objects
	at the specified transform.

********************************************************************************/

#define W3D_CURRENT_HLOD_VERSION			W3D_MAKE_VERSION(1,0)
#define NO_MAX_SCREEN_SIZE					WWMATH_FLOAT_MAX

struct W3dHLodHeaderStruct
{
	uint32					Version;
	uint32					LodCount;
	char						Name[W3D_NAME_LEN];
	char						HierarchyName[W3D_NAME_LEN];		// name of the hierarchy tree to use (\0 if none)
};

struct W3dHLodArrayHeaderStruct
{
	uint32					ModelCount;
	float32					MaxScreenSize;		// if model is bigger than this, switch to higher lod.
};

struct W3dHLodSubObjectStruct
{
	uint32					BoneIndex;
	char						Name[W3D_NAME_LEN*2];
};


/********************************************************************************

	Collision Boxes

	Collision boxes are meant to be used for, you guessed it, collision detection.
	For this reason, they only contain a minimal amount of rendering information
	(a color).

	Axis Aligned - This is a bounding box which is *always* aligned with the world
	coordinate system.  So, the center point is to be transformed by whatever
	transformation matrix is being used but the extents always point down the
	world space x,y, and z axes.  (in effect, you are translating the center).

	Oriented - This is an oriented 3D box.  It is aligned with the coordinate system
	it is in.  So its extents always point along the local coordinate system axes.

********************************************************************************/
#define W3D_BOX_CURRENT_VERSION								W3D_MAKE_VERSION(1,0)

#define W3D_BOX_ATTRIBUTE_ORIENTED							0x00000001
#define W3D_BOX_ATTRIBUTE_ALIGNED							0x00000002
#define W3D_BOX_ATTRIBUTE_COLLISION_TYPE_MASK			0x00000FF0		// mask for the collision type bits
#define W3D_BOX_ATTRIBUTE_COLLISION_TYPE_SHIFT						4		// shifting to get to the collision type bits
#define W3D_BOX_ATTRIBUTE_COLLISION_TYPE_PHYSICAL		0x00000010		// physical collisions
#define W3D_BOX_ATTRIBUTE_COLLISION_TYPE_PROJECTILE	0x00000020		// projectiles (rays) collide with this
#define W3D_BOX_ATTRIBUTE_COLLISION_TYPE_VIS				0x00000040		// vis rays collide with this mesh
#define W3D_BOX_ATTRIBUTE_COLLISION_TYPE_CAMERA			0x00000080		// cameras collide with this mesh
#define W3D_BOX_ATTRIBUTE_COLLISION_TYPE_VEHICLE		0x00000100		// vehicles collide with this mesh

struct W3dBoxStruct
{
	uint32				Version;						// file format version
	uint32				Attributes;					// box attributes (above #define's)
	char					Name[2*W3D_NAME_LEN];	// name is in the form <containername>.<boxname>
	W3dRGBStruct		Color;						// color to use when drawing the box
	W3dVectorStruct	Center;						// center of the box
	W3dVectorStruct	Extent;						// extent of the box
};




/********************************************************************************

	NULL Objects

	Null objects are used by the LOD system to make meshes dissappear at lower
	levels of detail.

********************************************************************************/
#define W3D_NULL_OBJECT_CURRENT_VERSION					W3D_MAKE_VERSION(1,0)

struct W3dNullObjectStruct
{
	uint32				Version;						// file format version
	uint32				Attributes;					// object attributes (currently un-used)
	uint32				pad[2];						// pad space
	char					Name[2*W3D_NAME_LEN];	// name is in the form <containername>.<boxname>
};


/********************************************************************************

	Dazzle Objects

	The only data needed to instantiate a dazzle object is the type-name of
	the dazzle to use.  The dazzle is always assumed to be at the pivot point
	of the bone it is attached to (you should enable Export_Transform) for
	dazzles.  If the dazzle-type (from dazzle.ini) is directional, then the
	coordinate-system of the bone will define the direction.

********************************************************************************/



/********************************************************************************

	Sound render objects

	The following structs are used to define sound render object in the w3d file.

	These objects are used to trigger a sound effect in the world.  When the object
	is shown, its associated sound is added to the world and played, when the object
	is hidden, the associated sound is stopped and removed from the world.


********************************************************************************/

#define W3D_CURRENT_SOUNDROBJ_VERSION			0x00010000

//
//	Note:  This structure is follwed directly by a chunk (W3D_CHUNK_SOUNDROBJ_DEFINITION)
// that contains an embedded AudibleSoundDefinitionClass's storage.  See audibledound.h
// for details.
//
struct W3dSoundRObjHeaderStruct
{
	uint32				Version;
	char					Name[W3D_NAME_LEN];
	uint32				Flags;
	uint32				Padding[8];
};

/********************************************************************************

	ShdMesh Render Objects

	The following structs are used to define a ShdMesh in a w3d file.  This class
	is the mesh class used with the scaleable shader system.  It contains a number
	of sub-meshes; each which use a single shader.

	NOTE: ShdMeshes re-use the following chunks from regular meshes:

  			W3D_CHUNK_VERTICES
			W3D_CHUNK_VERTEX_NORMALS
			W3D_CHUNK_TRIANGLES
			W3D_CHUNK_MESH_USER_TEXT
			W3D_CHUNK_VERTEX_INFLUENCES
			W3D_CHUNK_VERTEX_SHADE_INDICES
			W3D_CHUNK_AABTREE

********************************************************************************/

#define W3D_CURRENT_SHDMESH_VERSION			0x00010000

/**
** Header for a ShdMesh
*/
struct W3dShdMeshHeaderStruct
{
	uint32					Version;
	uint32					Attributes;			// Uses same #defines as Mesh, e.g. W3D_MESH_FLAG_GEOMETRY_TYPE_SKIN

	//
	// Counts, these can be regarded as an inventory of what is to come in the file.
	//
	uint32					NumTris;				// number of triangles
	uint32					NumVertices;		// number of unique vertices
	uint32					NumSubMeshes;		// number of sub-meshes
	uint32					FutureCounts[5];	// future counts

	//
	// Bounding volumes
	//
	W3dVectorStruct		BoxMin;				// Min corner of the bounding box
	W3dVectorStruct		BoxMax;				// Max corner of the bounding box
	W3dVectorStruct		SphCenter;			// Center of bounding sphere
	float32					SphRadius;			// Bounding sphere radius
};

/**
** Header for a sub-mesh inside an ShdMesh
*/
struct W3dShdSubMeshHeaderStruct
{
	//
	// Counts, these can be regarded as an inventory of what is to come in the file.
	//
	uint32					NumTris;				// number of triangles
	uint32					NumVertices;		// number of unique vertices
	uint32					FutureCounts[2];	// future counts

	//
	// Bounding volumes
	//
	W3dVectorStruct		BoxMin;				// Min corner of the bounding box
	W3dVectorStruct		BoxMax;				// Max corner of the bounding box
	W3dVectorStruct		SphCenter;			// Center of bounding sphere
	float32					SphRadius;			// Bounding sphere radius
};

/*
** Include the obsolete structures and chunk ID's
*/
#include "w3d_obsolete.h"
