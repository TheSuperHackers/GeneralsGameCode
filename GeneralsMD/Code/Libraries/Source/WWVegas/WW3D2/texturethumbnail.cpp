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

#include "texturethumbnail.h"
#include "hashtemplate.h"
#include "missingtexture.h"
#include "TARGA.h"
#include "ww3dformat.h"
#include "ddsfile.h"
#include "textureloader.h"
#include "bitmaphandler.h"
#include "ffactory.h"
#include "RAWFILE.h"
#include "wwprofile.h"
#include <windows.h>

static DLListClass<ThumbnailManagerClass> ThumbnailManagerList;
static ThumbnailManagerClass* GlobalThumbnailManager;
bool ThumbnailManagerClass::CreateThumbnailIfNotFound=false;

static void Create_Hash_Name(StringClass& name, const StringClass& thumb_name)
{
	name=thumb_name;
	int len=name.Get_Length();
	WWASSERT(!stricmp(&name[len-4],".tga") || !stricmp(&name[len-4],".dds"));
	name[len-4]='\0';
	_strlwr(name.Peek_Buffer());
}

	/*	file_auto_ptr my_tga_file(_TheFileFactory,filename);
	if (my_tga_file->Is_Available()) {
		my_tga_file->Open();
		unsigned size=my_tga_file->Size();
		char* tga_memory=new char[size];
		my_tga_file->Read(tga_memory,size);
		my_tga_file->Close();

		StringClass pth("data\\");
		pth+=filename;
		RawFileClass tmp_tga_file(pth);
		tmp_tga_file.Create();
		tmp_tga_file.Write(tga_memory,size);
		tmp_tga_file.Close();
		delete[] tga_memory;

	}
*/


ThumbnailClass::ThumbnailClass(
	ThumbnailManagerClass* manager,
	const char* name,
	unsigned char* bitmap,
	unsigned w,
	unsigned h,
	unsigned original_w,
	unsigned original_h,
	unsigned original_mip_level_count,
	WW3DFormat original_format,
	bool allocated,
	unsigned long date_time)
	:
	Manager(manager),
	Name(name),
	Bitmap(bitmap),
	Allocated(allocated),
	Width(w),
	Height(h),
	OriginalTextureWidth(original_w),
	OriginalTextureHeight(original_h),
	OriginalTextureMipLevelCount(original_mip_level_count),
	OriginalTextureFormat(original_format),
	DateTime(date_time)
{
	Manager->Insert_To_Hash(this);
}

// ----------------------------------------------------------------------------
//
// Load texture and generate mipmap levels if requested. The function tries
// to create texture that matches targa format. If suitable format is not
// available, it selects closest matching format and performs color space
// conversion.
//
// ----------------------------------------------------------------------------

