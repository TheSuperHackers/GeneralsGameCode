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
 *                     $Archive:: /Commando/Code/ww3d2/formconv.cpp                           $*
 *                                                                                             *
 *              Original Author:: Nathaniel Hoffman                                            *
 *                                                                                             *
 *                       Author : Kenny Mitchell                                               *
 *                                                                                             *
 *                     $Modtime:: 06/27/02 1:27p                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 * 06/27/02 KM Z Format support																						*
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#include "formconv.h"

D3DFORMAT WW3DFormatToD3DFormatConversionArray[WW3D_FORMAT_COUNT] = {
	D3DFMT_UNKNOWN,
	D3DFMT_R8G8B8,
	D3DFMT_A8R8G8B8,
	D3DFMT_X8R8G8B8,
	D3DFMT_R5G6B5,
	D3DFMT_X1R5G5B5,
	D3DFMT_A1R5G5B5,
	D3DFMT_A4R4G4B4,
	D3DFMT_R3G3B2,
	D3DFMT_A8,
	D3DFMT_A8R3G3B2,
	D3DFMT_X4R4G4B4,
	D3DFMT_A8P8,
	D3DFMT_P8,
	D3DFMT_L8,
	D3DFMT_A8L8,
	D3DFMT_A4L4,
	D3DFMT_V8U8,		// Bumpmap
	D3DFMT_L6V5U5,		// Bumpmap
	D3DFMT_X8L8V8U8,	// Bumpmap
	D3DFMT_DXT1,
	D3DFMT_DXT2,
	D3DFMT_DXT3,
	D3DFMT_DXT4,
	D3DFMT_DXT5
};

// adding depth stencil format conversion
D3DFORMAT WW3DZFormatToD3DFormatConversionArray[WW3D_ZFORMAT_COUNT] =
{
#ifndef _XBOX
	D3DFMT_UNKNOWN,
	D3DFMT_D16_LOCKABLE, // 16-bit z-buffer bit depth. This is an application-lockable surface format.
	D3DFMT_D32, // 32-bit z-buffer bit depth.
	D3DFMT_D15S1, // 16-bit z-buffer bit depth where 15 bits are reserved for the depth channel and 1 bit is reserved for the stencil channel.
	D3DFMT_D24S8, // 32-bit z-buffer bit depth using 24 bits for the depth channel and 8 bits for the stencil channel.
	D3DFMT_D16, // 16-bit z-buffer bit depth.
	D3DFMT_D24X8, // 32-bit z-buffer bit depth using 24 bits for the depth channel.
	D3DFMT_D24X4S4, // 32-bit z-buffer bit depth using 24 bits for the depth channel and 4 bits for the stencil channel.
#else
	D3DFMT_UNKNOWN,
	D3DFMT_D16_LOCKABLE, // 16-bit z-buffer bit depth. This is an application-lockable surface format.
	D3DFMT_D32, // 32-bit z-buffer bit depth.
	D3DFMT_D15S1, // 16-bit z-buffer bit depth where 15 bits are reserved for the depth channel and 1 bit is reserved for the stencil channel.
	D3DFMT_D24S8, // 32-bit z-buffer bit depth using 24 bits for the depth channel and 8 bits for the stencil channel.
	D3DFMT_D16, // 16-bit z-buffer bit depth.
	D3DFMT_D24X8, // 32-bit z-buffer bit depth using 24 bits for the depth channel.
	D3DFMT_D24X4S4, // 32-bit z-buffer bit depth using 24 bits for the depth channel and 4 bits for the stencil channel.

	D3DFMT_LIN_D24S8,
	D3DFMT_LIN_F24S8,
	D3DFMT_LIN_D16,
	D3DFMT_LIN_F16
#endif
};


