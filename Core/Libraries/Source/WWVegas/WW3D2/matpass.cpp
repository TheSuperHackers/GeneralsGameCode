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
 *                 Project Name : WW3d                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/matpass.cpp                            $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                       Author : Kenny Mitchell                                               *
 *                                                                                             *
 *                     $Modtime:: 06/27/02 1:27p                                              $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 * 06/27/02 KM Changes to max texture stage caps															*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "matpass.h"
#include "vertmaterial.h"
#include "shader.h"
#include "texture.h"
#include "statistics.h"
#include "dx8wrapper.h"


bool MaterialPassClass::EnablePerPolygonCulling = true;


MaterialPassClass::MaterialPassClass() :
	Shader(0),
	Material(nullptr),
	CullVolume(nullptr),
	EnableOnTranslucentMeshes(true)
{
	for (int i=0; i<MAX_TEX_STAGES; i++) {
		Texture[i] = nullptr;
	}
}

MaterialPassClass::~MaterialPassClass()
{
	for (int i=0; i<MAX_TEX_STAGES; i++) {
		REF_PTR_RELEASE(Texture[i]);
	}
	REF_PTR_RELEASE(Material);
}


// Plug our material settings into D3D.
void MaterialPassClass::Install_Materials() const
{
	DX8Wrapper::Set_Material(Peek_Material());
	DX8Wrapper::Set_Shader(Peek_Shader());
	for (int i=0;i<DX8Wrapper::Get_Current_Caps()->Get_Max_Textures_Per_Pass();++i)
	{
		DX8Wrapper::Set_Texture(i,Peek_Texture(i));
	}
}


// The texture can be nullptr.
void MaterialPassClass::Set_Texture(TextureClass * tex,int stage)
{
	WWASSERT(stage >= 0);
	WWASSERT(stage < MAX_TEX_STAGES);

	REF_PTR_SET(Texture[stage],tex);
}


void MaterialPassClass::Set_Shader(ShaderClass shader)
{
	Shader = shader;
	Shader.Enable_Fog ("MaterialPassClass");
}


void MaterialPassClass::Set_Material(VertexMaterialClass * mat)
{
	REF_PTR_SET(Material,mat);
}


TextureClass * MaterialPassClass::Get_Texture(int stage) const
{
	WWASSERT(stage >= 0);
	WWASSERT(stage < MAX_TEX_STAGES);

	if (Texture[stage]) {
		Texture[stage]->Add_Ref();
	}
	return Texture[stage];
}


VertexMaterialClass * MaterialPassClass::Get_Material() const
{
	if (Material) {
		Material->Add_Ref();
	}
	return Material;
}
