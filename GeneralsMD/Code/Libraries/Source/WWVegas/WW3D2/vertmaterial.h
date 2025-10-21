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
 *                     $Archive:: /Commando/Code/ww3d2/vertmaterial.h                         $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 4/27/01 2:22p                                               $*
 *                                                                                             *
 *                    $Revision:: 25                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#pragma once

#ifndef VERTMATERIAL_H
#define VERTMATERIAL_H

#include "always.h"

#include "refcount.h"
#include "vector3.h"
#include "w3d_file.h"
#include "meshbuild.h"
#include "w3derr.h"
#include "mapper.h"
#include "wwstring.h"

#include <string.h>

class ChunkLoadClass;
class ChunkSaveClass;

#define DYN_MAT8
#ifdef DYN_MAT8
class DynD3DMATERIAL8;
#else
struct _D3DMATERIAL8;
#endif

/**
** VertexMaterialClass
** This is simply the typical W3D thin-wrapper around the surrender vertex material.
** The vertex material defines things like the lighting properties of a vertex.
*/
class VertexMaterialClass : public W3DMPO, public RefCountClass
{
	W3DMPO_GLUE(VertexMaterialClass)

	friend DX8Wrapper;

public:
	/*
	** Similar to the TextureClass, these enumerations are set up to be exactly the same as
	** the surrender material enumerations.  Nearly all functions in this class are inline
	** so that it will go away since the only purpose of Material Class is to protect our
	** application code from the ever-changing surrender...
	*/
	enum MappingType {
		MAPPING_NONE = -1,											// no mapping needed
		MAPPING_UV = W3DMAPPING_UV,								// default, use the u-v values in the model
		MAPPING_ENVIRONMENT = W3DMAPPING_ENVIRONMENT,		// use the environment mapper
	};

	enum FlagsType {
		DEPTH_CUE= 0,					// enable depth cueing (default = false)
		DEPTH_CUE_TO_ALPHA,
		COPY_SPECULAR_TO_DIFFUSE,
	};

	enum ColorSourceType {
		MATERIAL = 0,				// D3DMCS_MATERIAL - the color source should be taken from the material setting
		COLOR1,						// D3DMCS_COLOR1 - the color should be taken from per-vertex color array 1 (aka D3DFVF_DIFFUSE)
		COLOR2,						// D3DMCS_COLOR2 - the color should be taken from per-vertex color array 2 (aka D3DFVF_SPECULAR)
	};

	enum PresetType
	{
		PRELIT_DIFFUSE=0,
		PRELIT_NODIFFUSE,
		PRESET_COUNT
	};


	VertexMaterialClass(void);
	VertexMaterialClass(const VertexMaterialClass & src);
	~VertexMaterialClass(void);

	VertexMaterialClass &	operator = (const VertexMaterialClass &src);
	VertexMaterialClass *	Clone(void) { VertexMaterialClass * mat = NEW_REF (VertexMaterialClass,()); *mat = *this; return mat;}

	/*
	** Name Access
	*/
	void					Set_Name(const char * name)
	{
		Name = name;
	}

	const char *		Get_Name(void) const
	{
		return Name;
	}

	/*
	** Control over the material flags
	*/
	void					Set_Flag(FlagsType flag,bool onoff)
	{
		CRCDirty=true;
		if (onoff)
			Flags|=(1<<flag);
		else
			Flags&=~(1<<flag);
	}
	int					Get_Flag(FlagsType flag)
	{ return (Flags>>flag) & 0x1; }

	/*
	** Basic material properties
	*/
	float			Get_Shininess(void) const;
	void			Set_Shininess(float shin);

	float			Get_Opacity(void) const;
	void			Set_Opacity(float o);

	void			Get_Ambient(Vector3 * set_color) const;
	void			Set_Ambient(const Vector3 & color);
	void			Set_Ambient(float r,float g,float b);

	void			Get_Diffuse(Vector3 * set_color) const;
	void			Set_Diffuse(const Vector3 & color);
	void			Set_Diffuse(float r,float g,float b);

	void			Get_Specular(Vector3 * set_color) const;
	void			Set_Specular(const Vector3 & color);
	void			Set_Specular(float r,float g,float b);

	void			Get_Emissive(Vector3 * set_color) const;
	void			Set_Emissive(const Vector3 & color);
	void			Set_Emissive(float r,float g,float b);

	void			Set_Lighting(bool lighting) { CRCDirty=true; UseLighting=lighting; };
	bool			Get_Lighting() const { return UseLighting; };

	/*
	** Color source control.  Note that if you set one of the sources to be one of
	** the arrays, then the setting in the material is ignored.  (i.e. if you
	** set the diffuse source to array0, then the diffuse color set into the
	** vertex material is ignored.
	*/
	void					Set_Ambient_Color_Source(ColorSourceType src);
	ColorSourceType	Get_Ambient_Color_Source(void);

