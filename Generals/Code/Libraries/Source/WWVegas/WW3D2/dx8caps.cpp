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
 *                     $Archive:: /Commando/Code/ww3d2/dx8caps.cpp                            $*
 *                                                                                             *
 *              Original Author:: Hector Yee                                                   *
 *                                                                                             *
 *                       Author : Kenny Mitchell                                               *
 *                                                                                             *
 *                     $Modtime:: 06/27/02 1:27p                                              $*
 *                                                                                             *
 *                    $Revision:: 31                                                          $*
 *                                                                                             *
 * 06/27/02 KM Z Format support																						*
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"
#include "dx8caps.h"
#include "dx8wrapper.h"
#include "formconv.h"
#pragma warning (disable : 4201)		// nonstandard extension - nameless struct
#include <windows.h>
#include <mmsystem.h>

static StringClass CapsWorkString;


enum {
	VENDOR_ID_NVIDIA=0x10de,
	VENROD_ID_ATI=0x1002
};


static const char* VendorNames[]={
	"Unknown",
	"NVidia",
	"ATI",
	"Intel",
	"S3",
	"PowerVR",
	"Matrox",
	"3Dfx",
	"3DLabs",
	"CirrusLogic",
	"Rendition"
};

DX8Caps::VendorIdType DX8Caps::Define_Vendor(unsigned vendor_id)
{
	switch (vendor_id) {
	case 0x3d3d:
	case 0x104c: return VENDOR_3DLABS;
	case 0x12D2: // STB - NVIDIA's Riva128
	case 0x14AF: // Guillemot's NVIDIA based cards
	case 0x10de: return VENDOR_NVIDIA;
	case 0x1002: return VENDOR_ATI;
	case 0x8086: return VENDOR_INTEL;
	case 0x5333: return VENDOR_S3;
	case 0x104A: return VENDOR_POWERVR;
	case 0x102B: return VENDOR_MATROX;
	case 0x1142: // Alliance based reference cards
	case 0x109D: // Macronix based reference cards
	case 0x121A: return VENDOR_3DFX;
	default:
		return VENDOR_UNKNOWN;
	}
}

static const char* DeviceNamesNVidia[]={
	"Unknown NVidia device",
	"GeForce3",
	"Quadro2 PRO",
	"GeForce2 Go",
	"GeForce2 ULTRA",
	"GeForce2 GTS",
	"Quadro",
	"GeForce DDR",
	"GeForce 256",
	"TNT2 Aladdin",
	"TNT2",
	"TNT2 ULTRA",
	"TNT2 Vanta",
	"TNT2 M64",
	"TNT",
	"RIVA 128",
	"TNT Vanta",
	"NV1",
	"GeForce2 MX",
	"GeForce4 Ti 4600",
	"GeForce4 Ti 4400",
	"GeForce4 Ti",
	"GeForce4 Ti 4200",
	"GeForce4 MX 460",
	"GeForce4 MX 440",
	"GeForce4 MX 420",
	"GeForce4",
	"GeForce4 Go 440",
	"GeForce4 Go 420",
	"GeForce4 Go 420 32M",
	"GeForce4 Go 440 64M",
	"GeForce4 Go",
	"GeForce3 Ti 500",
	"GeForce3 Ti 200",
	"GeForce2 Integrated",
	"GeForce2 Ti",
	"Quadro2 MXR//EX//GO",
	"GeFORCE2_MX 100//200",
	"GeFORCE2_MX 400",
	"Quadro DCC"
};

static const char* DeviceNamesATI[]={
	"Unknown ATI Device",
	"Rage II",
	"Rage II+",
	"Rage IIc PCI",
	"Rage IIc AGP",
	"Rage 128 Mobility",
	"Rage 128 Mobility M3",
	"Rage 128 Mobility M4",
	"Rage 128 PRO ULTRA",
	"Rage 128 4X",
	"Rage 128 PRO GL",
	"Rage 128 PRO VR",
	"Rage 128 GL",
	"Rage 128 VR",
	"Rage PRO",
	"Rage PRO Mobility",
	"Mobility Radeon",
	"Mobility Radeon VE(M6)",
	"Radeon VE",
	"Radeon DDR",
	"Radeon",
	"Mobility R7500",
	"R7500",
	"R8500"
};

