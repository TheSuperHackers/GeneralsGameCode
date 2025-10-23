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
 *                     $Archive:: /Commando/Code/ww3d2/decalmsh.h                             $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             *
 *                     $Modtime:: 11/24/01 6:18p                                              $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "always.h"
#include "bittype.h"
#include "simplevec.h"
#include "Vector.H"
#include "vector2.h"
#include "vector3.h"
#include "Vector3i.h"
#include "vector4.h"
#include "shader.h"
#include "vertmaterial.h"
#include "meshgeometry.h"

class MeshClass;
class RenderInfoClass;
class DecalGeneratorClass;
class DecalSystemClass;
class OBBoxClass;
class TextureClass;

/**
** DecalMeshClass
** This is a "subordinate" class to MeshModel which simply adds "decal" polygons to the mesh.
** These polygons will always be exact copies of polygons already in the parent mesh.
**
** Design Goals:
** - Each decal can have its own material settings
** - Dynamically growing array of decals
** - Each decal is assigned a unique "logical-decal-id" by the decal manager
** - A decal mesh may be instructed to remove a specified "logical-decal"
**
** DecalMeshClass is an abstract base class from which we derive concrete classes.
*/

class DecalMeshClass : public RefCountClass
{
public:

	DecalMeshClass(MeshClass * parent,DecalSystemClass * system);
	virtual ~DecalMeshClass(void);

	// world_vertex_locs and world_vertex_norms are dynamically updated worldspace vertex data
	// which are used by some decal types which cannot use static object geometry (such as decals
	// for skins, procedurally generated meshes, etc.)

	virtual void											Render(void) = 0;

	virtual bool											Create_Decal(	DecalGeneratorClass * generator,
																					const OBBoxClass & localbox,
																					SimpleDynVecClass<uint32> & apt,
																					const DynamicVectorClass<Vector3> * world_vertex_locs = 0) = 0;

	virtual bool											Delete_Decal(uint32 id) = 0;

	virtual int												Decal_Count(void) = 0;
	virtual uint32											Get_Decal_ID(int decal_index) = 0;

	MeshClass *												Peek_Parent(void);
	DecalSystemClass *									Peek_System(void);

	DecalMeshClass *										Peek_Next_Visible(void) { return NextVisible; }
	void														Set_Next_Visible(DecalMeshClass * mesh) { NextVisible = mesh; }

protected:

	/*
	** Members
	*/
	MeshClass *												Parent;
	DecalSystemClass *									DecalSystem;
	DecalMeshClass *										NextVisible;
};


/*
** RigidDecalMeshClass: a concrete class derived from DecalMeshClass which is
** used for decals on rigid (non-skin) meshes.
*/

class RigidDecalMeshClass : public DecalMeshClass
{
public:

	RigidDecalMeshClass(MeshClass * parent,DecalSystemClass * system);
	virtual ~RigidDecalMeshClass(void);

	// Rigid decal meshes have static geometry so they do not use world_vertex_locs/norms

	virtual void											Render(void);

	virtual bool											Create_Decal(	DecalGeneratorClass * generator,
																					const OBBoxClass & localbox,
																					SimpleDynVecClass<uint32> & apt,
																					const DynamicVectorClass<Vector3> * world_vertex_locs = 0);

	virtual bool											Delete_Decal(uint32 id);

	int														Decal_Count(void);
	uint32													Get_Decal_ID(int decal_index);

protected:

	int														Process_Material_Run(int start_index);

	/*
	** Connectivity
	*/
	SimpleDynVecClass<TriIndex>						Polys;

	/*
	** Geometry
	*/
	SimpleDynVecClass<Vector3>							Verts;
	SimpleDynVecClass<Vector3>							VertNorms;

	/*
	** Materials
	*/
	SimpleDynVecClass<ShaderClass>					Shaders;
	SimpleDynVecClass<TextureClass *>				Textures;
	SimpleDynVecClass<VertexMaterialClass *>		VertexMaterials;
	SimpleDynVecClass<Vector2>							TexCoords;

	/*
	** Decal Organization
	*/
	struct DecalStruct
	{
		uint32	DecalID;
		uint16	VertexStartIndex;
		uint16	VertexCount;
		uint16	FaceStartIndex;
		uint16	FaceCount;
	};

	SimpleDynVecClass<DecalStruct>					Decals;
};


/*
** SkinDecalMeshClass: a concrete class derived from DecalMeshClass which is
** used for decals on skin meshes.
*/

class SkinDecalMeshClass : public DecalMeshClass
{
public:

	SkinDecalMeshClass(MeshClass * parent,DecalSystemClass * system);
	virtual ~SkinDecalMeshClass(void);

	// Skin decals use world_vertex_locs/norms since they cannot use static geometry

	virtual void											Render(void);

	virtual bool											Create_Decal(	DecalGeneratorClass * generator,
																					const OBBoxClass & localbox,
																					SimpleDynVecClass<uint32> & apt,
																					const DynamicVectorClass<Vector3> * world_vertex_locs);

	virtual bool											Delete_Decal(uint32 id);

	int														Decal_Count(void);
	uint32													Get_Decal_ID(int decal_index);

protected:

	int														Process_Material_Run(int start_index);

	/*
	** Connectivity
	*/
	SimpleDynVecClass<TriIndex>						Polys;

	/*
	** Indirected vertex indices (for copying dynamically updated mesh geometry)
	*/
	SimpleDynVecClass<uint32> 							ParentVertexIndices;

	/*
	** Materials
	*/
	SimpleDynVecClass<ShaderClass>					Shaders;
	SimpleDynVecClass<TextureClass *>				Textures;
	SimpleDynVecClass<VertexMaterialClass *>		VertexMaterials;
	SimpleDynVecClass<Vector2>							TexCoords;

	/*
	** Decal Organization
	*/
	struct DecalStruct
	{
		uint32	DecalID;
		uint16	VertexStartIndex;
		uint16	VertexCount;
		uint16	FaceStartIndex;
		uint16	FaceCount;
	};

	SimpleDynVecClass<DecalStruct>					Decals;
};


/*
** DecalMeshClass inline functions
*/

inline MeshClass * DecalMeshClass::Peek_Parent(void)
{
	return Parent;
}

inline DecalSystemClass * DecalMeshClass::Peek_System(void)
{
	return DecalSystem;
}


/*
** RigidDecalMeshClass inline functions
*/

inline int RigidDecalMeshClass::Decal_Count(void)
{
	return Decals.Count();
}

inline uint32 RigidDecalMeshClass::Get_Decal_ID(int decal_index)
{
	return Decals[decal_index].DecalID;
}


/*
** SkinDecalMeshClass inline functions
*/

inline int SkinDecalMeshClass::Decal_Count(void)
{
	return Decals.Count();
}

inline uint32 SkinDecalMeshClass::Get_Decal_ID(int decal_index)
{
	return Decals[decal_index].DecalID;
}
