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

// FILE: MapUtil.cpp //////////////////////////////////////////////////////////////////////////////
// Author: Matt Campbell, December 2001
// Description: Map utility/convenience routines
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/crc.h"
#include "Common/FileSystem.h"
#include "Common/LocalFileSystem.h"
#include "Common/file.h"
#include "Common/GlobalData.h"
#include "Common/GameState.h"
#include "Common/GameEngine.h"
#include "Common/NameKeyGenerator.h"
#include "Common/DataChunk.h"
#include "Common/MapReaderWriterInfo.h"
#include "Common/MessageStream.h"
#include "Common/WellKnownKeys.h"
#include "Common/INI.h"
#include "Common/QuotedPrintable.h"
#include "Common/SkirmishBattleHonors.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "Common/MapObject.h"
#include "GameClient/GameText.h" 
#include "GameClient/WindowLayout.h"
#include "GameClient/Gadget.h"
#include "GameClient/Image.h"
#include "GameClient/Shell.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GadgetListBox.h"
#include "GameClient/MapUtil.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/FPUControl.h"
#include "GameNetwork/GameInfo.h"
#include "GameNetwork/NetworkDefs.h"


//-------------------------------------------------------------------------------
// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////////
static const char *mapExtension = ".map";

static Int m_width = 0;						///< Height map width.
static Int m_height = 0;					///< Height map height (y size of array).
static Int m_borderSize = 0;			///< Non-playable border area.
static std::vector<ICoord2D> m_boundaries;	///< All the boundaries we use for the map
static Int m_dataSize = 0;				///< size of m_data.
static UnsignedByte *m_data = 0;	///< array of z(height) values in the height map.
static Dict worldDict = 0;

static WaypointMap *m_waypoints = 0;
static Coord3DList	m_supplyPositions;
static Coord3DList	m_techPositions;

static Int m_mapDX = 0;
static Int m_mapDY = 0;

static UnsignedInt calcCRC( AsciiString dirName, AsciiString fname )
{
	CRC theCRC;
	theCRC.clear();

	// Try the official map dir
	AsciiString asciiFile;
	char	tempBuf[_MAX_PATH];
	char	filenameBuf[_MAX_PATH];
	int length = 0;
	strcpy(tempBuf, fname.str());
	length = strlen( tempBuf );
	if( length >= 4 )
	{
		memset( filenameBuf, '\0', _MAX_PATH);
		strncpy( filenameBuf, tempBuf, length - 4);
	}

	File *fp;
	asciiFile = fname;
	fp = TheFileSystem->openFile(asciiFile.str(), File::READ);
	if( !fp )
	{
		DEBUG_CRASH(("Couldn't open '%s'", fname.str()));
		return 0;
	}

	UnsignedByte buf[4096];
	Int num;
	while ( (num=fp->read(buf, 4096)) > 0 )
	{
		theCRC.computeCRC(buf, num);
	}

	fp->close();
	fp = NULL;

	return theCRC.get();
}

static Bool ParseObjectDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
	Bool readDict = info->version >= K_OBJECTS_VERSION_2;

	Coord3D loc;
	loc.x = file.readReal();
	loc.y = file.readReal();
	loc.z = file.readReal();
	if (info->version <= K_OBJECTS_VERSION_2) 
	{
		loc.z = 0;
	}

	Real angle = file.readReal();
	Int flags = file.readInt(); 
	AsciiString name = file.readAsciiString();
	Dict d;
	if (readDict)
	{
		d = file.readDict();
	}
	MapObject *pThisOne;
	
	// create the map object
	pThisOne = newInstance( MapObject )( loc, name, angle, flags, &d, 
														TheThingFactory->findTemplate( name, FALSE ) );

//DEBUG_LOG(("obj %s owner %s",name.str(),d.getAsciiString(TheKey_originalOwner).str()));

	if (pThisOne->getProperties()->getType(TheKey_waypointID) == Dict::DICT_INT)
	{
		pThisOne->setIsWaypoint();

		// grab useful info
		(*m_waypoints)[pThisOne->getWaypointName()] = loc;
	}
	else if (pThisOne->getThingTemplate() && pThisOne->getThingTemplate()->isKindOf(KINDOF_TECH_BUILDING))
	{
		m_techPositions.push_back(loc);
	}
	else if (pThisOne->getThingTemplate() && pThisOne->getThingTemplate()->isKindOf(KINDOF_SUPPLY_SOURCE_ON_PREVIEW))
	{
		m_supplyPositions.push_back(loc);
	}

	deleteInstance(pThisOne);
	return TRUE;
}

static Bool ParseObjectsDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
	file.m_currentObject = NULL;
	file.registerParser( AsciiString("Object"), info->label, ParseObjectDataChunk );
	return (file.parse(userData));
}

static Bool ParseWorldDictDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
	worldDict = file.readDict();
	return true;
}

