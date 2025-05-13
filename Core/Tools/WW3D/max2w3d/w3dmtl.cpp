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
 *                 Project Name : Max2W3d                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/w3dmtl.cpp                     $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 8/22/01 7:47a                                               $*
 *                                                                                             *
 *                    $Revision:: 32                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "w3dmtl.h"
#include <max.h>
#include <stdmat.h>
#include "gamemtl.h"
#include "realcrc.h"

static W3dRGBStruct Color_To_W3d(Color & c)
{
	W3dRGBStruct wc;
	wc.R = (c.r * 255.0f);
	wc.G = (c.g * 255.0f);
	wc.B = (c.b * 255.0f);
	wc.pad = 0;
	return wc;
}

/*


	Implementation of W3dMapClass



*/

W3dMapClass::W3dMapClass(const W3dMapClass & that)
{
	Set_Filename(that.Filename);
	if (that.AnimInfo) {
		Set_Anim_Info(that.AnimInfo);
	}
}
	
W3dMapClass & W3dMapClass::operator = (const W3dMapClass & that)
{
	if (this != &that) {
		Set_Filename(that.Filename);
		Set_Anim_Info(that.AnimInfo);
	}
	return *this;
}

W3dMapClass::~W3dMapClass(void)
{
	if (Filename) free(Filename);
	if (AnimInfo) delete AnimInfo;
}

void W3dMapClass::Reset(void)
{
	if (Filename) free(Filename);
	if (AnimInfo) delete AnimInfo;
	Filename = NULL;
	AnimInfo = NULL;
}

void W3dMapClass::Set_Filename(const char * fullpath) 
{ 
	if (Filename) {
		free(Filename); 
	}
	if (fullpath) {
		char name[_MAX_FNAME];
		char exten[_MAX_EXT];
		char fname[_MAX_FNAME+_MAX_EXT+2];

		_splitpath(fullpath,NULL,NULL,name,exten);
		_makepath(fname,NULL,NULL,name,exten);
		//strupr(fname);						(gth) need to preserve case since unix/PS2 is case sensitive...
		Filename = strdup(fname); 
	} else {
		Filename = NULL;
	}
}

void W3dMapClass::Set_Anim_Info(const W3dTextureInfoStruct * info) 
{ 
	if (info == NULL) {
		if (AnimInfo) {
			delete AnimInfo;
			AnimInfo = NULL;
			return;
		}
	} else {
		if (AnimInfo == NULL) {
			AnimInfo = new W3dTextureInfoStruct;
		}
		*AnimInfo = *info;
	}
}

void W3dMapClass::Set_Anim_Info(int framecount,float framerate)
{
	if (AnimInfo == NULL) {
		AnimInfo = new W3dTextureInfoStruct;
	}

	AnimInfo->FrameCount = framecount;
	AnimInfo->FrameRate = framerate;
}



/*
 

	Implementation of W3dMaterialClass


*/


W3dMaterialClass::W3dMaterialClass(void)
{
	PassCount = 0;
	SortLevel = SORT_LEVEL_NONE;
	for (int pass = 0; pass < MAX_PASSES; pass++) {
		Materials[pass] = NULL;
		W3d_Shader_Reset(&(Shaders[pass]));
		for (int stage = 0; stage < MAX_STAGES; stage++) {
			Textures[pass][stage] = NULL;
			MapChannel[pass][stage] = 1;
			MapperArgs[pass][stage] = NULL;
		}
	}
}

W3dMaterialClass::~W3dMaterialClass(void)
{
	Free();
}

void W3dMaterialClass::Free(void)
{
	for (int pass = 0; pass < MAX_PASSES; pass++) {
		
		if (Materials[pass]) {
			delete Materials[pass];
			Materials[pass] = NULL;
		}

		for (int stage = 0; stage < MAX_STAGES; stage++) {
			if (Textures[pass][stage]) {
				delete Textures[pass][stage];
				Textures[pass][stage] = NULL;
			}

			if (MapperArgs[pass][stage]) {
				delete [] MapperArgs[pass][stage];
				MapperArgs[pass][stage] = NULL;
			}
		}
	}
}

void W3dMaterialClass::Reset(void)
{
	Free();
	SortLevel = SORT_LEVEL_NONE;
	for (int pass=0; pass < MAX_PASSES; pass++) {
		W3d_Shader_Reset(&(Shaders[pass]));
	
		for (int stage=0; stage < MAX_STAGES; stage++) {
			MapChannel[pass][stage] = 1;
		}
	}
}

void W3dMaterialClass::Set_Surface_Type(unsigned int type)
{
	SurfaceType = type;
}

void W3dMaterialClass::Set_Sort_Level(int level)
{
	assert(level <= MAX_SORT_LEVEL);
	SortLevel = level;
}

void W3dMaterialClass::Set_Pass_Count(int count)
{
	assert(count >= 0);
	assert(count < MAX_PASSES);
	PassCount = count;
}

void W3dMaterialClass::Set_Vertex_Material(const W3dVertexMaterialStruct & vmat,int pass)
{
	assert(pass >= 0);
	assert(pass < PassCount);

	if (Materials[pass] == NULL) {
		Materials[pass] = new W3dVertexMaterialStruct;
	}
	*(Materials[pass]) = vmat;
}

void W3dMaterialClass::Set_Mapper_Args(const char *args_buffer, int pass, int stage)
{
	assert(pass >= 0);
	assert(pass < PassCount);
	assert(stage >= 0);
	assert(stage < MAX_STAGES);

	if (MapperArgs[pass][stage] != NULL) {
		delete [] MapperArgs[pass][stage];
		MapperArgs[pass][stage] = NULL;
	}
	if (args_buffer) {
		int len = strlen(args_buffer);
		MapperArgs[pass][stage] = new char [len + 1];
		strcpy(MapperArgs[pass][stage], args_buffer);
	}
}

void W3dMaterialClass::Set_Shader(const W3dShaderStruct & shader,int pass)
{
	assert(pass >= 0);
	assert(pass < PassCount);

	Shaders[pass] = shader;	
}

