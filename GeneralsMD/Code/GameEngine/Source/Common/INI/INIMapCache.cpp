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

// FILE: INIMapCache.cpp ///////////////////////////////////////////////////////////////////////////
// Author: Matthew D. Campbell, February 2002
// Desc:   Parsing MapCache INI entries
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Lib/BaseType.h"
#include "Common/INI.h"
#include "GameClient/MapUtil.h"
#include "GameClient/GameText.h"
#include "GameNetwork/NetworkDefs.h"
#include "Common/NameKeyGenerator.h"
#include "Common/WellKnownKeys.h"
#include "Common/QuotedPrintable.h"


class MapMetaDataReader
{
public:
	Region3D m_extent;
	Int m_numPlayers;
	Bool m_isMultiplayer;
	AsciiString m_asciiDisplayName;
	AsciiString m_asciiNameLookupTag;

	Bool m_isOfficial;
	WinTimeStamp m_timestamp;
	UnsignedInt m_filesize;
	UnsignedInt m_CRC;

	Coord3D m_waypoints[MAX_SLOTS];
	Coord3D m_initialCameraPosition;
	Coord3DList m_supplyPositions;
	Coord3DList m_techPositions;
	static const FieldParse m_mapFieldParseTable[];		///< the parse table for INI definition
	const FieldParse *getFieldParse( void ) const { return m_mapFieldParseTable; }
};


void parseSupplyPositionCoord3D( INI* ini, void * instance, void * /*store*/, const void* /*userData*/ )
{
	MapMetaDataReader *mmdr = (MapMetaDataReader *)instance;
	Coord3D coord3d;
	INI::parseCoord3D(ini, NULL, &coord3d,NULL );
	mmdr->m_supplyPositions.push_front(coord3d);

}

void parseTechPositionsCoord3D( INI* ini, void * instance, void * /*store*/, const void* /*userData*/ )
{
	MapMetaDataReader *mmdr = (MapMetaDataReader *)instance;
	Coord3D coord3d;
	INI::parseCoord3D(ini, NULL, &coord3d,NULL );
	mmdr->m_techPositions.push_front(coord3d);

}

const FieldParse MapMetaDataReader::m_mapFieldParseTable[] =
{

	{ "isOfficial",							INI::parseBool,			NULL,	offsetof( MapMetaDataReader, m_isOfficial ) },
	{ "isMultiplayer",					INI::parseBool,			NULL,	offsetof( MapMetaDataReader, m_isMultiplayer ) },
	{ "extentMin",							INI::parseCoord3D,	NULL, offsetof( MapMetaDataReader, m_extent.lo ) },
	{ "extentMax",							INI::parseCoord3D,	NULL, offsetof( MapMetaDataReader, m_extent.hi ) },
	{ "numPlayers",							INI::parseInt,			NULL,	offsetof( MapMetaDataReader, m_numPlayers ) },
	{ "fileSize",								INI::parseUnsignedInt,	NULL,	offsetof( MapMetaDataReader, m_filesize ) },
	{ "fileCRC",								INI::parseUnsignedInt,	NULL,	offsetof( MapMetaDataReader, m_CRC ) },
	{ "timestampLo",						INI::parseInt,			NULL,	offsetof( MapMetaDataReader, m_timestamp.m_lowTimeStamp ) },
	{ "timestampHi",						INI::parseInt,			NULL,	offsetof( MapMetaDataReader, m_timestamp.m_highTimeStamp ) },
	{ "displayName",						INI::parseAsciiString,	NULL,	offsetof( MapMetaDataReader, m_asciiDisplayName ) },
	{ "nameLookupTag",					INI::parseAsciiString,	NULL,	offsetof( MapMetaDataReader, m_asciiNameLookupTag ) },

	{ "supplyPosition",					parseSupplyPositionCoord3D,	NULL, NULL },
	{ "techPosition",						parseTechPositionsCoord3D,	NULL, NULL },

	{ "Player_1_Start",					INI::parseCoord3D,	NULL,	offsetof( MapMetaDataReader, m_waypoints ) },
	{ "Player_2_Start",					INI::parseCoord3D,	NULL,	offsetof( MapMetaDataReader, m_waypoints ) + sizeof(Coord3D) * 1 },
	{ "Player_3_Start",					INI::parseCoord3D,	NULL,	offsetof( MapMetaDataReader, m_waypoints ) + sizeof(Coord3D) * 2 },
	{ "Player_4_Start",					INI::parseCoord3D,	NULL,	offsetof( MapMetaDataReader, m_waypoints ) + sizeof(Coord3D) * 3 },
	{ "Player_5_Start",					INI::parseCoord3D,	NULL,	offsetof( MapMetaDataReader, m_waypoints ) + sizeof(Coord3D) * 4 },
	{ "Player_6_Start",					INI::parseCoord3D,	NULL,	offsetof( MapMetaDataReader, m_waypoints ) + sizeof(Coord3D) * 5 },
	{ "Player_7_Start",					INI::parseCoord3D,	NULL,	offsetof( MapMetaDataReader, m_waypoints ) + sizeof(Coord3D) * 6 },
	{ "Player_8_Start",					INI::parseCoord3D,	NULL,	offsetof( MapMetaDataReader, m_waypoints ) + sizeof(Coord3D) * 7 },

	{ "InitialCameraPosition",	INI::parseCoord3D,	NULL,	offsetof( MapMetaDataReader, m_initialCameraPosition ) },

	{ NULL,					NULL,						NULL,						0 }

};

