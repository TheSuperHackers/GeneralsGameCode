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
 *                 Project Name : WW3D                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/vertmaterial.cpp                       $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 8/22/01 11:06a                                              $*
 *                                                                                             *
 *                    $Revision:: 42                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Init -- init code                                                                         *
 *   Shutdown -- shutdown code                                                                 *
 *   Get_Preset -- retrieve presets                                                            *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "vertmaterial.h"
#include "realcrc.h"
#include "wwdebug.h"
#include "w3d_util.h"
#include "chunkio.h"
#include "w3derr.h"
#include "INI.h"
#include "XSTRAW.h"
#include "dx8wrapper.h"


static unsigned int unique=1;

VertexMaterialClass* VertexMaterialClass::Presets[VertexMaterialClass::PRESET_COUNT];

#ifdef DYN_MAT8
class DynD3DMATERIAL8 : public W3DMPO
{
	W3DMPO_GLUE(DynD3DMATERIAL8)
public:
	D3DMATERIAL8 Mat;
};
#define Material				(&MaterialDyn->Mat)
#define SRCMATPTR(src)	(&(src)->MaterialDyn->Mat)
#else
#define Material				(MaterialOld)
#define SRCMATPTR(src)	((src)->MaterialOld)
#endif

/*
** VertexMaterialClass Implementation
*/
VertexMaterialClass::VertexMaterialClass(void):
#ifdef DYN_MAT8
	MaterialDyn(NULL),
#else
	MaterialOld(NULL),
#endif
	Flags(0),
	AmbientColorSource(D3DMCS_MATERIAL),
	EmissiveColorSource(D3DMCS_MATERIAL),
	DiffuseColorSource(D3DMCS_MATERIAL),
	UseLighting(false),
	UniqueID(0),
	CRCDirty(true)
{
	int i;

	for (i=0; i<MeshBuilderClass::MAX_STAGES; i++)
	{
		Mapper[i]=NULL;
		UVSource[i] = i;
	}

#ifdef DYN_MAT8
	MaterialDyn=W3DNEW DynD3DMATERIAL8;
#else
	MaterialOld=W3DNEW D3DMATERIAL8;
#endif
	memset(Material,0,sizeof(D3DMATERIAL8));
	Set_Ambient(1.0f,1.0f,1.0f);
	Set_Diffuse(1.0f,1.0f,1.0f);

	Set_Opacity(1.0f);
}

VertexMaterialClass::VertexMaterialClass(const VertexMaterialClass & src) :
#ifdef DYN_MAT8
	MaterialDyn(NULL),
#else
	MaterialOld(NULL),
#endif
	Flags(src.Flags),
	AmbientColorSource(src.AmbientColorSource),
	EmissiveColorSource(src.EmissiveColorSource),
	DiffuseColorSource(src.DiffuseColorSource),
	UseLighting(src.UseLighting),
	Name(src.Name),
	UniqueID(src.UniqueID),
	CRCDirty(true)
{
	int i;
	for (i=0; i<MeshBuilderClass::MAX_STAGES; i++)
	{
		Mapper[i]=NULL;
		if (src.Mapper[i])
		{
			TextureMapperClass *mapper=src.Mapper[i]->Clone();
			Set_Mapper(mapper,i);
			mapper->Release_Ref();
		}

		UVSource[i] = src.UVSource[i];
	}

#ifdef DYN_MAT8
	MaterialDyn=W3DNEW DynD3DMATERIAL8;
#else
	MaterialOld=W3DNEW D3DMATERIAL8;
#endif
	memcpy(Material, SRCMATPTR(&src), sizeof(D3DMATERIAL8));
}

void VertexMaterialClass::Make_Unique()
{
	CRCDirty=true;
	UniqueID=unique;
	unique++;
}

VertexMaterialClass::~VertexMaterialClass(void)
{
	int i;

	for (i=0; i<MeshBuilderClass::MAX_STAGES; i++)
	{
		if (Mapper[i])
		{
			REF_PTR_RELEASE(Mapper[i]);
			Mapper[i]=NULL;
		}
	}

#ifdef DYN_MAT8
	delete MaterialDyn;
#else
	delete MaterialOld;
#endif
}