ThumbnailClass::ThumbnailClass(ThumbnailManagerClass* manager, const StringClass& filename)
	:
	Manager(manager),
	Bitmap(0),
	Name(filename),
	Allocated(false),
	Width(0),
	Height(0),
	OriginalTextureWidth(0),
	OriginalTextureHeight(0),
	OriginalTextureMipLevelCount(0),
	OriginalTextureFormat(WW3D_FORMAT_UNKNOWN),
	DateTime(0)
{
	WWPROFILE(("ThumbnailClass::ThumbnailClass"));
	unsigned reduction_factor=3;

	// First, try loading image from a DDS file
	DDSFileClass dds_file(filename,reduction_factor);
	if (dds_file.Is_Available() && dds_file.Load()) {
		DateTime=dds_file.Get_Date_Time();

		int len=Name.Get_Length();
		WWASSERT(len>4);
		Name[len-3]='d';
		Name[len-2]='d';
		Name[len-1]='s';

		unsigned level=0;
		while (dds_file.Get_Width(level)>32 || dds_file.Get_Height(level)>32) {
			if (level>=dds_file.Get_Mip_Level_Count()) break;
			level++;
		}

		OriginalTextureWidth=dds_file.Get_Full_Width();
		OriginalTextureHeight=dds_file.Get_Full_Height();
		OriginalTextureFormat=dds_file.Get_Format();
		OriginalTextureMipLevelCount=dds_file.Get_Mip_Level_Count();
		Width=dds_file.Get_Width(0);
		Height=dds_file.Get_Height(0);
		Bitmap=W3DNEWARRAY unsigned char[Width*Height*2];
		Allocated=true;
		dds_file.Copy_Level_To_Surface(
			0,			// Level
			WW3D_FORMAT_A4R4G4B4,
			Width,
			Height,
			Bitmap,
			Width*2,
			Vector3(0.0f,0.0f,0.0f));// We don't want to HSV-shift here
	}
	// If DDS file can't be used try loading from TGA
	else {
		// Make sure the file can be opened. If not, return missing texture.
		Targa targa;
		if (TARGA_ERROR_HANDLER(targa.Open(filename,TGA_READMODE),filename)) return;

		// DX8 uses image upside down compared to TGA
		targa.Header.ImageDescriptor ^= TGAIDF_YORIGIN;

		WW3DFormat src_format,dest_format;
		unsigned src_bpp=0;
		Get_WW3D_Format(src_format,src_bpp,targa);
		if (src_format==WW3D_FORMAT_UNKNOWN) {
			WWDEBUG_SAY(("Unknown texture format for %s",filename.str()));
			return;
		}

		// Destination size will be the next power of two square from the larger width and height...
		OriginalTextureWidth=targa.Header.Width;
		OriginalTextureHeight=targa.Header.Height;
		OriginalTextureFormat=src_format;
		Width=targa.Header.Width>>reduction_factor;
		Height=targa.Header.Height>>reduction_factor;
		OriginalTextureMipLevelCount=1;
		unsigned iw=1;
		unsigned ih=1;
		while (iw<OriginalTextureWidth && ih<OriginalTextureHeight) {
			iw+=iw;
			ih+=ih;
			OriginalTextureMipLevelCount++;
		}

		while (Width>32 || Height>32) {
			reduction_factor++;
			Width>>=2;
			Height>>=2;
		}

		unsigned poweroftwowidth = 1;
		while (poweroftwowidth < Width) {
			poweroftwowidth <<= 1;
		}

		unsigned poweroftwoheight = 1;
		while (poweroftwoheight < Height) {
			poweroftwoheight <<= 1;
		}

		Width=poweroftwowidth;
		Height=poweroftwoheight;

		unsigned src_width=targa.Header.Width;
		unsigned src_height=targa.Header.Height;

		// NOTE: We load the palette but we do not yet support paletted textures!
		char palette[256*4];
		targa.SetPalette(palette);
		if (TARGA_ERROR_HANDLER(targa.Load(filename, TGAF_IMAGE, false),filename)) return;

		// Get time stamp from the tga file
		{
			file_auto_ptr my_tga_file(_TheFileFactory,filename);
			WWASSERT(my_tga_file->Is_Available());
			my_tga_file->Open();
			DateTime=my_tga_file->Get_Date_Time();
			my_tga_file->Close();
		}

		unsigned char* src_surface=(unsigned char*)targa.GetImage();

		int len=Name.Get_Length();
		WWASSERT(len>4);
		Name[len-3]='t';
		Name[len-2]='g';
		Name[len-1]='a';

		Bitmap=W3DNEWARRAY unsigned char[Width*Height*2];
		Allocated=true;

		dest_format=WW3D_FORMAT_A8R8G8B8;
		BitmapHandlerClass::Copy_Image(
			Bitmap,
			Width,
			Height,
			Width*2,
			WW3D_FORMAT_A4R4G4B4,
			src_surface,
			src_width,
			src_height,
			src_width*src_bpp,
			src_format,
			(unsigned char*)targa.GetPalette(),
			targa.Header.CMapDepth>>3,
			false);
	}

	Manager->Insert_To_Hash(this);
}

ThumbnailClass::~ThumbnailClass()
{
	if (Allocated) delete[] Bitmap;
	Manager->Remove_From_Hash(this);
}


// ----------------------------------------------------------------------------
ThumbnailManagerClass::ThumbnailManagerClass(const char* thumbnail_filename)
	:
	ThumbnailMemory(NULL),
	ThumbnailFileName(thumbnail_filename),
	PerTextureTimeStampUsed(false),
	Changed(false),
	DateTime(0)
{
}

// ----------------------------------------------------------------------------
ThumbnailManagerClass::~ThumbnailManagerClass()
{
	HashTemplateIterator<StringClass,ThumbnailClass*> ite(ThumbnailHash);
	ite.First();
	while (!ite.Is_Done()) {
		ThumbnailClass* thumb=ite.Peek_Value();
		delete thumb;
		ite.First();
	}

	delete[] ThumbnailMemory;
	ThumbnailMemory=NULL;
}

