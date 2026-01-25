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
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando / G 3D Library                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/font3d.cpp                             $*
 *                                                                                             *
 *                  $Org Author:: Jani_p                                  $*
 *                                                                         *
 *                      $Author:: Kenny_m                                  $*
 *                                                                         *
 *                     $Modtime:: 08/05/02 10:44a                          $*
 *                                                                                             *
 *                    $Revision:: 17                                                          $*
 *                                                                                             *
 * 08/05/02 KM Texture class redesign
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "font3d.h"
#include "assetmgr.h"
#include "texture.h"
#include <assert.h>
#include <wwdebug.h>
#include "surfaceclass.h"
#include "texture.h"
#include "vector2i.h"

static	SurfaceClass	*_surface;

/***********************************************************************************************
 *                                                                                             *
 * Font3DDataClass::Font3DDataClass -- constructor																	  *
 *                                                                                             *
 * Constructs and load a Targa font image to create a texture matetial								  *
 *                                                                                             *
 ***********************************************************************************************/
Font3DDataClass::Font3DDataClass( const char *filename )
{
	Texture = nullptr;
	Load_Font_Image( filename);
	Name = strdup( filename);
	Name = strupr( Name);
}


/***********************************************************************************************
 *                                                                                             *
 * Font3DDataClass::~Font3DDataClass -- destructor																	  *
 *                                                                                             *
 ***********************************************************************************************/
Font3DDataClass::~Font3DDataClass(void)
{
	free(Name);
	Name = nullptr;

	REF_PTR_RELEASE(Texture);
}


/***********************************************************************************************
 *                                                                                             *
 * FontClass::Minimize_Font_Image																				  *
 *                                                                                             *
 * Rebuilds the give image to better pack characters and to insure a square power of two size  *
 * Must be called AFTER Make_Proportional() so each chars minimal bounding box is known        *
 * Will only create a new texture of size 128x128 or 256x256, dependent on original width      *
 *                                                                                             *
 ***********************************************************************************************/
SurfaceClass *Font3DDataClass::Minimize_Font_Image( SurfaceClass *surface )
{
	SurfaceClass::SurfaceDescription sd;

	surface->Get_Description(sd);

	float current_width = sd.Width;
	float current_height = sd.Height;

	// determine new width make the size of the new image either 128x128 or 256x256,
	// dependent on the width of the original image
   int new_width;
	if (current_width < 256) {
		new_width = 128;
	} else {
		new_width = 256;
	}

   int new_height = new_width;
	//  create a new 4 bit alpha image to build into
	// We dont support non-homogeneous copies just yet
	SurfaceClass	*new_surface = NEW_REF(SurfaceClass,(new_width, new_height,WW3D_FORMAT_A4R4G4B4));
	//SurfaceClass	*new_surface0 = NEW_REF(SurfaceClass,(new_width, new_height,sd.Format));

	// fill with transparent black
	new_surface->Clear();

	// indices for the location of each added char
	int	new_x = 0;
	int	new_y = 0;

	// for each character, copy minimum bounding area to (new_x, new_y) in the new image
	for (int char_index = 0; char_index < 256; char_index++) {

		// find the lop left coordinate and the height and width of the char's bounding box
		// (must convert the normalized uv tables to pixels and round off)
		int src_x = (int)(UOffsetTable[ char_index ] * current_width + 0.5);
		int src_y = (int)(VOffsetTable[ char_index ] * current_height + 0.5);
		int width = (int)(UWidthTable[ char_index ] * current_width + 0.5);
		int height = (int)(VHeight * current_height + 0.5);

		// if the character has any visible pixels at all...
		if (width != 0) {

			// if this charactger will not fit on the current line, goto the next line
			if (new_x + width > new_width) {
				new_x = 0;
				new_y += height;

				// if we have run out of lines, we have a problem
				// we assert because we have already modified tables for some of the chars
				if (new_y + height > new_height) {
					new_y -= height;
					WWDEBUG_SAY(( "Font doesn't fit texture 2 on char %c", char_index ));
				}
			}

			// blit from original image to new image

			new_surface->Copy(new_x, new_y,src_x,src_y,width,height,surface);

		}

		// update the U and V tables to show new character location
		UOffsetTable[ char_index ] = (float)(new_x) / (float)new_width;
		VOffsetTable[ char_index ] = (float)(new_y) / (float)new_width;

		// update width in terms of new normal image width
		UWidthTable[ char_index ] *= (float)current_width / (float)new_width;

		new_x += width;
	}

	// update height in terms of new normal image height
	VHeight *= (float)current_height / (float)new_height;

	// be sure the new image is SMALLER than the old image
//	assert ( (new_width * new_height) <= (current_width * current_height));

	// release the old surface and return the new one
	REF_PTR_RELEASE(surface);

	_surface = new_surface;

	return _surface;
}