void W3dMaterialClass::Set_Texture(const W3dMapClass & map,int pass,int stage)
{
	assert(pass >= 0);
	assert(pass < PassCount);

	if (Textures[pass][stage] == NULL) {
		Textures[pass][stage] = new W3dMapClass;
	}
	*(Textures[pass][stage]) = map;
}

void W3dMaterialClass::Set_Map_Channel(int pass,int stage,int channel)
{
	assert(pass >= 0);
	assert(pass < PassCount);
	MapChannel[pass][stage] = channel;	
}

unsigned int W3dMaterialClass::Get_Surface_Type(void) const
{
	return SurfaceType;
}

int W3dMaterialClass::Get_Sort_Level(void) const
{
	return SortLevel;
}

int W3dMaterialClass::Get_Pass_Count(void) const
{
	return PassCount;
}

W3dVertexMaterialStruct * W3dMaterialClass::Get_Vertex_Material(int pass ) const
{
	assert(pass >= 0);
	assert(pass < PassCount);
	
	return Materials[pass];
}

const char * W3dMaterialClass::Get_Mapper_Args(int pass, int stage) const
{
	assert(pass >= 0);
	assert(pass < PassCount);
	assert(stage >= 0);
	assert(stage < MAX_STAGES);
	
	return MapperArgs[pass][stage];
}

W3dShaderStruct W3dMaterialClass::Get_Shader(int pass) const
{
	assert(pass >= 0);
	assert(pass < PassCount);
	return Shaders[pass];	
}

W3dMapClass * W3dMaterialClass::Get_Texture(int pass,int stage) const
{
	assert(pass >= 0);
	assert(pass < PassCount);
	assert(stage >= 0);
	assert(stage < MAX_STAGES);
	
	return Textures[pass][stage];
}

int W3dMaterialClass::Get_Map_Channel(int pass,int stage) const
{
	assert(pass >= 0);
	assert(pass < PassCount);
	assert(stage >= 0);
	assert(stage < MAX_STAGES);

	return MapChannel[pass][stage];
}

void W3dMaterialClass::Init(Mtl * mtl, char *materialColorTexture)
{
	bool ps2_mat = false;
	Reset();

	if (mtl->ClassID() == PS2GameMaterialClassID)
	{
		ps2_mat = true;
	}

	if ((mtl->ClassID() == GameMaterialClassID) || ps2_mat) {
		Init((GameMtl*)mtl, materialColorTexture);	///@todo: Fix this for substituted textures.
		return;
	}

	Texmap * tmap;
	PassCount = 1;

	tmap = mtl->GetSubTexmap(ID_RL);
	if (tmap && tmap->ClassID() == Class_ID(BMTEX_CLASS_ID,0)) {
		PassCount++;
	}

	W3dVertexMaterialStruct mat;
	W3dShaderStruct shader;
	W3dMapClass tex;

	/*
	** Setting up the diffuse color pass
	*/
	W3d_Shader_Reset(&shader);

	mat.Attributes = 0;
	mat.Emissive.R = mat.Emissive.G = mat.Emissive.B = 0; //(uint8)(255 .0f * mtl->GetSelfIllum());
	
	Color diffuse =			mtl->GetDiffuse();
	mat.Diffuse.R =			(uint8)(diffuse.r * 255.0f);
	mat.Diffuse.G =			(uint8)(diffuse.g * 255.0f);		
	mat.Diffuse.B =			(uint8)(diffuse.b * 255.0f);
	mat.Ambient =				mat.Diffuse;

	Color specular =			mtl->GetSpecular();
	mat.Specular.R =			(uint8)(specular.r * 255.0f);
	mat.Specular.G =			(uint8)(specular.g * 255.0f);		
	mat.Specular.B =			(uint8)(specular.b * 255.0f);

	mat.Shininess =			mtl->GetShininess();
	mat.Opacity =				1.0f - mtl->GetXParency();
	mat.Translucency =		0.0f;

	tmap = mtl->GetSubTexmap(ID_DI);
	if (tmap && tmap->ClassID() == Class_ID(BMTEX_CLASS_ID,0)) {

		char * tname = ((BitmapTex *)tmap)->GetMapName();
		if (tname) {
			tex.Set_Filename(tname);
			mat.Diffuse.R = mat.Diffuse.G = mat.Diffuse.B = 255;
			W3d_Shader_Set_Texturing(&shader,W3DSHADER_TEXTURING_ENABLE);
			Set_Texture(tex,0,0);
		}
	}

	if (materialColorTexture && !mtl->GetSubTexmap(ID_DI) && !mtl->GetSubTexmap(ID_RL))
	{	//no textures on material, substitute textures to improve rendering speed.
		tex.Set_Filename(materialColorTexture);	///@todo: Fix this to procedural name/path
		W3d_Shader_Set_Texturing(&shader,W3DSHADER_TEXTURING_ENABLE);
		//This texture will hold solid pixels of material color, don't need any filtering.
//		W3dTextureInfoStruct texinfo;
//		memset(&texinfo,0,sizeof(texinfo));			
//		texinfo.Attributes = texinfo.Attributes | /*W3DTEXTURE_NO_LOD|*/W3DTEXTURE_CLAMP_U | W3DTEXTURE_CLAMP_V;
//		tex.Set_Anim_Info(&texinfo);	
		Set_Texture(tex,0,0);
	}

	if (mat.Opacity != 1.0f) {
		W3d_Shader_Set_Dest_Blend_Func(&shader,W3DSHADER_DESTBLENDFUNC_ONE_MINUS_SRC_ALPHA);
		W3d_Shader_Set_Src_Blend_Func(&shader,W3DSHADER_SRCBLENDFUNC_SRC_ALPHA);
	}
	
	Set_Vertex_Material(mat,0);
	Set_Shader(shader,0);


	/*
	** Setting up the reflection pass (envmap)
	*/
	if (PassCount == 2) {

		W3d_Shader_Reset(&shader);
		if (ps2_mat)
		{
			W3d_Shader_Set_Pri_Gradient(&shader,PSS_PRIGRADIENT_MODULATE);
		} 
		else
		{
			W3d_Shader_Set_Pri_Gradient(&shader,W3DSHADER_PRIGRADIENT_MODULATE);
		}
		W3d_Shader_Set_Sec_Gradient(&shader,W3DSHADER_SECGRADIENT_DISABLE);
		W3d_Shader_Set_Depth_Mask(&shader,W3DSHADER_DEPTHMASK_WRITE_DISABLE);
		W3d_Shader_Set_Dest_Blend_Func(&shader,W3DSHADER_DESTBLENDFUNC_ONE);
		W3d_Shader_Set_Src_Blend_Func(&shader,W3DSHADER_SRCBLENDFUNC_ONE);
		W3d_Shader_Set_Texturing(&shader,W3DSHADER_TEXTURING_ENABLE);

		// PS2 specific paramaters.
		W3d_Shader_Set_PS2_Param_A(&shader, PSS_SRC);
		W3d_Shader_Set_PS2_Param_B(&shader, PSS_ZERO);
		W3d_Shader_Set_PS2_Param_B(&shader, PSS_ONE);
		W3d_Shader_Set_PS2_Param_B(&shader, PSS_DEST);

		W3d_Vertex_Material_Reset(&mat);
		mat.Diffuse.R = mat.Diffuse.G = mat.Diffuse.B = 128;
		mat.Attributes &= W3DVERTMAT_STAGE0_MAPPING_MASK;
		mat.Attributes |= W3DVERTMAT_STAGE0_MAPPING_ENVIRONMENT;

		tmap = mtl->GetSubTexmap(ID_RL);
		if (tmap && tmap->ClassID() == Class_ID(BMTEX_CLASS_ID,0)) {

			char * tname = ((BitmapTex *)tmap)->GetMapName();
			if (tname) {
				tex.Set_Filename(tname);
				Set_Texture(tex,1);
			}
		}

		Set_Vertex_Material(mat,1);
		Set_Shader(shader,1);
	}
}

