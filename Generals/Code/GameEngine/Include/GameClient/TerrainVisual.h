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

// FILE: TerrainVisual.h //////////////////////////////////////////////////////////////////////////
// Interface for visual representation of terrain on the client
// Author: Colin Day, April 2001
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __TERRAINVISUAL_H_
#define __TERRAINVISUAL_H_

#include "Common/Terrain.h"
#include "Common/Snapshot.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class TerrainType;
class WaterHandle;
class Matrix3D;
class Object;
class Drawable;

//-------------------------------------------------------------------------------------------------
/** LOD values for terrain, keep this in sync with TerrainLODNames[] */
//-------------------------------------------------------------------------------------------------
typedef enum _TerrainLOD CPP_11(: Int)
{
	TERRAIN_LOD_INVALID								= 0,
	TERRAIN_LOD_MIN										= 1,  // note that this is less than max
	TERRAIN_LOD_STRETCH_NO_CLOUDS			= 2,
	TERRAIN_LOD_HALF_CLOUDS						= 3,
	TERRAIN_LOD_NO_CLOUDS							= 4,
	TERRAIN_LOD_STRETCH_CLOUDS				= 5,
	TERRAIN_LOD_NO_WATER							= 6,
	TERRAIN_LOD_MAX										= 7,  // note that this is larger than min
	TERRAIN_LOD_AUTOMATIC							= 8,
	TERRAIN_LOD_DISABLE								= 9,

	TERRAIN_LOD_NUM_TYPES

} TerrainLOD;
#ifdef DEFINE_TERRAIN_LOD_NAMES
static const char * TerrainLODNames[] =
{
	"NONE",
	"MIN",
	"STRETCH_NO_CLOUDS",
	"HALF_CLOUDS",
	"NO_CLOUDS",
	"STRETCH_CLOUDS",
	"NO_WATER",
	"MAX",
	"AUTOMATIC",
	"DISABLE",

	NULL
};
#endif  // end DEFINE_TERRAIN_LOD_NAMES

//-------------------------------------------------------------------------------------------------
/** Device independent implementation for visual terrain */
//-------------------------------------------------------------------------------------------------
class TerrainVisual : public Snapshot,
											public SubsystemInterface
{

public:

	enum {NumSkyboxTextures = 5};

	TerrainVisual();
	virtual ~TerrainVisual();

	virtual void init( void );
	virtual void reset( void );
	virtual void update( void );

	virtual Bool load( AsciiString filename );

	/// get color of texture on the terrain at location specified
	virtual void getTerrainColorAt( Real x, Real y, RGBColor *pColor ) = 0;

	/// get the terrain tile type at the world location in the (x,y) plane ignoring Z
	virtual TerrainType *getTerrainTile( Real x, Real y ) = 0;

	/** intersect the ray with the terrain, if a hit occurs TRUE is returned
	and the result point on the terrain is returned in "result" */
	virtual Bool intersectTerrain( Coord3D *rayStart,
																 Coord3D *rayEnd,
																 Coord3D *result ) { return FALSE; }

	//
	// water methods
	//
	virtual void enableWaterGrid( Bool enable ) = 0;
	/// set min/max height values allowed in water grid pointed to by waterTable
	virtual void setWaterGridHeightClamps( const WaterHandle *waterTable, Real minZ, Real maxZ ) = 0;
	/// adjust fallof parameters for grid change method
	virtual void setWaterAttenuationFactors( const WaterHandle *waterTable, Real a, Real b, Real c, Real range ) = 0;
	/// set the water table position and orientation in world space
	virtual void setWaterTransform( const WaterHandle *waterTable, Real angle, Real x, Real y, Real z ) = 0;
	virtual void setWaterTransform( const Matrix3D *transform ) = 0;
	/// get water transform parameters
	virtual void getWaterTransform( const WaterHandle *waterTable, Matrix3D *transform ) = 0;
	/// water grid resolution spacing
	virtual void setWaterGridResolution( const WaterHandle *waterTable, Real gridCellsX, Real gridCellsY, Real cellSize ) = 0;
	virtual void getWaterGridResolution( const WaterHandle *waterTable, Real *gridCellsX, Real *gridCellsY, Real *cellSize ) = 0;
	/// adjust the water grid in world coords by the delta
	virtual void changeWaterHeight( Real x, Real y, Real delta ) = 0;
	/// adjust the velocity at a water grid point corresponding to the world x,y
	virtual void addWaterVelocity( Real worldX, Real worldY, Real velocity, Real preferredHeight ) = 0;
	/// get height of water grid at specified position
	virtual Bool getWaterGridHeight( Real worldX, Real worldY, Real *height) = 0;

	/// set detail of terrain tracks.
	virtual void setTerrainTracksDetail(void)=0;
	virtual void setShoreLineDetail(void)=0;

	/// Add a bib for an object at location.
	virtual void addFactionBib(Object *factionBuilding, Bool highlight, Real extra = 0)=0;
	/// Remove a bib.
	virtual void removeFactionBib(Object *factionBuilding)=0;

	/// Add a bib for a drawable at location.
	virtual void addFactionBibDrawable(Drawable *factionBuilding, Bool highlight, Real extra = 0)=0;
	/// Remove a bib.
	virtual void removeFactionBibDrawable(Drawable *factionBuilding)=0;

	virtual void removeAllBibs(void)=0;
	virtual void removeBibHighlighting(void)=0;


	//
	// Modify height.
	//
	virtual void setRawMapHeight(const ICoord2D *gridPos, Int height)=0;

	/// Replace the skybox texture
	virtual void replaceSkyboxTextures(const AsciiString *oldTexName[NumSkyboxTextures], const AsciiString *newTexName[NumSkyboxTextures])=0;

protected:

	// snapshot methods
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void );

	AsciiString m_filenameString;							///< file with terrain data

};

// EXTERNALS //////////////////////////////////////////////////////////////////////////////////////
extern TerrainVisual *TheTerrainVisual;  ///< singleton extern

#endif  // end __TERRAINVISUAL_H_
