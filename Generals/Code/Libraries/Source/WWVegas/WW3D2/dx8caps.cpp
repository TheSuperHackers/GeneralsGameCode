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
 *                 Project Name : dx8 caps                                                     *
 *                                                                                             *
 *                     $Archive:: /VSS_Sync/ww3d2/dx8caps.cpp                                 $*
 *                                                                                             *
 *              Original Author:: Hector Yee                                                   *
 *                                                                                             *
 *                      $Author:: Vss_sync                                                    $*
 *                                                                                             *
 *                     $Modtime:: 8/29/01 8:16p                                               $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"
#include "dx8caps.h"
#include "dx8wrapper.h"
#include "formconv.h"


static D3DCAPS9 hwVPCaps;
static D3DCAPS9 swVPCaps;
static bool UseTnL;
static bool SupportDXTC;
static bool supportGamma;
static bool SupportNPatches;
static bool SupportDOT3;
static bool SupportBumpEnvmap;
static bool SupportBumpEnvmapLuminance;
static bool SupportTextureFormat[WW3D_FORMAT_COUNT];
static int VertexShaderVersion;
static int PixelShaderVersion;
static int MaxSimultaneousTextures;

enum {
	VENDOR_ID_NVIDIA=0x10de,
	VENROD_ID_ATI=0x1002
};

// ----------------------------------------------------------------------------
//
// Init the caps structure
//
// ----------------------------------------------------------------------------

void DX8Caps::Init_Caps(IDirect3DDevice9* D3DDevice)
{
	D3DDevice->SetSoftwareVertexProcessing(TRUE);
	DX8CALL(GetDeviceCaps(&swVPCaps));

	if ((swVPCaps.DevCaps&D3DDEVCAPS_HWTRANSFORMANDLIGHT)==D3DDEVCAPS_HWTRANSFORMANDLIGHT) {
		UseTnL=true;

		D3DDevice->SetSoftwareVertexProcessing(FALSE);
		DX8CALL(GetDeviceCaps(&hwVPCaps));	
	} else {
		UseTnL=false;			
	}
}

// ----------------------------------------------------------------------------
//
// Compute the caps bits
//
// ----------------------------------------------------------------------------

void DX8Caps::Compute_Caps(D3DFORMAT display_format, D3DFORMAT depth_stencil_format, IDirect3DDevice9* D3DDevice)
{
	const D3DADAPTER_IDENTIFIER9& adapter_id=DX8Wrapper::Get_Current_Adapter_Identifier();

	Init_Caps(D3DDevice);

	const D3DCAPS9& caps=Get_Default_Caps();

	if ((caps.DevCaps&D3DDEVCAPS_NPATCHES)==D3DDEVCAPS_NPATCHES) {
		SupportNPatches=true;
	} else {
		SupportNPatches=false;
	}

	if ((caps.TextureOpCaps&D3DTEXOPCAPS_DOTPRODUCT3)==D3DTEXOPCAPS_DOTPRODUCT3) {
		SupportDOT3=true;
	} else {
		SupportDOT3=false;
	}

	supportGamma=((swVPCaps.Caps2&D3DCAPS2_FULLSCREENGAMMA)==D3DCAPS2_FULLSCREENGAMMA);

	Check_Texture_Format_Support(display_format,caps);
	Check_Texture_Compression_Support(caps);
	Check_Bumpmap_Support(caps);
	Check_Shader_Support(caps);
	Check_Maximum_Texture_Support(caps);
	Vendor_Specific_Hacks(adapter_id);
}

// ----------------------------------------------------------------------------
//
// Check bump map texture support
//
// ----------------------------------------------------------------------------

void DX8Caps::Check_Bumpmap_Support(const D3DCAPS9& caps)
{
	SupportBumpEnvmap=!!(caps.TextureOpCaps & D3DTEXOPCAPS_BUMPENVMAP);
	SupportBumpEnvmapLuminance=!!(caps.TextureOpCaps & D3DTEXOPCAPS_BUMPENVMAPLUMINANCE);
}

// ----------------------------------------------------------------------------
//
// Check compressed texture support
//
// ----------------------------------------------------------------------------

