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
 *                     $Archive:: /Commando/Code/ww3d2/meshmdl.h                              $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 11/24/01 6:17p                                              $*
 *                                                                                             *
 *                    $Revision:: 40                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "vector2.h"
#include "vector3.h"
#include "vector4.h"
#include "Vector3i.h"
#include "sharebuf.h"
#include "shader.h"
#include "wwdebug.h"
#include "vertmaterial.h"
#include "bittype.h"
#include "colmath.h"
#include "simplevec.h"
#include "wwstring.h"
#include "rinfo.h"
#include "meshgeometry.h"
#include "meshmatdesc.h"
#include "dx8list.h"

class TextureClass;
class RenderInfoClass;
class SpecialRenderInfoClass;
class MatBufferClass;
class TexBufferClass;
class AABoxClass;
class OBBoxClass;
class FrustumClass;
class SphereClass;
class AABTreeClass;
class MaterialInfoClass;
class MeshLoadContextClass;
class MeshSaveContextClass;
class ChunkLoadClass;
class ChunkSaveClass;
class MeshClass;
class HTreeClass;
class DecalGeneratorClass;
class LightEnvironmentClass;

class DX8MeshRendererClass;
class DX8PolygonRendererAttachClass;
class DX8SkinFVFCategoryContainer;
class GapFillerClass;

struct VertexFormatXYZNDUV2;

/**
** MeshModelClass
** This class is a repository for all of the geometry information that defines the mesh.
** Its purpose is to allow separate instances of a mesh to share as much data as possible.
**
** There are some tricky aspects to this class that may not be immediately obvious.  The
** arrays of pointers to textures and vertex materials must be handled in a special way
** due to the fact that they are also ref-counted objects which should only be released
** when the last reference to the array is released (i.e. when no one is using the array
** any more...)
**
** Copy/Add_Ref Rules:
** The purpose of this model was to share data between models whenever possible.  To this
** end, some of the arrays of data are handled differently:
**
** ALWAYS SHARED: These are *ALWAYS* Add_Ref'd and thus all point to the same array
** Poly - Connectivity of a mesh are always shared (cannot be changed at runtime)
** VertexShadeIdx - Shade indices of a mesh are always shared (cannot be changed at runtime)
** VertexInfluences - Vertex bone attachments are always shared (cannot be changed at runtime)
**
** SHARED UNTIL SCALED, SKIN DEFORMED, or DAMAGED:
** Vertex - vertex positions must be copied if any are moved...
** VertexNorm - vertex normals cannot be shared if a vertex is moved
** PlaneEq - plane equations cannot be shared if a vertex is moved
** CullTree - culling tree becomes instance specific if a vertex moves (shouldn't even use this with skins...)
**
** ALWAYS UNIQUE, BUT SHARE ARRAYS BETWEEN ALTERNATE MATERIAL REPRESENTATIONS (should we share some of these?)
** UV, DIG, DCG, SCG
** Texture, Shader, Material,
** TextureArray, MaterialArray, ShaderArray
*/


/**
** GapFillerClass
** This class is used to generate gap-filling polygons for "N-Patched" meshes
*/
class GapFillerClass : public W3DMPO
{
	W3DMPO_GLUE(GapFillerClass)

	TriIndex* PolygonArray;
	unsigned PolygonCount;
	unsigned ArraySize;
	TextureClass** TextureArray[MeshMatDescClass::MAX_PASSES][MeshMatDescClass::MAX_TEX_STAGES];
	VertexMaterialClass** MaterialArray[MeshMatDescClass::MAX_PASSES];
	ShaderClass* ShaderArray[MeshMatDescClass::MAX_PASSES];
	MeshModelClass* mmc;

	GapFillerClass& operator = (const GapFillerClass&) CPP_11(= delete);
public:
	GapFillerClass(MeshModelClass* mmc);
	GapFillerClass(const GapFillerClass& that);
	~GapFillerClass();

