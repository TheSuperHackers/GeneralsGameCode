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
 *                 Project Name : WWSHADE                                                         *
 *                                                                                             *
 *                     $Archive:: wwshade/shdlegacyw3d.cpp                           $*
 *                                                                                             *
 *                  $Org Author:: Jani_p
 *																																	
 *                      $Author:: Jani_p
 *																																	
 *							  $Modtime:: 7/12/02 3:12p                                               $*
 *                                                                                             *
 *                    $Revision:: 1                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include <d3dx8math.h>
#include "dx8fvf.h"
#include "dx8wrapper.h"
#include "assetmgr.h"

#include "shdlegacyw3d.h"

#include "editable.h"
#include "chunkio.h"
#include "w3d_util.h"
#include "persistfactory.h"
#include "wwhack.h"

#include "shdclassids.h"
#include "shddeffactory.h"
#include "shdinterface.h"
#include "shdhwshader.h"


DECLARE_FORCE_LINK(LegacyW3DShader);
REGISTER_SHDDEF(ShdLegacyW3DDefClass,SHDDEF_CLASSID_LEGACYW3D,"LegacyW3D");


// Save-Load methods for ShdDefClass
enum 
{
	SHDLEGACY_CHUNKID_VARIABLES =		0x11070958,

	SHDLEGACY_CHUNKID_TEXTURE_NAME00,
	SHDLEGACY_CHUNKID_TEXTURE_NAME01,
	SHDLEGACY_CHUNKID_TEXTURE_NAME10,
	SHDLEGACY_CHUNKID_TEXTURE_NAME11,
	SHDLEGACY_CHUNKID_TEXTURE_NAME20,
	SHDLEGACY_CHUNKID_TEXTURE_NAME21,
	SHDLEGACY_CHUNKID_TEXTURE_NAME30,
	SHDLEGACY_CHUNKID_TEXTURE_NAME31,
		
	SHDLEGACY_CHUNKID_MAPPERARGS00,
	SHDLEGACY_CHUNKID_MAPPERARGS01,
	SHDLEGACY_CHUNKID_MAPPERARGS10,
	SHDLEGACY_CHUNKID_MAPPERARGS11,
	SHDLEGACY_CHUNKID_MAPPERARGS20,
	SHDLEGACY_CHUNKID_MAPPERARGS21,
	SHDLEGACY_CHUNKID_MAPPERARGS30,
	SHDLEGACY_CHUNKID_MAPPERARGS31,

	VARID_PASS_COUNT =					0x00,

	VARID_SHADER0,
	VARID_SHADER1,
	VARID_SHADER2,
	VARID_SHADER3,

	VARID_MATERIAL0,
	VARID_MATERIAL1,
	VARID_MATERIAL2,
	VARID_MATERIAL3,

	VARID_TEXTURE_ATTRIBUTES00,
	VARID_TEXTURE_ATTRIBUTES01,
	VARID_TEXTURE_ATTRIBUTES10,
	VARID_TEXTURE_ATTRIBUTES11,
	VARID_TEXTURE_ATTRIBUTES20,
	VARID_TEXTURE_ATTRIBUTES21,
	VARID_TEXTURE_ATTRIBUTES30,
	VARID_TEXTURE_ATTRIBUTES31,

	VARID_MAPCHANNEL00,
	VARID_MAPCHANNEL01,
	VARID_MAPCHANNEL10,
	VARID_MAPCHANNEL11,
	VARID_MAPCHANNEL20,
	VARID_MAPCHANNEL21,
	VARID_MAPCHANNEL30,
	VARID_MAPCHANNEL31,

	VARID_AMBIENT_COLOR,
	VARID_DIFFUSE_COLOR
};


