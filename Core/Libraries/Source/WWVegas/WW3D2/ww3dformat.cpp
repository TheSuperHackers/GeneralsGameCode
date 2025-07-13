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
 *                 Project Name : WW3D                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/ww3dformat.cpp                         $*
 *                                                                                             *
 *              Original Author:: Hector Yee                                                   *
 *                                                                                             *
 *                       Author : Kenny Mitchell                                               * 
 *                                                                                             * 
 *                     $Modtime:: 06/27/02 1:27p                                              $*
 *                                                                                             *
 *                    $Revision:: 14                                                          $*
 *                                                                                             *
 * 06/27/02 KM Z Format support																						*
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "ww3dformat.h"
#include "vector4.h"
#include "wwdebug.h"
#include "TARGA.H"
#include "dx8wrapper.h"
#include "dx8caps.h"
#include <d3d8.h>

 /*
	WW3D_FORMAT_UNKNOWN=0,
	WW3D_FORMAT_R8G8B8,
	WW3D_FORMAT_A8R8G8B8,
	WW3D_FORMAT_X8R8G8B8,
	WW3D_FORMAT_R5G6B5,
	WW3D_FORMAT_X1R5G5B5,
	WW3D_FORMAT_A1R5G5B5,
	WW3D_FORMAT_A4R4G4B4,
	WW3D_FORMAT_R3G3B2,
	WW3D_FORMAT_A8,
	WW3D_FORMAT_A8R3G3B2,
	WW3D_FORMAT_X4R4G4B4,
	WW3D_FORMAT_A8P8,
	WW3D_FORMAT_P8,
	WW3D_FORMAT_L8,
	WW3D_FORMAT_A8L8,
	WW3D_FORMAT_A4L4,
	WW3D_FORMAT_COUNT	// Used only to determine number of surface formats
*/

void Get_WW3D_Format_Name(WW3DFormat format, StringClass& name)
{
	switch (format) {
	default:
	case WW3D_FORMAT_UNKNOWN: name="Unknown"; break;
	case WW3D_FORMAT_R8G8B8: name="R8G8B8"; break;
	case WW3D_FORMAT_A8R8G8B8: name="A8R8G8B8"; break;
	case WW3D_FORMAT_X8R8G8B8: name="X8R8G8B8"; break;
	case WW3D_FORMAT_R5G6B5: name="R5G6B5"; break;
	case WW3D_FORMAT_X1R5G5B5: name="X1R5G5B5"; break;
	case WW3D_FORMAT_A1R5G5B5: name="A1R5G5B5"; break;
	case WW3D_FORMAT_A4R4G4B4: name="A4R4G4B4"; break;
	case WW3D_FORMAT_R3G3B2: name="R3G3B2"; break;
	case WW3D_FORMAT_A8: name="A8"; break;
	case WW3D_FORMAT_A8R3G3B2: name="A8R3G3B2"; break;
	case WW3D_FORMAT_X4R4G4B4: name="X4R4G4B4"; break;
	case WW3D_FORMAT_A8P8: name="A8P8"; break;
	case WW3D_FORMAT_P8: name="P8"; break;
	case WW3D_FORMAT_L8: name="L8"; break;
	case WW3D_FORMAT_A8L8: name="A8L8"; break;
	case WW3D_FORMAT_A4L4: name="A4L4"; break;
	case WW3D_FORMAT_U8V8: name="U8V8"; break;		// Bumpmap
	case WW3D_FORMAT_L6V5U5: name="L6V5U5"; break;	// Bumpmap
	case WW3D_FORMAT_X8L8V8U8: name="X8L8V8U8"; break;	// Bumpmap
	case WW3D_FORMAT_DXT1: name="DXT1"; break;
	case WW3D_FORMAT_DXT2: name="DXT2"; break;
	case WW3D_FORMAT_DXT3: name="DXT3"; break;
	case WW3D_FORMAT_DXT4: name="DXT4"; break;
	case WW3D_FORMAT_DXT5: name="DXT5"; break;
	}
}

