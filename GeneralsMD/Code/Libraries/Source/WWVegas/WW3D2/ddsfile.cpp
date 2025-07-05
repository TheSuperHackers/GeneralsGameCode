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

// 08/06/02 KM Added cube map and volume texture support
#include "ddsfile.h"
#include "ffactory.h"
#include "bufffile.h"
#include "formconv.h"
#include "dx8wrapper.h"
#include "bitmaphandler.h"
#include "colorspace.h"
#include <string.h>
#include <ddraw.h>

// ----------------------------------------------------------------------------

DDSFileClass::DDSFileClass(const char* name,unsigned reduction_factor)
	:
	DDSMemory(NULL),
	Width(0),
	Height(0),
	Depth(0),
	FullWidth(0),
	FullHeight(0),
	FullDepth(0),
	LevelSizes(NULL),
	LevelOffsets(NULL),
	MipLevels(0),
	ReductionFactor(reduction_factor),
	Format(WW3D_FORMAT_UNKNOWN),
	Type(DDS_TEXTURE),
	DateTime(0),
	CubeFaceSize(0)
{
	strncpy(Name,name,sizeof(Name));
	// The name could be given in .tga or .dds format, so ensure we're opening .dds...
	int len=strlen(Name);
	Name[len-3]='d';
	Name[len-2]='d';
	Name[len-1]='s';

	file_auto_ptr file(_TheFileFactory,Name);	
	if (!file->Is_Available()) 
	{
		return;
	}

	int result=file->Open();
	if (!result)
	{
		WWASSERT("File would not open\n");
		return;
	}

	DateTime=file->Get_Date_Time();
	char header[4];

	unsigned read_bytes=file->Read(header,4);
	if (!read_bytes)
	{
		WWASSERT("File loading failed trying to read header\n");
		return;
	}
	// Now, we read DDSURFACEDESC2 defining the compressed data
	read_bytes=file->Read(&SurfaceDesc,sizeof(LegacyDDSURFACEDESC2));
	// Verify the structure size matches the read size
	if (read_bytes==0 || read_bytes!=SurfaceDesc.Size) 
	{
		StringClass tmp(0,true);
		tmp.Format("File %s loading failed.\nTried to read %d bytes, got %d. (SurfDesc.size=%d)\n",name,sizeof(LegacyDDSURFACEDESC2),read_bytes,SurfaceDesc.Size);
		WWASSERT_PRINT(0,tmp.str());
		return;
	}

	Format=D3DFormat_To_WW3DFormat((D3DFORMAT)SurfaceDesc.PixelFormat.FourCC);
	WWASSERT(
		Format==WW3D_FORMAT_DXT1 ||
		Format==WW3D_FORMAT_DXT2 ||
		Format==WW3D_FORMAT_DXT3 ||
		Format==WW3D_FORMAT_DXT4 ||
		Format==WW3D_FORMAT_DXT5);

	MipLevels=SurfaceDesc.MipMapCount;
	if (MipLevels==0) MipLevels=1;

	if (MipLevels>ReductionFactor) MipLevels-=ReductionFactor;
	else {
		MipLevels=1;
		ReductionFactor=ReductionFactor-MipLevels;
	}

	// Drop the two lowest miplevels!
	if (MipLevels>2) MipLevels-=2;
	else MipLevels=1;

	// check texture type, normal, cube or volume
	if (SurfaceDesc.Caps.Caps2&DDSCAPS2_CUBEMAP)
	{
		Type=DDS_CUBEMAP;
	}
	else if (SurfaceDesc.Caps.Caps2&DDSCAPS2_VOLUME)
	{
		Type=DDS_VOLUME;
	}


	FullWidth=SurfaceDesc.Width;
	FullHeight=SurfaceDesc.Height;
	FullDepth=SurfaceDesc.Depth;
	Width=SurfaceDesc.Width>>ReductionFactor;
	Height=SurfaceDesc.Height>>ReductionFactor;
	Depth=SurfaceDesc.Depth;

	unsigned level_size=Calculate_DXTC_Surface_Size
	(
		SurfaceDesc.Width,
		SurfaceDesc.Height,
		Format
	);
	unsigned level_offset=0;

	unsigned level_mip_dec=4;
	if (Type==DDS_VOLUME)
	{
		// add slices to level data size
		level_size*=SurfaceDesc.Depth;
		level_mip_dec=8;
	}

	LevelSizes=W3DNEWARRAY unsigned[MipLevels];
	LevelOffsets=W3DNEWARRAY unsigned[MipLevels];
	unsigned level=0;
	for (;level<ReductionFactor;++level) 
	{
		if (level_size>16) 
		{	// If surface is bigger than one block (8 or 16 bytes)...
			level_size/=level_mip_dec;
		}
	}
	for (level=0;level<MipLevels;++level) 
	{
		LevelSizes[level]=level_size;
		LevelOffsets[level]=level_offset;
		level_offset+=level_size;
		if (level_size>16) 
		{	// If surface is bigger than one block (8 or 16 bytes)...
			level_size/=level_mip_dec;
		}
	}

	if (Type==DDS_CUBEMAP)
	{
		for (level=0; level<MipLevels;++level)
		{
			CubeFaceSize+=LevelSizes[level];
		}

		// this accounts for dropping 2 lowest mip levels
		if (MipLevels>2)
			CubeFaceSize+=16;
	}
	file->Close();
}

// ----------------------------------------------------------------------------

DDSFileClass::~DDSFileClass()
{
	delete[] DDSMemory;
	delete[] LevelSizes;
	delete[] LevelOffsets;
}

unsigned DDSFileClass::Get_Width(unsigned level) const
{
	WWASSERT(level<MipLevels); 
	unsigned width=Width>>level;
	if (width<4) width=4;
	return width;
}

unsigned DDSFileClass::Get_Height(unsigned level) const
{
	WWASSERT(level<MipLevels); 
	unsigned height=Height>>level;
	if (height<4) height=4;
	return height;
}

unsigned DDSFileClass::Get_Depth(unsigned level) const
{
	WWASSERT(level<MipLevels);
	unsigned depth=Depth>>level;
	if (depth<4) depth=4;
	return depth;
}

