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
 *                 Project Name : ww3d                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/polyinfo.h                             $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 2/07/01 12:48p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#ifndef POLYINFO_H
#define POLYINFO_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class TextureClass;
class VertexMaterialClass;
class ShaderClass;


/**
**	PolyInfo.h
**
**	This class is a simple storage class that keeps track of a texture, vertex material
**	and a shader.
**
*/
class PolygonInfoClass
{

public:

	TextureClass			* Peek_Texture()				const	{ return Texture; }
	VertexMaterialClass	* Peek_Vertex_Material()	const { return VertexMaterial; }
	ShaderClass				* Peek_Shader()				const { return Shader; }

	void Set_Texture(TextureClass *texture);
	void Set_Vertex_Material(VertexMaterialClass *vertexMaterial);
	void Set_Shader(ShaderClass *shader);

	void Set(TextureClass *texture, VertexMaterialClass *vertexMaterial, ShaderClass *shader)
	{
		Set_Texture(texture);
		Set_Vertex_Material(vertexMaterial);
		Set_Shader(shader);
	}

	PolygonInfoClass(TextureClass *texture = 0, VertexMaterialClass *vertexMaterial = 0, ShaderClass *shader = 0)
	: Texture(0), VertexMaterial(0), Shader(0)
	{
		Set(texture, vertexMaterial, shader);
	}

	~PolygonInfoClass();


protected:

	TextureClass *Texture;
	VertexMaterialClass *VertexMaterial;
	ShaderClass *Shader;
};

#endif

