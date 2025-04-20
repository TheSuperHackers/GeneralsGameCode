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
 *                     $Archive:: /Commando/Code/ww3d2/texture.cpp                            $*
 *                                                                                             *
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             *
 *                     $Modtime:: 8/29/01 7:06p                                               $*
 *                                                                                             *
 *                    $Revision:: 65                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   FileListTextureClass::Load_Frame_Surface -- Load source texture                           * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "texture.h"

#include <d3d8.h>
#include <stdio.h>
#include <d3dx8core.h>
#include "dx8wrapper.h"
#include "TARGA.H"
#include <nstrdup.h>
#include "w3d_file.h"
#include "assetmgr.h"
#include "formconv.h"
#include "textureloader.h"
#include "missingtexture.h"
#include "ffactory.h"
#include "dx8caps.h"
#include "dx8texman.h"
#include "meshmatdesc.h"

/*
** Definitions of static members:
*/

static unsigned unused_texture_id;

// ----------------------------------------------------------------------------

static int Calculate_Texture_Memory_Usage(const TextureClass* texture,int red_factor=0)
{
	// Set performance statistics

	int size=0;
	IDirect3DTexture8* d3d_texture=const_cast<TextureClass*>(texture)->Peek_D3D_Texture();
	if (!d3d_texture) return 0;
	for (unsigned i=red_factor;i<d3d_texture->GetLevelCount();++i) {
		D3DSURFACE_DESC desc;
		DX8_ErrorCode(d3d_texture->GetLevelDesc(i,&desc));
		size+=desc.Size;
	}
	return size;
}

/*************************************************************************
**                             TextureClass
*************************************************************************/

TextureClass::TextureClass(unsigned width, unsigned height, WW3DFormat format, MipCountType mip_level_count, PoolType pool,bool rendertarget)
	:
	D3DTexture(NULL),
	texture_id(unused_texture_id++),
	Initialized(true),
	Filter(mip_level_count),
	MipLevelCount(mip_level_count),
	Pool(pool),
	Dirty(false),
	IsLightmap(false),
	IsProcedural(true),
	Name(""),
	TextureFormat(format),
	IsCompressionAllowed(false),
	TextureLoadTask(NULL),
	Width(width),
	Height(height)
{
	switch (format) 
	{
	case WW3D_FORMAT_DXT1:
	case WW3D_FORMAT_DXT2:
	case WW3D_FORMAT_DXT3:
	case WW3D_FORMAT_DXT4:
	case WW3D_FORMAT_DXT5:
		IsCompressionAllowed=true;
		break;
	default : break;
	}
		
	D3DPOOL d3dpool=(D3DPOOL)0;
	switch(pool)
	{
	case POOL_DEFAULT		: d3dpool=D3DPOOL_DEFAULT; break;
	case POOL_MANAGED		: d3dpool=D3DPOOL_MANAGED; break;
	case POOL_SYSTEMMEM	: d3dpool=D3DPOOL_SYSTEMMEM; break;
	default: WWASSERT(0);
	}
	D3DTexture = DX8Wrapper::_Create_DX8_Texture(width, height, format, mip_level_count,d3dpool,rendertarget);
	if (pool==POOL_DEFAULT)
	{
		Dirty=true;
		DX8TextureTrackerClass *track=W3DNEW DX8TextureTrackerClass
		(
			width, 
			height, 
			format, 
			mip_level_count,
			rendertarget,
			this
		);
		DX8TextureManagerClass::Add(track);
	}
	LastAccessed=WW3D::Get_Sync_Time();
}

// ----------------------------------------------------------------------------

