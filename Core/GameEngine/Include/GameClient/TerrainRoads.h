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

// FILE: TerrainRoads.h ///////////////////////////////////////////////////////////////////////////
// Author: Colin Day, December 2001
// Desc:   Terrain road descriptions
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/GameMemory.h"
#include "Common/SubsystemInterface.h"

#include "GameLogic/Module/BodyModule.h"


// FORWARD DECLARATIONS ///////////////////////////////////////////////////////////////////////////
struct FieldParse;
class AsciiString;

// ------------------------------------------------------------------------------------------------
/** Bridges have 4 towers around it that the player can attack or use to repair the bridge */
// ------------------------------------------------------------------------------------------------
enum BridgeTowerType CPP_11(: Int)
{
	BRIDGE_TOWER_FROM_LEFT = 0,
	BRIDGE_TOWER_FROM_RIGHT,
	BRIDGE_TOWER_TO_LEFT,
	BRIDGE_TOWER_TO_RIGHT,

	BRIDGE_MAX_TOWERS
};

// ------------------------------------------------------------------------------------------------
enum { MAX_BRIDGE_BODY_FX = 3 };

//-------------------------------------------------------------------------------------------------
/** Terrain road description, good for roads and bridges */
//-------------------------------------------------------------------------------------------------
class TerrainRoadType : public MemoryPoolObject
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( TerrainRoadType, "TerrainRoadType" )

public:

	TerrainRoadType( void );
	// destructor prototypes defined by memory pool object

	AsciiString getName( void ) { return m_name; }
	AsciiString getTexture( void ) { return m_texture; }
	Bool isBridge( void ) { return m_isBridge; }
	UnsignedInt getID( void ) { return m_id; }

	Real getRoadWidth( void ) { return m_roadWidth; }
	Real getRoadWidthInTexture( void ) { return m_roadWidthInTexture; }

	Real getBridgeScale( void ) { return m_bridgeScale; }
	AsciiString getScaffoldObjectName( void ) { return m_scaffoldObjectName; }
	AsciiString getScaffoldSupportObjectName( void ) { return m_scaffoldSupportObjectName; }
	RGBColor getRadarColor( void ) { return m_radarColor; }
	AsciiString getBridgeModel( void ) { return m_bridgeModelName; }
	AsciiString getBridgeModelNameDamaged( void ) { return m_bridgeModelNameDamaged; }
	AsciiString getBridgeModelNameReallyDamaged( void ) { return m_bridgeModelNameReallyDamaged; }
	AsciiString getBridgeModelNameBroken( void ) { return m_bridgeModelNameBroken; }
	AsciiString getTextureDamaged( void ) { return m_textureDamaged; }
	AsciiString getTextureReallyDamaged( void ) { return m_textureReallyDamaged; }
	AsciiString getTextureBroken( void ) { return m_textureBroken; }
	AsciiString getTowerObjectName( BridgeTowerType tower ) { return m_towerObjectName[ tower ]; }
	AsciiString getDamageToSoundString( BodyDamageType state ) { return m_damageToSoundString[ state ]; }
	AsciiString getDamageToOCLString( BodyDamageType state, Int index ) { return m_damageToOCLString[ state ][ index ]; }
	AsciiString getDamageToFXString( BodyDamageType state, Int index ) { return m_damageToFXString[ state ][ index ]; }
	AsciiString getRepairedToSoundString( BodyDamageType state ) { return m_repairedToSoundString[ state ]; }
	AsciiString getRepairedToOCLString( BodyDamageType state, Int index ) { return m_repairedToOCLString[ state ][ index ]; }
	AsciiString getRepairedToFXString( BodyDamageType state, Int index ) { return m_repairedToFXString[ state ][ index ]; }
	Real getTransitionEffectsHeight( void ) { return m_transitionEffectsHeight; }
	Int getNumFXPerType( void ) { return m_numFXPerType; }

	// friend access methods to be used by the road collection only!
	void friend_setName( AsciiString name ) { m_name = name; }
	void friend_setTexture( AsciiString texture ) { m_texture = texture; }
	void friend_setBridge( Bool isBridge ) { m_isBridge = isBridge; }
	void friend_setID( UnsignedInt id ) { m_id = id; }
	void friend_setNext( TerrainRoadType *next ) { m_next = next; }
	TerrainRoadType *friend_getNext( void ) { return m_next; }
	void friend_setRoadWidth( Real width ) { m_roadWidth = width; }
	void friend_setRoadWidthInTexture( Real width ) { m_roadWidthInTexture = width; }
	void friend_setBridgeScale( Real scale ) { m_bridgeScale = scale; }
	void friend_setScaffoldObjectName( AsciiString name ) { m_scaffoldObjectName = name; }
	void friend_setScaffoldSupportObjectName( AsciiString name ) { m_scaffoldSupportObjectName = name; }
	void friend_setBridgeModelName( AsciiString name ) { m_bridgeModelName = name; }
	void friend_setBridgeModelNameDamaged( AsciiString name ) { m_bridgeModelNameDamaged = name; }
	void friend_setBridgeModelNameReallyDamaged( AsciiString name ) { m_bridgeModelNameReallyDamaged = name; }
	void friend_setBridgeModelNameBroken( AsciiString name ) { m_bridgeModelNameBroken = name; }
	void friend_setTextureDamaged( AsciiString texture ) { m_textureDamaged = texture; }
	void friend_setTextureReallyDamaged( AsciiString texture ) { m_textureReallyDamaged = texture; }
	void friend_setTextureBroken( AsciiString texture ) { m_textureBroken = texture; }
	void friend_setTowerObjectName( BridgeTowerType tower, AsciiString name ) { m_towerObjectName[ tower ] = name; }
	void friend_setDamageToSoundString( BodyDamageType state, AsciiString s ) { m_damageToSoundString[ state ] = s; }
	void friend_setDamageToOCLString( BodyDamageType state, Int index, AsciiString s ) { m_damageToOCLString[ state ][ index ] = s; }
	void friend_setDamageToFXString( BodyDamageType state, Int index, AsciiString s ) { m_damageToFXString[ state ][ index ] = s; }
	void friend_setRepairedToSoundString( BodyDamageType state, AsciiString s ) { m_repairedToSoundString[ state ] = s; }
	void friend_setRepairedToOCLString( BodyDamageType state, Int index, AsciiString s ) { m_repairedToOCLString[ state ][ index ] = s; }
	void friend_setRepairedToFXString( BodyDamageType state, Int index, AsciiString s ) { m_repairedToFXString[ state ][ index ] = s; }
	void friend_setTransitionEffectsHeight( Real height ) { m_transitionEffectsHeight = height; }
	void friend_setNumFXPerType( Int num ) { m_numFXPerType = num; }

	/// get the parsing table for INI
	const FieldParse *getRoadFieldParse( void ) { return m_terrainRoadFieldParseTable; }
	const FieldParse *getBridgeFieldParse( void ) { return m_terrainBridgeFieldParseTable; }

