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
 *                     $Archive:: /Commando/Code/ww3d2/polyinfo.cpp                           $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 2/07/01 12:50p                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "polyinfo.h"
#include "texture.h"
#include "vertmaterial.h"
#include "shader.h"

void PolygonInfoClass::Set_Texture(TextureClass *texture)
{
	if(texture)
		texture->Add_Ref();
	if(Texture)
		Texture->Release_Ref();
	Texture = texture;
}
void PolygonInfoClass::Set_Vertex_Material(VertexMaterialClass *vertexMaterial)
{
	if(vertexMaterial)
		vertexMaterial->Add_Ref();
	if(VertexMaterial)
		VertexMaterial->Release_Ref();

	VertexMaterial = vertexMaterial;
}

void PolygonInfoClass::Set_Shader(ShaderClass *shader)
{
	delete Shader;

	// todo : update for refcounted shaders
	Shader = W3DNEW ShaderClass(* shader);
}


PolygonInfoClass::~PolygonInfoClass()
{
	if(Texture)
		Texture->Release_Ref();
	if(VertexMaterial)
		VertexMaterial->Release_Ref();

	// todo : update for refcounted shaders
	delete Shader;
}


