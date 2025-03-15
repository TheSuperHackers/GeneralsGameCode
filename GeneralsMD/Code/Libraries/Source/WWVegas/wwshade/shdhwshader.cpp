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
 *                     $Archive:: /Commando/Code/ww3d2/shdhwshader.cpp                           $*
 *                                                                                             *
 *                  $Org Author:: Kenny_m
 *                                                                                             *
 *                      $Author:: Kenny_m
 *																																	
 *							  $Modtime:: 07/07/02 12:18p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                          $*
 *                                                                                             *
 * 06/06/02 KM added software vertex shader fallback check
 * 07/07/02 KM Generic shader functions for factoring HW shader code
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include <stdio.h>
#include "shdhwshader.h"
#include "dx8wrapper.h"
#include "rinfo.h"
#include "shdhw_constants.h"

//**********************************************************************************************
//! Execute and wait for a hidden shell command
/*! 5/27/02 5:39p KJM Created
*/
void ShdHWShader::Shell_Run(char* cmd)
{
	int result;

#ifdef WIN32
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	memset(&pi, 0, sizeof(pi));
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags=STARTF_USESHOWWINDOW;
	si.wShowWindow=SW_HIDE;

	static bool found=false;
	static char work_dir[_MAX_PATH];

	if (!found)
	{
		const char* shader_path="\\shaders\\";

		::GetCurrentDirectory(_MAX_PATH,work_dir);
		strcat(work_dir,shader_path);
	}

	result=CreateProcess(0,cmd,0,0,0,0,0,work_dir,&si,&pi);

	if (result) 
	{
		WaitForSingleObject(pi.hThread,  INFINITE);

		// Close process and thread handles. 
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		found=true;
	}
	else
	{
		int err=GetLastError();
		MessageBox
		(
			NULL,
			"Failed to execute preprocessor on shader\n"
			"Ensure shaders folder is in application subdirectory\n",
			"Shader Asset Error",
			MB_OK
		);
		WWASSERT_PRINT(result,"Failed to execute preprocessor on shader (ensure shaders folder is in application subdirectory)");
	}
#else
	result=system(cmd);
#endif
}


//**********************************************************************************************
//! Preprocess and assemble a HW shader from file
/*! 5/27/02 5:39p KJM Created
*/
void ShdHWShader::Preprocess_And_Assemble_Shader_From_File
(	
	char*				file_name,	
	LPD3DXBUFFER*	constants,	
	LPD3DXBUFFER*	shader_code
)
{
	char shell_command[_MAX_PATH];
	char temp_path[_MAX_PATH];
	char temp_file[_MAX_PATH];


   GetTempPath(_MAX_PATH, temp_path);
	GetTempFileName(temp_path,"shd",1,temp_file);

	_snprintf
	(
		shell_command, 
		sizeof(shell_command), 
		"shaders\\rspp %s %s", 
		file_name, 
		temp_file
	);

	LPD3DXBUFFER* errors=NULL;

	Shell_Run(shell_command);

	HRESULT result=D3DXAssembleShaderFromFile
	(
		temp_file, 
		NULL, 
		constants, 
		shader_code, 
		errors
	);
	WWASSERT_PRINT(result==D3D_OK,"Failed to assemble shader from file");
}

// 06/06/02 KM added software vertex shader fallback check
bool	ShdHWVertexShader::Using_Hardware=true;

//**********************************************************************************************
//! Destruct this vertex shader
/*! 5/27/02 5:39p KJM Created
*/
ShdHWVertexShader::~ShdHWVertexShader()
{
	Destroy();
}

//**********************************************************************************************
//! Destroy this vertex shader
/*! 5/27/02 5:39p KJM Created
*/
void ShdHWVertexShader::Destroy()
{
	if (Shader) 
	{
		DX8Wrapper::_Get_D3D_Device8()->SetVertexShader(D3DFVF_XYZ|D3DFVF_DIFFUSE);
		DX8Wrapper::_Get_D3D_Device8()->DeleteVertexShader(Shader);
		Shader=0;
	}
}

