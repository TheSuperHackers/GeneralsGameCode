//////////////////////////////////////////
// FILE: MapData.cpp
// Store some extra metadata about the current map
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/MapData.h"

// PUBLIC DATA ////////////////////////////////////////////////////////////////////////////////////
MapData* TheWriteableMapData = NULL;				///< The current map data singleton

///////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
/*static*/ const FieldParse MapData::s_MapDataFieldParseTable[] =
{
	{ "HeightMapScale",									INI::parseReal,				NULL,			offsetof( MapData, m_HeightmapScale) },
	{ "EnableShips",									  INI::parseBool,       NULL,     offsetof( MapData, m_enableShips) },
	{ NULL,					NULL,						NULL,						0 }  // keep this last

};

MapData::MapData() : SubsystemInterface()
{
	m_HeightmapScale = 1.0f;
	m_enableShips = false;
}

void MapData::init() {
	m_HeightmapScale = 1.0f;
	m_enableShips = false;
}

void MapData::reset() {
	m_HeightmapScale = 1.0f;
	m_enableShips = false;
}

void MapData::parseMapDataDefinition(INI* ini) {
	ini->initFromINI(TheWriteableMapData, s_MapDataFieldParseTable);
}
