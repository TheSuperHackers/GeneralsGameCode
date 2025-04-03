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

#include "textureloader.h"
#include "mutex.h"
#include "thread.h"
#include "wwdebug.h"
#include "texture.h"
#include "ffactory.h"
#include "wwstring.h"
#include	"bufffile.h"
#include "ww3d.h"
#include "assetmgr.h"
#include "dx8wrapper.h"
#include "dx8caps.h"
#include "missingtexture.h"
#include "TARGA.H"
#include <d3dx8tex.h>
#include <cstdio>
#include "wwmemlog.h"
#include "texture.h"
#include "formconv.h"
#include "texturethumbnail.h"
#include "ddsfile.h"
#include "bitmaphandler.h"

static TextureLoadTaskClass* LoadListHead;
static TextureLoadTaskClass* DeferredListHead;
static TextureLoadTaskClass* FinishedListHead;
static TextureLoadTaskClass* ThumbnailListHead;
static TextureLoadTaskClass* DeleteTaskListHead;

TextureLoadTaskClass* TextureLoadTaskClass::FreeTaskListHead;

static bool Is_Format_Compressed(WW3DFormat texture_format,bool allow_compression)
{
	// Verify that the user isn't requesting compressed texture without hardware support

	bool compressed=false;
	if (texture_format!=WW3D_FORMAT_UNKNOWN) {
		if (!DX8Caps::Support_DXTC() || !allow_compression) {
			WWASSERT(texture_format!=WW3D_FORMAT_DXT1);
			WWASSERT(texture_format!=WW3D_FORMAT_DXT2);
			WWASSERT(texture_format!=WW3D_FORMAT_DXT3);
			WWASSERT(texture_format!=WW3D_FORMAT_DXT4);
			WWASSERT(texture_format!=WW3D_FORMAT_DXT5);
		}
		if (texture_format==WW3D_FORMAT_DXT1 ||
			texture_format==WW3D_FORMAT_DXT2 ||
			texture_format==WW3D_FORMAT_DXT3 ||
			texture_format==WW3D_FORMAT_DXT4 ||
			texture_format==WW3D_FORMAT_DXT5) {
			compressed=true;
		}
	}

	// If hardware supports DXTC compression, load a compressed texture. Proceed only if the texture format hasn't been
	// defined as non-compressed.
	compressed|=(
		texture_format==WW3D_FORMAT_UNKNOWN && 
		DX8Caps::Support_DXTC() && 
		WW3D::Get_Texture_Compression_Mode()==WW3D::TEXTURE_COMPRESSION_ENABLE &&
		allow_compression);

	return compressed;
}

// ----------------------------------------------------------------------------

static CriticalSectionClass mutex;
static class LoaderThreadClass : public ThreadClass
{
	TextureLoadTaskClass* Get_Task_From_Load_List();

	static void Add_Task_To_Finished_List(TextureLoadTaskClass* task);

public:
	LoaderThreadClass() : ThreadClass() {}

	void Thread_Function();

	static void Add_Task_To_Load_List(TextureLoadTaskClass* task);

} thread;

// ----------------------------------------------------------------------------

void TextureLoader::Init()
{
	WWASSERT(!thread.Is_Running());

	ThumbnailClass::Init();

	thread.Execute();
	thread.Set_Priority(-3);
}

// ----------------------------------------------------------------------------

void TextureLoader::Deinit()
{
	CriticalSectionClass::LockClass m(mutex);
	thread.Stop();

	ThumbnailClass::Deinit();
}

// ----------------------------------------------------------------------------
//
// Modify given texture size to nearest valid size on current hardware.
//
// ----------------------------------------------------------------------------

void TextureLoader::Validate_Texture_Size(unsigned& width, unsigned& height)
{
	const D3DCAPS8& dx8caps=DX8Caps::Get_Default_Caps();

	unsigned poweroftwowidth = 1;
	while (poweroftwowidth < width) {
		poweroftwowidth <<= 1;
	}

	unsigned poweroftwoheight = 1;
	while (poweroftwoheight < height) {
		poweroftwoheight <<= 1;
	}

//	unsigned size = MAX (width, height);
//	unsigned poweroftwosize = 1;
//	while (poweroftwosize < size) {
//		poweroftwosize <<= 1;
//	}

	if (poweroftwowidth>dx8caps.MaxTextureWidth) {
		poweroftwowidth=dx8caps.MaxTextureWidth;
	}
	if (poweroftwoheight>dx8caps.MaxTextureHeight) {
		poweroftwoheight=dx8caps.MaxTextureHeight;
	}

	if (poweroftwowidth>poweroftwoheight) {
		while (poweroftwowidth/poweroftwoheight>8) {
			poweroftwoheight*=2;
		}
	}
	else {
		while (poweroftwoheight/poweroftwowidth>8) {
			poweroftwowidth*=2;
		}
	}

//	width = height = poweroftwosize;
	width=poweroftwowidth;
	height=poweroftwoheight;
}