VertexMaterialClass & VertexMaterialClass::operator = (const VertexMaterialClass &src)
{

	if (this != &src) {
		Name=src.Name;
		Flags = src.Flags;
		AmbientColorSource = src.AmbientColorSource;
		EmissiveColorSource = src.EmissiveColorSource;
		DiffuseColorSource = src.DiffuseColorSource;
		UseLighting=src.UseLighting;
		UniqueID=src.UniqueID;
		CRCDirty=src.CRCDirty;
		int stage;
		for (stage=0;stage<MeshBuilderClass::MAX_STAGES;++stage) {
			if (Mapper[stage] != NULL) {
				Mapper[stage]->Release_Ref();
				Mapper[stage] = NULL;
			}
		}
		for (stage=0;stage<MeshBuilderClass::MAX_STAGES;++stage) {
			if (src.Mapper[stage]) {
				TextureMapperClass *mapper = src.Mapper[stage]->Clone();
				Set_Mapper(mapper,stage);
				mapper->Release_Ref();
			}
			UVSource[stage] = src.UVSource[stage];
		}

		*Material = *SRCMATPTR(&src);
	}
	return *this;
}

unsigned long VertexMaterialClass::Compute_CRC(void) const
{
	unsigned long crc = 0;

// don't include the name when determining whether two vertex materials match
//	crc = CRC_Memory(reinterpret_cast<const unsigned char *>(Name.Peek_Buffer()),sizeof(char)*strlen(Name),crc);

	crc = CRC_Memory(reinterpret_cast<const unsigned char *>(Material),sizeof(D3DMATERIAL8),crc);
	crc = CRC_Memory(reinterpret_cast<const unsigned char *>(&Flags),sizeof(Flags),crc);
	crc = CRC_Memory(reinterpret_cast<const unsigned char *>(&DiffuseColorSource),sizeof(DiffuseColorSource),crc);
	crc = CRC_Memory(reinterpret_cast<const unsigned char *>(&AmbientColorSource),sizeof(AmbientColorSource),crc);
	crc = CRC_Memory(reinterpret_cast<const unsigned char *>(&EmissiveColorSource),sizeof(EmissiveColorSource),crc);
	crc = CRC_Memory(reinterpret_cast<const unsigned char *>(&UVSource),sizeof(UVSource),crc);
	crc = CRC_Memory(reinterpret_cast<const unsigned char *>(&UseLighting),sizeof(UseLighting),crc);
	crc = CRC_Memory(reinterpret_cast<const unsigned char *>(&UniqueID),sizeof(UniqueID),crc);

	int i;
	for (i=0; i<MeshBuilderClass::MAX_STAGES; i++)
	{
		if (Mapper[i]) crc = CRC_Memory(reinterpret_cast<const unsigned char *>(&(Mapper[i])),sizeof(TextureMapperClass*),crc);
	}

	return crc;
}

// Ambient Get and Sets

void VertexMaterialClass::Get_Ambient(Vector3 * set) const
{
	assert(set);
	*set=Vector3(Material->Ambient.r,Material->Ambient.g,Material->Ambient.b);
}

void VertexMaterialClass::Set_Ambient(const Vector3 & color)
{
	CRCDirty=true;
	Material->Ambient.r=color.X;
	Material->Ambient.g=color.Y;
	Material->Ambient.b=color.Z;
}

void VertexMaterialClass::Set_Ambient(float r,float g,float b)
{
	CRCDirty=true;
	Material->Ambient.r=r;
	Material->Ambient.g=g;
	Material->Ambient.b=b;
}

// Diffuse Get and Sets

void VertexMaterialClass::Get_Diffuse(Vector3 * set) const
{
	assert(set);
	*set=Vector3(Material->Diffuse.r,Material->Diffuse.g,Material->Diffuse.b);
}

void VertexMaterialClass::Set_Diffuse(const Vector3 & color)
{
	CRCDirty=true;
	Material->Diffuse.r=color.X;
	Material->Diffuse.g=color.Y;
	Material->Diffuse.b=color.Z;
}

void VertexMaterialClass::Set_Diffuse(float r,float g,float b)
{
	CRCDirty=true;
	Material->Diffuse.r=r;
	Material->Diffuse.g=g;
	Material->Diffuse.b=b;
}

// Specular Get and Sets

void VertexMaterialClass::Get_Specular(Vector3 * set) const
{
	assert(set);
	*set=Vector3(Material->Specular.r,Material->Specular.g,Material->Specular.b);
}

void VertexMaterialClass::Set_Specular(const Vector3 & color)
{
	CRCDirty=true;
	Material->Specular.r=color.X;
	Material->Specular.g=color.Y;
	Material->Specular.b=color.Z;
}