TextureClass::TextureClass
(
	const char *name,
	const char *full_path,
	MipCountType mip_level_count,
	WW3DFormat texture_format,
	bool allow_compression)
	:
	D3DTexture(NULL),
	texture_id(unused_texture_id++),
	Initialized(false),
	Filter(mip_level_count),
	MipLevelCount(mip_level_count),
	Pool(POOL_MANAGED),
	Dirty(false),
	IsLightmap(false),
	IsProcedural(false),
	TextureFormat(texture_format),
	IsCompressionAllowed(allow_compression),
	TextureLoadTask(NULL),
	Width(0),
	Height(0)
{
	switch (TextureFormat) 
	{
	case WW3D_FORMAT_DXT1:
	case WW3D_FORMAT_DXT2:
	case WW3D_FORMAT_DXT3:
	case WW3D_FORMAT_DXT4:
	case WW3D_FORMAT_DXT5:
		IsCompressionAllowed=true;
		break;
	case WW3D_FORMAT_U8V8:		// Bumpmap
	case WW3D_FORMAT_L6V5U5:	// Bumpmap
	case WW3D_FORMAT_X8L8V8U8:	// Bumpmap
		// If requesting bumpmap format that isn't available we'll just return the surface in whatever color
		// format the texture file is in. (This is illegal case, the format support should always be queried
		// before creating a bump texture!)
		if (!DX8Wrapper::Get_Current_Caps()->Support_Texture_Format(TextureFormat)) 
		{
			TextureFormat=WW3D_FORMAT_UNKNOWN;
		}
		// If bump format is valid, make sure compression is not allowed so that we don't even attempt to load
		// from a compressed file (quality isn't good enough for bump map). Also disable mipmapping.
		else 
		{
			IsCompressionAllowed=false;
			MipLevelCount=MIP_LEVELS_1;
			Filter.Set_Mip_Mapping(TextureFilterClass::FILTER_TYPE_NONE);
		}
		break;
	default:	break;
	}

	WWASSERT_PRINT(name && name[0], "TextureClass CTor: NULL or empty texture name\n");
	int len=strlen(name);
	for (int i=0;i<len;++i) 
	{
		if (name[i]=='+') 
		{
			IsLightmap=true;

			// Set bilinear filtering for lightmaps (they are very stretched and
			// low detail so we don't care for anisotropic or trilinear filtering...)
			Filter.Set_Min_Filter(TextureFilterClass::FILTER_TYPE_FAST);
			Filter.Set_Mag_Filter(TextureFilterClass::FILTER_TYPE_FAST);
			if (mip_level_count!=MIP_LEVELS_1) Filter.Set_Mip_Mapping(TextureFilterClass::FILTER_TYPE_FAST);
			break;
		}
	}
	Set_Texture_Name(name);
	Set_Full_Path(full_path);
	WWASSERT(name[0]!='\0');
	if (!WW3D::Is_Texturing_Enabled()) 
	{
		Initialized=true;
		D3DTexture=0;
	}
	else if (WW3D::Get_Thumbnail_Enabled()==false || mip_level_count==MIP_LEVELS_1)
	{
		Initialized=true;
		D3DTexture=0;
		TextureLoader::Request_High_Priority_Loading(this,mip_level_count);
	}
	else {
		Load_Locked_Surface();
		TextureFormat=texture_format;	// Locked surface may be in a wrong format
	}
	LastAccessed=WW3D::Get_Sync_Time();
}

// ----------------------------------------------------------------------------

TextureClass::TextureClass(SurfaceClass *surface, MipCountType mip_level_count)
	:
	D3DTexture(NULL),
	texture_id(unused_texture_id++),
	Initialized(true),
	Filter(mip_level_count),
	MipLevelCount(mip_level_count),
	Pool(POOL_MANAGED),
	Dirty(false),
	IsLightmap(false),
	Name(""),
	IsProcedural(true),
	TextureFormat(surface->Get_Surface_Format()),
	IsCompressionAllowed(false),
	TextureLoadTask(NULL),
	Width(0),
	Height(0)
{
	SurfaceClass::SurfaceDescription sd;
	surface->Get_Description(sd);
	Width=sd.Width;
	Height=sd.Height;
	switch (sd.Format) 
	{
	case WW3D_FORMAT_DXT1:
	case WW3D_FORMAT_DXT2:
	case WW3D_FORMAT_DXT3:
	case WW3D_FORMAT_DXT4:
	case WW3D_FORMAT_DXT5:
		IsCompressionAllowed=true;
		break;
	default: break;
	}
	
	D3DTexture = DX8Wrapper::_Create_DX8_Texture(surface->Peek_D3D_Surface(), mip_level_count);
	LastAccessed=WW3D::Get_Sync_Time();
}

// ----------------------------------------------------------------------------

