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

// TileData.h
// Class to hold 1 tile's data.
// Author: John Ahlquist, April 2001

#pragma once

#include <stdio.h>

#include "Lib/BaseType.h"
#include "WWLib/refcount.h"
#include "Common/AsciiString.h"

typedef struct {
	Int blendNdx;
	UnsignedByte horiz;
	UnsignedByte vert;
	UnsignedByte rightDiagonal;
	UnsignedByte leftDiagonal;
	UnsignedByte inverted;
	UnsignedByte longDiagonal;
	Int customBlendEdgeClass; // Class of texture for a blend edge.  -1 means use alpha.
} TBlendTileInfo;

#define INVERTED_MASK	0x1		//AND this with TBlendTileInfo.inverted to get actual inverted state
#define FLIPPED_MASK	0x2		//AND this with TBlendTileInfo.inverted to get forced flip state (for horizontal/vertical flips).
#define TILE_PIXEL_EXTENT 64
#define TILE_BYTES_PER_PIXEL 4
#define DATA_LEN_BYTES TILE_PIXEL_EXTENT*TILE_PIXEL_EXTENT*TILE_BYTES_PER_PIXEL
#define DATA_LEN_PIXELS TILE_PIXEL_EXTENT*TILE_PIXEL_EXTENT
#define TILE_PIXEL_EXTENT_MIP1 32
#define TILE_PIXEL_EXTENT_MIP2 16
#define TILE_PIXEL_EXTENT_MIP3 8
#define TILE_PIXEL_EXTENT_MIP4 4
#define TILE_PIXEL_EXTENT_MIP5 2
#define TILE_PIXEL_EXTENT_MIP6 1
#define TEXTURE_WIDTH 2048 // was 1024 jba

/** This class holds the bitmap data from the .tga texture files.  It is used to
create the D3D texture in the game and 3d windows, and to create DIB data for the
2d window. */
class TileData : public RefCountClass
{
protected:

	// data is bgrabgrabgra to be compatible with windows blt. jba.
	// Also, first byte is lower left pixel, not upper left pixel.
	// so 0,0 is lower left, not upper left.
	UnsignedByte m_tileData[DATA_LEN_BYTES];
	/// Mipped down copies of the tile data.
	UnsignedByte m_tileDataMip32[TILE_PIXEL_EXTENT_MIP1*TILE_PIXEL_EXTENT_MIP1*TILE_BYTES_PER_PIXEL];
	UnsignedByte m_tileDataMip16[TILE_PIXEL_EXTENT_MIP2*TILE_PIXEL_EXTENT_MIP2*TILE_BYTES_PER_PIXEL];
	UnsignedByte m_tileDataMip8[TILE_PIXEL_EXTENT_MIP3*TILE_PIXEL_EXTENT_MIP3*TILE_BYTES_PER_PIXEL];
	UnsignedByte m_tileDataMip4[TILE_PIXEL_EXTENT_MIP4*TILE_PIXEL_EXTENT_MIP4*TILE_BYTES_PER_PIXEL];
	UnsignedByte m_tileDataMip2[TILE_PIXEL_EXTENT_MIP5*TILE_PIXEL_EXTENT_MIP5*TILE_BYTES_PER_PIXEL];
	UnsignedByte m_tileDataMip1[TILE_PIXEL_EXTENT_MIP6*TILE_PIXEL_EXTENT_MIP6*TILE_BYTES_PER_PIXEL];

public:
	ICoord2D	m_tileLocationInTexture;


protected:
	/** doMip - generates the next mip level mipping pHiRes down to pLoRes.
				pLoRes is 1/2 the width of pHiRes, and both are square. */
	static void doMip(UnsignedByte *pHiRes, Int hiRow, UnsignedByte *pLoRes);



public:
	TileData(void);

public:
	UnsignedByte *getDataPtr(void) {return(m_tileData);};
	static Int dataLen(void) {return(DATA_LEN_BYTES);};

	void updateMips(void);

	Bool hasRGBDataForWidth(Int width);
	UnsignedByte *getRGBDataForWidth(Int width);
};
