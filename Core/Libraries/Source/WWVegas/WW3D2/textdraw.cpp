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
 *                 Project Name : Commando                                 * 
 *                                                                         * 
 *                     $Archive:: /Commando/Code/ww3d2/textdraw.cpp       $* 
 *                                                                         * 
 *                      $Author:: Jani_p                                  $* 
 *                                                                         * 
 *                     $Modtime:: 3/22/01 8:03p                           $* 
 *                                                                         * 
 *                    $Revision:: 7                                       $* 
 *                                                                         * 
 *-------------------------------------------------------------------------* 
 * Functions:                                                              * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "textdraw.h"
#include "font3d.h"
#include "simplevec.h"

/*********************************************************************************************** 
 *                                                                                             * 
 * TextDrawClass::TextDrawClass( int ) -- Constructor										              *
 *                                                                                             * 
 * Creates a TextDrawClass object by creating and initializing a Dynamic Mesh, inserting it	  *
 * into the given scene, and allocating space for the given number of maximum chars.			  *
 *                                                                                             * 
 ***********************************************************************************************/
TextDrawClass::TextDrawClass( int max_chars ) : 
	DynamicMeshClass( max_chars * 2, max_chars * 4 ),
	TextColor( 1.0f, 1.0f, 1.0f )
{
	// Build the default Vertex Material
	DefaultVertexMaterial = NEW_REF( VertexMaterialClass, () );
	DefaultVertexMaterial->Set_Diffuse( 0, 0, 0 );	
	DefaultVertexMaterial->Set_Opacity(1);
	DefaultVertexMaterial->Set_Emissive( 1, 1, 1 );
	Set_Vertex_Material( DefaultVertexMaterial );

	// Build the default Shader
	DefaultShader.Set_Depth_Mask( ShaderClass::DEPTH_WRITE_DISABLE );
	DefaultShader.Set_Depth_Compare( ShaderClass::PASS_ALWAYS );
	DefaultShader.Set_Dst_Blend_Func( ShaderClass::DSTBLEND_ONE_MINUS_SRC_ALPHA );
	DefaultShader.Set_Src_Blend_Func( ShaderClass::SRCBLEND_SRC_ALPHA );
	DefaultShader.Set_Texturing( ShaderClass::TEXTURING_ENABLE );
	DefaultShader.Set_Cull_Mode( ShaderClass::CULL_MODE_DISABLE );
	Set_Shader( DefaultShader );
	Enable_Sort();

	Set_Coordinate_Ranges( Vector2( 0,0 ), Vector2( 1,1 ), Vector2( -1,0.75f ), Vector2( 1,-0.75 ) );
//	Set_Coordinate_Ranges( Vector2( -320,240 ), Vector2( 320,-240 ), Vector2( -320,240 ), Vector2( 320,-240 ) );
}

/*********************************************************************************************** 
 *                                                                                             * 
 * TextDrawClass::~TextDrawClass( void ) -- Destructor													  *
 *                                                                                             * 
 ***********************************************************************************************/
TextDrawClass::~TextDrawClass( void )
{
	DefaultVertexMaterial->Release_Ref();
}

/*********************************************************************************************** 
 *                                                                                             * 
 * TextDrawClass::Reset( void ) -- Flush the mesh
 *                                                                                             * 
 ***********************************************************************************************/
void TextDrawClass::Reset( void )
{
	Reset_Flags();	
	Reset_Mesh_Counters();	

	// Reinstall the default vertex material and shader
	Enable_Sort();
	Set_Vertex_Material( DefaultVertexMaterial );
	Set_Shader( DefaultShader );
}

/*
**
*/
void	TextDrawClass::Set_Coordinate_Ranges(	
			const Vector2 & src_ul, const Vector2 & src_lr, 
			const Vector2 & dest_ul, const Vector2 & dest_lr )
{
	TranslateScale.X = (dest_lr.X - dest_ul.X) / (src_lr.X - src_ul.X);
	TranslateScale.Y = (dest_lr.Y - dest_ul.Y) / (src_lr.Y - src_ul.Y);
	TranslateOffset.X = dest_ul.X - TranslateScale.X * src_ul.X;
	TranslateOffset.Y = dest_ul.Y - TranslateScale.Y * src_ul.Y;

	PixelSize.X = fabs((src_lr.X - src_ul.X) / 640.0f);
	PixelSize.Y = fabs((src_lr.Y - src_ul.Y) / 480.0f);
}