static const char* DeviceNames3DLabs[]={
	"Unknown 3DLabs Device",
	"Permedia",
	"300SX",
	"500TX",
	"Delta",
	"MX",
	"Gamma",
	"Permedia2S (ST)",
	"Permedia3",
	"R3",
	"Permedia4",
	"R4",
	"G2",
	"Oxygen VX1",
	"TI P1",
	"Permedia2"
};

static const char* DeviceNames3Dfx[]={
	"Unknown 3Dfx Device",
	"Voodoo 5500 AGP",
	"Voodoo 3",
	"Banshee",
	"Voodoo 2",
	"Voodoo Graphics",
	"Voodoo Rush"
};

static const char* DeviceNamesMatrox[]={
	"Unknown Matrox Device",
	"G550",
	"G400",
	"G200 AGP",
	"G200 PCI",
	"G100 PCI",
	"G100 AGP",
	"Millennium II AGP",
	"Millennium II PCI",
	"Mystique",
	"Millennium",
	"Parhelia",
	"Parhelia AGP 8X"
};

static const char* DeviceNamesPowerVR[]={
	"Unknown PowerVR Device",
	"Kyro"
};

static const char* DeviceNamesS3[]={
	"Unknown S3 Device",
	"Savage MX",
	"Savage 4",
	"Savage 200"
};

static const char* DeviceNamesIntel[]={
	"Unknown Intel Device",
	"i810",
	"i810e",
	"i815"
};

DX8Caps::DeviceTypeATI DX8Caps::Get_ATI_Device(unsigned device_id)
{
	switch (device_id) {
	case 0x4754: return DEVICE_ATI_RAGE_II;
	case 0x4755: return DEVICE_ATI_RAGE_II_PLUS;
	case 0x5656: return DEVICE_ATI_RAGE_IIC_PCI;
	case 0x4756: return DEVICE_ATI_RAGE_IIC_PCI;
	case 0x475A: return DEVICE_ATI_RAGE_IIC_AGP;
	case 0x4759: return DEVICE_ATI_RAGE_IIC_AGP;
	case 0x4757: return DEVICE_ATI_RAGE_IIC_AGP;
	case 0x4742:
	case 0x4744:
	case 0x4749:
	case 0x4750:
	case 0x474C:
	case 0x474E:
	case 0x474D:
	case 0x474F:
	case 0x4752: return DEVICE_ATI_RAGE_PRO;

	case 0x4C4D:
	case 0x4C52:
	case 0x4C42:
	case 0x4C49:
	case 0x4C50: return DEVICE_ATI_RAGE_PRO_MOBILITY;

	case 0x4C57: return DEVICE_ATI_MOBILITY_R7500;

	case 0x4C59:
	case 0x4C5A: return DEVICE_ATI_MOBILITY_RADEON_VE_M6;

	case 0x4D46:
	case 0x4D4C: return DEVICE_ATI_RAGE_128_MOBILITY_M4;
	case 0x4C45:
	case 0x4C46: return DEVICE_ATI_RAGE_128_MOBILITY_M3;

	case 0x5041:
	case 0x5042:
	case 0x5043:
	case 0x5044:
	case 0x5045:
	case 0x5046: return DEVICE_ATI_RAGE_128_PRO_GL;

	case 0x5047:
	case 0x5048:
	case 0x5049:
	case 0x504A:
	case 0x504B:
	case 0x504C:
	case 0x504D:
	case 0x504E:
	case 0x504F:
	case 0x5050:
	case 0x5051:
	case 0x5052:
	case 0x5053:
	case 0x5054:
	case 0x5055:
	case 0x5056:
	case 0x5057:
	case 0x5058: return DEVICE_ATI_RAGE_128_PRO_VR;

	case 0x5159:
	case 0x515A: return DEVICE_ATI_RADEON_VE;

	case 0x5144:
	case 0x5145:
	case 0x5146:
	case 0x5147: return DEVICE_ATI_RADEON_DDR;

	case 0x514c:
	case 0x514e:
	case 0x514f: return DEVICE_ATI_R8500;

	case 0x5157: return DEVICE_ATI_R7500;

	case 0x5245:
	case 0x5246:
	case 0x534B:
	case 0x534C:
	case 0x534D: return DEVICE_ATI_RAGE_128_GL;

	case 0x524B:
	case 0x524C:
	case 0x5345:
	case 0x5346:
	case 0x5347: return DEVICE_ATI_RAGE_128_VR;

	case 0x5446:
	case 0x544C:
	case 0x5452:
	case 0x5453:
	case 0x5454:
	case 0x5455: return DEVICE_ATI_RAGE_128_PRO_ULTRA;

	case 0x534E: return DEVICE_ATI_RAGE_128_4X;

	default: return DEVICE_ATI_UNKNOWN;
	}
}

