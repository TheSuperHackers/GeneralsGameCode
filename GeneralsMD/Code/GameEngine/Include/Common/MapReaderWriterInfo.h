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

// MapReaderWriterInfo.h
// Version and interface info for map file writers and reader.
// Author: John Ahlquist, October 2001

#pragma once

#define K_HEIGHT_MAP_VERSION_1	1	// Height map cell = 5.0
#define K_HEIGHT_MAP_VERSION_2	2	// Height map cell = 10.0
#define K_HEIGHT_MAP_VERSION_3	3	// Added m_borderSize
#define K_HEIGHT_MAP_VERSION_4	4	// Major rev. See comments at bottom of file.
#define K_BLEND_TILE_VERSION_1	1	// Height map cell = 5.0
#define K_BLEND_TILE_VERSION_2	2	// Height map cell = 10.0
#define K_BLEND_TILE_VERSION_3	3	// Added long diagonal blends.
#define K_BLEND_TILE_VERSION_4	4	// Added custom edge blends.
#define K_BLEND_TILE_VERSION_5	5	// Added custom cliff u/v coordinates.
#define K_BLEND_TILE_VERSION_6	6	// Added extra blend layer for 3 textures in cell.
#define K_BLEND_TILE_VERSION_7	7	// Added flag for painting passable/impassable to cells.
#define K_BLEND_TILE_VERSION_8	8	// Added flag for painting passable/impassable to cells.
#define K_OBJECTS_VERSION_1			1	// no dict
#define K_OBJECTS_VERSION_2			2	// includes dict
#define K_OBJECTS_VERSION_3			3	// includes dict
#define K_MAP_OBJECT_VERSION_1	1
#define K_WAYPOINTS_VERSION_1		1
#define K_PLAYERLIST_VERSION_1	1
#define K_TRIGGERS_VERSION_1 1
#define K_TRIGGERS_VERSION_2 2		// Added m_isWaterArea
#define K_TRIGGERS_VERSION_3 3		// Added m_isRiver & m_riverStart
#define K_TRIGGERS_VERSION_4 4		// Added layer name.
#define K_LIGHTING_VERSION_1	1
#define K_LIGHTING_VERSION_2	2	// Added 2 additional global lights for objects.
#define K_LIGHTING_VERSION_3	3	// Added 2 additional global lights for terrain.
#define K_WORLDDICT_VERSION_1 1
#define K_MAPPREVIEW_VERSION_1 1
/** Virtual helper class, so that we can write map data using FILE* or CFile. */
class OutputStream {
public:
	virtual Int write(const void *pData, Int numBytes) = 0;
};

/** Virtual helper class, so that we can read in tile and map data from a
variety of sources, such as FILE* or CFile. */
class InputStream {
public:
	virtual Int read(void *pData, Int numBytes) = 0;
};

/** Virtual helper class, so that we can read in tile and map data from a
variety of sources, such as FILE* or CFile. */
class ChunkInputStream : public InputStream{
public:
	virtual Int read(void *pData, Int numBytes) = 0;
	virtual UnsignedInt tell(void) = 0;
	virtual Bool absoluteSeek(UnsignedInt pos) = 0;
	virtual Bool eof(void) = 0;
};

/** An instance of InputStream that uses a FILE* to read data. */
class CachedFileInputStream : public ChunkInputStream
{
protected:
	int m_size;
	char* m_buffer;
	int m_pos;
public:
	CachedFileInputStream(void);
	~CachedFileInputStream(void);
	Bool open(AsciiString path);	///< Returns true if open succeeded.
	void close(void);  ///< Explict close.  Destructor closes if file is left open.
	virtual Int read(void *pData, Int numBytes);
	virtual UnsignedInt tell(void);
	virtual Bool absoluteSeek(UnsignedInt pos);
	virtual Bool eof(void);
	void rewind(void);
};

/** An instance of InputStream that uses a FILE* to read data. */
/* Always using Cached version, as that lets us do compression, etc easily
class FileInputStream : public ChunkInputStream
{
protected:
	File *m_file;
public:
	FileInputStream(void);
	~FileInputStream(void);
	Bool open(AsciiString path);	///< Returns true if open succeeded.
	void close(void);  ///< Explict close.  Destructor closes if file is left open.
	virtual Int read(void *pData, Int numBytes);
	virtual UnsignedInt tell(void);
	virtual Bool absoluteSeek(UnsignedInt pos);
	virtual Bool eof(void);
	void rewind(void);
};
*/

/*
	rev K_HEIGHT_MAP_VERSION_4

	This is a major rev of the heightmap chunk. Here's the basic overview of what has happened:
	We now support multiple boundary areas. They are stored in a vector of ICoord2Ds. The lower-
	left corner is always (0,0), and the ICoord2D specifies the top-right coordinate.
	The boundary also contains a name.
*/
