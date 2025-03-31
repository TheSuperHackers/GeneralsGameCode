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
#include "TARGA.H"
#include "ww3dformat.h"
#include "ddsfile.h"
#include "textureloader.h"
#include "bitmaphandler.h"
#include "ffactory.h"
#include "RAWFILE.H"
#include "wwprofile.h"
#include <windows.h>

static DLListClass<ThumbnailManagerClass> ThumbnailManagerList;
static ThumbnailManagerClass* GlobalThumbnailManager;
static bool message_box_displayed=false;
bool ThumbnailManagerClass::CreateThumbnailIfNotFound=false;

const char* ThumbFileHeader="THU6";

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
			WWDEBUG_SAY(("Unknown texture format for %s\n",filename));
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

void ThumbnailManagerClass::Create_Thumbnails()
{
#if 0
	SimpleFileFactoryClass ff;
//	ff.Set_Sub_Directory("Data\\");
	ff.Set_Sub_Directory("..\\data\\client\\mixfiles\\");

	MixFileFactoryClass mix(MixFileName, &ff);
	FileFactoryClass* old_file_factory=_TheFileFactory;
	_TheFileFactory=&mix;
	if (mix.Is_Valid()) {
		DynamicVectorClass<StringClass> list;
		list.Set_Growth_Step (1000);
		mix.Build_Filename_List(list);
		for (int i=0;i<list.Count();++i) {
			int len=list[i].Get_Length();
			if (!stricmp(&list[i][len-4],".tga") || !stricmp(&list[i][len-4],".dds")) {
				StringClass tex_name(list[i]);
				if (!Peek_Thumbnail_Instance(tex_name)) {
					new ThumbnailClass(this,tex_name);
				}
				// Remove from Global manager if texture exists in it
				if (GlobalThumbnailManager) {
					ThumbnailClass* g_thumb=GlobalThumbnailManager->Peek_Thumbnail_Instance(tex_name);
					if (g_thumb) {
						delete g_thumb;
					}
				}
			}
		}
	}
	_TheFileFactory=old_file_factory;
#endif
}

void ThumbnailManagerClass::Load()
{
	WWASSERT(!ThumbnailMemory);

	// If the thumbnail hash table file is available, init hash table
#if 0 // don't do thumbnail file.
	DateTime=0;
	SimpleFileFactoryClass ff;
	ff.Set_Sub_Directory("..\\data\\client\\mixfiles\\");
	FileClass* thumb_file=ff.Get_File(ThumbnailFileName);

	if (thumb_file->Is_Available()) {
		thumb_file->Open(FileClass::READ);

		DateTime=thumb_file->Get_Date_Time();

		char tmp[4];
		thumb_file->Read(tmp,4);
		if (tmp[0]==ThumbFileHeader[0] &&
			tmp[1]==ThumbFileHeader[1] &&
			tmp[2]==ThumbFileHeader[2] &&
			tmp[3]==ThumbFileHeader[3]) {

			int total_thumb_count;
			int total_header_length;
			int total_data_length;
			thumb_file->Read(&total_thumb_count,sizeof(int));
			thumb_file->Read(&total_header_length,sizeof(int));
			thumb_file->Read(&total_data_length,sizeof(int));
			if (total_thumb_count) {
				WWASSERT(total_data_length && total_header_length);
				ThumbnailMemory=W3DNEWARRAY unsigned char[total_data_length];
				// Load thumbs
				for (int i=0;i<total_thumb_count;++i) {
					char name[256];
					int offset;
					int width;
					int height;
					int original_width;
					int original_height;
					int original_mip_level_count;
					WW3DFormat original_format;
					int name_len;
					unsigned long date_time;
					thumb_file->Read(&date_time,sizeof(unsigned long));
					thumb_file->Read(&offset,sizeof(int));
					thumb_file->Read(&width,sizeof(int));
					thumb_file->Read(&height,sizeof(int));
					thumb_file->Read(&original_width,sizeof(int));
					thumb_file->Read(&original_height,sizeof(int));
					thumb_file->Read(&original_mip_level_count,sizeof(int));
					thumb_file->Read(&original_format,sizeof(int));
					thumb_file->Read(&name_len,sizeof(int));
					WWASSERT(name_len<255);
					thumb_file->Read(name,name_len);
					name[name_len]='\0';

					// If per-texture time stamp test is enabled, thumbnail is only used if its time stamp
					// matches the texture's time stamp.
					bool valid=true;
					if (Is_Per_Texture_Time_Stamp_Used()) {
						file_auto_ptr texture_file(_TheFileFactory, name);
						if (texture_file->Is_Available()) {
							texture_file->Open();
							if (texture_file->Get_Date_Time()!=date_time) {
								valid=false;
							}
							texture_file->Close();
						}
						else {
							valid=false;
						}
					}

					if (valid) {
						W3DNEW ThumbnailClass(
							this,
							name,
							ThumbnailMemory+offset-total_header_length,
							width,
							height,
							original_width,
							original_height,
							original_mip_level_count,
							original_format,
							false,
							date_time);
					}
				}
				thumb_file->Read(ThumbnailMemory,total_data_length);
			}
		}
		thumb_file->Close();
	}
	Changed=false;
	ff.Return_File(thumb_file);
#endif
}