DX8Caps::DeviceType3DLabs DX8Caps::Get_3DLabs_Device(unsigned device_id)
{
	switch (device_id) {
	case 0x0001: return DEVICE_3DLABS_300SX;
	case 0x0002: return DEVICE_3DLABS_500TX;
	case 0x0003: return DEVICE_3DLABS_DELTA;
	case 0x0004: return DEVICE_3DLABS_PERMEDIA;
	case 0x0006: return DEVICE_3DLABS_MX;
	case 0x0007: return DEVICE_3DLABS_PERMEDIA2;
	case 0x0008: return DEVICE_3DLABS_GAMMA;
	case 0x0009: return DEVICE_3DLABS_PERMEDIA2S_ST;
	case 0x000a: return DEVICE_3DLABS_PERMEDIA3;
	case 0x000b: return DEVICE_3DLABS_R3;
	case 0x000c: return DEVICE_3DLABS_PERMEDIA4;
	case 0x000d: return DEVICE_3DLABS_R4;
	case 0x000e: return DEVICE_3DLABS_G2;
	case 0x4C59: return DEVICE_3DLABS_OXYGEN_VX1;
	case 0x3D04: return DEVICE_3DLABS_TI_P1;
	case 0x3D07: return DEVICE_3DLABS_PERMEDIA2;
	default: return DEVICE_3DLABS_UNKNOWN;
	}
}

DX8Caps::DeviceTypeNVidia DX8Caps::Get_NVidia_Device(unsigned device_id)
{
	switch (device_id) {
	case 0x0250: return DEVICE_NVIDIA_GEFORCE4_TI_4600;
	case 0x0251: return DEVICE_NVIDIA_GEFORCE4_TI_4400;
	case 0x0252: return DEVICE_NVIDIA_GEFORCE4_TI;
	case 0x0253: return DEVICE_NVIDIA_GEFORCE4_TI_4200;
	case 0x0170: return DEVICE_NVIDIA_GEFORCE4_MX_460;
	case 0x0171: return DEVICE_NVIDIA_GEFORCE4_MX_440;
	case 0x0172: return DEVICE_NVIDIA_GEFORCE4_MX_420;
	case 0x0173: return DEVICE_NVIDIA_GEFORCE4;
	case 0x0174: return DEVICE_NVIDIA_GEFORCE4_GO_440;
	case 0x0175: return DEVICE_NVIDIA_GEFORCE4_GO_420;
	case 0x0176: return DEVICE_NVIDIA_GEFORCE4_GO_420_32M;
	case 0x0178: return DEVICE_NVIDIA_GEFORCE4_GO;
	case 0x0179: return DEVICE_NVIDIA_GEFORCE4_GO_440_64M;
	case 0x0203: return DEVICE_NVIDIA_QUADRO_DCC;
	case 0x0202: return DEVICE_NVIDIA_GEFORCE3_TI_500;
	case 0x0201: return DEVICE_NVIDIA_GEFORCE3_TI_200;
	case 0x0200: return DEVICE_NVIDIA_GEFORCE3;
	case 0x01A0: return DEVICE_NVIDIA_GEFORCE2_INTEGRATED;
	case 0x0153: return DEVICE_NVIDIA_QUADRO2_PRO;
	case 0x0152: return DEVICE_NVIDIA_GEFORCE2_ULTRA;
	case 0x0151: return DEVICE_NVIDIA_GEFORCE2_TI;
	case 0x0150: return DEVICE_NVIDIA_GEFORCE2_GTS;
	case 0x0113: return DEVICE_NVIDIA_QUADRO2_MXR_EX_GO;
	case 0x0112: return DEVICE_NVIDIA_GEFORCE2_GO;
	case 0x0111: return DEVICE_NVIDIA_GEFORCE2_MX_100_200;
	case 0x0110: return DEVICE_NVIDIA_GEFORCE2_MX_400;
	case 0x0103: return DEVICE_NVIDIA_QUADRO;
	case 0x0101: return DEVICE_NVIDIA_GEFORCE_DDR;
	case 0x0100: return DEVICE_NVIDIA_GEFORCE_256;
	case 0x00A0: return DEVICE_NVIDIA_TNT2_ALADDIN;
	case 0x0028: return DEVICE_NVIDIA_TNT2;
	case 0x0029: return DEVICE_NVIDIA_TNT2_ULTRA;
	case 0x002C: return DEVICE_NVIDIA_TNT2_VANTA;
	case 0x002D: return DEVICE_NVIDIA_TNT2_M64;
	case 0x0020: return DEVICE_NVIDIA_TNT;
	case 0x0008: return DEVICE_NVIDIA_NV1;

	// STB cards
	case 0x0019:
	case 0x0018: return DEVICE_NVIDIA_RIVA_128;

	// Guillemot Cards
	case 0x5008: return DEVICE_NVIDIA_TNT_VANTA;		// Maxi Gamer Phoenix 2
	case 0x5810: return DEVICE_NVIDIA_TNT2;			// Maxi Gamer Xentor
	case 0x5820: return DEVICE_NVIDIA_TNT2_ULTRA;	// Maxi Gamer Xentor 32
	case 0x4d20: return DEVICE_NVIDIA_TNT2_M64;		// Maxi Gamer Cougar
	case 0x5620: return DEVICE_NVIDIA_TNT2_M64;		// Maxi Gamer Cougar Video Edition
	case 0x5020: return DEVICE_NVIDIA_GEFORCE_256;	// Maxi Gamer 3D Prophet

	default: return DEVICE_NVIDIA_UNKNOWN;
	}
}