ShdLegacyW3DDefClass::ShdLegacyW3DDefClass() 
:	ShdDefClass(SHDDEF_CLASSID_LEGACYW3D),
	PassCount(0)
{
	// init defaults
	for (int pass_index=0; pass_index < MeshMatDescClass::MAX_PASSES; pass_index++) {
		memset(&(Shaders[pass_index]),0,sizeof(W3dShaderStruct));
		memset(&(Materials[pass_index]),0,sizeof(W3dVertexMaterialStruct));

		for (int stage_index=0; stage_index < MeshMatDescClass::MAX_TEX_STAGES; stage_index++) {
			TextureAttributes[pass_index][stage_index] = 0;
			MapChannel[pass_index][stage_index] = 0;
		}
	}
}

ShdLegacyW3DDefClass::ShdLegacyW3DDefClass(const ShdLegacyW3DDefClass& that)
:	ShdDefClass(that),
	PassCount(that.PassCount)
{
	// init defaults
	for (int pass_index=0; pass_index < MeshMatDescClass::MAX_PASSES; pass_index++) {
		memset(&(Shaders[pass_index]),0,sizeof(W3dShaderStruct));
		memset(&(Materials[pass_index]),0,sizeof(W3dVertexMaterialStruct));

		for (int stage_index=0; stage_index < MeshMatDescClass::MAX_TEX_STAGES; stage_index++) {
			TextureAttributes[pass_index][stage_index] = 0;
			MapChannel[pass_index][stage_index] = 0;
		}
	}

	for (int pass=0;pass<PassCount;++pass) {
		for (int stage=0;stage<MeshMatDescClass::MAX_TEX_STAGES;++stage) {
			TextureNames[pass][stage]=that.TextureNames[pass][stage];
		}
//		Shaders[pass]=that.Shaders[pass];
//		Materials[pass]=that.Materials[pass];
	}
}

ShdLegacyW3DDefClass::~ShdLegacyW3DDefClass()
{
}

bool ShdLegacyW3DDefClass::Is_Valid_Config(StringClass &message)
{
	return true;
}

bool ShdLegacyW3DDefClass::Uses_UV_Channel(int i) const
{
	for (int pass=0; pass<PassCount; pass++) {
		for (int stage=0; stage<MeshMatDescClass::MAX_TEX_STAGES; stage++) {
			if (MapChannel[pass][stage] == i) {
				return true;
			}
		}
	}
	return false;
}

bool ShdLegacyW3DDefClass::Save(ChunkSaveClass &csave)
{
	// Call parent class
	ShdDefClass::Save(csave);

	// Save our variables
	csave.Begin_Chunk(SHDLEGACY_CHUNKID_VARIABLES);	
	Save_Variables(csave);	
	csave.End_Chunk();

	// Save any mapper args and texture names present
	for (int pass_index=0; pass_index<PassCount; pass_index++) {
		for (int stage_index=0; stage_index<2; stage_index++) {
	
			// Write the texture name
			if (!TextureNames[pass_index][stage_index].Is_Empty()) {

/* (gth) need to save entire path for Max to work properly right???  
				char fname[_MAX_PATH];
				char extension[_MAX_PATH];
				_splitpath(TextureNames[pass_index][stage_index].Peek_Buffer(),NULL,NULL,fname,extension);
				strcat(fname,extension);
*/
				int chunk_id = SHDLEGACY_CHUNKID_TEXTURE_NAME00 + pass_index * 2 + stage_index;
				WRITE_WWSTRING_CHUNK(csave,chunk_id,TextureNames[pass_index][stage_index]);
			}

			// Write the mapper args			
			if (!MapperArgs[pass_index][stage_index].Is_Empty()) {
				int chunk_id = SHDLEGACY_CHUNKID_MAPPERARGS00 + pass_index * 2 + stage_index;
				WRITE_WWSTRING_CHUNK(csave,chunk_id,MapperArgs[pass_index][stage_index])
			}
		}
	}

	return false;
}

