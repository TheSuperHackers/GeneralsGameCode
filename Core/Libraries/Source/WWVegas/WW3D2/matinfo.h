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
 *                     $Archive:: /VSS_Sync/ww3d2/matinfo.h                                   $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 8/29/01 7:29p                                               $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "always.h"
#include "wwdebug.h"
#include "Vector.H"
#include "vertmaterial.h"
#include "texture.h"
#include "shader.h"
#ifdef _UNIX
#include "osdep.h"
#endif

class MeshModelClass;
class MeshMatDescClass;

/***********************************************************************************************
** MaterialInfoClass
**
** This class gives an interface to the "changeable" material parameters inside a
** W3D render object.  Typically, this will include things like the following:
** - one or more textures,
** - the vertex material used by the mesh (defines lighting properties, etc)
**
** Another purpose of this class is to actually hold onto a ref for each material
** object that a mesh uses.  The "per-triangle" pointer arrays are not ref counted
** so the material info serves as a place to hold a reference to each material object
** that the mesh is using.
**
***********************************************************************************************/
class MaterialInfoClass : public W3DMPO, public RefCountClass
{
	W3DMPO_GLUE(MaterialInfoClass)
public:

	MaterialInfoClass();
	MaterialInfoClass(const MaterialInfoClass & src);
	~MaterialInfoClass();
	MaterialInfoClass * Clone(void) const;

	void							Reset(void)									{ Free(); }
	int							Vertex_Material_Count(void) const	{ return VertexMaterials.Count(); }
	int							Texture_Count(void) const				{ return Textures.Count(); }

	int							Add_Vertex_Material(VertexMaterialClass * vmat);
	int							Add_Texture(TextureClass * tex);

	int							Get_Vertex_Material_Index(const char * name);
	int							Get_Texture_Index(const char * name);

	VertexMaterialClass *	Get_Vertex_Material(int index);
	VertexMaterialClass *	Get_Vertex_Material(const char * name);
	VertexMaterialClass *	Peek_Vertex_Material(int index);
	VertexMaterialClass *	Peek_Vertex_Material(const char * name);
	void							Replace_Material(int index, VertexMaterialClass *newMaterial);
	void							Reset_Texture_Mappers(void);
	void							Make_Vertex_Materials_Unique(void);
	bool							Has_Time_Variant_Texture_Mappers(void);

	TextureClass *				Get_Texture(int index);
	TextureClass *				Get_Texture(const char * name);
	TextureClass *				Peek_Texture(int index);
	TextureClass *				Peek_Texture(const char * name);
	void							Replace_Texture(int index, TextureClass *newTexture);

//	void							Set_Texture_Reduction_Factor(float trf);
//	void							Process_Texture_Reduction(void);

private:

	void Free(void);

	DynamicVectorClass<VertexMaterialClass *>		VertexMaterials;
	DynamicVectorClass<TextureClass *>			Textures;

};



/***********************************************************************************************
** MaterialRemapperClass
** This class is used when we need to "remap" all of the material pointers in a mesh
** to a new set of cloned materials.  Basically, it assumes that you are going to initialize
** it with two identical material info objects (src and dest) and then you can give it pointers
** to materials in the "src" material info and it will give you back pointers to materials
** in the "dest" material info class.
**
** This class attempts to take advantage of the fact that meshes should normally be sorted
** with respect to their materials so we shouldn't really be doing a linear search for
** each remap...
**
** Please Note: this class does not hold references to the materials and is meant only to
** be used in a temporary fashion.  Create it, do your conversion, then delete it :-)
***********************************************************************************************/
class MaterialRemapperClass
{
public:
	MaterialRemapperClass(MaterialInfoClass * src,MaterialInfoClass * dest);
	~MaterialRemapperClass(void);

	TextureClass *					Remap_Texture(TextureClass * src);
	VertexMaterialClass *		Remap_Vertex_Material(VertexMaterialClass * src);
	void								Remap_Mesh(const MeshMatDescClass * srcmeshmatdesc, MeshMatDescClass * destmeshmatdesc);

private:

	struct VmatRemapStruct
	{
		VertexMaterialClass *	Src;
		VertexMaterialClass *	Dest;
	};

	struct TextureRemapStruct
	{
		TextureClass *				Src;
		TextureClass *				Dest;
	};

	MaterialInfoClass *			SrcMatInfo;
	MaterialInfoClass *			DestMatInfo;

	int								TextureCount;
	TextureRemapStruct *			TextureRemaps;
	int								VertexMaterialCount;
	VmatRemapStruct *				VertexMaterialRemaps;

	VertexMaterialClass *		LastSrcVmat;
	VertexMaterialClass *		LastDestVmat;
	TextureClass *					LastSrcTex;
	TextureClass *					LastDestTex;
};