TextureClass::TextureClass(IDirect3DTexture8* d3d_texture)
	:
	D3DTexture(d3d_texture),
	texture_id(unused_texture_id++),
	Initialized(true),
	Filter((MipCountType)d3d_texture->GetLevelCount()),
	MipLevelCount((MipCountType)d3d_texture->GetLevelCount()),
	Pool(POOL_MANAGED),
	Dirty(false),
	IsLightmap(false),
	Name(""),
	IsProcedural(true),
	IsCompressionAllowed(false),
	TextureLoadTask(NULL),
	Width(0),
	Height(0)
{
	D3DTexture->AddRef();
	IDirect3DSurface8* surface;
	DX8_ErrorCode(Peek_D3D_Texture()->GetSurfaceLevel(0,&surface));
	D3DSURFACE_DESC d3d_desc;
	::ZeroMemory(&d3d_desc, sizeof(D3DSURFACE_DESC));
	DX8_ErrorCode(surface->GetDesc(&d3d_desc));
	Width=d3d_desc.Width;
	Height=d3d_desc.Height;
	TextureFormat=D3DFormat_To_WW3DFormat(d3d_desc.Format);
	switch (TextureFormat) 
	{
	case WW3D_FORMAT_DXT1:
	case WW3D_FORMAT_DXT2:
	case WW3D_FORMAT_DXT3:
	case WW3D_FORMAT_DXT4:
	case WW3D_FORMAT_DXT5:
		IsCompressionAllowed=true;
		break;
	default: break;
	}

	LastAccessed=WW3D::Get_Sync_Time();
}

// ----------------------------------------------------------------------------

TextureClass::~TextureClass(void)
{
	TextureLoadTaskClass::Release_Instance(TextureLoadTask);
	TextureLoadTask=NULL;

	if (!Initialized) {
		WWDEBUG_SAY(("Warning: Texture %s was loaded but never used\n",Get_Texture_Name()));
	}

	if (D3DTexture) {
		D3DTexture->Release();
		D3DTexture = NULL;
	}
	DX8TextureManagerClass::Remove(this);
}

// ----------------------------------------------------------------------------

void TextureClass::Init()
{
	// If the texture has already been initialised we should exit now
	if (Initialized) return;

	TextureLoader::Add_Load_Task(this);
	LastAccessed=WW3D::Get_Sync_Time();
}

void TextureClass::Invalidate()
{
	// Don't invalidate procedural textures
	if (IsProcedural) return;

	// Don't invalidate missing texture
	if (Is_Missing_Texture()) return;

	if (D3DTexture) {
		D3DTexture->Release();
		D3DTexture = NULL;
	}

	if (!WW3D::Is_Texturing_Enabled()) {
		Initialized=true;
		D3DTexture=0;
	}
	else if (WW3D::Get_Thumbnail_Enabled()==false || MipLevelCount==MIP_LEVELS_1)
	{
		Initialized=true;
		D3DTexture=0;
		TextureLoader::Request_High_Priority_Loading(this,MipLevelCount);
	}
	else {
		Initialized=false;
		Load_Locked_Surface();
	}
}

//**********************************************************************************************
//! Returns a pointer to the d3d texture
/*! 
*/
IDirect3DBaseTexture8 * TextureClass::Peek_D3D_Base_Texture() const 
{ 	
	LastAccessed=WW3D::Get_Sync_Time(); 
	return D3DTexture; 
}

//**********************************************************************************************
//! Set the d3d texture pointer.  Handles ref counts properly.
/*! 
*/
void TextureClass::Set_D3D_Base_Texture(IDirect3DBaseTexture8* tex) 
{ 
	// (gth) Generals does stuff directly with the D3DTexture pointer so lets
	// reset the access timer whenever someon messes with this pointer.
	LastAccessed=WW3D::Get_Sync_Time();
	
	if (D3DTexture != NULL) {
		D3DTexture->Release();
	}
	D3DTexture = tex;
	if (D3DTexture != NULL) {
		D3DTexture->AddRef();
	}
}

// ----------------------------------------------------------------------------

void TextureClass::Load_Locked_Surface()
{
	if (D3DTexture) D3DTexture->Release();
	D3DTexture=0;
	TextureLoader::Request_Thumbnail(this);
	Initialized=false;
}