void W3dMaterialClass::Init(GameMtl * gamemtl, char *materialColorTexture)
{
	Reset();
	PassCount = gamemtl->Get_Pass_Count();
	Set_Surface_Type (gamemtl->Get_Surface_Type ());
	Set_Sort_Level(gamemtl->Get_Sort_Level());

	for (int pass=0;pass<gamemtl->Get_Pass_Count(); pass++) {

		/*
		** set up the shader
		*/
		W3dShaderStruct shader;
		W3d_Shader_Reset(&shader);
		shader.DepthCompare = gamemtl->Get_Depth_Compare(pass);
		shader.DepthMask = gamemtl->Get_Depth_Mask(pass);
		shader.AlphaTest = gamemtl->Get_Alpha_Test(pass);
		shader.DestBlend = gamemtl->Get_Dest_Blend(pass);
		shader.PriGradient = gamemtl->Get_Pri_Gradient(pass);
		shader.SecGradient = gamemtl->Get_Sec_Gradient(pass);
		shader.SrcBlend = gamemtl->Get_Src_Blend(pass);
		shader.DetailColorFunc = gamemtl->Get_Detail_Color_Func(pass);
		shader.DetailAlphaFunc = gamemtl->Get_Detail_Alpha_Func(pass);
		shader.Texturing = W3DSHADER_TEXTURING_DISABLE;
		shader.PostDetailColorFunc = gamemtl->Get_Detail_Color_Func(pass);	// (gth) set up the post-detail options
		shader.PostDetailAlphaFunc = gamemtl->Get_Detail_Alpha_Func(pass);

		// PS2 specific paramaters.
		W3d_Shader_Set_PS2_Param_A(&shader, gamemtl->Get_PS2_Shader_Param_A(pass));
		W3d_Shader_Set_PS2_Param_B(&shader, gamemtl->Get_PS2_Shader_Param_B(pass));
		W3d_Shader_Set_PS2_Param_C(&shader, gamemtl->Get_PS2_Shader_Param_C(pass));
		W3d_Shader_Set_PS2_Param_D(&shader, gamemtl->Get_PS2_Shader_Param_D(pass));

		/*
		** set up the vertex material
		*/
		W3dVertexMaterialStruct mat;
		mat.Attributes = 0;
		
		if (gamemtl->Get_Copy_Specular_To_Diffuse(pass)) {
			mat.Attributes |= W3DVERTMAT_COPY_SPECULAR_TO_DIFFUSE;
		}

		// mapping type for stage 0		
		switch(gamemtl->Get_Mapping_Type(pass, 0))
		{
			case GAMEMTL_MAPPING_UV:					mat.Attributes |= W3DVERTMAT_STAGE0_MAPPING_UV;						break;
			case GAMEMTL_MAPPING_ENV:					mat.Attributes |= W3DVERTMAT_STAGE0_MAPPING_ENVIRONMENT;			break;
			case GAMEMTL_MAPPING_CHEAP_ENV:			mat.Attributes |= W3DVERTMAT_STAGE0_MAPPING_CHEAP_ENVIRONMENT;	break;
			case GAMEMTL_MAPPING_SCREEN:				mat.Attributes |= W3DVERTMAT_STAGE0_MAPPING_SCREEN;				break;
			case GAMEMTL_MAPPING_LINEAR_OFFSET:		mat.Attributes |= W3DVERTMAT_STAGE0_MAPPING_LINEAR_OFFSET;		break;
			case GAMEMTL_MAPPING_SILHOUETTE:			mat.Attributes |= W3DVERTMAT_STAGE0_MAPPING_SILHOUETTE;			break;

			case	GAMEMTL_MAPPING_SCALE:	mat.Attributes |= W3DVERTMAT_STAGE0_MAPPING_SCALE;			break;
			case	GAMEMTL_MAPPING_GRID:	mat.Attributes |= W3DVERTMAT_STAGE0_MAPPING_GRID;			break;
			case	GAMEMTL_MAPPING_ROTATE:	mat.Attributes |= W3DVERTMAT_STAGE0_MAPPING_ROTATE;			break;
			case	GAMEMTL_MAPPING_SINE_LINEAR_OFFSET:	mat.Attributes |= W3DVERTMAT_STAGE0_MAPPING_SINE_LINEAR_OFFSET;			break;
			case	GAMEMTL_MAPPING_STEP_LINEAR_OFFSET:	mat.Attributes |= W3DVERTMAT_STAGE0_MAPPING_STEP_LINEAR_OFFSET;			break;
			case	GAMEMTL_MAPPING_ZIGZAG_LINEAR_OFFSET:	mat.Attributes |= W3DVERTMAT_STAGE0_MAPPING_ZIGZAG_LINEAR_OFFSET;			break;
			case	GAMEMTL_MAPPING_WS_CLASSIC_ENV:	mat.Attributes |= W3DVERTMAT_STAGE0_MAPPING_WS_CLASSIC_ENV;			break;
			case	GAMEMTL_MAPPING_WS_ENVIRONMENT:	mat.Attributes |= W3DVERTMAT_STAGE0_MAPPING_WS_ENVIRONMENT;			break;
			case	GAMEMTL_MAPPING_GRID_CLASSIC_ENV:	mat.Attributes |= W3DVERTMAT_STAGE0_MAPPING_GRID_CLASSIC_ENV;			break;
			case	GAMEMTL_MAPPING_GRID_ENVIRONMENT:	mat.Attributes |= 	W3DVERTMAT_STAGE0_MAPPING_GRID_ENVIRONMENT;			break;
			case	GAMEMTL_MAPPING_RANDOM:	mat.Attributes |= W3DVERTMAT_STAGE0_MAPPING_RANDOM;			break;
			case	GAMEMTL_MAPPING_EDGE:	mat.Attributes |= W3DVERTMAT_STAGE0_MAPPING_EDGE;			break;
			case	GAMEMTL_MAPPING_BUMPENV:	mat.Attributes |= W3DVERTMAT_STAGE0_MAPPING_BUMPENV;		break;
		};

		// mapping type for stage 1
		switch(gamemtl->Get_Mapping_Type(pass, 1))
		{
			case GAMEMTL_MAPPING_UV:					mat.Attributes |= W3DVERTMAT_STAGE1_MAPPING_UV;						break;
			case GAMEMTL_MAPPING_ENV:					mat.Attributes |= W3DVERTMAT_STAGE1_MAPPING_ENVIRONMENT;			break;
			case GAMEMTL_MAPPING_CHEAP_ENV:			mat.Attributes |= W3DVERTMAT_STAGE1_MAPPING_CHEAP_ENVIRONMENT;	break;
			case GAMEMTL_MAPPING_SCREEN:				mat.Attributes |= W3DVERTMAT_STAGE1_MAPPING_SCREEN;				break;
			case GAMEMTL_MAPPING_LINEAR_OFFSET:		mat.Attributes |= W3DVERTMAT_STAGE1_MAPPING_LINEAR_OFFSET;		break;
			case GAMEMTL_MAPPING_SILHOUETTE:			mat.Attributes |= W3DVERTMAT_STAGE1_MAPPING_SILHOUETTE;			break;

			case	GAMEMTL_MAPPING_SCALE:	mat.Attributes |= W3DVERTMAT_STAGE1_MAPPING_SCALE;			break;
			case	GAMEMTL_MAPPING_GRID:	mat.Attributes |= W3DVERTMAT_STAGE1_MAPPING_GRID;			break;
			case	GAMEMTL_MAPPING_ROTATE:	mat.Attributes |= W3DVERTMAT_STAGE1_MAPPING_ROTATE;			break;
			case	GAMEMTL_MAPPING_SINE_LINEAR_OFFSET:	mat.Attributes |= W3DVERTMAT_STAGE1_MAPPING_SINE_LINEAR_OFFSET;			break;
			case	GAMEMTL_MAPPING_STEP_LINEAR_OFFSET:	mat.Attributes |= W3DVERTMAT_STAGE1_MAPPING_STEP_LINEAR_OFFSET;			break;
			case	GAMEMTL_MAPPING_ZIGZAG_LINEAR_OFFSET:	mat.Attributes |= W3DVERTMAT_STAGE1_MAPPING_ZIGZAG_LINEAR_OFFSET;			break;
			case	GAMEMTL_MAPPING_WS_CLASSIC_ENV:	mat.Attributes |= W3DVERTMAT_STAGE1_MAPPING_WS_CLASSIC_ENV;			break;
			case	GAMEMTL_MAPPING_WS_ENVIRONMENT:	mat.Attributes |= W3DVERTMAT_STAGE1_MAPPING_WS_ENVIRONMENT;			break;
			case	GAMEMTL_MAPPING_GRID_CLASSIC_ENV:	mat.Attributes |= W3DVERTMAT_STAGE1_MAPPING_GRID_CLASSIC_ENV;			break;
			case	GAMEMTL_MAPPING_GRID_ENVIRONMENT:	mat.Attributes |= 	W3DVERTMAT_STAGE1_MAPPING_GRID_ENVIRONMENT;			break;
			case	GAMEMTL_MAPPING_RANDOM:	mat.Attributes |= W3DVERTMAT_STAGE1_MAPPING_RANDOM;			break;
			case	GAMEMTL_MAPPING_EDGE:	mat.Attributes |= W3DVERTMAT_STAGE1_MAPPING_EDGE;			break;
			case	GAMEMTL_MAPPING_BUMPENV:	mat.Attributes |= W3DVERTMAT_STAGE1_MAPPING_BUMPENV;		break;
		};

		switch(gamemtl->Get_PSX_Translucency(pass))
		{
			case GAMEMTL_PSX_TRANS_NONE:		mat.Attributes |= W3DVERTMAT_PSX_TRANS_NONE;					break;
			case GAMEMTL_PSX_TRANS_100:		mat.Attributes |= W3DVERTMAT_PSX_TRANS_100;					break;
			case GAMEMTL_PSX_TRANS_50:			mat.Attributes |= W3DVERTMAT_PSX_TRANS_50;					break;
			case GAMEMTL_PSX_TRANS_25:			mat.Attributes |= W3DVERTMAT_PSX_TRANS_25;					break;
			case GAMEMTL_PSX_TRANS_MINUS_100:mat.Attributes |= W3DVERTMAT_PSX_TRANS_MINUS_100;			break;
		};

		if (!gamemtl->Get_PSX_Lighting(pass)) {
			mat.Attributes |= W3DVERTMAT_PSX_NO_RT_LIGHTING;
		}

		mat.Ambient = Color_To_W3d(gamemtl->Get_Ambient(pass,0));
		mat.Diffuse = Color_To_W3d(gamemtl->Get_Diffuse(pass,0));
		mat.Specular = Color_To_W3d(gamemtl->Get_Specular(pass,0));
		mat.Emissive = Color_To_W3d(gamemtl->Get_Emissive(pass,0));
		mat.Shininess = gamemtl->Get_Shininess(pass,0);
		mat.Opacity = gamemtl->Get_Opacity(pass,0);
		mat.Translucency = gamemtl->Get_Translucency(pass,0);

		/*
		** install the two textures if present
		*/
		W3dMapClass w3dmap;
		BitmapTex * tex = NULL;

		for (int stage=0; stage < MAX_STAGES; stage++) {
				
			if (gamemtl->Get_Texture_Enable(pass,stage) && gamemtl->Get_Texture(pass,stage)) {
				
				w3dmap.Reset();
								
				// get the filename for the w3dmap texture
				tex = (BitmapTex *)gamemtl->Get_Texture(pass,stage);
				assert(tex->GetMapName());
				w3dmap.Set_Filename(tex->GetMapName());
				
				// get the animation and flags for the w3dmap texture 
				W3dTextureInfoStruct texinfo;
				memset(&texinfo,0,sizeof(texinfo));			
				
				texinfo.AnimType = gamemtl->Get_Texture_Anim_Type(pass,stage);	

				if (gamemtl->Get_Texture_Publish(pass,stage)) {
					texinfo.Attributes = texinfo.Attributes | W3DTEXTURE_PUBLISH;
				}
				if (gamemtl->Get_Texture_No_LOD(pass,stage)) {
					texinfo.Attributes = texinfo.Attributes | W3DTEXTURE_NO_LOD;
				}
				if (gamemtl->Get_Texture_Clamp_U(pass,stage)) {
					texinfo.Attributes = texinfo.Attributes | W3DTEXTURE_CLAMP_U;
				}
				if (gamemtl->Get_Texture_Clamp_V(pass,stage)) {
					texinfo.Attributes = texinfo.Attributes | W3DTEXTURE_CLAMP_V;
				}
				if (gamemtl->Get_Texture_Alpha_Bitmap(pass,stage)) {
					texinfo.Attributes = texinfo.Attributes | W3DTEXTURE_ALPHA_BITMAP;
				}
				texinfo.Attributes = texinfo.Attributes | (
					(gamemtl->Get_Texture_Hint(pass,stage) << W3DTEXTURE_HINT_SHIFT)
					& W3DTEXTURE_HINT_MASK);

				// If the shader indicates bump-environment mapping mark this texture as a bumpmap.
				if ((stage == 0) && (gamemtl->Get_Pri_Gradient (pass) == W3DSHADER_PRIGRADIENT_BUMPENVMAP)) {
					texinfo.Attributes |= W3DTEXTURE_TYPE_BUMPMAP;
				}

				texinfo.FrameCount = gamemtl->Get_Texture_Frame_Count(pass,stage);
				texinfo.FrameRate = gamemtl->Get_Texture_Frame_Rate(pass,stage);

				if ((texinfo.FrameCount > 1) || (texinfo.Attributes != 0)) {
					w3dmap.Set_Anim_Info(&texinfo);	
				}
				
				// plug it in and turn on texturing in the shader
				Set_Texture(w3dmap,pass,stage);
				shader.Texturing = W3DSHADER_TEXTURING_ENABLE;
							
				// copy over the mapping channel
				Set_Map_Channel(pass,stage,gamemtl->Get_Map_Channel(pass,stage));
	
				// copy over the mapper args
				Set_Mapper_Args(gamemtl->Get_Mapping_Arg_Buffer(pass, stage), pass, stage);

			} else {
						if (materialColorTexture)
						{	//no textures on material, substitute textures to improve rendering speed.
							w3dmap.Reset();
							w3dmap.Set_Filename(materialColorTexture);
							
							// plug it in and turn on texturing in the shader
							Set_Texture(w3dmap,pass,stage);
							shader.Texturing = W3DSHADER_TEXTURING_ENABLE;
						}
						break;	// break out of the loop (and dont put in stage 1 if stage 0 is missing...)
			}
		}

		Set_Shader(shader,pass);
		Set_Vertex_Material(mat,pass);
	}
}