static Bool ParseSizeOnly(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
	m_width = file.readInt();
	m_height = file.readInt();
	if (info->version >= K_HEIGHT_MAP_VERSION_3) {
		m_borderSize = file.readInt();
	} else {
		m_borderSize = 0;
	}

	if (info->version >= K_HEIGHT_MAP_VERSION_4) {
		Int numBorders = file.readInt();
		m_boundaries.resize(numBorders);
		for (int i = 0; i < numBorders; ++i) {
			m_boundaries[i].x = file.readInt();
			m_boundaries[i].y = file.readInt();
		}
	}
	return true;

	m_dataSize = file.readInt();
	m_data = NEW UnsignedByte[m_dataSize];	// pool[]ify
	if (m_dataSize <= 0 || (m_dataSize != (m_width*m_height))) {
		throw ERROR_CORRUPT_FILE_FORMAT	;
	}
	file.readArrayOfBytes((char *)m_data, m_dataSize);
	// Resize me. 
	if (info->version == K_HEIGHT_MAP_VERSION_1) {
		Int newWidth = (m_width+1)/2;
		Int newHeight = (m_height+1)/2;
		Int i, j;
		for (i=0; i<newHeight; i++) {
			for (j=0; j<newWidth; j++) {
				m_data[i*newWidth+j] = m_data[2*i*m_width+2*j];
			}
		}
		m_width = newWidth;
		m_height = newHeight;
	}
	return true;
}

static Bool ParseSizeOnlyInChunk(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
	return ParseSizeOnly(file, info, userData);
}

static Bool loadMap( AsciiString filename )
{
	char	tempBuf[_MAX_PATH];
	char	filenameBuf[_MAX_PATH];
	AsciiString asciiFile;
	int length = 0;

	strcpy(tempBuf, filename.str());

	length = strlen( tempBuf );
	if( length >= 4 )
	{
		memset( filenameBuf, '\0', _MAX_PATH);
		strncpy( filenameBuf, tempBuf, length - 4);
	}

	CachedFileInputStream fileStrm;

	asciiFile = filename;
	if( !fileStrm.open(asciiFile) )
	{
		return FALSE;
	}

	ChunkInputStream *pStrm = &fileStrm;

	DataChunkInput file( pStrm );

	m_waypoints = NEW WaypointMap;

	file.registerParser( AsciiString("HeightMapData"), AsciiString::TheEmptyString, ParseSizeOnlyInChunk );
	file.registerParser( AsciiString("WorldInfo"), AsciiString::TheEmptyString, ParseWorldDictDataChunk );
	file.registerParser( AsciiString("ObjectsList"), AsciiString::TheEmptyString, ParseObjectsDataChunk );
	if (!file.parse(NULL)) {
		throw(ERROR_CORRUPT_FILE_FORMAT);
	}

	m_mapDX = m_width  - 2*m_borderSize;
	m_mapDY = m_height - 2*m_borderSize;

	return TRUE;
}

static void resetMap( void )
{
	if (m_data)
	{
		delete[] m_data;
		m_data = 0;
	}

	if (m_waypoints)
	{
		delete m_waypoints;
		m_waypoints = 0;
	}
	m_techPositions.clear();
	m_supplyPositions.clear();
}

static void getExtent( Region3D *extent )
{
	extent->lo.x = 0.0f;

	extent->lo.y = 0.0f;

	// Note - m_mapDX & Y are the number of height map grids wide, so we have to
	// multiply by the grid width.
	extent->hi.x = m_mapDX*MAP_XY_FACTOR;
	extent->hi.y = m_mapDY*MAP_XY_FACTOR;

	extent->lo.z = 0;
	extent->hi.z = 0;
}

//-------------------------------------------------------------------------------

void WaypointMap::update( void )
{
	if (!m_waypoints)
	{
		m_numStartSpots = 1;
		return;
	}

	this->clear();

	AsciiString startingCamName = TheNameKeyGenerator->keyToName(TheKey_InitialCameraPosition);
	WaypointMap::const_iterator it;

	it = m_waypoints->find(startingCamName);
	if (it != m_waypoints->end())
	{
		(*this)[startingCamName] = it->second;
	}

	m_numStartSpots = 0;
	for (Int i=0; i<MAX_SLOTS; ++i)
	{
		startingCamName.format("Player_%d_Start", i+1); // start pos waypoints are 1-based
		it = m_waypoints->find(startingCamName);
		if (it != m_waypoints->end())
		{
			(*this)[startingCamName] = it->second;
			++m_numStartSpots;
		}
		else
		{
			break;
		}
	}

	m_numStartSpots = max(1, m_numStartSpots);
}

const char * MapCache::m_mapCacheName = "MapCache.ini";

AsciiString MapCache::getMapDir() const 
{ 
	return AsciiString("Maps"); 
}

AsciiString MapCache::getUserMapDir() const
{
	AsciiString tmp = TheGlobalData->getPath_UserData();
	tmp.concat(getMapDir());
	return tmp;
}

AsciiString MapCache::getMapExtension() const
{
	return AsciiString("map");
}