IDirect3DTexture8* TextureLoader::Load_Thumbnail(const StringClass& filename,WW3DFormat texture_format)
{
	ThumbnailClass* thumb=ThumbnailClass::Peek_Instance(filename);
	if (!thumb) {
		thumb=W3DNEW ThumbnailClass(filename);
		// If load failed, return missing texture
		if (!thumb->Peek_Bitmap()) {
			delete thumb;
			return MissingTexture::_Get_Missing_Texture();
		}
	}

	unsigned src_pitch=thumb->Get_Width()*4;	// Thumbs are always 32 bits
	WW3DFormat dest_format;
	if (texture_format==WW3D_FORMAT_UNKNOWN) {
		dest_format=Get_Valid_Texture_Format(WW3D_FORMAT_A8R8G8B8,false); // no compressed formats please
	}
	else {
		dest_format=Get_Valid_Texture_Format(texture_format,false);	// no compressed formats please
		WWASSERT(dest_format==texture_format);
	}

	IDirect3DTexture8* d3d_texture = DX8Wrapper::_Create_DX8_Texture(
		thumb->Get_Width(),
		thumb->Get_Height(),
		dest_format,
		MIP_LEVELS_ALL);

	unsigned level=0;
	D3DLOCKED_RECT locked_rects[12];
	WWASSERT(d3d_texture->GetLevelCount()<=12);

	// Lock all surfaces
	for (level=0;level<d3d_texture->GetLevelCount();++level) {
		DX8_ErrorCode(
			d3d_texture->LockRect(
				level,
				&locked_rects[level],
				NULL,
				0));
	}

	unsigned char* src_surface=thumb->Peek_Bitmap();
	WW3DFormat src_format=WW3D_FORMAT_A8R8G8B8;
	unsigned width=thumb->Get_Width();
	unsigned height=thumb->Get_Height();

	for (level=0;level<d3d_texture->GetLevelCount()-1;++level) {
		BitmapHandlerClass::Copy_Image_Generate_Mipmap(
			width,
			height,
			(unsigned char*)locked_rects[level].pBits, 
			locked_rects[level].Pitch,
			dest_format,
			src_surface,
			src_pitch,
			src_format,
			(unsigned char*)locked_rects[level+1].pBits,	// mipmap
			locked_rects[level+1].Pitch);// mipmap

		src_format=dest_format;
		src_surface=(unsigned char*)locked_rects[level].pBits;
		src_pitch=locked_rects[level].Pitch;
		width>>=1;
		height>>=1;
	}

	// Unlock all surfaces
	for (level=0;level<d3d_texture->GetLevelCount();++level) {
		DX8_ErrorCode(d3d_texture->UnlockRect(level));
	}

	return d3d_texture;
}

static bool Is_Power_Of_Two(unsigned i)
{
	if (!i) return false;
	unsigned n=i;
	unsigned shift=0;
	while (n) {
		shift++;
		n>>=1;
	}
	return ((i>>(shift-1))<<(shift-1))==i;
}

// ----------------------------------------------------------------------------

// TODO: Legacy - remove this call!
IDirect3DTexture8* Load_Compressed_Texture(
	const StringClass& filename, 
	unsigned reduction_factor,
	MipCountType mip_level_count,
	WW3DFormat dest_format)
{
	// If DDS file isn't available, use TGA file to convert to DDS.

	DDSFileClass dds_file(filename,reduction_factor);
	if (!dds_file.Is_Available()) return NULL;
	if (!dds_file.Load()) return NULL;

	unsigned width=dds_file.Get_Width(0);
	unsigned height=dds_file.Get_Height(0);
	unsigned mips=dds_file.Get_Mip_Level_Count();

	// If format isn't defined get the nearest valid texture format to the compressed file format
	// Note that the nearest valid format could be anything, even uncompressed.
	if (dest_format==WW3D_FORMAT_UNKNOWN) dest_format=Get_Valid_Texture_Format(dds_file.Get_Format(),true);

	IDirect3DTexture8* d3d_texture = DX8Wrapper::_Create_DX8_Texture(
		width,
		height,
		dest_format,
		(MipCountType)mips);

	for (unsigned level=0;level<mips;++level) {
		IDirect3DSurface8* d3d_surface=NULL;
		WWASSERT(d3d_texture);
		DX8_ErrorCode(d3d_texture->GetSurfaceLevel(level/*-reduction_factor*/,&d3d_surface));
		dds_file.Copy_Level_To_Surface(level,d3d_surface);
		d3d_surface->Release();
	}
	return d3d_texture;
}

// ----------------------------------------------------------------------------
//
// Load image to a surface. The function tries to create texture that matches
// targa format. If suitable format is not available, it selects closest matching
// format and performs color space conversion.
//
// ----------------------------------------------------------------------------
IDirect3DSurface8* TextureLoader::Load_Surface_Immediate(
	const StringClass& filename,
	WW3DFormat texture_format,
	bool allow_compression)
{
	bool compressed=Is_Format_Compressed(texture_format,allow_compression);

	if (compressed) {
		IDirect3DTexture8* comp_tex=Load_Compressed_Texture(filename,0,MIP_LEVELS_1,WW3D_FORMAT_UNKNOWN);
		if (comp_tex) {
			IDirect3DSurface8* d3d_surface=NULL;
			DX8_ErrorCode(comp_tex->GetSurfaceLevel(0,&d3d_surface));
			comp_tex->Release();
			return d3d_surface;
		}
	}

	// Make sure the file can be opened. If not, return missing texture.
	Targa targa;
	if (TARGA_ERROR_HANDLER(targa.Open(filename, TGA_READMODE),filename)) return MissingTexture::_Create_Missing_Surface();

	// DX8 uses image upside down compared to TGA
	targa.Header.ImageDescriptor ^= TGAIDF_YORIGIN;

	WW3DFormat src_format,dest_format;
	unsigned src_bpp=0;
	Get_WW3D_Format(dest_format,src_format,src_bpp,targa);

	if (texture_format!=WW3D_FORMAT_UNKNOWN) {
		dest_format=texture_format;
	}

	// Destination size will be the next power of two square from the larger width and height...
	unsigned width, height;
	width=targa.Header.Width;
	height=targa.Header.Height;
	unsigned src_width=targa.Header.Width;
	unsigned src_height=targa.Header.Height;

	// NOTE: We load the palette but we do not yet support paletted textures!
	char palette[256*4];
	targa.SetPalette(palette);
	if (TARGA_ERROR_HANDLER(targa.Load(filename, TGAF_IMAGE, false),filename)) return MissingTexture::_Create_Missing_Surface();

	unsigned char* src_surface=(unsigned char*)targa.GetImage();

	// No paletted destination format allowed
	unsigned char* converted_surface=NULL;
	if (src_format==WW3D_FORMAT_A1R5G5B5 || src_format==WW3D_FORMAT_R5G6B5 || src_format==WW3D_FORMAT_A4R4G4B4 ||
		src_format==WW3D_FORMAT_P8 || src_format==WW3D_FORMAT_L8 || src_width!=width || src_height!=height) {
		converted_surface=W3DNEWARRAY unsigned char[width*height*4];
		dest_format=Get_Valid_Texture_Format(WW3D_FORMAT_A8R8G8B8,false);
		BitmapHandlerClass::Copy_Image(
			converted_surface, 
			width,
			height,
			width*4,
			WW3D_FORMAT_A8R8G8B8,//dest_format,
			src_surface,
			src_width,
			src_height,
			src_width*src_bpp,
			src_format,
			(unsigned char*)targa.GetPalette(),
			targa.Header.CMapDepth>>3,
			false);
		src_surface=converted_surface;
		src_format=WW3D_FORMAT_A8R8G8B8;//dest_format;
		src_width=width;
		src_height=height;
		src_bpp=Get_Bytes_Per_Pixel(src_format);
	}

	unsigned src_pitch=src_width*src_bpp;

	IDirect3DSurface8* d3d_surface = DX8Wrapper::_Create_DX8_Surface(width,height,dest_format);
	WWASSERT(d3d_surface);
	D3DLOCKED_RECT locked_rect;
	DX8_ErrorCode(
		d3d_surface->LockRect(
			&locked_rect,
			NULL,
			0));

	BitmapHandlerClass::Copy_Image(
		(unsigned char*)locked_rect.pBits, 
		width,
		height,
		locked_rect.Pitch,
		dest_format,
		src_surface,
		src_width,
		src_height,
		src_pitch,
		src_format,
		(unsigned char*)targa.GetPalette(),
		targa.Header.CMapDepth>>3,
		false);	// No mipmap

	DX8_ErrorCode(d3d_surface->UnlockRect());

	if (converted_surface) delete[] converted_surface;

	return d3d_surface;
}