bool W3dMaterialClass::Is_Multi_Pass_Transparent(void) const
{
	return ((PassCount >= 2) && (Get_Shader(0).DestBlend != W3DSHADER_DESTBLENDFUNC_ZERO));
}


/*
 

	Implementation of W3dMaterialDescClass::VertClass


*/
W3dMaterialDescClass::VertMatClass::VertMatClass(void) : 
	PassIndex(-1), 
	Crc(0),
	Name(NULL)
{
	for (int stage=0; stage < W3dMaterialClass::MAX_STAGES; ++stage) {
		MapperArgs[stage] = NULL;
	}
}

W3dMaterialDescClass::VertMatClass::~VertMatClass(void) 
{ 
	if (Name) free(Name);

	for (int stage=0; stage < W3dMaterialClass::MAX_STAGES; ++stage) {
		if (MapperArgs[stage]) {
			delete [] (MapperArgs[stage]); 
			MapperArgs[stage] = NULL;
		}
	}
}

W3dMaterialDescClass::VertMatClass & 
W3dMaterialDescClass::VertMatClass::operator = (const VertMatClass & that)
{
	if (this != &that) {
		Material = that.Material;
		PassIndex = that.PassIndex;
		Crc = that.Crc;
		Set_Name(that.Name);
		for (int stage=0; stage < W3dMaterialClass::MAX_STAGES; stage++) {
			Set_Mapper_Args(that.MapperArgs[stage], stage);
		}
	}
	return *this;
}