void VertexMaterialClass::Set_Specular(float r,float g,float b)
{
	CRCDirty=true;
	Material->Specular.r=r;
	Material->Specular.g=g;
	Material->Specular.b=b;
}

// Emissive Get and Sets

void VertexMaterialClass::Get_Emissive(Vector3 * set) const
{
	assert(set);
	*set=Vector3(Material->Emissive.r,Material->Emissive.g,Material->Emissive.b);
}

void VertexMaterialClass::Set_Emissive(const Vector3 & color)
{
	CRCDirty=true;
	Material->Emissive.r=color.X;
	Material->Emissive.g=color.Y;
	Material->Emissive.b=color.Z;
}

void VertexMaterialClass::Set_Emissive(float r,float g,float b)
{
	CRCDirty=true;
	Material->Emissive.r=r;
	Material->Emissive.g=g;
	Material->Emissive.b=b;
}


float	VertexMaterialClass::Get_Shininess(void) const
{
	return Material->Power;
}

void	VertexMaterialClass::Set_Shininess(float shin)
{
	CRCDirty=true;
	Material->Power=shin;
}

float	VertexMaterialClass::Get_Opacity(void) const
{
	return Material->Diffuse.a;
}

void	VertexMaterialClass::Set_Opacity(float o)
{
	CRCDirty=true;
	Material->Diffuse.a=o;
}

void	VertexMaterialClass::Set_Ambient_Color_Source(ColorSourceType src)
{
	CRCDirty=true;
	switch (src)
	{
	case	COLOR1:		AmbientColorSource = D3DMCS_COLOR1; break;
	case	COLOR2:		AmbientColorSource = D3DMCS_COLOR2; break;
	default:				AmbientColorSource = D3DMCS_MATERIAL; break;
	}
}

void	VertexMaterialClass::Set_Emissive_Color_Source(ColorSourceType src)
{
	CRCDirty=true;
	switch (src)
	{
	case	COLOR1:		EmissiveColorSource = D3DMCS_COLOR1; break;
	case	COLOR2:		EmissiveColorSource = D3DMCS_COLOR2; break;
	default:				EmissiveColorSource = D3DMCS_MATERIAL; break;
	}
}

void	VertexMaterialClass::Set_Diffuse_Color_Source(ColorSourceType src)
{
	CRCDirty=true;
	switch (src)
	{
	case	COLOR1:		DiffuseColorSource = D3DMCS_COLOR1; break;
	case	COLOR2:		DiffuseColorSource = D3DMCS_COLOR2; break;
	default:				DiffuseColorSource = D3DMCS_MATERIAL; break;
	}
}

VertexMaterialClass::ColorSourceType
VertexMaterialClass::Get_Ambient_Color_Source(void)
{
	switch(AmbientColorSource)
	{
	case D3DMCS_COLOR1:	return COLOR1;
	case D3DMCS_COLOR2:	return COLOR2;
	default:					return MATERIAL;
	}
}

VertexMaterialClass::ColorSourceType
VertexMaterialClass::Get_Emissive_Color_Source(void)
{
	switch(EmissiveColorSource)
	{
	case D3DMCS_COLOR1:	return COLOR1;
	case D3DMCS_COLOR2:	return COLOR2;
	default:					return MATERIAL;
	}
}

VertexMaterialClass::ColorSourceType
VertexMaterialClass::Get_Diffuse_Color_Source(void)
{
	switch(DiffuseColorSource)
	{
	case D3DMCS_COLOR1:	return COLOR1;
	case D3DMCS_COLOR2:	return COLOR2;
	default:					return MATERIAL;
	}
}

void VertexMaterialClass::Set_UV_Source(int stage,int array_index)
{
	WWASSERT(stage >= 0);
	WWASSERT(stage < MeshBuilderClass::MAX_STAGES);
	WWASSERT(array_index >= 0);
	WWASSERT(array_index < 8);
	CRCDirty=true;
	UVSource[stage] = array_index;
}

int VertexMaterialClass::Get_UV_Source(int stage)
{
	WWASSERT(stage >= 0);
	WWASSERT(stage < MeshBuilderClass::MAX_STAGES);
	return UVSource[stage];
}


