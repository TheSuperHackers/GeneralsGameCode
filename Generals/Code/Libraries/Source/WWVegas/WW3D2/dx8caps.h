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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : DX8 Caps                                                     *
 *                                                                                             *
 *                     $Archive:: /VSS_Sync/ww3d2/dx8caps.h                                   $*
 *                                                                                             *
 *              Original Author:: Hector Yee                                                   *
 *                                                                                             *
 *                      $Author:: Vss_sync                                                    $*
 *                                                                                             *
 *                     $Modtime:: 8/29/01 8:16p                                               $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef DX8CAPS_H
#define DX8CAPS_H

#include "always.h"
#include "ww3dformat.h"
#include <d3d9.h>

class DX8Caps
{
public:
	static void Compute_Caps(D3DFORMAT display_format, D3DFORMAT depth_stencil_format, IDirect3DDevice9* D3DDevice);
	static bool Use_TnL();// { return UseTnL; };
	static bool Support_DXTC();// { return SupportDXTC; }
	static bool Support_Gamma();// { return supportGamma; }
	static bool Support_NPatches();// { return SupportNPatches; }
	static bool Support_DOT3();// { return SupportDOT3; }
	static bool	Support_Bump_Envmap();// { return SupportBumpEnvmap; }
	static bool	Support_Bump_Envmap_Luminance();// { return SupportBumpEnvmapLuminance; }

	// -------------------------------------------------------------------------
	//
	// Vertex shader support. Version number is split in major and minor, such that 1.0 would
	// have 1 as major and 0 as minor version number.
	//
	// -------------------------------------------------------------------------

	static int Get_Vertex_Shader_Majon_Version();// { return 0xff & (VertexShaderVersion >> 8); }
	static int Get_Vertex_Shader_Minor_Version();// { return 0xff&(VertexShaderVersion); }
	static int Get_Pixel_Shader_Majon_Version();// { return 0xff&(PixelShaderVersion>>8); }
	static int Get_Pixel_Shader_Minor_Version();// { return 0xff&(PixelShaderVersion); }
	static int Get_Max_Simultaneous_Textures();// { return MaxSimultaneousTextures; }

	static bool Support_Texture_Format(WW3DFormat format);// { return SupportTextureFormat[format]; }

	static D3DCAPS9 const & Get_HW_VP_Caps();// { return hwVPCaps; };
	static D3DCAPS9 const & Get_SW_VP_Caps();// { return swVPCaps; };
	static D3DCAPS9 const & Get_Default_Caps();// { return (UseTnL?hwVPCaps:swVPCaps); };

private:
	static void Init_Caps(IDirect3DDevice9* D3DDevice);
	static void Check_Texture_Format_Support(D3DFORMAT display_format,const D3DCAPS9& caps);
	static void Check_Texture_Compression_Support(const D3DCAPS9& caps);
	static void Check_Bumpmap_Support(const D3DCAPS9& caps);
	static void Check_Shader_Support(const D3DCAPS9& caps);
	static void Check_Maximum_Texture_Support(const D3DCAPS9& caps);
	static void Vendor_Specific_Hacks(const D3DADAPTER_IDENTIFIER9& adapter_id);
};

#endif