//**********************************************************************************************
//! Get W3D depth stencil format string name
/*! 06/27/02 KJM
*/
void Get_WW3D_ZFormat_Name(WW3DZFormat format, StringClass& name)
{
	switch (format) 
	{
	default:
	case WW3D_FORMAT_UNKNOWN		: name="Unknown"; break;
	case WW3D_ZFORMAT_D16_LOCKABLE: name="D16Lockable"; break; // 16-bit z-buffer bit depth. This is an application-lockable surface format. 
	case WW3D_ZFORMAT_D32			: name="D32"; break; // 32-bit z-buffer bit depth. 
	case WW3D_ZFORMAT_D15S1			: name="D15S1"; break; // 16-bit z-buffer bit depth where 15 bits are reserved for the depth channel and 1 bit is reserved for the stencil channel. 
	case WW3D_ZFORMAT_D24S8			: name="D24S8"; break; // 32-bit z-buffer bit depth using 24 bits for the depth channel and 8 bits for the stencil channel. 
	case WW3D_ZFORMAT_D16			: name="D16"; break; // 16-bit z-buffer bit depth. 
	case WW3D_ZFORMAT_D24X8			: name="D24X8"; break; // 32-bit z-buffer bit depth using 24 bits for the depth channel. 
	case WW3D_ZFORMAT_D24X4S4		: name="D24X4S4"; break; // 32-bit z-buffer bit depth using 24 bits for the depth channel and 4 bits for the stencil channel. 
#ifdef _XBOX
	case WW3D_ZFORMAT_LIN_D24S8	: name="D24S8LIN"; break;
	case WW3D_ZFORMAT_LIN_F24S8	: name="F24S8LIN"; break;
	case WW3D_ZFORMAT_LIN_D16		: name="D16LIN"; break;
	case WW3D_ZFORMAT_LIN_F16		: name="F16LIN"; break;
#endif
	}
}

// extract the luminance from the RGB using the CIE 709 standard
unsigned char RGB_to_CIEY(Vector4 color)
{
	float lum=0.2126f*color.X + 0.7152f*color.Y + 0.0722f*color.Z;
	return (unsigned char) (255.0f*lum);
}

void Vector4_to_Color(unsigned int *outc,const Vector4 &inc,const WW3DFormat format)
{
	// convert to ARGB 32-bit
	unsigned int color=DX8Wrapper::Convert_Color(inc);
	unsigned char *argb=(unsigned char*) &color;
	unsigned char r,g,b,a,lum;

	switch (format)
	{
	case WW3D_FORMAT_R8G8B8:		
	case WW3D_FORMAT_A8R8G8B8:
	case WW3D_FORMAT_X8R8G8B8:
		*outc=color;
		break;
	case WW3D_FORMAT_R5G6B5:
		r=argb[1] >> 3;
		g=argb[2] >> 2;
		b=argb[3] >> 3;
		*outc=(r << 11) | (g<<5) | b;
		break;
	case WW3D_FORMAT_X1R5G5B5:
	case WW3D_FORMAT_A1R5G5B5:
		a=argb[0] >> 7;
		r=argb[1] >> 3;
		g=argb[2] >> 3;
		b=argb[3] >> 3;
		*outc=(a<<15) | (r<<10) | (g<<5) | b;
		break;

	case WW3D_FORMAT_A4R4G4B4:
	case WW3D_FORMAT_X4R4G4B4:
		a=argb[0] >> 4;
		r=argb[1] >> 4;
		g=argb[2] >> 4;
		b=argb[3] >> 4;
		*outc=(a<<12) | (r<<8) | (g<<4) | b;
		break;
	case WW3D_FORMAT_R3G3B2:
	case WW3D_FORMAT_A8R3G3B2:
		a=argb[0];
		r=argb[1] >> 5;
		g=argb[2] >> 5;
		b=argb[3] >> 6;
		*outc=(a<<8) | (r<<5) | (g<<2) | b;
		break;
	case WW3D_FORMAT_A8:
		*outc=argb[0];
		break;
	case WW3D_FORMAT_L8:
		lum=RGB_to_CIEY(inc);
		*outc=lum;
		break;
	case WW3D_FORMAT_A8L8:
		a=argb[0];		
		lum=RGB_to_CIEY(inc);
		*outc=(a<<8) | lum;
		break;
	case WW3D_FORMAT_A4L4:
		a=argb[0] >> 4;		
		lum=RGB_to_CIEY(inc);
		lum=lum>>4;
		*outc=(a<<4) | lum;
		break;
	default:
		WWASSERT(0);
	}
}

