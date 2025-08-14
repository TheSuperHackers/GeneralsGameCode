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

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef BITMAPHANDLER_H
#define BITMAPHANDLER_H

#include "always.h"
#include "ww3dformat.h"
#include "vector3.h"
#include "colorspace.h"

void Bitmap_Assert(bool condition);

class BitmapHandlerClass
{
public:
	// Read pixel at given address
	WWINLINE static void Read_B8G8R8A8(
		unsigned char* argb,
		const unsigned char* src_ptr,
		WW3DFormat src_format,
		const unsigned char* palette,
		unsigned palette_bpp);

	// Read pixel at given address
	WWINLINE static void Read_B8G8R8A8(
		unsigned& argb,
		const unsigned char* src_ptr,
		WW3DFormat src_format,
		const unsigned char* palette,
		unsigned palette_bpp);

	// Read pixel from surface
	WWINLINE static void Read_B8G8R8A8(
		unsigned& argb,
		const unsigned char* src_ptr,
		WW3DFormat src_format,
		int x,
		int y,
		int width,
		int height,
		const unsigned char* palette,
		unsigned palette_bpp);

	WWINLINE static void Write_B8G8R8A8(
		unsigned char* dest_ptr,
		WW3DFormat dest_format,
		const unsigned char* argb);

	WWINLINE static void Write_B8G8R8A8(
		unsigned char* dest_ptr,
		WW3DFormat dest_format,
		const unsigned& argb);

	WWINLINE static void Copy_Pixel(
		unsigned char* dest_ptr,
		WW3DFormat dest_format,
		const unsigned char* src_ptr,
		WW3DFormat src_format,
		const unsigned char* palette,
		unsigned palette_bpp);

	WWINLINE static void Copy_Pixel(
		unsigned char* dest_ptr,
		WW3DFormat dest_format,
		const unsigned char* src_ptr,
		WW3DFormat src_format,
		const unsigned char* palette,
		unsigned palette_bpp,
		const Vector3& hsv_shift);

	WWINLINE static unsigned Combine_A8R8G8B8(
		unsigned bgra1,
		unsigned bgra2,
		unsigned bgra3,
		unsigned bgra4);

	static void Create_Mipmap_B8G8R8A8(
		unsigned char* dest_surface,
		unsigned dest_surface_pitch,
		unsigned char* src_surface,
		unsigned src_surface_pitch,
		unsigned width,
		unsigned height);

	static void Copy_Image_Generate_Mipmap(
		unsigned width,
		unsigned height,
		unsigned char* dest_surface,
		unsigned dest_pitch,
		WW3DFormat dest_format,
		unsigned char* src_surface,
		unsigned src_pitch,
		WW3DFormat src_format,
		unsigned char* mip_surface,
		unsigned mip_pitch,
		const Vector3& hsv_shift=Vector3(0.0f,0.0f,0.0f));

	static void Copy_Image(
		unsigned char* dest_surface,
		unsigned dest_surface_width,
		unsigned dest_surface_height,
		unsigned dest_surface_pitch,
		WW3DFormat dest_surface_format,
		unsigned char* src_surface,
		unsigned src_surface_width,
		unsigned src_surface_height,
		unsigned src_surface_pitch,
		WW3DFormat src_surface_format,
		const unsigned char* src_palette,
		unsigned src_palette_bpp,
		bool generate_mip_level,
		const Vector3& hsv_shift=Vector3(0.0f,0.0f,0.0f));

};

// ----------------------------------------------------------------------------
//
// Read color value of given type in BGRA (D3D) byte order. Regarless
// of the source format the color value is converted to 32-bit format.
//
// ----------------------------------------------------------------------------

