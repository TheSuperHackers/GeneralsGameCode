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

// FILE: TerrainRoads.cpp /////////////////////////////////////////////////////////////////////////
// Author: Colin Day, December 2001
// Desc:   Terrain road/bridge descriptions
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLDUES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#define DEFINE_BODYDAMAGETYPE_NAMES
#include "Common/INI.h"
#include "GameClient/TerrainRoads.h"

// PUBLIC DATA ////////////////////////////////////////////////////////////////////////////////////
TerrainRoadCollection *TheTerrainRoads = NULL;

// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////////
UnsignedInt TerrainRoadCollection::m_idCounter = 0;

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
const FieldParse TerrainRoadType::m_terrainRoadFieldParseTable[] = 
{

	{ "Texture",						INI::parseAsciiString,				NULL, offsetof( TerrainRoadType, m_texture ) },
	{ "RoadWidth",					INI::parseReal,								NULL, offsetof( TerrainRoadType, m_roadWidth ) },
	{ "RoadWidthInTexture",	INI::parseReal,								NULL, offsetof( TerrainRoadType, m_roadWidthInTexture ) },

	{ NULL,									NULL,													NULL, 0 },

};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
const FieldParse TerrainRoadType::m_terrainBridgeFieldParseTable[] = 
{

	{ "BridgeScale",									INI::parseReal,								NULL, offsetof( TerrainRoadType, m_bridgeScale ) },
	{ "ScaffoldObjectName",						INI::parseAsciiString,				NULL, offsetof( TerrainRoadType, m_scaffoldObjectName ) },
	{ "ScaffoldSupportObjectName",		INI::parseAsciiString,				NULL, offsetof( TerrainRoadType, m_scaffoldSupportObjectName ) },
	{ "RadarColor",										INI::parseRGBColor,						NULL, offsetof( TerrainRoadType, m_radarColor ) },

	{ "TransitionEffectsHeight",			INI::parseReal,								NULL, offsetof( TerrainRoadType, m_transitionEffectsHeight ) },
	{ "NumFXPerType",									INI::parseInt,								NULL,	offsetof( TerrainRoadType, m_numFXPerType ) },

	{ "BridgeModelName",							INI::parseAsciiString,				NULL, offsetof( TerrainRoadType, m_bridgeModelName ) },
	{ "Texture",											INI::parseAsciiString,				NULL, offsetof( TerrainRoadType, m_texture ) },
	{ "BridgeModelNameDamaged",				INI::parseAsciiString,				NULL, offsetof( TerrainRoadType, m_bridgeModelNameDamaged ) },
	{ "TextureDamaged",								INI::parseAsciiString,				NULL, offsetof( TerrainRoadType, m_textureDamaged ) },
	{ "BridgeModelNameReallyDamaged",	INI::parseAsciiString,				NULL, offsetof( TerrainRoadType, m_bridgeModelNameReallyDamaged ) },
	{ "TextureReallyDamaged",					INI::parseAsciiString,				NULL, offsetof( TerrainRoadType, m_textureReallyDamaged ) },
	{ "BridgeModelNameBroken",				INI::parseAsciiString,				NULL, offsetof( TerrainRoadType, m_bridgeModelNameBroken ) },
	{ "TextureBroken",								INI::parseAsciiString,				NULL, offsetof( TerrainRoadType, m_textureBroken ) },

	{ "TowerObjectNameFromLeft",			INI::parseAsciiString,				NULL,	offsetof( TerrainRoadType, m_towerObjectName[ BRIDGE_TOWER_FROM_LEFT ] ) },
	{ "TowerObjectNameFromRight",			INI::parseAsciiString,				NULL,	offsetof( TerrainRoadType, m_towerObjectName[ BRIDGE_TOWER_FROM_RIGHT ] ) },
	{ "TowerObjectNameToLeft",				INI::parseAsciiString,				NULL,	offsetof( TerrainRoadType, m_towerObjectName[ BRIDGE_TOWER_TO_LEFT ] ) },
	{ "TowerObjectNameToRight",				INI::parseAsciiString,				NULL,	offsetof( TerrainRoadType, m_towerObjectName[ BRIDGE_TOWER_TO_RIGHT ] ) },

	{ "DamagedToSound",								INI::parseAsciiString,				NULL, offsetof( TerrainRoadType, m_damageToSoundString[ BODY_DAMAGED ] ) },
	{ "RepairedToSound",							INI::parseAsciiString,				NULL, offsetof( TerrainRoadType, m_repairedToSoundString[ BODY_DAMAGED ] ) },
	{ "TransitionToOCL",							parseTransitionToOCL,					NULL,	NULL },
	{ "TransitionToFX",								parseTransitionToFX,					NULL, NULL },


	{ NULL,									NULL,													NULL, 0 },

};