void MapCache::writeCacheINI( Bool userDir )
{
	AsciiString mapDir;
	if (!userDir || TheGlobalData->m_buildMapCache)
	{
		mapDir = getMapDir();
	}
	else
	{
		mapDir = getUserMapDir();
	}

	AsciiString filepath = mapDir;
	filepath.concat('\\');

	TheFileSystem->createDirectory(mapDir);

	filepath.concat(m_mapCacheName);
	FILE *fp = fopen(filepath.str(), "w");
	DEBUG_ASSERTCRASH(fp != NULL, ("Failed to create %s", filepath.str()));
	if (fp == NULL) {
		return;
	}
	fprintf(fp, "; FILE: %s /////////////////////////////////////////////////////////////\n", filepath.str());
	fprintf(fp, "; This INI file is auto-generated - do not modify\n");
	fprintf(fp, "; /////////////////////////////////////////////////////////////////////////////\n");
	mapDir.toLower();

	MapCache::iterator it = begin();
	MapMetaData md;
	while (it != end())
	{
		if (it->first.startsWithNoCase(mapDir.str()))
		{
			md = it->second;
			fprintf(fp, "\nMapCache %s\n", AsciiStringToQuotedPrintable(it->first.str()).str());
			fprintf(fp, "  fileSize = %u\n", md.m_filesize);
			fprintf(fp, "  fileCRC = %u\n", md.m_CRC);
			fprintf(fp, "  timestampLo = %d\n", md.m_timestamp.m_lowTimeStamp);
			fprintf(fp, "  timestampHi = %d\n", md.m_timestamp.m_highTimeStamp);
			fprintf(fp, "  isOfficial = %s\n", (md.m_isOfficial)?"yes":"no");

			fprintf(fp, "  isMultiplayer = %s\n", (md.m_isMultiplayer)?"yes":"no");
			fprintf(fp, "  numPlayers = %d\n", md.m_numPlayers);

			fprintf(fp, "  extentMin = X:%2.2f Y:%2.2f Z:%2.2f\n", md.m_extent.lo.x, md.m_extent.lo.y, md.m_extent.lo.z);
			fprintf(fp, "  extentMax = X:%2.2f Y:%2.2f Z:%2.2f\n", md.m_extent.hi.x, md.m_extent.hi.y, md.m_extent.hi.z);

// BAD AND NOW UNUSED:  the mapcache.ini should not contain localized data... using the lookup tag instead
//			fprintf(fp, "  displayName = %s\n", UnicodeStringToQuotedPrintable(md.m_displayName).str());
			fprintf(fp, "  nameLookupTag = %s\n", md.m_nameLookupTag.str());

			Coord3D pos;
			WaypointMap::iterator itw = md.m_waypoints.begin();
			while (itw != md.m_waypoints.end())
			{
				pos = itw->second;
				fprintf(fp, "  %s = X:%2.2f Y:%2.2f Z:%2.2f\n", itw->first.str(), pos.x, pos.y, pos.z);
				++itw;
			}
			Coord3DList::iterator itc3d = md.m_techPositions.begin();
			while (itc3d != md.m_techPositions.end())
			{
				pos = *itc3d;
				fprintf(fp, "  techPosition = X:%2.2f Y:%2.2f Z:%2.2f\n", pos.x, pos.y, pos.z);
				itc3d++;
			}
			
			itc3d = md.m_supplyPositions.begin();
			while (itc3d != md.m_supplyPositions.end())
			{
				pos = *itc3d;
				fprintf(fp, "  supplyPosition = X:%2.2f Y:%2.2f Z:%2.2f\n", pos.x, pos.y, pos.z);
				itc3d++;
			}
			fprintf(fp, "END\n\n");
		}
		else
		{
			//DEBUG_LOG(("%s does not start %s", mapDir.str(), it->first.str()));
		}
		++it;
	}

	fclose(fp);
}

void MapCache::updateCache( void )
{
	setFPMode();

	TheFileSystem->createDirectory(getUserMapDir());

	if (loadUserMaps())
	{
		writeCacheINI( TRUE );
	}
	loadStandardMaps();	// we shall overwrite info from matching user maps to prevent munkees from getting rowdy :)
#if defined(RTS_DEBUG)
	if (TheLocalFileSystem->doesFileExist(getMapDir().str()))
	{
		// only create the map cache file if "Maps" exist
		Bool wasBuildMapCache = TheGlobalData->m_buildMapCache;
		TheWritableGlobalData->m_buildMapCache = true;
		loadUserMaps();
		TheWritableGlobalData->m_buildMapCache = wasBuildMapCache;
		writeCacheINI( FALSE );
	}
#endif
}

Bool MapCache::clearUnseenMaps( AsciiString dirName )
{
	dirName.toLower();
	Bool erasedSomething = FALSE;

	std::map<AsciiString, Bool>::iterator it = m_seen.begin();

	while (it != m_seen.end())
	{
		AsciiString mapName = it->first;
		if (it->second == FALSE && mapName.startsWithNoCase(dirName.str()))
		{
			// not seen in the dir - clear it out.
			erase(mapName);
			erasedSomething = TRUE;
		}
		++it;
	}
	return erasedSomething;
}

void MapCache::loadStandardMaps(void)
{
	INI ini;
	AsciiString fname;
	fname.format("%s\\%s", getMapDir().str(), m_mapCacheName);
#if defined(RTS_DEBUG)
	File *fp = TheFileSystem->openFile(fname.str(), File::READ);
	if (fp != NULL)
	{
		fp->close();
		fp = NULL;
#endif
		ini.load( fname, INI_LOAD_OVERWRITE, NULL );
#if defined(RTS_DEBUG)
	}
#endif
}