	WWINLINE const TriIndex* Get_Polygon_Array() const { return PolygonArray; }
	WWINLINE unsigned Get_Polygon_Count() const { return PolygonCount; }
	WWINLINE TextureClass** Get_Texture_Array(int pass, int stage) const { return TextureArray[pass][stage]; }
	WWINLINE VertexMaterialClass** Get_Material_Array(int pass) const { return MaterialArray[pass]; }
	WWINLINE ShaderClass* Get_Shader_Array(int pass) const { return ShaderArray[pass]; }

	void Add_Polygon(unsigned polygon_index,unsigned vidx1,unsigned vidx2, unsigned vidx3);
	void Shrink_Buffers();
};

class MeshModelClass : public MeshGeometryClass
{
	W3DMPO_GLUE(MeshModelClass)

public:

	MeshModelClass(void);
	MeshModelClass(const MeshModelClass & that);
	~MeshModelClass(void);

	MeshModelClass & operator = (const MeshModelClass & that);
	void							Reset(int polycount,int vertcount,int passcount);
	void							Register_For_Rendering();
	void							Shadow_Render(SpecialRenderInfoClass & rinfo,const Matrix3D & tm,const HTreeClass * htree);

	/////////////////////////////////////////////////////////////////////////////////////
	// Material interface, All of these functions call through to the current
	// material description.
	/////////////////////////////////////////////////////////////////////////////////////
	void							Set_Pass_Count(int passes)														{ CurMatDesc->Set_Pass_Count(passes); }
	int							Get_Pass_Count(void) const														{ return CurMatDesc->Get_Pass_Count(); }

	const Vector2 *			Get_UV_Array(int pass = 0, int stage = 0)									{ return CurMatDesc->Get_UV_Array(pass,stage); }
	int							Get_UV_Array_Count(void)														{ return CurMatDesc->Get_UV_Array_Count(); }
	const Vector2 *			Get_UV_Array_By_Index(int index)												{ return CurMatDesc->Get_UV_Array_By_Index(index, false); }

	unsigned *					Get_DCG_Array(int pass)															{ return CurMatDesc->Get_DCG_Array(pass); }
	unsigned *					Get_DIG_Array(int pass)															{ return CurMatDesc->Get_DIG_Array(pass); }
	VertexMaterialClass::ColorSourceType Get_DCG_Source(int pass)										{ return CurMatDesc->Get_DCG_Source(pass); }
	VertexMaterialClass::ColorSourceType Get_DIG_Source(int pass)										{ return CurMatDesc->Get_DIG_Source(pass); }

	unsigned *					Get_Color_Array(int array_index,bool create = true)					{ return CurMatDesc->Get_Color_Array(array_index,create); }

	void							Set_Single_Material(VertexMaterialClass * vmat,int pass=0)			{ CurMatDesc->Set_Single_Material(vmat,pass); }
	void							Set_Single_Texture(TextureClass * tex,int pass=0,int stage=0)		{ CurMatDesc->Set_Single_Texture(tex,pass,stage); }
	void							Set_Single_Shader(ShaderClass shader,int pass=0)						{ CurMatDesc->Set_Single_Shader(shader,pass); }

	// the "Get" functions add a reference before returning the pointer (if appropriate)
	VertexMaterialClass *	Get_Single_Material(int pass=0) const										{ return CurMatDesc->Get_Single_Material(pass); }
	TextureClass *				Get_Single_Texture(int pass=0,int stage=0) const						{ return CurMatDesc->Get_Single_Texture(pass,stage); }
	ShaderClass					Get_Single_Shader(int pass=0) const											{ return CurMatDesc->Get_Single_Shader(pass); }

	// the "Peek" functions just return the pointer and it's the caller's responsibility to
	// maintain a reference to an object with a reference to the data
	VertexMaterialClass *	Peek_Single_Material(int pass=0) const										{ return CurMatDesc->Peek_Single_Material(pass); }
	TextureClass *				Peek_Single_Texture(int pass=0,int stage=0) const						{ return CurMatDesc->Peek_Single_Texture(pass,stage); }

	void							Set_Material(int vidx,VertexMaterialClass * vmat,int pass=0)		{ CurMatDesc->Set_Material(vidx,vmat,pass); }
	void							Set_Shader(int pidx,ShaderClass shader,int pass=0)						{ CurMatDesc->Set_Shader(pidx,shader,pass); }
	void							Set_Texture(int pidx,TextureClass * tex,int pass=0,int stage=0)	{ CurMatDesc->Set_Texture(pidx,tex,pass,stage); }