bool W3dMaterialDescClass::VertMatClass::operator != (const VertMatClass & that)	
{ 
	return !(*this == that); 
}

bool W3dMaterialDescClass::VertMatClass::operator == (const VertMatClass & that) 
{ 
	assert(0); return false; 
}

void W3dMaterialDescClass::VertMatClass::Set_Name(const char * name) 
{ 
	if (Name) free(Name); 
	
	if (name) { 
		Name = strdup(name); 
	} else { 
		Name = NULL; 
	} 
}

void W3dMaterialDescClass::VertMatClass::Set_Mapper_Args(const char * args, int stage) 
{ 
	if (MapperArgs[stage]) {
		delete [] (MapperArgs[stage]); 
		MapperArgs[stage] = NULL;
	}
	
	if (args) {
		int len = strlen(args);
		MapperArgs[stage] = new char [len + 1];
		strcpy(MapperArgs[stage], args);
	} else { 
		MapperArgs[stage] = NULL; 
	} 
}


/*
 

	Implementation of W3dMaterialDescClass


*/
W3dMaterialDescClass::MaterialRemapClass::MaterialRemapClass(void)
{
	PassCount = -1;
	for (int pass=0; pass<W3dMaterialClass::MAX_PASSES; pass++) {
		VertexMaterialIdx[pass] = -1;
		ShaderIdx[pass] = -1;
		for (int stage=0; stage<W3dMaterialClass::MAX_STAGES; stage++) {
			TextureIdx[pass][stage] = -1;
		}
	}
}

