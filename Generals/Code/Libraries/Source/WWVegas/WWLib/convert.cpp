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
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /G/wwlib/Convert.cpp                                        $*
 *                                                                                             *
 *                      $Author:: Eric_c                                                      $*
 *                                                                                             *
 *                     $Modtime:: 2/19/99 11:51a                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"always.h"
#include	"blitblit.h"
#include	"Convert.h"
#include	"dsurface.h"
#include	"hsv.h"
#include	"rlerle.h"


ConvertClass::ConvertClass(PaletteClass const & artpalette, PaletteClass const & screenpalette, Surface const & surface) :
	BBP(surface.Bytes_Per_Pixel()),
	PlainBlitter(NULL),
	TransBlitter(NULL),
	ShadowBlitter(NULL),
	RemapBlitter(NULL),
	Translucent1Blitter(NULL),
	Translucent2Blitter(NULL),
	Translucent3Blitter(NULL),
	RLETransBlitter(NULL),
	RLEShadowBlitter(NULL),
	RLERemapBlitter(NULL),
	RLETranslucent1Blitter(NULL),
	RLETranslucent2Blitter(NULL),
	RLETranslucent3Blitter(NULL),
	Translator(NULL),
	ShadowTable(NULL),
	RemapTable(NULL)
{
	/*
	**	The draw data initialization is greatly dependant upon the pixel format
	**	of the display surface. Check the pixel format and set the values accordingly.
	*/
	if (BBP == 1) {

		/*
		**	Build the shadow table by creating a slightly darker version of
		**	the color and then finding the closest match to it.
		*/
		ShadowTable = W3DNEWARRAY unsigned char [256];
		ShadowTable[0] = 0;
		for (int shadow = 1; shadow < 256; shadow++) {
			HSVClass hsv = artpalette[shadow];
			hsv.Set_Value((unsigned char)(hsv.Get_Value() / 2));
			ShadowTable[shadow] = (unsigned char)artpalette.Closest_Color(hsv);
		}

		/*
		**	The translator table is created by finding the closest color
		**	in the display palette from each color in the source art
		**	palette.
		*/
		unsigned char * trans = W3DNEWARRAY unsigned char [256];
		trans[0] = 0;
		for (int index = 1; index < 256; index++) {
			trans[index] = (unsigned char)screenpalette.Closest_Color(artpalette[index]);
		}
		Translator = (void *)trans;

		/*
		**	Construct all the blitter objects necessary to support the functionality
		**	required for the draw permutations.
		*/
		PlainBlitter = W3DNEW BlitPlainXlat<unsigned char>((unsigned char const *)Translator);
		TransBlitter = W3DNEW BlitTransXlat<unsigned char>((unsigned char const *)Translator);
		RemapBlitter = W3DNEW BlitTransZRemapXlat<unsigned char>(&RemapTable, (unsigned char const *)Translator);
		ShadowBlitter = W3DNEW BlitTransRemapDest<unsigned char>(ShadowTable);
		Translucent1Blitter = W3DNEW BlitTransRemapXlat<unsigned char>(ShadowTable, (unsigned char const *)Translator);
		Translucent2Blitter = W3DNEW BlitTransRemapXlat<unsigned char>(ShadowTable, (unsigned char const *)Translator);
		Translucent3Blitter = W3DNEW BlitTransRemapXlat<unsigned char>(ShadowTable, (unsigned char const *)Translator);

		/*
		**	Create the RLE aware blitter objects.
		*/
		RLETransBlitter = W3DNEW RLEBlitTransXlat<unsigned char>((unsigned char const *)Translator);
		RLERemapBlitter = W3DNEW RLEBlitTransZRemapXlat<unsigned char>(&RemapTable, (unsigned char const *)Translator);
		RLEShadowBlitter = W3DNEW RLEBlitTransRemapDest<unsigned char>(ShadowTable);
		RLETranslucent1Blitter = W3DNEW RLEBlitTransRemapXlat<unsigned char>(ShadowTable, (unsigned char const *)Translator);
		RLETranslucent2Blitter = W3DNEW RLEBlitTransRemapXlat<unsigned char>(ShadowTable, (unsigned char const *)Translator);
		RLETranslucent3Blitter = W3DNEW RLEBlitTransRemapXlat<unsigned char>(ShadowTable, (unsigned char const *)Translator);

	} else {

		/*
		**	The hicolor translation table is constructed according to the pixel
		**	format of the display and the source art palette.
		*/
		//assert(surface.Is_Direct_Draw());
		Translator = W3DNEWARRAY unsigned short [256];
		((DSurface &)surface).Build_Remap_Table((unsigned short *)Translator, artpalette);

		/*
		**	Fetch the pixel mask values to be used for the various algorithmic
		**	pixel processing performed for hicolor displays.
		*/
		int maskhalf = ((DSurface &)surface).Get_Halfbright_Mask();
		int maskquarter = ((DSurface &)surface).Get_Quarterbright_Mask();

		/*
		**	Construct all the blitter objects necessary to support the functionality
		**	required for the draw permutations.
		*/
		PlainBlitter = W3DNEW BlitPlainXlat<unsigned short>((unsigned short const *)Translator);
		TransBlitter = W3DNEW BlitTransXlat<unsigned short>((unsigned short const *)Translator);
		RemapBlitter = W3DNEW BlitTransZRemapXlat<unsigned short>(&RemapTable, (unsigned short const *)Translator);
		ShadowBlitter = W3DNEW BlitTransDarken<unsigned short>((unsigned short)maskhalf);
		Translucent1Blitter = W3DNEW BlitTransLucent75<unsigned short>((unsigned short const *)Translator, (unsigned short)maskquarter);
		Translucent2Blitter = W3DNEW BlitTransLucent50<unsigned short>((unsigned short const *)Translator, (unsigned short)maskhalf);
		Translucent3Blitter = W3DNEW BlitTransLucent25<unsigned short>((unsigned short const *)Translator, (unsigned short)maskquarter);

		/*
		**	Create the RLE aware blitter objects.
		*/
		RLETransBlitter = W3DNEW RLEBlitTransXlat<unsigned short>((unsigned short const *)Translator);
		RLERemapBlitter = W3DNEW RLEBlitTransZRemapXlat<unsigned short>(&RemapTable, (unsigned short const *)Translator);
		RLEShadowBlitter = W3DNEW RLEBlitTransDarken<unsigned short>((unsigned short)maskhalf);
		RLETranslucent1Blitter = W3DNEW RLEBlitTransLucent75<unsigned short>((unsigned short const *)Translator, (unsigned short)maskquarter);
		RLETranslucent2Blitter = W3DNEW RLEBlitTransLucent50<unsigned short>((unsigned short const *)Translator, (unsigned short)maskhalf);
		RLETranslucent3Blitter = W3DNEW RLEBlitTransLucent25<unsigned short>((unsigned short const *)Translator, (unsigned short)maskquarter);
	}
}


