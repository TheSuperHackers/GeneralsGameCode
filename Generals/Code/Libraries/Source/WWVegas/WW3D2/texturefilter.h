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
 *                     $Archive:: ww3d2/texturefilter.h												$*
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

#ifndef TEXTUREFILTER_H
#define TEXTUREFILTER_H

#ifndef DX8_WRAPPER_H
//#include "dx8wrapper.h"
#endif

enum MipCountType 
{
	MIP_LEVELS_ALL=0,		// generate all mipmap levels down to 1x1 size
	MIP_LEVELS_1,			// no mipmapping at all (just one mip level)
	MIP_LEVELS_2,
	MIP_LEVELS_3,
	MIP_LEVELS_4,
	MIP_LEVELS_5,
	MIP_LEVELS_6,
	MIP_LEVELS_7,
	MIP_LEVELS_8,
	MIP_LEVELS_10,
	MIP_LEVELS_11,
	MIP_LEVELS_12,
	MIP_LEVELS_MAX			// This isn't to be used (use MIP_LEVELS_ALL instead), it is just an enum for creating static tables etc.
};


// NOTE: Since "texture wrapping" (NOT TEXTURE WRAP MODE - THIS IS
// SOMETHING ELSE) is a global state that affects all texture stages,
// and this class only affects its own stage, we will not worry about
// it for now. Later (probably when we implement world-oriented
// environment maps) we will consider where to put it.

// This is legacy and should be phased out into wwshade shader states
// keeping as an abstracted class for now to support this transition later
class TextureFilterClass
{
public:

	enum FilterType 
	{
		FILTER_TYPE_NONE,
		FILTER_TYPE_FAST,
		FILTER_TYPE_BEST,
		FILTER_TYPE_DEFAULT,
		FILTER_TYPE_COUNT
	};

	enum TextureFilterMode
	{
		TEXTURE_FILTER_BILINEAR,
		TEXTURE_FILTER_TRILINEAR,
		TEXTURE_FILTER_ANISOTROPIC
	};

	enum TxtAddrMode
	{
		TEXTURE_ADDRESS_REPEAT=0,
		TEXTURE_ADDRESS_CLAMP
	};

	TextureFilterClass(MipCountType mip_level_count);

	void Apply(unsigned int stage);

	// Filter and MIPmap settings:
	FilterType Get_Min_Filter(void) const { return TextureMinFilter; }
	FilterType Get_Mag_Filter(void) const { return TextureMagFilter; }
	FilterType Get_Mip_Mapping(void) const { return MipMapFilter; }
	void Set_Min_Filter(FilterType filter) { TextureMinFilter=filter; }
	void Set_Mag_Filter(FilterType filter) { TextureMagFilter=filter; }
	void Set_Mip_Mapping(FilterType mipmap);

	// Texture address mode
	TxtAddrMode Get_U_Addr_Mode(void) const { return UAddressMode; }
	TxtAddrMode Get_V_Addr_Mode(void) const { return VAddressMode; }
	void Set_U_Addr_Mode(TxtAddrMode mode) { UAddressMode=mode; }
	void Set_V_Addr_Mode(TxtAddrMode mode) { VAddressMode=mode; }

	// This needs to be called after device has been created
	static void _Init_Filters(void);

	static void _Set_Default_Min_Filter(FilterType filter);
	static void _Set_Default_Mag_Filter(FilterType filter);
	static void _Set_Default_Mip_Filter(FilterType filter);

private:
	// State not contained in the Direct3D texture object:
	FilterType TextureMinFilter;
	FilterType TextureMagFilter;
	FilterType MipMapFilter;
	TxtAddrMode UAddressMode;
	TxtAddrMode VAddressMode;
};

#endif
