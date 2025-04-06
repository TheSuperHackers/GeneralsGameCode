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
#include <d3d8.h>

class DX8Caps
{
public:
	enum DriverVersionStatusType {
		DRIVER_STATUS_UNKNOWN,
		DRIVER_STATUS_GOOD,
		DRIVER_STATUS_OK,
		DRIVER_STATUS_BAD
	};

	enum VendorIdType {
		VENDOR_UNKNOWN,
		VENDOR_NVIDIA,
		VENDOR_ATI,
		VENDOR_INTEL,
		VENDOR_S3,
		VENDOR_POWERVR,
		VENDOR_MATROX,
		VENDOR_3DFX,
		VENDOR_3DLABS,
		VENDOR_CIRRUSLOGIC,
		VENDOR_RENDITION
	};

	enum DeviceTypeATI {
		DEVICE_ATI_UNKNOWN,
		DEVICE_ATI_RAGE_II,
		DEVICE_ATI_RAGE_II_PLUS,
		DEVICE_ATI_RAGE_IIC_PCI,
		DEVICE_ATI_RAGE_IIC_AGP,
		DEVICE_ATI_RAGE_128_MOBILITY,
		DEVICE_ATI_RAGE_128_MOBILITY_M3,
		DEVICE_ATI_RAGE_128_MOBILITY_M4,
		DEVICE_ATI_RAGE_128_PRO_ULTRA,
		DEVICE_ATI_RAGE_128_4X,
		DEVICE_ATI_RAGE_128_PRO_GL,
		DEVICE_ATI_RAGE_128_PRO_VR,
		DEVICE_ATI_RAGE_128_GL,
		DEVICE_ATI_RAGE_128_VR,
		DEVICE_ATI_RAGE_PRO,
		DEVICE_ATI_RAGE_PRO_MOBILITY,
		DEVICE_ATI_MOBILITY_RADEON,
		DEVICE_ATI_MOBILITY_RADEON_VE_M6,
		DEVICE_ATI_RADEON_VE,
		DEVICE_ATI_RADEON_DDR,
		DEVICE_ATI_RADEON,
		DEVICE_ATI_MOBILITY_R7500,
		DEVICE_ATI_R7500,
		DEVICE_ATI_R8500
	};

	enum DeviceType3DLabs {
		DEVICE_3DLABS_UNKNOWN,
		DEVICE_3DLABS_PERMEDIA,
		DEVICE_3DLABS_300SX,
		DEVICE_3DLABS_500TX,
		DEVICE_3DLABS_DELTA,
		DEVICE_3DLABS_MX,
		DEVICE_3DLABS_GAMMA,
		DEVICE_3DLABS_PERMEDIA2S_ST,
		DEVICE_3DLABS_PERMEDIA3,
		DEVICE_3DLABS_R3,
		DEVICE_3DLABS_PERMEDIA4,
		DEVICE_3DLABS_R4,
		DEVICE_3DLABS_G2,
		DEVICE_3DLABS_OXYGEN_VX1,
		DEVICE_3DLABS_TI_P1,
		DEVICE_3DLABS_PERMEDIA2
	};

	enum DeviceTypeNVidia {
		DEVICE_NVIDIA_UNKNOWN,
		DEVICE_NVIDIA_GEFORCE3,
		DEVICE_NVIDIA_QUADRO2_PRO,
		DEVICE_NVIDIA_GEFORCE2_GO,
		DEVICE_NVIDIA_GEFORCE2_ULTRA,
		DEVICE_NVIDIA_GEFORCE2_GTS,
		DEVICE_NVIDIA_QUADRO,
		DEVICE_NVIDIA_GEFORCE_DDR,
		DEVICE_NVIDIA_GEFORCE_256,
		DEVICE_NVIDIA_TNT2_ALADDIN,
		DEVICE_NVIDIA_TNT2,
		DEVICE_NVIDIA_TNT2_ULTRA,
		DEVICE_NVIDIA_TNT2_VANTA,
		DEVICE_NVIDIA_TNT2_M64,
		DEVICE_NVIDIA_TNT,
		DEVICE_NVIDIA_RIVA_128,
		DEVICE_NVIDIA_TNT_VANTA,
		DEVICE_NVIDIA_NV1,
		DEVICE_NVIDIA_GEFORCE2_MX,
		DEVICE_NVIDIA_GEFORCE4_TI_4600,
		DEVICE_NVIDIA_GEFORCE4_TI_4400,
		DEVICE_NVIDIA_GEFORCE4_TI,
		DEVICE_NVIDIA_GEFORCE4_TI_4200,
		DEVICE_NVIDIA_GEFORCE4_MX_460,
		DEVICE_NVIDIA_GEFORCE4_MX_440,
		DEVICE_NVIDIA_GEFORCE4_MX_420,
		DEVICE_NVIDIA_GEFORCE4,
		DEVICE_NVIDIA_GEFORCE4_GO_440,
		DEVICE_NVIDIA_GEFORCE4_GO_420,
		DEVICE_NVIDIA_GEFORCE4_GO_420_32M,
		DEVICE_NVIDIA_GEFORCE4_GO_440_64M,
		DEVICE_NVIDIA_GEFORCE4_GO,
		DEVICE_NVIDIA_GEFORCE3_TI_500,
		DEVICE_NVIDIA_GEFORCE3_TI_200,
		DEVICE_NVIDIA_GEFORCE2_INTEGRATED,
		DEVICE_NVIDIA_GEFORCE2_TI,
		DEVICE_NVIDIA_QUADRO2_MXR_EX_GO,
		DEVICE_NVIDIA_GEFORCE2_MX_100_200,
		DEVICE_NVIDIA_GEFORCE2_MX_400,
		DEVICE_NVIDIA_QUADRO_DCC
	};