ConvertClass::~ConvertClass(void)
{
	delete PlainBlitter;
	PlainBlitter = NULL;

	delete TransBlitter;
	TransBlitter = NULL;

	delete ShadowBlitter;
	ShadowBlitter = NULL;

	delete RemapBlitter;
	RemapBlitter = NULL;

	delete Translucent1Blitter;
	Translucent1Blitter = NULL;

	delete Translucent2Blitter;
	Translucent2Blitter = NULL;

	delete Translucent3Blitter;
	Translucent3Blitter = NULL;

	delete [] Translator;
	Translator = NULL;

	delete [] ShadowTable;
	ShadowTable = NULL;

	delete RLETransBlitter;
	RLETransBlitter = NULL;

	delete RLEShadowBlitter;
	RLEShadowBlitter = NULL;

	delete RLERemapBlitter;
	RLERemapBlitter = NULL;

	delete RLETranslucent1Blitter;
	RLETranslucent1Blitter = NULL;

	delete RLETranslucent2Blitter;
	RLETranslucent2Blitter = NULL;

	delete RLETranslucent3Blitter;
	RLETranslucent3Blitter = NULL;
}


Blitter const * ConvertClass::Blitter_From_Flags(ShapeFlags_Type flags) const
{
	if (flags & SHAPE_REMAP) return(RemapBlitter);

	/*
	**	Quick check to see if this is a translucent operation. If so, then no
	**	further examination of the flags is necessary.
	*/
	switch (flags & (SHAPE_TRANSLUCENT25 | SHAPE_TRANSLUCENT50 | SHAPE_TRANSLUCENT75)) {
		case SHAPE_TRANSLUCENT25:
			return(Translucent3Blitter);

		case SHAPE_TRANSLUCENT50:
			return(Translucent2Blitter);

		case SHAPE_TRANSLUCENT75:
			return(Translucent1Blitter);
	}

	if (flags & SHAPE_DARKEN) return(ShadowBlitter);

	if (flags & SHAPE_NOTRANS) return(PlainBlitter);

	return(TransBlitter);
}


RLEBlitter const * ConvertClass::RLEBlitter_From_Flags(ShapeFlags_Type flags) const
{
	if (flags & SHAPE_REMAP) return(RLERemapBlitter);

	/*
	**	Quick check to see if this is a translucent operation. If so, then no
	**	further examination of the flags is necessary.
	*/
	switch (flags & (SHAPE_TRANSLUCENT25 | SHAPE_TRANSLUCENT50 | SHAPE_TRANSLUCENT75)) {
		case SHAPE_TRANSLUCENT25:
			return(RLETranslucent3Blitter);

		case SHAPE_TRANSLUCENT50:
			return(RLETranslucent2Blitter);

		case SHAPE_TRANSLUCENT75:
			return(RLETranslucent1Blitter);
	}

	if (flags & SHAPE_DARKEN) return(RLEShadowBlitter);

	// This should be fixed to return the RLEPlainBlitter when one is available
	// but if you need to use this in the mean time just don't RLE compress the
	// shape (since it only compresses transparent pixels and the reason we compress
	// them is so we can skip them easily.)
	if (flags & SHAPE_NOTRANS) return(RLETransBlitter);

	return(RLETransBlitter);
}