WWINLINE void BitmapHandlerClass::Read_B8G8R8A8(
	unsigned char* argb,
	const unsigned char* src_ptr,
	WW3DFormat src_format,
	const unsigned char* palette,
	unsigned palette_bpp)
{
	switch (src_format) {
	case WW3D_FORMAT_A8R8G8B8:
	case WW3D_FORMAT_X8R8G8B8:
		*(unsigned*)argb=*(unsigned*)src_ptr;
		break;
	case WW3D_FORMAT_R8G8B8:
		*argb++=src_ptr[0];
		*argb++=src_ptr[1];
		*argb++=src_ptr[2];
		*argb++=0xff;
		break;
	case WW3D_FORMAT_A4R4G4B4:
		{
			unsigned short tmp;
			tmp=*(unsigned short*)src_ptr;
			*argb++=((tmp&0x000f)<<4);
			*argb++=((tmp&0x00f0));
			*argb++=((tmp&0x0f00)>>4);
			*argb++=((tmp&0xf000)>>8);
		}
		break;
	case WW3D_FORMAT_A1R5G5B5:
		{
			unsigned short tmp;
			tmp=*(unsigned short*)src_ptr;
			argb[3]=tmp&0x8000 ? 0xff : 0x0;
			argb[2]=(tmp>>7)&0xf8;
			argb[1]=(tmp>>2)&0xf8;
			argb[0]=(tmp<<3)&0xf8;
		}
		break;
	case WW3D_FORMAT_R5G6B5:
		{
			unsigned short tmp;
			tmp=*(unsigned short*)src_ptr;
			argb[3]=0xff;
			argb[2]=(tmp>>8)&0xf8;
			argb[1]=(tmp>>3)&0xfc;
			argb[0]=(tmp<<3)&0xf8;
		}
		break;
	case WW3D_FORMAT_R3G3B2:
		{
			unsigned char tmp=*src_ptr;
			argb[3]=0xff;
			argb[2]=tmp&0xe0;
			argb[1]=(tmp<<3)&0xe0;
			argb[0]=(tmp<<6)&0xc0;
		}
		break;
	case WW3D_FORMAT_L8:
		{
			unsigned char tmp=*src_ptr++;
			*argb++=tmp;
			*argb++=tmp;
			*argb++=tmp;
			*argb++=0xff;
		}
		break;
	case WW3D_FORMAT_A8:
		{
			*argb++=0;
			*argb++=0;
			*argb++=0;
			*argb++=*src_ptr++;
		}
		break;
	case WW3D_FORMAT_P8:
		{
			unsigned char index=*src_ptr++;
			switch (palette_bpp) {
			case 4:
				*argb++=palette[palette_bpp*index+3];
				*argb++=palette[palette_bpp*index+2];
				*argb++=palette[palette_bpp*index+1];
				*argb++=palette[palette_bpp*index+0];
				break;
			case 3:
				*argb++=palette[palette_bpp*index+2];
				*argb++=palette[palette_bpp*index+1];
				*argb++=palette[palette_bpp*index+0];
				*argb++=0xff;
				break;
			case 2:
			case 1:
			default:
				Bitmap_Assert(0);
				break;
			}
		}
		break;
	case WW3D_FORMAT_DXT1:
	case WW3D_FORMAT_DXT2:
	case WW3D_FORMAT_DXT3:
	case WW3D_FORMAT_DXT4:
	case WW3D_FORMAT_DXT5:
	default:	Bitmap_Assert(0); break;
	}
}

WWINLINE void BitmapHandlerClass::Read_B8G8R8A8(
	unsigned& argb,
	const unsigned char* src_ptr,
	WW3DFormat src_format,
	const unsigned char* palette,
	unsigned palette_bpp)
{
	Read_B8G8R8A8((unsigned char*)&argb,src_ptr,src_format,palette,palette_bpp);
}

// Read pixel from surface
WWINLINE void BitmapHandlerClass::Read_B8G8R8A8(
	unsigned& argb,
	const unsigned char* src_ptr,
	WW3DFormat src_format,
	int x,
	int y,
	int width,
	int height,
	const unsigned char* palette,
	unsigned palette_bpp)
{
	if (x<0 || y<0 || x>=width || y>=height) {
		argb=0;
		return;
	}

	unsigned bpp=Get_Bytes_Per_Pixel(src_format);
	Read_B8G8R8A8(
		argb,
		src_ptr+bpp*x+width*bpp*y,
		src_format,
		palette,
		palette_bpp);
}