bool ShdLegacyW3DDefClass::Load(ChunkLoadClass &cload)
{	
	int offset = 0;
	int pass = 0;
	int stage = 0;

	// Call parent class
	ShdDefClass::Load(cload);
	
	//	Read in the chunks
	while (cload.Open_Chunk()) {
		switch (cload.Cur_Chunk_ID())
		{
		case SHDLEGACY_CHUNKID_VARIABLES:
			Load_Variables(cload);
			break;
		
		case SHDLEGACY_CHUNKID_TEXTURE_NAME00:
		case SHDLEGACY_CHUNKID_TEXTURE_NAME01:
		case SHDLEGACY_CHUNKID_TEXTURE_NAME10:
		case SHDLEGACY_CHUNKID_TEXTURE_NAME11:
		case SHDLEGACY_CHUNKID_TEXTURE_NAME20:
		case SHDLEGACY_CHUNKID_TEXTURE_NAME21:
		case SHDLEGACY_CHUNKID_TEXTURE_NAME30:
		case SHDLEGACY_CHUNKID_TEXTURE_NAME31:
			offset = cload.Cur_Chunk_ID() - SHDLEGACY_CHUNKID_TEXTURE_NAME00;
			pass = offset / 2;
			stage = offset % 2;
			cload.Read(TextureNames[pass][stage].Get_Buffer(cload.Cur_Chunk_Length()),cload.Cur_Chunk_Length());
			break;

		case SHDLEGACY_CHUNKID_MAPPERARGS00:
		case SHDLEGACY_CHUNKID_MAPPERARGS01:
		case SHDLEGACY_CHUNKID_MAPPERARGS10:
		case SHDLEGACY_CHUNKID_MAPPERARGS11:
		case SHDLEGACY_CHUNKID_MAPPERARGS20:
		case SHDLEGACY_CHUNKID_MAPPERARGS21:
		case SHDLEGACY_CHUNKID_MAPPERARGS30:
		case SHDLEGACY_CHUNKID_MAPPERARGS31:
			offset = cload.Cur_Chunk_ID() - SHDLEGACY_CHUNKID_MAPPERARGS00;
			pass = offset / 2;
			stage = offset % 2;
			cload.Read(MapperArgs[pass][stage].Get_Buffer(cload.Cur_Chunk_Length()),cload.Cur_Chunk_Length());
			break;

		default:
			break;
		}

		cload.Close_Chunk();
	}

	return true;
}

bool ShdLegacyW3DDefClass::Save_Variables(ChunkSaveClass &csave)
{
	WRITE_MICRO_CHUNK(csave, VARID_PASS_COUNT, PassCount);

	for (int pass_index=0; pass_index<PassCount; pass_index++) {
		
		// Write the shader and vertex material
		WRITE_MICRO_CHUNK(csave, VARID_SHADER0 + pass_index, Shaders[pass_index]);
		WRITE_MICRO_CHUNK(csave, VARID_MATERIAL0 + pass_index, Materials[pass_index]);

		for (int stage_index=0; stage_index<2; stage_index++) {

			// Write the texture attributes
			if (TextureAttributes[pass_index][stage_index] != 0) {
				WRITE_MICRO_CHUNK(csave, VARID_TEXTURE_ATTRIBUTES00 + pass_index*2 + stage_index, TextureAttributes[pass_index][stage_index]);
			}

			// Write the map channels
			if (MapChannel[pass_index][stage_index] != 0) {
				WRITE_MICRO_CHUNK(csave, VARID_MAPCHANNEL00 + pass_index*2 + stage_index, MapChannel[pass_index][stage_index]);
			}

		}
	}
	return true;
}

