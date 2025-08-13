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
 *                     $Archive:: /Commando/Code/ww3d2/w3d_util.cpp                           $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 7/23/01 5:31p                                               $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "w3d_util.h"
#include "vector3.h"
#include "vector4.h"
#include "quat.h"
#include "shader.h"


void	W3dUtilityClass::Convert_Vector(const W3dVectorStruct & v,Vector3 * set)
{
	set->X = v.X;
	set->Y = v.Y;
	set->Z = v.Z;
}

void	W3dUtilityClass::Convert_Vector(const Vector3 & v,W3dVectorStruct * set)
{
	set->X = v.X;
	set->Y = v.Y;
	set->Z = v.Z;
}

void	W3dUtilityClass::Convert_Quaternion(const W3dQuaternionStruct & q,Quaternion * set)
{
	set->X = q.Q[0];
	set->Y = q.Q[1];
	set->Z = q.Q[2];
	set->W = q.Q[3];
}

void W3dUtilityClass::Convert_Quaternion(const Quaternion & q,W3dQuaternionStruct * set)
{
	set->Q[0] = q.X;
	set->Q[1] = q.Y;
	set->Q[2] = q.Z;
	set->Q[3] = q.W;
}

void	W3dUtilityClass::Convert_Color(const W3dRGBStruct & rgb,Vector3 * set)
{
	set->X = (float)rgb.R / 255.0f;
	set->Y = (float)rgb.G / 255.0f;
	set->Z = (float)rgb.B / 255.0f;
}

void	W3dUtilityClass::Convert_Color(const Vector3 & v,W3dRGBStruct * color)
{
	color->R = (uint8)(255.0f * v.X);
	color->G = (uint8)(255.0f * v.Y);
	color->B = (uint8)(255.0f * v.Z);
	color->pad = 0;
}

void W3dUtilityClass::Convert_Color(const W3dRGBAStruct & rgb,Vector4 * set)
{
	set->X = (float)rgb.R / 255.0f;
	set->Y = (float)rgb.G / 255.0f;
	set->Z = (float)rgb.B / 255.0f;
	set->W = (float)rgb.A / 255.0f;
}

void W3dUtilityClass::Convert_Color(const Vector4 & v,W3dRGBAStruct * color)
{
	color->R = (uint8)(255.0f * v.X);
	color->G = (uint8)(255.0f * v.Y);
	color->B = (uint8)(255.0f * v.Z);
	color->A = (uint8)(255.0f * v.W);
}

void W3dUtilityClass::Convert_Shader(const W3dShaderStruct & shader,ShaderClass * set)
{
	set->Set_Depth_Compare		((ShaderClass::DepthCompareType)W3d_Shader_Get_Depth_Compare(&shader));
	set->Set_Depth_Mask			((ShaderClass::DepthMaskType)W3d_Shader_Get_Depth_Mask(&shader));
	set->Set_Color_Mask			(ShaderClass::COLOR_WRITE_ENABLE);
	set->Set_Dst_Blend_Func		((ShaderClass::DstBlendFuncType)W3d_Shader_Get_Dest_Blend_Func(&shader));
	set->Set_Fog_Func				(ShaderClass::FOG_DISABLE);
	set->Set_Primary_Gradient	((ShaderClass::PriGradientType)W3d_Shader_Get_Pri_Gradient(&shader));
	set->Set_Secondary_Gradient((ShaderClass::SecGradientType)W3d_Shader_Get_Sec_Gradient(&shader));
	set->Set_Src_Blend_Func		((ShaderClass::SrcBlendFuncType)W3d_Shader_Get_Src_Blend_Func(&shader));
	set->Set_Texturing			((ShaderClass::TexturingType)W3d_Shader_Get_Texturing(&shader));
	set->Set_Alpha_Test			((ShaderClass::AlphaTestType)W3d_Shader_Get_Alpha_Test(&shader));
// Jani: No cull mode settings for w3d shader - problem?
//	set->Set_Dither_Mask			(ShaderClass::DITHER_ENABLE);
	// The tools do not enable setting post-detail stuff, and in any case we probably wouldn't
	// want to confuse the artists with the distinction between detail and post-detail settings at
	// this point, so we just copy the detail settings into the post-detail settings.
//	set->Set_Post_Detail_Color_Func ((ShaderClass::DetailColorFuncType) W3d_Shader_Get_Post_Detail_Color_Func (&shader));
//	set->Set_Post_Detail_Alpha_Func ((ShaderClass::DetailAlphaFuncType) W3d_Shader_Get_Post_Detail_Alpha_Func (&shader));
	set->Set_Post_Detail_Color_Func ((ShaderClass::DetailColorFuncType) W3d_Shader_Get_Detail_Color_Func (&shader));
	set->Set_Post_Detail_Alpha_Func ((ShaderClass::DetailAlphaFuncType) W3d_Shader_Get_Detail_Alpha_Func (&shader));
}

void W3dUtilityClass::Convert_Shader(const ShaderClass & shader,W3dShaderStruct * set)
{
	W3d_Shader_Reset(set);
	W3d_Shader_Set_Depth_Compare(set,shader.Get_Depth_Compare());
	W3d_Shader_Set_Depth_Mask(set,shader.Get_Depth_Mask());
	W3d_Shader_Set_Dest_Blend_Func(set,shader.Get_Dst_Blend_Func());
	W3d_Shader_Set_Pri_Gradient(set,shader.Get_Primary_Gradient());
	W3d_Shader_Set_Sec_Gradient(set,shader.Get_Secondary_Gradient());
	W3d_Shader_Set_Src_Blend_Func(set,shader.Get_Src_Blend_Func());
	W3d_Shader_Set_Texturing(set,shader.Get_Texturing());
	W3d_Shader_Set_Alpha_Test(set,shader.Get_Alpha_Test());
	W3d_Shader_Set_Post_Detail_Color_Func(set,shader.Get_Post_Detail_Color_Func());
	W3d_Shader_Set_Post_Detail_Alpha_Func(set,shader.Get_Post_Detail_Alpha_Func());
}