void VertexMaterialClass::Init_From_Material3(const W3dMaterial3Struct & mat3)
{
	Vector3 tmp0,tmp1,tmp2;

	W3dUtilityClass::Convert_Color(mat3.DiffuseColor,&tmp0);
	W3dUtilityClass::Convert_Color(mat3.DiffuseCoefficients,&tmp1);
	tmp2.X = tmp0.X * tmp1.X;
	tmp2.Y = tmp0.Y * tmp1.Y;
	tmp2.Z = tmp0.Z * tmp1.Z;
	Set_Diffuse(tmp2);

	W3dUtilityClass::Convert_Color(mat3.SpecularColor,&tmp0);
	W3dUtilityClass::Convert_Color(mat3.SpecularCoefficients,&tmp1);
	tmp2.X = tmp0.X * tmp1.X;
	tmp2.Y = tmp0.Y * tmp1.Y;
	tmp2.Z = tmp0.Z * tmp1.Z;
	Set_Specular(tmp2);

	W3dUtilityClass::Convert_Color(mat3.EmissiveCoefficients,&tmp0);
	Set_Emissive(tmp0);

	W3dUtilityClass::Convert_Color(mat3.AmbientCoefficients,&tmp0);
	Set_Ambient(tmp0);

	Set_Shininess(mat3.Shininess);
	Set_Opacity(mat3.Opacity);
}