void ThumbnailManagerClass::Save(bool force)
{
	// Save only if changed
	if (!Changed && !force) return;
	Changed=false;

	// If the thumbnail hash table was modified, save it to disk
#if 0 // don't write thumbnails.  jba.

	HashTemplateIterator<StringClass,ThumbnailClass*> ite(ThumbnailHash);
	int total_header_length=0;
	int total_data_length=0;
	int total_thumb_count=0;
	total_header_length+=4;	// header
	total_header_length+=4;	// thumb count
	total_header_length+=4;	// header size
	total_header_length+=4;	// data length

	for (ite.First();!ite.Is_Done();ite.Next()) {
		ThumbnailClass* thumb=ite.Peek_Value();
		total_header_length+=4;	// per-thumb date-time
		total_header_length+=4;	// int bitmap offset
		total_header_length+=4;	// int bitmap width
		total_header_length+=4;	// int bitmap height
		total_header_length+=4;	// int original bitmap width
		total_header_length+=4;	// int original bitmap height
		total_header_length+=4;	// int original mip level count
		total_header_length+=4;	// int original format
		total_header_length+=4;	// int name string length

		total_header_length+=strlen(thumb->Get_Name());
		total_data_length+=thumb->Get_Width()*thumb->Get_Height()*2;
		total_thumb_count++;
	}
	int offset=total_header_length;

	SimpleFileFactoryClass ff;
	ff.Set_Sub_Directory("..\\data\\client\\mixfiles\\");
	FileClass* thumb_file=ff.Get_File(ThumbnailFileName);
//	file_auto_ptr thumb_file(_TheWritingFileFactory, ThumbnailFileName);

	if (thumb_file->Is_Available()) {
		thumb_file->Delete();
	}
	thumb_file->Create();
	thumb_file->Open(FileClass::WRITE);

	thumb_file->Write(ThumbFileHeader,4);
	thumb_file->Write(&total_thumb_count,sizeof(int));
	thumb_file->Write(&total_header_length,sizeof(int));
	thumb_file->Write(&total_data_length,sizeof(int));

	// Save names and offsets
	for (ite.First();!ite.Is_Done();ite.Next()) {
		ThumbnailClass* thumb=ite.Peek_Value();
		const char* name=thumb->Get_Name();
		int name_len=strlen(name);
		int width=thumb->Get_Width();
		int height=thumb->Get_Height();
		int original_width=thumb->Get_Original_Texture_Width();
		int original_height=thumb->Get_Original_Texture_Height();
		int original_mip_level_count=thumb->Get_Original_Texture_Mip_Level_Count();
		WW3DFormat original_format=thumb->Get_Original_Texture_Format();
		unsigned long date_time=thumb->Get_Date_Time();

		thumb_file->Write(&date_time,sizeof(unsigned long));
		thumb_file->Write(&offset,sizeof(int));
		thumb_file->Write(&width,sizeof(int));
		thumb_file->Write(&height,sizeof(int));
		thumb_file->Write(&original_width,sizeof(int));
		thumb_file->Write(&original_height,sizeof(int));
		thumb_file->Write(&original_mip_level_count,sizeof(int));
		thumb_file->Write(&original_format,sizeof(int));
		thumb_file->Write(&name_len,sizeof(int));
		thumb_file->Write(name,name_len);
		offset+=width*height*2;
	}

	// Save bitmaps
	offset=total_header_length;
	for (ite.First();!ite.Is_Done();ite.Next()) {
		ThumbnailClass* thumb=ite.Peek_Value();
		int width=thumb->Get_Width();
		int height=thumb->Get_Height();
		thumb_file->Write(thumb->Peek_Bitmap(),width*height*2);
	}
	if (DateTime) thumb_file->Set_Date_Time(DateTime);
	thumb_file->Close();
	ff.Return_File(thumb_file);
#endif
}