void Color_to_Vector4(Vector4* outc,const unsigned int inc,const WW3DFormat format)
{
	WWASSERT(outc);

	unsigned char *argb=(unsigned char*) &inc;
	unsigned char a,r,g,b;
	a=r=g=b=0;

	switch (format)
	{
	case WW3D_FORMAT_R8G8B8:
	case WW3D_FORMAT_A8R8G8B8:
	case WW3D_FORMAT_X8R8G8B8:
		a=argb[0];
		r=argb[1];
		g=argb[2];
		b=argb[3];
		break;
	case WW3D_FORMAT_R5G6B5:
		r=argb[1]<<3;
		g=argb[2]<<2;
		b=argb[3]<<3;
		break;
	case WW3D_FORMAT_X1R5G5B5:
	case WW3D_FORMAT_A1R5G5B5:
		a=argb[0]<<7;
		r=argb[1]<<3;
		g=argb[2]<<3;
		b=argb[3]<<3;
		break;
	case WW3D_FORMAT_A4R4G4B4:
		a=argb[0]<<4;
		r=argb[1]<<4;
		g=argb[2]<<4;
		b=argb[3]<<4;
		break;
	case WW3D_FORMAT_R3G3B2:		
		r=argb[1]<<5;
		g=argb[2]<<5;
		b=argb[3]<<6;
	break;
	case WW3D_FORMAT_A8:
		a=argb[0];
		break;
	case WW3D_FORMAT_A8R3G3B2:
		a=argb[0];
		r=argb[1]<<5;
		g=argb[2]<<5;
		b=argb[3]<<6;
		break;
	case WW3D_FORMAT_X4R4G4B4:
		r=argb[1]<<4;
		g=argb[2]<<4;
		b=argb[3]<<4;
		break;
	default:
		WWASSERT(0);
	}
	outc->X=r/255.0f;
	outc->Y=g/255.0f;
	outc->Z=b/255.0f;
	outc->W=a/255.0f;
}

// ----------------------------------------------------------------------------
//
// Utility function for determining WW3D format from TGA file header.
//
// ----------------------------------------------------------------------------

void Get_WW3D_Format(WW3DFormat& dest_format,WW3DFormat& src_format,unsigned& src_bpp,const Targa& targa)
{
	Get_WW3D_Format(src_format,src_bpp,targa);
	dest_format=src_format;
	if ((dest_format==WW3D_FORMAT_P8) || (dest_format==WW3D_FORMAT_L8)) {
		dest_format=WW3D_FORMAT_X8R8G8B8;
	}
	dest_format=Get_Valid_Texture_Format(dest_format,false);	// No compressed destination format if reading from targa...

}

void Get_WW3D_Format(WW3DFormat& src_format,unsigned& src_bpp,const Targa& targa)
{
	// Guess the format from the TGA Header bits:
	src_format = WW3D_FORMAT_UNKNOWN;
	src_bpp=0;
	switch (targa.Header.PixelDepth) {
		case 32:	src_format = WW3D_FORMAT_A8R8G8B8;	src_bpp=4; break;
		case 24:	src_format = WW3D_FORMAT_R8G8B8; src_bpp=3; break;
		case 16:	src_format = WW3D_FORMAT_A1R5G5B5; src_bpp=2; break;
		case 8:
			src_bpp=1;
			if (targa.Header.ColorMapType == 1) src_format = WW3D_FORMAT_P8;
			else if (targa.Header.ImageType == TGA_MONO) src_format = WW3D_FORMAT_L8;
			else src_format = WW3D_FORMAT_A8;
			break;
		default:
			WWDEBUG_SAY(("TextureClass: Targa has unsupported bitdepth(%i)",targa.Header.PixelDepth));
//			WWASSERT(0);
			break;
	}
}

// ----------------------------------------------------------------------------
//
// Utility function for determining valid WW3D format
//
// ----------------------------------------------------------------------------

