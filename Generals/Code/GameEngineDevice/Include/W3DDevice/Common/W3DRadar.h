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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: W3DRadar.h ///////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, January 2002
// Desc:   W3D radar implementation, this has the necessary device dependent drawing
//				 necessary for the radar

///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __W3DRADAR_H_
#define __W3DRADAR_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/Radar.h"
#include "WW3D2/ww3dformat.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class TextureClass;
class TerrainLogic;

// PROTOTYPES /////////////////////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------------------------------------
/** W3D radar class.  This has the device specific implementation of the radar such as
	* the drawing routines */
//-------------------------------------------------------------------------------------------------
class W3DRadar : public Radar
{

public:

	W3DRadar( void );
	~W3DRadar( void );

	virtual void xfer( Xfer *xfer );

	virtual void init( void );																		///< subsystem init
	virtual void update( void );																	///< subsystem update
	virtual void reset( void );																		///< subsystem reset

	virtual RadarObjectType addObject( Object *obj ); ///< add object to radar
	virtual RadarObjectType removeObject( Object *obj ); ///< remove object from radar

	virtual void newMap( TerrainLogic *terrain );				///< reset radar for new map

	virtual void draw( Int pixelX, Int pixelY, Int width, Int height );		///< draw the radar

	virtual void clearShroud();
	virtual void setShroudLevel(Int x, Int y, CellShroudStatus setting);

	virtual void refreshTerrain( TerrainLogic *terrain );

protected:

	void drawSingleBeaconEvent( Int pixelX, Int pixelY, Int width, Int height, Int index );
	void drawSingleGenericEvent( Int pixelX, Int pixelY, Int width, Int height, Int index );

	void initializeTextureFormats( void );				///< find format to use for the radar texture
	void deleteResources( void );									///< delete resources used
	void drawEvents( Int pixelX, Int pixelY, Int width, Int height);		///< draw all of the radar events
	void drawHeroIcon( Int pixelX, Int pixelY, Int width, Int height, const Coord3D *pos );	//< draw a hero icon
	void drawViewBox( Int pixelX, Int pixelY, Int width, Int height );  ///< draw view box
	void buildTerrainTexture( TerrainLogic *terrain );	 ///< create the terrain texture of the radar
	void drawIcons( Int pixelX, Int pixelY, Int width, Int height );	///< draw all of the radar icons
	void renderObjectList( const RadarObject *listHead, TextureClass *texture, Bool calcHero = FALSE );			 ///< render an object list to the texture
	void interpolateColorForHeight( RGBColor *color,
																	Real height,
																	Real hiZ,
																	Real midZ,
																	Real loZ );		///< "shade" color according to height value
	void reconstructViewBox( void );							///< remake the view box
	void radarToPixel( const ICoord2D *radar, ICoord2D *pixel,
										 Int radarUpperLeftX, Int radarUpperLeftY,
										 Int radarWidth, Int radarHeight );  ///< convert radar coord to pixel location

	WW3DFormat m_terrainTextureFormat;						///< format to use for terrain texture
	Image *m_terrainImage;												///< terrain image abstraction for drawing
	TextureClass *m_terrainTexture;								///< terrain background texture

	WW3DFormat m_overlayTextureFormat;						///< format to use for overlay texture
	Image *m_overlayImage;												///< overlay image abstraction for drawing
	TextureClass *m_overlayTexture;								///< overlay texture

	WW3DFormat m_shroudTextureFormat;							///< format to use for shroud texture
	Image *m_shroudImage;													///< shroud image abstraction for drawing
	TextureClass *m_shroudTexture;								///< shroud texture

	Int m_textureWidth;														///< width for all radar textures
	Int m_textureHeight;													///< height for all radar textures

	//
	// we want to keep a flag that tells us when to reconstruct the view box, we want
	// to reconstruct the box on map change, and when the camera changes height
	// or orientation.  We want to avoid making the view box every frame because
	// the 4 points visible on the edge of the screen will "jitter" unevenly as we
	// translate real world coords to integer radar spots
	//
	Bool m_reconstructViewBox;										///< true when we need to reconstruct the box
	Real m_viewAngle;															///< camera angle used for the view box we have
	Real m_viewZoom;															///< camera zoom used for the view box we have
	ICoord2D m_viewBox[ 4 ];											///< radar cell points for the 4 corners of view box

	std::vector<const Object *> m_cachedHeroObjectList; //< cache of hero objects for drawing icons in radar overlay
};


#endif // __W3DRADAR_H_
