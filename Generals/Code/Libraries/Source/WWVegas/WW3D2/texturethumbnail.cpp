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

#include "texturethumbnail.h"
#include "hashtemplate.h"
#include "missingtexture.h"
#include "TARGA.H"
#include "ww3dformat.h"
#include "ddsfile.h"
#include "textureloader.h"
#include "bitmaphandler.h"
#include "ffactory.h"

static HashTemplateClass<StringClass,ThumbnailClass*> thumbnail_hash;
static bool _ThumbHashModified;
static unsigned char* _ThumbnailMemory;
static const char *THUMBNAIL_FILENAME = "thumbnails.dat";

ThumbnailClass::ThumbnailClass(const char* name, unsigned char* bitmap, unsigned w, unsigned h, bool allocated)
	:
	Name(name), 
	Bitmap(bitmap), 
	Allocated(allocated),
	Width(w),
	Height(h)
{
	thumbnail_hash.Insert(Name,this);
}

// ----------------------------------------------------------------------------
//
// Load texture and generate mipmap levels if requested. The function tries
// to create texture that matches targa format. If suitable format is not
// available, it selects closest matching format and performs color space
// conversion.
//
// ----------------------------------------------------------------------------

ThumbnailClass::ThumbnailClass(const StringClass& filename)
	:
	Bitmap(0), 
	Name(filename), 
	Allocated(false),
	Width(0),
	Height(0)
{
	unsigned reduction_factor=3;

	// First, try loading image from a DDS file
	DDSFileClass dds_file(filename,reduction_factor);
	if (dds_file.Is_Available() && dds_file.Load()) {
		Width=dds_file.Get_Width(0);
		Height=dds_file.Get_Height(0);
		Bitmap=W3DNEWARRAY unsigned char[Width*Height*4];
		Allocated=true;
		dds_file.Copy_Level_To_Surface(
			0,			// Level
			WW3D_FORMAT_A8R8G8B8, 
			Width, 
			Height, 
			Bitmap, 
			Width*4);
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
		Get_WW3D_Format(dest_format,src_format,src_bpp,targa);

		// Destination size will be the next power of two square from the larger width and height...
		Width=targa.Header.Width>>reduction_factor;
		Height=targa.Header.Height>>reduction_factor;
		TextureLoader::Validate_Texture_Size(Width,Height);
		unsigned src_width=targa.Header.Width;
		unsigned src_height=targa.Header.Height;

		// NOTE: We load the palette but we do not yet support paletted textures!
		char palette[256*4];
		targa.SetPalette(palette);
		if (TARGA_ERROR_HANDLER(targa.Load(filename, TGAF_IMAGE, false),filename)) return;

		unsigned char* src_surface=(unsigned char*)targa.GetImage();

		Bitmap=W3DNEWARRAY unsigned char[Width*Height*4];
		Allocated=true;

		dest_format=WW3D_FORMAT_A8R8G8B8;
		BitmapHandlerClass::Copy_Image(
			Bitmap, 
			Width,
			Height,
			Width*4,
			WW3D_FORMAT_A8R8G8B8,
			src_surface,
			src_width,
			src_height,
			src_width*src_bpp,
			src_format,
			(unsigned char*)targa.GetPalette(),
			targa.Header.CMapDepth>>3,
			false);
	}

	_ThumbHashModified=true;
	thumbnail_hash.Insert(Name,this);
}

ThumbnailClass::~ThumbnailClass()
{
	if (Allocated) delete[] Bitmap;
	thumbnail_hash.Remove(Name);
}

ThumbnailClass* ThumbnailClass::Peek_Instance(const StringClass& name)
{
	return thumbnail_hash.Get(name);
}