void INI::parseMapCacheDefinition( INI* ini )
{
	const char *c;
	AsciiString name;
	MapMetaDataReader mdr;
	MapMetaData md;

	// read the name
	c = ini->getNextToken(" \n\r\t");
	name.set( c );
	name = QuotedPrintableToAsciiString(name);
	md.m_waypoints.clear();

	ini->initFromINI( &mdr, mdr.getFieldParse() );

	md.m_extent = mdr.m_extent;
	md.m_isOfficial = mdr.m_isOfficial != 0;
	md.m_isMultiplayer = mdr.m_isMultiplayer != 0;
	md.m_numPlayers = mdr.m_numPlayers;
	md.m_filesize = mdr.m_filesize;
	md.m_CRC = mdr.m_CRC;
	md.m_timestamp = mdr.m_timestamp;

	md.m_waypoints[TheNameKeyGenerator->keyToName(TheKey_InitialCameraPosition)] = mdr.m_initialCameraPosition;

#if RTS_GENERALS
	md.m_displayName = QuotedPrintableToUnicodeString(mdr.m_asciiDisplayName);
#else
	md.m_nameLookupTag = QuotedPrintableToAsciiString(mdr.m_asciiNameLookupTag);

	if (md.m_nameLookupTag.isEmpty())
	{
		// maps without localized name tags
		AsciiString tempdisplayname;
		tempdisplayname = name.reverseFind('\\') + 1;
		md.m_displayName.translate(tempdisplayname);
		if (md.m_numPlayers >= 2)
		{
			UnicodeString extension;
			extension.format(L" (%d)", md.m_numPlayers);
			md.m_displayName.concat(extension);
		}
	}
	else
	{
		// official maps with name tags
		md.m_displayName = TheGameText->fetch(md.m_nameLookupTag);
		if (md.m_numPlayers >= 2)
		{
			UnicodeString extension;
			extension.format(L" (%d)", md.m_numPlayers);
			md.m_displayName.concat(extension);
		}
	}
#endif

	AsciiString startingCamName;
	for (Int i=0; i<md.m_numPlayers; ++i)
	{
		startingCamName.format("Player_%d_Start", i+1); // start pos waypoints are 1-based
		md.m_waypoints[startingCamName] = mdr.m_waypoints[i];
	}

	Coord3DList::iterator it = mdr.m_supplyPositions.begin();
	while( it != mdr.m_supplyPositions.end())
	{
		md.m_supplyPositions.push_front(*it);
		it++;
	}

	it = mdr.m_techPositions.begin();
	while( it != mdr.m_techPositions.end())
	{
		md.m_techPositions.push_front(*it);
		it++;
	}

	if(TheMapCache && !md.m_displayName.isEmpty())
	{
		AsciiString lowerName = name;
		lowerName.toLower();
		md.m_fileName = lowerName;
//		DEBUG_LOG(("INI::parseMapCacheDefinition - adding %s to map cache", lowerName.str()));
		(*TheMapCache)[lowerName] = md;
	}
}

