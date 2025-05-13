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
 *                     $Archive:: ww3d2/texturefilter.cpp												$*
 *                                                                                             *
 *                  $Org Author:: Kenny Mitchell                                              $*
 *                                                                                             *
 *                       Author : Kenny Mitchell                                               * 
 *                                                                                             * 
 *                     $Modtime:: 08/05/02 1:27p                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                          $*
 *                                                                                             *
 * 08/05/02 KM Texture filter class abstraction																			*
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "texturefilter.h"
#include "dx8wrapper.h"
#include "meshmatdesc.h"

unsigned _MinTextureFilters[TextureFilterClass::FILTER_TYPE_COUNT];
unsigned _MagTextureFilters[TextureFilterClass::FILTER_TYPE_COUNT];
unsigned _MipMapFilters[TextureFilterClass::FILTER_TYPE_COUNT];

/*************************************************************************
**                             TextureFilterClass
*************************************************************************/
TextureFilterClass::TextureFilterClass(MipCountType mip_level_count=MIP_LEVELS_1)
:	TextureMinFilter(FILTER_TYPE_DEFAULT),
	TextureMagFilter(FILTER_TYPE_DEFAULT),
	UAddressMode(TEXTURE_ADDRESS_REPEAT),
	VAddressMode(TEXTURE_ADDRESS_REPEAT)
{
	if (mip_level_count!=MIP_LEVELS_1)
	{
		MipMapFilter=FILTER_TYPE_DEFAULT;
	}
	else
	{
		MipMapFilter=FILTER_TYPE_NONE;
	}
}

//**********************************************************************************************
//! Apply filters (legacy)
/*!
*/
void TextureFilterClass::Apply(unsigned int stage)
{
	DX8Wrapper::Set_DX8_Texture_Stage_State(stage,D3DTSS_MINFILTER,_MinTextureFilters[TextureMinFilter]);
	DX8Wrapper::Set_DX8_Texture_Stage_State(stage,D3DTSS_MAGFILTER,_MagTextureFilters[TextureMagFilter]);
	DX8Wrapper::Set_DX8_Texture_Stage_State(stage,D3DTSS_MIPFILTER,_MipMapFilters[MipMapFilter]);

	switch (Get_U_Addr_Mode()) 
	{
	case TEXTURE_ADDRESS_REPEAT:
		DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
		break;

	case TEXTURE_ADDRESS_CLAMP:
		DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		break;
	}

	switch (Get_V_Addr_Mode()) 
	{
	case TEXTURE_ADDRESS_REPEAT:
		DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
		break;

	case TEXTURE_ADDRESS_CLAMP:
		DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
		break;
	}
}