Bool MapCache::loadUserMaps()
{
	// Read in map list from disk
	AsciiString mapDir;
	if (TheGlobalData->m_buildMapCache)
	{
		mapDir = getMapDir();
	}
	else
	{
		mapDir = getUserMapDir();

		INI ini;
		AsciiString fname;
		fname.format("%s\\%s", mapDir.str(), m_mapCacheName);
		File *fp = TheFileSystem->openFile(fname.str(), File::READ);
		if (fp)
		{
			fp->close();
			ini.load( fname, INI_LOAD_OVERWRITE, NULL );
		}

	}

	// mark all as unseen
	m_seen.clear();
	MapCache::iterator it = begin();
	while (it != end())
	{
		m_seen[it->first] = FALSE;
		++it;
	}

	FilenameList filenameList;
	FilenameListIter iter;
	AsciiString toplevelPattern;
	toplevelPattern.format("%s\\", mapDir.str());
	Bool parsedAMap = FALSE;
	AsciiString filenamepattern;
	filenamepattern.format("*.%s", getMapExtension().str());

	TheFileSystem->getFileListInDirectory(toplevelPattern, filenamepattern, filenameList, TRUE);

	iter = filenameList.begin();

	while (iter != filenameList.end()) {
		FileInfo fileInfo;
		AsciiString tempfilename;
		tempfilename = (*iter);
		tempfilename.toLower();

		const char *s = tempfilename.reverseFind('\\');
		if (!s)
		{
			DEBUG_CRASH(("Couldn't find \\ in map name!"));
		}
		else
		{
			AsciiString endingStr;
			AsciiString fname = s+1;
			for (size_t i=0; i<strlen(mapExtension); ++i)
				fname.removeLastChar();

			endingStr.format("%s\\%s%s", fname.str(), fname.str(), mapExtension);

			Bool skipMap = FALSE;
			if (TheGlobalData->m_buildMapCache)
			{
				std::set<AsciiString>::const_iterator sit = m_allowedMaps.find(fname);
				if (m_allowedMaps.size() != 0 && sit == m_allowedMaps.end())
				{
					//DEBUG_LOG(("Skipping map: '%s'", fname.str()));
					skipMap = TRUE;
				}
				else
				{
					//DEBUG_LOG(("Parsing map: '%s'", fname.str()));
				}
			}

			if (!skipMap)
			{
				if (!tempfilename.endsWithNoCase(endingStr.str()))
				{
					DEBUG_CRASH(("Found map '%s' in wrong spot (%s)", fname.str(), tempfilename.str()));
				}
				else
				{
					if (TheFileSystem->getFileInfo(tempfilename, &fileInfo)) {
						char funk[_MAX_PATH];
						strcpy(funk, tempfilename.str());
						char *filenameptr = funk;
						char *tempchar = funk;
						while (*tempchar != 0) {
							if ((*tempchar == '\\') || (*tempchar == '/')) {
								filenameptr = tempchar+1;
							}
							++tempchar;
						}

						m_seen[tempfilename] = TRUE;
						parsedAMap |= addMap(mapDir, *iter, &fileInfo, TheGlobalData->m_buildMapCache);
					} else {
						DEBUG_CRASH(("Could not get file info for map %s", (*iter).str()));
					}
				}
			}
		}
		iter++;
	}

	// clean out unseen maps
	if (clearUnseenMaps(mapDir))
		return TRUE;

	return parsedAMap;
}