/*
#define HIGHEST_SUPPORTED_D3DFORMAT D3DFMT_X8L8V8U8	//A4L4
WW3DFormat D3DFormatToWW3DFormatConversionArray[HIGHEST_SUPPORTED_D3DFORMAT + 1] = {
	WW3D_FORMAT_UNKNOWN,		// 0
	WW3D_FORMAT_UNKNOWN,	WW3D_FORMAT_UNKNOWN,	WW3D_FORMAT_UNKNOWN,	WW3D_FORMAT_UNKNOWN,
	WW3D_FORMAT_UNKNOWN,	WW3D_FORMAT_UNKNOWN,	WW3D_FORMAT_UNKNOWN,	WW3D_FORMAT_UNKNOWN,
	WW3D_FORMAT_UNKNOWN,	WW3D_FORMAT_UNKNOWN,	WW3D_FORMAT_UNKNOWN,	WW3D_FORMAT_UNKNOWN,
	WW3D_FORMAT_UNKNOWN,	WW3D_FORMAT_UNKNOWN,	WW3D_FORMAT_UNKNOWN,	WW3D_FORMAT_UNKNOWN,
	WW3D_FORMAT_UNKNOWN,	WW3D_FORMAT_UNKNOWN,	WW3D_FORMAT_UNKNOWN,
	WW3D_FORMAT_R8G8B8,		// 20
	WW3D_FORMAT_A8R8G8B8,
	WW3D_FORMAT_X8R8G8B8,
	WW3D_FORMAT_R5G6B5,
	WW3D_FORMAT_X1R5G5B5,
	WW3D_FORMAT_A1R5G5B5,
	WW3D_FORMAT_A4R4G4B4,
	WW3D_FORMAT_R3G3B2,
	WW3D_FORMAT_A8,
	WW3D_FORMAT_A8R3G3B2,
	WW3D_FORMAT_X4R4G4B4,	// 30
	WW3D_FORMAT_UNKNOWN,	WW3D_FORMAT_UNKNOWN,	WW3D_FORMAT_UNKNOWN,	WW3D_FORMAT_UNKNOWN,
	WW3D_FORMAT_UNKNOWN,	WW3D_FORMAT_UNKNOWN,	WW3D_FORMAT_UNKNOWN,	WW3D_FORMAT_UNKNOWN,
	WW3D_FORMAT_UNKNOWN,
	WW3D_FORMAT_A8P8,			// 40
	WW3D_FORMAT_P8,
	WW3D_FORMAT_UNKNOWN,	WW3D_FORMAT_UNKNOWN,	WW3D_FORMAT_UNKNOWN,	WW3D_FORMAT_UNKNOWN,
	WW3D_FORMAT_UNKNOWN,	WW3D_FORMAT_UNKNOWN,	WW3D_FORMAT_UNKNOWN,	WW3D_FORMAT_UNKNOWN,
	WW3D_FORMAT_L8,			// 50
	WW3D_FORMAT_A8L8,
	WW3D_FORMAT_A4L4
};
*/

#ifndef _XBOX
#define HIGHEST_SUPPORTED_D3DFORMAT D3DFMT_X8L8V8U8
#define HIGHEST_SUPPORTED_D3DZFORMAT D3DFMT_D16
#else
#define HIGHEST_SUPPORTED_D3DFORMAT  D3DFMT_LIN_R8G8B8A8
#define HIGHEST_SUPPORTED_D3DZFORMAT    D3DFMT_LIN_F16
#endif
WW3DFormat D3DFormatToWW3DFormatConversionArray[HIGHEST_SUPPORTED_D3DFORMAT + 1];
WW3DZFormat D3DFormatToWW3DZFormatConversionArray[HIGHEST_SUPPORTED_D3DZFORMAT + 1];

D3DFORMAT WW3DFormat_To_D3DFormat(WW3DFormat ww3d_format) {
	if (ww3d_format >= WW3D_FORMAT_COUNT) {
		return D3DFMT_UNKNOWN;
	} else {
		return WW3DFormatToD3DFormatConversionArray[(unsigned int)ww3d_format];
	}
}

WW3DFormat D3DFormat_To_WW3DFormat(D3DFORMAT d3d_format)
{
	switch (d3d_format) {
	// The DXT-codes are created with FOURCC macro and thus can't be placed in the conversion table
	case D3DFMT_DXT1: return WW3D_FORMAT_DXT1;
	case D3DFMT_DXT2: return WW3D_FORMAT_DXT2;
	case D3DFMT_DXT3: return WW3D_FORMAT_DXT3;
	case D3DFMT_DXT4: return WW3D_FORMAT_DXT4;
	case D3DFMT_DXT5: return WW3D_FORMAT_DXT5;
	default:
		if (d3d_format > HIGHEST_SUPPORTED_D3DFORMAT) {
			return WW3D_FORMAT_UNKNOWN;
		} else {
			return D3DFormatToWW3DFormatConversionArray[(unsigned int)d3d_format];
		}
		break;
	}
}

//**********************************************************************************************
//! Depth Stencil W3D to D3D format conversion
/*! KJM
*/
D3DFORMAT WW3DZFormat_To_D3DFormat(WW3DZFormat ww3d_zformat)
{
	if (ww3d_zformat >= WW3D_ZFORMAT_COUNT)
	{
		return D3DFMT_UNKNOWN;
	}
	else
	{
		return WW3DZFormatToD3DFormatConversionArray[(unsigned int)ww3d_zformat];
	}
}

//**********************************************************************************************
//! D3D to Depth Stencil W3D format conversion
/*! KJM
*/
WW3DZFormat D3DFormat_To_WW3DZFormat(D3DFORMAT d3d_format)
{
	if (d3d_format>HIGHEST_SUPPORTED_D3DZFORMAT)
	{
		return WW3D_ZFORMAT_UNKNOWN;
	}
	else
	{
		return D3DFormatToWW3DZFormatConversionArray[(unsigned int)d3d_format];
	}
}

