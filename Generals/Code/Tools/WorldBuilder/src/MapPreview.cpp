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

// FILE: MapPreview.cpp /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//                                                                          
//                       Electronic Arts Pacific.                          
//                                                                          
//                       Confidential Information                           
//                Copyright (C) 2002 - All Rights Reserved                  
//                                                                          
//-----------------------------------------------------------------------------
//
//	created:	Oct 2002
//
//	Filename: 	MapPreview.cpp
//
//	author:		Chris Huybregts
//	
//	purpose:	Contains the code used to generate and save the map preview to 
//						the map file.  (Original code was taken from the Radar code in 
//						game engine).
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

#include "StdAfx.h" 
#include "resource.h"
#include "WHeightMapEdit.h"
#include "WorldBuilderDoc.h"
#include "MapPreview.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "Common/MapReaderWriterInfo.h"
#include "Common/FileSystem.h"
#include "WWLib/TARGA.H"
#include "Common/DataChunk.h"
//-----------------------------------------------------------------------------
// DEFINES ////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
Bool localIsUnderwater( Real x, Real y);
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
MapPreview::MapPreview(void )
{
	memset(m_pixelBuffer, 0xffffffff, sizeof(m_pixelBuffer));
}

void MapPreview::save( CString mapName )
{
	CString newStr = mapName;
	newStr.Replace(".map", ".tga");
	buildMapPreviewTexture( newStr );

/*
	chunkWriter.openDataChunk("MapPreview", K_MAPPREVIEW_VERSION_1);
	chunkWriter.writeInt(MAP_PREVIEW_WIDTH);
	chunkWriter.writeInt(MAP_PREVIEW_HEIGHT);
	DEBUG_LOG(("BeginMapPreviewInfo\n"));
	for(Int i = 0; i < MAP_PREVIEW_HEIGHT; ++i)
	{
		for(Int j = 0; j < MAP_PREVIEW_WIDTH; ++j)
		{
			chunkWriter.writeInt(m_pixelBuffer[i][j]);
			DEBUG_LOG(("x:%d, y:%d, %X\n", j, i, m_pixelBuffer[i][j]));
		}
	}
	chunkWriter.closeDataChunk();
	DEBUG_LOG(("EndMapPreviewInfo\n"));
*/
}

void MapPreview::interpolateColorForHeight( RGBColor *color,	
																					Real height, 
																					Real hiZ,
																					Real midZ,
																					Real loZ )
{
	const Real howBright = 0.30f;  // bigger is brighter (0.0 to 1.0)
	const Real howDark   = 0.60f;  // bigger is darker (0.0 to 1.0)
	
	// sanity on map height (flat maps bomb)
	if (hiZ == midZ)
		hiZ = midZ+0.1f;
	if (midZ == loZ)
		loZ = midZ-0.1f;
	if (hiZ == loZ)
		hiZ = loZ+0.2f;

//	Real heightPercent = height / (hiZ - loZ);
	Real t;
	RGBColor colorTarget;

	// if "over" the middle height, interpolate lighter
	if( height >= midZ )
	{

		// how far are we from the middleZ towards the hi Z
		t = (height - midZ) / (hiZ - midZ);

		// compute what our "lightest" color possible we want to use is
		colorTarget.red = color->red + (1.0f - color->red) * howBright;
		colorTarget.green = color->green + (1.0f - color->green) * howBright;
		colorTarget.blue = color->blue + (1.0f - color->blue) * howBright;

	}  // end if
	else  // interpolate darker
	{

		// how far are we from the middleZ towards the low Z
		t = (midZ - height) / (midZ - loZ);

		// compute what the "darkest" color possible we want to use is
		colorTarget.red = color->red + (0.0f - color->red) * howDark;
		colorTarget.green = color->green + (0.0f - color->green) * howDark;
		colorTarget.blue = color->blue + (0.0f - color->blue) * howDark;

	}  // end else

	// interpolate toward the target color
	color->red = color->red + (colorTarget.red - color->red) * t;
	color->green = color->green + (colorTarget.green - color->green) * t;
	color->blue = color->blue + (colorTarget.blue - color->blue) * t;

	// keep the color real
	if( color->red < 0.0f )
		color->red = 0.0f;
	if( color->red > 1.0f )
		color->red = 1.0f;
	if( color->green < 0.0f )
		color->green = 0.0f;
	if( color->green > 1.0f )
		color->green = 1.0f;
	if( color->blue < 0.0f )
		color->blue = 0.0f;
	if( color->blue > 1.0f )
		color->blue = 1.0f;

}  // end interpolateColorForHeight