bool W3dMaterialDescClass::MaterialRemapClass::operator != (const MaterialRemapClass & that)
{
	return !(*this == that);
}

bool W3dMaterialDescClass::MaterialRemapClass::operator == (const MaterialRemapClass & that)
{
	for (int pass=0; pass<W3dMaterialClass::MAX_PASSES; pass++) {
		
		if (VertexMaterialIdx[pass] != that.VertexMaterialIdx[pass]) return false;
		if (ShaderIdx[pass] != that.ShaderIdx[pass]) return false;

		for (int stage=0; stage<W3dMaterialClass::MAX_STAGES; stage++) {
		
			if (TextureIdx[pass][stage] != that.TextureIdx[pass][stage]) return false;
		}
	}
	return true;
}



W3dMaterialDescClass::W3dMaterialDescClass(void)
{
	Reset();
}

W3dMaterialDescClass::~W3dMaterialDescClass(void)
{
}

void W3dMaterialDescClass::Reset(void)
{	
	PassCount = -1;
	SortLevel = -1;
	MaterialRemaps.Clear();
	Shaders.Clear();
	VertexMaterials.Clear();
	Textures.Clear();
}


W3dMaterialDescClass::ErrorType W3dMaterialDescClass::Add_Material(const W3dMaterialClass & mat,const char * name)
{
	/*
	** If passes hasn't been set yet, set it.
	*/
	if (PassCount == -1) {
		PassCount = mat.Get_Pass_Count();
	}

	/*
	** Same for sort level.
	*/
	if (SortLevel == -1) {
		SortLevel = mat.Get_Sort_Level();
	}

	/*
	** Verify that this material uses the same number of passes that any
	** other materials we have use.
	*/
	if (mat.Get_Pass_Count() != PassCount) {
		return INCONSISTENT_PASSES;
	}

	if (mat.Is_Multi_Pass_Transparent()) {
		char msg[100];
		sprintf(msg,"Enable Multipass-Transparency?");
		HWND window=GetForegroundWindow();
		if (IDYES!=MessageBox(window,msg,"Confirmation",MB_YESNO|MB_ICONINFORMATION|MB_TASKMODAL))
			return MULTIPASS_TRANSPARENT;
	}

	/*
	** Verify that this material uses the same sort level as all other
	** materials used on this mesh.
	*/
	if (mat.Get_Sort_Level() != SortLevel) {
		return INCONSISTENT_SORT_LEVEL;
	}
	
	/*
	** Ok, lets re-index this material and store the unique parts
	*/
	MaterialRemapClass remap;

	for (int pass=0; pass<PassCount; pass++) {

		remap.VertexMaterialIdx[pass] = Add_Vertex_Material(
			mat.Get_Vertex_Material(pass),mat.Get_Mapper_Args(pass, 0),mat.Get_Mapper_Args(pass, 1),pass,name);

		remap.ShaderIdx[pass] = Add_Shader(mat.Get_Shader(pass),pass);
		
		for (int stage=0; stage<W3dMaterialClass::MAX_STAGES; stage++) {

			remap.TextureIdx[pass][stage] = Add_Texture(mat.Get_Texture(pass,stage),pass,stage);
			remap.MapChannel[pass][stage] = mat.Get_Map_Channel(pass,stage);
		
		}
	}

	MaterialRemaps.Add(remap);
	return OK;
}


int W3dMaterialDescClass::Material_Count(void)
{
	return MaterialRemaps.Count();
}

int W3dMaterialDescClass::Pass_Count(void)
{
	return PassCount;
}

int W3dMaterialDescClass::Vertex_Material_Count(void)
{
	return VertexMaterials.Count();
}

int W3dMaterialDescClass::Shader_Count(void)
{
	return Shaders.Count();
}

int W3dMaterialDescClass::Texture_Count(void)
{
	return Textures.Count();
}