// ----------------------------------------------------------------------------
//
// Load mipmap levels to a pre-generated and locked texture object based on
// information in load task object. Try loading from a DDS file first and if
// that fails try a TGA.
//
// ----------------------------------------------------------------------------

void TextureLoader::Load_Mipmap_Levels(TextureLoadTaskClass* task)
{
	WWASSERT(task->Peek_D3D_Texture());
	WWMEMLOG(MEM_TEXTURE);

	if (task->Peek_Texture()->Is_Compression_Allowed()) {
		DDSFileClass dds_file(task->Peek_Texture()->Get_Full_Path(),task->Get_Reduction());
		if (dds_file.Is_Available() && dds_file.Load()) {
			unsigned width=task->Get_Width();
			unsigned height=task->Get_Height();
			for (unsigned level=0;level<task->Get_Mip_Level_Count();++level) {
				WWASSERT(width && height);
				dds_file.Copy_Level_To_Surface(
					level,
					task->Get_Format(),
					width, 
					height, 
					task->Get_Locked_Surface_Ptr(level), 
					task->Get_Locked_Surface_Pitch(level));
				width>>=1;
				height>>=1;
			}
			return;
		}
	}
	if (Load_Uncompressed_Mipmap_Levels_From_TGA(task)) return;
}

// ----------------------------------------------------------------------------
//
// Use load task to load texture surface from a targa file. Calculate mipmaps
// if needed.
//
// ----------------------------------------------------------------------------

bool TextureLoader::Load_Uncompressed_Mipmap_Levels_From_TGA(TextureLoadTaskClass* task)
{
	if (!task->Get_Mip_Level_Count()) return false;
	TextureClass* texture=task->Peek_Texture();

	Targa targa;
	if (TARGA_ERROR_HANDLER(targa.Open(texture->Get_Full_Path(), TGA_READMODE),texture->Get_Full_Path())) {
		task->Set_Fail(true);
		return false;
	}
	// DX8 uses image upside down compared to TGA
	targa.Header.ImageDescriptor ^= TGAIDF_YORIGIN;

	WW3DFormat src_format,dest_format;
	unsigned src_bpp=0;
	Get_WW3D_Format(dest_format,src_format,src_bpp,targa);
//	WWASSERT(task->Get_Format()==dest_format);
	dest_format=task->Get_Format();	// Texture can be requested in different format than the most obvious from the TGA

	char palette[256*4];
	targa.SetPalette(palette);

	unsigned src_width=targa.Header.Width;
	unsigned src_height=targa.Header.Height;
	unsigned width=task->Get_Width();
	unsigned height=task->Get_Height();

	// NOTE: We load the palette but we do not yet support paletted textures!
	if (TARGA_ERROR_HANDLER(targa.Load(texture->Get_Full_Path(), TGAF_IMAGE, false),texture->Get_Full_Path())) {
		task->Set_Fail(true);
		return false;
	}
	unsigned char* src_surface=(unsigned char*)targa.GetImage();

	// No paletted format allowed when generating mipmaps
	unsigned char* converted_surface=NULL;
	if (src_format==WW3D_FORMAT_A1R5G5B5 || src_format==WW3D_FORMAT_R5G6B5 || src_format==WW3D_FORMAT_A4R4G4B4 ||
		src_format==WW3D_FORMAT_P8 || src_format==WW3D_FORMAT_L8 || src_width!=width || src_height!=height) {
		converted_surface=W3DNEWARRAY unsigned char[width*height*4];
		dest_format=Get_Valid_Texture_Format(WW3D_FORMAT_A8R8G8B8,false);
		BitmapHandlerClass::Copy_Image(
			converted_surface, 
			width,
			height,
			width*4,
			WW3D_FORMAT_A8R8G8B8,	//dest_format,
			src_surface,
			src_width,
			src_height,
			src_width*src_bpp,
			src_format,
			(unsigned char*)targa.GetPalette(),
			targa.Header.CMapDepth>>3,
			false);
		src_surface=converted_surface;
		src_format=WW3D_FORMAT_A8R8G8B8;	//dest_format;
		src_width=width;
		src_height=height;
		src_bpp=Get_Bytes_Per_Pixel(src_format);
	}

	unsigned src_pitch=src_width*src_bpp;

	for (unsigned level=0;level<task->Get_Mip_Level_Count();++level) {
		WWASSERT(task->Get_Locked_Surface_Ptr(level));
		BitmapHandlerClass::Copy_Image(
			task->Get_Locked_Surface_Ptr(level),
			width,
			height,
			task->Get_Locked_Surface_Pitch(level),
			task->Get_Format(),
			src_surface,
			src_width,
			src_height,
			src_pitch,
			src_format,
			NULL,
			0,
			true);

		width>>=1;
		height>>=1;
		src_width>>=1;
		src_height>>=1;
		if (!width || !height || !src_width || !src_height) break;
	}

	if (converted_surface) delete[] converted_surface;

	return true;
}