// ------------------------------------------------------------------------------------------------
/** In the form of 
	* Label = Transition:<Damage|Repair> ToState:<BODYTYPE> EffectNum:<INT> OCL:<OCL NAME> */
// ------------------------------------------------------------------------------------------------
/*static*/ void TerrainRoadType::parseTransitionToOCL( INI *ini, 
																											 void *instance, 
																											 void *store, 
																											 const void *userData )
{
	const char *token;
	TerrainRoadType *theInstance = (TerrainRoadType *)instance;

	// which transition is this
	Bool damageTransition;
	token = ini->getNextSubToken( "Transition" );
	if( stricmp( token, "Damage" ) == 0 )
		damageTransition = TRUE;
	else if( stricmp( token, "Repair" ) == 0 )
		damageTransition = FALSE;
	else
	{

		DEBUG_CRASH(( "Expected Damage/Repair transition keyword\n" ));
		throw INI_INVALID_DATA;

	}  // end else

	// get body damage state
	token = ini->getNextSubToken( "ToState" );
	BodyDamageType state = (BodyDamageType)ini->scanIndexList( token, TheBodyDamageTypeNames );

	// get effect num
	token = ini->getNextSubToken( "EffectNum" );
	Int effectNum = ini->scanInt( token );
	
	// make effectNum zero based
	--effectNum;

	// sanity check effect num
	if( effectNum < 0 || effectNum >= MAX_BRIDGE_BODY_FX )
	{

		DEBUG_CRASH(( "Effect number max on bridge transitions is '%d'\n", MAX_BRIDGE_BODY_FX ));
		throw INI_INVALID_DATA;

	}  // end if

	// read the string
	token = ini->getNextSubToken( "OCL" );
	if( damageTransition )
		theInstance->friend_setDamageToOCLString( state, effectNum, AsciiString( token ) );
	else
		theInstance->friend_setRepairedToOCLString( state, effectNum, AsciiString( token ) );
	
}  // end parseTransitionToOCL

// ------------------------------------------------------------------------------------------------
/** In the form of
	* Label = Transition:<Damage|Repair> ToState:<BODYTYPE> EffectNum:<INT> FX:<FXLIST NAME> */
