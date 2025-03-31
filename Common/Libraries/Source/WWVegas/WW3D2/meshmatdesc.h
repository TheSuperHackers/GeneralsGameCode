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
 *                     $Archive:: /Commando/Code/ww3d2/meshmatdesc.h                          $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 1/18/02 3:08p                                               $*
 *                                                                                             *
 *                    $Revision:: 14                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef MESHMATDESC_H
#define MESHMATDESC_H

#include "always.h"
#include "vector2.h"
#include "vector3.h"
#include "Vector3i.h"
#include "vector4.h"
#include "sharebuf.h"
#include "shader.h"
#include "vertmaterial.h"

class MatBufferClass;
class TexBufferClass;
class UVBufferClass;
class TextureClass;
class MeshModelClass;

/**
** MeshMatDescClass - This class encapsulates all of the material description data for a mesh.
** WARNING: The vertex count and polygon count *MUST* be kept in sync with the mesh
*/
class MeshMatDescClass : public W3DMPO
{
	W3DMPO_GLUE(MeshMatDescClass)
public:

	enum 
	{
		MAX_PASSES = 4,
		MAX_TEX_STAGES = 2,
		MAX_COLOR_ARRAYS = 2,
		MAX_UV_ARRAYS = MAX_PASSES * MAX_TEX_STAGES
	};

	MeshMatDescClass(void);
	MeshMatDescClass(const MeshMatDescClass & that);
	~MeshMatDescClass(void);
	void							Reset(int polycount,int vertcount,int passcount);
	MeshMatDescClass &		operator = (const MeshMatDescClass & that);

	/*
	** Initialize the alternate material description by copying the default materials
	** and overriding the entries that exist in the alternate_desc
	*/
	void							Init_Alternate(MeshMatDescClass & def_mat_desc,MeshMatDescClass & alternate_desc);
	bool							Is_Empty(void);

	/*
	** Counts, make sure the vertex and polygon counts match the parent mesh.
	*/
	void							Set_Pass_Count(int passes)			{ PassCount = passes; }
	int							Get_Pass_Count(void) const			{ return PassCount; }
	void							Set_Vertex_Count(int vertcount)	{ VertexCount = vertcount; }
	int							Get_Vertex_Count(void) const		{ return VertexCount; }
	void							Set_Polygon_Count(int polycount)	{ PolyCount = polycount; }
	int							Get_Polygon_Count(void) const		{ return PolyCount; }

	/*
	** Material Interface
	*/
	Vector2 *					Get_UV_Array(int pass,int stage);
	void							Install_UV_Array(int pass,int stage,Vector2 * uvs,int count);
	void							Set_UV_Source(int pass,int stage,int sourceindex);
	int							Get_UV_Source(int pass,int stage);

	int							Get_UV_Array_Count(void);
	Vector2 *					Get_UV_Array_By_Index(int index, bool create = true);
	
	unsigned*					Get_DCG_Array(int pass);
	unsigned*					Get_DIG_Array(int pass);
	void							Set_DCG_Source(int pass,VertexMaterialClass::ColorSourceType source);
	void							Set_DIG_Source(int pass,VertexMaterialClass::ColorSourceType source);
	VertexMaterialClass::ColorSourceType Get_DCG_Source(int pass);
	VertexMaterialClass::ColorSourceType Get_DIG_Source(int pass);
	unsigned *					Get_Color_Array(int array,bool create = true);

	void							Set_Single_Material(VertexMaterialClass * vmat,int pass=0);
	void							Set_Single_Texture(TextureClass * tex,int pass=0,int stage=0);
	void							Set_Single_Shader(ShaderClass shader,int pass=0);

	/*
	** the "Get" functions add a reference before returning the pointer (if appropriate)
	*/
	VertexMaterialClass *	Get_Single_Material(int pass=0) const;
	TextureClass *				Get_Single_Texture(int pass=0,int stage=0) const;	
	ShaderClass					Get_Single_Shader(int pass=0) const;

	/*
	** the "Peek" functions just return the pointer and it's the caller's responsibility to 
	** maintain a reference to an object with a reference to the data
	*/
	VertexMaterialClass *	Peek_Single_Material(int pass=0) const;
	TextureClass *				Peek_Single_Texture(int pass=0,int stage=0) const;	

