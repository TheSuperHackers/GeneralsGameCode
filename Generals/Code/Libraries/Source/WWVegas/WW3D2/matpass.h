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
 *                 Project Name : ww3d                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/matpass.h                              $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 5/13/01 11:25a                                              $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "refcount.h"
#include "shader.h"
#include "wwdebug.h"


class TextureClass;
class VertexMaterialClass;
class MeshModelClass;
class OBBoxClass;

/**
** MaterialPassClass
**
** This class wraps all of the data needed to describe an additional
** material pass for any object.  The motivation for this class is to
** implement certain types of special effects.  All data needed to
** apply the pass should be generated procedurally.  Typically a
** vertex processor will be used to generate any needed u-v's or vertex
** colors.  Alternatively, we could add the option to request to
** re-use the model's existing u-v's or vertex colors.
**
**
*/
class MaterialPassClass : public RefCountClass
{
public:

	MaterialPassClass(void);
	~MaterialPassClass(void);

	/// MW: Had to make this virtual so app can perform direct/custom D3D setup.
	virtual void	Install_Materials(void) const;
	virtual void	UnInstall_Materials(void) const { };	///< reset/cleanup D3D states

	void							Set_Texture(TextureClass * Texture,int stage = 0);
	void							Set_Shader(ShaderClass shader);
	void							Set_Material(VertexMaterialClass * mat);

	TextureClass *				Get_Texture(int stage = 0) const;
	VertexMaterialClass *	Get_Material(void) const;

	TextureClass *				Peek_Texture(int stage = 0) const;
	ShaderClass					Peek_Shader(void)	const							{ return Shader; }
	VertexMaterialClass *	Peek_Material(void) const						{ return Material; }

	void							Set_Cull_Volume(OBBoxClass * volume)		{ CullVolume = volume; }
	OBBoxClass *				Get_Cull_Volume(void) const					{ return CullVolume; }

	void							Enable_On_Translucent_Meshes(bool onoff)	{ EnableOnTranslucentMeshes = onoff; }
	bool							Is_Enabled_On_Translucent_Meshes(void)		{ return EnableOnTranslucentMeshes; }

	static void					Enable_Per_Polygon_Culling(bool onoff)		{ EnablePerPolygonCulling = onoff; }
	static bool					Is_Per_Polygon_Culling_Enabled(void)		{ return EnablePerPolygonCulling; }

protected:

	enum { MAX_TEX_STAGES = 2 };

	TextureClass *				Texture[MAX_TEX_STAGES];
	ShaderClass					Shader;
	VertexMaterialClass *	Material;
	bool							EnableOnTranslucentMeshes;

	OBBoxClass *				CullVolume;
	static bool					EnablePerPolygonCulling;

};


inline TextureClass * MaterialPassClass::Peek_Texture(int stage) const
{
	WWASSERT(stage >= 0);
	WWASSERT(stage < MAX_TEX_STAGES);
	return Texture[stage];
}