/***********************************************************************************************
 *                                                                                             *
 * FontClass::Make_Proportional																					  *
 *                                                                                             *
 * Modifys U and Width tables to convert a monospace font into a proportional font.  Hieght	  *
 * remains the same.  Performed by getting the current mono-space bounding box and bringing	  *
 * in the left and right edges to the first non-transparent ( != 0 ) pixel.  Then the U and	  *
 * width tables are updated with the new values.  The image itself is not modified unless...	  *
 * 																														  *
 * we complete by calling Minimize_Font_Image to shink the image & insure a power of 2 square  *
 *                                                                                             *
 ***********************************************************************************************/
SurfaceClass *Font3DDataClass::Make_Proportional( SurfaceClass	*surface )
{
	SurfaceClass::SurfaceDescription sd;
	surface->Get_Description(sd);
	float width  =	sd.Width;
	float height =	sd.Height;

	// for each character in the font...
	for (int char_index = 0; char_index < 256; char_index++) {

		// find the current bounding box
		// (must convert the normalized uv tables to pixels and round off)
		int x0 = (int)(UOffsetTable[ char_index ] * width + 0.5);
		int y0 = (int)(VOffsetTable[ char_index ] * height + 0.5);
		int x1 = x0 + (int)(UWidthTable[ char_index ] * width + 0.5);
		int y1 = y0 + (int)(VHeight * height + 0.5);

		//	find minimum bounding box by finding the minimum and maximum non-0 x pixel location
		Vector2i minb(x0,y0);
		Vector2i maxb(x1,y1);

		surface->FindBB(&minb,&maxb);

		// set the new edges
		x0 = minb.I;
		x1 = maxb.I+1;

		// if we didn't find ANY non-transparent pixels, the char has no width.
		if (x1 < x0) {
			x1 = x0;
		}

		// turn off all character after del
		if (char_index > 0x80) {
			x1 = x0;
		}

		// update the U and width tables
		UOffsetTable[ char_index ] = (float)x0 / width;
		UWidthTable[ char_index ] = (float)( x1 - x0 ) / width;
		CharWidthTable[ char_index ] = x1 - x0;
	}

	// now shink the image given the minimum char sizes
//	surface = Minimize_Font_Image( surface );
	Minimize_Font_Image( _surface );
	return nullptr;
}

/***********************************************************************************************
 *                                                                                             *
 * Font3DDataClass::Load_Font_Image( SR_SCENE *scene, char *filename )								  *
 *                                                                                             *
 * Loads a targa font image file, arranged as 16x16 characters, and builds u v tables to 		  *
 * find each character.  Converts the mono-space font into a proportional font, then uploads	  *
 * the image to the scene as a textur material.                                                *
 *                                                                                             *
 ***********************************************************************************************/