// ----------------------------------------------------------------------------
//
// Return a task from the load list head. The loading thread uses this function
// to retrieve tasks from the load list.
//
// ----------------------------------------------------------------------------

TextureLoadTaskClass* LoaderThreadClass::Get_Task_From_Load_List()
{
	CriticalSectionClass::LockClass m(mutex);

	TextureLoadTaskClass* task=LoadListHead;
	if (task) {
		LoadListHead=task->Peek_Succ();
		task->Set_Succ(NULL);
	}
	return task;
}

// ----------------------------------------------------------------------------
//
// This function adds a load task to the head of the loading thread task list.
// The latest added task will be the next processed (There are good reasons
// for such ordering). The loading thread will process tasks from this list
// as soons as it can and then move the tasks to finished list.
//
// ----------------------------------------------------------------------------

void LoaderThreadClass::Add_Task_To_Load_List(TextureLoadTaskClass* task)
{
	CriticalSectionClass::LockClass m(mutex);

	WWASSERT(task->Peek_Succ()==NULL);

	task->Set_Succ(LoadListHead);
	LoadListHead=task;
}

// ----------------------------------------------------------------------------
//
// After the loading thread is done with the texture, it is moved to the list
// of finished tasks so that the main thread can then finish up by unlocking
// the surfaces and applying the changes to the texture class object.
//
// ----------------------------------------------------------------------------

void LoaderThreadClass::Add_Task_To_Finished_List(TextureLoadTaskClass* task)
{
	CriticalSectionClass::LockClass m(mutex);

	WWASSERT(task->Peek_Succ()==NULL);

	task->Set_Succ(FinishedListHead);
	FinishedListHead=task;
}

// ----------------------------------------------------------------------------
//
// If we need to find out if the load task list is empty this is the function
// to use. We can't use Get_Task_From_Load_List() as if the list isn't empty
// it also removes the head node from the list.
//
// ----------------------------------------------------------------------------

bool Is_Load_List_Empty()
{
	return !LoadListHead;
}
// ----------------------------------------------------------------------------
//
// Texture loading thread loads textures that appear in loading_task_list.
// If the list is empty the thread sleeps.
//
// ----------------------------------------------------------------------------

void LoaderThreadClass::Thread_Function()
{
	while (running) {
		TextureLoadTaskClass* task=Get_Task_From_Load_List();
		if (task) {
			TextureLoader::Load_Mipmap_Levels(task);
			Add_Task_To_Finished_List(task);
		}

		Switch_Thread();
	}
}

// ----------------------------------------------------------------------------
//
// Update refreshes all completed texture loading tasks
//
// ----------------------------------------------------------------------------

TextureLoadTaskClass* Get_Finished_Task()
{
	CriticalSectionClass::LockClass m(mutex);

	TextureLoadTaskClass* task=FinishedListHead;
	if (task) {
		FinishedListHead=task->Peek_Succ();
		task->Set_Succ(NULL);
	}
	return task;
}

TextureLoadTaskClass* Get_Thumbnail_Task()
{
	CriticalSectionClass::LockClass m(mutex);

	TextureLoadTaskClass* task=ThumbnailListHead;
	if (task) {
		ThumbnailListHead=task->Peek_Succ();
		task->Set_Succ(NULL);
	}
	return task;
}

void Add_Thumbnail_Task(TextureLoadTaskClass* task)
{
	CriticalSectionClass::LockClass m(mutex);

	WWASSERT(task->Peek_Succ()==NULL);

	task->Set_Succ(ThumbnailListHead);
	ThumbnailListHead=task;

}

// ----------------------------------------------------------------------------
//
// The main thread's update function deletes tasks from the load task list
// once a frame.
//
// ----------------------------------------------------------------------------

TextureLoadTaskClass* Get_Task_From_Delete_List()
{
	WWASSERT(ThreadClass::_Get_Current_Thread_ID()==DX8Wrapper::_Get_Main_Thread_ID());

	TextureLoadTaskClass* task=DeleteTaskListHead;
	if (task) {
		DeleteTaskListHead=task->Peek_Succ();
		task->Set_Succ(NULL);
	}
	return task;
}

// ----------------------------------------------------------------------------
//
// When task wants to delete itself it adds itself to a delete list. This list
// can only be accessed from the main thread.
//
// ----------------------------------------------------------------------------

void Add_Task_To_Delete_List(TextureLoadTaskClass* task)
{
	WWASSERT(ThreadClass::_Get_Current_Thread_ID()==DX8Wrapper::_Get_Main_Thread_ID());

	WWASSERT(task->Peek_Succ()==NULL);

	task->Set_Succ(DeleteTaskListHead);
	DeleteTaskListHead=task;
}