DX8Caps::DeviceType3Dfx DX8Caps::Get_3Dfx_Device(unsigned device_id)
{
	switch (device_id) {
	case 0x0009: return DEVICE_3DFX_VOODOO_5500_AGP;
	case 0x0005: return DEVICE_3DFX_VOODOO_3;
	case 0x0003: return DEVICE_3DFX_BANSHEE;
	case 0x0002: return DEVICE_3DFX_VOODOO_2;
	case 0x0001: return DEVICE_3DFX_VOODOO_GRAPHICS;
	case 0x643d: // Alliance AT25/AT3D based reference board
	case 0x8626: // Macronix based reference board
		return DEVICE_3DFX_VOODOO_RUSH;
	default: return DEVICE_3DFX_UNKNOWN;
	}
}

DX8Caps::DeviceTypeMatrox DX8Caps::Get_Matrox_Device(unsigned device_id)
{
	switch (device_id) {
	case 0x2527: return DEVICE_MATROX_G550;
	case 0x0525: return DEVICE_MATROX_G400;
	case 0x0521: return DEVICE_MATROX_G200_AGP;
	case 0x0520: return DEVICE_MATROX_G200_PCI;
	case 0x1000: return DEVICE_MATROX_G100_PCI;
	case 0x1001: return DEVICE_MATROX_G100_AGP;
	case 0x051F: return DEVICE_MATROX_MILLENNIUM_II_AGP;
	case 0x051B: return DEVICE_MATROX_MILLENNIUM_II_PCI;
	case 0x051A: return DEVICE_MATROX_MYSTIQUE;
	case 0x0519: return DEVICE_MATROX_MILLENNIUM;
	case 0x0527: return DEVICE_MATROX_PARHELIA;
	case 0x0528: return DEVICE_MATROX_PARHELIA_AGP8X;

	default: return DEVICE_MATROX_UNKNOWN;
	}
}

DX8Caps::DeviceTypePowerVR DX8Caps::Get_PowerVR_Device(unsigned device_id)
{
	switch (device_id) {
	case 0x0010: return DEVICE_POWERVR_KYRO;
	default: return DEVICE_POWERVR_UNKNOWN;
	}
}