//**********************************************************************************************
//! Create Vertex Shader from a file and vertex stream declaration
/*! 05/27/02 5:39p KJM Created
/*! 06/06/02 KM added software vertex shader fallback check
*/
DWORD ShdHWVertexShader::Create
(
	char* file_name, 
	DWORD* vertex_shader_declaration
)
{
	// Create vertex shader
	LPD3DXBUFFER shader_code=NULL;

	Preprocess_And_Assemble_Shader_From_File
	(
		file_name,
		NULL,
		&shader_code
	);

	// try hardware first
	Using_Hardware=true;
	HRESULT result=DX8Wrapper::_Get_D3D_Device8()->CreateVertexShader
	(
		(DWORD*)vertex_shader_declaration,
		(DWORD*)shader_code->GetBufferPointer(),
		(DWORD*)&Shader,
		0
	);
	if (result!=D3D_OK)
	{
		// try software
		Using_Hardware=false;
		result=DX8Wrapper::_Get_D3D_Device8()->CreateVertexShader
		(
			(DWORD*)vertex_shader_declaration,
			(DWORD*)shader_code->GetBufferPointer(),
			(DWORD*)&Shader,
			D3DUSAGE_SOFTWAREPROCESSING
		);
		WWASSERT_PRINT(result==D3D_OK,"Failed to create vertex shader");
	}

	if (shader_code) shader_code->Release();

	return Shader;
}


//**********************************************************************************************
//! Create Vertex Shader from a dword constant and vertex stream declaration
/*! 07/19/02 3:39p KJM Created
*/
DWORD ShdHWVertexShader::Create
(
	DWORD* shader_code_str, 
	DWORD* vertex_shader_declaration
)
{
	HRESULT result;
	LPD3DXBUFFER shader_code=NULL;
	LPD3DXBUFFER errors=NULL;

	result=D3DXAssembleShader
	(
		shader_code_str,
		strlen((char*)shader_code_str),
		NULL,//D3DXASM_DEBUG,
		NULL,
		&shader_code,
		&errors
	);
	if (result!=D3D_OK)
	{
		OutputDebugString((char*)errors->GetBufferPointer());
		WWASSERT_PRINT(result==D3D_OK,"Failed to assemble shader");
	}

	// try hardware first
	Using_Hardware=true;
	result=DX8Wrapper::_Get_D3D_Device8()->CreateVertexShader
	(
		(DWORD*)vertex_shader_declaration,
		(DWORD*)shader_code->GetBufferPointer(),
		(DWORD*)&Shader,
		0
	);
	if (result!=D3D_OK)
	{
		// try software
		Using_Hardware=false;
		result=DX8Wrapper::_Get_D3D_Device8()->CreateVertexShader
		(
			(DWORD*)vertex_shader_declaration,
			(DWORD*)shader_code->GetBufferPointer(),
			(DWORD*)&Shader,
			D3DUSAGE_SOFTWAREPROCESSING
		);
		WWASSERT_PRINT(result==D3D_OK,"Failed to create vertex shader");
		if (result!=D3D_OK)
		{
			OutputDebugString((char*)shader_code_str);
		}
	}

	return Shader;
}

//**********************************************************************************************
//! Destruct this pixel shader
/*! 5/27/02 5:39p KJM Created
*/
ShdHWPixelShader::~ShdHWPixelShader()
{
	Destroy();
}

//**********************************************************************************************
//! Destroy this pixel shader
/*! 5/27/02 5:39p KJM Created
*/
void ShdHWPixelShader::Destroy()
{
	if (Shader) 
	{
		DX8Wrapper::_Get_D3D_Device8()->SetPixelShader(0);
		DX8Wrapper::_Get_D3D_Device8()->DeletePixelShader(Shader);
		Shader=0;
	}
}

//**********************************************************************************************
//! Create Pixel Shader from a file
/*! 5/27/02 5:39p KJM Created
*/
DWORD ShdHWPixelShader::Create(char* file_name)
{
	// Create pixel shader
	LPD3DXBUFFER shader_code=NULL;

	Preprocess_And_Assemble_Shader_From_File
	(
		file_name,
		NULL,
		&shader_code
	);

	HRESULT result=DX8Wrapper::_Get_D3D_Device8()->CreatePixelShader
	(
		(DWORD*)shader_code->GetBufferPointer(), 
		(DWORD*)&Shader
	);
	WWASSERT_PRINT(result==D3D_OK,"Failed to create pixel shader");

	return Shader;
}


//**********************************************************************************************
//! Create Pixel Shader from a dword constant 
/*! 07/19/02 3:39p KJM Created
*/
DWORD ShdHWPixelShader::Create(DWORD* shader_code_str)
{
	HRESULT result;
	LPD3DXBUFFER shader_code=NULL;
	LPD3DXBUFFER errors;

	result=D3DXAssembleShader
	(
		shader_code_str,
		strlen((char*)shader_code_str),
		NULL,
		NULL,
		&shader_code,
		&errors
	);
	if (result!=D3D_OK)
	{
		OutputDebugString((char*)errors->GetBufferPointer());
		WWASSERT_PRINT(result==D3D_OK,"Failed to assemble shader");
	}

	result=DX8Wrapper::_Get_D3D_Device8()->CreatePixelShader
	(
		(DWORD*)shader_code->GetBufferPointer(), 
		(DWORD*)&Shader
	);
	WWASSERT_PRINT(result==D3D_OK,"Failed to create pixel shader");

	return Shader;
}