void ThumbnailClass::Init()
{
	WWASSERT(!_ThumbnailMemory);

	// If the thumbnail hash table file is available, init hash table
#if 0 // don't do thumbnail file.
	file_auto_ptr thumb_file(_TheFileFactory, THUMBNAIL_FILENAME);
	if (thumb_file->Is_Available()) {
		thumb_file->Open(FileClass::READ);

		char tmp[4];
		thumb_file->Read(tmp,4);
		if (tmp[0]=='T' && tmp[1]=='M' && tmp[2]=='B' && tmp[3]=='1') {

			int total_thumb_count;
			int total_header_length;
			int total_data_length;
			thumb_file->Read(&total_thumb_count,sizeof(int));
			thumb_file->Read(&total_header_length,sizeof(int));
			thumb_file->Read(&total_data_length,sizeof(int));
			if (total_thumb_count) {
				WWASSERT(total_data_length && total_header_length);
				_ThumbnailMemory=W3DNEWARRAY unsigned char[total_data_length];
				// Load thumbs
				for (int i=0;i<total_thumb_count;++i) {
					char name[256];
					int offset;
					int width;
					int height;
					unsigned long date_time;
					int name_len;
					thumb_file->Read(&offset,sizeof(int));
					thumb_file->Read(&width,sizeof(int));
					thumb_file->Read(&height,sizeof(int));
					thumb_file->Read(&date_time,sizeof(unsigned long));
					thumb_file->Read(&name_len,sizeof(int));
					WWASSERT(name_len<255);
					thumb_file->Read(name,name_len);
					name[name_len]='\0';


					// Make sure the file is available and the timestamp matches
					file_auto_ptr myfile(_TheFileFactory,name);	
					if (myfile->Is_Available()) {
						myfile->Open(FileClass::READ);
						if (date_time==myfile->Get_Date_Time()) {
							W3DNEW ThumbnailClass(
								name, 
								_ThumbnailMemory+offset-total_header_length, 
								width,
								height,
								false);
						}
						myfile->Close();
					}
				}
				thumb_file->Read(_ThumbnailMemory,total_data_length);
			}
		}
		thumb_file->Close();
	}
#endif
}

void ThumbnailClass::Deinit()
{
	// If the thumbnail hash table was modified, save it to disk
	HashTemplateIterator<StringClass,ThumbnailClass*> ite(thumbnail_hash);
	if (_ThumbHashModified) {
#if 0 // don't write thumbnails.  jba.
		int total_header_length=0;
		int total_data_length=0;
		int total_thumb_count=0;
		total_header_length+=4;	// header 'TMB1'
		total_header_length+=4;	// thumb count
		total_header_length+=4;	// header size
		total_header_length+=4;	// data length

		for (ite.First();!ite.Is_Done();ite.Next()) {
			total_header_length+=4;	// int bitmap offset
			total_header_length+=4;	// int bitmap width
			total_header_length+=4;	// int bitmap height
			total_header_length+=4;	// unsigned long date_time
			total_header_length+=4;	// int name string length
			ThumbnailClass* thumb=ite.Peek_Value();
			total_header_length+=strlen(thumb->Get_Name());
			total_data_length+=thumb->Get_Width()*thumb->Get_Height()*4;
			total_thumb_count++;
		}
		int offset=total_header_length;

		file_auto_ptr thumb_file(_TheWritingFileFactory, THUMBNAIL_FILENAME);
		if (thumb_file->Is_Available()) {
			thumb_file->Delete();
		}
		thumb_file->Create();
		thumb_file->Open(FileClass::WRITE);

		char* header="TMB1";
		thumb_file->Write(header,4);
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
			unsigned long date_time=0;
			file_auto_ptr myfile(_TheFileFactory,name);	
			if (myfile->Is_Available()) {
				myfile->Open(FileClass::READ);
				date_time=myfile->Get_Date_Time();
				myfile->Close();
			}
			thumb_file->Write(&offset,sizeof(int));
			thumb_file->Write(&width,sizeof(int));
			thumb_file->Write(&height,sizeof(int));
			thumb_file->Write(&date_time,sizeof(unsigned long));
			thumb_file->Write(&name_len,sizeof(int));
			thumb_file->Write(name,name_len);
			offset+=width*height*4;
		}

		// Save bitmaps
		offset=total_header_length;
		for (ite.First();!ite.Is_Done();ite.Next()) {
			ThumbnailClass* thumb=ite.Peek_Value();
			int width=thumb->Get_Width();
			int height=thumb->Get_Height();
			thumb_file->Write(thumb->Peek_Bitmap(),width*height*4);
		}

		thumb_file->Close();							
#endif
	}

	ite.First();
	while (!ite.Is_Done()) {
		ThumbnailClass* thumb=ite.Peek_Value();
		delete thumb;
		ite.First();
	}

	delete [] _ThumbnailMemory;
	_ThumbnailMemory=NULL;
}