// ----------------------------------------------------------------------------
//
// Write color value of given type in BGRA (D3D) byte order. The source value
// is always 32 bit and it is converted to defined destination format.
//
// ----------------------------------------------------------------------------

WWINLINE void BitmapHandlerClass::Write_B8G8R8A8(
	unsigned char* dest_ptr,
	WW3DFormat dest_format,
	const unsigned char* argb)
{
	switch (dest_format) {
	case WW3D_FORMAT_A8R8G8B8:
	case WW3D_FORMAT_X8R8G8B8:
		*(unsigned*)dest_ptr=*(unsigned*)argb;
		break;
	case WW3D_FORMAT_R8G8B8:
		*dest_ptr++=*argb++;
		*dest_ptr++=*argb++;
		*dest_ptr++=*argb++;
		break;
	case WW3D_FORMAT_A4R4G4B4:
		{
			unsigned short tmp;
			tmp=((argb[3])&0xf0)<<8;
			tmp|=((argb[2])&0xf0)<<4;
			tmp|=((argb[1])&0xf0);
			tmp|=((argb[0])&0xf0)>>4;
			*(unsigned short*)dest_ptr=tmp;
		}
		break;
	case WW3D_FORMAT_A1R5G5B5:
		{
			unsigned short tmp;
			tmp=argb[3] ? 0x8000 : 0x0;
			tmp|=((argb[2])&0xf8)<<7;
			tmp|=((argb[1])&0xf8)<<2;
			tmp|=((argb[0])&0xf8)>>3;
			*(unsigned short*)dest_ptr=tmp;
		}
		break;
	case WW3D_FORMAT_R5G6B5:
		{
			unsigned short tmp;
			tmp=((argb[2])&0xf8)<<8;
			tmp|=((argb[1])&0xfc)<<3;
			tmp|=((argb[0])&0xf8)>>3;
			*(unsigned short*)dest_ptr=tmp;
		}
		break;
	case WW3D_FORMAT_R3G3B2:
		{
			unsigned char tmp;
			tmp=((argb[2])&0xe0);
			tmp|=((argb[1])&0xe0)>>3;
			tmp|=((argb[0])&0xc0)>>6;
			*(unsigned short*)dest_ptr=tmp;
		}
		break;
	case WW3D_FORMAT_L8:
		{
			// CIE Req. 709: Y709 = 0.2125R + 0.7154G + 0.0721B
			unsigned char tmp = (unsigned char)	( (
				((unsigned int)argb[0] * (unsigned int)0x1275) +	// 0.0721B
				((unsigned int)argb[1] * (unsigned int)0xB725) +	// 0.7154G (rounded up so FF, FF, FF becomes FF)
				((unsigned int)argb[2] * (unsigned int)0x3666)		// 0.2125R
			) >> 16);
			*dest_ptr++=tmp;
		}
		break;
	case WW3D_FORMAT_A8:
		{
			*dest_ptr++=*argb++;
		}
		break;
	case WW3D_FORMAT_DXT1:
	case WW3D_FORMAT_DXT2:
	case WW3D_FORMAT_DXT3:
	case WW3D_FORMAT_DXT4:
	case WW3D_FORMAT_DXT5:
	case WW3D_FORMAT_P8:	// Paletted destination not supported
	default:	Bitmap_Assert(0); break;
	}
}

WWINLINE void BitmapHandlerClass::Write_B8G8R8A8(
	unsigned char* dest_ptr,
	WW3DFormat dest_format,
	const unsigned& argb)
{
	Write_B8G8R8A8(dest_ptr,dest_format,(unsigned char*)&argb);
}