	void							Set_Material(int vidx,VertexMaterialClass * vmat,int pass=0);
	void							Set_Shader(int pidx,ShaderClass shader,int pass=0);
	void							Set_Texture(int pidx,TextureClass * tex,int pass=0,int stage=0);

	/*
	** Queries for determining whether this model has per-polygon arrays of Materials, Shaders, or Textures
	*/
	bool							Has_Material_Array(int pass) const;
	bool							Has_Shader_Array(int pass) const;
	bool							Has_Texture_Array(int pass,int stage) const;

	/*
	** Determine whether this material description contains data for the specified category
	*/
	bool							Has_UV(int pass,int stage)					{ return UVSource[pass][stage] != -1; }
	bool							Has_Color_Array(int array)					{ return ColorArray[array] != NULL; }
	
	bool							Has_Texture_Data(int pass,int stage)	{ return (Texture[pass][stage] != NULL) || (TextureArray[pass][stage] != NULL); }
	bool							Has_Shader_Data(int pass)					{ return (Shader[pass] != NullShader) || (ShaderArray[pass] != NULL); }					
	bool							Has_Material_Data(int pass)				{ return (Material[pass] != NULL) || (MaterialArray[pass] != NULL); }				

	/*
	** "Get" functions for Materials, Textures, and Shaders when there are more than one (per-polygon or per-vertex)
	*/
	VertexMaterialClass *	Get_Material(int vidx,int pass=0) const;
	TextureClass *				Get_Texture(int pidx,int pass=0,int stage=0) const;
	ShaderClass					Get_Shader(int pidx,int pass=0) const;

	/*
	** "Peek" functions for Materials and Textures when there are more than one (per-polygon or per-vertex)
	*/
	VertexMaterialClass *	Peek_Material(int vidx,int pass=0) const;
	TextureClass *				Peek_Texture(int pidx,int pass=0,int stage=0) const;

	/*
	** Access to the arrays
	*/
	TexBufferClass *			Get_Texture_Array(int pass,int stage,bool create = true);
	MatBufferClass *			Get_Material_Array(int pass,bool create = true);
	ShaderClass *				Get_Shader_Array(int pass,bool create = true);

	void							Make_UV_Array_Unique(int pass,int stage);
	void							Make_Color_Array_Unique(int index);

	/*
	** Post-Load processing, configures all materials to use the correct passes and 
	** material color sources, etc.
	*/
	void							Post_Load_Process(bool enable_lighting = true,MeshModelClass * parent = NULL);
	void							Disable_Lighting(void);

	/*
	** Do any of the vertex materials require vertex normals?
	*/
	bool							Do_Mappers_Need_Normals(void);

	static ShaderClass NullShader;	// Used to mark no shader data

protected:
	
	void							Configure_Material(VertexMaterialClass * mtl,int pass,bool lighting_enabled);
	void							Disable_Backface_Culling(void);
	void							Delete_Pass(int pass);

	int													PassCount;
	int													VertexCount;
	int													PolyCount;	

	// u-v coordinates
	UVBufferClass *									UV[MAX_UV_ARRAYS];
	int													UVSource[MAX_PASSES][MAX_TEX_STAGES];

	// vertex color arrays, we support two arrays: each can only be used on the 
	// first pass.
	ShareBufferClass<unsigned> *					ColorArray[2];	
	VertexMaterialClass::ColorSourceType		DCGSource[MAX_PASSES];
	VertexMaterialClass::ColorSourceType		DIGSource[MAX_PASSES];

	// default textures, shader, vmat
	TextureClass *										Texture[MAX_PASSES][MAX_TEX_STAGES];
	ShaderClass											Shader[MAX_PASSES];
	VertexMaterialClass *							Material[MAX_PASSES];

	// array textures, shaders, vmats
	TexBufferClass *									TextureArray[MAX_PASSES][MAX_TEX_STAGES];
	MatBufferClass *									MaterialArray[MAX_PASSES];
	ShareBufferClass<ShaderClass> *				ShaderArray[MAX_PASSES];

	friend class MeshModelClass;
};