bool ShdLegacyW3DDefClass::Load_Variables(ChunkLoadClass &cload)
{
	int offset = 0;
	int pass = 0;
	int stage = 0;

	while (cload.Open_Micro_Chunk()) 
	{
		switch (cload.Cur_Micro_Chunk_ID()) 
		{
			READ_MICRO_CHUNK(cload, VARID_PASS_COUNT, PassCount);

			case VARID_SHADER0:
			case VARID_SHADER1:
			case VARID_SHADER2:
			case VARID_SHADER3:
				pass = cload.Cur_Micro_Chunk_ID() - VARID_SHADER0;
				cload.Read(&Shaders[pass],sizeof(Shaders[pass]));
				break;

			case VARID_MATERIAL0:
			case VARID_MATERIAL1:
			case VARID_MATERIAL2:
			case VARID_MATERIAL3:
				pass = cload.Cur_Micro_Chunk_ID() - VARID_MATERIAL0;
				cload.Read(&Materials[pass],sizeof(Materials[pass]));
				break;

			case VARID_TEXTURE_ATTRIBUTES00:
			case VARID_TEXTURE_ATTRIBUTES01:
			case VARID_TEXTURE_ATTRIBUTES10:
			case VARID_TEXTURE_ATTRIBUTES11:
			case VARID_TEXTURE_ATTRIBUTES20:
			case VARID_TEXTURE_ATTRIBUTES21:
			case VARID_TEXTURE_ATTRIBUTES30:
			case VARID_TEXTURE_ATTRIBUTES31:
				offset = cload.Cur_Micro_Chunk_ID() - VARID_TEXTURE_ATTRIBUTES00;
				pass = offset / 2;
				stage = offset % 2;
				cload.Read(&TextureAttributes[pass][stage],sizeof(TextureAttributes[pass][stage]));
				break;

			case VARID_MAPCHANNEL00:
			case VARID_MAPCHANNEL01:
			case VARID_MAPCHANNEL10:
			case VARID_MAPCHANNEL11:
			case VARID_MAPCHANNEL20:
			case VARID_MAPCHANNEL21:
			case VARID_MAPCHANNEL30:
			case VARID_MAPCHANNEL31:
				offset = cload.Cur_Micro_Chunk_ID() - VARID_MAPCHANNEL00;
				pass = offset / 2;
				stage = offset % 2;
				cload.Read(&MapChannel[pass][stage],sizeof(MapChannel[pass][stage]));
				break;
		}

		cload.Close_Micro_Chunk();
	}
	return true;
}


void ShdLegacyW3DDefClass::Init()
{
	Shd6LegacyW3DClass::Init();
}

void ShdLegacyW3DDefClass::Shutdown()
{
	Shd6LegacyW3DClass::Shutdown();
}

ShdInterfaceClass* ShdLegacyW3DDefClass::Create() const
{
	return NEW_REF(Shd6LegacyW3DClass,(this));
}

Shd6LegacyW3DClass::Shd6LegacyW3DClass(const ShdDefClass* def)
:	ShdInterfaceClass(def,SHDDEF_CLASSID_LEGACYW3D),
	PassCount(0)
{
	int pass;
	int stage;
	for (pass=0;pass<MeshMatDescClass::MAX_PASSES;++pass) {
		Materials[pass]=NULL;
		for (int stage=0;stage<MeshMatDescClass::MAX_TEX_STAGES;++stage) {
			Textures[pass][stage]=NULL;
		}
	}

	ShdLegacyW3DDefClass* Definition=(ShdLegacyW3DDefClass*)def;
	PassCount=Definition->Get_Pass_Count();
	
	for (pass=0;pass<PassCount;++pass) {
		
		// Set up the shaders
		W3dUtilityClass::Convert_Shader(Definition->Get_Shader(pass),&(Shaders[pass]));

		// Set up the vertex materials
		VertexMaterialClass * vmat = NEW_REF(VertexMaterialClass,());
		vmat->Parse_W3dVertexMaterialStruct(Definition->Get_Material(pass));

		char * map0_args = NULL;
		char * map1_args = NULL;
		if (!Definition->Get_Mapper_Args(pass,0).Is_Empty()) {
			map0_args = Definition->Get_Mapper_Args(pass,0).Peek_Buffer();
		}
		if (!Definition->Get_Mapper_Args(pass,1).Is_Empty()) {
			map1_args = Definition->Get_Mapper_Args(pass,1).Peek_Buffer();
		}
		vmat->Parse_Mapping_Args(Definition->Get_Material(pass),map0_args,map1_args);
		vmat->Set_UV_Source(0,Definition->Get_Map_Channel(pass,0));
		vmat->Set_UV_Source(1,Definition->Get_Map_Channel(pass,1));
		vmat->Set_Lighting(true);
		REF_PTR_SET(Materials[pass],vmat);
		REF_PTR_RELEASE(vmat);

		// Set up the textures
		for (stage=0;stage<MeshMatDescClass::MAX_TEX_STAGES;++stage) {
			StringClass tex_name=Definition->Get_Texture_Name(pass,stage);
			if (tex_name!="") {
				Textures[pass][stage]=WW3DAssetManager::Get_Instance()->Get_Texture(tex_name);
			}
			else {
				Textures[pass][stage]=NULL;
			}
		}
	}
}