void DX8Caps::Check_Texture_Compression_Support(const D3DCAPS9& caps)
{
	SupportDXTC=SupportTextureFormat[WW3D_FORMAT_DXT1]|
		SupportTextureFormat[WW3D_FORMAT_DXT2]|
		SupportTextureFormat[WW3D_FORMAT_DXT3]|
		SupportTextureFormat[WW3D_FORMAT_DXT4]|
		SupportTextureFormat[WW3D_FORMAT_DXT5];
}

void DX8Caps::Check_Texture_Format_Support(D3DFORMAT display_format,const D3DCAPS9& caps)
{
	for (unsigned i=0;i<WW3D_FORMAT_COUNT;++i) {
		if (i==WW3D_FORMAT_UNKNOWN) {
			SupportTextureFormat[i]=false;
		}
		else {
			SupportTextureFormat[i]=SUCCEEDED(
				DX8Wrapper::_Get_D3D8()->CheckDeviceFormat(
					caps.AdapterOrdinal,
					caps.DeviceType,
					display_format,
					0,
					D3DRTYPE_TEXTURE,
					WW3DFormat_To_D3DFormat((WW3DFormat)i)));
		}
	}
}

void DX8Caps::Check_Maximum_Texture_Support(const D3DCAPS9& caps)
{
	MaxSimultaneousTextures=caps.MaxSimultaneousTextures;
}

void DX8Caps::Check_Shader_Support(const D3DCAPS9& caps)
{
	VertexShaderVersion=caps.VertexShaderVersion;
	PixelShaderVersion=caps.PixelShaderVersion;
}

// ----------------------------------------------------------------------------
//
// Implement some vendor-specific hacks to fix certain driver bugs that can't be
// avoided otherwise.
//
// ----------------------------------------------------------------------------

void DX8Caps::Vendor_Specific_Hacks(const D3DADAPTER_IDENTIFIER9& adapter_id)
{
	if (adapter_id.VendorId==VENDOR_ID_NVIDIA) {
		SupportNPatches = false;	// Driver incorrectly report N-Patch support
		SupportTextureFormat[WW3D_FORMAT_DXT1] = false;			// DXT1 is broken on NVidia hardware
		SupportDXTC=
			SupportTextureFormat[WW3D_FORMAT_DXT1]|
			SupportTextureFormat[WW3D_FORMAT_DXT2]|
			SupportTextureFormat[WW3D_FORMAT_DXT3]|
			SupportTextureFormat[WW3D_FORMAT_DXT4]|
			SupportTextureFormat[WW3D_FORMAT_DXT5];
	}

//	SupportDXTC=false;

}

bool DX8Caps::Use_TnL() { return UseTnL; };
bool DX8Caps::Support_DXTC() { return SupportDXTC; }
bool DX8Caps::Support_Gamma() { return supportGamma; }
bool DX8Caps::Support_NPatches() { return SupportNPatches; }
bool DX8Caps::Support_DOT3() { return SupportDOT3; }
bool	DX8Caps::Support_Bump_Envmap() { return SupportBumpEnvmap; }
bool	DX8Caps::Support_Bump_Envmap_Luminance() { return SupportBumpEnvmapLuminance; }
int DX8Caps::Get_Vertex_Shader_Minor_Version() { return 0xff & (VertexShaderVersion); }
int DX8Caps::Get_Pixel_Shader_Majon_Version() { return 0xff & (PixelShaderVersion >> 8); }
int DX8Caps::Get_Pixel_Shader_Minor_Version() { return 0xff & (PixelShaderVersion); }
int DX8Caps::Get_Max_Simultaneous_Textures() { return MaxSimultaneousTextures; }

bool DX8Caps::Support_Texture_Format(WW3DFormat format) { return SupportTextureFormat[format]; }

D3DCAPS9 const& DX8Caps::Get_HW_VP_Caps() { return hwVPCaps; };
D3DCAPS9 const& DX8Caps::Get_SW_VP_Caps() { return swVPCaps; };
D3DCAPS9 const& DX8Caps::Get_Default_Caps() { return (UseTnL ? hwVPCaps : swVPCaps); };