Bool MapPreview::mapPreviewToWorld(const ICoord2D *radar, Coord3D *world)
{
	Int x, y;

	// sanity
	if( radar == NULL || world == NULL )
		return FALSE;

	// get the coords
	x = radar->x;
	y = radar->y;

	// more sanity
	if( x < 0 )
		x = 0;
	if( x >= MAP_PREVIEW_WIDTH )
		x = MAP_PREVIEW_WIDTH - 1;
	if( y < 0 )
		y = 0;
	if( y >= MAP_PREVIEW_HEIGHT )
		y = MAP_PREVIEW_HEIGHT - 1;
	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	WorldHeightMapEdit *pMap = pDoc->GetHeightMap();
	Real xSample, ySample;
	
	xSample = INT_TO_REAL(pMap->getXExtent() - (2 * pMap->getBorderSize())) / MAP_PREVIEW_WIDTH;
	ySample = INT_TO_REAL(pMap->getYExtent() - (2 * pMap->getBorderSize())) / MAP_PREVIEW_HEIGHT;
	// translate to world
	world->x = x * xSample + pMap->getBorderSize();
	world->y = y * ySample + pMap->getBorderSize();

	// find the terrain height here
//	world->z = pMap->gethigh
//	TheTerrainLogic->getGroundHeight( world->x, world->y );
		
	return TRUE;

}