protected:

	AsciiString m_name;								///< entry name
	Bool m_isBridge;									///< true if entry is for a bridge
	UnsignedInt m_id;									///< unique id
	TerrainRoadType *m_next;					///< next in road list

	// for parsing from INI
	static const FieldParse m_terrainRoadFieldParseTable[];		///< the parse table for INI definition
	static const FieldParse m_terrainBridgeFieldParseTable[];		///< the parse table for INI definition
	static void parseTransitionToOCL( INI *ini, void *instance, void *store, const void *userData );
	static void parseTransitionToFX( INI *ini, void *instance, void *store, const void *userData );

	//
	// *note* I would union the road and bridge data, but unions can't have a copy
	// constructor such as the AsciiString does
	//

	// road data
	Real m_roadWidth;														///< width of road
	Real m_roadWidthInTexture;									///< width of road in the texture

	// bridge data
	Real m_bridgeScale;													///< scale for bridge

	AsciiString m_scaffoldObjectName;						///< scaffold object name
	AsciiString m_scaffoldSupportObjectName;		///< scaffold support object name

	RGBColor m_radarColor;											///< color for this bridge on the radar

	AsciiString m_bridgeModelName;							///< model name for bridge
	AsciiString m_texture;											///< texture filename

	AsciiString m_bridgeModelNameDamaged;				///< model name for bridge
	AsciiString m_textureDamaged;								///< model name for bridge

	AsciiString m_bridgeModelNameReallyDamaged;	///< model name for bridge
	AsciiString m_textureReallyDamaged;					///< model name for bridge

	AsciiString m_bridgeModelNameBroken;				///< model name for bridge
	AsciiString m_textureBroken;								///< model name for bridge

	AsciiString m_towerObjectName[ BRIDGE_MAX_TOWERS ];	///< object names for the targetable towers on the bridge

	//
	// the following strings are for repair/damage transition events, what sounds to
	// play and a collection of OCL and FX lists to play over the bridge area
	//
	AsciiString m_damageToSoundString[ BODYDAMAGETYPE_COUNT ];
	AsciiString m_damageToOCLString[ BODYDAMAGETYPE_COUNT ][ MAX_BRIDGE_BODY_FX ];
	AsciiString m_damageToFXString[ BODYDAMAGETYPE_COUNT ][ MAX_BRIDGE_BODY_FX ];
	AsciiString m_repairedToSoundString[ BODYDAMAGETYPE_COUNT ];
	AsciiString m_repairedToOCLString[ BODYDAMAGETYPE_COUNT ][ MAX_BRIDGE_BODY_FX ];
	AsciiString m_repairedToFXString[ BODYDAMAGETYPE_COUNT ][ MAX_BRIDGE_BODY_FX ];
	Real m_transitionEffectsHeight;
	Int m_numFXPerType; ///< for *each* fx/ocl we will make this many of them on the bridge area

};

//-------------------------------------------------------------------------------------------------
/** Collection of all roads and bridges */
//-------------------------------------------------------------------------------------------------
class TerrainRoadCollection : public SubsystemInterface
{

public:

	TerrainRoadCollection( void );
	~TerrainRoadCollection( void );

	void init() { }
	void reset() { }
	void update() { }

	TerrainRoadType *findRoad( AsciiString name );		///< find road with matching name
	TerrainRoadType *newRoad( AsciiString name );			///< allocate new road, assign name, and link to list
	TerrainRoadType *firstRoad( void ) { return m_roadList; }			///< return first road
	TerrainRoadType *nextRoad( TerrainRoadType *road );						///< get next road

	TerrainRoadType *findBridge( AsciiString name );	///< find bridge with matching name
	TerrainRoadType *newBridge( AsciiString name );		///< allocate new bridge, assign name, and link
	TerrainRoadType *firstBridge( void ) { return m_bridgeList; } ///< return first bridge
	TerrainRoadType *nextBridge( TerrainRoadType *bridge );				///< get next bridge

	TerrainRoadType *findRoadOrBridge( AsciiString name );				///< search roads and bridges

protected:

	TerrainRoadType *m_roadList;				///< list of available roads
	TerrainRoadType *m_bridgeList;			///< list of available bridges
	static UnsignedInt m_idCounter;			///< unique id counter when allocating roads/bridges

};

// EXTERNAL ////////////////////////////////////////////////////////////////////////////////////////
extern TerrainRoadCollection *TheTerrainRoads;