// ----------------------------------------------------------------------------

bool TextureClass::Is_Missing_Texture()
{
	bool flag = false;
	IDirect3DTexture8 *missing_texture = MissingTexture::_Get_Missing_Texture();
	
	if(D3DTexture == missing_texture)
		flag = true;

	if(missing_texture)
		missing_texture->Release();

	return flag;
}

// ----------------------------------------------------------------------------

void TextureClass::Set_Texture_Name(const char * name)
{
	Name=name;
}

// ----------------------------------------------------------------------------

unsigned int TextureClass::Get_Mip_Level_Count(void)
{
	if (!D3DTexture) {
		WWASSERT_PRINT(0, "Get_Mip_Level_Count: D3DTexture is NULL!\n");
		return 0;
	}

	return D3DTexture->GetLevelCount();
}

// ----------------------------------------------------------------------------

void TextureClass::Get_Level_Description(SurfaceClass::SurfaceDescription &surface_desc, unsigned int level)
{
	if (!D3DTexture) {
		WWASSERT_PRINT(0, "Get_Surface_Description: D3DTexture is NULL!\n");
	}

	D3DSURFACE_DESC d3d_surf_desc;
	DX8_ErrorCode(Peek_D3D_Texture()->GetLevelDesc(level, &d3d_surf_desc));
	surface_desc.Format = D3DFormat_To_WW3DFormat(d3d_surf_desc.Format);
	surface_desc.Height = d3d_surf_desc.Height; 
	surface_desc.Width = d3d_surf_desc.Width;
}

// ----------------------------------------------------------------------------

SurfaceClass *TextureClass::Get_Surface_Level(unsigned int level)
{
	IDirect3DSurface8 *d3d_surface = NULL;
	DX8_ErrorCode(Peek_D3D_Texture()->GetSurfaceLevel(level, &d3d_surface));
	SurfaceClass *surface = W3DNEW SurfaceClass(d3d_surface);
	d3d_surface->Release();
	return surface;
}

// ----------------------------------------------------------------------------

unsigned int TextureClass::Get_Priority(void)
{
	if (!D3DTexture) {
		WWASSERT_PRINT(0, "Get_Priority: D3DTexture is NULL!\n");
		return 0;
	}

	return D3DTexture->GetPriority();
}

// ----------------------------------------------------------------------------

unsigned int TextureClass::Set_Priority(unsigned int priority)
{
	if (!D3DTexture) {
		WWASSERT_PRINT(0, "Set_Priority: D3DTexture is NULL!\n");
		return 0;
	}

	return D3DTexture->SetPriority(priority);
}

unsigned TextureClass::Get_Reduction() const
{
	if (MipLevelCount==MIP_LEVELS_1) return 0;

	int reduction=WW3D::Get_Texture_Reduction();
	if (MipLevelCount && reduction>MipLevelCount) {
		reduction=MipLevelCount;
	}
	return reduction;
}

// ----------------------------------------------------------------------------

void TextureClass::Apply(unsigned int stage)
{
	if (!Initialized) 
	{
		Init();
	}
	LastAccessed=WW3D::Get_Sync_Time();

	DX8_RECORD_TEXTURE(this);

	// Set texture itself
	if (WW3D::Is_Texturing_Enabled()) 
	{
		DX8Wrapper::Set_DX8_Texture(stage, D3DTexture);
	}
	else 
	{
		DX8Wrapper::Set_DX8_Texture(stage, NULL);
	}

	Filter.Apply(stage);
}

// ----------------------------------------------------------------------------

void TextureClass::Apply_Null(unsigned int stage)
{
	// This function sets the render states for a "NULL" texture
	DX8Wrapper::Set_DX8_Texture(stage, NULL);
}

// ----------------------------------------------------------------------------

