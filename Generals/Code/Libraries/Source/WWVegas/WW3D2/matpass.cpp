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
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   MaterialPassClass::MaterialPassClass -- Constructor                                       *
 *   MaterialPassClass::~MaterialPassClass -- Destructor                                       *
 *   MaterialPassClass::Install_Materials -- Plug our material settings into D3D               *
 *   MaterialPassClass::Set_Texture -- Set texture to use                                      *
 *   MaterialPassClass::Set_Shader -- Set the shader to use                                    *
 *   MaterialPassClass::Set_Material -- set vertex material to use                             *
 *   MaterialPassClass::Get_Texture -- Get a pointer to the texture                            *
 *   MaterialPassClass::Get_Material -- get the vertex material                                *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "matpass.h"
#include "vertmaterial.h"
#include "shader.h"
#include "texture.h"
#include "statistics.h"
#include "dx8wrapper.h"


bool MaterialPassClass::EnablePerPolygonCulling = true;


/***********************************************************************************************
 * MaterialPassClass::MaterialPassClass -- Constructor                                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/26/2001  gth : Created.                                                                 *
 *=============================================================================================*/
MaterialPassClass::MaterialPassClass(void) :
	Shader(0),
	Material(NULL),
	CullVolume(NULL),
	EnableOnTranslucentMeshes(true)
{
	for (int i=0; i<MAX_TEX_STAGES; i++) {
		Texture[i] = NULL;
	}
}

/***********************************************************************************************
 * MaterialPassClass::~MaterialPassClass -- Destructor                                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/9/99    gth : Created.                                                                 *
 *	  06/27/02   kjm : Changes to max texture stage caps															*
 *=============================================================================================*/
MaterialPassClass::~MaterialPassClass(void)
{
	for (int i=0; i<MAX_TEX_STAGES; i++) {
		REF_PTR_RELEASE(Texture[i]);
	}
	REF_PTR_RELEASE(Material);
}


/***********************************************************************************************
 * MaterialPassClass::Install_Materials -- Plug our material settings into D3D                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/9/99    gth : Created.                                                                 *
 *   2/26/2001  gth : Changed to Install_Materials                                             *
 *=============================================================================================*/
void MaterialPassClass::Install_Materials(void) const
{
	DX8Wrapper::Set_Material(Peek_Material());
	DX8Wrapper::Set_Shader(Peek_Shader());
	for (int i=0;i<DX8Wrapper::Get_Current_Caps()->Get_Max_Textures_Per_Pass();++i)
	{
		DX8Wrapper::Set_Texture(i,Peek_Texture(i));
	}
}


/***********************************************************************************************
 * MaterialPassClass::Set_Texture -- Set texture to use                                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 * tex - pointer to the texture for this material pass (or NULL)                               *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/9/99    gth : Created.                                                                 *
 *=============================================================================================*/
void MaterialPassClass::Set_Texture(TextureClass * tex,int stage)
{
	WWASSERT(stage >= 0);
	WWASSERT(stage < MAX_TEX_STAGES);

	REF_PTR_SET(Texture[stage],tex);
}


/***********************************************************************************************
 * MaterialPassClass::Set_Shader -- Set the shader to use                                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 * shader - shader for this material pass                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/9/99    gth : Created.                                                                 *
 *=============================================================================================*/
void MaterialPassClass::Set_Shader(ShaderClass shader)
{
	Shader = shader;
	Shader.Enable_Fog ("MaterialPassClass");
}


/***********************************************************************************************
 * MaterialPassClass::Set_Material -- set vertex material to use                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 * mat - pointer to the vertex material this material pass uses                                *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/9/99    gth : Created.                                                                 *
 *=============================================================================================*/
void MaterialPassClass::Set_Material(VertexMaterialClass * mat)
{
	REF_PTR_SET(Material,mat);
}


/***********************************************************************************************
 * MaterialPassClass::Get_Texture -- Get a pointer to the texture                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 * reference counted pointer to the texture this material pass is using                        *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/9/99    gth : Created.                                                                 *
 *=============================================================================================*/
TextureClass * MaterialPassClass::Get_Texture(int stage) const
{
	WWASSERT(stage >= 0);
	WWASSERT(stage < MAX_TEX_STAGES);

	if (Texture[stage]) {
		Texture[stage]->Add_Ref();
	}
	return Texture[stage];
}


/***********************************************************************************************
 * MaterialPassClass::Get_Material -- get the vertex material                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 * reference counted pointer to the vertex material being used by this material pass           *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/9/99    gth : Created.                                                                 *
 *=============================================================================================*/
VertexMaterialClass * MaterialPassClass::Get_Material(void) const
{
	if (Material) {
		Material->Add_Ref();
	}
	return Material;
}