WW3DErrorType VertexMaterialClass::Load_W3D(ChunkLoadClass & cload)
{
	char name[256];

	W3dVertexMaterialStruct vmat;
	bool hasname = false;

	char *mapping0_arg_buffer = NULL;
	char *mapping1_arg_buffer = NULL;
	unsigned int mapping0_arg_len = 0U;
	unsigned int mapping1_arg_len = 0U;

	while (cload.Open_Chunk()) {
		switch (cload.Cur_Chunk_ID()) {
			case W3D_CHUNK_VERTEX_MATERIAL_NAME:
				cload.Read(&name,cload.Cur_Chunk_Length());
				hasname = true;
				break;

			case W3D_CHUNK_VERTEX_MATERIAL_INFO:
				if (cload.Read(&vmat,sizeof(vmat)) != sizeof(vmat)) {
					return WW3D_ERROR_LOAD_FAILED;
				}
				break;

			case W3D_CHUNK_VERTEX_MAPPER_ARGS0:
				mapping0_arg_len = cload.Cur_Chunk_Length();
				mapping0_arg_buffer = MSGW3DNEWARRAY("VertexMaterialClassTemp") char[mapping0_arg_len];
				if (cload.Read(mapping0_arg_buffer, mapping0_arg_len) != mapping0_arg_len) {
					return WW3D_ERROR_LOAD_FAILED;
				}
				break;

			case W3D_CHUNK_VERTEX_MAPPER_ARGS1:
				mapping1_arg_len = cload.Cur_Chunk_Length();
				mapping1_arg_buffer = MSGW3DNEWARRAY("VertexMaterialClassTemp") char[mapping1_arg_len];
				if (cload.Read(mapping1_arg_buffer, mapping1_arg_len) != mapping1_arg_len) {
					return WW3D_ERROR_LOAD_FAILED;
				}
				break;
		};
		cload.Close_Chunk();
	}

	if (hasname) {
		Set_Name(name);
	}

	// Read an INIClass from the mapping argument buffer - this will be used
	// to initialize any special mappers used.
	INIClass mapping0_arg_ini;
	if (mapping0_arg_buffer) {

		char *extended_arg_buffer = MSGW3DNEWARRAY("VertexMaterialClassTemp") char[mapping0_arg_len + 10];
		sprintf(extended_arg_buffer, "[Args]\n%s", mapping0_arg_buffer);
		mapping0_arg_len = strlen(extended_arg_buffer) + 1;

		delete [] mapping0_arg_buffer;
		mapping0_arg_buffer = NULL;

		BufferStraw map_arg_buf_straw((void *)extended_arg_buffer, mapping0_arg_len);

		mapping0_arg_ini.Load(map_arg_buf_straw);

		delete [] extended_arg_buffer;
		extended_arg_buffer = NULL;
	}
	INIClass mapping1_arg_ini;
	if (mapping1_arg_buffer) {

		char *extended_arg_buffer = MSGW3DNEWARRAY("VertexMaterialClassTemp") char[mapping1_arg_len + 20];
		sprintf(extended_arg_buffer, "[Args]\n%s", mapping1_arg_buffer);
		mapping1_arg_len = strlen(extended_arg_buffer) + 1;

		delete [] mapping1_arg_buffer;
		mapping1_arg_buffer = NULL;

		BufferStraw map_arg_buf_straw((void *)extended_arg_buffer, mapping1_arg_len);

		mapping1_arg_ini.Load(map_arg_buf_straw);

		delete [] extended_arg_buffer;
		extended_arg_buffer = NULL;
	}

	if (vmat.Attributes & W3DVERTMAT_USE_DEPTH_CUE) {
		Set_Flag(VertexMaterialClass::DEPTH_CUE,true);
	}

	if (vmat.Attributes & W3DVERTMAT_COPY_SPECULAR_TO_DIFFUSE) {
		Set_Flag(VertexMaterialClass::COPY_SPECULAR_TO_DIFFUSE,true);
	}

	// Set up the vertex mapper.  If it is one of the simple
	// ones, set the pointer to one of the global instances.
	int mapping = vmat.Attributes & W3DVERTMAT_STAGE0_MAPPING_MASK;

	switch(mapping) {

		case W3DVERTMAT_STAGE0_MAPPING_UV:
			break;

		case W3DVERTMAT_STAGE0_MAPPING_ENVIRONMENT:
			{
				EnvironmentMapperClass *mapper = NEW_REF(EnvironmentMapperClass,(0));
				Set_Mapper(mapper);
				mapper->Release_Ref();
			}
			break;
		case W3DVERTMAT_STAGE0_MAPPING_CHEAP_ENVIRONMENT:
			{
				ClassicEnvironmentMapperClass *mapper = NEW_REF(ClassicEnvironmentMapperClass,(0));
				Set_Mapper(mapper);
				mapper->Release_Ref();
			}
			break;
		case W3DVERTMAT_STAGE0_MAPPING_LINEAR_OFFSET:
			{
				LinearOffsetTextureMapperClass *mapper =
					NEW_REF(LinearOffsetTextureMapperClass,(mapping0_arg_ini, "Args", 0));
				Set_Mapper(mapper);
				mapper->Release_Ref();
			}
			break;

		case W3DVERTMAT_STAGE0_MAPPING_SCREEN:
			{
				ScreenMapperClass *mapper =
					NEW_REF(ScreenMapperClass,(mapping0_arg_ini, "Args", 0));
				Set_Mapper(mapper);
				mapper->Release_Ref();
			}
			break;

		case W3DVERTMAT_STAGE0_MAPPING_SCALE:
			{
				ScaleTextureMapperClass *mapper =
					NEW_REF(ScaleTextureMapperClass,(mapping0_arg_ini, "Args", 0));
				Set_Mapper(mapper);
				mapper->Release_Ref();
			}
			break;

		case W3DVERTMAT_STAGE0_MAPPING_GRID:
			{
				GridTextureMapperClass *mapper =
					NEW_REF(GridTextureMapperClass,(mapping0_arg_ini, "Args", 0));
				Set_Mapper(mapper,0);
				mapper->Release_Ref();
			}
			break;

		case W3DVERTMAT_STAGE0_MAPPING_ROTATE:
			{
				RotateTextureMapperClass *mapper =
					NEW_REF(RotateTextureMapperClass,(mapping0_arg_ini, "Args", 0));
				Set_Mapper(mapper,0);
				mapper->Release_Ref();
			}
			break;

		case W3DVERTMAT_STAGE0_MAPPING_SINE_LINEAR_OFFSET:
			{
				SineLinearOffsetTextureMapperClass *mapper =
					NEW_REF(SineLinearOffsetTextureMapperClass,(mapping0_arg_ini, "Args", 0));
				Set_Mapper(mapper,0);
				mapper->Release_Ref();
			}
			break;

		case W3DVERTMAT_STAGE0_MAPPING_STEP_LINEAR_OFFSET:
			{
				StepLinearOffsetTextureMapperClass *mapper =
					NEW_REF(StepLinearOffsetTextureMapperClass,(mapping0_arg_ini, "Args", 0));
				Set_Mapper(mapper,0);
				mapper->Release_Ref();
			}
			break;

		case W3DVERTMAT_STAGE0_MAPPING_ZIGZAG_LINEAR_OFFSET:
			{
				ZigZagLinearOffsetTextureMapperClass *mapper =
					NEW_REF(ZigZagLinearOffsetTextureMapperClass,(mapping0_arg_ini, "Args", 0));
				Set_Mapper(mapper,0);
				mapper->Release_Ref();
			}
			break;

		case W3DVERTMAT_STAGE0_MAPPING_WS_CLASSIC_ENV:
			{
				WSClassicEnvironmentMapperClass *mapper = NEW_REF(WSClassicEnvironmentMapperClass,(0));
				Set_Mapper(mapper,0);
				mapper->Release_Ref();
			}
			break;

		case W3DVERTMAT_STAGE0_MAPPING_WS_ENVIRONMENT:
			{
				WSEnvironmentMapperClass *mapper = NEW_REF(WSEnvironmentMapperClass,(0));
				Set_Mapper(mapper,0);
				mapper->Release_Ref();
			}
			break;

		case W3DVERTMAT_STAGE0_MAPPING_GRID_CLASSIC_ENV:
			{
				GridClassicEnvironmentMapperClass *mapper =
					NEW_REF(GridClassicEnvironmentMapperClass,(mapping0_arg_ini, "Args", 0));
				Set_Mapper(mapper,0);
				mapper->Release_Ref();
			}
			break;

		case W3DVERTMAT_STAGE0_MAPPING_GRID_ENVIRONMENT:
			{
				GridEnvironmentMapperClass *mapper =
					NEW_REF(GridEnvironmentMapperClass,(mapping0_arg_ini, "Args", 0));
				Set_Mapper(mapper,0);
				mapper->Release_Ref();
			}
			break;

		case W3DVERTMAT_STAGE0_MAPPING_RANDOM:
			{
				RandomTextureMapperClass *mapper =
					NEW_REF(RandomTextureMapperClass,(mapping0_arg_ini, "Args", 0));
				Set_Mapper(mapper,0);
				mapper->Release_Ref();
			}
			break;

		case W3DVERTMAT_STAGE0_MAPPING_EDGE:
		{
			EdgeMapperClass *mapper =
				NEW_REF(EdgeMapperClass,(mapping0_arg_ini, "Args", 0));
			Set_Mapper(mapper,0);
			mapper->Release_Ref();
		}
		break;

		case W3DVERTMAT_STAGE0_MAPPING_BUMPENV:
		{
			BumpEnvTextureMapperClass *mapper =
				NEW_REF(BumpEnvTextureMapperClass,(mapping0_arg_ini, "Args", 0));
			Set_Mapper(mapper,0);
			mapper->Release_Ref();
		}
		break;

		default:
				WWDEBUG_SAY(("Unsupported mapper in %s",name));
			break;
	}

	// Same setup for stage 1's mapper.
	mapping = vmat.Attributes & W3DVERTMAT_STAGE1_MAPPING_MASK;
	switch(mapping) {

		case W3DVERTMAT_STAGE1_MAPPING_UV:
			break;

		case W3DVERTMAT_STAGE1_MAPPING_ENVIRONMENT:
		{
			EnvironmentMapperClass *mapper = W3DNEW EnvironmentMapperClass(1);
			Set_Mapper(mapper, 1);
			mapper->Release_Ref();
		}
		break;
		case W3DVERTMAT_STAGE1_MAPPING_CHEAP_ENVIRONMENT:
		{
			ClassicEnvironmentMapperClass *mapper = W3DNEW ClassicEnvironmentMapperClass(1);
			Set_Mapper(mapper, 1);
			mapper->Release_Ref();
		}
		break;

		case W3DVERTMAT_STAGE1_MAPPING_LINEAR_OFFSET:
		{
			LinearOffsetTextureMapperClass *mapper =
				W3DNEW LinearOffsetTextureMapperClass(mapping1_arg_ini, "Args", 1);
			Set_Mapper(mapper, 1);
			mapper->Release_Ref();
		}
		break;

		case W3DVERTMAT_STAGE1_MAPPING_SCREEN:
		{
			ScreenMapperClass *mapper =
				W3DNEW ScreenMapperClass(mapping1_arg_ini, "Args", 1);
			Set_Mapper(mapper, 1);
			mapper->Release_Ref();
		}
		break;

		case W3DVERTMAT_STAGE1_MAPPING_SCALE:
			{
				ScaleTextureMapperClass *mapper =
					NEW_REF(ScaleTextureMapperClass,(mapping1_arg_ini, "Args", 1));
				Set_Mapper(mapper,1);
				mapper->Release_Ref();
			}
			break;

		case W3DVERTMAT_STAGE1_MAPPING_GRID:
			{
				GridTextureMapperClass *mapper =
					NEW_REF(GridTextureMapperClass,(mapping1_arg_ini, "Args", 1));
				Set_Mapper(mapper,1);
				mapper->Release_Ref();
			}
			break;

		case W3DVERTMAT_STAGE1_MAPPING_ROTATE:
			{
				RotateTextureMapperClass *mapper =
					NEW_REF(RotateTextureMapperClass,(mapping1_arg_ini, "Args", 1));
				Set_Mapper(mapper,1);
				mapper->Release_Ref();
			}
			break;

		case W3DVERTMAT_STAGE1_MAPPING_SINE_LINEAR_OFFSET:
			{
				SineLinearOffsetTextureMapperClass *mapper =
					NEW_REF(SineLinearOffsetTextureMapperClass,(mapping1_arg_ini, "Args", 1));
				Set_Mapper(mapper,1);
				mapper->Release_Ref();
			}
			break;

		case W3DVERTMAT_STAGE1_MAPPING_STEP_LINEAR_OFFSET:
			{
				StepLinearOffsetTextureMapperClass *mapper =
					NEW_REF(StepLinearOffsetTextureMapperClass,(mapping1_arg_ini, "Args", 1));
				Set_Mapper(mapper,1);
				mapper->Release_Ref();
			}
			break;

		case W3DVERTMAT_STAGE1_MAPPING_ZIGZAG_LINEAR_OFFSET:
			{
				ZigZagLinearOffsetTextureMapperClass *mapper =
					NEW_REF(ZigZagLinearOffsetTextureMapperClass,(mapping1_arg_ini, "Args", 1));
				Set_Mapper(mapper,1);
				mapper->Release_Ref();
			}
			break;

		case W3DVERTMAT_STAGE1_MAPPING_WS_CLASSIC_ENV:
			{
				WSClassicEnvironmentMapperClass *mapper = NEW_REF(WSClassicEnvironmentMapperClass,(1));
				Set_Mapper(mapper,1);
				mapper->Release_Ref();
			}
			break;

		case W3DVERTMAT_STAGE1_MAPPING_WS_ENVIRONMENT:
			{
				WSEnvironmentMapperClass *mapper = NEW_REF(WSEnvironmentMapperClass,(1));
				Set_Mapper(mapper,1);
				mapper->Release_Ref();
			}
			break;

		case W3DVERTMAT_STAGE1_MAPPING_GRID_CLASSIC_ENV:
			{
				GridClassicEnvironmentMapperClass *mapper =
					NEW_REF(GridClassicEnvironmentMapperClass,(mapping1_arg_ini, "Args", 1));
				Set_Mapper(mapper,1);
				mapper->Release_Ref();
			}
			break;

		case W3DVERTMAT_STAGE1_MAPPING_GRID_ENVIRONMENT:
			{
				GridEnvironmentMapperClass *mapper =
					NEW_REF(GridEnvironmentMapperClass,(mapping1_arg_ini, "Args", 1));
				Set_Mapper(mapper,1);
				mapper->Release_Ref();
			}
			break;

		case W3DVERTMAT_STAGE1_MAPPING_RANDOM:
			{
				RandomTextureMapperClass *mapper =
					NEW_REF(RandomTextureMapperClass,(mapping1_arg_ini, "Args", 1));
				Set_Mapper(mapper,1);
				mapper->Release_Ref();
			}
			break;

		case W3DVERTMAT_STAGE1_MAPPING_EDGE:
			{
				EdgeMapperClass *mapper =
					NEW_REF(EdgeMapperClass,(mapping1_arg_ini, "Args", 1));
				Set_Mapper(mapper,1);
				mapper->Release_Ref();
			}
			break;

		case W3DVERTMAT_STAGE0_MAPPING_BUMPENV:
			{
				BumpEnvTextureMapperClass *mapper =
					NEW_REF(BumpEnvTextureMapperClass,(mapping1_arg_ini, "Args", 0));
				Set_Mapper(mapper,1);
				mapper->Release_Ref();
			}
			break;

		default:
			WWDEBUG_SAY(("Unsupported mapper in %s",name));
			break;
	}

	Vector3 tmp;
	W3dUtilityClass::Convert_Color(vmat.Ambient,&tmp);
	Set_Ambient(tmp);

	W3dUtilityClass::Convert_Color(vmat.Diffuse,&tmp);
	Set_Diffuse(tmp);

	W3dUtilityClass::Convert_Color(vmat.Specular,&tmp);
	Set_Specular(tmp);

	W3dUtilityClass::Convert_Color(vmat.Emissive,&tmp);
	Set_Emissive(tmp);

	Set_Shininess(vmat.Shininess);
	Set_Opacity(vmat.Opacity);

	return WW3D_ERROR_OK;
}