/*
**
*/
void TextDrawClass::Quad( float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1 )
{
	// Translate coordinates
	x0 = x0 * TranslateScale.X + TranslateOffset.X;
	x1 = x1 * TranslateScale.X + TranslateOffset.X;
	y0 = y0 * TranslateScale.Y + TranslateOffset.Y;
	y1 = y1 * TranslateScale.Y + TranslateOffset.Y;

	bool flip_faces = ((x1-x0) * (y1-y0)) > 0;

	Begin_Tri_Strip();
		Vertex( x0,	y0, 0, u0, v0);
		if ( flip_faces ) {
			Vertex( x1,	y0, 0, u1, v0);
			Vertex( x0,	y1, 0, u0, v1);
		} else {
			Vertex( x0,	y1, 0, u0, v1);
			Vertex( x1,	y0, 0, u1, v0);
		}
		Vertex( x1,	y1, 0, u1, v1);
	End_Tri_Strip();
}

void TextDrawClass::Quad( const RectClass	& rect, const RectClass	& uv )
{
	TextDrawClass::Quad( rect.Left, rect.Top, rect.Right, rect.Bottom, uv.Left, uv.Top, uv.Right, uv.Bottom );
}

void TextDrawClass::Line( const Vector2 & _a, const Vector2 & _b, float width )
{
	// Translate coordinates
	Vector2 a;
	Vector2 b;
	a.X = _a.X * TranslateScale.X + TranslateOffset.X;
	a.Y = _a.Y * TranslateScale.Y + TranslateOffset.Y;
	b.X = _b.X * TranslateScale.X + TranslateOffset.X;
	b.Y = _b.Y * TranslateScale.Y + TranslateOffset.Y;
	width *= WWMath::Fabs(TranslateScale.X);

	Vector2	corner_offset = a - b;							// get line relative to b
	float temp = corner_offset.X;					// Rotate 90
	corner_offset.X = corner_offset.Y;
	corner_offset.Y = -temp;
	corner_offset.Normalize();						// scale to length width/2
	corner_offset *= width / 2;

	Begin_Tri_Strip();
		Vertex( a + corner_offset );
		Vertex( a - corner_offset );
		Vertex( b + corner_offset );
		Vertex( b - corner_offset );
	End_Tri_Strip();
}

void TextDrawClass::Line_Ends( const Vector2 & a, const Vector2 & b, float width, float end_percent )
{
	Vector2 a_ = b - a;
	a_ *= end_percent;
	a_ += a;
	Line( a, a_, width );
	Vector2 b_ = a - b;
	b_ *= end_percent;
	b_ += b;
	Line( b, b_, width );
}


/*********************************************************************************************** 
 *                                                                                             * 
 * float	TextDrawClass::Get_Width( Font3DInstanceClass *, char * )									  *
 *                                                                                             * 
 * WARNING:  Should not be used to draw characters which need to wrap or have embedded line    *
 *           feeds.                                                                            *
 *                                                                                             * 
 * Returns the scaled string width in normalized screen unit                                   *
 *                                                                                             * 
 ***********************************************************************************************/
float	TextDrawClass::Get_Width( Font3DInstanceClass *font, const char *message )
{
	float total_width = 0.0f;

	/*
	** for each character, get_width it 
	*/
	while (*message != 0) {
		total_width += font->Char_Spacing( *message++ );
	}

	return total_width;
}

float	TextDrawClass::Get_Inter_Char_Width( Font3DInstanceClass *font )
{
	// Get rid of this...
//	return font->Get_Inter_Char_Spacing();
	return 1;
}

float	TextDrawClass::Get_Height( Font3DInstanceClass *font, const char *message )
{
	return	font->Char_Height();
}


/*********************************************************************************************** 
 *                                                                                             * 
 * float	TextDrawClass::Print( Font3DInstanceClass *, char, float, float, float )							  *
 *                                                                                             * 
 * Draws (actually creates two trianlges to display) a character on the screen at the given    *
 * normalized screen unit coordinates at the current font scale.                               *
 *                                                                                             * 
 * Returns the scaled character width in normalized screen unit                                *
 *                                                                                             * 
 ***********************************************************************************************/