// ----------------------------------------------------------------------------
ThumbnailManagerClass::ThumbnailManagerClass(const char* thumbnail_filename, const char* mix_filename)
	:
	ThumbnailMemory(NULL),
	ThumbnailFileName(thumbnail_filename),
	MixFileName(mix_filename),
	PerTextureTimeStampUsed(false),
	Changed(false),
	DateTime(0)
{
	Load();
}

// ----------------------------------------------------------------------------
ThumbnailManagerClass::~ThumbnailManagerClass()
{
	Save();
	HashTemplateIterator<StringClass,ThumbnailClass*> ite(ThumbnailHash);
	ite.First();
	while (!ite.Is_Done()) {
		ThumbnailClass* thumb=ite.Peek_Value();
		delete thumb;
		ite.First();
	}

	if (ThumbnailMemory) delete[] ThumbnailMemory;
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
void ThumbnailManagerClass::Add_Thumbnail_Manager(const char* thumbnail_filename, const char* mix_filename)
{
	// First loop over all thumbnail managers to see if we already have this one created. This isn't
	// supposed to be called often at all and there are usually just couple managers alive,
	// so we'll do pure string compares here...

	// Must NOT add global manager with this function
	WWASSERT(stricmp(thumbnail_filename,GLOBAL_THUMBNAIL_MANAGER_FILENAME));

	ThumbnailManagerClass* man=Peek_Thumbnail_Manager(thumbnail_filename);
	if (man) return;

	// Always update thumbnail files when they're out of date
	Update_Thumbnail_File(mix_filename,false);

	// Not found, create and add to the list.
	man=new ThumbnailManagerClass(thumbnail_filename,mix_filename);
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

void ThumbnailManagerClass::Update_Thumbnail_File(const char* mix_file_name,bool display_message_box)
{
	if (!mix_file_name) return;

	SimpleFileFactoryClass ff;
	ff.Set_Sub_Directory("..\\data\\client\\mixfiles\\");

	StringClass thumb_file_name(mix_file_name,true);
	int len=thumb_file_name.Get_Length();
	WWASSERT(len>4);
	thumb_file_name[len-3]='t';
	thumb_file_name[len-2]='h';
	thumb_file_name[len-1]='6';
	FileClass* mix_file=ff.Get_File(mix_file_name);
	FileClass* thumb_file=ff.Get_File(thumb_file_name);

	WWASSERT(mix_file && thumb_file);

	// If mix file isn't found but thumb file is, delete the obsolete thumb file
	mix_file->Open(FileClass::READ);
	thumb_file->Open(FileClass::READ);		//|FileClass::WRITE);	Changed this to READ only since we never use the handle for writing and it may cause contention amongst slave servers ST - 12/14/2001 8:26PM
	if (!mix_file->Is_Available()) {
		if (thumb_file->Is_Available()) {
			thumb_file->Delete();
		}
		mix_file->Close();
		thumb_file->Close();
		ff.Return_File(mix_file);
		ff.Return_File(thumb_file);
		return;
	}

	unsigned long mix_date_time=mix_file->Get_Date_Time();
	if (thumb_file->Is_Available()) {
		unsigned long thumb_date_time=thumb_file->Get_Date_Time();
		if (mix_date_time!=thumb_date_time) {
			thumb_file->Delete();
		}
		// Read header to make sure the thumb file is of correct version.
		char tmp[4];
		thumb_file->Read(tmp,4);
		if (tmp[0]!=ThumbFileHeader[0] ||
			tmp[1]!=ThumbFileHeader[1] ||
			tmp[2]!=ThumbFileHeader[2] ||
			tmp[3]!=ThumbFileHeader[3]) {
			thumb_file->Delete();
		}
	}

	if (thumb_file->Is_Available()) {
		mix_file->Close();
		thumb_file->Close();
		ff.Return_File(mix_file);
		ff.Return_File(thumb_file);
		return;
	}

	if (display_message_box && !message_box_displayed) {
		message_box_displayed=true;
		::MessageBox(NULL,
			"Some or all texture thumbnails need to be updated.\n"
			"This will take a while. The update will only be done once\n"
			"each time a mix file changes and thumb database hasn't been\n"
			"updated.",
			"Updating texture thumbnails",
			MB_OK);
	}

	// we don't currently have a thumbnail file (either we just deleted it or it never existed, we don't care)
	// so we must create one now.
	ThumbnailManagerClass* manager=new ThumbnailManagerClass(thumb_file_name, mix_file_name);
	manager->DateTime=mix_date_time;	// Set the datetime to mixfile's datetime.
	manager->Create_Thumbnails();
	manager->Save(true);
	delete manager;

	// close files and return pointers
	mix_file->Close();
	thumb_file->Close();
	ff.Return_File(mix_file);
	ff.Return_File(thumb_file);
}

// Verify that up-to-date thumbnails exist for all textures
void ThumbnailManagerClass::Pre_Init(bool display_message_box)
{
	WWASSERT(!ThumbnailManagerList.Head());

	// Collect all mix file names
	DynamicVectorClass<StringClass> mix_names;

	char cur_dir[256];
	GetCurrentDirectory(sizeof(cur_dir),cur_dir);
	StringClass new_dir(cur_dir,true);
	new_dir+="\\Data";
	SetCurrentDirectory(new_dir);

	WIN32_FIND_DATA find_data;
	HANDLE handle=FindFirstFile("*.mix",&find_data);
	if (handle!=INVALID_HANDLE_VALUE) {
		for (;;) {
			if (!(find_data.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)) {
				mix_names.Add(find_data.cFileName);
			}
			if (!FindNextFile(handle,&find_data)) {
				FindClose(handle);
				break;
			}
		}
	}
	SetCurrentDirectory(cur_dir);

	// First generate thumbnails for always.dat
	Update_Thumbnail_File("always.dat",display_message_box);

	// Then loop over all .mix files
	for (int i=0;i<mix_names.Count();++i) {
		Update_Thumbnail_File(mix_names[i],display_message_box);
	}
}

void ThumbnailManagerClass::Init()
{
	WWASSERT(GlobalThumbnailManager == NULL);
	GlobalThumbnailManager=new ThumbnailManagerClass(GLOBAL_THUMBNAIL_MANAGER_FILENAME,NULL);
	GlobalThumbnailManager->Enable_Per_Texture_Time_Stamp(true);
}

void ThumbnailManagerClass::Deinit()
{
	while (ThumbnailManagerClass* man=ThumbnailManagerList.Head()) {
		delete man;
	}
	if (GlobalThumbnailManager) {
		delete GlobalThumbnailManager;
		GlobalThumbnailManager=NULL;
	}
}
