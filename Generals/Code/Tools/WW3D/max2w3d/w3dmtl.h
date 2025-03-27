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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Max2W3d                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/w3dmtl.h                       $*
 *                                                                                             *
 *                      $Author:: Andre_a                                                     $*
 *                                                                                             *
 *                     $Modtime:: 12/07/00 2:47p                                              $*
 *                                                                                             *
 *                    $Revision:: 15                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef W3DMTL_H
#define W3DMTL_H

#include "always.h"
#include "w3d_file.h"
#include "Vector.H"

class GameMtl;
class Mtl;
class ChunkSaveClass;


/*
** W3dMapClass.  
** This class simply ties together the map info and the map filename
*/
class W3dMapClass
{
public:
	W3dMapClass(void) : Filename(NULL), AnimInfo(NULL) {};
	W3dMapClass(const W3dMapClass & that);
	~W3dMapClass(void);
	
	W3dMapClass & operator = (const W3dMapClass & that);

	void Reset(void);
	void Set_Filename(const char * name);
	void Set_Anim_Info(const W3dTextureInfoStruct * info);
	void Set_Anim_Info(int framecount,float framerate);

	char *						Filename;
	W3dTextureInfoStruct *	AnimInfo;
};


/*
** W3dMaterialClass. 
** This class ties together w3d structures for up to 'MAX_PASSES' material passes.
** It is typically plugged into the next class (W3dMaterialDescClass) so that
** duplicate members can detected and shared.
*/
class W3dMaterialClass
{
public:

	W3dMaterialClass(void);
	~W3dMaterialClass(void);

	enum { MAX_PASSES = 4, MAX_STAGES = 2 };

	void								Reset(void);

	/*
	** Construction from Max materials
	*/
	void								Init(Mtl * mtl, char *materialColorTexture=NULL);
	void								Init(GameMtl * gamemtl, char *materialColorTexture=NULL);

	/*
	** Manual Construction
	*/
	void								Set_Surface_Type(unsigned int type);
	void								Set_Sort_Level(int level);
	void								Set_Pass_Count(int count);
	void								Set_Vertex_Material(const W3dVertexMaterialStruct & vmat,int pass = 0);
	void								Set_Mapper_Args(const char *args_buffer, int pass = 0, int stage = 0);
	void								Set_Shader(const W3dShaderStruct & shader,int pass = 0);
	void								Set_Texture(const W3dMapClass & map,int pass = 0,int stage = 0);
	void								Set_Map_Channel(int pass,int stage,int channel);

	/*
	** Inspection
	*/
	unsigned int					Get_Surface_Type(void) const;
	int								Get_Sort_Level(void) const;
	int								Get_Pass_Count(void) const;
	W3dVertexMaterialStruct *	Get_Vertex_Material(int pass = 0) const;
	const char *					Get_Mapper_Args(int pass /*= 0*/, int stage /*= 0*/) const;
	W3dShaderStruct				Get_Shader(int pass = 0) const;
	W3dMapClass *					Get_Texture(int pass = 0,int stage = 0) const;
	int								Get_Map_Channel(int pass = 0,int stage = 0) const;

	bool								Is_Multi_Pass_Transparent(void) const;

protected:
	
	void								Free(void);	
	
	unsigned int					SurfaceType;
	int								SortLevel;
	int								PassCount;	
	
	W3dShaderStruct				Shaders[MAX_PASSES];
	W3dVertexMaterialStruct *	Materials[MAX_PASSES];
	char *							MapperArgs[MAX_PASSES][MAX_STAGES];
	W3dMapClass *					Textures[MAX_PASSES][MAX_STAGES];
	int								MapChannel[MAX_PASSES][MAX_STAGES];

};


/*
** W3dMaterialDescClass
** This class's purpose is to process the set of w3dmaterials used by a mesh into a set
** of surrender passes with shaders, vertexmaterials, textures.  Part of its job is
** to detect duplicated shaders and vertex materials and remove them.
*/
class W3dMaterialDescClass
{
public:

	typedef enum ErrorType
	{
		OK = 0,							// material description was built successfully
		INCONSISTENT_PASSES,			// material doesn't have same number of passes
		MULTIPASS_TRANSPARENT,		// material is transparent and multi-pass (NO-NO!)
		INCONSISTENT_SORT_LEVEL,	// material doesn't have the same sort level!
	};

	W3dMaterialDescClass(void);
	~W3dMaterialDescClass(void);
	
	void								Reset(void);

	/*
	** Interface for adding a material description.  The material will be assigned 
	** an index based on the order at which they are added.  Add your materials in 
	** order, then use their indices to find the remapped vertex materials, textures, 
	** and shaders...
	*/
	ErrorType						Add_Material(const W3dMaterialClass & mat,const char * name = NULL);
	
	/*
	** Global Information.  These methods give access to all of the unique vertex materials,
	** shaders, and textures being used.
	*/
	int								Material_Count(void);
	int								Pass_Count(void);
	int								Vertex_Material_Count(void);
	int								Shader_Count(void);
	int								Texture_Count(void);
	int								Get_Sort_Level(void);

	W3dVertexMaterialStruct *	Get_Vertex_Material(int vmat_index);
	const char *					Get_Mapper_Args(int vmat_index, int stage);
	W3dShaderStruct *				Get_Shader(int shader_index);
	W3dMapClass *					Get_Texture(int texture_index);

