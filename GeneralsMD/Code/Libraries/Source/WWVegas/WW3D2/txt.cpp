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

#include <stdio.h>
#include <math.h>
#include "pot.h"
#include "txt.h"
#include "bsurface.h"
#include "texture.h"
#include "FONT.H"
#include "nstrdup.h"

TextTextureClass::TextTextureClass(void) :
	TextureString(NULL),
	Texture(NULL),
	TextureSize(0),
	ForegroundColor(0),
	BackgroundColor(0),
	Font(0),
	Convert(0)
{
}

TextTextureClass::~TextTextureClass(void)
{
	REF_PTR_RELEASE(Texture);

	if (TextureString) {
		delete[] TextureString;
	}
}

bool TextTextureClass::Is_Texture_Valid(FontClass &font, const char *str, int fore, int back, ConvertClass &conv)
{
	// if there is no texture at all then obviously it is not valid
	if (!Texture) return false;

	// if all the parameters the texture was created with the last time are the same as the parameters the texture
	// was created with this time then the texture is fine!
	if (TextureString == str && Font == &font && Convert == &conv && ForegroundColor == fore && BackgroundColor == back)
		return true;

	return false;
}

bool TextTextureClass::Build_Texture(FontClass &font, const char *str, int fore, int back, ConvertClass &conv)
{
	static char default_font_palette[] = {
		0, 14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	};

	// right off determine if we need to rebuild the texture or the current texture
	// is just fine.  If we are rebuilding store of the properties of the texture
	// we are building for next time.
	if (Is_Texture_Valid(font, str, fore, back, conv)) {
		return false;
	} else {
		if (TextureString) delete[] TextureString;
		Font					= &font;
		TextureString		= nstrdup(str);
		Convert				= &conv;
		BackgroundColor	= back;
		ForegroundColor	= fore;
	}
	

	default_font_palette[0] = back;
	default_font_palette[1] = fore;

	// find the width and height of the text we want to display to the screen
	// note that the width could be more than the max width of a texture
	// so we must be prepared to break it down into blocks.  Theoretically the
	// height could be greater than the texture max but lets not be ridiculous
	int fw		= font.String_Pixel_Width(str)+1;
	int fh		= font.Get_Height()+1;

	// Note: we are currently printing the text into
	// a rectangular buffer.  We will later blit the text into a square buffer
	// since some cards require square textures.
	Rect rect(0,0,fw,fh);
	BSurface bsurf(fw, fh, 1);
	bsurf.Fill(0);
	font.Print(str, bsurf, rect, TPoint2D<int>(0,0), conv, (unsigned char *)default_font_palette);

	// figure out the size of the best texture which can hold the
	// text we wrote.  Since textures need to be assumed to be square 
	// the best size should be found by finding the  rectangular surface 
	// area of the text, and creating a Power of Two sized square which 
	// can hold the data in the ractangular surface area.
   float fsize = sqrt(fw * fh);
	TextureSize = Find_POT(ceil(fsize));
	// If this is still not enough, quadruple area:
	if ((TextureSize / fh) * TextureSize < fw) {
		TextureSize *= 2;
	}	

	// we now need to create a westwood style surface out of the surrender surface
	// so we can use all the westwood drawing primitives.  Clear this surface to
	// black (shouldn't need to do this I dont think)
	BSurface bsurf2(TextureSize, TextureSize, 1);
	bsurf2.Fill(0);

	// we need to calculate how many texture pot widths it takes
	// to hold the width of our font.
	int mw		= (fw & (TextureSize - 1)) ? (fw / TextureSize)+1 : (fw /TextureSize);

	// now we need to blit the old surface into the new surface (effectively
	// making the text in a square surface).
	for (int lp = 0; lp < mw; lp ++) {
		int blitw = MIN(fw - (TextureSize *lp), TextureSize);
		int lp_tsize = lp * TextureSize;
		Rect destrect(0,fh*lp,blitw,fh*lp+fh);
		Rect srcrect(lp_tsize, 0, lp_tsize + blitw, fh);
		/* original code
		d->blit(0, fh * lp, *s, lp_tsize, 0, lp_tsize + blitw, fh);
		*/
		bsurf2.Blit_From(destrect,bsurf,srcrect);
	}

	// Create texture without mip mapping
	REF_PTR_RELEASE(Texture);

	// TODO: copy bsurf2 into texture

	return true;
}