TextureLoadTaskClass* Get_Deferred_Task()
{
	CriticalSectionClass::LockClass m(mutex);

	TextureLoadTaskClass* task=DeferredListHead;
	if (task) {
		DeferredListHead=task->Peek_Succ();
		task->Set_Succ(NULL);
	}
	return task;
}

void Add_Deferred_Task(TextureLoadTaskClass* task)
{
	CriticalSectionClass::LockClass m(mutex);

	WWASSERT(task->Peek_Succ()==NULL);
	task->Set_Succ(DeferredListHead);
	DeferredListHead=task;
}

void TextureLoader::Flush_Pending_Load_Tasks()
{
	while (!Is_Load_List_Empty()) {
		Update();
		ThreadClass::Switch_Thread();
	}
}

void TextureLoader::Update()
{
	WWASSERT_PRINT(DX8Wrapper::_Get_Main_Thread_ID()==ThreadClass::_Get_Current_Thread_ID(),"TextureLoader::Update must be called from the main thread!");

	while (TextureLoadTaskClass* task=Get_Deferred_Task()) {
		task->Begin_Texture_Load();	// This will add the task to load list
	}

	while (TextureLoadTaskClass* task=Get_Finished_Task()) {
		task->End_Load();
		task->Apply(true);
		TextureLoadTaskClass::Release_Instance(task);
	}

	while (TextureLoadTaskClass* task=Get_Thumbnail_Task()) {
		task->Begin_Thumbnail_Load();
	}

	while (TextureLoadTaskClass* task=Get_Task_From_Delete_List()) {
//		delete task;
		TextureLoadTaskClass::Release_Instance(task);
	}
}

// ----------------------------------------------------------------------------

static DWORD VectortoRGBA( D3DXVECTOR3* v, FLOAT fHeight )
{
    DWORD r = (DWORD)( 127.0f * v->x + 128.0f );
    DWORD g = (DWORD)( 127.0f * v->y + 128.0f );
    DWORD b = (DWORD)( 127.0f * v->z + 128.0f );
    DWORD a = (DWORD)( 255.0f * fHeight );
    
    return( (a<<24L) + (r<<16L) + (g<<8L) + (b<<0L) );
}

IDirect3DTexture8* TextureLoader::Generate_Bumpmap(TextureClass* texture)
{
	WW3DFormat bump_format=WW3D_FORMAT_U8V8;
	if (!DX8Caps::Support_Texture_Format(bump_format)) {
		return MissingTexture::_Get_Missing_Texture();
	}

	D3DSURFACE_DESC desc;
	IDirect3DTexture8* src_d3d_tex=texture->Peek_DX8_Texture();
	WWASSERT(src_d3d_tex);
	DX8_ErrorCode(src_d3d_tex->GetLevelDesc(0,&desc));
	unsigned width=desc.Width;
	unsigned height=desc.Height;

	IDirect3DTexture8* d3d_texture = DX8Wrapper::_Create_DX8_Texture(
		width,
		height,
		bump_format,
		MIP_LEVELS_1);

	D3DLOCKED_RECT src_locked_rect;
	DX8_ErrorCode(
		texture->Peek_DX8_Texture()->LockRect(
			0,
			&src_locked_rect,
			NULL,
			D3DLOCK_READONLY));

	D3DLOCKED_RECT dest_locked_rect;
	DX8_ErrorCode(
		d3d_texture->LockRect(
			0,
			&dest_locked_rect,
			NULL,
			0));

	WW3DFormat format=D3DFormat_To_WW3DFormat(desc.Format);
	unsigned bpp=Get_Bytes_Per_Pixel(format);

	for( unsigned y=0; y<desc.Height; y++ )
	{
		unsigned char* dest_ptr  = (unsigned char*)dest_locked_rect.pBits;
		dest_ptr+=y*dest_locked_rect.Pitch;
		unsigned char* src_ptr_mid = (unsigned char*)src_locked_rect.pBits;
		src_ptr_mid+=y*src_locked_rect.Pitch;
		unsigned char* src_ptr_next_line = ( src_ptr_mid + src_locked_rect.Pitch );
		unsigned char* src_ptr_prev_line = ( src_ptr_mid - src_locked_rect.Pitch );

		if( y == desc.Height-1 )  // Don't go past the last line
			src_ptr_next_line = src_ptr_mid;
		if( y == 0 )               // Don't go before first line
			src_ptr_prev_line = src_ptr_mid;

		for( unsigned x=0; x<desc.Width; x++ )
		{
			unsigned pixel00;
			unsigned pixel01;
			unsigned pixelM1;
			unsigned pixel10;
			unsigned pixel1M;

			BitmapHandlerClass::Read_B8G8R8A8(pixel00,src_ptr_mid,format,NULL,0);
			BitmapHandlerClass::Read_B8G8R8A8(pixel01,src_ptr_mid+bpp,format,NULL,0);
			BitmapHandlerClass::Read_B8G8R8A8(pixelM1,src_ptr_mid-bpp,format,NULL,0);
			BitmapHandlerClass::Read_B8G8R8A8(pixel10,src_ptr_prev_line,format,NULL,0);
			BitmapHandlerClass::Read_B8G8R8A8(pixel1M,src_ptr_next_line,format,NULL,0);

			// Convert to luminance
			unsigned char bv00;
			unsigned char bv01;
			unsigned char bvM1;
			unsigned char bv10;
			unsigned char bv1M;
			BitmapHandlerClass::Write_B8G8R8A8(&bv00,WW3D_FORMAT_L8,pixel00);
			BitmapHandlerClass::Write_B8G8R8A8(&bv01,WW3D_FORMAT_L8,pixel01);
			BitmapHandlerClass::Write_B8G8R8A8(&bvM1,WW3D_FORMAT_L8,pixelM1);
			BitmapHandlerClass::Write_B8G8R8A8(&bv10,WW3D_FORMAT_L8,pixel10);
			BitmapHandlerClass::Write_B8G8R8A8(&bv1M,WW3D_FORMAT_L8,pixel1M);
			int v00=bv00,v01=bv01,vM1=bvM1,v10=bv10,v1M=bv1M;

			int iDu = (vM1-v01); // The delta-u bump value
			int iDv = (v1M-v10); // The delta-v bump value

			if( (v00 < vM1) && (v00 < v01) )  // If we are at valley
			{
				 iDu = vM1-v00;                 // Choose greater of 1st order diffs
				 if( iDu < v00-v01 )
					  iDu = v00-v01;
			}

			// The luminance bump value (land masses are less shiny)
			unsigned short uL = ( v00>1 ) ? 63 : 127;

			switch( bump_format )
			{
			case WW3D_FORMAT_U8V8:
				*dest_ptr++ = (unsigned char)iDu;
				*dest_ptr++ = (unsigned char)iDv;
				break;

			case WW3D_FORMAT_L6V5U5:
				*(unsigned short*)dest_ptr  = (unsigned short)( ( (iDu>>3) & 0x1f ) <<  0 );
				*(unsigned short*)dest_ptr |= (unsigned short)( ( (iDv>>3) & 0x1f ) <<  5 );
				*(unsigned short*)dest_ptr |= (unsigned short)( ( ( uL>>2) & 0x3f ) << 10 );
				dest_ptr += 2;
				break;

			case WW3D_FORMAT_X8L8V8U8:
				*dest_ptr++ = (unsigned char)iDu;
				*dest_ptr++ = (unsigned char)iDv;
				*dest_ptr++ = (unsigned char)uL;
				*dest_ptr++ = (unsigned char)0L;
				break;
			}

			// Move one pixel to the left (src is 32-bpp)
			src_ptr_mid+=bpp;
			src_ptr_prev_line+=bpp;
			src_ptr_next_line+=bpp;
		}
	}

	DX8_ErrorCode(d3d_texture->UnlockRect(0));
	DX8_ErrorCode(texture->Peek_DX8_Texture()->UnlockRect(0));
	return d3d_texture;
}