bool	Font3DDataClass::Load_Font_Image( const char *filename )
{
	// get the font surface
	SurfaceClass	*surface = NEW_REF(SurfaceClass,(filename));
	WWASSERT(surface);

	SurfaceClass::SurfaceDescription sd;
	surface->Get_Description(sd);

	// If input is a font strike (strip) process it as such
	if ( sd.Width > 8 * sd.Height ) {

 		// the height of the strike is the height of the characters
		VHeight = 1;
		CharHeight = sd.Height;

		int	column = 0;
		int	width = sd.Width;


		// for each char, find the uv start location and set the
		// mono-spaced width and height in normalized screen units
		for (int char_index = 0; char_index < 256; char_index++) {

			if ( char_index >= 0x7F ) {

				UOffsetTable[ char_index ] = 0;
				VOffsetTable[ char_index ] = 0;
				UWidthTable[ char_index ] = 0;
				CharWidthTable[ char_index ] = 0;

			} else {

				// find the first non-transparent column...
				while (( column < width ) && ( surface->Is_Transparent_Column(column) )) column++;
				int start = column;

				// find the first transparent column...
				while (( column < width ) && ( !surface->Is_Transparent_Column(column) )) column++;
				int end = column;

				if ( end <= start ) {
					WWDEBUG_SAY(( "Error Char %d start %d end %d width %d", char_index, start, end, width ));
				}

//				WWASSERT( end > start );

				UOffsetTable[ char_index ] = (float)start / width;
				VOffsetTable[ char_index ] = 0;
				UWidthTable[ char_index ] = (float)(end - start) / width;
				CharWidthTable[ char_index ] = end - start;
			}

		}

		// convert the just created mon-spaced font to proportional (optional)
//		surface = Make_Proportional( surface );
		_surface = surface;
		surface = nullptr;
		Minimize_Font_Image( _surface );

	} else {

		// Determine the width and height of each mono spaced character in pixels
		// (assumes 16x16 array of chars)
		float	font_width = sd.Width;
		float	font_height = sd.Height;
		float	mono_pixel_width = (font_width / 16);
		float	mono_pixel_height = (font_height / 16);

		// for each char, find the uv start location and set the
		// mono-spaced width and height in normalized screen units
		for (int char_index = 0; char_index < 256; char_index++) {
			UOffsetTable[ char_index ] = (float)((char_index % 16) * mono_pixel_width) / font_width;
			VOffsetTable[ char_index ] = (float)((char_index / 16) * mono_pixel_height) / font_height;
			UWidthTable[ char_index ] = mono_pixel_width / font_width;
			CharWidthTable[ char_index ] = mono_pixel_width;
		}
		VHeight = mono_pixel_height / font_height;
		CharHeight = mono_pixel_height;

		// convert the just created mon-spaced font to proportional (optional)

		_surface = surface;
		surface = nullptr;
		Make_Proportional( _surface );
	}

	// create the texture
	if ( _surface ) {
		Texture = NEW_REF(TextureClass,(_surface,MIP_LEVELS_1));
		REF_PTR_RELEASE(_surface);
	}

	// return SUCCESS!
	return true;
}


/***********************************************************************************************
 *                                                                                             *
 * Font3DInstanceClass::Font3DInstanceClass -- constructor											     *
 *                                                                                             *
 * Constructs and load a Targa font image to create a texture matetial								  *
 *                                                                                             *
 ***********************************************************************************************/
Font3DInstanceClass::Font3DInstanceClass( const char *filename )
{
	FontData = WW3DAssetManager::Get_Instance()->Get_Font3DData( filename);
	MonoSpacing = 0.0f;
	Scale = 1.0f;
	SpaceSpacing = (int)(FontData->Char_Width('H') / 2.0f);
	InterCharSpacing = 1;
	Build_Cached_Tables();
}

/***********************************************************************************************
 *                                                                                             *
 * Font3DInstanceClass::~Font3DInstanceClass -- destructor																	  *
 *                                                                                             *
 ***********************************************************************************************/
Font3DInstanceClass::~Font3DInstanceClass(void)
{
	REF_PTR_RELEASE(FontData);
}

/*
**
*/
void	Font3DInstanceClass::Set_Mono_Spaced( void )
{
	MonoSpacing = FontData->Char_Width('W') + 1;
	Build_Cached_Tables();
}

void	Font3DInstanceClass::Build_Cached_Tables()
{
	// Rebuild the cached tables
	for (int a=0;a<256;++a) {
		float width = (float)FontData->Char_Width(a);
		if ( a == ' ' ) {
			width = SpaceSpacing;
		}

		ScaledWidthTable[a] = Scale * width;
		if (MonoSpacing != 0.0f) {
			ScaledSpacingTable[a] = Scale * MonoSpacing;
		} else {
			ScaledSpacingTable[a] = Scale * (width + InterCharSpacing);
		}
	}
	ScaledHeight = floorf(Scale * (float)FontData->Char_Height('A'));
}

/***********************************************************************************************
 *                                                                                             *
 * Font3DInstanceClass::String_Screen_Width( char *test_str )									        *
 *                                                                                             *
 * Finds the normalized screenspace width of a character string - useful for checking before   *
 * printing to avoid overflowing the screen.																	  *                                                                                             *
 ***********************************************************************************************/
float	Font3DInstanceClass::String_Width( const WCHAR *test_str )
{
	float width = 0.0;
	for (; *test_str; test_str++) {
		width += Char_Spacing(*test_str);
	}

	return width;
}

float	Font3DInstanceClass::String_Width( const char *test_str )
{
	float width = 0.0;
	for (; *test_str; test_str++) {
		width += Char_Spacing(*test_str);
	}

	return width;
}