// ------------------------------------------------------------------------------------------------
/*static*/ void TerrainRoadType::parseTransitionToFX( INI *ini, 
																											void *instance, 
																											void *store, 
																											const void *userData )
{
	const char *token;
	TerrainRoadType *theInstance = (TerrainRoadType *)instance;

	// which transition is this
	Bool damageTransition;
	token = ini->getNextSubToken( "Transition" );
	if( stricmp( token, "Damage" ) == 0 )
		damageTransition = TRUE;
	else if( stricmp( token, "Repair" ) == 0 )
		damageTransition = FALSE;
	else
	{

		DEBUG_CRASH(( "Expected Damage/Repair transition keyword\n" ));
		throw INI_INVALID_DATA;

	}  // end else

	// get body damage state
	token = ini->getNextSubToken( "ToState" );
	BodyDamageType state = (BodyDamageType)ini->scanIndexList( token, TheBodyDamageTypeNames );

	// get effect num
	token = ini->getNextSubToken( "EffectNum" );
	Int effectNum = ini->scanInt( token );
	
	// make effectNum zero based
	--effectNum;

	// sanity check effect num
	if( effectNum < 0 || effectNum >= MAX_BRIDGE_BODY_FX )
	{

		DEBUG_CRASH(( "Effect number max on bridge transitions is '%d'\n", MAX_BRIDGE_BODY_FX ));
		throw INI_INVALID_DATA;

	}  // end if

	// read the string
	token = ini->getNextSubToken( "FX" );
	if( damageTransition )
		theInstance->friend_setDamageToFXString( state, effectNum, AsciiString( token ) );
	else
		theInstance->friend_setRepairedToFXString( state, effectNum, AsciiString( token ) );

}  // end parseTransitionToFX

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
TerrainRoadType::TerrainRoadType( void )
{

	m_isBridge = FALSE;
	m_id = 0;
	m_next = NULL;
	m_roadWidth = 0.0f;
	m_roadWidthInTexture = 0.0f;
	m_bridgeScale = 1.0f;
	m_radarColor.red = 0.0f;
	m_radarColor.green = 0.0f;
	m_radarColor.blue = 0.0f;
	m_transitionEffectsHeight = 0.0f;
	m_numFXPerType = 0;

}  // end TerrainRoadType

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
TerrainRoadType::~TerrainRoadType( void )
{

}  // end ~TerrainRoadType

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
TerrainRoadCollection::TerrainRoadCollection( void )
{

	m_roadList = NULL;
	m_bridgeList = NULL;

	m_idCounter = 1;   ///< MUST start this at 1.

}  // end TerrainRoadCollection

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
TerrainRoadCollection::~TerrainRoadCollection( void )
{
	TerrainRoadType *temp;

	// delete all roads in the list
	while( m_roadList )
	{

		// get next road
		temp = m_roadList->friend_getNext();

		// delete this road
		MemoryPoolObject::deleteInstance(m_roadList);

		// set the new head of the list
		m_roadList = temp;

	}  // end while

	// delete all bridges in the list
	while( m_bridgeList )
	{

		// get next bridge
		temp = m_bridgeList->friend_getNext();

		// delete this bridge
		MemoryPoolObject::deleteInstance(m_bridgeList);

		// set the new head of the list
		m_bridgeList = temp;

	}  // end while

}  // end ~TerrainRoadCollection

//-------------------------------------------------------------------------------------------------
/** Find road with matching name */
//-------------------------------------------------------------------------------------------------
TerrainRoadType *TerrainRoadCollection::findRoad( AsciiString name )
{
	TerrainRoadType *road;

	for( road = m_roadList; road; road = nextRoad( road ) )
	{

		if( road->getName() == name )
			return road;

	}  // end for road

	// not found
	return NULL;

}  // end findRoad

//-------------------------------------------------------------------------------------------------
/** Find bridge with matching name */
//-------------------------------------------------------------------------------------------------
TerrainRoadType *TerrainRoadCollection::findBridge( AsciiString name )
{
	TerrainRoadType *bridge;

	for( bridge = m_bridgeList; bridge; bridge = nextBridge( bridge ) )
	{

		if( bridge->getName() == name )
			return bridge;

	}  // end for bridge

	// not found
	return NULL;

}  // end findBridge

//-------------------------------------------------------------------------------------------------
/** Search the roads AND bridge lists for the name */
//-------------------------------------------------------------------------------------------------
TerrainRoadType *TerrainRoadCollection::findRoadOrBridge( AsciiString name )
{
	TerrainRoadType *road = findRoad( name );

	if( road )
		return road;
	else
		return findBridge( name );

}  // end findRoadOrBridge

//-------------------------------------------------------------------------------------------------
/** Allocate a new road, set the name, and link to the road list */
//-------------------------------------------------------------------------------------------------
TerrainRoadType *TerrainRoadCollection::newRoad( AsciiString name )
{
	TerrainRoadType *road = newInstance(TerrainRoadType);

	// assign the name
	road->friend_setName( name );

	// assign unique id
	road->friend_setID( m_idCounter++ );

	// is not a bridge
	road->friend_setBridge( FALSE );

	// set defaults from the default road
	TerrainRoadType *defaultRoad = findRoad( AsciiString( "DefaultRoad" ) );
	if( defaultRoad )
	{
		
		road->friend_setTexture( defaultRoad->getTexture() );
		road->friend_setRoadWidth( defaultRoad->getRoadWidth() );
		road->friend_setRoadWidthInTexture( defaultRoad->getRoadWidthInTexture() );

	}  // end if

	// link to list
	road->friend_setNext( m_roadList );
	m_roadList = road;

	// return the new road
	return road;

}  // end newRoad