WW3DFormat Get_Valid_Texture_Format(WW3DFormat format, bool is_compression_allowed)
{
	int w,h,bits;
	bool windowed;

	if (!DX8Wrapper::Get_Current_Caps()->Support_DXTC() || 
		!is_compression_allowed) {
		switch (format) {
		case WW3D_FORMAT_DXT1: format=WW3D_FORMAT_R8G8B8; break;
		case WW3D_FORMAT_DXT2:
		case WW3D_FORMAT_DXT3:
		case WW3D_FORMAT_DXT4:
		case WW3D_FORMAT_DXT5: format=WW3D_FORMAT_A8R8G8B8; break;
		default: break;
		}
	}
	else {
		switch (format) {
		case WW3D_FORMAT_DXT1:
			// NVidia hack - switch to DXT2 is there is no DXT1 support (which is disabled on NVidia cards)
			if (!DX8Wrapper::Get_Current_Caps()->Support_Texture_Format(WW3D_FORMAT_DXT1) && 
				DX8Wrapper::Get_Current_Caps()->Support_Texture_Format(WW3D_FORMAT_DXT2)) {
				format=WW3D_FORMAT_DXT2;
			}
			break;
		case WW3D_FORMAT_DXT2:
		case WW3D_FORMAT_DXT3:
		case WW3D_FORMAT_DXT4:
		case WW3D_FORMAT_DXT5:
			if (!DX8Wrapper::Get_Current_Caps()->Support_Texture_Format(format)) format=WW3D_FORMAT_A8R8G8B8;
			break;
		}
	}

	if (format==WW3D_FORMAT_R8G8B8) {
		format=WW3D_FORMAT_X8R8G8B8;
	}

	WW3D::Get_Device_Resolution(w,h,bits,windowed);
	if (WW3D::Get_Texture_Bitdepth()==16) bits=16;

	// if the device bitdepth is 16, don't allow 32 bit textures
	if (bits<=16) {
		switch (format) {
		case WW3D_FORMAT_A8R8G8B8: return WW3D_FORMAT_A4R4G4B4;
		case WW3D_FORMAT_X8R8G8B8:
		case WW3D_FORMAT_R8G8B8: return WW3D_FORMAT_R5G6B5;
		case WW3D_FORMAT_A4R4G4B4:
		case WW3D_FORMAT_A1R5G5B5:
		case WW3D_FORMAT_R5G6B5:
		case WW3D_FORMAT_L8:
		case WW3D_FORMAT_A8:
		case WW3D_FORMAT_P8:
		default:
			// Basically, anything goes here (just make sure the most common 32 bit formats are converted to 16 bit
			break;
		}

	}

	// Fallback if the hardware doesn't support the texture format
	if (!DX8Wrapper::Get_Current_Caps()->Support_Texture_Format(format)) {
		format=WW3D_FORMAT_A8R8G8B8;
		if (!DX8Wrapper::Get_Current_Caps()->Support_Texture_Format(format)) {
			format=WW3D_FORMAT_A4R4G4B4;
			if (!DX8Wrapper::Get_Current_Caps()->Support_Texture_Format(format)) {
				// If still no luck, try non-alpha formats

				format=WW3D_FORMAT_X8R8G8B8;
				if (!DX8Wrapper::Get_Current_Caps()->Support_Texture_Format(format)) {
					format=WW3D_FORMAT_R5G6B5;
					if (!DX8Wrapper::Get_Current_Caps()->Support_Texture_Format(format)) {
						WWASSERT_PRINT(0,("No valid texture format found"));
					}
				}
			}
		}
	}

	return format;
}

unsigned Get_Bytes_Per_Pixel(WW3DFormat format)
{
	switch (format) {
	case WW3D_FORMAT_X8R8G8B8:
	case WW3D_FORMAT_X8L8V8U8:
	case WW3D_FORMAT_A8R8G8B8: return 4;
	case WW3D_FORMAT_R8G8B8: return 3;
	case WW3D_FORMAT_A1R5G5B5:
	case WW3D_FORMAT_A4R4G4B4:
	case WW3D_FORMAT_U8V8:
	case WW3D_FORMAT_L6V5U5:
	case WW3D_FORMAT_R5G6B5: return 2;
	case WW3D_FORMAT_R3G3B2:
	case WW3D_FORMAT_L8:
	case WW3D_FORMAT_A8:
	case WW3D_FORMAT_P8: return 1;

	default:	WWASSERT(0); break;
	}
	return 0;
}

unsigned Get_Num_Depth_Bits(WW3DZFormat zformat)
{
	switch (zformat)
	{
	case WW3D_ZFORMAT_D16_LOCKABLE: return 16; break;
	case WW3D_ZFORMAT_D32			: return 32; break;
	case WW3D_ZFORMAT_D15S1			: return 15; break;
	case WW3D_ZFORMAT_D24S8			: return 24; break;
	case WW3D_ZFORMAT_D16			: return 16; break;
	case WW3D_ZFORMAT_D24X8			: return 24; break;
	case WW3D_ZFORMAT_D24X4S4		: return 24; break;
#ifdef _XBOX
	case WW3D_ZFORMAT_LIN_D24S8	: return 24; break;
	case WW3D_ZFORMAT_LIN_F24S8	: return 24; break;
	case WW3D_ZFORMAT_LIN_D16		: return 16; break;
	case WW3D_ZFORMAT_LIN_F16		: return 16; break;
#endif
	};
	return 0;
};

unsigned Get_Num_Stencil_Bits(WW3DZFormat zformat)
{
	switch (zformat)
	{
	case WW3D_ZFORMAT_D16_LOCKABLE: return 0; break;
	case WW3D_ZFORMAT_D32			: return 0; break;
	case WW3D_ZFORMAT_D15S1			: return 1; break;
	case WW3D_ZFORMAT_D24S8			: return 8; break;
	case WW3D_ZFORMAT_D16			: return 0; break;
	case WW3D_ZFORMAT_D24X8			: return 0; break;
	case WW3D_ZFORMAT_D24X4S4		: return 4; break;
#ifdef _XBOX
	case WW3D_ZFORMAT_LIN_D24S8	: return 8; break;
	case WW3D_ZFORMAT_LIN_F24S8	: return 8; break;
	case WW3D_ZFORMAT_LIN_D16		: return 0; break;
	case WW3D_ZFORMAT_LIN_F16		: return 0; break;
#endif
	};
	return 0;
};