//**********************************************************************************************
//! Init filters (legacy)
/*!
*/
void TextureFilterClass::_Init_Filters(void)
{
	const D3DCAPS8& dx8caps=DX8Wrapper::Get_Current_Caps()->Get_DX8_Caps();

	_MinTextureFilters[FILTER_TYPE_NONE]=D3DTEXF_POINT;
	_MagTextureFilters[FILTER_TYPE_NONE]=D3DTEXF_POINT;
	_MipMapFilters[FILTER_TYPE_NONE]=D3DTEXF_NONE;

	_MinTextureFilters[FILTER_TYPE_FAST]=D3DTEXF_LINEAR;
	_MagTextureFilters[FILTER_TYPE_FAST]=D3DTEXF_LINEAR;
	_MipMapFilters[FILTER_TYPE_FAST]=D3DTEXF_POINT;

	// Jani: Disabling anisotropic filtering as it doesn't seem to work with the latest nVidia drivers.
	if (dx8caps.TextureFilterCaps&D3DPTFILTERCAPS_MAGFAFLATCUBIC) _MagTextureFilters[FILTER_TYPE_BEST]=D3DTEXF_FLATCUBIC;
	else if (dx8caps.TextureFilterCaps&D3DPTFILTERCAPS_MAGFANISOTROPIC) _MagTextureFilters[FILTER_TYPE_BEST]=D3DTEXF_ANISOTROPIC;
	else if (dx8caps.TextureFilterCaps&D3DPTFILTERCAPS_MAGFGAUSSIANCUBIC) _MagTextureFilters[FILTER_TYPE_BEST]=D3DTEXF_GAUSSIANCUBIC;
	else if (dx8caps.TextureFilterCaps&D3DPTFILTERCAPS_MAGFLINEAR) _MagTextureFilters[FILTER_TYPE_BEST]=D3DTEXF_LINEAR;
	else if (dx8caps.TextureFilterCaps&D3DPTFILTERCAPS_MAGFPOINT) _MagTextureFilters[FILTER_TYPE_BEST]=D3DTEXF_POINT;
	else {
		WWASSERT_PRINT(0,("No magnification filter found!"));
	}

	if (dx8caps.TextureFilterCaps&D3DPTFILTERCAPS_MINFANISOTROPIC) _MinTextureFilters[FILTER_TYPE_BEST]=D3DTEXF_ANISOTROPIC;
	else if (dx8caps.TextureFilterCaps&D3DPTFILTERCAPS_MINFLINEAR) _MinTextureFilters[FILTER_TYPE_BEST]=D3DTEXF_LINEAR;
	else if (dx8caps.TextureFilterCaps&D3DPTFILTERCAPS_MINFPOINT) _MinTextureFilters[FILTER_TYPE_BEST]=D3DTEXF_POINT;
	else {
		WWASSERT_PRINT(0,("No minification filter found!"));
	}

	if (dx8caps.TextureFilterCaps&D3DPTFILTERCAPS_MIPFLINEAR) _MipMapFilters[FILTER_TYPE_BEST]=D3DTEXF_LINEAR;
	else if (dx8caps.TextureFilterCaps&D3DPTFILTERCAPS_MIPFPOINT) _MipMapFilters[FILTER_TYPE_BEST]=D3DTEXF_POINT;
	else {
		WWASSERT_PRINT(0,("No mip filter found!"));
	}

//_MagTextureFilters[FILTER_TYPE_BEST]=D3DTEXF_FLATCUBIC;
//	WWASSERT(Validate_Filters(FILTER_TYPE_BEST));

	_MinTextureFilters[FILTER_TYPE_DEFAULT]=_MinTextureFilters[FILTER_TYPE_BEST];
	_MagTextureFilters[FILTER_TYPE_DEFAULT]=_MagTextureFilters[FILTER_TYPE_BEST];
	_MipMapFilters[FILTER_TYPE_DEFAULT]=_MipMapFilters[FILTER_TYPE_BEST];

	for (int stage=0;stage<MeshMatDescClass::MAX_TEX_STAGES;++stage) {
		DX8Wrapper::Set_DX8_Texture_Stage_State(stage,D3DTSS_MAXANISOTROPY,2);
	}
}


//**********************************************************************************************
//! Set mip mapping filter (legacy)
/*!
*/
void TextureFilterClass::Set_Mip_Mapping(FilterType mipmap)
{
//	if (mipmap != FILTER_TYPE_NONE && Get_Mip_Level_Count() <= 1 && Is_Initialized()) 
//	{
//		WWASSERT_PRINT(0, "Trying to enable MipMapping on texture w/o Mip levels!\n");
//		return;
//	}
	MipMapFilter=mipmap;
}

//**********************************************************************************************
//! Set default min filter (legacy)
/*!
*/
void TextureFilterClass::_Set_Default_Min_Filter(FilterType filter)
{
	_MinTextureFilters[FILTER_TYPE_DEFAULT]=_MinTextureFilters[filter];
}


//**********************************************************************************************
//! Set default mag filter (legacy)
/*!
*/
void TextureFilterClass::_Set_Default_Mag_Filter(FilterType filter)
{
	_MagTextureFilters[FILTER_TYPE_DEFAULT]=_MagTextureFilters[filter];
}

//**********************************************************************************************
//! Set default mip filter (legacy)
/*!
*/
void TextureFilterClass::_Set_Default_Mip_Filter(FilterType filter)
{
	_MipMapFilters[FILTER_TYPE_DEFAULT]=_MipMapFilters[filter];
}