const unsigned char* DDSFileClass::Get_Memory_Pointer(unsigned level) const
{
	WWASSERT(level<MipLevels); 
	return DDSMemory+LevelOffsets[level];
}

unsigned DDSFileClass::Get_Level_Size(unsigned level) const
{
	WWASSERT(level<MipLevels); 
	return LevelSizes[level];
}

// For some reason DX-Tex tool doesn't fill the surface size field, so we need to calculate it...
unsigned DDSFileClass::Calculate_DXTC_Surface_Size
(
	unsigned width, 
	unsigned height, 
	WW3DFormat format
)
{
	unsigned level_size=(width/4)*(height/4);
	switch (format) 
	{
	case WW3D_FORMAT_DXT1:
		level_size*=8;
		break;
	case WW3D_FORMAT_DXT2:
	case WW3D_FORMAT_DXT3:
	case WW3D_FORMAT_DXT4:
	case WW3D_FORMAT_DXT5:
		level_size*=16;
		break;
	}

	return level_size;
}

// ----------------------------------------------------------------------------

bool DDSFileClass::Load()
{
	if (DDSMemory) return false;
	if (!LevelSizes || !LevelOffsets) return false;

	file_auto_ptr file(_TheFileFactory,Name);	
	if (!file->Is_Available()) 
	{
		return false;
	}

	file->Open();
	// Data size is file size minus the header and info block
	unsigned size=file->Size()-SurfaceDesc.Size-4;

	if (!size)
	{
		return false;
	}

	// Skip mip levels if reduction factor is not zero
	unsigned level_size=Calculate_DXTC_Surface_Size
	(
		SurfaceDesc.Width,
		SurfaceDesc.Height,
		Format
	);

	unsigned skipped_offset=0;
	for (unsigned i=0;i<ReductionFactor;++i) 
	{
		skipped_offset+=level_size;
		size-=level_size;
		if (level_size>16) 
		{	// If surface is bigger than one block (8 or 16 bytes)...
			level_size/=4;
		}
	}

	// Skip the header and info block and possible unused mip levels
	unsigned seek_size=file->Seek(SurfaceDesc.Size+4+skipped_offset);
	WWASSERT(seek_size==(SurfaceDesc.Size+4+skipped_offset));

	if (size && size<0x80000000) 
	{
		// Allocate memory for the data excluding the headers
		DDSMemory=MSGW3DNEWARRAY("DDSMemory") unsigned char[size];
		// Read data
		unsigned read_size=file->Read(DDSMemory,size);
		// Verify we got all the data
		WWASSERT(read_size==size);
	}
	file->Close();
	return true;
}

// ----------------------------------------------------------------------------

WWINLINE static unsigned RGB565_To_ARGB8888(unsigned short rgb)
{
	unsigned rgba=0;
	rgba|=unsigned(rgb&0x001f)<<3;
	rgba|=unsigned(rgb&0x07e0)<<5;
	rgba|=unsigned(rgb&0xf800)<<8;
	return rgba;
}

WWINLINE static unsigned short ARGB8888_To_RGB565(unsigned argb_)
{
	unsigned char* argb=(unsigned char*)&argb_;
	unsigned short rgb;
	rgb=((argb[2])&0xf8)<<8;
	rgb|=((argb[1])&0xfc)<<3;
	rgb|=((argb[0])&0xf8)>>3;
	return rgb;
}


// ----------------------------------------------------------------------------
//
// Copy mipmap level to D3D surface. The copying is performed using another
// Copy_Level_To_Surface function (see below).
//
// ----------------------------------------------------------------------------

void DDSFileClass::Copy_Level_To_Surface(unsigned level,IDirect3DSurface8* d3d_surface,const Vector3& hsv_shift)
{
	WWASSERT(d3d_surface);
	// Verify that the destination surface size matches the source surface size
	D3DSURFACE_DESC surface_desc;
	DX8_ErrorCode(d3d_surface->GetDesc(&surface_desc));

	// First lock the surface
	D3DLOCKED_RECT locked_rect;
	DX8_ErrorCode(d3d_surface->LockRect(&locked_rect,NULL,0));

	Copy_Level_To_Surface(
		level,
		D3DFormat_To_WW3DFormat(surface_desc.Format),
		surface_desc.Width,
		surface_desc.Height,
		reinterpret_cast<unsigned char*>(locked_rect.pBits),
		locked_rect.Pitch,
		hsv_shift);

	// Finally, unlock the surface
	DX8_ErrorCode(d3d_surface->UnlockRect());
}

// ----------------------------------------------------------------------------
//
// Copy one mipmap level of texture to a memory surface. Surface type conversion
// is performed if the destination is of different format. Scaling will be done
// one of these days as well. Conversions between different types of compressed
// surfaces are not performed and scaling of compressed surfaces is also not
// possible.
//
// ----------------------------------------------------------------------------

