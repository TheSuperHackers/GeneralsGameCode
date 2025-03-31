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
 *                 Project Name : WWSHADE                                                         *
 *                                                                                             *
 *                     $Archive:: wwshade/shd6bumpdiff.h													$*
 *                                                                                             *
 *                  $Org Author:: Kenny_m
 *																																	
 *                      $Author:: Kenny_m
 *																																	
 *							  $Modtime:: 7/11/02 10:36p                                               $*
 *                                                                                             *
 *                    $Revision:: 1                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef SHD6BUMPDIFF_H
#define SHD6BUMPDIFF_H

#ifndef SHDINTERFACE_H
#include "shdinterface.h"
#endif

#ifndef SHDHWSHADER_H
#include "shdhwshader.h"
#endif


class Shd6BumpDiffClass : public ShdInterfaceClass
{
public:
	Shd6BumpDiffClass(const ShdDefClass* def);
	virtual ~Shd6BumpDiffClass();
	
	static void Init();
	static void Shutdown();

	virtual int							Get_Pass_Count() { return 1; }

	virtual int							Get_Texture_Count() const { return 1; }
	virtual TextureClass*			Peek_Texture(int idx) const { return Texture; }

	virtual void						Apply_Shared(int cur_pass, RenderInfoClass& rinfo);
	virtual void						Apply_Instance(int cur_pass, RenderInfoClass& rinfo);

	virtual unsigned					Get_Vertex_Stream_Count() const;
	virtual unsigned					Get_Vertex_Size(unsigned stream) const;
	virtual bool						Use_HW_Vertex_Processing() const { return Vertex_Shader.Is_Using_Hardware(); }
	virtual void						Copy_Vertex_Stream
	(
		unsigned stream, 
		void* dest_buffer, 
		const VertexStreamStruct& vss, 
		unsigned vertex_count
	);

protected:

	static ShdHWVertexShader		Vertex_Shader;
	static Matrix4x4					View_Projection_Matrix;

	TextureClass*			Texture;

	Vector4					Ambient;
	Vector4					Diffuse;
};

#endif // SHD6BUMPDIFF_H