WW3DErrorType VertexMaterialClass::Save_W3D(ChunkSaveClass & csave)
{
	WWASSERT(0);
	return WW3D_ERROR_OK;
}

void VertexMaterialClass::Apply(void) const
{
	int i;

	DX8Wrapper::Set_DX8_Material(Material);

	if (WW3D::Is_Coloring_Enabled())
		DX8Wrapper::Set_DX8_Render_State(D3DRS_LIGHTING,FALSE);
	else
		DX8Wrapper::Set_DX8_Render_State(D3DRS_LIGHTING,UseLighting);
	DX8Wrapper::Set_DX8_Render_State(D3DRS_AMBIENTMATERIALSOURCE,AmbientColorSource);
	DX8Wrapper::Set_DX8_Render_State(D3DRS_DIFFUSEMATERIALSOURCE,DiffuseColorSource);
	DX8Wrapper::Set_DX8_Render_State(D3DRS_EMISSIVEMATERIALSOURCE,EmissiveColorSource);

	// set to default values if no mappers
	for (i=0; i<MeshBuilderClass::MAX_STAGES; i++) {
		if (Mapper[i]) {
			Mapper[i]->Apply(UVSource[i]);
		} else {
			DX8Wrapper::Set_DX8_Texture_Stage_State(i,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_PASSTHRU | UVSource[i]);
			DX8Wrapper::Set_DX8_Texture_Stage_State(i,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_DISABLE);
		}
	}
}