// ----------------------------------------------------------------------------
ThumbnailManagerClass* ThumbnailManagerClass::Peek_Thumbnail_Manager(const char* thumbnail_filename)
{
	ThumbnailManagerClass* man=ThumbnailManagerList.Head();
	while (man) {
		if (man->ThumbnailFileName==thumbnail_filename) return man;
		man=man->Succ();
	}
	if (GlobalThumbnailManager &&
		GlobalThumbnailManager->ThumbnailFileName==thumbnail_filename) return GlobalThumbnailManager;
	return NULL;
}

// ----------------------------------------------------------------------------
void ThumbnailManagerClass::Add_Thumbnail_Manager(const char* thumbnail_filename)
{
	// First loop over all thumbnail managers to see if we already have this one created. This isn't
	// supposed to be called often at all and there are usually just couple managers alive,
	// so we'll do pure string compares here...

	// Must NOT add global manager with this function
	WWASSERT(stricmp(thumbnail_filename,GLOBAL_THUMBNAIL_MANAGER_FILENAME));

	ThumbnailManagerClass* man=Peek_Thumbnail_Manager(thumbnail_filename);
	if (man) return;

	// Not found, create and add to the list.
	man=new ThumbnailManagerClass(thumbnail_filename);
	ThumbnailManagerList.Add_Tail(man);
}
// ----------------------------------------------------------------------------
void ThumbnailManagerClass::Remove_Thumbnail_Manager(const char* thumbnail_filename)
{
	ThumbnailManagerClass* man=ThumbnailManagerList.Head();
	while (man) {
		if (man->ThumbnailFileName==thumbnail_filename) {
			delete man;
			return;
		}
		man=man->Succ();
	}
	if (GlobalThumbnailManager &&
		GlobalThumbnailManager->ThumbnailFileName==thumbnail_filename) {
		delete GlobalThumbnailManager;
		GlobalThumbnailManager=NULL;
	}
}
// ----------------------------------------------------------------------------
ThumbnailClass* ThumbnailManagerClass::Peek_Thumbnail_Instance(const StringClass& name)
{

	return Get_From_Hash(name);
}

ThumbnailClass* ThumbnailManagerClass::Peek_Thumbnail_Instance_From_Any_Manager(const StringClass& filename)
{
	WWPROFILE(("Peek_Thumbnail_Instance_From_Any_Manager"));
	ThumbnailManagerClass* thumb_man=ThumbnailManagerList.Head();
	while (thumb_man) {
		ThumbnailClass* thumb=thumb_man->Peek_Thumbnail_Instance(filename);
		if (thumb) return thumb;
		thumb_man=thumb_man->Succ();
	}

	if (GlobalThumbnailManager) {
		ThumbnailClass* thumb=GlobalThumbnailManager->Peek_Thumbnail_Instance(filename);
		if (thumb) return thumb;
	}

// If thumbnail is not found, see if we can find a texture. It is possible that the texture is outside of
// a mix file and didn't get included in any thumbnail database based on a mixfile. If so, we'll add it to
// our global thumbnail database.
	if (Is_Thumbnail_Created_If_Not_Found()) {
		if (GlobalThumbnailManager) {
			ThumbnailClass* thumb=new ThumbnailClass(GlobalThumbnailManager,filename);
			if (!thumb->Peek_Bitmap()) {
				delete thumb;
				thumb=NULL;
			}
			return thumb;
		}
	}

	return NULL;
}


void ThumbnailManagerClass::Insert_To_Hash(ThumbnailClass* thumb)
{
	Changed=true;
	StringClass hash_name(0,true);
	Create_Hash_Name(hash_name,thumb->Get_Name());
	ThumbnailHash.Insert(hash_name,thumb);
}

ThumbnailClass* ThumbnailManagerClass::Get_From_Hash(const StringClass& name)
{
	StringClass hash_name(0,true);
	Create_Hash_Name(hash_name,name);
	return ThumbnailHash.Get(hash_name);
}

void ThumbnailManagerClass::Remove_From_Hash(ThumbnailClass* thumb)
{
	Changed=true;
	StringClass hash_name(0,true);
	Create_Hash_Name(hash_name,thumb->Get_Name());
	ThumbnailHash.Remove(hash_name);
}

void ThumbnailManagerClass::Init()
{
	WWASSERT(GlobalThumbnailManager == NULL);
	GlobalThumbnailManager=new ThumbnailManagerClass(GLOBAL_THUMBNAIL_MANAGER_FILENAME);
	GlobalThumbnailManager->Enable_Per_Texture_Time_Stamp(true);
}

void ThumbnailManagerClass::Deinit()
{
	while (ThumbnailManagerClass* man=ThumbnailManagerList.Head()) {
		delete man;
	}

	delete GlobalThumbnailManager;
	GlobalThumbnailManager=NULL;
}