float	TextDrawClass::Print( Font3DInstanceClass *font, char ch, float screen_x, float screen_y )
{
	/*
	** Get the char width in scaled normalized screen units
	*/
	float	width = font->Char_Width( ch ); 			// in scaled normalized screen units
	float	spacing = font->Char_Spacing( ch ); 	// in scaled normalized screen units

	/*
	** If asked to draw the space char (' '), don't add any triangles, just return the scaled spacing
	*/
	if (ch == ' ' )
		return spacing;

	/*
	** Calculate the lower right edge of the displayed rectangle
	*/
	// center each char in its spacing (in case mono spaced )
	// also, round to the nearest 640x480 pixels (needs to change for other reses)
	float	screen_x0 = screen_x + spacing/2 - width/2;
	screen_x0 = floor(screen_x0 / PixelSize.X + 0.5f) * PixelSize.X;
	float	screen_x1 = screen_x0 + width;
	screen_x1 = floor(screen_x1 / PixelSize.X + 0.5f) * PixelSize.X;
	float	screen_y0 = screen_y;
	screen_y0 = floor(screen_y0 / PixelSize.Y + 0.5f) * PixelSize.Y;
	float	screen_y1 = screen_y0 + (font->Char_Height() * WWMath::Sign( -TranslateScale.Y ));
	screen_y1 = floor(screen_y1 / PixelSize.Y + 0.5f) * PixelSize.Y;

	if ( WW3D::Is_Screen_UV_Biased() ) {	// Global bais setting
		screen_x0 += PixelSize.X / 2;
		screen_x1 += PixelSize.X / 2;
		screen_y0 += PixelSize.Y / 2;
		screen_y1 += PixelSize.Y / 2;
	}

	/*
	** Get the font texture uv coordinate for teh upper right and lower left corners of the rect
	*/
	RectClass font_uv = font->Char_UV( ch );

	/*
	** Set the triangles' texture
	*/
	Set_Texture( font->Peek_Texture( ch ) );

	/*
	** Draw the quad
	*/
	Quad( screen_x0, screen_y0, screen_x1, screen_y1, font_uv.Left, font_uv.Top, font_uv.Right, font_uv.Bottom );

	/*
	** return the scaled spacing
	*/
	return spacing;
}


/*********************************************************************************************** 
 *                                                                                             * 
 * float	TextDrawClass::Print( Font3DInstanceClass *, char *, float, float, float )						  *
 *                                                                                             * 
 * Draws the given string at the given pixel coordinates.  Uses the given font and its current *
 * scale.  Passes each character to the above routine and moves the x-coordinate forward after *
 * each char.                                                                                  *          * 
 *                                                                                             * 
 * WARNING:  Should not be used to draw characters which need to wrap or have embedded line    *
 *           feeds.                                                                            *                * 
 *                                                                                             * 
 * Returns the string pixel width																				  *
 *                                                                                             * 
 ***********************************************************************************************/
float	TextDrawClass::Print( Font3DInstanceClass *font, const char *message, float screen_x, float screen_y )
{
	/*
	** Keep track of the total drawn width
	*/
	float total_width = 0.0f;

	/*
	** for each character, print it and moved screen_x forward
	*/
	while (*message != 0) {
		float width = Print( font, *message++, screen_x, screen_y );
		screen_x += width;
		total_width += width;
	}

	/*
	** return the total drawn width
	*/
	return total_width;
}


/*********************************************************************************************** 
 *                                                                                             * 
 * void	TextDrawClass::Show_Font( Font3DInstanceClass *, float, float, float )								  *
 *                                                                                             * 
 * Dumps the font texture to the screen as two triangles. For debugging only.						  *
 *                                                                                             * 
 ***********************************************************************************************/
void	TextDrawClass::Show_Font( Font3DInstanceClass *font, float screen_x, float screen_y )
{
	// normalize coordinates
   float size_x = PixelSize.X * 256;
   float size_y = PixelSize.Y * 256;

	Set_Texture( font->Peek_Texture('A') );

	Quad( screen_x, screen_y, screen_x + size_x, screen_y + size_y * WWMath::Sign( -TranslateScale.Y ) );
}