int W3dMaterialDescClass::Get_Sort_Level(void)
{
	return SortLevel;
}

W3dVertexMaterialStruct * W3dMaterialDescClass::Get_Vertex_Material(int vmat_index)
{
	assert(vmat_index >= 0);
	assert(vmat_index < VertexMaterials.Count());
	return &(VertexMaterials[vmat_index].Material);
}

const char * W3dMaterialDescClass::Get_Mapper_Args(int vmat_index, int stage)
{
	assert(vmat_index >= 0);
	assert(vmat_index < VertexMaterials.Count());
	assert(stage >= 0);
	assert(stage < W3dMaterialClass::MAX_STAGES);
	return VertexMaterials[vmat_index].MapperArgs[stage];
}

W3dShaderStruct * W3dMaterialDescClass::Get_Shader(int shader_index)
{
	assert(shader_index >= 0);
	assert(shader_index < Shaders.Count());
	return &(Shaders[shader_index].Shader);
}

W3dMapClass * W3dMaterialDescClass::Get_Texture(int texture_index)
{
	assert(texture_index >= 0);
	assert(texture_index < Textures.Count());
	return &(Textures[texture_index].Map);
}

int W3dMaterialDescClass::Get_Vertex_Material_Index(int mat_index,int pass)
{
	assert(mat_index >= 0);
	assert(mat_index < MaterialRemaps.Count());
	assert(pass >= 0);
	assert(pass < PassCount);
	return MaterialRemaps[mat_index].VertexMaterialIdx[pass];
}

int W3dMaterialDescClass::Get_Shader_Index(int mat_index,int pass)
{
	assert(mat_index >= 0);
	assert(mat_index < MaterialRemaps.Count());
	assert(pass >= 0);
	assert(pass < PassCount);
	return MaterialRemaps[mat_index].ShaderIdx[pass];
}

int W3dMaterialDescClass::Get_Texture_Index(int mat_index,int pass,int stage)
{
	assert(mat_index >= 0);
	assert(mat_index < MaterialRemaps.Count());
	assert(pass >= 0);
	assert(pass < PassCount);
	assert(stage >= 0);
	assert(stage < W3dMaterialClass::MAX_STAGES);
	return MaterialRemaps[mat_index].TextureIdx[pass][stage];
}

W3dVertexMaterialStruct * W3dMaterialDescClass::Get_Vertex_Material(int mat_index,int pass)
{
	int index = Get_Vertex_Material_Index(mat_index,pass);
	if (index == -1) {
		return NULL;
	} else {
		return Get_Vertex_Material(index);
	}
}

const char * W3dMaterialDescClass::Get_Mapper_Args(int mat_index,int pass,int stage)
{
	int index = Get_Vertex_Material_Index(mat_index,pass);
	if (index == -1) {
		return NULL;
	} else {
		return Get_Mapper_Args(index,stage);
	}
}

W3dShaderStruct * W3dMaterialDescClass::Get_Shader(int mat_index,int pass)
{
	int index = Get_Shader_Index(mat_index,pass);
	if (index == -1) {
		return NULL;
	} else {
		return Get_Shader(index);
	}
}

W3dMapClass * W3dMaterialDescClass::Get_Texture(int mat_index,int pass,int stage)
{
	int index = Get_Texture_Index(mat_index,pass,stage);
	if (index == -1) {
		return NULL;
	} else {
		return Get_Texture(index);
	}
}

int W3dMaterialDescClass::Get_Map_Channel(int mat_index,int pass,int stage)
{
	return MaterialRemaps[mat_index].MapChannel[pass][stage];
}

const char * W3dMaterialDescClass::Get_Vertex_Material_Name(int mat_index,int pass)
{
	int index = Get_Vertex_Material_Index(mat_index,pass);
	if (index == -1) {
		return NULL;
	} else {
		return VertexMaterials[index].Name;
	}
}

const char * W3dMaterialDescClass::Get_Vertex_Material_Name(int vmat_index)
{
	return VertexMaterials[vmat_index].Name;
}

bool W3dMaterialDescClass::Stage_Needs_Texture_Coordinates(int pass,int stage)
{
	for (int mi=0; mi<MaterialRemaps.Count();mi++) {
		W3dShaderStruct * shader = Get_Shader(mi,pass);
		
		if (shader) {
			if (stage == 0) {
				if (W3d_Shader_Get_Texturing(shader) == W3DSHADER_TEXTURING_ENABLE) return true;
			}

			if (stage == 1) {
				if ((W3d_Shader_Get_Detail_Color_Func(shader) != W3DSHADER_DETAILCOLORFUNC_DISABLE) ||
					(W3d_Shader_Get_Detail_Alpha_Func(shader) != W3DSHADER_DETAILALPHAFUNC_DISABLE)) {
					return true;
				}
			}
		}
	}
	return false;
}

bool W3dMaterialDescClass::Pass_Uses_Vertex_Alpha(int pass)
{
	/*
	** Only the last alpha pass gets vertex alpha
	*/
	int max_alpha_pass = -1;
	for (int pi=0; pi<Pass_Count(); pi++) {
		if (Pass_Uses_Alpha(pi)) {
			max_alpha_pass = pi;
		}
	}
	return (max_alpha_pass == pass);
}


bool W3dMaterialDescClass::Pass_Uses_Alpha(int pass) 
{
	for (int mi=0; mi<MaterialRemaps.Count(); mi++) {

		W3dShaderStruct * shader = Get_Shader(mi,pass);
		
		int dst_blend = W3d_Shader_Get_Dest_Blend_Func(shader);
		int src_blend = W3d_Shader_Get_Src_Blend_Func(shader);
		int alpha_test = W3d_Shader_Get_Alpha_Test(shader);

		if (	(dst_blend == W3DSHADER_DESTBLENDFUNC_SRC_ALPHA) ||
				(dst_blend == W3DSHADER_DESTBLENDFUNC_ONE_MINUS_SRC_ALPHA) ||
				(src_blend == W3DSHADER_SRCBLENDFUNC_SRC_ALPHA) ||
				(src_blend == W3DSHADER_SRCBLENDFUNC_ONE_MINUS_SRC_ALPHA) ||
				(alpha_test != 0)  )
		{
			return true;
		}
	}
	return false;
}