// ----------------------------------------------------------------------------
//
// Public texture request functions. These functions can be used to request
// texture loading.
//
// ----------------------------------------------------------------------------

void TextureLoader::Add_Load_Task(TextureClass* tc)
{
	// If the texture is already being loaded we just exit here.
	if (tc->TextureLoadTask) return;

	TextureLoadTaskClass* task=TextureLoadTaskClass::Get_Instance(tc,false);
	task->Begin_Texture_Load();
}

// ----------------------------------------------------------------------------

void TextureLoader::Request_High_Priority_Loading(
	TextureClass* tc,
	MipCountType mip_level_count)
{
	TextureLoadTaskClass* task=TextureLoadTaskClass::Get_Instance(tc,true);
	task->Begin_Texture_Load();
}

// ----------------------------------------------------------------------------

void TextureLoader::Request_Thumbnail(TextureClass* tc)
{
	// If the texture is already being loaded we just exit here.
	if (tc->TextureLoadTask) return;

	TextureLoadTaskClass* task=TextureLoadTaskClass::Get_Instance(tc,false);
	task->Begin_Thumbnail_Load();
}

// ----------------------------------------------------------------------------
//
// Texture loader task handler
//
// ----------------------------------------------------------------------------

TextureLoadTaskClass::TextureLoadTaskClass()
	:
	Texture(0),
	Succ(0), 
	D3DTexture(0),
	Width(0),
	Height(0),
	Format(WW3D_FORMAT_UNKNOWN),
	IsLoading(false),
	HasFailed(false),
	MipLevelCount(0),
	HighPriorityRequested(false),
	Reduction(0)
{
}

// ----------------------------------------------------------------------------
//
// Destruct texture load task. The load task deinitialization will fail from
// any other than the main thread so the task must be either deleted from the
// main thread or deinitialized in the main thread prior to deleting it in
// another thread.
//
// ----------------------------------------------------------------------------

TextureLoadTaskClass::~TextureLoadTaskClass()
{
	Deinit();
}

void TextureLoadTaskClass::Init(TextureClass* tc,bool high_priority)
{
	// Make sure texture has a filename.
	REF_PTR_SET(Texture,tc);
	WWASSERT(Texture->Get_Full_Path() != NULL);

	Reduction=Texture->Get_Reduction();
	HighPriorityRequested=high_priority;
	IsLoading=false;
	HasFailed=false;
	MipLevelCount=tc->MipLevelCount;
	D3DTexture=0;
	Width=0;
	Height=0;
	Format=Texture->Get_Texture_Format();

	Texture->TextureLoadTask=this;
	for (int i=0;i<MIP_LEVELS_MAX;++i) {
		LockedSurfacePtr[i]=NULL;
		LockedSurfacePitch[i]=0;
	}
}

void TextureLoadTaskClass::Deinit()
{
	WWASSERT(Succ==NULL);
	WWASSERT(D3DTexture==NULL);
	WWASSERT(IsLoading==false);
	for (int i=0;i<MIP_LEVELS_MAX;++i) {
		WWASSERT(LockedSurfacePtr[i]==NULL);
	}
	if (Texture) {
		WWASSERT(Texture->TextureLoadTask==this);
		Texture->TextureLoadTask=NULL;
	}

	REF_PTR_RELEASE(Texture);
	WWASSERT(!D3DTexture);
}

// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------