void DDSFileClass::Copy_Level_To_Surface
(
	unsigned level,
	WW3DFormat dest_format, 
	unsigned dest_width, 
	unsigned dest_height, 
	unsigned char* dest_surface, 
	unsigned dest_pitch,
	const Vector3& hsv_shift
)
{
	WWASSERT(DDSMemory);
	WWASSERT(dest_surface);

	if (!DDSMemory || !Get_Memory_Pointer(level))
	{
		WWASSERT_PRINT(DDSMemory,"Surface mip level pointer is missing");
		return;
	}

	// If the format and size is a match just copy the contents
	bool has_hsv_shift = hsv_shift[0]!=0.0f || hsv_shift[1]!=0.0f || hsv_shift[2]!=0.0f;
	if (dest_format==Format && dest_width==Get_Width(level) && dest_height==Get_Height(level)) {
		// If hue shift, we can't just copy...
		if (has_hsv_shift) {
			if (Format==WW3D_FORMAT_DXT1) {
				const unsigned* src_ptr=reinterpret_cast<const unsigned*>(Get_Memory_Pointer(level));
				unsigned* dest_ptr=reinterpret_cast<unsigned*>(dest_surface);
				for (unsigned y=0;y<dest_height;y+=4) {
					for (unsigned x=0;x<dest_width;x+=4) {
						unsigned cols=*src_ptr++;		// Bytes 1-4 of color block
						unsigned col0=RGB565_To_ARGB8888((unsigned short)(cols>>16));
						unsigned col1=RGB565_To_ARGB8888((unsigned short)(cols&0xffff));
						Recolor(col0,hsv_shift);
						Recolor(col1,hsv_shift);
						col0=ARGB8888_To_RGB565(col0);
						col1=ARGB8888_To_RGB565(col1);
						cols=unsigned(col0)<<16|col1;
						*dest_ptr++=cols;

						*dest_ptr++=*src_ptr++;		// Bytes 5-8 of color block
					}
				}
			}
			else if (Format==WW3D_FORMAT_DXT5) {
				const unsigned* src_ptr=reinterpret_cast<const unsigned*>(Get_Memory_Pointer(level));
				unsigned* dest_ptr=reinterpret_cast<unsigned*>(dest_surface);
				for (unsigned y=0;y<dest_height;y+=4) {
					for (unsigned x=0;x<dest_width;x+=4) {
						*dest_ptr++=*src_ptr++;		// Bytes 1-4 of alpha block
						*dest_ptr++=*src_ptr++;		// Bytes 5-8 of alpha block
						unsigned cols=*src_ptr++;		// Bytes 1-4 of color block
						unsigned col0=RGB565_To_ARGB8888((unsigned short)(cols>>16));
						unsigned col1=RGB565_To_ARGB8888((unsigned short)(cols&0xffff));
						Recolor(col0,hsv_shift);
						Recolor(col1,hsv_shift);
						col0=ARGB8888_To_RGB565(col0);
						col1=ARGB8888_To_RGB565(col1);
						cols=unsigned(col0)<<16|col1;
						*dest_ptr++=cols;

						*dest_ptr++=*src_ptr++;		// Bytes 5-8 of color block
					}
				}
			}
			else {
				WWASSERT(0);
			}

		}
		else {
			unsigned compressed_size=Get_Level_Size(level);
			memcpy(dest_surface,Get_Memory_Pointer(level),compressed_size);
		}
	}
	else {
		// If size matches, copy each pixel linearly with color space conversion
		if (dest_width==Get_Width(level) && dest_height==Get_Height(level)) {
			// An exception here - if the source format is DXT1 and the destination
			// is DXT2, just copy the contents and create an empty alpha channel.
			// This is needed on NVidia cards that have problems with DXT1 compression.
			if (Format==WW3D_FORMAT_DXT1 && dest_format==WW3D_FORMAT_DXT2) {
				// If hue shift, we can't just copy...
				if (has_hsv_shift) {
					const unsigned* src_ptr=reinterpret_cast<const unsigned*>(Get_Memory_Pointer(level));
					unsigned* dest_ptr=reinterpret_cast<unsigned*>(dest_surface);
					for (unsigned y=0;y<dest_height;y+=4) {
						for (unsigned x=0;x<dest_width;x+=4) {
							*dest_ptr++=0xffffffff;		// Bytes 1-4 of alpha block
							*dest_ptr++=0xffffffff;		// Bytes 5-8 of alpha block
//							*dest_ptr++=*src_ptr++;		// Bytes 1-4 of color block

							unsigned cols=*src_ptr++;	// Bytes 1-4 of color block
							unsigned col0=RGB565_To_ARGB8888((unsigned short)(cols>>16));
							unsigned col1=RGB565_To_ARGB8888((unsigned short)(cols&0xffff));
							Recolor(col0,hsv_shift);
							Recolor(col1,hsv_shift);
							col0=ARGB8888_To_RGB565(col0);
							col1=ARGB8888_To_RGB565(col1);
							cols=unsigned(col0)<<16|col1;
							*dest_ptr++=cols;

							*dest_ptr++=*src_ptr++;		// Bytes 5-8 of color block
						}
					}
				}
				else {
					const unsigned* src_ptr=reinterpret_cast<const unsigned*>(Get_Memory_Pointer(level));
					unsigned* dest_ptr=reinterpret_cast<unsigned*>(dest_surface);
					for (unsigned y=0;y<dest_height;y+=4) {
						for (unsigned x=0;x<dest_width;x+=4) {
							*dest_ptr++=0xffffffff;		// Bytes 1-4 of alpha block
							*dest_ptr++=0xffffffff;		// Bytes 5-8 of alpha block
							*dest_ptr++=*src_ptr++;		// Bytes 1-4 of color block
							*dest_ptr++=*src_ptr++;		// Bytes 5-8 of color block
						}
					}
				}
			}
			else {
				unsigned dest_bpp=Get_Bytes_Per_Pixel(dest_format);

				// Copy 4x4 block at a time
				bool contains_alpha=false;
				for (unsigned y=0;y<dest_height;y+=4) {
					unsigned char* dest_ptr=dest_surface;
					dest_ptr+=y*dest_pitch;
					for (unsigned x=0;x<dest_width;x+=4,dest_ptr+=dest_bpp*4) {
						contains_alpha|=Get_4x4_Block(dest_ptr,dest_pitch,dest_format,level,x,y,hsv_shift);
					}
				}
				if (Format==WW3D_FORMAT_DXT1 && contains_alpha) {
					WWDEBUG_SAY(("Warning: DXT1 format should not contain alpha information - file %s",Name));
				}
			}
		}
	}
}

// cube map
const unsigned char* DDSFileClass::Get_CubeMap_Memory_Pointer
(
	unsigned int face,
	unsigned int level
) const
{
	return &DDSMemory[CubeFaceSize*face+LevelOffsets[level]];
}