int W3dMaterialDescClass::Add_Vertex_Material(W3dVertexMaterialStruct * vmat,
		const char *mapper_args0,const char *mapper_args1,int pass,const char * name)
{
	if (vmat == NULL) {
		return -1;
	}

	int crc = Compute_Crc(*vmat, mapper_args0, mapper_args1);
	for (int vi=0; vi<VertexMaterials.Count(); vi++) {
		if ((crc == VertexMaterials[vi].Crc) && (pass == VertexMaterials[vi].PassIndex)) {
			break;
		}
	}

	if (vi == VertexMaterials.Count()) {
		VertMatClass vm;
		vm.Material = *vmat;
		vm.Crc = crc;
		vm.PassIndex = pass;
		vm.Set_Name(name);
		vm.Set_Mapper_Args(mapper_args0, 0);
		vm.Set_Mapper_Args(mapper_args1, 1);
		VertexMaterials.Add(vm);
	}
	
	return vi;
}

int W3dMaterialDescClass::Add_Shader(const W3dShaderStruct & shader,int pass)
{
	int crc = Compute_Crc(shader);
	for (int si=0; si<Shaders.Count(); si++) {
		if (crc == Shaders[si].Crc) break;
	}
	
	if (si == Shaders.Count()) {
		ShadeClass s;
		s.Shader = shader;
		s.Crc = crc;
		Shaders.Add(s);
	} 
	
	return si;
}

int W3dMaterialDescClass::Add_Texture(W3dMapClass * map,int pass,int stage)
{
	if ((map == NULL) || (map->Filename == NULL)) {
		return -1;
	}

	int crc = Compute_Crc(*map);
	for (int ti=0; ti<Textures.Count(); ti++) {
		if (crc == Textures[ti].Crc) {
			break;
		}
	}

	if (ti == Textures.Count()) {
		TexClass tex;
		tex.Map = *map;
		tex.Crc = crc;
		Textures.Add(tex);
	}

	return ti;
}

unsigned long W3dMaterialDescClass::Compute_Crc(const W3dVertexMaterialStruct & vmat,
																const char *mapper_args0,
																const char *mapper_args1)
{
	unsigned long crc = 0;
	crc = CRC_Memory((const unsigned char *)&vmat.Attributes,sizeof(vmat.Attributes),crc);
	crc = CRC_Memory((const unsigned char *)&vmat.Ambient,sizeof(vmat.Ambient),crc);
	crc = CRC_Memory((const unsigned char *)&vmat.Diffuse,sizeof(vmat.Diffuse),crc);
	crc = CRC_Memory((const unsigned char *)&vmat.Specular,sizeof(vmat.Specular),crc);
	crc = CRC_Memory((const unsigned char *)&vmat.Emissive,sizeof(vmat.Emissive),crc);
	crc = CRC_Memory((const unsigned char *)&vmat.Shininess,sizeof(vmat.Shininess),crc);
	crc = CRC_Memory((const unsigned char *)&vmat.Opacity,sizeof(vmat.Opacity),crc);
	crc = CRC_Memory((const unsigned char *)&vmat.Translucency,sizeof(vmat.Translucency),crc);

	// Add mapper args string to crc. We are stripping out spaces, tabs, and
	// leading/trailing newlines before computing the CRC so two strings will
	// only differ if they have visible differences.
	crc = Add_String_To_Crc(mapper_args0, crc);
	crc = Add_String_To_Crc(mapper_args1, crc);

	return crc;
}

unsigned long W3dMaterialDescClass::Compute_Crc(const W3dShaderStruct & shader)
{
	unsigned long crc = 0;
	crc = CRC_Memory((const unsigned char *)&shader,sizeof(shader),crc);
	return crc;
}

unsigned long W3dMaterialDescClass::Compute_Crc(const W3dMapClass & map)
{
	unsigned long crc = 0;
	if (map.AnimInfo != NULL) {
		crc = CRC_Memory((const unsigned char *)&map.AnimInfo->Attributes,sizeof(map.AnimInfo->Attributes),crc);
		crc = CRC_Memory((const unsigned char *)&map.AnimInfo->AnimType,sizeof(map.AnimInfo->AnimType),crc);
		crc = CRC_Memory((const unsigned char *)&map.AnimInfo->FrameCount,sizeof(map.AnimInfo->FrameCount),crc);
		crc = CRC_Memory((const unsigned char *)&map.AnimInfo->FrameRate,sizeof(map.AnimInfo->FrameRate),crc);
	}
	crc = CRC_Stringi(map.Filename, crc);
	return crc;
}

unsigned long W3dMaterialDescClass::Add_String_To_Crc(const char *str, unsigned long in_crc)
{
	unsigned long out_crc = in_crc;
	if (str) {
		int len = strlen(str);
		char *temp = new char[len + 1];
		const char *p_in = str;

		// skip leading spaces, tabs, newlines
		for (; *p_in == ' ' || *p_in == '\t' || *p_in == '\r' || *p_in == '\n'; p_in++);

		// copy string, skipping spaces and tabs
		char * p_out = temp;
		int count = 0;
		for (; *p_in; p_in++) {
			for (; *p_in == ' ' || *p_in == '\t'; p_in++);
			if (!(*p_in)) break;
			*p_out = *p_in;
			p_out++;
			count++;
		}
		*p_out = 0;

		// Erase any trailing newlines:
		if (count) {
			// p_out now points to the ending null - make it point to the
			// character before it (the last character of the string proper)
			p_out--;

			for (; *p_out == '\r' || *p_out == '\n'; p_out--) {
				*p_out = '\000';
				count--;
			}
		}

		out_crc = CRC_Memory((const unsigned char *)temp,count,in_crc);
		delete [] temp;
	}
	return out_crc;
}