//Bool MapCache::addMap( AsciiString dirName, AsciiString fname, WinTimeStamp timestamp, UnsignedInt filesize, Bool isOfficial )
Bool MapCache::addMap( AsciiString dirName, AsciiString fname, FileInfo *fileInfo, Bool isOfficial)
{
	if (fileInfo == NULL) {
		return FALSE;
	}

	AsciiString lowerFname;
	lowerFname = fname;
	lowerFname.toLower();
	MapCache::iterator it = find(lowerFname);

	MapMetaData md;
	UnsignedInt filesize = fileInfo->sizeLow;

	if (it != end())
	{
		// Found the map in our cache.  Check to see if it has changed.
		md = it->second;

		if ((md.m_filesize == filesize) &&
				(md.m_CRC != 0))
		{
			// Force a lookup so that we don't display the English localization in all builds.
			if (md.m_nameLookupTag.isEmpty())
			{
				// unofficial maps or maps without names
				AsciiString tempdisplayname;
				tempdisplayname = fname.reverseFind('\\') + 1;
				(*this)[lowerFname].m_displayName.translate(tempdisplayname);
				if (md.m_numPlayers >= 2)
				{
					UnicodeString extension;
					extension.format(L" (%d)", md.m_numPlayers);
					(*this)[lowerFname].m_displayName.concat(extension);
				}
			}
			else
			{
				// official maps with name tags
				(*this)[lowerFname].m_displayName = TheGameText->fetch(md.m_nameLookupTag);
				if (md.m_numPlayers >= 2)
				{
					UnicodeString extension;
					extension.format(L" (%d)", md.m_numPlayers);
					(*this)[lowerFname].m_displayName.concat(extension);
				}
			}
//			DEBUG_LOG(("MapCache::addMap - found match for map %s", lowerFname.str()));
			return FALSE;	// OK, it checks out.
		}
		DEBUG_LOG(("%s didn't match file in MapCache", fname.str()));
		DEBUG_LOG(("size: %d / %d", filesize, md.m_filesize));
		DEBUG_LOG(("time1: %d / %d", fileInfo->timestampHigh, md.m_timestamp.m_highTimeStamp));
		DEBUG_LOG(("time2: %d / %d", fileInfo->timestampLow, md.m_timestamp.m_lowTimeStamp));
//		DEBUG_LOG(("size: %d / %d", filesize, md.m_filesize));
//		DEBUG_LOG(("time1: %d / %d", timestamp.m_highTimeStamp, md.m_timestamp.m_highTimeStamp));
//		DEBUG_LOG(("time2: %d / %d", timestamp.m_lowTimeStamp, md.m_timestamp.m_lowTimeStamp));
	}

	DEBUG_LOG(("MapCache::addMap(): caching '%s' because '%s' was not found", fname.str(), lowerFname.str()));

	loadMap(fname); // Just load for querying the data, since we aren't playing this map.

	// The map is now loaded.  Pick out what we need.
	md.m_fileName = lowerFname;
	md.m_filesize = filesize;
	md.m_isOfficial = isOfficial;
	md.m_waypoints.update();
	md.m_numPlayers = md.m_waypoints.m_numStartSpots;
	md.m_isMultiplayer = (md.m_numPlayers >= 2);
	md.m_timestamp.m_highTimeStamp = fileInfo->timestampHigh;
	md.m_timestamp.m_lowTimeStamp = fileInfo->timestampLow;
	md.m_supplyPositions = m_supplyPositions;
	md.m_techPositions = m_techPositions;
	md.m_CRC = calcCRC(dirName, fname);

	Bool exists = false;
	AsciiString munkee = worldDict.getAsciiString(TheKey_mapName, &exists);
	md.m_nameLookupTag = munkee;
	if (!exists || munkee.isEmpty())
	{
		DEBUG_LOG(("Missing TheKey_mapName!"));
		AsciiString tempdisplayname;
		tempdisplayname = fname.reverseFind('\\') + 1;
		md.m_displayName.translate(tempdisplayname);
		if (md.m_numPlayers >= 2)
		{
			UnicodeString extension;
			extension.format(L" (%d)", md.m_numPlayers);
			md.m_displayName.concat(extension);
		}
		TheGameText->reset();
	}
	else
	{
		AsciiString stringFileName;
		stringFileName.format("%s\\%s", dirName.str(), fname.str());
		for (Int i=0; i<4; ++i)
			stringFileName.removeLastChar();
		stringFileName.concat("\\map.str");
		TheGameText->initMapStringFile(stringFileName);
		md.m_displayName = TheGameText->fetch(munkee);
		if (md.m_numPlayers >= 2)
		{
			UnicodeString extension;
			extension.format(L" (%d)", md.m_numPlayers);
			md.m_displayName.concat(extension);
		}
		DEBUG_LOG(("Map name is now '%ls'", md.m_displayName.str()));
		TheGameText->reset();
	}

	getExtent(&(md.m_extent));

	(*this)[lowerFname] = md;

	DEBUG_LOG(("  filesize = %d bytes", md.m_filesize));
	DEBUG_LOG(("  displayName = %ls", md.m_displayName.str()));
	DEBUG_LOG(("  CRC = %X", md.m_CRC));
	DEBUG_LOG(("  timestamp = %d", md.m_timestamp));
	DEBUG_LOG(("  isOfficial = %s", (md.m_isOfficial)?"yes":"no"));

	DEBUG_LOG(("  isMultiplayer = %s", (md.m_isMultiplayer)?"yes":"no"));
	DEBUG_LOG(("  numPlayers = %d", md.m_numPlayers));

	DEBUG_LOG(("  extent = (%2.2f,%2.2f) -> (%2.2f,%2.2f)",
		md.m_extent.lo.x, md.m_extent.lo.y,
		md.m_extent.hi.x, md.m_extent.hi.y));

	Coord3D pos;
	WaypointMap::iterator itw = md.m_waypoints.begin();
	while (itw != md.m_waypoints.end())
	{
		pos = itw->second;
		DEBUG_LOG(("    waypoint %s: (%2.2f,%2.2f)", itw->first.str(), pos.x, pos.y));
		++itw;
	}

	resetMap();

	return TRUE;
}

MapCache *TheMapCache = NULL;

// PUBLIC FUNCTIONS //////////////////////////////////////////////////////////////////////////////

Bool WouldMapTransfer( const AsciiString& mapName )
{
	return mapName.startsWithNoCase(TheMapCache->getUserMapDir());
}