void TextureLoadTaskClass::Begin_Texture_Load()
{
	// If we're in main thread, init for loading and add to the load list
	if (ThreadClass::_Get_Current_Thread_ID()==DX8Wrapper::_Get_Main_Thread_ID()) {

		bool loaded=false;
		if (Texture->Is_Compression_Allowed()) {
			DDSFileClass dds_file(Texture->Get_Full_Path(),Get_Reduction());
			if (dds_file.Is_Available()) {
				// Destination size will be the next power of two square from the larger width and height...
				unsigned width, height;
				width=dds_file.Get_Width(0);
				height=dds_file.Get_Height(0);
				TextureLoader::Validate_Texture_Size(width,height);

				// If the size doesn't match, try and see if texture reduction would help... (mainly for
				// cases where loaded texture is larger than hardware limit)
				if (width!=dds_file.Get_Width(0) || height!=dds_file.Get_Height(0)) {
					for (unsigned i=1;i<dds_file.Get_Mip_Level_Count();++i) {
						unsigned w=dds_file.Get_Width(i);
						unsigned h=dds_file.Get_Height(i);
						TextureLoader::Validate_Texture_Size(width,height);
						if (w==dds_file.Get_Width(i) || h==dds_file.Get_Height(i)) {
							Reduction+=i;
							width=w;
							height=h;
							break;
						}
					}
				}

				IsLoading=true;
				Width=width;
				Height=height;
				Format=Get_Valid_Texture_Format(dds_file.Get_Format(),Texture->Is_Compression_Allowed());

				unsigned mip_level_count=Get_Mip_Level_Count();
				// If texture wants all mip levels, take as many as the file contains (not necessarily all)
				// Otherwise take as many mip levels as the texture wants, not to exceed the count in file...
				if (!mip_level_count) mip_level_count=dds_file.Get_Mip_Level_Count();
				else if (mip_level_count>dds_file.Get_Mip_Level_Count()) mip_level_count=dds_file.Get_Mip_Level_Count();

				// Once more, verify that the mip level count is correct (in case it was changed here it might not
				// match the size...well actually it doesn't have to match but it can't be bigger than the size)
				unsigned max_mip_level_count=1;
				unsigned w=4;
				unsigned h=4;
				while (w<Width && h<Height) {
					w+=w;
					h+=h;
					max_mip_level_count++;
				}
				if (mip_level_count>max_mip_level_count) mip_level_count=max_mip_level_count;

				D3DTexture=DX8Wrapper::_Create_DX8_Texture(Width,Height,Format,(MipCountType)mip_level_count);
				MipLevelCount=mip_level_count;
				//Texture->MipLevelCount);
				loaded=true;
			}
		}

		if (!loaded) {
			Targa targa;
			if (TARGA_ERROR_HANDLER(targa.Open(Texture->Get_Full_Path(), TGA_READMODE),Texture->Get_Full_Path())) {
				D3DTexture=MissingTexture::_Get_Missing_Texture();
				HasFailed=true;
				IsLoading=false;
				End_Load();
				Apply(true);
				Add_Task_To_Delete_List(this);
				return;
			}


			unsigned bpp;
			WW3DFormat src_format,dest_format;
			Get_WW3D_Format(dest_format,src_format,bpp,targa);
			if (src_format!=WW3D_FORMAT_A8R8G8B8 &&
				src_format!=WW3D_FORMAT_R8G8B8 &&
				src_format!=WW3D_FORMAT_X8R8G8B8) {
				WWDEBUG_SAY(("Invalid TGA format used in %s - only 24 and 32 bit formats should be used!\n",Texture->Get_Full_Path()));
			}

			// Destination size will be the next power of two square from the larger width and height...
			unsigned width=targa.Header.Width, height=targa.Header.Height;
			int ReductionFactor=Get_Reduction();
			int MipLevels=0;

			//Figure out how many mip levels this texture will occupy
			for (int i=width, j=height; i > 0 && j > 0; i>>=1, j>>=1)
					MipLevels++;

			//Adjust the reduction factor to keep textures above some minimum dimensions
			if (MipLevels <= WW3D::Get_Texture_Min_Mip_Levels())
				ReductionFactor=0;
			else
			{	int mipToDrop=MipLevels-WW3D::Get_Texture_Min_Mip_Levels();
				if (ReductionFactor >= mipToDrop)
					ReductionFactor=mipToDrop;
			}

			width=targa.Header.Width>>ReductionFactor;
			height=targa.Header.Height>>ReductionFactor;
			unsigned ow=width;
			unsigned oh=height;
			TextureLoader::Validate_Texture_Size(width,height);
			if (width!=ow || height!=oh) {
				WWDEBUG_SAY(("Invalid texture size, scaling required. Texture: %s, size: %d x %d -> %d x %d\n",Texture->Get_Full_Path(),ow,oh,width,height));
			}

			IsLoading=true;
			Width=width;
			Height=height;

			if (Format==WW3D_FORMAT_UNKNOWN) {
				Format=Get_Valid_Texture_Format(dest_format,false);
			}
			else {
				Format=Get_Valid_Texture_Format(Format,false);
			}

			D3DTexture=DX8Wrapper::_Create_DX8_Texture(Width,Height,Format,Texture->MipLevelCount);
		}

		MipLevelCount=D3DTexture->GetLevelCount();
		for (unsigned i=0;i<MipLevelCount;++i) {
			D3DLOCKED_RECT locked_rect;
			DX8_ErrorCode(
				D3DTexture->LockRect(
					i,
					&locked_rect,
					NULL,
					0));
			LockedSurfacePtr[i]=(unsigned char*)locked_rect.pBits;
			LockedSurfacePitch[i]=locked_rect.Pitch;
		}

		if (HighPriorityRequested) {
			TextureLoader::Load_Mipmap_Levels(this);
			End_Load();
			Apply(true);
			Add_Task_To_Delete_List(this);
		}
		else {
			LoaderThreadClass::Add_Task_To_Load_List(this);
		}
	}
	// Otherwise add to deferred list which will be handled by main thread
	else {
		Add_Deferred_Task(this);
	}
}

