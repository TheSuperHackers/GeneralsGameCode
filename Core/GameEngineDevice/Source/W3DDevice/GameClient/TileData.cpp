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

// TileData.cpp
// Class to handle tile data.
// Author: John Ahlquist, April 2001

#include "W3DDevice/GameClient/TileData.h"
#include "W3DDevice/GameClient/WorldHeightMap.h"



//
// TileData - no destructor.
//

//
// TileData - create a new texture tile .
//
TileData::TileData()
{

}

#define TILE_PIXEL_EXTENT_MIP1 32
#define TILE_PIXEL_EXTENT_MIP2 16
#define TILE_PIXEL_EXTENT_MIP3 8
#define TILE_PIXEL_EXTENT_MIP4 4
#define TILE_PIXEL_EXTENT_MIP5 2
#define TILE_PIXEL_EXTENT_MIP6 1

Bool TileData::hasRGBDataForWidth(Int width)
{
	if (width == TILE_PIXEL_EXTENT) return(true);
	if (width == TILE_PIXEL_EXTENT_MIP1) return(true);
	if (width == TILE_PIXEL_EXTENT_MIP2) return(true);
	if (width == TILE_PIXEL_EXTENT_MIP3) return(true);
	if (width == TILE_PIXEL_EXTENT_MIP4) return(true);
	if (width == TILE_PIXEL_EXTENT_MIP5) return(true);
	if (width == TILE_PIXEL_EXTENT_MIP6) return(true);
	return(false);
}

UnsignedByte * TileData::getRGBDataForWidth(Int width)
{
	// default
	if (width == TILE_PIXEL_EXTENT_MIP1) return(m_tileDataMip32);
	if (width == TILE_PIXEL_EXTENT_MIP2) return(m_tileDataMip16);
	if (width == TILE_PIXEL_EXTENT_MIP3) return(m_tileDataMip8);
	if (width == TILE_PIXEL_EXTENT_MIP4) return(m_tileDataMip4);
	if (width == TILE_PIXEL_EXTENT_MIP5) return(m_tileDataMip2);
	if (width == TILE_PIXEL_EXTENT_MIP6) return(m_tileDataMip1);
	return(m_tileData);
}

void TileData::updateMips(void)
{
	doMip(m_tileData, TILE_PIXEL_EXTENT, m_tileDataMip32);
	doMip(m_tileDataMip32, TILE_PIXEL_EXTENT_MIP1, m_tileDataMip16);
	doMip(m_tileDataMip16, TILE_PIXEL_EXTENT_MIP2, m_tileDataMip8);
	doMip(m_tileDataMip8, TILE_PIXEL_EXTENT_MIP3, m_tileDataMip4);
	doMip(m_tileDataMip4, TILE_PIXEL_EXTENT_MIP4, m_tileDataMip2);
	doMip(m_tileDataMip2, TILE_PIXEL_EXTENT_MIP5, m_tileDataMip1);
}


void TileData::doMip(UnsignedByte *pHiRes, Int hiRow, UnsignedByte *pLoRes)
{
	Int i, j;
	for (i=0; i<hiRow; i+=2) {
		for (j=0; j<hiRow; j+=2) {
			Int pxl;
			Int ndx = (j*hiRow+i)*TILE_BYTES_PER_PIXEL;
			Int loNdx = (j/2)*(hiRow/2) + (i/2);
			loNdx *= TILE_BYTES_PER_PIXEL;
			Int p;
			for (p=0; p<TILE_BYTES_PER_PIXEL; p++,ndx++,loNdx++) {
				pxl = pHiRes[ndx] + pHiRes[ndx+TILE_BYTES_PER_PIXEL] + pHiRes[ndx+TILE_BYTES_PER_PIXEL*hiRow] + pHiRes[ndx+TILE_BYTES_PER_PIXEL*hiRow+TILE_BYTES_PER_PIXEL] +2;
				pxl /= 4;
				pLoRes[loNdx] = pxl;
			}
		}
	}

}