/**
** MatBufferClass
** This is a ShareBufferClass of pointers to vertex materials.  Should be written as a template...
** Get and Peek work like normal, and all non-NULL pointers will be released when the buffer 
** is destroyed.
*/
class MatBufferClass : public ShareBufferClass < VertexMaterialClass * >
{
	W3DMPO_GLUE(MatBufferClass)
public:
	MatBufferClass(int count, const char* msg) : ShareBufferClass<VertexMaterialClass *>(count, msg) { Clear(); }
	MatBufferClass(const MatBufferClass & that);
	~MatBufferClass(void);

	void							Set_Element(int index,VertexMaterialClass * mat);
	VertexMaterialClass *	Get_Element(int index);
	VertexMaterialClass *	Peek_Element(int index);

private:
	// not implemented
	MatBufferClass & operator = (const MatBufferClass & that);
};

/**
** TexBufferClass
** This is a ShareBufferClass of pointers to textures.  Works just like MatBufferClass but with 
** TextureClass's...
*/
class TexBufferClass : public ShareBufferClass < TextureClass * >
{
	W3DMPO_GLUE(TexBufferClass)
public:
	TexBufferClass(int count, const char* msg) : ShareBufferClass<TextureClass *>(count, msg) { Clear(); }
	TexBufferClass(const TexBufferClass & that);
	~TexBufferClass(void);						

	void				Set_Element(int index,TextureClass * mat);
	TextureClass *	Get_Element(int index);
	TextureClass *	Peek_Element(int index);

private:
	// not implemented
	TexBufferClass & operator = (const TexBufferClass & that);
};

/**
** UVBufferClass
** This is a ShareBufferClass of uv coordinates.  At load time, we are detecting redundant UV arrays
** so this class stores a CRC for quick checking.
*/
class UVBufferClass : public ShareBufferClass < Vector2 >
{
	W3DMPO_GLUE(UVBufferClass)
public:
	UVBufferClass(int count, const char* msg) : ShareBufferClass<Vector2>(count, msg), CRC(0xFFFFFFFF) { }
	UVBufferClass(const UVBufferClass & that);

	bool				operator == (const UVBufferClass & that);
	bool				Is_Equal_To(const UVBufferClass & that);

	void				Update_CRC(void);
	unsigned int	Get_CRC(void) { return CRC; }

private:
	unsigned int	CRC;

	// not implemented
	UVBufferClass & operator = (const UVBufferClass & that);
};

/*********************************************************************************************************
**
** MeshMatDescClass Inline Functions
** These functions manage the data associated with the material description for the mesh.  Since there
** can be an alternate material description, these functions are encapsulated into MatDescClass.
**
*********************************************************************************************************/

inline Vector2 * MeshMatDescClass::Get_UV_Array(int pass,int stage)
{
	if (UVSource[pass][stage] == -1) {
		return NULL;
	}
	if (UV[UVSource[pass][stage]] != NULL) {
		return UV[UVSource[pass][stage]]->Get_Array();
	}
	return NULL;
}

inline void MeshMatDescClass::Set_UV_Source(int pass,int stage,int sourceindex)
{
	WWASSERT(pass >= 0);
	WWASSERT(pass < MAX_PASSES);
	WWASSERT(stage >= 0);
	WWASSERT(stage < MAX_TEX_STAGES);
	UVSource[pass][stage] = sourceindex;	
}

inline int MeshMatDescClass::Get_UV_Source(int pass,int stage)
{
	WWASSERT(pass >= 0);
	WWASSERT(pass < MAX_PASSES);
	WWASSERT(stage >= 0);
	WWASSERT(stage < MAX_TEX_STAGES);
	return UVSource[pass][stage];
}

inline int MeshMatDescClass::Get_UV_Array_Count(void)
{
	int count = 0;
	while ((UV[count] != NULL) && (count < MAX_UV_ARRAYS)) {
		count++;
	}
	return count;
}

inline Vector2 * MeshMatDescClass::Get_UV_Array_By_Index(int index, bool create)
{
	WWASSERT((index >= 0)&&(index < MAX_UV_ARRAYS));

	if (create && !UV[index]) {
		UV[index] = NEW_REF(UVBufferClass,(VertexCount, "MeshMatDescClass::UV"));
	}
	if (UV[index] != NULL) {
		return UV[index]->Get_Array();
	}
	return NULL;
}