// ----------------------------------------------------------------------------
//
// Copy pixel. Perform color space conversion if needed. The source and
// destination are always D3D-style BGRA.
//
// ----------------------------------------------------------------------------

WWINLINE void BitmapHandlerClass::Copy_Pixel(
	unsigned char* dest_ptr,
	WW3DFormat dest_format,
	const unsigned char* src_ptr,
	WW3DFormat src_format,
	const unsigned char* palette,
	unsigned palette_bpp)
{
	// Color space conversion needed?

	if (dest_format==src_format) {
		switch (dest_format) {
		case WW3D_FORMAT_A8R8G8B8:
		case WW3D_FORMAT_X8R8G8B8:
			*(unsigned*)dest_ptr=*(unsigned*)src_ptr;
			break;
		case WW3D_FORMAT_R8G8B8:
			*dest_ptr++=src_ptr[0];
			*dest_ptr++=src_ptr[1];
			*dest_ptr++=src_ptr[2];
			break;
		case WW3D_FORMAT_A4R4G4B4:
			{
				unsigned short tmp=*(unsigned short*)src_ptr;
				*(unsigned short*)dest_ptr=((tmp&0x000f)<<12)|((tmp&0x00f0)<<4)|((tmp&0x0f00)>>4)|((tmp&0xf000)>>12);
			}
			break;
		case WW3D_FORMAT_A1R5G5B5:
			{
				unsigned short tmp=*(unsigned short*)src_ptr;
				*(unsigned short*)dest_ptr=((tmp&0x001f)<<11)|((tmp&0x03e0)<<1)|((tmp&0x7c00)>>9)|((tmp&0x8000)>>15);
			}
			break;
		case WW3D_FORMAT_R5G6B5:
			{
				unsigned short tmp=*(unsigned short*)src_ptr;
				*(unsigned short*)dest_ptr=((tmp&0x001f)<<11)|(tmp&0x07e0)|((tmp&0xf800)>>11);
			}
			break;
		case WW3D_FORMAT_R3G3B2:
		case WW3D_FORMAT_L8:
		case WW3D_FORMAT_A8: *dest_ptr++=*src_ptr++;
			break;
		case WW3D_FORMAT_P8:	// Paletted destinations not supported
		default:	Bitmap_Assert(0); break;
		}
	}
	else {
		unsigned b8g8r8a8;
		Read_B8G8R8A8(b8g8r8a8,src_ptr,src_format,palette,palette_bpp);
		Write_B8G8R8A8(dest_ptr,dest_format,b8g8r8a8);
	}
}

// ----------------------------------------------------------------------------
//
// Copy pixel with HSV shift. The source and destination are always D3D-style BGRA.
//
// ----------------------------------------------------------------------------

WWINLINE void BitmapHandlerClass::Copy_Pixel(
	unsigned char* dest_ptr,
	WW3DFormat dest_format,
	const unsigned char* src_ptr,
	WW3DFormat src_format,
	const unsigned char* palette,
	unsigned palette_bpp,
	const Vector3& hsv_shift)
{
	unsigned b8g8r8a8;
	Read_B8G8R8A8(b8g8r8a8,src_ptr,src_format,palette,palette_bpp);
	Recolor(b8g8r8a8,hsv_shift);
	Write_B8G8R8A8(dest_ptr,dest_format,b8g8r8a8);
}

WWINLINE unsigned BitmapHandlerClass::Combine_A8R8G8B8(
	unsigned bgra1,
	unsigned bgra2,
	unsigned bgra3,
	unsigned bgra4)
{
	bgra1&=0xfcfcfcfc;
	bgra2&=0xfcfcfcfc;
	bgra3&=0xfcfcfcfc;
	bgra4&=0xfcfcfcfc;
	bgra1>>=2;
	bgra2>>=2;
	bgra3>>=2;
	bgra4>>=2;
	bgra1+=bgra2;
	bgra3+=bgra4;
	bgra1+=bgra3;
	return bgra1;
}


#endif