void TextureClass::Apply_New_Surface(bool initialized)
{
	if (D3DTexture) D3DTexture->Release();
	D3DTexture=TextureLoadTask->Peek_D3D_Texture();
	D3DTexture->AddRef();
	if (initialized) Initialized=true;

	WWASSERT(D3DTexture);
	IDirect3DSurface8* surface;
	DX8_ErrorCode(Peek_D3D_Texture()->GetSurfaceLevel(0,&surface));
	D3DSURFACE_DESC d3d_desc;
	::ZeroMemory(&d3d_desc, sizeof(D3DSURFACE_DESC));
	DX8_ErrorCode(surface->GetDesc(&d3d_desc));
//	if (TextureFormat==WW3D_FORMAT_UNKNOWN) {
		TextureFormat=D3DFormat_To_WW3DFormat(d3d_desc.Format);
		Width=d3d_desc.Width;
		Height=d3d_desc.Height;
//	}
//	else {
//		WWASSERT(D3DFormat_To_WW3DFormat(d3d_desc.Format)==TextureFormat);
//	}
	surface->Release();
}

// ----------------------------------------------------------------------------

unsigned TextureClass::Get_Texture_Memory_Usage() const
{
	if (/*!ReductionEnabled || */!Initialized) return Calculate_Texture_Memory_Usage(this,0);
//	unsigned reduction=WW3D::Get_Texture_Reduction();
//	if (CurrentReductionFactor>reduction) reduction=CurrentReductionFactor;
	return Calculate_Texture_Memory_Usage(this,0);//reduction);
}

// ----------------------------------------------------------------------------

int TextureClass::_Get_Total_Locked_Surface_Size()
{
	int total_locked_surface_size=0;

	HashTemplateIterator<StringClass,TextureClass*> ite(WW3DAssetManager::Get_Instance()->Texture_Hash());
	// Loop through all the textures in the manager
	for (ite.First ();!ite.Is_Done();ite.Next ()) {

		// Get the current texture
		TextureClass* tex=ite.Peek_Value();
		if (!tex->Initialized) {
//			total_locked_surface_size+=tex->Get_Non_Reduced_Texture_Memory_Usage();
			total_locked_surface_size+=tex->Get_Texture_Memory_Usage();
		}
	}
	return total_locked_surface_size;
}

// ----------------------------------------------------------------------------

int TextureClass::_Get_Total_Texture_Size()
{
	int total_texture_size=0;

	HashTemplateIterator<StringClass,TextureClass*> ite(WW3DAssetManager::Get_Instance()->Texture_Hash());
	// Loop through all the textures in the manager
	for (ite.First ();!ite.Is_Done();ite.Next ()) {
		// Get the current texture
		TextureClass* tex=ite.Peek_Value();
		total_texture_size+=tex->Get_Texture_Memory_Usage();
	}
	return total_texture_size;
}

// ----------------------------------------------------------------------------

int TextureClass::_Get_Total_Lightmap_Texture_Size()
{
	int total_texture_size=0;

	HashTemplateIterator<StringClass,TextureClass*> ite(WW3DAssetManager::Get_Instance()->Texture_Hash());
	// Loop through all the textures in the manager
	for (ite.First ();!ite.Is_Done();ite.Next ()) {
		// Get the current texture
		TextureClass* tex=ite.Peek_Value();
		if (tex->Is_Lightmap()) {
			total_texture_size+=tex->Get_Texture_Memory_Usage();
		}
	}
	return total_texture_size;
}

// ----------------------------------------------------------------------------

int TextureClass::_Get_Total_Procedural_Texture_Size()
{
	int total_texture_size=0;

	HashTemplateIterator<StringClass,TextureClass*> ite(WW3DAssetManager::Get_Instance()->Texture_Hash());
	// Loop through all the textures in the manager
	for (ite.First ();!ite.Is_Done();ite.Next ()) {
		// Get the current texture
		TextureClass* tex=ite.Peek_Value();
		if (tex->Is_Procedural()) {
			total_texture_size+=tex->Get_Texture_Memory_Usage();
		}
	}
	return total_texture_size;
}

// ----------------------------------------------------------------------------

int TextureClass::_Get_Total_Texture_Count()
{
	int texture_count=0;

	HashTemplateIterator<StringClass,TextureClass*> ite(WW3DAssetManager::Get_Instance()->Texture_Hash());
	// Loop through all the textures in the manager
	for (ite.First ();!ite.Is_Done();ite.Next ()) {
		texture_count++;
	}

	return texture_count;
}

// ----------------------------------------------------------------------------