DX8Caps::DeviceTypeS3 DX8Caps::Get_S3_Device(unsigned device_id)
{
	switch (device_id) {
	case 0x8C10: return DEVICE_S3_SAVAGE_MX;
	case 0x8A22: return DEVICE_S3_SAVAGE_4;
	case 0x9102: return DEVICE_S3_SAVAGE_200;
	default: return DEVICE_S3_UNKNOWN;
	}
}

DX8Caps::DeviceTypeIntel DX8Caps::Get_Intel_Device(unsigned device_id)
{
	switch (device_id) {
	case 0x7123: return DEVICE_INTEL_810;
	case 0x7121: return DEVICE_INTEL_810E;
	case 0x1132: return DEVICE_INTEL_815;
	default: return DEVICE_INTEL_UNKNOWN;
	}
}

DX8Caps::DX8Caps(
	IDirect3D8* direct3d,
	IDirect3DDevice8* D3DDevice,
	WW3DFormat display_format,
	const D3DADAPTER_IDENTIFIER8& adapter_id)
	:
	Direct3D(direct3d),
	MaxDisplayWidth(0),
	MaxDisplayHeight(0)
{
	Init_Caps(D3DDevice);
	Compute_Caps(display_format, adapter_id);
}

//Don't really need this but I added this function to free static variables so
//they don't show up in our memory manager as a leak. -MW 7-22-03
void DX8Caps::Shutdown(void)
{
	CapsWorkString.Release_Resources();
}

// ----------------------------------------------------------------------------
//
// Init the caps structure
//
// ----------------------------------------------------------------------------

void DX8Caps::Init_Caps(IDirect3DDevice8* D3DDevice)
{
	D3DDevice->SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING,TRUE);
	DX8CALL(GetDeviceCaps(&swVPCaps));

	if ((swVPCaps.DevCaps&D3DDEVCAPS_HWTRANSFORMANDLIGHT)==D3DDEVCAPS_HWTRANSFORMANDLIGHT) {
		SupportTnL=true;

		D3DDevice->SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING,FALSE);
		DX8CALL(GetDeviceCaps(&hwVPCaps));
	} else {
		SupportTnL=false;
	}
}

// ----------------------------------------------------------------------------
//
// Compute the caps bits
//
// ----------------------------------------------------------------------------
void DX8Caps::Compute_Caps(WW3DFormat display_format, const D3DADAPTER_IDENTIFIER8& adapter_id)
{
//	Init_Caps(D3DDevice);

	const D3DCAPS8& caps=Get_DX8_Caps();

	if ((caps.DevCaps&D3DDEVCAPS_NPATCHES)==D3DDEVCAPS_NPATCHES) {
		SupportNPatches=true;
	} else {
		SupportNPatches=false;
	}

	if ((caps.TextureOpCaps&D3DTEXOPCAPS_DOTPRODUCT3)==D3DTEXOPCAPS_DOTPRODUCT3)
	{
		SupportDot3=true;
	} else {
		SupportDot3=false;
	}

	supportGamma=((swVPCaps.Caps2&D3DCAPS2_FULLSCREENGAMMA)==D3DCAPS2_FULLSCREENGAMMA);

	MaxTexturesPerPass=MAX_TEXTURE_STAGES;

	Check_Texture_Format_Support(display_format,caps);
	Check_Texture_Compression_Support(caps);
	Check_Bumpmap_Support(caps);
	Check_Shader_Support(caps);
	Check_Maximum_Texture_Support(caps);
	Vendor_Specific_Hacks(adapter_id);
	CapsWorkString="";
}

// ----------------------------------------------------------------------------
//
// Check bump map texture support
//
// ----------------------------------------------------------------------------

void DX8Caps::Check_Bumpmap_Support(const D3DCAPS8& caps)
{
	SupportBumpEnvmap=!!(caps.TextureOpCaps & D3DTEXOPCAPS_BUMPENVMAP);
	SupportBumpEnvmapLuminance=!!(caps.TextureOpCaps & D3DTEXOPCAPS_BUMPENVMAPLUMINANCE);
}

// ----------------------------------------------------------------------------
//
// Check compressed texture support
//
// ----------------------------------------------------------------------------