	/*
	** Per-Pass Information.  These methods convert a material index and pass index pair into
	** an index to the desired vertex material, texture or shader.
	*/
	int								Get_Vertex_Material_Index(int mat_index,int pass);
	int								Get_Shader_Index(int mat_index,int pass);
	int								Get_Texture_Index(int mat_index,int pass,int stage);
	W3dVertexMaterialStruct *	Get_Vertex_Material(int mat_index,int pass);
	const char *					Get_Mapper_Args(int mat_index,int pass,int stage);
	W3dShaderStruct *				Get_Shader(int mat_index,int pass);
	W3dMapClass *					Get_Texture(int mat_index,int pass,int stage);
	int								Get_Map_Channel(int mat_index,int pass,int stage);
	bool								Stage_Needs_Texture_Coordinates(int pass,int stage);
	bool								Pass_Uses_Vertex_Alpha(int pass);
	bool								Pass_Uses_Alpha(int pass);

	/*
	** Vertex Material Names.  It will be useful to have named vertex materials.  I'll keep
	** the name of the first material which contained each vertex material as its name.  Use
	** these functions to get the name associated with a vertex material
	*/
	const char *					Get_Vertex_Material_Name(int mat_index,int pass);
	const char *					Get_Vertex_Material_Name(int vmat_index);

private:

	int								Add_Vertex_Material(W3dVertexMaterialStruct * vmat,const char *mapper_args0,const char *mapper_args1,int pass,const char * name);
	int								Add_Shader(const W3dShaderStruct & shader,int pass);
	int								Add_Texture(W3dMapClass * map,int pass,int stage);
	unsigned long					Compute_Crc(const W3dVertexMaterialStruct & vmat,const char *mapper_args0,const char *mapper_args1);
	unsigned long					Compute_Crc(const W3dShaderStruct & shader);
	unsigned long					Compute_Crc(const W3dMapClass & map);
	unsigned long					Add_String_To_Crc(const char *str, unsigned long crc);

	/*
	** MaterialRemapClass
	** When the user supplies a W3dMaterial to this material description class,
	** its sub-parts are installed and an instance of this class is created to
	** re-index to each one.
	*/
	class MaterialRemapClass
	{
	public:
		MaterialRemapClass(void);

		bool operator != (const MaterialRemapClass & that);
		bool operator == (const MaterialRemapClass & that);

		int		PassCount;
		int		VertexMaterialIdx[W3dMaterialClass::MAX_PASSES];
		int		ShaderIdx[W3dMaterialClass::MAX_PASSES];
		int		TextureIdx[W3dMaterialClass::MAX_PASSES][W3dMaterialClass::MAX_STAGES];
		int		MapChannel[W3dMaterialClass::MAX_PASSES][W3dMaterialClass::MAX_STAGES];
	};

	/*
	** VertMatClass
	** This class encapsulates a vertex material structure and makes it extendable for 
	** any purposes needed by the plugin code.  For example, the pass index is stored
	** so that we can prevent "welding" of vertex materials in different passes (since
	** this may not be desireable...)
	*/
	class VertMatClass
	{
	public:
		VertMatClass(void);
		~VertMatClass(void);

		VertMatClass & operator = (const VertMatClass & that);
		bool operator != (const VertMatClass & that);
		bool operator == (const VertMatClass & that);
		void Set_Name(const char * name);
		void Set_Mapper_Args(const char * args, int stage);

		W3dVertexMaterialStruct		Material;
		char *							MapperArgs[W3dMaterialClass::MAX_STAGES];		// note: these strings are new'ed, not malloc'ed (unlike Name)
		int								PassIndex;				// using this to prevent joining of vertmats in different passes.
		int								Crc;						// crc, used for quick rejection when checking for matches.
		char *							Name;						// material name associated with the first occurence of this vmat.
	};

	/*
	** ShadeClass
	** Again, simply here to make the shader extendable for any purposes needed by this
	** pre-processing code...
	*/
	class ShadeClass
	{
	public:
		ShadeClass & operator = (const ShadeClass & that) { Shader = that.Shader; Crc = that.Crc; return *this;}
		bool operator != (const ShadeClass & that) { return !(*this == that); }
		bool operator == (const ShadeClass & that) { assert(0); return false; }

		W3dShaderStruct				Shader;
		int								Crc;
	};
	
	/*
	** TexClass
	** Simply here to allow extra info to be stored with each texture, as needed by this
	** pre-processing code...
	*/
	class TexClass
	{
	public:
		TexClass & operator = (const TexClass & that) { Map = that.Map; Crc = that.Crc; return *this; }
		bool operator != (const TexClass & that) { return !(*this == that); }
		bool operator == (const TexClass & that) { assert(0); return false; }

		W3dMapClass						Map;
		int								Crc;
	};
	
	int																PassCount;
	int																SortLevel;
	DynamicVectorClass < MaterialRemapClass >				MaterialRemaps;
	DynamicVectorClass < ShadeClass >						Shaders;
	DynamicVectorClass < VertMatClass >						VertexMaterials;
	DynamicVectorClass < TexClass >							Textures;

};



#endif