int TextureClass::_Get_Total_Lightmap_Texture_Count()
{
	int texture_count=0;

	HashTemplateIterator<StringClass,TextureClass*> ite(WW3DAssetManager::Get_Instance()->Texture_Hash());
	// Loop through all the textures in the manager
	for (ite.First ();!ite.Is_Done();ite.Next ()) {
		if (ite.Peek_Value()->Is_Lightmap()) {
			texture_count++;
		}
	}

	return texture_count;
}

// ----------------------------------------------------------------------------

int TextureClass::_Get_Total_Procedural_Texture_Count()
{
	int texture_count=0;

	HashTemplateIterator<StringClass,TextureClass*> ite(WW3DAssetManager::Get_Instance()->Texture_Hash());
	// Loop through all the textures in the manager
	for (ite.First ();!ite.Is_Done();ite.Next ()) {
		if (ite.Peek_Value()->Is_Procedural()) {
			texture_count++;
		}
	}

	return texture_count;
}

// ----------------------------------------------------------------------------

int TextureClass::_Get_Total_Locked_Surface_Count()
{
	int texture_count=0;

	HashTemplateIterator<StringClass,TextureClass*> ite(WW3DAssetManager::Get_Instance()->Texture_Hash());
	// Loop through all the textures in the manager
	for (ite.First ();!ite.Is_Done();ite.Next ()) {
		// Get the current texture
		TextureClass* tex=ite.Peek_Value();
		if (!tex->Initialized) {
			texture_count++;
		}
	}

	return texture_count;
}
/*
bool Validate_Filters(unsigned type)
{
	ShaderClass shader=ShaderClass::_PresetOpaqueShader;
	shader.Apply();
	DX8Wrapper::Set_DX8_Texture(0, MissingTexture::_Get_Missing_Texture());
	DX8Wrapper::Set_DX8_Texture_Stage_State(0,D3DTSS_MINFILTER,_MinTextureFilters[type]);
	DX8Wrapper::Set_DX8_Texture_Stage_State(0,D3DTSS_MAGFILTER,_MagTextureFilters[type]);
	DX8Wrapper::Set_DX8_Texture_Stage_State(0,D3DTSS_MIPFILTER,_MipMapFilters[type]);
	unsigned long passes;
	HRESULT hres=DX8Wrapper::_Get_D3D_Device8()->ValidateDevice(&passes);
	return !FAILED(hres);
}
*/


