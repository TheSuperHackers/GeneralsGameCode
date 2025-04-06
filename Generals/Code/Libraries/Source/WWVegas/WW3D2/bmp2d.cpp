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

/*************************************************************************** 
 ***    C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S     *** 
 *************************************************************************** 
 *                                                                         * 
 *                 Project Name : Commando/G                               * 
 *                                                                         * 
 *                     $Archive:: /Commando/Code/ww3d2/bmp2d.cpp          $* 
 *                                                                         * 
 *                      $Author:: Naty_h                                  $* 
 *                                                                         * 
 *                     $Modtime:: 4/13/01 1:37p                           $* 
 *                                                                         * 
 *                    $Revision:: 10                                      $* 
 *                                                                         * 
 *-------------------------------------------------------------------------* 
 * Functions:                                                              * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "bmp2d.h"
#include "pot.h"
#include "ww3d.h"
#include "texture.h"
#include "surfaceclass.h"

Bitmap2DObjClass::Bitmap2DObjClass
(
	const char *filename,
	float screen_x,
	float screen_y,
	bool center,
	bool additive,
	bool colorizable,
	int usable_width,
	int usable_height,
	bool ignore_alpha
)
	:	DynamicScreenMeshClass(2, 4)
{
	
	int resw, resh, resbits;
	bool windowed;
	
	// find the resolution (for centering and pixel to pixel translation)
	WW3D::Get_Device_Resolution(resw, resh, resbits, windowed);
	// This should be the correct way to do things
	// but other code expects an aspect ratio of 1.0
	// Hector Yee 2/22/01
	// Set_Aspect(resh/(float)resw);
	

	// load up the surfaces file name
	SurfaceClass *surface=NEW_REF(SurfaceClass,(filename));	

	SurfaceClass::SurfaceDescription sd;
	surface->Get_Description(sd);

	if (usable_width == -1)
		usable_width = sd.Width;
	if (usable_height == -1)
		usable_height = sd.Height;

	// if we requested the image to be centered around a point adjust the
	// coordinates accordingly.
	if (center) {
		screen_x -= ((float)usable_width  / resw) / 2;
		screen_y -= ((float)usable_height / resh) / 2;
	}

	// The image will be broken down into square textures. The size of these
	// textures will be the smallest POT (power of two) which is equal or
	// greater than the smaller dimension of the image. Also, the pieces can
	// never be larger than 256 texels because some rendering devices don't
	// support textures larger than that.
	int surf_w = usable_width;
	int surf_h = usable_height;
	int piece = Find_POT(MIN(surf_w, surf_h));
	piece = MIN(piece, 256);

	// now take the image in question and break it down into
	// "piece"x"piece"-pixel polygons and calculate the number of textures
	// based from those calculations.
	int mw = (surf_w & (piece - 1)) ? (surf_w / piece)+1 : (surf_w /piece);
	int mh = (surf_h & (piece - 1)) ? (surf_h / piece)+1 : (surf_h /piece);
	
	// for every square texture it takes four vertexes to express the two
	// polygons.
	Resize(mw * mh *2, mw * mh * 4);

	// Set shader to additive if requested, else alpha or opaque depending on
	// whether the texture has an alpha channel. Sorting is always set so that
	// sortbias can be used to determine occlusion between the various 2D
	// elements.
	ShaderClass shader;

	if (additive) {
		shader = ShaderClass::_PresetAdditive2DShader;
	} else {
		if (ignore_alpha == false && Has_Alpha(sd.Format)) {
			shader = ShaderClass::_PresetAlpha2DShader;
		} else {
			shader = ShaderClass::_PresetOpaque2DShader;
		}
	}

	Enable_Sort();


	// If we want to be able to colorize this bitmap later (by setting
	// emissive color for the vertex material, or via a vertex emissive color
	// array) we need to enable the primary gradient in the shader (it is
	// disabled in the 2D presets), and set an appropriate vertex material.
	if (colorizable) {
		shader.Set_Primary_Gradient(ShaderClass::GRADIENT_MODULATE);
		VertexMaterialClass *vertex_material = NEW_REF( VertexMaterialClass, ());
		vertex_material->Set_Ambient(0.0f, 0.0f, 0.0f);
		vertex_material->Set_Diffuse(0.0f, 0.0f, 0.0f);
		vertex_material->Set_Specular(0.0f, 0.0f, 0.0f);
		vertex_material->Set_Emissive(1.0f, 1.0f, 1.0f);
		Set_Vertex_Material(vertex_material, true);
		vertex_material->Release_Ref();
	}

	// Set desired shader.
	Set_Shader(shader);

	// loop through the rows and columns of the image and make valid
	// textures from the pieces.
	for (int lpy = 0, tlpy=0; lpy < mh; lpy++, tlpy += piece) {
		for (int lpx = 0, tlpx = 0; lpx < mw; lpx++, tlpx += piece) {

			// figure the desired width and height of the texture (max "piece")
			int iw				= MIN(piece, usable_width - (tlpx));
			int ih				= MIN(piece, usable_height - (tlpy));
			int pot				= MAX(Find_POT(iw), Find_POT(ih));

			// create the texture and turn MIP-mapping off.
			SurfaceClass *piece_surface=NEW_REF(SurfaceClass,(pot,pot,sd.Format));			
			piece_surface->Copy(0,0,tlpx,tlpy,pot,pot,surface);
			TextureClass *piece_texture =NEW_REF(TextureClass,(piece_surface,MIP_LEVELS_1));			
			piece_texture->Get_Filter().Set_U_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
			piece_texture->Get_Filter().Set_V_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
			REF_PTR_RELEASE(piece_surface);			

			// calculate our actual texture coordinates based on the difference between
			// the width and height of the texture and the width and height the font
			// occupys.
			float tw = (float)iw / (float)pot;
			float th = (float)ih / (float)pot;

			// convert image width and image height to normalized values
			float vw = (float)iw / (float)resw;
			float vh = (float)ih / (float)resh;

			// figure out the screen space x and y positions of the object in question.
			float x	= screen_x + (((float)tlpx) / (float)resw);
			float y	= screen_y + (((float)tlpy) / (float)resh);
			
			Set_Texture(piece_texture);
			Begin_Tri_Strip();
				Vertex( x, 			y, 		0, 	0, 	0);
				Vertex( x + vw, 	y, 		0, 	tw, 	0);
				Vertex( x, 			y + vh, 	0, 	0, 	th);
				Vertex( x + vw, 	y + vh, 	0, 	tw, 	th);
			End_Tri_Strip();

			// release our reference to the texture
			REF_PTR_RELEASE(piece_texture);			
		}
	}
	REF_PTR_RELEASE(surface);	

	Set_Dirty();
}

Bitmap2DObjClass::Bitmap2DObjClass
(
	TextureClass *texture,
	float screen_x,
	float screen_y,
	bool center,
	bool additive,
	bool colorizable,
	bool ignore_alpha
)
	:	DynamicScreenMeshClass(2, 4)
{
	int resw, resh, resbits;
	bool windowed;

	// find the resolution (for centering and pixel to pixel translation)
	WW3D::Get_Device_Resolution(resw, resh, resbits, windowed);
	// This should be the correct way to do things
	// but other code expects an aspect ratio of 1.0
	// Hector Yee 2/22/01
	//Set_Aspect(resh/(float)resw);

	// Find the dimensions of the texture:
	SurfaceClass::SurfaceDescription sd;
	texture->Get_Level_Description(sd);
		
	// convert image width and image height to normalized values
	float vw = (float) sd.Width / (float)resw;
	float vh = (float) sd.Height / (float)resh;

	// if we requested the image to be centered around a point adjust the
	// coordinates accordingly.
	if (center) {
		screen_x -= vw / 2;
		screen_y -= vh / 2;
	}

	// Set shader to additive if requested, else alpha or opaque depending on whether the texture
	// has an alpha channel. Sorting is never set - if you wish to sort these types of objects you
	// should use static sort levels (note that static sort levels are not implemented for these
	// objects yet, but it should be very simple to do).
	ShaderClass shader;

	if (additive) {
		shader = ShaderClass::_PresetAdditive2DShader;
	} else {
		if (ignore_alpha == false && Has_Alpha(sd.Format)) {
			shader = ShaderClass::_PresetAlpha2DShader;
		} else {
			shader = ShaderClass::_PresetOpaque2DShader;
		}
	}
	Disable_Sort();

	// If we want to be able to colorize this bitmap later (by setting
	// emissive color for the vertex material, or via a vertex emissive color
	// array) we need to enable the primary gradient in the shader (it is
	// disabled in the 2D presets), and set an appropriate vertex material.
	if (colorizable) {
		shader.Set_Primary_Gradient(ShaderClass::GRADIENT_MODULATE);
		VertexMaterialClass *vertex_material = NEW_REF( VertexMaterialClass, ());
		vertex_material->Set_Ambient(0.0f, 0.0f, 0.0f);
		vertex_material->Set_Diffuse(0.0f, 0.0f, 0.0f);
		vertex_material->Set_Specular(0.0f, 0.0f, 0.0f);
		vertex_material->Set_Emissive(1.0f, 1.0f, 1.0f);
		Set_Vertex_Material(vertex_material, true);
		vertex_material->Release_Ref();
	}

	// Set desired shader.
	Set_Shader(shader);

	// Set texture to requested texture:
	Set_Texture(texture);

	Begin_Tri_Strip();
		Vertex( screen_x,			screen_y,		0,	0,		0);
		Vertex( screen_x + vw, 	screen_y,		0,	1.0,	0);
		Vertex( screen_x, 		screen_y + vh,	0,	0,		1.0);
		Vertex( screen_x + vw, 	screen_y + vh,	0,	1.0,	1.0);
	End_Tri_Strip();

	Set_Dirty();
}

RenderObjClass * Bitmap2DObjClass::Clone(void) const
{
	return NEW_REF( Bitmap2DObjClass, (*this));
}