void DDSFileClass::Copy_CubeMap_Level_To_Surface
(
	unsigned face,
	unsigned level,
	WW3DFormat dest_format,
	unsigned dest_width,
	unsigned dest_height,
	unsigned char* dest_surface,
	unsigned dest_pitch,
	const Vector3& hsv_shift
)
{
	WWASSERT(DDSMemory);
	WWASSERT(dest_surface);

	if (!DDSMemory || !Get_CubeMap_Memory_Pointer(face,level))
	{
		WWASSERT_PRINT(DDSMemory,"Surface mip level pointer is missing");
		return;
	}

	// If the format and size is a match just copy the contents
	bool has_hsv_shift = hsv_shift[0]!=0.0f || hsv_shift[1]!=0.0f || hsv_shift[2]!=0.0f;
	if (dest_format==Format && dest_width==Get_Width(level) && dest_height==Get_Height(level)) 
	{
		// If hue shift, we can't just copy...
		if (has_hsv_shift) 
		{
			if (Format==WW3D_FORMAT_DXT1) 
			{
				const unsigned* src_ptr=reinterpret_cast<const unsigned*>(Get_CubeMap_Memory_Pointer(face,level));
				unsigned* dest_ptr=reinterpret_cast<unsigned*>(dest_surface);
				for (unsigned y=0;y<dest_height;y+=4) 
				{
					for (unsigned x=0;x<dest_width;x+=4) 
					{
						unsigned cols=*src_ptr++;		// Bytes 1-4 of color block
						unsigned col0=RGB565_To_ARGB8888((unsigned short)(cols>>16));
						unsigned col1=RGB565_To_ARGB8888((unsigned short)(cols&0xffff));
						Recolor(col0,hsv_shift);
						Recolor(col1,hsv_shift);
						col0=ARGB8888_To_RGB565(col0);
						col1=ARGB8888_To_RGB565(col1);
						cols=unsigned(col0)<<16|col1;
						*dest_ptr++=cols;

						*dest_ptr++=*src_ptr++;		// Bytes 5-8 of color block
					}
				}
			}
			else if (Format==WW3D_FORMAT_DXT5) 
			{
				const unsigned* src_ptr=reinterpret_cast<const unsigned*>(Get_CubeMap_Memory_Pointer(face,level));
				unsigned* dest_ptr=reinterpret_cast<unsigned*>(dest_surface);
				for (unsigned y=0;y<dest_height;y+=4) 
				{
					for (unsigned x=0;x<dest_width;x+=4) 
					{
						*dest_ptr++=*src_ptr++;		// Bytes 1-4 of alpha block
						*dest_ptr++=*src_ptr++;		// Bytes 5-8 of alpha block
						unsigned cols=*src_ptr++;		// Bytes 1-4 of color block
						unsigned col0=RGB565_To_ARGB8888((unsigned short)(cols>>16));
						unsigned col1=RGB565_To_ARGB8888((unsigned short)(cols&0xffff));
						Recolor(col0,hsv_shift);
						Recolor(col1,hsv_shift);
						col0=ARGB8888_To_RGB565(col0);
						col1=ARGB8888_To_RGB565(col1);
						cols=unsigned(col0)<<16|col1;
						*dest_ptr++=cols;

						*dest_ptr++=*src_ptr++;		// Bytes 5-8 of color block
					}
				}
			}
			else 
			{
				WWASSERT(0);
			}

		}
		else
		{
			unsigned compressed_size=Get_Level_Size(level);
			memcpy(dest_surface,Get_CubeMap_Memory_Pointer(face,level),compressed_size);
		}
	}
	else 
	{
		// If size matches, copy each pixel linearly with color space conversion
		if (dest_width==Get_Width(level) && dest_height==Get_Height(level)) 
		{
			// An exception here - if the source format is DXT1 and the destination
			// is DXT2, just copy the contents and create an empty alpha channel.
			// This is needed on NVidia cards that have problems with DXT1 compression.
			if (Format==WW3D_FORMAT_DXT1 && dest_format==WW3D_FORMAT_DXT2) 
			{
				// If hue shift, we can't just copy...
				if (has_hsv_shift) 
				{
					const unsigned* src_ptr=reinterpret_cast<const unsigned*>(Get_CubeMap_Memory_Pointer(face,level));
					unsigned* dest_ptr=reinterpret_cast<unsigned*>(dest_surface);
					for (unsigned y=0;y<dest_height;y+=4) 
					{
						for (unsigned x=0;x<dest_width;x+=4) 
						{
							*dest_ptr++=0xffffffff;		// Bytes 1-4 of alpha block
							*dest_ptr++=0xffffffff;		// Bytes 5-8 of alpha block
//							*dest_ptr++=*src_ptr++;		// Bytes 1-4 of color block

							unsigned cols=*src_ptr++;	// Bytes 1-4 of color block
							unsigned col0=RGB565_To_ARGB8888((unsigned short)(cols>>16));
							unsigned col1=RGB565_To_ARGB8888((unsigned short)(cols&0xffff));
							Recolor(col0,hsv_shift);
							Recolor(col1,hsv_shift);
							col0=ARGB8888_To_RGB565(col0);
							col1=ARGB8888_To_RGB565(col1);
							cols=unsigned(col0)<<16|col1;
							*dest_ptr++=cols;

							*dest_ptr++=*src_ptr++;		// Bytes 5-8 of color block
						}
					}
				}
				else 
				{
					const unsigned* src_ptr=reinterpret_cast<const unsigned*>(Get_CubeMap_Memory_Pointer(face,level));
					unsigned* dest_ptr=reinterpret_cast<unsigned*>(dest_surface);
					for (unsigned y=0;y<dest_height;y+=4) 
					{
						for (unsigned x=0;x<dest_width;x+=4) 
						{
							*dest_ptr++=0xffffffff;		// Bytes 1-4 of alpha block
							*dest_ptr++=0xffffffff;		// Bytes 5-8 of alpha block
							*dest_ptr++=*src_ptr++;		// Bytes 1-4 of color block
							*dest_ptr++=*src_ptr++;		// Bytes 5-8 of color block
						}
					}
				}
			}
			else 
			{
				unsigned dest_bpp=Get_Bytes_Per_Pixel(dest_format);

				// Copy 4x4 block at a time
				bool contains_alpha=false;
				for (unsigned y=0;y<dest_height;y+=4) 
				{
					unsigned char* dest_ptr=dest_surface;
					dest_ptr+=y*dest_pitch;
					for (unsigned x=0;x<dest_width;x+=4,dest_ptr+=dest_bpp*4) 
					{
						contains_alpha|=Get_4x4_Block(dest_ptr,dest_pitch,dest_format,level,x,y,hsv_shift);
					}
				}
				if (Format==WW3D_FORMAT_DXT1 && contains_alpha) 
				{
					WWDEBUG_SAY(("Warning: DXT1 format should not contain alpha information - file %s",Name));
				}
			}
		}
	}
}

