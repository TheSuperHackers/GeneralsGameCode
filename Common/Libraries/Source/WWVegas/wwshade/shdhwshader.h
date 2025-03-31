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
 *                     $Archive:: /Commando/Code/ww3d2/shdhwshader.h                           $*
 *                                                                                             *
 *                  $Org Author:: Kenny_m
 *                                                                                             *
 *                      $Author:: Kenny_m
 *																																	
 *							  $Modtime:: 07/07/02 11:18p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                          $*
 *                                                                                             *
 * 06/06/02 KM added software vertex shader fallback check
 * 07/07/02 KM Generic shader functions for factoring HW shader code
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef SHDHWSHADER_H
#define SHDHWSHADER_H

#ifndef _D3D8_H_
#include <d3d8.h>
#endif

#ifndef __D3DX8_H__
#include <d3dx8.h>
#endif

#ifndef SHDHW_CONSTANTS_H
#include "shdhw_constants.h"
#endif


class RenderInfoClass;
class Vector4;

class ShdHWShader
{
public:
	ShdHWShader() : Shader(0) {}
	virtual ~ShdHWShader() {}

	DWORD Peek_Shader() const { return Shader; }

protected:

	void Shell_Run(char* cmd);

	void Preprocess_And_Assemble_Shader_From_File
	(
		char*				file_name,
		LPD3DXBUFFER*	constants,
		LPD3DXBUFFER*	shader_code
	);

	DWORD Shader;
};

class ShdHWVertexShader : public ShdHWShader
{
public:
	virtual ~ShdHWVertexShader();

	DWORD Create
	(
		char* file_name, 
		DWORD* vertex_shader_declaration
	);

	DWORD Create
	(
		DWORD* shader_code, 
		DWORD* vertex_shader_declaration
	);

	void Destroy();

	static bool	Is_Using_Hardware() { return Using_Hardware; }

	static void Light
	(
		RenderInfoClass&		rinfo,
		Vector4&					ambient,
		Vector4&					diffuse,
		Vector4&					specular
	);

	static void Light
	(
		RenderInfoClass&		rinfo,
		Vector4&					ambient,
		Vector4&					diffuse
	);

private:
	static bool Using_Hardware;
};

class ShdHWPixelShader : public ShdHWShader
{
public:
	virtual ~ShdHWPixelShader();

	DWORD Create(char* file_name);
	DWORD Create(DWORD* shader_code);

	void Destroy();
};

#endif