//-------------------------------------------------------------------------------------------------
/** Load the listbox with all the map files available to play */
//-------------------------------------------------------------------------------------------------
Int populateMapListboxNoReset( GameWindow *listbox, Bool useSystemMaps, Bool isMultiplayer, AsciiString mapToSelect )
{
	if(!TheMapCache)
		return -1;

	if (!listbox)
		return -1;
	
	// reset the listbox content
	//GadgetListBoxReset( listbox );
	
	Int numColumns = GadgetListBoxGetNumColumns( listbox );
	const Image *easyImage = NULL;
	const Image *mediumImage = NULL;
	const Image *brutalImage = NULL;
	const Image *maxBrutalImage = NULL;
	SkirmishBattleHonors *battleHonors = NULL;
	Int w = 10, h = 10;
	if (numColumns > 1)
	{
		easyImage = TheMappedImageCollection->findImageByName("Star-Bronze");
		mediumImage = TheMappedImageCollection->findImageByName("Star-Silver");
		brutalImage = TheMappedImageCollection->findImageByName("Star-Gold");
		maxBrutalImage = TheMappedImageCollection->findImageByName("RedYell_Star");
		battleHonors = new SkirmishBattleHonors;

		w = (brutalImage)?brutalImage->getImageWidth():10;
		w = min(GadgetListBoxGetColumnWidth(listbox, 0), w);
		h = w;
	}

	Color color = GameMakeColor( 255, 255, 255, 255 );
	UnicodeString mapDisplayName;

	Int selectionIndex = 0; // always select *something*

	MapCache::iterator it = TheMapCache->begin();
	AsciiString mapDir;
	if (useSystemMaps)
	{
		mapDir = TheMapCache->getMapDir();
	}
	else
	{
		mapDir = TheGlobalData->getPath_UserData();
		mapDir.concat(TheMapCache->getMapDir());
	}
	mapDir.toLower();

typedef std::set<UnicodeString, rts::less_than_nocase<UnicodeString> > MapNameList;
typedef MapNameList::iterator MapNameListIter;

typedef std::map<UnicodeString, AsciiString> MapDisplayToFileNameList;
typedef MapDisplayToFileNameList::iterator MapDisplayToFileNameListIter;

	MapNameList tempCache;
	MapDisplayToFileNameList filenameMap;
	UnsignedInt numMapsListed = 0;
	UnsignedInt curNumPlayersInMap = 0;

	while (numMapsListed < TheMapCache->size()) {

		DEBUG_LOG(("Adding maps with %d players", curNumPlayersInMap));
		it = TheMapCache->begin();
		while (it != TheMapCache->end()) {
			const MapMetaData *md = &(it->second);
			if (md != NULL) {
				if (md->m_numPlayers == curNumPlayersInMap) {
					tempCache.insert(it->second.m_displayName);
					filenameMap[it->second.m_displayName] = it->first;
					DEBUG_LOG(("Adding map %s to temp cache.", it->first.str()));
					++numMapsListed;
				}
			}
			++it;
		}

		MapNameListIter tempit = tempCache.begin();

		while (tempit != tempCache.end())
		{

			AsciiString asciiMapName;
			asciiMapName = filenameMap[*tempit];
			it = TheMapCache->find(asciiMapName);
			/*
			if (it != TheMapCache->end())
			{
				DEBUG_LOG(("populateMapListbox(): looking at %s (displayName = %ls), mp = %d (== %d?) mapDir=%s (ok=%d)",
					it->first.str(), it->second.m_displayName.str(), it->second.m_isMultiplayer, isMultiplayer,
					mapDir.str(), it->first.startsWith(mapDir.str())));
			}
			*/
			
			//Patch 1.03 -- Purposely filter out these broken maps that exist in Generals.
			if( !asciiMapName.compare( "maps\\armored fury\\armored fury.map" ) || 
				!asciiMapName.compare( "maps\\scorched earth\\scorched earth.map" ) )
			{
				++tempit;
				continue;
			}

			DEBUG_ASSERTCRASH(it != TheMapCache->end(), ("Map %s not found in map cache.", tempit->str()));
			if (it->first.startsWithNoCase(mapDir.str()) && isMultiplayer == it->second.m_isMultiplayer && !it->second.m_displayName.isEmpty())
			{
				/// @todo: mapDisplayName = TheGameText->fetch(it->second.m_displayName.str());
				mapDisplayName = it->second.m_displayName;
				Int index = -1;
				Int imageItemData = -1;
				if (numColumns > 1 && it->second.m_isMultiplayer)
				{
					Int numEasy = battleHonors->getEnduranceMedal(it->first.str(), SLOT_EASY_AI);
					Int numMedium = battleHonors->getEnduranceMedal(it->first.str(), SLOT_MED_AI);
					Int numBrutal = battleHonors->getEnduranceMedal(it->first.str(), SLOT_BRUTAL_AI);
					if (numBrutal)
					{
						int maxBrutalSlots = it->second.m_numPlayers - 1;
						if (numBrutal == maxBrutalSlots)
						{
							index = GadgetListBoxAddEntryImage( listbox, maxBrutalImage, index, 0, w, h, TRUE);
							imageItemData = 4;
						}
						else	
						{
							index = GadgetListBoxAddEntryImage( listbox, brutalImage, index, 0, w, h, TRUE);
							imageItemData = 3;
						}
					}
					else if (numMedium)
					{
						imageItemData = 2;
						index = GadgetListBoxAddEntryImage( listbox, mediumImage, index, 0, w, h, TRUE);
					}
					else if (numEasy)
					{
						imageItemData = 1;
						index = GadgetListBoxAddEntryImage( listbox, easyImage, index, 0, w, h, TRUE);
					}
					else
					{
						imageItemData = 0;
						index = GadgetListBoxAddEntryImage( listbox, NULL, index, 0, w, h, TRUE);
					}
				}
				index = GadgetListBoxAddEntryText( listbox, mapDisplayName, color, index, numColumns-1 );

				if (it->first == mapToSelect)
				{
					selectionIndex = index;
				}

				// now set the char* as the item data.  this works because the map cache isn't being
				// modified while a map listbox is up.
				GadgetListBoxSetItemData( listbox, (void *)(it->first.str()), index );

				if (numColumns > 1)
				{
					GadgetListBoxSetItemData( listbox, (void *)imageItemData, index, 1 );
				}
			}
			++tempit;
		}

		tempCache.clear();
		filenameMap.clear();
		++curNumPlayersInMap;
	}

	if (battleHonors)
	{
		delete battleHonors;
		battleHonors = NULL;
	}

	GadgetListBoxSetSelected(listbox, &selectionIndex, 1);
	if (selectionIndex >= 0)
	{
		Int topIndex = GadgetListBoxGetTopVisibleEntry(listbox);
		Int bottomIndex = GadgetListBoxGetBottomVisibleEntry(listbox);
		Int rowsOnScreen = bottomIndex - topIndex;

		if (selectionIndex >= bottomIndex)
		{
			Int newTop = max( 0, selectionIndex - max( 1, rowsOnScreen / 2 ) ); 
		//The trouble is that rowsonscreen/2 can be zero if bottom is 1 and top is zero
			GadgetListBoxSetTopVisibleEntry( listbox, newTop );
		}
	}
	return selectionIndex;

}  // end loadMapListbox