// volume texture copy
const unsigned char* DDSFileClass::Get_Volume_Memory_Pointer(unsigned int level)  const
{
	return NULL;//DDSMemory[
}

void DDSFileClass::Copy_Volume_Level_To_Surface
(
	unsigned level,
	unsigned depth,
	WW3DFormat dest_format,
	unsigned dest_width,
	unsigned dest_height,
	unsigned char* dest_surface,
	unsigned row_pitch,
	unsigned slice_pitch,
	const Vector3& hsv_shift
)
{
	WWASSERT(DDSMemory);
	WWASSERT(dest_surface);

	if (!DDSMemory || !Get_Volume_Memory_Pointer(level))
	{
		WWASSERT_PRINT(DDSMemory,"Surface mip level pointer is missing");
		return;
	}

	// get 'dest_surface'


	// If the format and size is a match just copy the contents
	bool has_hsv_shift = hsv_shift[0]!=0.0f || hsv_shift[1]!=0.0f || hsv_shift[2]!=0.0f;
	if (dest_format==Format && dest_width==Get_Width(level) && dest_height==Get_Height(level)) 
	{
		// If hue shift, we can't just copy...
		if (has_hsv_shift) 
		{
			if (Format==WW3D_FORMAT_DXT1) 
			{
				const unsigned* src_ptr=reinterpret_cast<const unsigned*>(Get_Volume_Memory_Pointer(level));
				unsigned* dest_ptr=reinterpret_cast<unsigned*>(dest_surface);
				for (unsigned y=0;y<dest_height;y+=4) 
				{
					for (unsigned x=0;x<dest_width;x+=4) 
					{
						unsigned cols=*src_ptr++;		// Bytes 1-4 of color block
						unsigned col0=RGB565_To_ARGB8888((unsigned short)(cols>>16));
						unsigned col1=RGB565_To_ARGB8888((unsigned short)(cols&0xffff));
						Recolor(col0,hsv_shift);
						Recolor(col1,hsv_shift);
						col0=ARGB8888_To_RGB565(col0);
						col1=ARGB8888_To_RGB565(col1);
						cols=unsigned(col0)<<16|col1;
						*dest_ptr++=cols;

						*dest_ptr++=*src_ptr++;		// Bytes 5-8 of color block
					}
				}
			}
			else if (Format==WW3D_FORMAT_DXT5) 
			{
				const unsigned* src_ptr=reinterpret_cast<const unsigned*>(Get_Volume_Memory_Pointer(level));
				unsigned* dest_ptr=reinterpret_cast<unsigned*>(dest_surface);
				for (unsigned y=0;y<dest_height;y+=4) 
				{
					for (unsigned x=0;x<dest_width;x+=4) 
					{
						*dest_ptr++=*src_ptr++;		// Bytes 1-4 of alpha block
						*dest_ptr++=*src_ptr++;		// Bytes 5-8 of alpha block
						unsigned cols=*src_ptr++;		// Bytes 1-4 of color block
						unsigned col0=RGB565_To_ARGB8888((unsigned short)(cols>>16));
						unsigned col1=RGB565_To_ARGB8888((unsigned short)(cols&0xffff));
						Recolor(col0,hsv_shift);
						Recolor(col1,hsv_shift);
						col0=ARGB8888_To_RGB565(col0);
						col1=ARGB8888_To_RGB565(col1);
						cols=unsigned(col0)<<16|col1;
						*dest_ptr++=cols;

						*dest_ptr++=*src_ptr++;		// Bytes 5-8 of color block
					}
				}
			}
			else 
			{
				WWASSERT(0);
			}

		}
		else 
		{
			unsigned compressed_size=Get_Level_Size(level);
			memcpy(dest_surface,Get_Volume_Memory_Pointer(level),compressed_size);
		}
	}
	else 
	{
		// If size matches, copy each pixel linearly with color space conversion
		if (dest_width==Get_Width(level) && dest_height==Get_Height(level)) 
		{
			// An exception here - if the source format is DXT1 and the destination
			// is DXT2, just copy the contents and create an empty alpha channel.
			// This is needed on NVidia cards that have problems with DXT1 compression.
			if (Format==WW3D_FORMAT_DXT1 && dest_format==WW3D_FORMAT_DXT2) 
			{
				// If hue shift, we can't just copy...
				if (has_hsv_shift) 
				{
					const unsigned* src_ptr=reinterpret_cast<const unsigned*>(Get_Volume_Memory_Pointer(level));
					unsigned* dest_ptr=reinterpret_cast<unsigned*>(dest_surface);
					for (unsigned y=0;y<dest_height;y+=4) 
					{
						for (unsigned x=0;x<dest_width;x+=4) 
						{
							*dest_ptr++=0xffffffff;		// Bytes 1-4 of alpha block
							*dest_ptr++=0xffffffff;		// Bytes 5-8 of alpha block
//							*dest_ptr++=*src_ptr++;		// Bytes 1-4 of color block

							unsigned cols=*src_ptr++;	// Bytes 1-4 of color block
							unsigned col0=RGB565_To_ARGB8888((unsigned short)(cols>>16));
							unsigned col1=RGB565_To_ARGB8888((unsigned short)(cols&0xffff));
							Recolor(col0,hsv_shift);
							Recolor(col1,hsv_shift);
							col0=ARGB8888_To_RGB565(col0);
							col1=ARGB8888_To_RGB565(col1);
							cols=unsigned(col0)<<16|col1;
							*dest_ptr++=cols;

							*dest_ptr++=*src_ptr++;		// Bytes 5-8 of color block
						}
					}
				}
				else 
				{
					const unsigned* src_ptr=reinterpret_cast<const unsigned*>(Get_Volume_Memory_Pointer(level));
					unsigned* dest_ptr=reinterpret_cast<unsigned*>(dest_surface);
					for (unsigned y=0;y<dest_height;y+=4) 
					{
						for (unsigned x=0;x<dest_width;x+=4) 
						{
							*dest_ptr++=0xffffffff;		// Bytes 1-4 of alpha block
							*dest_ptr++=0xffffffff;		// Bytes 5-8 of alpha block
							*dest_ptr++=*src_ptr++;		// Bytes 1-4 of color block
							*dest_ptr++=*src_ptr++;		// Bytes 5-8 of color block
						}
					}
				}
			}
			else 
			{
				WWASSERT(0);
			/*	todo
				unsigned dest_bpp=Get_Bytes_Per_Pixel(dest_format);

				// Copy 4x4 block at a time
				bool contains_alpha=false;
				for (unsigned z=0;z<dest_depth;z++)
				{
					for (unsigned y=0;y<dest_height;y+=4) 
					{
						unsigned char* dest_ptr=dest_surface;
						row_ptr+=y*row_pitch;
						for (unsigned x=0;x<dest_width;x+=4,dest_ptr+=dest_bpp*4) 
						{
							contains_alpha|=Get_4x4_Block(dest_ptr,dest_pitch,dest_format,level,x,y,hsv_shift);
						}
					}
					if (Format==WW3D_FORMAT_DXT1 && contains_alpha) 
					{
						WWDEBUG_SAY(("Warning: DXT1 format should not contain alpha information - file %s",Name));
					}
				}*/
			}
		}
	}
}

