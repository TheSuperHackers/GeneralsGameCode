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



#ifndef SHDLEGACYW3D_H
#define SHDLEGACYW3D_H

#include "shdinterface.h"
#include "shddef.h"
#include "meshmatdesc.h"

/**
** ShdLegacyW3DDefClass - This ShaderDef's sole purpose for existance is to let us make the 
** transition to using shaders for all of our materials.  This shader will wrap the old
** material system completely.  The plan is to eventually have a full set of "real" shaders and
** not need to use this at all.
*/
class ShdLegacyW3DDefClass : public ShdDefClass
{
public:

	/////////////////////////////////////////////////////////////////////
	//	Editable interface requirements
	/////////////////////////////////////////////////////////////////////
	DECLARE_EDITABLE(ShdLegacyW3DDefClass, ShdDefClass);

	ShdLegacyW3DDefClass();
	ShdLegacyW3DDefClass(const ShdLegacyW3DDefClass& that);
	virtual ~ShdLegacyW3DDefClass();
	
	virtual ShdDefClass*	Clone() const	{ return new ShdLegacyW3DDefClass(*this); }

	// Shader Creation (should create a shader compatible with the current hardware/API)
	virtual void						Init();
	virtual void						Shutdown();
	virtual ShdInterfaceClass*		Create() const;

	// Validation methods
	virtual bool						Is_Valid_Config(StringClass &message);

	// Requirements
	virtual bool						Requires_Normals() const { return true; }
	virtual bool						Requires_Tangent_Space_Vectors() const { return false; }
	virtual bool						Requires_Sorting() const { return false; }
	virtual int							Static_Sort_Index() const { return 0; }
	virtual bool						Uses_UV_Channel(int i) const;

	// From PersistClass
	virtual bool						Save(ChunkSaveClass &csave);
	virtual bool						Load(ChunkLoadClass &cload);

	// Accessors
	int										Get_Pass_Count() { return PassCount; }
	const StringClass&					Get_Texture_Name(int pass, int stage) { return TextureNames[pass][stage]; }
	int										Get_Texture_Attributes(int pass,int stage) { return TextureAttributes[pass][stage]; }
	const W3dShaderStruct&				Get_Shader(int pass) { return Shaders[pass]; }
	const W3dVertexMaterialStruct &	Get_Material(int pass) { return Materials[pass]; }
	StringClass &							Get_Mapper_Args(int pass,int stage) { return MapperArgs[pass][stage]; }
	int										Get_Map_Channel(int pass,int stage) { return MapChannel[pass][stage]; }

	void										Set_Pass_Count(int passes) { PassCount=passes; }
	void										Set_Texture_Name(int pass, int stage, const StringClass& name) { TextureNames[pass][stage]=name; }
	void										Set_Texture_Attributes(int pass, int stage, unsigned int attributes) { TextureAttributes[pass][stage] = attributes; }
	void										Set_Shader(int pass, const W3dShaderStruct& shader) { Shaders[pass] = shader; }
	void										Set_Material(int pass, const W3dVertexMaterialStruct& mat) { Materials[pass] = mat; }
	void										Set_Mapper_Args(int pass, int stage,const char * args) { MapperArgs[pass][stage] = args; }
	void										Set_Map_Channel(int pass, int stage, int channel) { MapChannel[pass][stage] = channel; }

private:

	bool									Save_Variables(ChunkSaveClass &csave);
	bool									Load_Variables(ChunkLoadClass &cload);

	int									PassCount;
	
	// Textures
	StringClass							TextureNames[MeshMatDescClass::MAX_PASSES][MeshMatDescClass::MAX_TEX_STAGES];
	unsigned int						TextureAttributes[MeshMatDescClass::MAX_PASSES][MeshMatDescClass::MAX_TEX_STAGES];

	// Shaders
	W3dShaderStruct					Shaders[MeshMatDescClass::MAX_PASSES];
	
	// Materials
	W3dVertexMaterialStruct			Materials[MeshMatDescClass::MAX_PASSES];
	StringClass							MapperArgs[MeshMatDescClass::MAX_PASSES][MeshMatDescClass::MAX_TEX_STAGES];
	int									MapChannel[MeshMatDescClass::MAX_PASSES][MeshMatDescClass::MAX_TEX_STAGES];

};



class Shd6LegacyW3DClass : public ShdInterfaceClass
{
public:
	Shd6LegacyW3DClass(const ShdDefClass* def);
	virtual ~Shd6LegacyW3DClass();
	
	static void Init();
	static void Shutdown();

	virtual bool						Greater_Than(const ShdInterfaceClass& s,int pass) const;

	virtual int							Get_Pass_Count() { return PassCount; }
	void									Set_Pass_Count(int pass_count) { PassCount=pass_count; }

	virtual int							Get_Texture_Count() const { return 0; }
	virtual TextureClass*			Peek_Texture(int idx) const { return NULL; }

	virtual void						Apply_Shared(int cur_pass, RenderInfoClass& rinfo);
	virtual void						Apply_Instance(int cur_pass, RenderInfoClass& rinfo);

	virtual unsigned					Get_Vertex_Stream_Count() const;
	virtual unsigned					Get_Vertex_Size(unsigned stream) const;
	virtual bool						Use_HW_Vertex_Processing() const { return true; }

	virtual void						Copy_Vertex_Stream
	(
		unsigned stream, 
		void* dest_buffer, 
		const VertexStreamStruct& vss, 
		unsigned vertex_count
	);

	virtual bool						Is_Opaque(void) const { return (Shaders[0].Get_Dst_Blend_Func() == ShaderClass::DSTBLEND_ZERO); }

	// Shd6LegacyW3DClass Accessors
	VertexMaterialClass*				Get_Material(int pass) { if (Materials[pass]) Materials[pass]->Add_Ref(); return Materials[pass]; }
	void									Set_Material(int pass,VertexMaterialClass* mat) { REF_PTR_SET(Materials[pass],mat); }
	const ShaderClass&				Get_Shader(int pass) { return Shaders[pass]; }
	void									Set_Shader(int pass,const ShaderClass& shader) { Shaders[pass]=shader; }
	void									Set_Texture(int pass,int stage,TextureClass* texture) { REF_PTR_SET(Textures[pass][stage],texture); }
	void									Set_FVF(unsigned fvf) { FVF=fvf; }

protected:

	unsigned								FVF;
	int									PassCount;
	TextureClass*						Textures[MeshMatDescClass::MAX_PASSES][MeshMatDescClass::MAX_TEX_STAGES];
	ShaderClass							Shaders[MeshMatDescClass::MAX_PASSES];
	VertexMaterialClass*				Materials[MeshMatDescClass::MAX_PASSES];

};

#endif //SHDLEGACYW3D_H