Shd6LegacyW3DClass::~Shd6LegacyW3DClass()
{
	for (int pass=0;pass<PassCount;++pass) {
		for (int stage=0;stage<MeshMatDescClass::MAX_TEX_STAGES;++stage) {
			REF_PTR_RELEASE(Textures[pass][stage]);
		}
		REF_PTR_RELEASE(Materials[pass]);
	}
}

void Shd6LegacyW3DClass::Init()
{
}

void Shd6LegacyW3DClass::Shutdown()
{
}

bool Shd6LegacyW3DClass::Greater_Than(const ShdInterfaceClass& s,int pass) const
{
	// By class
	if (s.Get_Class_ID()!=Get_Class_ID()) {
		return s.Get_Class_ID()>Get_Class_ID();
	}

	const Shd6LegacyW3DClass* src=static_cast<const Shd6LegacyW3DClass*>(&s);

	// If same class, sort by textures
	if (src->Textures[pass][0]!=Textures[pass][0]) {
		return src->Textures[pass][0]>Textures[pass][0];
	}
	
	// If same texture, sort by materials
	if (src->Materials[pass]!=Materials[pass]) {
		return src->Materials[pass]>Materials[pass];
	}
	
	// If same material, sort by shaders
	if (src->Shaders[pass].Get_Bits()!=(unsigned)Shaders[pass].Get_Bits()) {
		return src->Shaders[pass].Get_Bits()>Shaders[pass].Get_Bits();
	}
	
	return false;
}



/**********************************************************************************************
//! Apply shared states for 1 pass DX6
/*! 7/12/02 3:39p KJM Created
*/
void Shd6LegacyW3DClass::Apply_Shared(int pass, RenderInfoClass& rinfo)
{
	SNAPSHOT_SAY(("Shd6LegacyW3DClass::Apply_Shared(pass: %d)\n",pass));
	// fixed function uses pass through by default
	DX8Wrapper::Set_DX8_Texture_Stage_State(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU);
}

//**********************************************************************************************
//! Apply per instance states for 1 pass DX6 
/*! 7/10/02 5:39p KJM Created
*/
void Shd6LegacyW3DClass::Apply_Instance(int cur_pass, RenderInfoClass& rinfo)
{
	SNAPSHOT_SAY(("Shd6LegacyW3DClass::Apply_Instance(pass: %d)\n",cur_pass));

	// set vertex shader
	StringClass str;
	SNAPSHOT_SAY(("legacyshader:DX8Wrapper::Set_Shader(%s)\n",Shaders[cur_pass].Get_Description(str)));
	DX8Wrapper::Set_Shader(Shaders[cur_pass]);

	DX8Wrapper::Set_Vertex_Shader(FVF);

	SNAPSHOT_SAY(("legacyshader:DX8Wrapper::Set_Material(%s)\n",Materials[cur_pass] ? Materials[cur_pass]->Get_Name() : "NULL"));
	DX8Wrapper::Set_Material(Materials[cur_pass]);

	for (int stage=0;stage<MeshMatDescClass::MAX_TEX_STAGES;++stage) {
		SNAPSHOT_SAY(("legacyshader:DX8Wrapper::Set_Texture(%d,%s)\n",stage,Textures[cur_pass][stage] ? Textures[cur_pass][stage]->Get_Full_Path().Peek_Buffer() : "NULL"));
		DX8Wrapper::Set_Texture(stage, Textures[cur_pass][stage]);
	}
}