// Utility functions
TextureClass* Load_Texture(ChunkLoadClass & cload)
{
	// Assume failure
	TextureClass *newtex = NULL;

	char name[256];
	if (cload.Open_Chunk () && (cload.Cur_Chunk_ID () == W3D_CHUNK_TEXTURE)) 
	{

		W3dTextureInfoStruct texinfo;
		bool hastexinfo = false;

		/*
		** Read in the texture filename, and a possible texture info structure.
		*/
		while (cload.Open_Chunk()) {
			switch (cload.Cur_Chunk_ID()) {
				case W3D_CHUNK_TEXTURE_NAME:
					cload.Read(&name,cload.Cur_Chunk_Length());
					break;

				case W3D_CHUNK_TEXTURE_INFO:
					cload.Read(&texinfo,sizeof(W3dTextureInfoStruct));
					hastexinfo = true;
					break;
			};
			cload.Close_Chunk();
		}
		cload.Close_Chunk();

		/*
		** Get the texture from the asset manager
		*/
		if (hastexinfo) 
		{

			MipCountType mipcount;

			bool no_lod = ((texinfo.Attributes & W3DTEXTURE_NO_LOD) == W3DTEXTURE_NO_LOD);

			if (no_lod) 
			{
				mipcount = MIP_LEVELS_1;
			} 
			else 
			{
				switch (texinfo.Attributes & W3DTEXTURE_MIP_LEVELS_MASK) {

					case W3DTEXTURE_MIP_LEVELS_ALL:
						mipcount = MIP_LEVELS_ALL;
						break;

					case W3DTEXTURE_MIP_LEVELS_2:
						mipcount = MIP_LEVELS_2;
						break;

					case W3DTEXTURE_MIP_LEVELS_3:
						mipcount = MIP_LEVELS_3;
						break;

					case W3DTEXTURE_MIP_LEVELS_4:
						mipcount = MIP_LEVELS_4;
						break;

					default:
						WWASSERT (false);
						mipcount = MIP_LEVELS_ALL;
						break;
				}
			}
			newtex = WW3DAssetManager::Get_Instance()->Get_Texture (name, mipcount);

			if (no_lod) 
			{
				newtex->Get_Filter().Set_Mip_Mapping(TextureFilterClass::FILTER_TYPE_NONE);
			}
			bool u_clamp = ((texinfo.Attributes & W3DTEXTURE_CLAMP_U) != 0);
			newtex->Get_Filter().Set_U_Addr_Mode(u_clamp ? TextureFilterClass::TEXTURE_ADDRESS_CLAMP : TextureFilterClass::TEXTURE_ADDRESS_REPEAT);
			bool v_clamp = ((texinfo.Attributes & W3DTEXTURE_CLAMP_V) != 0);
			newtex->Get_Filter().Set_V_Addr_Mode(v_clamp ? TextureFilterClass::TEXTURE_ADDRESS_CLAMP : TextureFilterClass::TEXTURE_ADDRESS_REPEAT);

			switch (texinfo.Attributes & W3DTEXTURE_TYPE_MASK) 
			{

				case W3DTEXTURE_TYPE_COLORMAP:
					// Do nothing.
					break;

				case W3DTEXTURE_TYPE_BUMPMAP:
				{
					TextureClass *releasetex = newtex;

					// Format is assumed to be a grayscale heightmap. Convert it to a bump map.
					newtex = WW3DAssetManager::Get_Instance()->Get_Bumpmap_Based_On_Texture (newtex);
					WW3DAssetManager::Get_Instance()->Release_Texture (releasetex);
					break;
				}

				default:
					WWASSERT (false);
					break;
			}

		} else {
			newtex = WW3DAssetManager::Get_Instance()->Get_Texture(name);
		}

		WWASSERT(newtex);
	}

	// Return a pointer to the new texture	
	return newtex;
}

// Utility function used by Save_Texture
void setup_texture_attributes(TextureClass * tex, W3dTextureInfoStruct * texinfo)
{
	texinfo->Attributes = 0;

	if (tex->Get_Filter().Get_Mip_Mapping() == TextureFilterClass::FILTER_TYPE_NONE) texinfo->Attributes |= W3DTEXTURE_NO_LOD;
	if (tex->Get_Filter().Get_U_Addr_Mode() == TextureFilterClass::TEXTURE_ADDRESS_CLAMP) texinfo->Attributes |= W3DTEXTURE_CLAMP_U;
	if (tex->Get_Filter().Get_V_Addr_Mode() == TextureFilterClass::TEXTURE_ADDRESS_CLAMP) texinfo->Attributes |= W3DTEXTURE_CLAMP_V;
}


void Save_Texture(TextureClass * texture,ChunkSaveClass & csave)
{
	const char * filename;
	W3dTextureInfoStruct texinfo;
	memset(&texinfo,0,sizeof(texinfo));

	filename = texture->Get_Full_Path();

	setup_texture_attributes(texture, &texinfo);

	csave.Begin_Chunk(W3D_CHUNK_TEXTURE_NAME);
	csave.Write(filename,strlen(filename)+1);
	csave.End_Chunk();

	if ((texinfo.Attributes != 0) || (texinfo.AnimType != 0) || (texinfo.FrameCount != 0)) {
		csave.Begin_Chunk(W3D_CHUNK_TEXTURE_INFO);
		csave.Write(&texinfo, sizeof(texinfo));
		csave.End_Chunk();
	}
}

// ----------------------------------------------------------------------------

BumpmapTextureClass::BumpmapTextureClass(TextureClass* texture)
	:
//	TextureClass(texture->Get_Width(),texture->Get_Height(),texture->Get_Textur4e_Format(),MIP_LEVELS_1)
	TextureClass(TextureLoader::Generate_Bumpmap(texture))
{
//	D3DTexture=TextureLoader::Generate_Bumpmap(texture);
//		TextureLoader:::Generage_Bumpmap
}

BumpmapTextureClass::~BumpmapTextureClass()
{
}

