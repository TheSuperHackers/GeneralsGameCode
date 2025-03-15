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

// FILE: TerrainVisual.h //////////////////////////////////////////////////////////////////////////
// Interface for visual representation of terrain on the client
// Author: Colin Day, April 2001
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __TERRAINVISUAL_H_
#define __TERRAINVISUAL_H_

#include "Common/Terrain.h"
#include "Common/Snapshot.h"
#include "Common/MapObject.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class TerrainType;
class WaterHandle;
class Matrix3D;
class Object;
class Drawable;
class GeometryInfo;


class WorldHeightMap;
struct SeismicSimulationNode;
class SeismicSimulationFilterBase;




#define DEFAULT_SEISMIC_SIMULATION_MAGNITUDE (20.0f)
struct SeismicSimulationNode; // just a forward declaration folks, no cause for alarm
class SeismicSimulationFilterBase
{
public:
  enum SeismicSimStatusCode
  {
    SEISMIC_STATUS_INVALID,
    SEISMIC_STATUS_ACTIVE,
    SEISMIC_STATUS_ZERO_ENERGY,
  };
  
  virtual SeismicSimStatusCode filterCallback( WorldHeightMapInterfaceClass *heightMap, const SeismicSimulationNode *node ) = 0;
  virtual Real applyGravityCallback( Real velocityIn ) = 0;
};





struct SeismicSimulationNode
{
  SeismicSimulationNode()
  {
    m_center.x    = 0;
    m_center.y    = 0;
    m_radius      = 0;
    m_region.lo.x = 0;
    m_region.lo.y = 0;
    m_region.hi.x = 0;
    m_region.hi.y = 0;
    m_clean = FALSE;
    callbackFilter = NULL;
    m_life = 0;
    m_magnitude = DEFAULT_SEISMIC_SIMULATION_MAGNITUDE;

  }
  SeismicSimulationNode( const SeismicSimulationNode &ssn )
  {
    m_center.x    = ssn.m_center.x;
    m_center.y    = ssn.m_center.y;
    m_radius      = ssn.m_radius;
    m_region.lo.x = ssn.m_region.lo.x;
    m_region.lo.y = ssn.m_region.lo.y;
    m_region.hi.x = ssn.m_region.hi.x;
    m_region.hi.y = ssn.m_region.hi.y;
    m_clean       = ssn.m_clean;
    callbackFilter= ssn.callbackFilter;
    m_life        = ssn.m_life;
    m_magnitude   = ssn.m_magnitude;

  }
  SeismicSimulationNode( const Coord3D* ctr, Real rad, Real mag, SeismicSimulationFilterBase *cbf = NULL )
  {
    m_center.x    = REAL_TO_INT_FLOOR(ctr->x/MAP_XY_FACTOR);
    m_center.y    = REAL_TO_INT_FLOOR(ctr->y/MAP_XY_FACTOR);
    m_radius      = (rad-1)/MAP_XY_FACTOR;
    UnsignedInt regionSize = rad/MAP_XY_FACTOR;
    m_region.lo.x = m_center.x - regionSize;
    m_region.lo.y = m_center.y - regionSize;
    m_region.hi.x = m_center.x + regionSize;
    m_region.hi.y = m_center.y + regionSize;
    m_clean       = false;
    callbackFilter= cbf;
    m_life        = 0;
    m_magnitude   = mag;

  }

  SeismicSimulationFilterBase::SeismicSimStatusCode handleFilterCallback( WorldHeightMapInterfaceClass *heightMap )
  {
    if ( callbackFilter == NULL )
      return SeismicSimulationFilterBase::SEISMIC_STATUS_INVALID;

    ++m_life;

    return callbackFilter->filterCallback( heightMap, this );
  }

  Real applyGravity( Real velocityIn )
  {
    DEBUG_ASSERTCRASH( callbackFilter, ("SeismicSimulationNode::applyGravity() has no callback filter!") );

    if ( callbackFilter == NULL )
      return velocityIn;//oops, we have no callback!

    return callbackFilter->applyGravityCallback( velocityIn );
    
  }

  IRegion2D   m_region;
  ICoord2D    m_center;
  Bool        m_clean;
  Real        m_magnitude;
  UnsignedInt m_radius;
  UnsignedInt m_life;
  
  SeismicSimulationFilterBase *callbackFilter;
  
};
typedef std::list<SeismicSimulationNode> SeismicSimulationList;
typedef SeismicSimulationList::iterator SeismicSimulationListIt;

class DomeStyleSeismicFilter : public SeismicSimulationFilterBase
{
  virtual SeismicSimStatusCode filterCallback( WorldHeightMapInterfaceClass *heightMap, const SeismicSimulationNode *node );
  virtual Real applyGravityCallback( Real velocityIn );
};







//-------------------------------------------------------------------------------------------------
/** LOD values for terrain, keep this in sync with TerrainLODNames[] */
//-------------------------------------------------------------------------------------------------
typedef enum _TerrainLOD
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

	TERRAIN_LOD_NUM_TYPES								// keep this last

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

	virtual void removeTreesAndPropsForConstruction(
		const Coord3D* pos, 
		const GeometryInfo& geom,
		Real angle
	) = 0;

	virtual void addProp(const ThingTemplate *tt, const Coord3D *pos, Real angle) = 0;

	//
	// Modify height.
	//
	virtual void setRawMapHeight(const ICoord2D *gridPos, Int height)=0;
	virtual Int getRawMapHeight(const ICoord2D *gridPos)=0;


  ////////////////////////////////////////////////////
  ////////////////////////////////////////////////////
  ////////////////////////////////////////////////////
#ifdef DO_SEISMIC_SIMULATIONS
  virtual void updateSeismicSimulations( void ) = 0; /// walk the SeismicSimulationList and, well, do it.
  virtual void addSeismicSimulation( const SeismicSimulationNode& sim ) = 0;
#endif
  virtual WorldHeightMap* getLogicHeightMap( void ) {return NULL;};
  virtual WorldHeightMap* getClientHeightMap( void ) {return NULL;};
  ////////////////////////////////////////////////////
  ////////////////////////////////////////////////////
  ////////////////////////////////////////////////////



	/// Replace the skybox texture
	virtual void replaceSkyboxTextures(const AsciiString *oldTexName[NumSkyboxTextures], const AsciiString *newTexName[NumSkyboxTextures])=0;

protected:

	// snapshot methods
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void );

	AsciiString m_filenameString;							///< file with terrain data

};  // end class TerrainVisual

// EXTERNALS //////////////////////////////////////////////////////////////////////////////////////
extern TerrainVisual *TheTerrainVisual;  ///< singleton extern

#endif  // end __TERRAINVISUAL_H_