//**********************************************************************************************
//! Apply generic lighting
/*! 07/07/02 12:23p KJM Created
*/
void ShdHWVertexShader::Light
(
	RenderInfoClass&		rinfo,
	Vector4&					ambient,
	Vector4&					diffuse,
	Vector4&					specular
)
{
	LightEnvironmentClass* lenv=rinfo.light_environment;

	Vector4 light_ambient(1,1,1,1);

	if (lenv)
	{
		int num_lights=lenv->Get_Light_Count();
		
		// most cases set up and use the light environment
		if (num_lights)
		{
			const Vector3& amb=lenv->Get_Equivalent_Ambient();
			light_ambient.Set
			(
				amb.X,
				amb.Y,
				amb.Z,
				0
			);
			int i;
			
			int max=LightEnvironmentClass::Get_Max_Lights();

			if (num_lights>max) num_lights=max;

			Vector4 cdir, ccol;
			for (i=0;i<max;i++)
			{
				if (i<num_lights)
				{
					Vector3 rot;
					const Vector3&	dir=lenv->Get_Light_Direction(i);
					const Vector3&	col=lenv->Get_Light_Diffuse(i);

					cdir.Set(dir.X,dir.Y,dir.Z,1.0f);
					ccol.Set(col.X,col.Y,col.Z,1.0f);
				}
				else
				{
					// if no light just set color to zero
					ccol.Set(0,0,0,0);
				}

				DX8Wrapper::Set_Vertex_Shader_Constant(CV_LIGHT_DIRECTION_0+i, &cdir, 1);
				DX8Wrapper::Set_Vertex_Shader_Constant(CV_LIGHT_COLOR_0+i, &ccol, 1);
			}
		}
		else
		{
			const Vector3& amb_col=DX8Wrapper::Get_Ambient();

			light_ambient.X=amb_col.X;
			light_ambient.Y=amb_col.Y;
			light_ambient.Z=amb_col.Z;

			// otherwise have to extract light from render state
			int max=LightEnvironmentClass::Get_Max_Lights();
			int i;

			RenderStateStruct state;
			DX8Wrapper::Get_Render_State(state);
			Vector4 cdir, ccol;

			for (i=0;i<max;i++)
			{
				if (state.LightEnable[i])
				{
					D3DLIGHT8& light=state.Lights[i];

					// handle directional and positional lights
					if (light.Type==D3DLIGHT_DIRECTIONAL)
					{
						const D3DXVECTOR3& dir=state.Lights[i].Direction;

						cdir.Set(-dir.x,-dir.y,-dir.z,0.0f);
					}
					else
					{
						const D3DXVECTOR3& pos=state.Lights[i].Position;

						// just normalise the light position for now (as only appears to be used in w3dview)
						cdir.Set(pos.x,pos.y,pos.z,0.0f);
					}

					cdir.Normalize();
					ccol.Set
					(
						state.Lights[i].Diffuse.r,
						state.Lights[i].Diffuse.g,
						state.Lights[i].Diffuse.b,
						1.0f
					);
				}
				else
				{
					// if no light just set color to zero
					ccol.Set(0,0,0,0);
				}

				DX8Wrapper::Set_Vertex_Shader_Constant(CV_LIGHT_DIRECTION_0+i, &cdir, 1);
				DX8Wrapper::Set_Vertex_Shader_Constant(CV_LIGHT_COLOR_0+i, &ccol, 1);
			}
		}
	}

	light_ambient.X*=ambient.X;
	light_ambient.Y*=ambient.Y;
	light_ambient.Z*=ambient.Z;

	DX8Wrapper::Set_Vertex_Shader_Constant(CV_AMBIENT, &light_ambient, 1);
	DX8Wrapper::Set_Vertex_Shader_Constant(CV_DIFFUSE, &diffuse, 1);
	DX8Wrapper::Set_Vertex_Shader_Constant(CV_SPECULAR, &specular, 1);
}

//**********************************************************************************************
//! Apply generic lighting (no specular supplied)
/*! 07/07/02 12:23p KJM Created
*/
void ShdHWVertexShader::Light
(
	RenderInfoClass&		rinfo,
	Vector4&					ambient,
	Vector4&					diffuse
)
{
	Vector4 specular(0,0,0,0);
	Light(rinfo,ambient,diffuse,specular);
}

