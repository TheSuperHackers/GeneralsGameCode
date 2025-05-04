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
 *                 Project Name : Commando	                              * 
 *                                                                         * 
 *                     $Archive:: /Commando/Code/ww3d2/textdraw.h         $* 
 *                                                                         * 
 *                      $Author:: Greg_h                                  $* 
 *                                                                         * 
 *                     $Modtime:: 3/15/01 3:41p                           $* 
 *                                                                         * 
 *                    $Revision:: 4                                       $* 
 *                                                                         * 
 *-------------------------------------------------------------------------*/


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef TEXTDRAW_H
#define TEXTDRAW_H

#include "always.h"
#include "dynamesh.h"

// sgc : wwlib and wwmath contain different rect.h files...
#include "../WWMath/rect.h"

class	Font3DInstanceClass;

/******************************************************************
**
** TextDrawClass
**
** This class provides a simple method to draw 2D text into a scene.
** Both strings and individual characters can be drawn to any normalized 
** screen coordinates ( 0.. 1 ), or any scale/offset. 
**  This class uses a dynamic mesh for all polygon and vertex management
**
*******************************************************************/

class TextDrawClass : public DynamicMeshClass
{

public:
	/*
	** Constructor and Destructor
	*/
	TextDrawClass( int max_chars );
	~TextDrawClass();

	// Set Coordinate Range
	void	Set_Coordinate_Ranges(	const Vector2 & param_ul, const Vector2 & param_lr, 
											const Vector2 & dest_ul, const Vector2 & dest_lr );

	// Reset all polys and verts
	virtual	void Reset( void );

	/*
	** class id of this render object
	*/
	virtual int	Class_ID(void) const	{ return CLASSID_TEXTDRAW; }

	/*
	**
	*/
	float	Get_Width( Font3DInstanceClass *font, const char *message );
	float	Get_Char_Width( Font3DInstanceClass *font, const char c );
	float	Get_Inter_Char_Width( Font3DInstanceClass *font );
	float	Get_Height( Font3DInstanceClass *font, const char *message = "" );

	/*
	** Print the given char/string with the given font at the given loation in screen pixels
	** returns the pixel width of the drawn data.
	*/
	float	Print( Font3DInstanceClass *font, char ch, float screen_x, float screen_y);
	float	Print( Font3DInstanceClass *font, const char *message, float screen_x, float screen_y);

	void	Set_Text_Color( const Vector3 & color )		{ Set_Vertex_Color(color); }

	/*
	** dump the font image (debuging)
	*/
	void	Show_Font( Font3DInstanceClass *font, float screen_x, float screen_y );

	void Quad( float x0, float y0, float x1, float y1, float u0 = 0, float v0 = 0, float u1 = 1, float v1 = 1);
	void Quad( const RectClass	& rect, const RectClass	& uv = RectClass( 0, 0, 1, 1 ) );
	void Line( const Vector2 & a, const Vector2 & b, float width );
	void Line_Ends( const Vector2 & a, const Vector2 & b, float width, float end_percent );

private:
	Vector3					TextColor;
	VertexMaterialClass	*DefaultVertexMaterial;
	ShaderClass				DefaultShader;
	Vector2					TranslateScale;
	Vector2					TranslateOffset;
	Vector2					PixelSize;
};

#endif	// TEXTDRAW_H