/*	file_auto_ptr my_tga_file(_TheFileFactory,Texture->Get_Full_Path());	
	if (my_tga_file->Is_Available()) {
		my_tga_file->Open();
		unsigned size=my_tga_file->Size();
		char* tga_memory=W3DNEWARRAY char[size];
		my_tga_file->Read(tga_memory,size);
		my_tga_file->Close();

		StringClass pth("data\\");
		pth+=Texture->Get_Texture_Name();
		RawFileClass tmp_tga_file(pth);
		tmp_tga_file.Create();
		tmp_tga_file.Write(tga_memory,size);
		tmp_tga_file.Close();
		delete[] tga_memory;

	}
*/

// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------

void TextureLoadTaskClass::Begin_Thumbnail_Load()
{
//	CriticalSectionClass::LockClass m(mutex);

	unsigned thread_id=ThreadClass::_Get_Current_Thread_ID();
	if (thread_id==DX8Wrapper::_Get_Main_Thread_ID()) {
		WW3DFormat format=Texture->Get_Texture_Format();
		// No compressed thumbnails
		switch (format) {
		case WW3D_FORMAT_DXT1:
		case WW3D_FORMAT_DXT2:
		case WW3D_FORMAT_DXT3:
		case WW3D_FORMAT_DXT4:
		case WW3D_FORMAT_DXT5:
			format=WW3D_FORMAT_A8R8G8B8; break;
		default:
			break;
		}
		D3DTexture=TextureLoader::Load_Thumbnail(Texture->Get_Full_Path(),format);

		// Thumbnail loads are always high priority, so apply immediatelly
		End_Load();
		Apply(false);
		Add_Task_To_Delete_List(this);
		return;
	}
	else {
		Add_Thumbnail_Task(this);
	}
}

// ----------------------------------------------------------------------------
//
// Deinit can be called multiple times. If any surfaces are locked this call
// can only be called from the main thread.
//
// ----------------------------------------------------------------------------

void TextureLoadTaskClass::End_Load()
{
	for (unsigned i=0;i<MipLevelCount;++i) {
		if (LockedSurfacePtr[i]) {
			WWASSERT(ThreadClass::_Get_Current_Thread_ID()==DX8Wrapper::_Get_Main_Thread_ID());
			DX8_ErrorCode(D3DTexture->UnlockRect(i));
		}
		LockedSurfacePtr[i]=NULL;
	}
	IsLoading=false;

}

// ----------------------------------------------------------------------------
//
// Link the node to another task node. This can only be done if the node isn't
// linked to something else already. If the node is linked to some other node,
// the only acceptable parameter to this function is NULL, which will unlink
// the connection.
//
// ----------------------------------------------------------------------------

void TextureLoadTaskClass::Set_Succ(TextureLoadTaskClass* succ)
{
	WWASSERT((succ && !Succ) || (!succ));	// Can't set successor pointer if it has been set already
	Succ=succ; 
}

// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------

void TextureLoadTaskClass::Set_D3D_Texture(IDirect3DTexture8* texture)
{
	WWASSERT(D3DTexture==0);
	D3DTexture=texture;
}

// ----------------------------------------------------------------------------
//
// Apply the D3D texture surface to the client texture. The parameter 'initialize'
// determines whether the client texture will be set to initialized state or
// not. Generally thumbnail tasks will not set texture to initialised state but
// all other loads do.
//
// ----------------------------------------------------------------------------

void TextureLoadTaskClass::Apply(bool initialize)
{
	WWASSERT(D3DTexture);
	// Verify that none of the mip levels are locked
	for (unsigned i=0;i<MipLevelCount;++i) {
		WWASSERT(LockedSurfacePtr[i]==NULL);
	}

	Texture->Apply_New_Surface(initialize);

	D3DTexture->Release();
	D3DTexture=NULL;
}

// ----------------------------------------------------------------------------
//
// Return locked surface pointer at a specific level. The call will
// assert if level is greater or equal to the number of mip levels or if the
// requested level has not been locked.
//
// ----------------------------------------------------------------------------

unsigned char* TextureLoadTaskClass::Get_Locked_Surface_Ptr(unsigned level)
{
	WWASSERT(level<MipLevelCount);
	WWASSERT(LockedSurfacePtr[level]);
	return LockedSurfacePtr[level];
}

// ----------------------------------------------------------------------------
//
// Return locked surface pitch (in bytes) at a specific level. The call will
// assert if level is greater or equal to the number of mip levels or if the
// requested level has not been locked.
//
// ----------------------------------------------------------------------------

unsigned TextureLoadTaskClass::Get_Locked_Surface_Pitch(unsigned level) const
{
	WWASSERT(level<MipLevelCount);
	WWASSERT(LockedSurfacePtr[level]);
	return LockedSurfacePitch[level];
}

// ----------------------------------------------------------------------------
//
// Load tasks are stored in a pool when they are not used. If the pool is empty
// a new task is created.
//
// ----------------------------------------------------------------------------

TextureLoadTaskClass* TextureLoadTaskClass::Get_Instance(TextureClass* tc, bool high_priority)
{
	CriticalSectionClass::LockClass m(mutex);

	TextureLoadTaskClass* task=FreeTaskListHead;
	if (task) {
		FreeTaskListHead=task->Peek_Succ();
		task->Set_Succ(NULL);
	}
	else {
		task=W3DNEW TextureLoadTaskClass();
	}
	task->Init(tc,high_priority);
	return task;
}

// ----------------------------------------------------------------------------
//
// When task is no longer needed it is returned to the pool.
//
// ----------------------------------------------------------------------------

void TextureLoadTaskClass::Release_Instance(TextureLoadTaskClass* task)
{
	if (!task) return;

	CriticalSectionClass::LockClass m(mutex);

	task->Deinit();

	// Task must not be in any list when it is being freed
	WWASSERT(task->Peek_Succ()==NULL);

	task->Set_Succ(FreeTaskListHead);
	FreeTaskListHead=task;
}