	void					Set_Emissive_Color_Source(ColorSourceType src);
	ColorSourceType	Get_Emissive_Color_Source(void);

	void					Set_Diffuse_Color_Source(ColorSourceType src);
	ColorSourceType	Get_Diffuse_Color_Source(void);

	/*
	** UV source control.  The DX8 FVF can support up to 8 uv-arrays.  The vertex
	** material can/must be configured to index to the uv-arrays that you want to
	** use for the two texture stages.
	*/
	void					Set_UV_Source(int stage,int array_index);
	int					Get_UV_Source(int stage);

	/*
	** Mapper control.
	*/
	inline void							Set_Mapper(TextureMapperClass *mapper,int stage=0);
	inline TextureMapperClass *	Get_Mapper(int stage=0);
	inline TextureMapperClass *	Peek_Mapper(int stage=0);
	inline void							Reset_Mappers(void);

	/*
	** Loading and saving to W3D files
	*/
	WW3DErrorType		Load_W3D(ChunkLoadClass & cload);
	WW3DErrorType		Save_W3D(ChunkSaveClass & csave);

	void					Parse_W3dVertexMaterialStruct(const W3dVertexMaterialStruct & vmat);
	void					Parse_Mapping_Args(const W3dVertexMaterialStruct & vmat,char * mapping0_arg_buffer,char * mapping1_arg_buffer);
	void					Init_From_Material3(const W3dMaterial3Struct & mat3);

	/*
	** CRC, used by the loading code to build a list of the unique materials
	*/
	inline unsigned long Get_CRC(void) const
	{
		if (CRCDirty) {
			CRC=Compute_CRC();
			CRCDirty=false;
		}

		return CRC;
	}

	/*
	** Test whether this material is using any mappers which require vertex normals
	*/
	bool					Do_Mappers_Need_Normals(void) const;

	/*
	** Test whether this material is using any mappers which are time-variant
	*/
	bool					Are_Mappers_Time_Variant(void) const;

	// Infrastructure to support presets
	static void Init();
	static void Shutdown();
	// TODO: Make Get_Preset const
	static VertexMaterialClass *Get_Preset(PresetType type);

	void Make_Unique();

private:
	// We're using the pointer instead of the actual structure
	// so we don't have to include the d3d header - HY
#ifdef DYN_MAT8
	DynD3DMATERIAL8 *			MaterialDyn;
#else
	_D3DMATERIAL8 *				MaterialOld;
#endif
	unsigned int					Flags;
	unsigned int					AmbientColorSource;
	unsigned int					EmissiveColorSource;
	unsigned int					DiffuseColorSource;
	StringClass						Name;
	TextureMapperClass *	Mapper[MeshBuilderClass::MAX_STAGES];
	unsigned int					UVSource[MeshBuilderClass::MAX_STAGES];
	unsigned int					UniqueID;
	mutable unsigned long CRC;
	mutable bool					CRCDirty;
	bool									UseLighting;

private:
	/*
	** Apply the render states to D3D
	*/
	void					Apply(void) const;
	/*
	** Apply the render states corresponding to a NULL vetex material to D3D
	*/
	static void			Apply_Null(void);
	unsigned long		Compute_CRC(void) const;

	static VertexMaterialClass *Presets[PRESET_COUNT];
};

inline void VertexMaterialClass::Set_Mapper(TextureMapperClass *mapper, int stage)
{
	CRCDirty=true;
	REF_PTR_SET(Mapper[stage],mapper);
}

inline TextureMapperClass * VertexMaterialClass::Get_Mapper(int stage)
{
	if (Mapper[stage]) {
		Mapper[stage]->Add_Ref();
	}
	return Mapper[stage];
}

inline TextureMapperClass * VertexMaterialClass::Peek_Mapper(int stage)
{
	return Mapper[stage];
}

inline void VertexMaterialClass::Reset_Mappers(void)
{
	for (int stage = 0; stage < MeshBuilderClass::MAX_STAGES; stage++) {
		if (Mapper[stage]) {
			Mapper[stage]->Reset();
		}
	}
}

inline bool VertexMaterialClass::Do_Mappers_Need_Normals(void) const
{
	for (int stage = 0; stage < MeshBuilderClass::MAX_STAGES; stage++) {
		if (Mapper[stage] && (Mapper[stage]->Needs_Normals())) return true;
	}
	return false;
}

inline bool VertexMaterialClass::Are_Mappers_Time_Variant(void) const
{
	for (int stage = 0; stage < MeshBuilderClass::MAX_STAGES; stage++) {
		if (Mapper[stage] && (Mapper[stage]->Is_Time_Variant())) return true;
	}
	return false;
}

#endif //VERTMATERIAL_H