void MapPreview::buildMapPreviewTexture( CString tgaName )
{
//	SurfaceClass *surface;
	RGBColor waterColor;

	// we will want to reconstruct our new view box now
	//m_reconstructViewBox = TRUE;

	// setup our water color
	waterColor.red = 0.55f;
	waterColor.green = 0.55f;
	waterColor.blue = 1.0f;

	// fill the terrain texture with a representation of the terrain
//	Real mapDepth = m_mapExtent.depth();

	// build the terrain
	RGBColor sampleColor;
	RGBColor color;
	Int i, j, samples;
	Int x, y, z;
	ICoord2D radarPoint, boundary;
	Coord3D worldPoint;
	Real getTerrainAverageZ = 0;
	Real maxHeight = 0;
	Real minHeight = 100.0f;
	Real tempHeight = 0;
	Int terrainCount = 0;
	CWorldBuilderDoc* pDoc = CWorldBuilderDoc::GetActiveDoc();
	WorldHeightMapEdit *pMap = pDoc->GetHeightMap();
	pMap->getBoundary(0, &boundary );
	for(i = 0; i < MAP_PREVIEW_HEIGHT; ++i)
	{
		for(j = 0; j < MAP_PREVIEW_WIDTH; ++j)
		{	
			radarPoint.x = j;
			radarPoint.y = i;
			mapPreviewToWorld( &radarPoint, &worldPoint );
			tempHeight = pMap->getHeight(worldPoint.x, worldPoint.y);
			getTerrainAverageZ += tempHeight;
			if(tempHeight > maxHeight)
				maxHeight = tempHeight;
			if(tempHeight < minHeight)
				minHeight = tempHeight;
			++terrainCount;
		}
	}
	getTerrainAverageZ = getTerrainAverageZ /terrainCount;
	


//	Color paddingColor = GameMakeColor( 100, 100, 100, 255 );
	for( y = 0; y < MAP_PREVIEW_HEIGHT; y++ )
	{

		for( x = 0; x < MAP_PREVIEW_WIDTH; x++ )
		{

			// what point are we inspecting
			radarPoint.x = x;
			radarPoint.y = y;
			mapPreviewToWorld( &radarPoint, &worldPoint );
			

			// get height of the terrain at this sample point
			z = pMap->getHeight(worldPoint.x, worldPoint.y);
			
			// create a color based on the Z height of the map
			//Real waterZ;
			
			if( localIsUnderwater( MAP_XY_FACTOR *(worldPoint.x - pMap->getBorderSize()), MAP_XY_FACTOR * ( worldPoint.y - pMap->getBorderSize())) )
			{
				const Int waterSamplesAway = 1;		// how many "tiles" from the center tile we will sample away
																					// to average a color for the tile color

				sampleColor.red = sampleColor.green = sampleColor.blue = 0.0f;
				samples = 0;

				for( j = y - waterSamplesAway; j <= y + waterSamplesAway; j++ )
				{

					if( j >= 0 && j < MAP_PREVIEW_HEIGHT )
					{

						for( i = x - waterSamplesAway; i <= x + waterSamplesAway; i++ )
						{

							if( i >= 0 && i < MAP_PREVIEW_WIDTH )
							{

								// the the world point we are concerned with
								radarPoint.x = i;
								radarPoint.y = j;
								mapPreviewToWorld( &radarPoint, &worldPoint );
								
								// get Z at this sample height
								Real underwaterZ = pMap->getHeight(worldPoint.x, worldPoint.y);
								
								// get color for this Z and add to our sample color
								if( localIsUnderwater( MAP_XY_FACTOR *(worldPoint.x - pMap->getBorderSize()), MAP_XY_FACTOR * ( worldPoint.y - pMap->getBorderSize())))
								{

									// this is our "color" for water
									color = waterColor;									

									// interpolate the water color for height in the water table
									interpolateColorForHeight( &color, underwaterZ, pMap->getMaxHeightValue(),getTerrainAverageZ,  pMap->getMinHeightValue() );

									// add color to our samples
									sampleColor.red += color.red;
									sampleColor.green += color.green;
									sampleColor.blue += color.blue;
									samples++;

								}  // end if

							}  // end if

						}  // end for i

					}  // end if

				}  // end for j

				// prevent divide by zeros
				if( samples == 0 )
					samples = 1;

				// set the color to an average of the colors read
				color.red = sampleColor.red / (Real)samples;
				color.green = sampleColor.green / (Real)samples;
				color.blue = sampleColor.blue / (Real)samples;

			}  // end if
			else  // regular terrain ...
			{
				const Int samplesAway = 1;  // how many "tiles" from the center tile we will sample away
																		// to average a color for the tile color

				sampleColor.red = sampleColor.green = sampleColor.blue = 0.0f;
				samples = 0;

				for( j = y - samplesAway; j <= y + samplesAway; j++ )
				{

					if( j >= 0 && j < MAP_PREVIEW_HEIGHT )
					{

						for( i = x - samplesAway; i <= x + samplesAway; i++ )
						{

							if( i >= 0 && i < MAP_PREVIEW_WIDTH )
							{

								// the the world point we are concerned with
								radarPoint.x = i;
								radarPoint.y = j;
//								radarPoint.x = x;
//								radarPoint.y = y;
								mapPreviewToWorld( &radarPoint, &worldPoint );

								// get the color at this point
								pMap->getTerrainColorAt( MAP_XY_FACTOR *(worldPoint.x - pMap->getBorderSize()) ,MAP_XY_FACTOR * ( worldPoint.y - pMap->getBorderSize()), &color );

								// interpolate the color for height
								interpolateColorForHeight( &color, z, 
																						maxHeight, getTerrainAverageZ, minHeight);
																					 //pMap->getMaxHeightValue(),getTerrainAverageZ,  pMap->getMinHeightValue() );

								// add color to our sample
								sampleColor.red += color.red;
								sampleColor.green += color.green;
								sampleColor.blue += color.blue;
								samples++;

							}  // end if

						}  // end for i

					}  // end if

				}  // end for j

				// prevent divide by zeros
				if( samples == 0 )
					samples = 1;

				// set the color to an average of the colors read
				color.red = sampleColor.red / (Real)samples;
				color.green = sampleColor.green / (Real)samples;
				color.blue = sampleColor.blue / (Real)samples;

			}  // end else

			//
			// draw the pixel for the terrain at this point, note that because of the orientation
			// of our world we draw it with positive y in the "up" direction
			//
			// FYI: I tried making this faster by pulling out all the code inside DrawPixel
			// and locking only once ... but it made absolutely *no* performance difference,
			// the sampling and interpolation algorithm for generating pretty looking terrain
			// and water for the radar is just, well, expensive.
			//
			m_pixelBuffer[y][x]= 255 | (REAL_TO_INT(color.red *255) << 8) | (REAL_TO_INT(color.green *255) << 16) | REAL_TO_INT(color.blue * 255)<< 24; 
			
		}  // end for x

	}  // end for y
	Targa tga;
	tga.Header.Width = MAP_PREVIEW_WIDTH;
	tga.Header.Height = MAP_PREVIEW_HEIGHT;
	tga.Header.PixelDepth = 32;
	tga.Header.ImageType = TGA_TRUECOLOR;
	tga.SetImage((char *)m_pixelBuffer);
	tga.Save(tgaName,TGAF_IMAGE, FALSE);

}  // end buildTerrainTexture



//-----------------------------------------------------------------------------
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