void DX8Caps::Check_Texture_Compression_Support(const D3DCAPS8& caps)
{
	SupportDXTC=SupportTextureFormat[WW3D_FORMAT_DXT1]|
		SupportTextureFormat[WW3D_FORMAT_DXT2]|
		SupportTextureFormat[WW3D_FORMAT_DXT3]|
		SupportTextureFormat[WW3D_FORMAT_DXT4]|
		SupportTextureFormat[WW3D_FORMAT_DXT5];
}

void DX8Caps::Check_Texture_Format_Support(WW3DFormat display_format,const D3DCAPS8& caps)
{
	if (display_format==WW3D_FORMAT_UNKNOWN) {
		for (unsigned i=0;i<WW3D_FORMAT_COUNT;++i) {
			SupportTextureFormat[i]=false;
		}
		return;
	}
	D3DFORMAT d3d_display_format=WW3DFormat_To_D3DFormat(display_format);
	for (unsigned i=0;i<WW3D_FORMAT_COUNT;++i) {
		if (i==WW3D_FORMAT_UNKNOWN) {
			SupportTextureFormat[i]=false;
		}
		else {
			WW3DFormat format=(WW3DFormat)i;
			SupportTextureFormat[i]=SUCCEEDED(
				Direct3D->CheckDeviceFormat(
					caps.AdapterOrdinal,
					caps.DeviceType,
					d3d_display_format,
					0,
					D3DRTYPE_TEXTURE,
					WW3DFormat_To_D3DFormat(format)));
		}
	}
}

void DX8Caps::Check_Render_To_Texture_Support(WW3DFormat display_format,const D3DCAPS8& caps)
{
	if (display_format==WW3D_FORMAT_UNKNOWN) {
		for (unsigned i=0;i<WW3D_FORMAT_COUNT;++i) {
			SupportRenderToTextureFormat[i]=false;
		}
		return;
	}
	D3DFORMAT d3d_display_format=WW3DFormat_To_D3DFormat(display_format);
	for (unsigned i=0;i<WW3D_FORMAT_COUNT;++i) {
		if (i==WW3D_FORMAT_UNKNOWN) {
			SupportRenderToTextureFormat[i]=false;
		}
		else {
			WW3DFormat format=(WW3DFormat)i;
			SupportRenderToTextureFormat[i]=SUCCEEDED(
				Direct3D->CheckDeviceFormat(
					caps.AdapterOrdinal,
					caps.DeviceType,
					d3d_display_format,
					D3DUSAGE_RENDERTARGET,
					D3DRTYPE_TEXTURE,
					WW3DFormat_To_D3DFormat(format)));
		}
	}
}

//**********************************************************************************************
//! Check Depth Stencil Format Support
/*! KJM
*/
void DX8Caps::Check_Depth_Stencil_Support(WW3DFormat display_format, const D3DCAPS8& caps)
{
	if (display_format==WW3D_FORMAT_UNKNOWN)
	{
		for (unsigned i=0;i<WW3D_ZFORMAT_COUNT;++i)
		{
			SupportDepthStencilFormat[i]=false;
		}
		return;
	}

	D3DFORMAT d3d_display_format=WW3DFormat_To_D3DFormat(display_format);

	for (unsigned i=0;i<WW3D_ZFORMAT_COUNT;++i)
	{
		if (i==WW3D_ZFORMAT_UNKNOWN)
		{
			SupportDepthStencilFormat[i]=false;
		}
		else
		{
			WW3DZFormat format=(WW3DZFormat)i;
			SupportDepthStencilFormat[i]=SUCCEEDED
			(
				Direct3D->CheckDeviceFormat
				(
					caps.AdapterOrdinal,
					caps.DeviceType,
					d3d_display_format,
					D3DUSAGE_DEPTHSTENCIL,
					D3DRTYPE_TEXTURE,
					WW3DZFormat_To_D3DFormat(format)
				)
			);
		}
	}
}

void DX8Caps::Check_Maximum_Texture_Support(const D3DCAPS8& caps)
{
	MaxSimultaneousTextures=caps.MaxSimultaneousTextures;
}

void DX8Caps::Check_Shader_Support(const D3DCAPS8& caps)
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

void DX8Caps::Vendor_Specific_Hacks(const D3DADAPTER_IDENTIFIER8& adapter_id)
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

