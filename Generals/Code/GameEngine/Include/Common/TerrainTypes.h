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

// FILE: TerrainTypes.h ///////////////////////////////////////////////////////////////////////////
// Author: Colin Day, December 2001
// Desc:   Terrain type descriptions
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/GameMemory.h"
#include "Common/SubsystemInterface.h"

// FORWARD DECLARATIONS ///////////////////////////////////////////////////////////////////////////
struct FieldParse;

//-------------------------------------------------------------------------------------------------
/** Type classification, keep this in sync with terrainTypeNames[] */
//-------------------------------------------------------------------------------------------------
typedef enum
{
	TERRAIN_NONE = 0,
	TERRAIN_DESERT_1,
	TERRAIN_DESERT_2,
	TERRAIN_DESERT_3,
	TERRAIN_EASTERN_EUROPE_1,
	TERRAIN_EASTERN_EUROPE_2,
	TERRAIN_EASTERN_EUROPE_3,
	TERRAIN_SWISS_1,
	TERRAIN_SWISS_2,
	TERRAIN_SWISS_3,
	TERRAIN_SNOW_1,
	TERRAIN_SNOW_2,
	TERRAIN_SNOW_3,

	// remove all the terrain types below when Todd says he's redone them all
	TERRAIN_DIRT,
	TERRAIN_GRASS,
	TERRAIN_TRANSITION,
	TERRAIN_ROCK,
	TERRAIN_SAND,
	TERRAIN_CLIFF,
	TERRAIN_WOOD,
	TERRAIN_BLEND_EDGES,

	// New terrain types (for Samm Ivri)
	TERRAIN_LIVE_DESERT,
	TERRAIN_DRY_DESERT,
	TERRAIN_ACCENT_SAND,
	TERRAIN_TROPICAL_BEACH,
	TERRAIN_BEACH_PARK,
	TERRAIN_RUGGED_MOUNTAIN,
	TERRAIN_COBBLESTONE_GRASS,
	TERRAIN_ACCENT_GRASS,
	TERRAIN_RESIDENTIAL,
	TERRAIN_RUGGED_SNOW,
	TERRAIN_FLAT_SNOW,
	TERRAIN_FIELD,
	TERRAIN_ASPHALT,
	TERRAIN_CONCRETE,
	TERRAIN_CHINA,
	TERRAIN_ACCENT_ROCK,
	TERRAIN_URBAN,

	TERRAIN_NUM_CLASSES

} TerrainClass;
#ifdef DEFINE_TERRAIN_TYPE_NAMES
static const char *const terrainTypeNames[] =
{
	"NONE",
	"DESERT_1",
	"DESERT_2",
	"DESERT_3",
	"EASTERN_EUROPE_1",
	"EASTERN_EUROPE_2",
	"EASTERN_EUROPE_3",
	"SWISS_1",
	"SWISS_2",
	"SWISS_3",
	"SNOW_1",
	"SNOW_2",
	"SNOW_3",

	// remove all the terrain types below when Todd says he's redone them all
	"DIRT",
	"GRASS",
	"TRANSITION",
	"ROCK",
	"SAND",
	"CLIFF",
	"WOOD",
	"BLEND_EDGE",

		// New terrain types (for Samm Ivri)
	"DESERT_LIVE",
	"DESERT_DRY",
	"SAND_ACCENT",
	"BEACH_TROPICAL",
	"BEACH_PARK",
	"MOUNTAIN_RUGGED",
	"GRASS_COBBLESTONE",
	"GRASS_ACCENT",
	"RESIDENTIAL",
	"SNOW_RUGGED",
	"SNOW_FLAT",
	"FIELD",
	"ASPHALT",
	"CONCRETE",
	"CHINA",
	"ROCK_ACCENT",
	"URBAN",

	nullptr
};
static_assert(ARRAY_SIZE(terrainTypeNames) == TERRAIN_NUM_CLASSES + 1, "Incorrect array size");
#endif  // end DEFINE_TERRAIN_TYPE_NAMES

//-------------------------------------------------------------------------------------------------
/** A terrain type definition */
//-------------------------------------------------------------------------------------------------
class TerrainType : public MemoryPoolObject
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( TerrainType, "TerrainType" )

public:

	TerrainType( void );
	// destructor prototype defined by memory pool glue

	/// get the name for this terrain
	AsciiString getName( void ) { return m_name; }

	/// get whether this terrain is blend edge terrain.
	Bool isBlendEdge( void ) { return m_blendEdgeTexture; }

	/// get the type of this terrain
	TerrainClass getClass( void ) { return m_class; }

	/// get the construction restrictions
	Bool getRestrictConstruction( void ) { return m_restrictConstruction; }

	/// get the texture file for this terrain
	AsciiString getTexture( void ) { return m_texture; }

	/// get next terrain in list, only for use by the terrain collection
	TerrainType *friend_getNext( void ) { return m_next; }

	/// set the name for this terrain, for use by terrain collection only
	void friend_setName( AsciiString name ) { m_name = name; }

	/// set the next pointer for the terrain list, for use by terrain collection only
	void friend_setNext( TerrainType *next ) { m_next = next; }

	/// set the texture, for use by terrain collection only
	void friend_setTexture( AsciiString texture ) { m_texture = texture; }

	/// set the class, for use by terrain collection only
	void friend_setClass( TerrainClass terrainClass ) { m_class = terrainClass; }

	/// set the restrict construction flag, for use by terrain collection only
	void friend_setRestrictConstruction( Bool restrict ) { m_restrictConstruction = restrict; }

	/// set whether this terrain is blend edge terrain, for use by terrain collection only
	void friend_setBlendEdge( Bool isBlend ) { m_blendEdgeTexture = isBlend; }

	/// get the parsing table for INI
	const FieldParse *getFieldParse( void ) { return m_terrainTypeFieldParseTable; }

protected:

	AsciiString m_name;								///< terrain entry name
	AsciiString m_texture;						///< texture.tga file for terrain
	Bool m_blendEdgeTexture;					///< contains custom blend edges
	TerrainClass m_class;							///< type classification of name
	Bool m_restrictConstruction;			///< do not allow construction on this terrain tile
	TerrainType *m_next;							///< next in terrain list

	// for parsing from INI
	static const FieldParse m_terrainTypeFieldParseTable[];		///< the parse table for INI definition

};

//-------------------------------------------------------------------------------------------------
/** The collection of all terrain types */
//-------------------------------------------------------------------------------------------------
class TerrainTypeCollection : public SubsystemInterface
{

public:

	TerrainTypeCollection( void );
	~TerrainTypeCollection( void );

	void init() { }
	void reset() { }
	void update() { }

	TerrainType *findTerrain( AsciiString name );		///< find terrain by name
	TerrainType *newTerrain( AsciiString name );			///< allocate a new terrain

	/// get first terrain in list
	TerrainType *firstTerrain( void ) { return m_terrainList; }

	/// get next terrain in list
	TerrainType *nextTerrain( TerrainType *terrainType ) { return terrainType->friend_getNext(); }

protected:

	TerrainType *m_terrainList;						///< list of available terrain types

};

// EXTERNAL ///////////////////////////////////////////////////////////////////////////////////////
extern TerrainTypeCollection *TheTerrainTypes;
