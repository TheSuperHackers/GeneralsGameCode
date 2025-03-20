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

/*************************************************************************** 
 ***    C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S     *** 
 *************************************************************************** 
 *                                                                         * 
 *                 Project Name : Commando/G                               * 
 *                                                                         * 
 *                     $Archive:: /Commando/Code/ww3d2/txt2d.cpp          $* 
 *                                                                         * 
 *                      $Author:: Patrick                                 $* 
 *                                                                         * 
 *                     $Modtime:: 3/27/01 5:08p                           $* 
 *                                                                         * 
 *                    $Revision:: 3                                       $* 
 *                                                                         * 
 *-------------------------------------------------------------------------* 
 * Functions:                                                              * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "txt2d.h"
#include "FONT.H"
#include "assetmgr.h"
#include "ww3d.h"
#include "pot.h"
#include "bsurface.h"
#include "texture.h"
#include <stdio.h>
#include <stdarg.h>

#ifdef WW3D_DX8
#include <srTextureMap.hpp>

float Text2DObjClass::_LastWidth = 0.0f;
float Text2DObjClass::_LastHeight = 0.0f;

/************************************************************************** 
 * T2DOC::Text2DObjClass -- Constructor for 2D text objects               * 
 *                                                                        * 
 * INPUT:                                                                 * 
 *                                                                        * 
 * OUTPUT:                                                                * 
 *                                                                        * 
 * WARNINGS:                                                              * 
 *                                                                        * 
 * HISTORY:                                                               * 
 *   09/08/1997 PWG : Created.                                            * 
 *========================================================================*/
Text2DObjClass::Text2DObjClass(FontClass &font, const char *str, float screen_x, float screen_y, int fore, int back, ConvertClass &conv, bool center, bool clamp, ...):
	DynamicScreenMeshClass(2, 4)
{
	char text[255];
	// print the variable argument string into an argument list
	va_list args;
	va_start(args, clamp);
	vsprintf(text, str, args);
	va_end(args);

	Set_Text(font, text, screen_x, screen_y, fore, back, conv, center, clamp);
}


/************************************************************************** 
 * T2DOC::Text2DObjClass -- Constructor for 2D text objects               * 
 *                                                                        * 
 * INPUT:                                                                 * 
 *                                                                        * 
 * OUTPUT:                                                                * 
 *                                                                        * 
 * WARNINGS:                                                              * 
 *                                                                        * 
 * HISTORY:                                                               * 
 *   09/08/1997 PWG : Created.                                            * 
 *   01/28/2000 SKB : Dont' Set font width                                * 
 *========================================================================*/
void Text2DObjClass::Set_Text(FontClass &font, const char *str, float screen_x, float screen_y, int fore, int back, ConvertClass &conv, bool center, bool clamp, ...)
{
	int resw, resh, resbits;
	bool windowed;

	// find the resolution (for centering and pixel to pixel translation)
	WW3D::Get_Device_Resolution(resw, resh, resbits, windowed);

	// print the variable argument string into an argument list
	char		text[255];
	va_list	args;

	va_start(args, clamp);
	vsprintf(text, str, args);
	va_end(args);


	// if we didn't build a new texture than all the polygons will be just
	// fine so we don't need to do anything.
	if (!TextTexture.Build_Texture(font, text, fore, back, conv)) 
		return;

	// cache the TextTexture elements we think are important on the local 
	// stack.
	srTextureIFace *texture_map = TextTexture.Get_Texture();
	int tsize = TextTexture.Get_Texture_Size();

	// find the width and height of the text we want to display to the screen
	// note that the width could be more than the max width of a texture
	// so we must be prepared to break it down into blocks.  Theoretically the
	// height could be greater than the texture max but lets not be ridiculous

	// SKB: 1/25/99 This should not be determined by this function, it
	//					is up to the caller how he/she wants the font to look.
	//font.Set_XSpacing(-1);
	int fw		= font.String_Pixel_Width(text)+1;
	int fh		= font.Get_Height()+1;
	int mw		= (fw & (tsize - 1)) ? (fw / tsize)+1 : (fw /tsize);

	// if we requested the text to be centered around a point adjust the
	// coordinates accordingly.
	float x_delta = 0.0f;
	float y_delta = 0.0f;
	if (center) {
		x_delta = -((float)fw / (float)resw) / 2.0f;
		y_delta = -((float)fh / (float)resh) / 2.0f;
	}
	
	// Set so user can know where end of string is printed.								
	_LastWidth = ((float) fw / (float)resw);
	_LastHeight = ((float) fh / (float)resh);

	// if we requested that the coordinates be checked to ensure they are onscreen,
	// adjust the coordinates as neccessary.
	if (clamp) {

		if (screen_x < 0) screen_x = 0;
		if (screen_x + _LastWidth > 1.0f) {
			screen_x = 1.0f - _LastWidth;
			assert(screen_x >= 0.0f);
		}
		if (screen_y < 0) screen_y = 0;
		if (screen_y + _LastHeight > 1.0f) {
			screen_y = 1.0f - _LastHeight;
			assert(screen_y >= 0.0f);
		}
	}

	// for every square material it takes two triangles to represent it.
	Resize(mw * 2, mw * 4);

	// Set texture.
	Set_Texture(texture_map);

	// Set shader to 2d opaque preset.
	Set_Shader(ShaderClass::_PresetOpaque2DShader);

	// Sorting is always set on 2D objects so that sortbias can be used to
	// determine occlusion between them.
	Enable_Sort();

	// loop through the rows blocks of our text and make polygons mapped
	// with the correct section of our partial texture.
	for (int lp = 0; lp < mw; lp ++) {
		int pw = MIN(fw - (tsize *lp), tsize);
		int ph = fh;

		// calculate our actual texture coordinates based on the difference between
		// the width and height of the texture and the width and height the font
		// occupys.
		float tw = (float)pw / (float)tsize;
		float th = (float)ph / (float)tsize;
		float ty = ((float)fh * (float)lp) / (float)tsize;

		// figure out the screen space x and y positions of the object in question.
		float x	= ((float)lp * (float)tsize) / (float)resw;
//		x += (0.5f /resw);

		float y	= 0;
//		y += (0.5f /resh);

		// Adjust for centering if needed
		x += x_delta;
		y += y_delta;

		// convert image width and image height to normalized values
		float vw = (float)pw / (float)resw;
		float vh = (float)ph / (float)resh;

		Begin_Tri_Strip();
			Vertex( x, 			y, 		0, 	0, 	ty);
			Vertex( x + vw, 	y, 		0, 	tw, 	ty);
			Vertex( x, 			y + vh, 	0, 	0, 	ty + th);
			Vertex( x + vw, 	y + vh, 	0, 	tw, 	ty + th);
		End_Tri_Strip();
	}

	// fixup screen_x (align it with a pixel edge)
	float pixel_x, pixel_y;
	WW3D::Get_Pixel_Center(pixel_x, pixel_y);

	screen_x *= (float)resw;
	screen_x = floor(screen_x) + pixel_x;
	screen_x /= (float)resw;

	// fixup screen_y (align it with a pixel edge)
	screen_y *= (float)resh;
	screen_y = floor(screen_y) + pixel_y;
	screen_y /= (float)resh;

	Set_Position(Vector3(screen_x, screen_y, 0));

	Set_Dirty();
}

#endif //WW3D_DX8