//-------------------------------------------------------------------------------------------------
/** Load the listbox with all the map files available to play */
//-------------------------------------------------------------------------------------------------
Int populateMapListbox( GameWindow *listbox, Bool useSystemMaps, Bool isMultiplayer, AsciiString mapToSelect )
{
	if(!TheMapCache)
		return -1;

	if (!listbox)
		return -1;
	
	// reset the listbox content
	GadgetListBoxReset( listbox );

	return populateMapListboxNoReset( listbox, useSystemMaps, isMultiplayer, mapToSelect );
}
	


//-------------------------------------------------------------------------------------------------
/** Validate a map */
//-------------------------------------------------------------------------------------------------
Bool isValidMap( AsciiString mapName, Bool isMultiplayer )
{
	if(!TheMapCache || mapName.isEmpty())
		return FALSE;
	TheMapCache->updateCache();

	mapName.toLower();
	MapCache::iterator it = TheMapCache->find(mapName);
	if (it != TheMapCache->end())
	{
		if (isMultiplayer == it->second.m_isMultiplayer)
		{
			return TRUE;
		}
	}

	return FALSE;
}  // end isValidMap

//-------------------------------------------------------------------------------------------------
/** Find a valid map */
//-------------------------------------------------------------------------------------------------
AsciiString getDefaultMap( Bool isMultiplayer )
{
	if(!TheMapCache)
		return AsciiString::TheEmptyString;
	TheMapCache->updateCache();

	MapCache::iterator it = TheMapCache->begin();
	while (it != TheMapCache->end())
	{
		if (isMultiplayer == it->second.m_isMultiplayer)
		{
			return it->first;
		}
		++it;
	}

	return AsciiString::TheEmptyString;
}


AsciiString getDefaultOfficialMap()
{
	if(!TheMapCache)
		return AsciiString::TheEmptyString;
	TheMapCache->updateCache();

	MapCache::iterator it = TheMapCache->begin();
	while (it != TheMapCache->end())
	{
		if (it->second.m_isMultiplayer && it->second.m_isOfficial)
		{
			return it->first;
		}
		++it;
	}
	return AsciiString::TheEmptyString;
}


Bool isOfficialMap( AsciiString mapName )
{
	if(!TheMapCache || mapName.isEmpty())
		return FALSE;
	TheMapCache->updateCache();
	mapName.toLower();
	MapCache::iterator it = TheMapCache->find(mapName);
	if (it != TheMapCache->end())
		return it->second.m_isOfficial;
	return FALSE;
}


const MapMetaData *MapCache::findMap(AsciiString mapName)
{
	mapName.toLower();
	MapCache::iterator it = find(mapName);
	if (it == end())
		return NULL;
	return &(it->second);
}

// ------------------------------------------------------------------------------------------------
/** Embed the pristine map into the xfer stream */
// ------------------------------------------------------------------------------------------------
static void copyFromBigToDir( const AsciiString& infile, const AsciiString& outfile )
{
	// open the map file

	File *file = TheFileSystem->openFile( infile.str(), File::READ | File::BINARY );
	if( file == NULL )
	{
		DEBUG_CRASH(( "copyFromBigToDir - Error opening source file '%s'", infile.str() ));
		throw SC_INVALID_DATA;
	} // end if

	// how big is the map file
	Int fileSize = file->seek( 0, File::END );


	// rewind to beginning of file
	file->seek( 0, File::START );

	// allocate buffer big enough to hold the entire map file
	char *buffer = NEW char[ fileSize ];
	if( buffer == NULL )
	{
		DEBUG_CRASH(( "copyFromBigToDir - Unable to allocate buffer for file '%s'", infile.str() ));
		throw SC_INVALID_DATA;
	} // end if

	// copy the file to the buffer
	if( file->read( buffer, fileSize ) < fileSize )
	{
		DEBUG_CRASH(( "copyFromBigToDir - Error reading from file '%s'", infile.str() ));
		throw SC_INVALID_DATA;
	} // end if
	// close the BIG file
	file->close();
	
	File *filenew = TheFileSystem->openFile( outfile.str(), File::WRITE | File::CREATE | File::BINARY );
	
	if( !filenew || filenew->write(buffer, fileSize) < fileSize)
	{
		DEBUG_CRASH(( "copyFromBigToDir - Error writing to file '%s'", outfile.str() ));
		throw SC_INVALID_DATA;
	} // end if

	filenew->close();

	// delete the buffer
	delete [] buffer;
} // end embedPristineMap