inline unsigned* MeshMatDescClass::Get_DCG_Array(int pass)
{
	WWASSERT(pass >= 0);
	WWASSERT(pass < MAX_PASSES);
	switch (DCGSource[pass]) {
		case VertexMaterialClass::MATERIAL:
			return NULL;
			break;
		case VertexMaterialClass::COLOR1:
			if (ColorArray[0]) {
				return ColorArray[0]->Get_Array();
			} else {
				return NULL;
			}
			break;
		case VertexMaterialClass::COLOR2:
			if (ColorArray[1]) {
				return ColorArray[1]->Get_Array();
			} else {
				return NULL;
			}
			break;
		default:
			WWASSERT(0);
			return(NULL);
			break;
	};
}

inline unsigned * MeshMatDescClass::Get_DIG_Array(int pass)
{
	WWASSERT(pass >= 0);
	WWASSERT(pass < MAX_PASSES);
	switch (DIGSource[pass]) {
		case VertexMaterialClass::MATERIAL:
			return NULL;
			break;
		case VertexMaterialClass::COLOR1:
			if (ColorArray[0]) {
				return ColorArray[0]->Get_Array();
			} else {
				return NULL;
			}
			break;
		case VertexMaterialClass::COLOR2:
			if (ColorArray[1]) {
				return ColorArray[1]->Get_Array();
			} else {
				return NULL;
			}
			break;
		default:
			WWASSERT(0);
			return(NULL);
			break;
	};
}

inline void MeshMatDescClass::Set_DCG_Source(int pass,VertexMaterialClass::ColorSourceType source)
{
	DCGSource[pass] = source;
}

inline void MeshMatDescClass::Set_DIG_Source(int pass,VertexMaterialClass::ColorSourceType source)
{
	DIGSource[pass] = source;
}

inline VertexMaterialClass::ColorSourceType MeshMatDescClass::Get_DCG_Source(int pass)
{
	return DCGSource[pass];
}

inline VertexMaterialClass::ColorSourceType MeshMatDescClass::Get_DIG_Source(int pass)
{
	return DIGSource[pass];
}

inline unsigned * MeshMatDescClass::Get_Color_Array(int index,bool create)
{
	if (create && !ColorArray[index]) {
		ColorArray[index] = NEW_REF(ShareBufferClass<unsigned>,(VertexCount, "MeshMatDescClass::ColorArray"));
	}
	if (ColorArray[index]) {
		return ColorArray[index]->Get_Array();
	}
	return NULL;
}

inline VertexMaterialClass * MeshMatDescClass::Get_Single_Material(int pass) const
{
	if (Material[pass]) {
		Material[pass]->Add_Ref();
	}
	return Material[pass];
}

inline VertexMaterialClass * MeshMatDescClass::Peek_Single_Material(int pass) const
{
	return Material[pass];
}

inline TextureClass * MeshMatDescClass::Peek_Single_Texture(int pass,int stage) const
{
	return Texture[pass][stage];
}

inline ShaderClass MeshMatDescClass::Get_Single_Shader(int pass) const
{
	return Shader[pass];
}

inline bool MeshMatDescClass::Has_Material_Array(int pass) const
{
	return (MaterialArray[pass] != NULL);
}

inline bool MeshMatDescClass::Has_Shader_Array(int pass) const
{
	return (ShaderArray[pass] != NULL);
}

inline bool MeshMatDescClass::Has_Texture_Array(int pass,int stage) const
{
	return (TextureArray[pass][stage] != NULL);
}

inline void MeshMatDescClass::Disable_Backface_Culling(void) 
{
	for (int pass = 0; pass < PassCount; pass++) {
		Shader[pass].Set_Cull_Mode(ShaderClass::CULL_MODE_DISABLE);
		if (ShaderArray[pass]) {
			for (int tri = 0; tri < ShaderArray[pass]->Get_Count(); tri++) {
				ShaderArray[pass]->Get_Element(tri).Set_Cull_Mode(ShaderClass::CULL_MODE_DISABLE);
			}
		}
	}
}

#endif //MESHMATDESC_H