unsigned Shd6LegacyW3DClass::Get_Vertex_Stream_Count() const
{
	return 1;
}

unsigned Shd6LegacyW3DClass::Get_Vertex_Size(unsigned stream) const
{
	FVFInfoClass fvf_info(FVF);
	return fvf_info.Get_FVF_Size();
}

void Shd6LegacyW3DClass::Copy_Vertex_Stream
(
	unsigned stream, 
	void* dest_buffer, 
	const VertexStreamStruct& vss, 
	unsigned vertex_count
)
{
	FVFInfoClass fi(FVF);

	/*
	** Append the UV coordinates to the vertex buffer
	*/
	int uvcount = 0;
	if ((FVF&D3DFVF_TEX1) == D3DFVF_TEX1) {
		uvcount = 1;
	}
	if ((FVF&D3DFVF_TEX2) == D3DFVF_TEX2) {
		uvcount = 2;
	}
	if ((FVF&D3DFVF_TEX3) == D3DFVF_TEX3) {
		uvcount = 3;
	}
	if ((FVF&D3DFVF_TEX4) == D3DFVF_TEX4) {
		uvcount = 4;
	}
	if ((FVF&D3DFVF_TEX5) == D3DFVF_TEX5) {
		uvcount = 5;
	}
	if ((FVF&D3DFVF_TEX6) == D3DFVF_TEX6) {
		uvcount = 6;
	}
	if ((FVF&D3DFVF_TEX7) == D3DFVF_TEX7) {
		uvcount = 7;
	}
	if ((FVF&D3DFVF_TEX8) == D3DFVF_TEX8) {
		uvcount = 8;
	}

	unsigned char* vb=(unsigned char*)dest_buffer;

	for (unsigned i=0; i<vertex_count; ++i) 
	{
		if ((FVF&D3DFVF_XYZ)==D3DFVF_XYZ) {
			if (vss.Locations) 
			{
				*(Vector3*)(vb+fi.Get_Location_Offset())=vss.Locations[i];
			}
			else 
			{
				*(Vector3*)(vb+fi.Get_Location_Offset())=Vector3(0.0f,0.0f,0.0f);
			}
		}

		if ((FVF&D3DFVF_DIFFUSE)==D3DFVF_DIFFUSE) {
			if (vss.DiffuseInt) 
			{
				*(unsigned int*)(vb+fi.Get_Diffuse_Offset())=vss.DiffuseInt[i];
			}
			else 
			{
				*(unsigned int*)(vb+fi.Get_Diffuse_Offset())=0xffffffff;
			}
		}
	
		if ((FVF&D3DFVF_NORMAL)==D3DFVF_NORMAL) {
			if (vss.Normals) 
			{
				*(Vector3*)(vb+fi.Get_Normal_Offset())=vss.Normals[i];
			}
			else 
			{
				*(Vector3*)(vb+fi.Get_Normal_Offset())=Vector3(0.0f,0.0f,0.0f);
			}
		}

		
		for (int j=0; j<uvcount; j++) {
			const Vector2*uvs=vss.UV[j];
			if (uvs) {
				*(Vector2*)(vb+fi.Get_Tex_Offset(j))=uvs[i];
			}
			else {
				*(Vector2*)(vb+fi.Get_Tex_Offset(j))=Vector2(0.0f,0.0f);
			}
		}

		vb+=fi.Get_FVF_Size();
	}
}