void VertexMaterialClass::Apply_Null(void)
{
	int i;
	static D3DMATERIAL8 default_settings =
	{
		{ 1.0f, 1.0f, 1.0f, 1.0f },	// diffuse
		{ 1.0f, 1.0f, 1.0f, 1.0f },	// ambient
		{ 0.0f, 0.0f, 0.0f, 0.0f },	// specular
		{ 0.0f, 0.0f, 0.0f, 0.0f },	// emissive
		1.0f									// power
	};

	DX8Wrapper::Set_DX8_Render_State(D3DRS_LIGHTING,FALSE);
	DX8Wrapper::Set_DX8_Material(&default_settings);

	DX8Wrapper::Set_DX8_Render_State(D3DRS_AMBIENTMATERIALSOURCE,D3DMCS_MATERIAL);
	DX8Wrapper::Set_DX8_Render_State(D3DRS_DIFFUSEMATERIALSOURCE,D3DMCS_MATERIAL);
	DX8Wrapper::Set_DX8_Render_State(D3DRS_EMISSIVEMATERIALSOURCE,D3DMCS_MATERIAL);

	// set to default values if no mappers
	for (i=0; i<MeshBuilderClass::MAX_STAGES; i++) {
		DX8Wrapper::Set_DX8_Texture_Stage_State(i,D3DTSS_TEXCOORDINDEX,D3DTSS_TCI_PASSTHRU | i);
		DX8Wrapper::Set_DX8_Texture_Stage_State(i,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_DISABLE);
	}
}