// ----------------------------------------------------------------------------

WWINLINE static unsigned Combine_Colors(unsigned col1, unsigned col2, unsigned rel)
{
	const unsigned R_B_MASK=0x00ff00ff;
	const unsigned G_MASK=0x0000ff00;

	unsigned rel2=255-rel;

	unsigned r_b_col1=col1&R_B_MASK;
	r_b_col1*=rel;
	unsigned r_b_col2=col2&R_B_MASK;
	r_b_col2*=rel2;
	r_b_col1+=r_b_col2;
	r_b_col1>>=8;
	r_b_col1&=R_B_MASK;


	unsigned g_col1=col1&G_MASK;
	g_col1*=rel;
	unsigned g_col2=col2&G_MASK;
	g_col2*=rel2;
	g_col1+=g_col2;
	g_col1>>=8;
	g_col1&=G_MASK;

	return r_b_col1|g_col1;

/*	float f=float(rel)/256.0f;

	unsigned new_col=0;
	new_col|=int(float(int(col1&0x00ff0000))*f+float(int(col2&0x00ff0000))*(1.0f-f))&0x00ff0000;
	new_col|=int(float(int(col1&0x0000ff00))*f+float(int(col2&0x0000ff00))*(1.0f-f))&0x0000ff00;
	new_col|=int(float(int(col1&0x000000ff))*f+float(int(col2&0x000000ff))*(1.0f-f))&0x000000ff;
	return new_col;
*/
}

// ----------------------------------------------------------------------------
//
// Note that this is NOT an efficient way of extracting pixels from compressed image - we should implement
// faster block-copy method for non-scaled copying.
//
// ----------------------------------------------------------------------------

unsigned DDSFileClass::Get_Pixel(unsigned level,unsigned x,unsigned y) const
{
	WWASSERT(level<MipLevels);
	WWASSERT(x<Get_Width(level));
	WWASSERT(y<Get_Height(level));

	switch (Format) {
	// Note that we don't currently really support alpha on DXT1 - all alpha textures should use DXT5.
	// The reason for this is that when converting from DXT1 to 16 bit uncompressed texture we want
	// to be able to use RGB565 format instead of ARGB4444. As the alpha is encoded in DXT1 per-block
	// basis there isn't really a way to tell if the surface has an alpha or not so either we use alpha
	// or we don't.
	case WW3D_FORMAT_DXT1:
		{
			const unsigned char* block_memory=Get_Memory_Pointer(level)+(x/4)*8+((y/4)*(Get_Width(level)/4))*8;

			unsigned col0=RGB565_To_ARGB8888(*(unsigned short*)&block_memory[0]);
			unsigned col1=RGB565_To_ARGB8888(*(unsigned short*)&block_memory[2]);
			unsigned char line=block_memory[4+(y%4)];
			line>>=(x%4)*2;
			line=(line&3);
			if (col0>col1) {
				switch (line) {
				case 0: return col0|0xff000000;
				case 1: return col1|0xff000000;
				case 2: return Combine_Colors(col1,col0,85)|0xff000000;
				case 3: return Combine_Colors(col0,col1,85)|0xff000000;
				}
			}
			else {
				switch (line) {
				case 0: return col0|0xff000000;
				case 1: return col1|0xff000000;
				case 2: return Combine_Colors(col1,col0,128)|0xff000000;
				case 3: return 0x00000000;
				}
			}
		}
		break;
	case WW3D_FORMAT_DXT2:
		return 0xffffffff;
	case WW3D_FORMAT_DXT3:
		return 0xffffffff;
	case WW3D_FORMAT_DXT4:
		return 0xffffffff;
	case WW3D_FORMAT_DXT5:
		{
			const unsigned char* alpha_block=Get_Memory_Pointer(level)+(x/4)*16+((y/4)*(Get_Width(level)/4))*16;

			unsigned alpha0=alpha_block[0];
			unsigned alpha1=alpha_block[1];

			unsigned bit_idx=((x%4)+4*(y%4))*3;
			unsigned byte_idx=bit_idx/8;
			bit_idx%=8;
			unsigned alpha_index=0;
			for (int i=0;i<3;++i) {
				WWASSERT(byte_idx<6);
				unsigned alpha_bit=(alpha_block[2+byte_idx]>>(bit_idx))&1;
				alpha_index|=alpha_bit<<(i);
				bit_idx++;
				if (bit_idx>=8) {
					bit_idx=0;
					byte_idx++;
				}
			}
			WWASSERT(alpha_index<8);

			// 8-alpha or 6-alpha block?    
			unsigned alpha_value=0;
			if (alpha0>alpha1) {    
				// 8-alpha block:  derive the other six alphas.    
				// Bit code 000 = alpha_0, 001 = alpha_1, others are interpolated.
				switch (alpha_index) {
				case 0: alpha_value=alpha0; break;
				case 1: alpha_value=alpha1; break;
				case 2: alpha_value=(6*alpha0+1*alpha1+3) / 7; break;    // bit code 010
				case 3: alpha_value=(5*alpha0+2*alpha1+3) / 7; break;    // bit code 011
				case 4: alpha_value=(4*alpha0+3*alpha1+3) / 7; break;    // bit code 100
				case 5: alpha_value=(3*alpha0+4*alpha1+3) / 7; break;    // bit code 101
				case 6: alpha_value=(2*alpha0+5*alpha1+3) / 7; break;    // bit code 110
				case 7: alpha_value=(1*alpha0+6*alpha1+3) / 7; break;    // bit code 111  
				}
			}    
			else {  
				// 6-alpha block.    
				// Bit code 000 = alpha_0, 001 = alpha_1, others are interpolated.
				switch (alpha_index) {
				case 0: alpha_value=alpha0; break;
				case 1: alpha_value=alpha1; break;
				case 2: alpha_value=(4*alpha0+1*alpha1+2) / 5; break;    // Bit code 010
				case 3: alpha_value=(3*alpha0+2*alpha1+2) / 5; break;    // Bit code 011
				case 4: alpha_value=(2*alpha0+3*alpha1+2) / 5; break;    // Bit code 100
				case 5: alpha_value=(1*alpha0+4*alpha1+2) / 5; break;    // Bit code 101
				case 6: alpha_value=0; break;                            // Bit code 110
				case 7: alpha_value=255; break;                          // Bit code 111
				}
			}
			alpha_value<<=24;

			// Extract color

			const unsigned char* color_block=alpha_block+8;
			unsigned col0=RGB565_To_ARGB8888(*(unsigned short*)&color_block[0]);
			unsigned col1=RGB565_To_ARGB8888(*(unsigned short*)&color_block[2]);
			unsigned char line=color_block[4+(y%4)];
			line>>=(x%4)*2;
			line=(line&3);
			switch (line) {
			case 0: return col0|alpha_value;
			case 1: return col1|alpha_value;
			case 2: return Combine_Colors(col1,col0,85)|alpha_value;
			case 3: return Combine_Colors(col0,col1,85)|alpha_value;
			}
		}
		break;
	}
	return 0xffffffff;
}