	enum DeviceType3Dfx {
		DEVICE_3DFX_UNKNOWN,
		DEVICE_3DFX_VOODOO_5500_AGP,
		DEVICE_3DFX_VOODOO_3,
		DEVICE_3DFX_BANSHEE,
		DEVICE_3DFX_VOODOO_2,
		DEVICE_3DFX_VOODOO_GRAPHICS,
		DEVICE_3DFX_VOODOO_RUSH
	};

	enum DeviceTypeMatrox {
		DEVICE_MATROX_UNKNOWN,
		DEVICE_MATROX_G550,
		DEVICE_MATROX_G400,
		DEVICE_MATROX_G200_AGP,
		DEVICE_MATROX_G200_PCI,
		DEVICE_MATROX_G100_PCI,
		DEVICE_MATROX_G100_AGP,
		DEVICE_MATROX_MILLENNIUM_II_AGP,
		DEVICE_MATROX_MILLENNIUM_II_PCI,
		DEVICE_MATROX_MYSTIQUE,
		DEVICE_MATROX_MILLENNIUM,
		DEVICE_MATROX_PARHELIA,
		DEVICE_MATROX_PARHELIA_AGP8X
	};

	enum DeviceTypePowerVR {
		DEVICE_POWERVR_UNKNOWN,
		DEVICE_POWERVR_KYRO
	};

	enum DeviceTypeS3 {
		DEVICE_S3_UNKNOWN,
		DEVICE_S3_SAVAGE_MX,
		DEVICE_S3_SAVAGE_4,
		DEVICE_S3_SAVAGE_200
	};

	enum DeviceTypeIntel {
		DEVICE_INTEL_UNKNOWN,
		DEVICE_INTEL_810,
		DEVICE_INTEL_810E,
		DEVICE_INTEL_815
	};


	static void Compute_Caps(D3DFORMAT display_format, D3DFORMAT depth_stencil_format, IDirect3DDevice8* D3DDevice);
	static bool Use_TnL() { return UseTnL; };	
	static bool Support_DXTC() { return SupportDXTC; }
	static bool Support_Gamma() { return supportGamma; }
	static bool Support_NPatches() { return SupportNPatches; }
	static bool Support_DOT3() { return SupportDOT3; }
	static bool	Support_Bump_Envmap() { return SupportBumpEnvmap; }
	static bool	Support_Bump_Envmap_Luminance() { return SupportBumpEnvmapLuminance; }

	// -------------------------------------------------------------------------
	//
	// Vertex shader support. Version number is split in major and minor, such that 1.0 would
	// have 1 as major and 0 as minor version number.
	//
	// -------------------------------------------------------------------------

	static int Get_Vertex_Shader_Major_Version() { return 0xff&(VertexShaderVersion>>8); }
	static int Get_Vertex_Shader_Minor_Version() { return 0xff&(VertexShaderVersion); }
	static int Get_Pixel_Shader_Major_Version() { return 0xff&(PixelShaderVersion>>8); }
	static int Get_Pixel_Shader_Minor_Version() { return 0xff&(PixelShaderVersion); }
	static int Get_Max_Simultaneous_Textures()	{ return MaxSimultaneousTextures;}

	static bool Support_Texture_Format(WW3DFormat format) { return SupportTextureFormat[format]; }

	static D3DCAPS8 const & Get_HW_VP_Caps() { return hwVPCaps; };
	static D3DCAPS8 const & Get_SW_VP_Caps() { return swVPCaps; };
	static D3DCAPS8 const & Get_Default_Caps() { return (UseTnL?hwVPCaps:swVPCaps); };

private:
	static void Init_Caps(IDirect3DDevice8* D3DDevice);
	static void Check_Texture_Format_Support(D3DFORMAT display_format,const D3DCAPS8& caps);
	static void Check_Texture_Compression_Support(const D3DCAPS8& caps);
	static void Check_Bumpmap_Support(const D3DCAPS8& caps);
	static void Check_Shader_Support(const D3DCAPS8& caps);
	static void Check_Maximum_Texture_Support(const D3DCAPS8& caps);
	static void Vendor_Specific_Hacks(const D3DADAPTER_IDENTIFIER8& adapter_id);

	static D3DCAPS8 hwVPCaps;
	static D3DCAPS8 swVPCaps;
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
};

#endif