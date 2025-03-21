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
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Command & Conquer                                            * 
 *                                                                                             * 
 *                     $Archive:: /G/wwlib/Convert.h                                          $* 
 *                                                                                             * 
 *                      $Author:: Eric_c                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 4/02/99 11:59a                                              $*
 *                                                                                             * 
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef CONVERT_H
#define CONVERT_H

#include	"blitter.h"
#include	"PALETTE.H"
#include	"Surface.h"

/*
**	Flags used to fetch the appropriate blitter object.
*/
typedef enum {
	SHAPE_NORMAL 			= 0x0000,		// Standard shape (which is transparent)

	SHAPE_WIN_REL			= 0x0400,

	SHAPE_CENTER 			= 0x0200,		// Coords are based on shape's center pt

	SHAPE_DARKEN         = 0x0001,		// Force all pixels to darken the destination.
	SHAPE_TRANSLUCENT25  = 0x0002,		// Translucent to destination (25%).
	SHAPE_TRANSLUCENT50  = 0x0004,		// Translucent to destination (50%).
	SHAPE_TRANSLUCENT75  = 0x0006,		// Translucent to destination (75%).
	SHAPE_PREDATOR			= 0x0008,		// Predator effect
	SHAPE_REMAP				= 0x0010,		// Simple remap
	SHAPE_NOTRANS			= 0x0020			// A non transparent but otherwise standard shape
} ShapeFlags_Type;


/*
**	This class is the data that represents the marriage between a source art
**	palette and a destination palette/pixel-format. Facilities provied by this
**	class allow conversion from the source 8 bit pixel format to the correct 
**	screen pixel format.
**
**	Although this class can convert one pixel at a time through the conversion
**	function, the preferred way to convert pixels is through the translation
**	table provided. This table consists of 256 entries. Each entry is either 
**	an 8 bit or 16 bit pixel in the correct screen-space format. Use the 
**	Bytes_Per_Pixel() function to determine how to index into this translation
**	table.
**
**	Expected use of this class would be to create separate objects of this class for
**	every source art palette. For an 8 bit display, an additional object will be
**	required for every additional palette set to the video DAC registers. It is 
**	presumed that one general best-case palette will be used.
*/
class ConvertClass
{
	public:
		ConvertClass(PaletteClass const & artpalette, PaletteClass const & screenpalette, Surface const & typicalsurface);
		~ConvertClass(void);
		
		/*
		**	Convert from source pixel to dest screen pixel.
		*/
		int Convert_Pixel(int pixel) const {
			if (BBP == 1) return(((unsigned char const *)Translator)[pixel]);
			return(((unsigned short const *)Translator)[pixel]);
		}

		/*
		**	Fetch a blitter object to use according to the draw flags
		**	specified.
		*/
		Blitter const * Blitter_From_Flags(ShapeFlags_Type flags) const;
		RLEBlitter const * RLEBlitter_From_Flags(ShapeFlags_Type flags) const;

		/*
		**	This returns the bytes per pixel. Use this to determine how to index
		**	through the translation table.
		*/
		int Bytes_Per_Pixel(void) const {return(BBP);}

		/*
		**	Fetches the translation table. Sometimes the provided blitter objects
		**	won't suffice and manual access to the translation process is necessary.
		*/
		void const * Get_Translate_Table(void) const {return(Translator);}

		/*
		**	Sets the dynamic remap table so that the remapping blitters will use
		**	it without having to recreate the blitter objects.
		*/
		void Set_Remap(unsigned char const * remap) {RemapTable = remap;}

	protected:
		/*
		**	Bytes per pixel in screen format.
		*/
		int BBP;

		/*
		**	These are the blitter objects used to handle all the draw styles that this
		**	drawing dispatcher implements.
		*/
		Blitter * PlainBlitter;					// No transparency (rarely used).
		Blitter * TransBlitter;					// Skips transparent pixels.
		Blitter * ShadowBlitter;				// Shadowizes the destination pixels.
		Blitter * RemapBlitter;					//	Remaps source pixels then draws with transparency.
		Blitter * Translucent1Blitter;		// 25% translucent.
		Blitter * Translucent2Blitter;		// 50% translucent.
		Blitter * Translucent3Blitter;		//	75% translucent.

		/*
		**	These are the RLE aware blitters to handle all drawing styles that may
		**	be used by RLE compressed images.
		*/
		RLEBlitter * RLETransBlitter;					// Skips transparent pixels.
		RLEBlitter * RLEShadowBlitter;				// Shadowizes the destination pixels.
		RLEBlitter * RLERemapBlitter;					//	Remaps source pixels then draws with transparency.
		RLEBlitter * RLETranslucent1Blitter;		// 25% translucent.
		RLEBlitter * RLETranslucent2Blitter;		// 50% translucent.
		RLEBlitter * RLETranslucent3Blitter;		//	75% translucent.

		/*
		**	This is a translation table pointer. All source artwork is in 8 bit format.
		**	This table will translate this source pixel into a screen dependant pixel
		**	format datum.
		*/
		void * Translator;

		/*
		**	This will shade an 8 bit pixel to about 1/2 intensity.
		*/
		unsigned char * ShadowTable;

		/*
		**	Remap table pointer used for blits that require remapping. This value
		**	will change according to the draw parameter. The blitting routines keep track
		**	of this member object and use it to determine the remap table to use.
		*/
		mutable unsigned char const * RemapTable;
};

#endif