// ----------------------------------------------------------------------------
//
// Uncompress one 4x4 block from the compressed image.
//
// Returns: true if block contained alpha, false is not
//
// Note: Destination can't be DXT or paletted surface!
//
// ----------------------------------------------------------------------------

bool DDSFileClass::Get_4x4_Block(
	unsigned char* dest_ptr,			// Destination surface pointer
	unsigned dest_pitch,					// Destination surface pitch, in bytes
	WW3DFormat dest_format,				// Destination surface format, A8R8G8B8 is fastest
	unsigned level,						// DDS mipmap level to copy from
	unsigned source_x,					// DDS x offset to copy from, must be aligned by 4!
	unsigned source_y,					// DDS y offset to copy from, must be aligned by 4!
	const Vector3& hsv_shift) const
{
	// Verify the block alignment
	WWASSERT((source_x&3)==0);
	WWASSERT((source_y&3)==0);
	// Verify level
	WWASSERT(level<MipLevels);
	// Verify coordinate bounds
	WWASSERT(source_x<Get_Width(level));
	WWASSERT(source_y<Get_Height(level));

	unsigned dest_bpp=Get_Bytes_Per_Pixel(dest_format);

	bool has_hsv_shift = hsv_shift[0]!=0.0f || hsv_shift[1]!=0.0f || hsv_shift[2]!=0.0f;
	switch (Format) {
	// Note that we don't currently really support alpha on DXT1 - all alpha textures should use DXT5.
	// The reason for this is that when converting from DXT1 to 16 bit uncompressed texture we want
	// to be able to use RGB565 format instead of ARGB4444. As the alpha is encoded in DXT1 per-block
	// basis there isn't really a way to tell if the surface has an alpha or not so either we use alpha
	// or we don't.
	case WW3D_FORMAT_DXT1:
		{
			const unsigned char* block_memory=Get_Memory_Pointer(level)+(source_x/4)*8+((source_y/4)*(Get_Width(level)/4))*8;

			unsigned col0=RGB565_To_ARGB8888(*(unsigned short*)&block_memory[0]);
			unsigned col1=RGB565_To_ARGB8888(*(unsigned short*)&block_memory[2]);
			// Even if we don't support alpha, decompression is different if source has alpha
			unsigned dest_pixel=0;
			if (col0>col1) {
				if (has_hsv_shift) {
					Recolor(col0,hsv_shift);
					Recolor(col1,hsv_shift);
				}

				for (int y=0;y<4;++y) {
					unsigned char* tmp_dest_ptr=dest_ptr;
					dest_ptr+=dest_pitch;
					unsigned char line=block_memory[4+y];
					for (int x=0;x<4;++x) {
						switch (line&3) {
						case 0: dest_pixel=col0|0xff000000; break;
						case 1: dest_pixel=col1|0xff000000; break;
						case 2: dest_pixel=Combine_Colors(col1,col0,85)|0xff000000; break;
						case 3: dest_pixel=Combine_Colors(col0,col1,85)|0xff000000; break;
						}
						line>>=2;

						BitmapHandlerClass::Write_B8G8R8A8(tmp_dest_ptr,dest_format,dest_pixel);
						tmp_dest_ptr+=dest_bpp;
					}
				}
				return false;	// No alpha found in the block
			}
			else {
				if (has_hsv_shift) {
					Recolor(col0,hsv_shift);
					Recolor(col1,hsv_shift);
				}
				bool contains_alpha=false;
				for (int y=0;y<4;++y) {
					unsigned char* tmp_dest_ptr=dest_ptr;
					dest_ptr+=dest_pitch;
					unsigned char line=block_memory[4+y];
					for (int x=0;x<4;++x) {
						switch (line&3) {
						case 0: dest_pixel=col0|0xff000000; break;
						case 1: dest_pixel=col1|0xff000000; break;
						case 2: dest_pixel=Combine_Colors(col1,col0,128)|0xff000000; break;
						case 3: dest_pixel=0x00000000; contains_alpha=true; break;
						}
						line>>=2;

						BitmapHandlerClass::Write_B8G8R8A8(tmp_dest_ptr,dest_format,dest_pixel);
						tmp_dest_ptr+=dest_bpp;
					}
				}
				return contains_alpha;	// Alpha block...?
			}
		}
		break;
	case WW3D_FORMAT_DXT2:
		return false;
	case WW3D_FORMAT_DXT3:
		return false;
	case WW3D_FORMAT_DXT4:
		return false;
	case WW3D_FORMAT_DXT5:
		{
			// Init alphas
			const unsigned char* alpha_block=Get_Memory_Pointer(level)+(source_x/4)*16+((source_y/4)*(Get_Width(level)/4))*16;

			unsigned alphas[8];
			alphas[0]=alpha_block[0];
			alphas[1]=alpha_block[1];

			// 8-alpha or 6-alpha block?    
			if (alphas[0]>alphas[1]) {    
				alphas[2]=(6*alphas[0]+1*alphas[1]+3) / 7;   // bit code 010
				alphas[3]=(5*alphas[0]+2*alphas[1]+3) / 7;   // bit code 011
				alphas[4]=(4*alphas[0]+3*alphas[1]+3) / 7;   // bit code 100
				alphas[5]=(3*alphas[0]+4*alphas[1]+3) / 7;   // bit code 101
				alphas[6]=(2*alphas[0]+5*alphas[1]+3) / 7;   // bit code 110
				alphas[7]=(1*alphas[0]+6*alphas[1]+3) / 7;   // bit code 111  
			}
			else {
				alphas[2]=(4*alphas[0]+1*alphas[1]+2) / 5;   // Bit code 010
				alphas[3]=(3*alphas[0]+2*alphas[1]+2) / 5;   // Bit code 011
				alphas[4]=(2*alphas[0]+3*alphas[1]+2) / 5;   // Bit code 100
				alphas[5]=(1*alphas[0]+4*alphas[1]+2) / 5;   // Bit code 101
				alphas[6]=0; 										   // Bit code 110
				alphas[7]=255; 									   // Bit code 111
			}

			// Init colors
			const unsigned char* color_block=alpha_block+8;
			unsigned col0=RGB565_To_ARGB8888(*(unsigned short*)&color_block[0]);
			unsigned col1=RGB565_To_ARGB8888(*(unsigned short*)&color_block[2]);
			if (has_hsv_shift) {
				Recolor(col0,hsv_shift);
				Recolor(col1,hsv_shift);
			}

			unsigned dest_pixel=0;
			unsigned bit_idx=0;
			unsigned contains_alpha=0xff;

			unsigned alpha_indices[16];
			unsigned* ai_ptr=alpha_indices;
			for (int a=0;a<2;++a) {
				ai_ptr[0]=alpha_block[2]&0x7;
				ai_ptr[1]=(alpha_block[2]>>3)&0x7;
				ai_ptr[2]=(alpha_block[2]>>6)|((alpha_block[3]&1)<<2);
				ai_ptr[3]=(alpha_block[3]>>1)&0x7;
				ai_ptr[4]=(alpha_block[3]>>4)&0x7;
				ai_ptr[5]=(alpha_block[3]>>7)|((alpha_block[4]&3)<<1);
				ai_ptr[6]=(alpha_block[4]>>2)&0x7;
				ai_ptr[7]=(alpha_block[4]>>5);
				ai_ptr+=8;
				alpha_block+=3;
			}

			unsigned aii=0;
			for (int y=0;y<4;++y) {
				unsigned char* tmp_dest_ptr=dest_ptr;
				dest_ptr+=dest_pitch;
				unsigned char line=color_block[4+y];
				for (int x=0;x<4;++x,bit_idx+=3) {
					unsigned alpha_value=alphas[alpha_indices[aii++]];
					contains_alpha&=alpha_value;
					alpha_value<<=24;

					// Extract color

					switch (line&3) {
					case 0: dest_pixel=col0|alpha_value; break;
					case 1: dest_pixel=col1|alpha_value; break;
					case 2: dest_pixel=Combine_Colors(col1,col0,85)|alpha_value; break;
					case 3: dest_pixel=Combine_Colors(col0,col1,85)|alpha_value; break;
					}
					line>>=2;

					BitmapHandlerClass::Write_B8G8R8A8(tmp_dest_ptr,dest_format,dest_pixel);
					tmp_dest_ptr+=dest_bpp;
				}
			}




/*
			for (int y=0;y<4;++y) {
				unsigned char* tmp_dest_ptr=dest_ptr;
				dest_ptr+=dest_pitch;
				unsigned char line=color_block[4+y];
				for (int x=0;x<4;++x,bit_idx+=3) {
					unsigned byte_idx=bit_idx/8;
					unsigned tmp_bit_idx=bit_idx&7;
					unsigned alpha_index=0;
					for (int i=0;i<3;++i) {
						WWASSERT(byte_idx<6);
						unsigned alpha_bit=(alpha_block[2+byte_idx]>>(tmp_bit_idx))&1;
						alpha_index|=alpha_bit<<(i);
						tmp_bit_idx++;
						if (tmp_bit_idx>=8) {
							tmp_bit_idx=0;
							byte_idx++;
						}
					}
					WWASSERT(alpha_index<8);
					unsigned alpha_value=alphas[alpha_index];
					contains_alpha&=alpha_value;
					alpha_value<<=24;

					// Extract color

					switch (line&3) {
					case 0: dest_pixel=col0|alpha_value; break;
					case 1: dest_pixel=col1|alpha_value; break;
					case 2: dest_pixel=Combine_Colors(col1,col0,85)|alpha_value; break;
					case 3: dest_pixel=Combine_Colors(col0,col1,85)|alpha_value; break;
					}
					line>>=2;

					BitmapHandlerClass::Write_B8G8R8A8(tmp_dest_ptr,dest_format,dest_pixel);
					tmp_dest_ptr+=dest_bpp;
				}
			}
*/
			return contains_alpha!=0xff;	// Alpha block... DXT5 should only be used when the image needs alpha
													// but for now check anyway...
		}
	}
	return false;

}