//**********************************************************************************************
//! Init format conversion tables
/*!
 * 06/27/02 KM Z Format support																						*
*/
void Init_D3D_To_WW3_Conversion()
{
	int i=0;
	for (;i<HIGHEST_SUPPORTED_D3DFORMAT;++i) {
		D3DFormatToWW3DFormatConversionArray[i]=WW3D_FORMAT_UNKNOWN;
	}

	D3DFormatToWW3DFormatConversionArray[D3DFMT_R8G8B8]=WW3D_FORMAT_R8G8B8;
	D3DFormatToWW3DFormatConversionArray[D3DFMT_A8R8G8B8]=WW3D_FORMAT_A8R8G8B8;
	D3DFormatToWW3DFormatConversionArray[D3DFMT_X8R8G8B8]=WW3D_FORMAT_X8R8G8B8;
	D3DFormatToWW3DFormatConversionArray[D3DFMT_R5G6B5]=WW3D_FORMAT_R5G6B5;
	D3DFormatToWW3DFormatConversionArray[D3DFMT_X1R5G5B5]=WW3D_FORMAT_X1R5G5B5;
	D3DFormatToWW3DFormatConversionArray[D3DFMT_A1R5G5B5]=WW3D_FORMAT_A1R5G5B5;
	D3DFormatToWW3DFormatConversionArray[D3DFMT_A4R4G4B4]=WW3D_FORMAT_A4R4G4B4;
	D3DFormatToWW3DFormatConversionArray[D3DFMT_R3G3B2]=WW3D_FORMAT_R3G3B2;
	D3DFormatToWW3DFormatConversionArray[D3DFMT_A8]=WW3D_FORMAT_A8;
	D3DFormatToWW3DFormatConversionArray[D3DFMT_A8R3G3B2]=WW3D_FORMAT_A8R3G3B2;
	D3DFormatToWW3DFormatConversionArray[D3DFMT_X4R4G4B4]=WW3D_FORMAT_X4R4G4B4;
	D3DFormatToWW3DFormatConversionArray[D3DFMT_A8P8]=WW3D_FORMAT_A8P8;
	D3DFormatToWW3DFormatConversionArray[D3DFMT_P8]=WW3D_FORMAT_P8;
	D3DFormatToWW3DFormatConversionArray[D3DFMT_L8]=WW3D_FORMAT_L8;
	D3DFormatToWW3DFormatConversionArray[D3DFMT_A8L8]=WW3D_FORMAT_A8L8;
	D3DFormatToWW3DFormatConversionArray[D3DFMT_A4L4]=WW3D_FORMAT_A4L4;
	D3DFormatToWW3DFormatConversionArray[D3DFMT_V8U8]=WW3D_FORMAT_U8V8;				// Bumpmap
	D3DFormatToWW3DFormatConversionArray[D3DFMT_L6V5U5]=WW3D_FORMAT_L6V5U5;		// Bumpmap
	D3DFormatToWW3DFormatConversionArray[D3DFMT_X8L8V8U8]=WW3D_FORMAT_X8L8V8U8;	// Bumpmap

	// init depth stencil conversion
	for (i=0; i<HIGHEST_SUPPORTED_D3DZFORMAT; i++)
	{
		D3DFormatToWW3DZFormatConversionArray[i]=WW3D_ZFORMAT_UNKNOWN;
	}

	D3DFormatToWW3DZFormatConversionArray[D3DFMT_D16_LOCKABLE]=WW3D_ZFORMAT_D16_LOCKABLE;
	D3DFormatToWW3DZFormatConversionArray[D3DFMT_D32]=WW3D_ZFORMAT_D32;
	D3DFormatToWW3DZFormatConversionArray[D3DFMT_D15S1]=WW3D_ZFORMAT_D15S1;
	D3DFormatToWW3DZFormatConversionArray[D3DFMT_D24S8]=WW3D_ZFORMAT_D24S8;
	D3DFormatToWW3DZFormatConversionArray[D3DFMT_D16]=WW3D_ZFORMAT_D16;
	D3DFormatToWW3DZFormatConversionArray[D3DFMT_D24X8]=WW3D_ZFORMAT_D24X8;
	D3DFormatToWW3DZFormatConversionArray[D3DFMT_D24X4S4]=WW3D_ZFORMAT_D24X4S4;
#ifdef _XBOX
	D3DFormatToWW3DZFormatConversionArray[D3DFMT_LIN_D24S8]=WW3D_ZFORMAT_LIN_D24S8;
	D3DFormatToWW3DZFormatConversionArray[D3DFMT_LIN_F24S8]=WW3D_ZFORMAT_LIN_F24S8;
	D3DFormatToWW3DZFormatConversionArray[D3DFMT_LIN_D16]=WW3D_ZFORMAT_LIN_D16;
	D3DFormatToWW3DZFormatConversionArray[D3DFMT_LIN_F16]=WW3D_ZFORMAT_LIN_F16;
#endif
};
