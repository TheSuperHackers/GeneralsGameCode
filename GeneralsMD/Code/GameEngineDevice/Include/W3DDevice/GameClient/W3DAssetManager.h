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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : W3DAssetManager                                                  *
 *                                                                                             *
 *                     $Archive::                                                             $*
 *                                                                                             *
 *              Original Author:: Hector Yee                                                   *
 *                                                                                             *
 *                      $Author::                                                             $*
 *                                                                                             *
 *                     $Modtime::                                                             $*
 *                                                                                             *
 *                    $Revision::                                                             $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "assetmgr.h"
#include "Lib/BaseType.h"

class Vector3;
class VertexMaterialClass;

class W3DAssetManager: public WW3DAssetManager
{
public:
	W3DAssetManager(void);
	virtual ~W3DAssetManager(void);

	virtual RenderObjClass * Create_Render_Obj(const char * name);
	// unique to W3DAssetManager
	virtual HAnimClass *	Get_HAnim(const char * name);
	virtual bool Load_3D_Assets( const char * filename ); // This CANNOT be Bool, as it will not inherit properly if you make Bool == Int

	virtual TextureClass *	Get_Texture
	(
		const char * filename,
		MipCountType mip_level_count=MIP_LEVELS_ALL,
		WW3DFormat texture_format=WW3D_FORMAT_UNKNOWN,
		bool allow_compression=true,
		TextureBaseClass::TexAssetType type=TextureBaseClass::TEX_REGULAR,
		bool allow_reduction=true
	);

	//'Generals' customizations
	void Report_Used_Assets(void);
	void Report_Used_Prototypes (void);
	void Report_Used_Textures(void);
	void Report_Used_Font3DDatas( void );
	void Report_Used_FontChars (void);

	virtual RenderObjClass * Create_Render_Obj(const char * name,float scale, const int color, const char *oldTexure=NULL, const char *newTexture=NULL);
	///Swaps the specified textures in the render object prototype.
	int replacePrototypeTexture(RenderObjClass *robj, const char * oldname, const char * newname);

private:
	void Make_Mesh_Unique(RenderObjClass *robj,Bool geometry, Bool colors);
	void Make_HLOD_Unique(RenderObjClass *robj,Bool geometry, Bool colors);
	void Make_Unique(RenderObjClass *robj,Bool geometry, Bool colors);

	//'Generals' customizations
	int Recolor_Asset(RenderObjClass *robj, const int color);
	int Recolor_Mesh(RenderObjClass *robj, const int color);
	int Recolor_HLOD(RenderObjClass *robj, const int color);
	void Recolor_Vertex_Material(VertexMaterialClass *vmat, const int color);
	void Make_Mesh_Unique(RenderObjClass *robj, Bool colors);
	void Make_HLOD_Unique(RenderObjClass *robj, Bool colors);
	TextureClass * Find_Texture(const char * name, const int color);
	TextureClass * Recolor_Texture(TextureClass *texture, const int color);
	TextureClass * Recolor_Texture_One_Time(TextureClass *texture, const int color);
	void Remap_Palette(SurfaceClass *surface, const int color, Bool doPaletteOnly, Bool useAlpha);
	int replaceAssetTexture(RenderObjClass *robj, TextureClass *oldTex, TextureClass *newTex);
	int replaceHLODTexture(RenderObjClass *robj, TextureClass *oldTex, TextureClass *newTex);
	int replaceMeshTexture(RenderObjClass *robj, TextureClass *oldTex, TextureClass *newTex);

	//'E&B' customizations
/*	virtual RenderObjClass * Create_Render_Obj(const char * name, float scale, const Vector3 &hsv_shift);
	TextureClass * Get_Texture_With_HSV_Shift(const char * filename, const Vector3 &hsv_shift, TextureClass::MipCountType mip_level_count = TextureClass::MIP_LEVELS_ALL);
	void Recolor_Vertex_Material(VertexMaterialClass *vmat, const Vector3 &hsv_shift);
	void Recolor_Vertices(unsigned int *color, int count, const Vector3 &hsv_shift);
	void Recolor_Mesh(RenderObjClass *robj, const Vector3 &hsv_shift);
	TextureClass * Recolor_Texture(TextureClass *texture, const Vector3 &hsv_shift);
	TextureClass * Recolor_Texture_One_Time(TextureClass *texture, const Vector3 &hsv_shift);
	TextureClass * Find_Texture(const char * name, const Vector3 &hsv_shift);
	void Recolor_HLOD(RenderObjClass *robj, const Vector3 &hsv_shift);
	void Recolor_ParticleEmitter(RenderObjClass *robj, const Vector3 &hsv_shift);
	void Recolor_Asset(RenderObjClass *robj, const Vector3 &hsv_shift);*/
};