/***********************************************************************************************
** MaterialCollectorClass
**
** This class can be used to collect all of the unique instances of materials from a mesh.
** Its original motivation is to solve a problem encountered in trying to save a mesh
** to disk.  There are arrays of pointers to vertex materials in the mesh but no record of
** the set of unique vertex materials (all pointers could point to the same one...)  Similar
** to the remapper, it tries to take advantage of the fact that the materials and textures
** should be in sorted order to optimize the lookups...
**
** NOTE: pointer comparisons are used to determine if the objects are unique.  I don't
** check whether the contents of the objects are identical.  (Exporter does this, I assume
** that if there are two separate objects, they are that way for a reason here.)
***********************************************************************************************/
class MaterialCollectorClass
{
public:

	MaterialCollectorClass(void);
	~MaterialCollectorClass(void);

	void							Reset(void);
	void							Collect_Materials(MeshModelClass * mesh);
	void							Add_Texture(TextureClass * tex);
	void							Add_Shader(ShaderClass shader);
	void							Add_Vertex_Material(VertexMaterialClass * vmat);

	int							Get_Shader_Count(void);
	int							Get_Vertex_Material_Count(void);
	int							Get_Texture_Count(void);

	ShaderClass					Peek_Shader(int i);
	TextureClass *				Peek_Texture(int i);
	VertexMaterialClass *	Peek_Vertex_Material(int i);

	int							Find_Shader(const ShaderClass & shader);
	int							Find_Texture(TextureClass * tex);
	int							Find_Vertex_Material(VertexMaterialClass * mat);

protected:

	DynamicVectorClass<ShaderClass> 					Shaders;
	DynamicVectorClass<VertexMaterialClass *> 	VertexMaterials;
	DynamicVectorClass<TextureClass *> 				Textures;

	ShaderClass												LastShader;
	VertexMaterialClass *								LastMaterial;
	TextureClass *											LastTexture;
};



inline int MaterialInfoClass::Add_Vertex_Material(VertexMaterialClass * vmat)
{
	if (vmat != NULL) {
		vmat->Add_Ref();
	}
	int index = VertexMaterials.Count();
	VertexMaterials.Add(vmat);
	return index;
}

inline int MaterialInfoClass::Get_Vertex_Material_Index(const char * name)
{
	for (int i=0; i<VertexMaterials.Count(); i++) {
		if (stricmp(name,VertexMaterials[i]->Get_Name()) == 0) {
			return i;
		}
	}
	return -1;
}

inline VertexMaterialClass * MaterialInfoClass::Get_Vertex_Material(int index)
{
	WWASSERT(index >= 0);
	WWASSERT(index < VertexMaterials.Count());
	if (VertexMaterials[index]) {
		VertexMaterials[index]->Add_Ref();
	}
	return VertexMaterials[index];
}

inline VertexMaterialClass * MaterialInfoClass::Get_Vertex_Material(const char * name)
{
	int index = Get_Vertex_Material_Index(name);
	if (index == -1) {
		return NULL;
	} else {
		return Get_Vertex_Material(index);
	}
}

inline VertexMaterialClass * MaterialInfoClass::Peek_Vertex_Material(int index)
{
	WWASSERT(index >= 0);
	WWASSERT(index < VertexMaterials.Count());
	return VertexMaterials[index];
}

inline VertexMaterialClass * MaterialInfoClass::Peek_Vertex_Material(const char * name)
{
	int index = Get_Vertex_Material_Index(name);
	if (index == -1) {
		return NULL;
	} else {
		return Peek_Vertex_Material(index);
	}
}

inline void	MaterialInfoClass::Replace_Material(int index, VertexMaterialClass *newMaterial)
{
	REF_PTR_SET(VertexMaterials[index],newMaterial);
}

inline void	MaterialInfoClass::Reset_Texture_Mappers(void)
{
	int vmat_count = VertexMaterials.Count();
	for (int i = 0; i < vmat_count; i++) {
		VertexMaterials[i]->Reset_Mappers();
	}
}

inline bool	MaterialInfoClass::Has_Time_Variant_Texture_Mappers(void)
{
	int vmat_count = VertexMaterials.Count();
	for (int i = 0; i < vmat_count; i++) {
		if (VertexMaterials[i]->Are_Mappers_Time_Variant()) return true;
	}
	return false;
}

inline void MaterialInfoClass::Make_Vertex_Materials_Unique(void)
{
	int vmat_count = VertexMaterials.Count();
	for (int i = 0; i < vmat_count; i++) {
		VertexMaterials[i]->Make_Unique();
	}
}

inline TextureClass * MaterialInfoClass::Get_Texture(const char * name)
{
	int index = Get_Texture_Index(name);
	if (index == -1) {
		return NULL;
	} else {
		return Get_Texture(index);
	}
}

inline TextureClass *	MaterialInfoClass::Peek_Texture(int index)
{
	WWASSERT(index >= 0);
	WWASSERT(index < Textures.Count());
	return Textures[index];
}

inline void	MaterialInfoClass::Replace_Texture(int index, TextureClass *newTexture)
{
	REF_PTR_SET(Textures[index],newTexture);
}
