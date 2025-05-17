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
 *                     $Archive:: /Commando/Code/ww3d2/w3d_util.h                             $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 1/08/01 10:04a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef W3D_UTIL_H
#define W3D_UTIL_H

#include "always.h"
#include "w3d_file.h"

class Vector3;
class Vector4;
class Quaternion;
class ShaderClass;

/*
** These functions are just simple helpers which convert to and from the W3D file-format
** structures and the equivalent runtime classes.
*/
class W3dUtilityClass
{
public:

	static void	Convert_Vector(const W3dVectorStruct & v,Vector3 * set);
	static void	Convert_Vector(const Vector3 & v,W3dVectorStruct * set);

	static void	Convert_Quaternion(const W3dQuaternionStruct & q,Quaternion * set);
	static void	Convert_Quaternion(const Quaternion & q,W3dQuaternionStruct * set);

	static void	Convert_Color(const W3dRGBStruct & rgb,Vector3 * set);
	static void	Convert_Color(const Vector3 & v,W3dRGBStruct * set);

	static void	Convert_Color(const W3dRGBAStruct & rgb,Vector4 * set);
	static void	Convert_Color(const Vector4 & v,W3dRGBAStruct * set);
	
	static void Convert_Shader(const W3dShaderStruct & shader,ShaderClass * set);
	static void Convert_Shader(const ShaderClass & shader,W3dShaderStruct * set);

};

#endif


