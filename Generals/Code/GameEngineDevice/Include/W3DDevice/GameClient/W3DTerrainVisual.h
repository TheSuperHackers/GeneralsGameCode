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

// FILE: W3DTerrainVisual.h ///////////////////////////////////////////////////////////////////////
// W3D implementation details for visual aspects of terrain
// Author: Colin Day, April 2001
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameClient/TerrainVisual.h"
#include "W3DDevice/GameClient/W3DWater.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Matrix3D;
class WaterHandle;
class HeightMapRenderObjClass;
class WorldHeightMap;

//-------------------------------------------------------------------------------------------------
/** W3D impelmentation of visual terrain details singleton */
//-------------------------------------------------------------------------------------------------
class W3DTerrainVisual : public TerrainVisual
{

public:

	W3DTerrainVisual();
	virtual ~W3DTerrainVisual();

	virtual void init( void );
	virtual void reset( void );
	virtual void update( void );

	virtual Bool load( AsciiString filename );

	void getTerrainColorAt( Real x, Real y, RGBColor *pColor );

	/// get the terrain tile type at the world location in the (x,y) plane ignoring Z
	TerrainType *getTerrainTile( Real x, Real y );

	/** intersect the ray with the terrain, if a hit occurs TRUE is returned
	and the result point on the terrain is returned in "result" */
	virtual Bool intersectTerrain( Coord3D *rayStart, Coord3D *rayEnd, Coord3D *result );

	//
	// water methods
	//
	/// enable/disable the water grid
	virtual void enableWaterGrid( Bool enable );
	/// set min/max height values allowed in water grid pointed to by waterTable
	virtual void setWaterGridHeightClamps( const WaterHandle *waterTable, Real minZ, Real maxZ );
	/// adjust fallof parameters for grid change method
	virtual void setWaterAttenuationFactors( const WaterHandle *waterTable,
																					 Real a, Real b, Real c, Real range );
	/// set the water table position and orientation in world space
	virtual void setWaterTransform( const WaterHandle *waterTable,
																	Real angle, Real x, Real y, Real z );
	virtual void setWaterTransform( const Matrix3D *transform );
	virtual void getWaterTransform( const WaterHandle *waterTable, Matrix3D *transform );
	/// water grid resolution spacing
	virtual void setWaterGridResolution( const WaterHandle *waterTable,
																			 Real gridCellsX, Real gridCellsY, Real cellSize );
	virtual void getWaterGridResolution( const WaterHandle *waterTable,
																			 Real *gridCellsX, Real *gridCellsY, Real *cellSize );
	/// adjust the water grid in world coords by the delta
	virtual void changeWaterHeight( Real x, Real y, Real delta );
	/// adjust the velocity at a water grid point corresponding to the world x,y
	virtual void addWaterVelocity( Real worldX, Real worldY,
																 Real velocity, Real preferredHeight );
	virtual Bool getWaterGridHeight( Real worldX, Real worldY, Real *height);

	virtual void setTerrainTracksDetail(void);
	virtual void setShoreLineDetail(void);

	/// Add a bib at location.
	void addFactionBib(Object *factionBuilding, Bool highlight, Real extra = 0);
	/// Remove a bib.
	void removeFactionBib(Object *factionBuilding);

	/// Add a bib at location.
	void addFactionBibDrawable(Drawable *factionBuilding, Bool highlight, Real extra = 0);
	/// Remove a bib.
	void removeFactionBibDrawable(Drawable *factionBuilding);

	virtual void removeAllBibs(void);
	virtual void removeBibHighlighting(void);

	//
	// Modify height.
	//
	virtual void setRawMapHeight(const ICoord2D *gridPos, Int height);

	/// Replace the skybox texture
	virtual void replaceSkyboxTextures(const AsciiString *oldTexName[NumSkyboxTextures], const AsciiString *newTexName[NumSkyboxTextures]);


protected:

	// snapshot methods
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void );

	HeightMapRenderObjClass *m_terrainRenderObject;  ///< W3D render object for terrain
	WaterRenderObjClass	*m_waterRenderObject;	///< W3D render object for water plane
	WorldHeightMap *m_terrainHeightMap;  ///< height map used for render obj building
	Bool m_isWaterGridRenderingEnabled;
	AsciiString	m_currentSkyboxTexNames[NumSkyboxTextures];	///<store current texture names applied to skybox.
	AsciiString m_initialSkyboxTexNames[NumSkyboxTextures];	///<store starting texture/default skybox textures.

};
