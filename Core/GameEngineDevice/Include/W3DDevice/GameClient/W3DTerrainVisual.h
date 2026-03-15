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
class BaseHeightMapRenderObjClass;
class WorldHeightMap;

//-------------------------------------------------------------------------------------------------
/** W3D implementation of visual terrain details singleton */
//-------------------------------------------------------------------------------------------------
class W3DTerrainVisual : public TerrainVisual
{

public:

	W3DTerrainVisual();
	virtual ~W3DTerrainVisual() override;

	virtual void init() override;
	virtual void reset() override;
	virtual void update() override;

	virtual Bool load( AsciiString filename ) override;

	virtual void getTerrainColorAt( Real x, Real y, RGBColor *pColor ) override;

	/// get the terrain tile type at the world location in the (x,y) plane ignoring Z
	virtual TerrainType *getTerrainTile( Real x, Real y ) override;

	/** intersect the ray with the terrain, if a hit occurs TRUE is returned
	and the result point on the terrain is returned in "result" */
	virtual Bool intersectTerrain( Coord3D *rayStart, Coord3D *rayEnd, Coord3D *result ) override;

	//
	// water methods
	//
	/// enable/disable the water grid
	virtual void enableWaterGrid( Bool enable ) override;
	/// set min/max height values allowed in water grid pointed to by waterTable
	virtual void setWaterGridHeightClamps( const WaterHandle *waterTable, Real minZ, Real maxZ ) override;
	/// adjust fallof parameters for grid change method
	virtual void setWaterAttenuationFactors( const WaterHandle *waterTable,
																					 Real a, Real b, Real c, Real range ) override;
	/// set the water table position and orientation in world space
	virtual void setWaterTransform( const WaterHandle *waterTable,
																	Real angle, Real x, Real y, Real z ) override;
	virtual void setWaterTransform( const Matrix3D *transform ) override;
	virtual void getWaterTransform( const WaterHandle *waterTable, Matrix3D *transform ) override;
	/// water grid resolution spacing
	virtual void setWaterGridResolution( const WaterHandle *waterTable,
																			 Real gridCellsX, Real gridCellsY, Real cellSize ) override;
	virtual void getWaterGridResolution( const WaterHandle *waterTable,
																			 Real *gridCellsX, Real *gridCellsY, Real *cellSize ) override;
	/// adjust the water grid in world coords by the delta
	virtual void changeWaterHeight( Real x, Real y, Real delta ) override;
	/// adjust the velocity at a water grid point corresponding to the world x,y
	virtual void addWaterVelocity( Real worldX, Real worldY,
																 Real velocity, Real preferredHeight ) override;
	virtual Bool getWaterGridHeight( Real worldX, Real worldY, Real *height) override;

	virtual void setTerrainTracksDetail() override;
	virtual void setShoreLineDetail() override;

	/// Add a bib at location.
	virtual void addFactionBib(Object *factionBuilding, Bool highlight, Real extra = 0) override;
	/// Remove a bib.
	virtual void removeFactionBib(Object *factionBuilding) override;

	/// Add a bib at location.
	virtual void addFactionBibDrawable(Drawable *factionBuilding, Bool highlight, Real extra = 0) override;
	/// Remove a bib.
	virtual void removeFactionBibDrawable(Drawable *factionBuilding) override;

	virtual void removeAllBibs() override;
	virtual void removeBibHighlighting() override;

	virtual void addProp(const ThingTemplate *tt, const Coord3D *pos, Real angle) override;

	virtual void removeTreesAndPropsForConstruction(
		const Coord3D* pos,
		const GeometryInfo& geom,
		Real angle
	) override;


	//
	// Modify height.
	//
	virtual void setRawMapHeight(const ICoord2D *gridPos, Int height) override;
	virtual Int getRawMapHeight(const ICoord2D *gridPos) override;

	/// Replace the skybox texture
	virtual void replaceSkyboxTextures(const AsciiString *oldTexName[NumSkyboxTextures], const AsciiString *newTexName[NumSkyboxTextures]) override;

  ////////////////////////////////////////////////////
  ////////////////////////////////////////////////////
  ////////////////////////////////////////////////////
#ifdef DO_SEISMIC_SIMULATIONS
  virtual void addSeismicSimulation( const SeismicSimulationNode& sim );
#endif
  virtual WorldHeightMap* getLogicHeightMap() override {return m_logicHeightMap;};
  virtual WorldHeightMap* getClientHeightMap() override
  {
#ifdef DO_SEISMIC_SIMULATIONS
    return m_clientHeightMap;
#else
    return m_logicHeightMap;
#endif
  }
  ////////////////////////////////////////////////////
  ////////////////////////////////////////////////////
  ////////////////////////////////////////////////////

protected:

	// snapshot methods
	virtual void crc( Xfer *xfer ) override;
	virtual void xfer( Xfer *xfer ) override;
	virtual void loadPostProcess() override;

#ifdef DO_SEISMIC_SIMULATIONS
  ////////////////////////////////////////////////////
  ////////////////////////////////////////////////////
  ////////////////////////////////////////////////////
  virtual void handleSeismicSimulations();
  SeismicSimulationList m_seismicSimulationList;
  virtual void updateSeismicSimulations(); /// walk the SeismicSimulationList and, well, do it.

  ////////////////////////////////////////////////////
  ////////////////////////////////////////////////////
  ////////////////////////////////////////////////////
#endif

	BaseHeightMapRenderObjClass *m_terrainRenderObject;  ///< W3D render object for terrain
	WaterRenderObjClass	*m_waterRenderObject;	///< W3D render object for water plane

  WorldHeightMap *m_logicHeightMap;  ///< height map used for render obj building

#ifdef DO_SEISMIC_SIMULATIONS
  WorldHeightMap *m_clientHeightMap; ///< this is a workspace for animating the terrain elevations
#endif

	Bool m_isWaterGridRenderingEnabled;

  AsciiString	m_currentSkyboxTexNames[NumSkyboxTextures];	///<store current texture names applied to skybox.
	AsciiString m_initialSkyboxTexNames[NumSkyboxTextures];	///<store starting texture/default skybox textures.

};