/***********************************************************************************************
 * Init -- init code                                                                           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/14/2001  hy : Created.                                                                  *
 *=============================================================================================*/
void VertexMaterialClass::Init()
{
	int i;
	for (i=0; i<PRESET_COUNT;i++)
		Presets[i]=NEW_REF(VertexMaterialClass,());

	// Set up presets
	Presets[PRELIT_DIFFUSE]->Set_Diffuse_Color_Source(VertexMaterialClass::COLOR1);
	Presets[PRELIT_DIFFUSE]->Set_Lighting(false);
	Presets[PRELIT_NODIFFUSE]->Set_Lighting(false);
}


/***********************************************************************************************
 * Shutdown -- shutdown code                                                                   *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/14/2001  hy : Created.                                                                  *
 *=============================================================================================*/
void VertexMaterialClass::Shutdown()
{
	int i;
	for (i=0; i<PRESET_COUNT;i++)
		REF_PTR_RELEASE(Presets[i]);
}


/***********************************************************************************************
 * Get_Preset -- retrieve presets                                                              *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/14/2001  hy : Created.                                                                  *
 *=============================================================================================*/
VertexMaterialClass * VertexMaterialClass::Get_Preset(PresetType type)
{
	WWASSERT(type<PRESET_COUNT);
	Presets[type]->Add_Ref();
	return Presets[type];
}