//-------------------------------------------------------------------------------------------------
/** Allocate a new bridge */
//-------------------------------------------------------------------------------------------------
TerrainRoadType *TerrainRoadCollection::newBridge( AsciiString name )
{
	TerrainRoadType *bridge = newInstance(TerrainRoadType);

	// assign the name
	bridge->friend_setName( name );

	// assign unique id
	bridge->friend_setID( m_idCounter++ );

	// is a bridge
	bridge->friend_setBridge( TRUE );

	// set defaults from the default bridge
	TerrainRoadType *defaultBridge = findBridge( AsciiString( "DefaultBridge" ) );
	if( defaultBridge )
	{
		
		bridge->friend_setTexture( defaultBridge->getTexture() );
		bridge->friend_setBridgeScale( defaultBridge->getBridgeScale() );
		bridge->friend_setBridgeModelName( defaultBridge->getBridgeModel() );
		bridge->friend_setBridgeModelNameDamaged( defaultBridge->getBridgeModelNameDamaged() );
		bridge->friend_setBridgeModelNameReallyDamaged( defaultBridge->getBridgeModelNameReallyDamaged() );
		bridge->friend_setBridgeModelNameBroken( defaultBridge->getBridgeModelNameBroken() );
		bridge->friend_setTextureDamaged( defaultBridge->getTextureDamaged() );
		bridge->friend_setTextureReallyDamaged( defaultBridge->getTextureReallyDamaged() );
		bridge->friend_setTextureBroken( defaultBridge->getTextureBroken() );

		bridge->friend_setTransitionEffectsHeight( defaultBridge->getTransitionEffectsHeight() );
		bridge->friend_setNumFXPerType( defaultBridge->getNumFXPerType() );
		for( Int state = BODY_PRISTINE; state < BODYDAMAGETYPE_COUNT; state++ )
		{

			bridge->friend_setDamageToSoundString( (BodyDamageType)state, bridge->getDamageToSoundString( (BodyDamageType)state ) );
			bridge->friend_setRepairedToSoundString( (BodyDamageType)state, bridge->getRepairedToSoundString( (BodyDamageType)state ) );

			for( Int i = 0; i < MAX_BRIDGE_BODY_FX; i++ )
			{

				bridge->friend_setDamageToOCLString( (BodyDamageType)state, i, bridge->getDamageToOCLString( (BodyDamageType)state, i ) );
				bridge->friend_setDamageToFXString( (BodyDamageType)state, i, bridge->getDamageToOCLString( (BodyDamageType)state, i ) );
				bridge->friend_setRepairedToOCLString( (BodyDamageType)state, i, bridge->getDamageToOCLString( (BodyDamageType)state, i ) );
				bridge->friend_setRepairedToFXString( (BodyDamageType)state, i, bridge->getDamageToOCLString( (BodyDamageType)state, i ) );

			}  // end for i

		}  // end for

	}  // end if

	// link to list
	bridge->friend_setNext( m_bridgeList );
	m_bridgeList = bridge;

	// return the new bridge
	return bridge;

}  // end newBridge

//-------------------------------------------------------------------------------------------------
/** Return next road in list */
//-------------------------------------------------------------------------------------------------
TerrainRoadType *TerrainRoadCollection::nextRoad( TerrainRoadType *road )
{

	DEBUG_ASSERTCRASH( road->isBridge() == FALSE, ("nextRoad: road not a road\n") );
	return road->friend_getNext();

}  // end nextRoad

//-------------------------------------------------------------------------------------------------
/** Return next bridge in list */
//-------------------------------------------------------------------------------------------------
TerrainRoadType *TerrainRoadCollection::nextBridge( TerrainRoadType *bridge )
{

	DEBUG_ASSERTCRASH( bridge->isBridge() == TRUE, ("nextBridge, bridge is not a bridge\n") );
	return bridge->friend_getNext();

}  // end nextBridge

