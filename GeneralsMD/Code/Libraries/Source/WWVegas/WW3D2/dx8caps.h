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
 *                 Project Name : DX8 Caps                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/dx8caps.h                              $*
 *                                                                                             *
 *              Original Author:: Hector Yee                                                   *
 *                                                                                             *
 *                       Author : Kenny Mitchell                                               *
 *                                                                                             *
 *                     $Modtime:: 06/27/02 1:27p                                              $*
 *                                                                                             *
 *                    $Revision:: 24                                                          $*
 *                                                                                             *
 * 06/27/02 KM Z Format support																						*
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

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
		VENDOR_RENDITION,

		VENDOR_COUNT
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


	DX8Caps(IDirect3D8* direct3d, const D3DCAPS8& caps,WW3DFormat display_format, const D3DADAPTER_IDENTIFIER8& adapter_id);
	DX8Caps(IDirect3D8* direct3d, IDirect3DDevice8* D3DDevice,WW3DFormat display_format, const D3DADAPTER_IDENTIFIER8& adapter_id);
	static void Shutdown(void);

	void Compute_Caps(WW3DFormat display_format, const D3DADAPTER_IDENTIFIER8& adapter_id);
	bool Support_TnL() const { return SupportTnL; };
	bool Support_DXTC() const { return SupportDXTC; }
	bool Support_Gamma() const { return supportGamma; }
	bool Support_NPatches() const { return SupportNPatches; }
	bool Support_Bump_Envmap() const { return SupportBumpEnvmap; }
	bool Support_Bump_Envmap_Luminance() const { return SupportBumpEnvmapLuminance; }
	bool Support_ZBias() const { return SupportZBias; }
	bool Support_Anisotropic_Filtering() const { return SupportAnisotropicFiltering; }
	bool Support_ModAlphaAddClr() const { return SupportModAlphaAddClr; }
	bool Support_Dot3() const { return SupportDot3; }
	bool Support_PointSprites() const { return SupportPointSprites; }
	bool Support_Cubemaps() const { return SupportCubemaps; }
	bool Can_Do_Multi_Pass() const { return CanDoMultiPass; }
	bool Is_Fog_Allowed() const { return IsFogAllowed; }

	bool Is_Valid_Display_Format(int width, int height, WW3DFormat format);

	int Get_Max_Textures_Per_Pass() const { return MaxTexturesPerPass; }

	// -------------------------------------------------------------------------
	//
	// Vertex shader support. Version number is split in major and minor, such that 1.0 would
	// have 1 as major and 0 as minor version number.
	//
	// -------------------------------------------------------------------------

	int Get_Vertex_Shader_Major_Version() const { return 0xff&(VertexShaderVersion>>8); }
	int Get_Vertex_Shader_Minor_Version() const { return 0xff&(VertexShaderVersion); }
	int Get_Pixel_Shader_Major_Version() const { return 0xff&(PixelShaderVersion>>8); }
	int Get_Pixel_Shader_Minor_Version() const { return 0xff&(PixelShaderVersion); }
	int Get_Max_Simultaneous_Textures()	const { return MaxSimultaneousTextures;}

	bool Support_Texture_Format(WW3DFormat format) const { return SupportTextureFormat[format]; }
	bool Support_Render_To_Texture_Format(WW3DFormat format) const { return SupportRenderToTextureFormat[format]; }
	bool Support_Depth_Stencil_Format(WW3DZFormat format) const { return SupportDepthStencilFormat[format]; }

	D3DCAPS8 const & Get_DX8_Caps() const { return Caps; }

	const StringClass& Get_Log() const { return CapsLog; }
	const StringClass& Get_Compact_Log() const { return CompactLog; }

	unsigned Get_Vendor() const { return VendorId; }
	unsigned Get_Device() const { return DeviceId; }
	const StringClass& Get_Driver_Name() const { return DriverDLL; }
	unsigned Get_Driver_Build_Version() const { return DriverBuildVersion; }

	// This will return false if the driver version is known to have problems.
	DriverVersionStatusType Get_Driver_Version_Status() { return DriverVersionStatus; }

private:
	static VendorIdType Define_Vendor(unsigned vendor_id);
	static DeviceTypeATI Get_ATI_Device(unsigned device_id);
	static DeviceType3DLabs Get_3DLabs_Device(unsigned device_id);
	static DeviceTypeNVidia Get_NVidia_Device(unsigned device_id);
	static DeviceType3Dfx Get_3Dfx_Device(unsigned device_id);
	static DeviceTypeMatrox Get_Matrox_Device(unsigned device_id);
	static DeviceTypePowerVR Get_PowerVR_Device(unsigned device_id);
	static DeviceTypeS3 Get_S3_Device(unsigned device_id);
	static DeviceTypeIntel Get_Intel_Device(unsigned device_id);

	void Init_Caps(IDirect3DDevice8* D3DDevice);
	void Check_Texture_Format_Support(WW3DFormat display_format,const D3DCAPS8& caps);
	void Check_Render_To_Texture_Support(WW3DFormat display_format,const D3DCAPS8& caps);
	void Check_Depth_Stencil_Support(WW3DFormat display_format, const D3DCAPS8& caps);
	void Check_Texture_Compression_Support(const D3DCAPS8& caps);
	void Check_Bumpmap_Support(const D3DCAPS8& caps);
	void Check_Shader_Support(const D3DCAPS8& caps);
	void Check_Maximum_Texture_Support(const D3DCAPS8& caps);
	void Check_Driver_Version_Status();
	void Vendor_Specific_Hacks(const D3DADAPTER_IDENTIFIER8& adapter_id);

	int MaxDisplayWidth;
	int MaxDisplayHeight;

	D3DCAPS8 Caps;
	bool SupportTnL;
	bool SupportDXTC;
	bool supportGamma;
	bool SupportNPatches;
	bool SupportBumpEnvmap;
	bool SupportBumpEnvmapLuminance;
	bool SupportTextureFormat[WW3D_FORMAT_COUNT];
	bool SupportRenderToTextureFormat[WW3D_FORMAT_COUNT];
	bool SupportDepthStencilFormat[WW3D_ZFORMAT_COUNT];
	bool SupportZBias;
	bool SupportAnisotropicFiltering;
	bool SupportModAlphaAddClr;
	bool SupportDot3;
	bool SupportPointSprites;
	bool SupportCubemaps;
	bool CanDoMultiPass;
	bool IsFogAllowed;
	int MaxTexturesPerPass;
	int VertexShaderVersion;
	int PixelShaderVersion;
	int MaxSimultaneousTextures;
	unsigned DeviceId;
	unsigned DriverBuildVersion;
	DriverVersionStatusType DriverVersionStatus;
	VendorIdType VendorId;
	StringClass DriverDLL;
	IDirect3D8* Direct3D; // warning XDK name conflict KJM
	StringClass CapsLog;
	StringClass CompactLog;
};


#endif