Image *getMapPreviewImage( AsciiString mapName )
{
	if(!TheGlobalData)
		return NULL;
	DEBUG_LOG(("%s Map Name ", mapName.str()));
	AsciiString tgaName = mapName;
	AsciiString name;
	AsciiString tempName;
	AsciiString filename;
	tgaName.removeLastChar(); // p
	tgaName.removeLastChar(); // a
	tgaName.removeLastChar(); // m
	tgaName.removeLastChar(); // .
	name = tgaName;//.reverseFind('\\') + 1;
	filename = tgaName.reverseFind('\\') + 1;
	//tgaName = name;
	filename.concat(".tga");
	tgaName.concat(".tga");

	AsciiString portableName = TheGameState->realMapPathToPortableMapPath(name);
	tempName.set(AsciiString::TheEmptyString);
	for(Int i = 0; i < portableName.getLength(); ++i)
	{
		char c = portableName.getCharAt(i);
		if (c == '\\' || c == ':')
			tempName.concat('_');
		else
			tempName.concat(c);
	}
	
	name = tempName;
	name.concat(".tga");

	
	// copy file over	
	// copy source tgaName, to name

	Image *image = (Image *)TheMappedImageCollection->findImageByName(tempName);
	if(!image)
	{

		if(!TheFileSystem->doesFileExist(tgaName.str()))
			return NULL;	
		AsciiString mapPreviewDir;
		mapPreviewDir.format(MAP_PREVIEW_DIR_PATH, TheGlobalData->getPath_UserData().str());
		TheFileSystem->createDirectory(mapPreviewDir);

		mapPreviewDir.concat(name);

		Bool success = false;
		try
		{
			copyFromBigToDir(tgaName, mapPreviewDir);	
			success = true;
		} 
		catch (...)
		{
			success = false;	// no rethrow
		}
		
		if (success)
		{
    	image = newInstance(Image);
			image->setName(tempName);
			//image->setFullPath("mission.tga");
			image->setFilename(name);
			image->setStatus(IMAGE_STATUS_NONE);
			Region2D uv;
			uv.hi.x = 1.0f;
			uv.hi.y = 1.0f;
			uv.lo.x	= 0.0f;
			uv.lo.y = 0.0f;
			image->setUV(&uv);
			image->setTextureHeight(128);
			image->setTextureWidth(128);
			TheMappedImageCollection->addImage(image);
		}
		else
		{
			image = NULL;
		}
	}

	return image;

	
	
/*
	// sanity
	if( mapName.isEmpty() )
		return NULL;
	Region2D uv;
	mapPreviewImage = TheMappedImageCollection->findImageByName("MapPreview");
	if(mapPreviewImage)
		deleteInstance(mapPreviewImage);
	
	mapPreviewImage = TheMappedImageCollection->newImage();
	mapPreviewImage->setName("MapPreview");
	mapPreviewImage->setStatus(IMAGE_STATUS_RAW_TEXTURE);
// allocate our terrain texture
	TextureClass * texture = new TextureClass( size.x, size.y, 
																			 WW3D_FORMAT_X8R8G8B8, TextureClass::MIP_LEVELS_1 );
	uv.lo.x = 0.0f;
	uv.lo.y = 1.0f;
	uv.hi.x = 1.0f;
	uv.hi.y = 0.0f;
	mapPreviewImage->setStatus( IMAGE_STATUS_RAW_TEXTURE );
	mapPreviewImage->setRawTextureData( texture );
	mapPreviewImage->setUV( &uv );
	mapPreviewImage->setTextureWidth( size.x );
	mapPreviewImage->setTextureHeight( size.y );
	mapPreviewImage->setImageSize( &size );


	CachedFileInputStream theInputStream;
	if (theInputStream.open(AsciiString(mapName.str()))) 
	{
		ChunkInputStream *pStrm = &theInputStream;
		pStrm->absoluteSeek(0);
		DataChunkInput file( pStrm );
		if (file.isValidFileType()) {	// Backwards compatible files aren't valid data chunk files.
			// Read the waypoints.
			file.registerParser( AsciiString("MapPreview"), AsciiString::TheEmptyString, parseMapPreviewChunk );
			if (!file.parse(NULL)) {
				DEBUG_ASSERTCRASH(false,("Unable to read MapPreview info."));
				deleteInstance(mapPreviewImage);
				return NULL;
			}
		}
		theInputStream.close();
	}
	else
	{
		deleteInstance(mapPreviewImage);
		return NULL;
	}
	

	return mapPreviewImage;
	
*/
	return NULL;
}

Bool parseMapPreviewChunk(DataChunkInput &file, DataChunkInfo *info, void *userData)
{
/*
	ICoord2D size;
	
	SurfaceClass *surface;
	size.x = file.readInt();
	size.y = file.readInt();


	surface = (TextureClass *)mapPreviewImage->getRawTextureData()->Get_Surface_Level();
	//texture->Get_Surface_Level();
	
	DEBUG_LOG(("BeginMapPreviewInfo"));
	UnsignedInt *buffer = new UnsignedInt[size.x * size.y];
	Int x,y;
	for (y=0; y<size.y; y++) {
		for(x = 0; x< size.x; x++)
		{
			surface->DrawPixel( x, y, file.readInt() );
			buffer[y + x] = file.readInt();
			DEBUG_LOG(("x:%d, y:%d, %X", x, y, buffer[y + x]));
		}
	}
	mapPreviewImage->setRawTextureData(buffer);
	DEBUG_ASSERTCRASH(file.atEndOfChunk(), ("Unexpected data left over."));
	DEBUG_LOG(("EndMapPreviewInfo"));
	REF_PTR_RELEASE(surface);
	return true;
*/
	return FALSE;
}

void findDrawPositions( Int startX, Int startY, Int width, Int height, Region3D extent,
															 ICoord2D *ul, ICoord2D *lr )
{

	Real ratioWidth;
	Real ratioHeight;
	Coord2D radar;
	ratioWidth = extent.width()/(width * 1.0f);
	ratioHeight = extent.height()/(height* 1.0f);
	
	if( ratioWidth >= ratioHeight)
	{
		radar.x = extent.width() / ratioWidth;
		radar.y = extent.height()/ ratioWidth;
		ul->x = 0;
		ul->y = (height - radar.y) / 2.0f;
		lr->x = radar.x;
		lr->y = height - ul->y;
	}
	else
	{
		radar.x = extent.width() / ratioHeight;
		radar.y = extent.height()/ ratioHeight;
		ul->x = (width - radar.x ) / 2.0f;
		ul->y = 0;
		lr->x = width - ul->x;
		lr->y = radar.y;
	}

	// make them pixel positions
	ul->x += startX;
	ul->y += startY;
	lr->x += startX;
	lr->y += startY;

}  // end findDrawPositions