	// Queries for determining whether this model has per-polygon arrays of Materials, Shaders, or Textures
	bool							Has_Material_Array(int pass) const											{ return CurMatDesc->Has_Material_Array(pass); }
	bool							Has_Shader_Array(int pass) const												{ return CurMatDesc->Has_Shader_Array(pass); }
	bool							Has_Texture_Array(int pass,int stage) const								{ return CurMatDesc->Has_Texture_Array(pass,stage); }

	// "Get" functions for Materials, Textures, and Shaders when there are more than one (per-polygon/per-vertex)
	VertexMaterialClass *	Get_Material(int vidx,int pass=0) const									{ return CurMatDesc->Get_Material(vidx,pass); }
	TextureClass *				Get_Texture(int pidx,int pass=0,int stage=0) const						{ return CurMatDesc->Get_Texture(pidx,pass,stage); }
	ShaderClass					Get_Shader(int pidx,int pass=0) const										{ return CurMatDesc->Get_Shader(pidx,pass); }

	// "Peek" functions for Materials and Textures when there are more than one (per-polygon/per-vertex)
	VertexMaterialClass *	Peek_Material(int vidx,int pass=0) const									{ return CurMatDesc->Peek_Material(vidx,pass); }
	TextureClass *				Peek_Texture(int pidx,int pass=0,int stage=0) const					{ return CurMatDesc->Peek_Texture(pidx,pass,stage); }

	void							Replace_Texture(TextureClass* texture,TextureClass* new_texture);
	void							Replace_VertexMaterial(VertexMaterialClass* vmat,VertexMaterialClass* new_vmat);

	/////////////////////////////////////////////////////////////////////////////////////
	// Modification interface.  Call these functions to cause the model to ensure
	// that the specified array is unique to this instance.  (I.e. if the specified
	// data is being shared, break the link!)
	/////////////////////////////////////////////////////////////////////////////////////
	void							Make_Geometry_Unique();
	void							Make_UV_Array_Unique(int pass=0,int stage=0);
	void							Make_Color_Array_Unique(int array_index=0);

	// Load the w3d file format
	WW3DErrorType				Load_W3D(ChunkLoadClass & cload);

	/////////////////////////////////////////////////////////////////////////////////////
	//	Decal interface
	/////////////////////////////////////////////////////////////////////////////////////
	void							Create_Decal(DecalGeneratorClass * generator, MeshClass * parent);
	void							Delete_Decal(uint32 decal_id);

	/////////////////////////////////////////////////////////////////////////////////////
	//	Alternate Material Description Interface
	// Some models will allow you to alternate between multiple material descriptions
	/////////////////////////////////////////////////////////////////////////////////////
	void							Enable_Alternate_Material_Description(bool onoff);
	bool							Is_Alternate_Material_Description_Enabled(void);

	// Process texture reductions
//	void							Process_Texture_Reduction(void);

	// FVF category container will be null if the mesh hasn't been registered to the rendering system
	DX8FVFCategoryContainer* Peek_FVF_Category_Container();

	// Determine whether any rendering feature used by this mesh requires vertex normals
	bool							Needs_Vertex_Normals(void);

	void							Init_For_NPatch_Rendering();
	const GapFillerClass*	Get_Gap_Filler() const { return GapFiller; }

	bool							Has_Polygon_Renderers(void) { return !PolygonRendererList.Is_Empty(); }

protected:

	// MeshClass will set this for skins so that they can get the bone transforms
	void							Set_HTree(const HTreeClass * htree);

public: // Jani: I need to have an access to these for now...

	TexBufferClass *			Get_Texture_Array(int pass,int stage,bool create = true)
	{
		return CurMatDesc->Get_Texture_Array(pass,stage,create);
	}
	MatBufferClass *			Get_Material_Array(int pass,bool create = true)
	{
		return CurMatDesc->Get_Material_Array(pass,create);
	}
	ShaderClass *				Get_Shader_Array(int pass,bool create = true)
	{
		return CurMatDesc->Get_Shader_Array(pass,create);
	}

protected:

	int Register_Type();

	// loading
	WW3DErrorType read_chunks(ChunkLoadClass & cload,MeshLoadContextClass * context);
	WW3DErrorType read_texcoords(ChunkLoadClass & cload,MeshLoadContextClass * context);
	WW3DErrorType read_materials(ChunkLoadClass & cload,MeshLoadContextClass * context);
	WW3DErrorType read_v2_materials(ChunkLoadClass & cload,MeshLoadContextClass * context);
	WW3DErrorType read_v3_materials(ChunkLoadClass & cload,MeshLoadContextClass * context);
	WW3DErrorType read_per_tri_materials(ChunkLoadClass & cload,MeshLoadContextClass * context);
	WW3DErrorType read_vertex_colors(ChunkLoadClass & cload,MeshLoadContextClass * context);
	WW3DErrorType read_material_info(ChunkLoadClass & cload,MeshLoadContextClass * context);
	WW3DErrorType read_shaders(ChunkLoadClass & cload,MeshLoadContextClass * context);
	WW3DErrorType read_vertex_materials(ChunkLoadClass & cload,MeshLoadContextClass * context);
	WW3DErrorType read_textures(ChunkLoadClass & cload,MeshLoadContextClass * context);
	WW3DErrorType read_material_pass(ChunkLoadClass & cload,MeshLoadContextClass * context);
	WW3DErrorType read_vertex_material_ids(ChunkLoadClass & cload,MeshLoadContextClass * context);
	WW3DErrorType read_shader_ids(ChunkLoadClass & cload,MeshLoadContextClass * context);
	WW3DErrorType read_scg(ChunkLoadClass & cload,MeshLoadContextClass * context);
	WW3DErrorType read_dig(ChunkLoadClass & cload,MeshLoadContextClass * context);
	WW3DErrorType read_dcg(ChunkLoadClass & cload,MeshLoadContextClass * context);
	WW3DErrorType read_texture_stage(ChunkLoadClass & cload,MeshLoadContextClass * context);
	WW3DErrorType read_texture_ids(ChunkLoadClass & cload,MeshLoadContextClass * context);
	WW3DErrorType read_stage_texcoords(ChunkLoadClass & cload,MeshLoadContextClass * context);
	WW3DErrorType read_per_face_texcoord_ids (ChunkLoadClass &cload, MeshLoadContextClass *context);
	WW3DErrorType read_prelit_material (ChunkLoadClass &cload, MeshLoadContextClass *context);

	// post-processing
	void post_process(void);
	void post_process_fog(void);

	unsigned int get_sort_flags(int pass) const;
	unsigned int get_sort_flags(void) const;
	void compute_static_sort_levels(void);
	void modify_for_overbright(void);

	// mat info support
	void install_materials(MeshLoadContextClass * loadinfo);
	void clone_materials(const MeshModelClass & srcmesh);
	void install_alternate_material_desc(MeshLoadContextClass * context);

	// Material Descriptions
	// DefMatDesc - the default material description, allocated in constructor, always present.
	// AlternateMatDes - an optional alternate material description, allocated at load time if needed
	// CurMatDesc - pointer to the currently active material description.
	MeshMatDescClass *									DefMatDesc;
	MeshMatDescClass *									AlternateMatDesc;
	MeshMatDescClass *									CurMatDesc;

	// Collection of the unique materials in the mesh
	MaterialInfoClass	*									MatInfo;

	// DX8 Mesh rendering system data
	DX8PolygonRendererList								PolygonRendererList;

	// Jani: Adding this here temporarily... must fine better place
	GapFillerClass *										GapFiller;
	bool														HasBeenInUse;	// For debugging purposes!

	friend class MeshClass;
	friend class MeshDeformSetClass;
	friend class MeshDeformClass;
	friend class MeshLoadContextClass;
	friend class DX8SkinFVFCategoryContainer;
	friend class DX8MeshRendererClass;
	friend class DX8PolygonRendererClass;
};
