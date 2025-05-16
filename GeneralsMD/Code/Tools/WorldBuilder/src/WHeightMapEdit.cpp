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

// WorldHeightMapEdit.cpp
// Class to encapsulate height map.
// Author: John Ahlquist, April 2001

#include "StdAfx.h" 
#include "resource.h"
#include "Common/STLTypedefs.h"
#include "WHeightMapEdit.h"
#include "W3DDevice/GameClient/TileData.h"
#include "W3DDevice/GameClient/TerrainTex.h"
#include "TerrainModal.h"
#include "Common/Debug.h"
#include "Common/GlobalData.h"
#include "Common/MapReaderWriterInfo.h"
#include "Common/FileSystem.h"
#include "Common/TerrainTypes.h"
#include "GameLogic/PolygonTrigger.h"
#include "GameLogic/SidesList.h"
#include "rendobj.h"
#include "W3DDevice/GameClient/W3DShadow.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "Common/WellKnownKeys.h"
#include "mapobjectprops.h"
#include "LayersList.h"

#include "Common/DataChunk.h"

#ifdef RTS_INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

int WorldHeightMapEdit::m_numGlobalTextureClasses=0;
TGlobalTextureClass WorldHeightMapEdit::m_globalTextureClasses[NUM_TEXTURE_CLASSES];
/** Destructor -.
*/
WorldHeightMapEdit::~WorldHeightMapEdit(void)
{
}

void WorldHeightMapEdit::shutdown(void)
{
	Int i, j;
	for (i=0; i<m_numGlobalTextureClasses; i++) {
		for (j=0; j<MAX_TILES_PER_CLASS; j++) {

			REF_PTR_RELEASE(m_globalTextureClasses[i].tiles[j]);
			m_globalTextureClasses[i].name.clear();
			m_globalTextureClasses[i].filePath.clear();
			m_globalTextureClasses[i].uiName.clear();
		}
	}
	freeListOfMapObjects();
	PolygonTrigger::deleteTriggers();
}

void WorldHeightMapEdit::init(void)
{
	Int i, j;
	for (i=0; i<NUM_TEXTURE_CLASSES; i++) {
		for (j=0; j<MAX_TILES_PER_CLASS; j++) {
			m_globalTextureClasses[i].tiles[j] = NULL;
		}
		m_globalTextureClasses[i].terrainType = NULL;
	}
	loadBaseImages();

}

//
/// WorldHeightMapEdit - create a new height map .
//
WorldHeightMapEdit::WorldHeightMapEdit(Int width, Int height, UnsignedByte initialHeight, Int border):
					WorldHeightMap(),
					m_warnTooManyTex(false),
					m_warnTooManyBlend(false)
{
	int i;
	for (i=0; i<NUM_SOURCE_TILES; i++) {
		m_sourceTiles[i]=NULL;
		m_edgeTiles[i]=NULL;
	}
	if (width<0 || height < 0) {
		AfxMessageBox(IDS_BAD_VALUE);
		return;
	}
	width += 2*border;
	height += 2*border;
	m_dataSize = width*height;
	if (m_dataSize<=0) {
		AfxMessageBox(IDS_BAD_VALUE);
		return;
	}
	this->freeListOfMapObjects();

	m_width = width;
	m_height = height;
	m_borderSize = border;
	m_tileNdxes = new Short[m_dataSize];
	m_blendTileNdxes = new Short[m_dataSize];
	m_extraBlendTileNdxes = new Short[m_dataSize];
	m_cliffInfoNdxes = new Short[m_dataSize];
	m_data = new UnsignedByte[m_dataSize + m_width+1];
	m_numBitmapTiles = 0;
	m_numBlendedTiles = 1;
	m_numCliffInfo = 1;
	// Note - we have one less cell than the width & height. But for paranoia, allocate
	// extra row. jba.
	// 
	Int numBytesX = (m_width+7)/8;	//how many bytes to fit all bitflags
	Int numBytesY = m_height;	

	m_flipStateWidth=numBytesX;

	m_cellFlipState	= new UnsignedByte[numBytesX*numBytesY];
	m_cellCliffState	= new UnsignedByte[numBytesX*numBytesY];
	memset(m_cellFlipState,0,numBytesX*numBytesY);	//clear all flags
	memset(m_cellCliffState,0,numBytesX*numBytesY);	//clear all flags


	Int j;
	if (m_data == NULL) {
		AfxMessageBox(IDS_OUT_OF_MEMORY);
		m_dataSize = 0;
	} else {
		Int i;
		for (i=0; i<m_dataSize; i++) {
			m_tileNdxes[i] = 0;
			m_blendTileNdxes[i] = 0;
			m_extraBlendTileNdxes[i] = 0;
			m_cliffInfoNdxes[i] = 0;
			m_data[i] = initialHeight;
		}
	}

	// fill the map with a certain terrain texture
	for (i=0; i<m_width; i++) {
		for (j=0; j<m_height; j++) {
			Short ndx = getTileNdxForClass( i, j, m_numGlobalTextureClasses-1 );
			m_tileNdxes[i+m_width*j] = ndx;
		}
	}
	setDrawOrg(0,0);

	ICoord2D initialBorder;
	initialBorder.x = width - 2 * border;
	initialBorder.y = height - 2 * border;
	addBoundary(&initialBorder);
}

//
/// WorldHeightMapEdit - create a new height map from another map.
//
WorldHeightMapEdit::WorldHeightMapEdit(WorldHeightMapEdit *pThis):
WorldHeightMap(),
m_warnTooManyTex(false),
m_warnTooManyBlend(false)
{
	int i;
	for (i=0; i<NUM_SOURCE_TILES; i++) {
		m_edgeTiles[i]=NULL;
	}
	REF_PTR_SET(m_alphaEdgeTex, pThis->m_alphaEdgeTex);
	REF_PTR_SET(m_terrainTex, pThis->m_terrainTex);
	REF_PTR_SET(m_alphaTerrainTex, pThis->m_alphaTerrainTex);
	m_dataSize = pThis->m_dataSize;
	m_terrainTexHeight = pThis->m_terrainTexHeight;
	m_alphaTexHeight = pThis->m_alphaTexHeight;
	m_alphaEdgeHeight = pThis->m_alphaEdgeHeight;
	m_width = pThis->m_width;
#ifdef EVAL_TILING_MODES																		  
	m_tileMode = pThis->m_tileMode;
#endif
	m_height = pThis->m_height;
	m_borderSize = pThis->m_borderSize;
	m_drawOriginX = pThis->m_drawOriginX;
	m_drawOriginY = pThis->m_drawOriginY;
	m_drawWidthX = pThis->m_drawWidthX;
	m_drawHeightY = pThis->m_drawHeightY;
	m_numBitmapTiles = pThis->m_numBitmapTiles;
	m_numEdgeTiles = pThis->m_numEdgeTiles;
	m_numBlendedTiles = pThis->m_numBlendedTiles;
	m_numCliffInfo = pThis->m_numCliffInfo;
	m_numTextureClasses = pThis->m_numTextureClasses;
	for (i=0; i<m_numGlobalTextureClasses; i++) {
		m_textureClasses[i] = pThis->m_textureClasses[i];
	}
	m_numEdgeTextureClasses = pThis->m_numEdgeTextureClasses;
	for (i=0; i<m_numGlobalTextureClasses; i++) {
		m_edgeTextureClasses[i] = pThis->m_edgeTextureClasses[i];
	}
	m_tileNdxes = new Short[m_dataSize];
	m_blendTileNdxes = new Short[m_dataSize];
	m_extraBlendTileNdxes = new Short[m_dataSize];
	m_cliffInfoNdxes = new Short[m_dataSize];
	// Note - we have one less cell than the width & height. But for paranoia, allocate
	// extra row. jba.
	// 
	Int numBytesX = (m_width+7)/8;	//how many bytes to fit all bitflags
	Int numBytesY = m_height;	

	m_flipStateWidth=numBytesX;

	m_cellFlipState	= new UnsignedByte[numBytesX*numBytesY];
	m_cellCliffState	= new UnsignedByte[numBytesX*numBytesY];
	memset(m_cellFlipState,0,numBytesX*numBytesY);	//clear all flags
	memset(m_cellCliffState,0,numBytesX*numBytesY);	//clear all flags
	m_data = new UnsignedByte[m_dataSize + m_width+1];
	if (m_data == NULL) {
		AfxMessageBox(IDS_OUT_OF_MEMORY);
		m_dataSize = 0;
	} else {
		Int i;
		for (i=0; i<NUM_SOURCE_TILES; i++) {
			REF_PTR_SET(m_sourceTiles[i], pThis->m_sourceTiles[i]);
			REF_PTR_SET(m_edgeTiles[i], pThis->m_edgeTiles[i]);
		}
		for (i=0; i<pThis->m_numBlendedTiles; i++) {
			m_blendedTiles[i] = pThis->m_blendedTiles[i];
		}
		for (i=0; i<pThis->m_numCliffInfo; i++) {
			m_cliffInfo[i] = pThis->m_cliffInfo[i];
		}
		for (i=0; i<m_dataSize; i++) {
			m_data[i] = pThis->m_data[i];
			m_tileNdxes[i] = pThis->m_tileNdxes[i];
			m_blendTileNdxes[i] = pThis->m_blendTileNdxes[i];
			m_extraBlendTileNdxes[i] = pThis->m_extraBlendTileNdxes[i];
			m_cliffInfoNdxes[i] = pThis->m_cliffInfoNdxes[i];
		}
		for (i=0; i<m_flipStateWidth*numBytesY; i++) {
			m_cellFlipState[i] = pThis->m_cellFlipState[i];
			m_cellCliffState[i] = pThis->m_cellCliffState[i];
		}
	}

	m_boundaries = pThis->m_boundaries;
}

/**
 WorldHeightMapEdit - read a height map from a file.
 Just calls WorldHeightMap::WorldHeightMap(FILE *pStrm),
 then loads the texture classes.
*/	 
WorldHeightMapEdit::WorldHeightMapEdit(ChunkInputStream *pStrm):
	WorldHeightMap(pStrm),
	m_warnTooManyTex(false),
	m_warnTooManyBlend(false)
{
	Bool didMajorRemap = false;
	Int i, j;
	for (i=0; i<m_numGlobalTextureClasses; i++) {
		for (j=0; j<m_numTextureClasses; j++) {
			if (m_globalTextureClasses[i].name == m_textureClasses[j].name) {
				DEBUG_ASSERTCRASH(m_textureClasses[j].globalTextureClass == -1, ("oops")); // should be unintialized at this point.
				if (m_globalTextureClasses[i].width != m_textureClasses[i].width) {
					didMajorRemap = true;	// This will handle the differing tile widths in setBlendUsingCanonicalTile
				}
				m_textureClasses[j].globalTextureClass = i;
			}
		}
	}


	Bool didCancel = false;

	// check for missing texture classes.
	for (i=0; i<m_numTextureClasses; i++) {
		if (m_textureClasses[i].globalTextureClass < 0) {
			TerrainModal modalTerrainDlg(m_textureClasses[i].name, this);	
			if (IDOK==modalTerrainDlg.DoModal()) {
				Int globalTex = modalTerrainDlg.getNewNdx();
				m_textureClasses[i].globalTextureClass = globalTex;
				m_textureClasses[i].name = m_globalTextureClasses[globalTex].name;
				didMajorRemap = true;
			} else {
				didCancel = true;
				for (j=0; j<m_textureClasses[i].numTiles; j++) {
					REF_PTR_RELEASE(m_sourceTiles[m_textureClasses[i].firstTile+j]);
				}
			}
		}
	}
	if (didMajorRemap) {
		optimizeTiles();
	}
	selectDuplicates();
#ifdef DEBUG_CRASHING
	if (didCancel) {
		return; // won't check out right.
	}
//	Int curTile = 0;
	for (i=0; i<m_numTextureClasses; i++) {
		DEBUG_ASSERTCRASH(m_textureClasses[i].globalTextureClass >= 0, ("oops"));
	}

	for (i=0; i<m_dataSize; i++) {
		Int texNdx = this->m_tileNdxes[i];
		DEBUG_ASSERTCRASH( (texNdx>>2) < m_numBitmapTiles,("oops"));
		Int texClass = getTextureClassFromNdx(texNdx);
		DEBUG_ASSERTCRASH(texClass>=0,("oops"));
	}
#endif
}

/******************************************************************
	remapTextures
		Remaps the textures in the map.
*/
Bool WorldHeightMapEdit::remapTextures(void)
{
	Int i;
	Bool anyChanges;
	for (i=0; i<m_numTextureClasses; i++) {
		TerrainModal modalTerrainDlg(m_textureClasses[i].name, this);	
		if (IDOK==modalTerrainDlg.DoModal()) {
			Int globalTex = modalTerrainDlg.getNewNdx();
			m_textureClasses[i].globalTextureClass = globalTex;
			anyChanges = true;
			m_textureClasses[i].name = m_globalTextureClasses[globalTex].name;
		} 
	}
	if (anyChanges) {
		optimizeTiles();
	}
	return(anyChanges);
}

/// Load a tga bitmap into a set of tiles.
void WorldHeightMapEdit::loadBitmap(char *path, const char *uiName)
{
	CachedFileInputStream stream;
	if (!stream.open(AsciiString(path))) 
	{
		return;
	}
	InputStream *pStrm = &stream;

	Int numTiles = WorldHeightMap::countTiles(pStrm);
	int width;
	Bool ok=false;
	if (numTiles < 1) {
		return;
	}

	stream.rewind();

	Int texToUse = m_numGlobalTextureClasses;
	Int j;
	for (j=0; j<MAX_TILES_PER_CLASS; j++) {
		REF_PTR_RELEASE(m_globalTextureClasses[texToUse].tiles[j]);
	}
	for (width = 10; width >= 1; width--) {
		if (numTiles >= width*width) {
			numTiles = width*width;
			break;
		}
	}

#define BLEND_TILE_PREFIX "TE"
	ok = readTiles(pStrm, m_globalTextureClasses[texToUse].tiles, width);
	if (!ok) return;
	m_globalTextureClasses[texToUse].numTiles = numTiles;
	m_globalTextureClasses[texToUse].width = width;
	m_globalTextureClasses[texToUse].name = path;
	m_globalTextureClasses[texToUse].uiName = uiName;
	Bool isBlend = (0==strncmp(BLEND_TILE_PREFIX, uiName, strlen(BLEND_TILE_PREFIX)));
	m_globalTextureClasses[texToUse].isBlendEdgeTile = isBlend;
	m_globalTextureClasses[texToUse].filePath = path;
	m_numGlobalTextureClasses++;
}

/// Load the available bitmap images.
void WorldHeightMapEdit::loadBaseImages(void) 
{
 
 	/// @todo - take this out when we are done evaluating terrain textures. 
#if (defined(RTS_DEBUG) || defined(RTS_INTERNAL))
 	loadDirectoryOfImages("..\\TestArt\\TestTerrain");
#endif

	// load terrain types from INI definitions
	TerrainType *terrain;

	for( terrain = TheTerrainTypes->firstTerrain();
	     terrain;
			 terrain = TheTerrainTypes->nextTerrain( terrain ) )
	{
		 if (m_numGlobalTextureClasses == NUM_TEXTURE_CLASSES) 
		 {
			 break;
		 }
		// load the terrain definition for the WorldBuilder to reference
		loadImagesFromTerrainType( terrain );

	}  // end for

}

/// Loads all the images in a directory (including subdirectories)
void WorldHeightMapEdit::loadDirectoryOfImages(const char *pFilePath) 
{
	char				dirBuf[_MAX_PATH];
	char				findBuf[_MAX_PATH];
	char				fileBuf[_MAX_PATH];

	strcpy(dirBuf, pFilePath);
	int len = strlen(dirBuf);

	if (len > 0 && dirBuf[len - 1] != '\\') {
		dirBuf[len++] = '\\';
		dirBuf[len] = 0;
	}
	strcpy(findBuf, dirBuf);

	FilenameList filenameList;
	TheFileSystem->getFileListInDirectory(AsciiString(findBuf), AsciiString("*.*"), filenameList, TRUE);

	if (filenameList.size() == 0) {
		return;
	}
	FilenameList::iterator it = filenameList.begin();
	do {
		AsciiString filename = *it;
		//strcpy(fileBuf, dirBuf);
		strcpy(fileBuf, filename.str());
		loadBitmap(fileBuf, filename.str());

		++it;
	} while (it != filenameList.end());
}


/** Loads all the all terrain information for the WorldBuilder given the logical
	* TerrainType entity (i.e. tga file)*/
void WorldHeightMapEdit::loadImagesFromTerrainType( TerrainType *terrain )
{

	// sanity
	if( terrain == NULL )
		return;

	char buffer[ _MAX_PATH ];

	// build path to texture file
	sprintf( buffer, "%s%s", TERRAIN_TGA_DIR_PATH, terrain->getTexture().str() );

	// create ascii string for texture path
	AsciiString texturePath( buffer );

	// open file
	CachedFileInputStream stream;
	if( !stream.open( texturePath ) )
		return;

	// get pointer to stream
	InputStream *pStrm = &stream;

	// could the tiles in the file
	Int numTiles = WorldHeightMap::countTiles( pStrm );
	Int width;
	if( numTiles < 1 )
		return;

	// rewind the stream
	stream.rewind();

	// setup our entry in the global textures classes
	Int texToUse = m_numGlobalTextureClasses;
	Int j;
	for( j = 0; j < MAX_TILES_PER_CLASS; j++ )
		REF_PTR_RELEASE( m_globalTextureClasses[ texToUse ].tiles[ j ] );

	for (width = 10; width >= 1; width--) {
		if (numTiles >= width*width) {
			numTiles = width*width;
			break;
		}
	}

// -->

	// read the tiles out of .tga file
	Bool ok = readTiles( pStrm, m_globalTextureClasses[ texToUse ].tiles, width );
	if( !ok )
		return;
	m_globalTextureClasses[ texToUse ].name = terrain->getName();
	m_globalTextureClasses[ texToUse ].numTiles = numTiles;
	m_globalTextureClasses[ texToUse ].width = width;
	m_globalTextureClasses[ texToUse ].filePath = texturePath;
	m_globalTextureClasses[ texToUse ].uiName = terrain->getName();
	m_globalTextureClasses[ texToUse ].terrainType = terrain;
	m_globalTextureClasses[ texToUse ].isBlendEdgeTile = terrain->isBlendEdge();
	m_numGlobalTextureClasses++;

}																													

UnsignedByte * WorldHeightMapEdit::getPointerToClassTileData(Int texClass)
{
	TileData *pSrc = NULL;
	if (texClass >= 0 && texClass <= m_numGlobalTextureClasses) {
		pSrc = m_globalTextureClasses[texClass].tiles[0];
	}
	if (pSrc != NULL) {
		return(pSrc->getDataPtr());
	}
	return(NULL);
}



void WorldHeightMapEdit::getTexClassNeighbors(Int xIndex, Int yIndex, Int textureClass,
									  Int *pSideCount, Int *pTotalCount)
{
	Int i,j;
	int sideCount=0;
	int totalCount=0;
	for (i=xIndex-1; i<xIndex+2; i++) {
		for (j=yIndex-1; j<yIndex+2; j++) {
			if (i==xIndex && j==yIndex) continue;  // don't count self.
			// Use x and y for indexing, in case we are along an edge.
			// If we fall off the edge, clamp to the edge so that 
			// tiles on the edge have 8 neighbors too.
			Int x = i;
			Int y = j;
			if (x<0) x=0;
			if (y<0) y=0;
			if (x>=m_width) x = m_width-1;
			if (y>=m_height) y = m_height-1;

			Int curClass = this->getTextureClass(x, y);

			if (curClass == textureClass) {
				totalCount++;
				if (i==xIndex || j==yIndex) {
					sideCount++;
				}
			}
		}
	}
	*pSideCount = sideCount;
	*pTotalCount = totalCount;
}




//
/// SaveToFile - saves a height map to a file.
// Format is 
//		tag - 4 bytes 'HtMp'
//		version - 2 bytes
//		width, height, length - 4 byte Int little endian.
// 
//		length bytes of map data.
//		length bytes of tile ndxes.
//		numBitmapTiles, numBlendedTiles - 4 byte int little endian
//		numBlendedTiles of tile data.
//		
//		
//
void WorldHeightMapEdit::saveToFile(DataChunkOutput &chunkWriter)
{
	// This is the chunk writer stuff.  
	int i;

	/**********HEIGHT MAP DATA ***********************/
	chunkWriter.openDataChunk("HeightMapData", K_HEIGHT_MAP_VERSION_4);
	Int numBoundaries = m_boundaries.size();
		chunkWriter.writeInt(m_width);	
		chunkWriter.writeInt(m_height);
		chunkWriter.writeInt(m_borderSize);
		chunkWriter.writeInt(numBoundaries);
		for (i = 0; i < numBoundaries; ++i) {
			chunkWriter.writeInt(m_boundaries[i].x);
			chunkWriter.writeInt(m_boundaries[i].y);
		}
		chunkWriter.writeInt(m_dataSize);
		chunkWriter.writeArrayOfBytes((char *)m_data, m_dataSize);

	/*
		chunkWriter.writeInt(m_width);
		chunkWriter.writeInt(m_height);
		chunkWriter.writeInt(m_borderSize);
		chunkWriter.writeInt(m_dataSize);
		chunkWriter.writeArrayOfBytes((char *)m_data, m_dataSize);
	*/
		
	chunkWriter.closeDataChunk();

	/***************BLEND TILE DATA ***************/
	chunkWriter.openDataChunk("BlendTileData", K_BLEND_TILE_VERSION_8);
		chunkWriter.writeInt(m_dataSize);
		chunkWriter.writeArrayOfBytes((char*)m_tileNdxes, m_dataSize*sizeof(Short));
		chunkWriter.writeArrayOfBytes((char*)m_blendTileNdxes, m_dataSize*sizeof(Short));
		chunkWriter.writeArrayOfBytes((char*)m_extraBlendTileNdxes, m_dataSize*sizeof(Short));
		chunkWriter.writeArrayOfBytes((char*)m_cliffInfoNdxes, m_dataSize*sizeof(Short));
		chunkWriter.writeArrayOfBytes((char*)m_cellCliffState, m_height*m_flipStateWidth);
		chunkWriter.writeInt(m_numBitmapTiles);
		chunkWriter.writeInt(m_numBlendedTiles);
		chunkWriter.writeInt(m_numCliffInfo);

		// write out the terrain texture information
		// -->
		chunkWriter.writeInt(m_numTextureClasses);
		for (i=0; i<m_numTextureClasses; i++) {
			chunkWriter.writeInt(m_textureClasses[i].firstTile);
			chunkWriter.writeInt(m_textureClasses[i].numTiles);
			chunkWriter.writeInt(m_textureClasses[i].width);
			chunkWriter.writeInt(0); 
			chunkWriter.writeAsciiString(m_textureClasses[i].name);
		}

		// write out the terrain texture information
		// -->
		chunkWriter.writeInt(m_numEdgeTiles);
		chunkWriter.writeInt(m_numEdgeTextureClasses);
		for (i=0; i<m_numEdgeTextureClasses; i++) {
			chunkWriter.writeInt(m_edgeTextureClasses[i].firstTile);
			chunkWriter.writeInt(m_edgeTextureClasses[i].numTiles);
			chunkWriter.writeInt(m_edgeTextureClasses[i].width);
			chunkWriter.writeAsciiString(m_edgeTextureClasses[i].name);
		}

		for (i=1; i<m_numBlendedTiles; i++) {
			Int flag = FLAG_VAL;
			chunkWriter.writeInt(m_blendedTiles[i].blendNdx);
			chunkWriter.writeByte(m_blendedTiles[i].horiz);
			chunkWriter.writeByte(m_blendedTiles[i].vert);
			chunkWriter.writeByte(m_blendedTiles[i].rightDiagonal);
			chunkWriter.writeByte(m_blendedTiles[i].leftDiagonal);
			chunkWriter.writeByte(m_blendedTiles[i].inverted);
			chunkWriter.writeByte(m_blendedTiles[i].longDiagonal);
			chunkWriter.writeInt(m_blendedTiles[i].customBlendEdgeClass);
			chunkWriter.writeInt(flag);
		}
		for (i=1; i<m_numCliffInfo; i++) {
			chunkWriter.writeInt(m_cliffInfo[i].tileIndex);
			chunkWriter.writeReal(m_cliffInfo[i].u0);
			chunkWriter.writeReal(m_cliffInfo[i].v0);
			chunkWriter.writeReal(m_cliffInfo[i].u1);
			chunkWriter.writeReal(m_cliffInfo[i].v1);
			chunkWriter.writeReal(m_cliffInfo[i].u2);
			chunkWriter.writeReal(m_cliffInfo[i].v2);
			chunkWriter.writeReal(m_cliffInfo[i].u3);
			chunkWriter.writeReal(m_cliffInfo[i].v3);
			chunkWriter.writeByte(m_cliffInfo[i].flip);
			chunkWriter.writeByte(m_cliffInfo[i].mutant);
		}
	chunkWriter.closeDataChunk();

#ifdef EVAL_TILING_MODES
	chunkWriter.openDataChunk("FUNKY_TILING", 1);
	chunkWriter.writeInt(m_tileMode);
	chunkWriter.closeDataChunk();
#endif

	/***************WORLD DATA ***************/
	/* important, must write this chunk BEFORE the player-list chunk */
	MapObject::getWorldDict()->setInt(TheKey_weather, (Int)TheGlobalData->m_weather);
	chunkWriter.openDataChunk("WorldInfo", K_WORLDDICT_VERSION_1);
	chunkWriter.writeDict(*MapObject::getWorldDict());	
	chunkWriter.closeDataChunk();

	/***************PLAYER DATA ***************/
	/* important, must write this chunk BEFORE the object-list chunk */
	SidesList::WriteSidesDataChunk(chunkWriter);	// Player data dicts are in the sides chunk.

	/***************OBJECTS DATA ***************/
	chunkWriter.openDataChunk("ObjectsList", 	K_OBJECTS_VERSION_3);
		
	MapObject *pObj;
	for (pObj = MapObject::getFirstMapObject(); pObj; pObj = pObj->getNext()) 
	{
		chunkWriter.openDataChunk("Object", 	K_OBJECTS_VERSION_3);
			Coord3D loc = *pObj->getLocation();
			chunkWriter.writeReal( loc.x);
			chunkWriter.writeReal( loc.y);
			chunkWriter.writeReal( loc.z);
			chunkWriter.writeReal( pObj->getAngle());
			chunkWriter.writeInt(pObj->getFlags()); 
			chunkWriter.writeAsciiString(pObj->getName());	

			chunkWriter.writeDict(*pObj->getProperties());	

#ifdef NOT_IN_USE
			if (pObj->getFlag(FLAG_WAYPOINT)) {
				chunkWriter.writeInt(pObj->getWaypointID()); 
				chunkWriter.writeAsciiString(AsciiString(pObj->getWaypointName()));	
			}
			else if (pObj->getFlag(FLAG_LIGHT)) {
				TLightInfo info;
				pObj->getLightInfo(&info);
				chunkWriter.writeReal(info.m_heightAboveTerrain);
				chunkWriter.writeReal(info.m_innerRadius);
				chunkWriter.writeReal(info.m_outerRadius);
				chunkWriter.writeReal(info.m_lightAmbientColor.red);
				chunkWriter.writeReal(info.m_lightAmbientColor.green);
				chunkWriter.writeReal(info.m_lightAmbientColor.blue);
				chunkWriter.writeReal(info.m_lightDiffuseColor.red);
				chunkWriter.writeReal(info.m_lightDiffuseColor.green);
				chunkWriter.writeReal(info.m_lightDiffuseColor.blue);
			}
#endif
		chunkWriter.closeDataChunk();
	}
	chunkWriter.closeDataChunk();

	/***************POLY TRIGGER DATA ***************/
	PolygonTrigger::WritePolygonTriggersDataChunk(chunkWriter);

	/***************GLOBAL LIGHTING DATA ***************/
	chunkWriter.openDataChunk("GlobalLighting", 	K_LIGHTING_VERSION_3);
		chunkWriter.writeInt(TheGlobalData->m_timeOfDay);
		for (i=0; i<4; i++) {
			chunkWriter.writeReal(TheGlobalData->m_terrainLighting[i+TIME_OF_DAY_FIRST][0].ambient.red);
			chunkWriter.writeReal(TheGlobalData->m_terrainLighting[i+TIME_OF_DAY_FIRST][0].ambient.green);
			chunkWriter.writeReal(TheGlobalData->m_terrainLighting[i+TIME_OF_DAY_FIRST][0].ambient.blue);
			chunkWriter.writeReal(TheGlobalData->m_terrainLighting[i+TIME_OF_DAY_FIRST][0].diffuse.red);
			chunkWriter.writeReal(TheGlobalData->m_terrainLighting[i+TIME_OF_DAY_FIRST][0].diffuse.green);
			chunkWriter.writeReal(TheGlobalData->m_terrainLighting[i+TIME_OF_DAY_FIRST][0].diffuse.blue);
			chunkWriter.writeReal(TheGlobalData->m_terrainLighting[i+TIME_OF_DAY_FIRST][0].lightPos.x);
			chunkWriter.writeReal(TheGlobalData->m_terrainLighting[i+TIME_OF_DAY_FIRST][0].lightPos.y);
			chunkWriter.writeReal(TheGlobalData->m_terrainLighting[i+TIME_OF_DAY_FIRST][0].lightPos.z);

			chunkWriter.writeReal(TheGlobalData->m_terrainObjectsLighting[i+TIME_OF_DAY_FIRST][0].ambient.red);
			chunkWriter.writeReal(TheGlobalData->m_terrainObjectsLighting[i+TIME_OF_DAY_FIRST][0].ambient.green);
			chunkWriter.writeReal(TheGlobalData->m_terrainObjectsLighting[i+TIME_OF_DAY_FIRST][0].ambient.blue);
			chunkWriter.writeReal(TheGlobalData->m_terrainObjectsLighting[i+TIME_OF_DAY_FIRST][0].diffuse.red);
			chunkWriter.writeReal(TheGlobalData->m_terrainObjectsLighting[i+TIME_OF_DAY_FIRST][0].diffuse.green);
			chunkWriter.writeReal(TheGlobalData->m_terrainObjectsLighting[i+TIME_OF_DAY_FIRST][0].diffuse.blue);
			chunkWriter.writeReal(TheGlobalData->m_terrainObjectsLighting[i+TIME_OF_DAY_FIRST][0].lightPos.x);
			chunkWriter.writeReal(TheGlobalData->m_terrainObjectsLighting[i+TIME_OF_DAY_FIRST][0].lightPos.y);
			chunkWriter.writeReal(TheGlobalData->m_terrainObjectsLighting[i+TIME_OF_DAY_FIRST][0].lightPos.z);

			Int j=1;
			for (; j<MAX_GLOBAL_LIGHTS; j++)
			{	//save state of new lights added in version 3.
				chunkWriter.writeReal(TheGlobalData->m_terrainObjectsLighting[i+TIME_OF_DAY_FIRST][j].ambient.red);
				chunkWriter.writeReal(TheGlobalData->m_terrainObjectsLighting[i+TIME_OF_DAY_FIRST][j].ambient.green);
				chunkWriter.writeReal(TheGlobalData->m_terrainObjectsLighting[i+TIME_OF_DAY_FIRST][j].ambient.blue);
				chunkWriter.writeReal(TheGlobalData->m_terrainObjectsLighting[i+TIME_OF_DAY_FIRST][j].diffuse.red);
				chunkWriter.writeReal(TheGlobalData->m_terrainObjectsLighting[i+TIME_OF_DAY_FIRST][j].diffuse.green);
				chunkWriter.writeReal(TheGlobalData->m_terrainObjectsLighting[i+TIME_OF_DAY_FIRST][j].diffuse.blue);
				chunkWriter.writeReal(TheGlobalData->m_terrainObjectsLighting[i+TIME_OF_DAY_FIRST][j].lightPos.x);
				chunkWriter.writeReal(TheGlobalData->m_terrainObjectsLighting[i+TIME_OF_DAY_FIRST][j].lightPos.y);
				chunkWriter.writeReal(TheGlobalData->m_terrainObjectsLighting[i+TIME_OF_DAY_FIRST][j].lightPos.z);
			}
			for (j=1; j<MAX_GLOBAL_LIGHTS; j++)
			{
				chunkWriter.writeReal(TheGlobalData->m_terrainLighting[i+TIME_OF_DAY_FIRST][j].ambient.red);
				chunkWriter.writeReal(TheGlobalData->m_terrainLighting[i+TIME_OF_DAY_FIRST][j].ambient.green);
				chunkWriter.writeReal(TheGlobalData->m_terrainLighting[i+TIME_OF_DAY_FIRST][j].ambient.blue);
				chunkWriter.writeReal(TheGlobalData->m_terrainLighting[i+TIME_OF_DAY_FIRST][j].diffuse.red);
				chunkWriter.writeReal(TheGlobalData->m_terrainLighting[i+TIME_OF_DAY_FIRST][j].diffuse.green);
				chunkWriter.writeReal(TheGlobalData->m_terrainLighting[i+TIME_OF_DAY_FIRST][j].diffuse.blue);
				chunkWriter.writeReal(TheGlobalData->m_terrainLighting[i+TIME_OF_DAY_FIRST][j].lightPos.x);
				chunkWriter.writeReal(TheGlobalData->m_terrainLighting[i+TIME_OF_DAY_FIRST][j].lightPos.y);
				chunkWriter.writeReal(TheGlobalData->m_terrainLighting[i+TIME_OF_DAY_FIRST][j].lightPos.z);
			}
		}
 		chunkWriter.writeInt(TheW3DShadowManager->getShadowColor());

	chunkWriter.closeDataChunk();

#ifdef DEBUG_CRASHING
	for (i=0; i<m_dataSize; i++) {
		Int texNdx = this->m_tileNdxes[i];
		DEBUG_ASSERTCRASH((texNdx>>2) < m_numBitmapTiles,("oops"));
		Int texClass = getTextureClassFromNdx(texNdx);
		DEBUG_ASSERTCRASH(texClass>=0,("oops"));
	}
#endif
	
}

//
// duplicate - Makes a copy.
// Returns NULL if allocation failed.

WorldHeightMapEdit *WorldHeightMapEdit::duplicate(void)
{
	WorldHeightMapEdit *newMap = new WorldHeightMapEdit(this); 
	if (newMap->m_data == NULL) {
		delete newMap;
		return(NULL);
	}
	return(newMap);
}



Bool WorldHeightMapEdit::setTileNdx(Int xIndex, Int yIndex, Int textureClass, Bool singleTile) 
{ 
	Int ndx = (yIndex*m_width)+xIndex;
	Int numClasses = m_numTextureClasses;
	DEBUG_ASSERTCRASH(ndx>=0 && ndx<this->m_dataSize,("oops"));
	if (ndx<0 || ndx >= this->m_dataSize) return false;
	Int texNdx = getTileNdxForClass(xIndex, yIndex, textureClass);
	DEBUG_ASSERTCRASH((texNdx>>2)<m_numBitmapTiles,("oops"));
	m_tileNdxes[ndx] = texNdx;
	m_blendTileNdxes[ndx] = 0;  // opaque.
	m_extraBlendTileNdxes[ndx] = 0;	//opaque.
	m_cliffInfoNdxes[ndx] = 0;
	if (singleTile) {
		updateFlatCellForAdjacentCliffs(xIndex, yIndex, textureClass);
	}
	return (numClasses<m_numTextureClasses);
}
/* return index of globalTextureClass corresponding to texture holding this tile*/
Int WorldHeightMapEdit::getTextureClassFromNdx(Int tileNdx) 
{
	Int i;
	tileNdx = tileNdx>>2;
	for (i=0; i<m_numTextureClasses; i++) {
		if (m_textureClasses[i].firstTile<0) {
			continue;
		}
		// see if the blend tile is in a texture class, and get the right tile for xIndex, yIndex.
		if (tileNdx >= m_textureClasses[i].firstTile && 
			tileNdx < m_textureClasses[i].firstTile+m_textureClasses[i].numTiles) {
			return(m_textureClasses[i].globalTextureClass);
		}
	}
	return(-1);
}
/*Get index into globalTextureClasses*/
Int WorldHeightMapEdit::getTextureClass(Int xIndex, Int yIndex, Bool baseClass)
{
	Int ndx = (yIndex*m_width)+xIndex;
	DEBUG_ASSERTCRASH((ndx>=0 && ndx<this->m_dataSize),("oops"));
	if (ndx<0 || ndx >= this->m_dataSize) return(-1);
	Int textureNdx = m_tileNdxes[ndx];
	if (!baseClass && (m_blendTileNdxes[ndx] != 0 || m_extraBlendTileNdxes[ndx] != 0)) {
		return(-1);  // blended, so not of the original class.
	}
	return getTextureClassFromNdx(textureNdx);	//get globalTextureClass index
}
/*Get index of sub-tile that would show up here if it wasn't blended but tiled across*/
Int WorldHeightMapEdit::getBlendTileNdxForClass(Int xIndex, Int yIndex, Int textureClass)
{
	Int ndx = getTileNdxForClass(xIndex, yIndex, textureClass);
	return ndx;
}

Int WorldHeightMapEdit::getTileIndexFromTerrainType( TerrainType *terrain )
{

	// sanity
	if( terrain == NULL )
		return -1;

	// search the texture list for a matching texture filename
	for( Int i = 0;  i< NUM_TEXTURE_CLASSES; i++ )
		if( m_globalTextureClasses[ i ].terrainType == terrain )
			return i;

	// not found
	return -1;

}  // end getTileIndexFromTerrainType

Int WorldHeightMapEdit::allocateTiles(Int textureClass)
{
//	int tileNdx = 0;
	if (textureClass >= 0 && textureClass <m_numGlobalTextureClasses) {
		Int firstTile = getFirstTile(textureClass); 
		if (firstTile >= 0) return firstTile;
		// hasn't been copied into m_sourceTiles yet.
		if (this->m_numBitmapTiles + m_globalTextureClasses[textureClass].numTiles>NUM_SOURCE_TILES) {
			m_warnTooManyTex = true;
			return(-1);
		}
		Int i;
		m_textureClasses[m_numTextureClasses].globalTextureClass = textureClass;
		m_textureClasses[m_numTextureClasses].firstTile = m_numBitmapTiles;
		firstTile = m_numBitmapTiles;
		m_textureClasses[m_numTextureClasses].numTiles = m_globalTextureClasses[textureClass].numTiles;
		m_textureClasses[m_numTextureClasses].width = m_globalTextureClasses[textureClass].width;
		m_textureClasses[m_numTextureClasses].name = m_globalTextureClasses[textureClass].name;
		m_textureClasses[m_numTextureClasses].isBlendEdgeTile = m_globalTextureClasses[textureClass].isBlendEdgeTile;
		m_numTextureClasses++;
		REF_PTR_RELEASE(m_terrainTex); // need to update the texture.
		updateTileTexturePositions(NULL);
		for (i=0; i<m_numTextureClasses; i++) {
			if (m_textureClasses[i].positionInTexture.x == 0) {
				// Couldn't fit the tiles in the texture.
				m_warnTooManyTex = true;
				m_numTextureClasses--;
				return(-1);
			}
		}
		Int numTiles = m_globalTextureClasses[textureClass].numTiles;
		for (i=0; i<numTiles; i++) {
			REF_PTR_SET(m_sourceTiles[m_numBitmapTiles+i],m_globalTextureClasses[textureClass].tiles[i]); 
		}
		m_numBitmapTiles += numTiles;
		return firstTile;
	}

	return -1;
}

Int WorldHeightMapEdit::allocateEdgeTiles(Int globalTextureClass)
{
	if (globalTextureClass >= 0 && globalTextureClass <m_numGlobalTextureClasses) {
		Int localClass;
		Int firstTile = -1; 
		for (localClass=0; localClass<m_numEdgeTextureClasses; localClass++) {
			Int globalClass = m_edgeTextureClasses[localClass].globalTextureClass;
			if (globalClass == globalTextureClass) {
				return localClass;
			}
		}
		DEBUG_ASSERTCRASH(m_globalTextureClasses[globalTextureClass].isBlendEdgeTile, ("Shouldn't use this for edge tiles."));
		// hasn't been copied into m_sourceTiles yet.
		if (m_numEdgeTiles + m_globalTextureClasses[globalTextureClass].numTiles>NUM_SOURCE_TILES) {
			m_warnTooManyTex = true;
			return(-1);
		}
		Int i;
		localClass = m_numEdgeTextureClasses;
		m_edgeTextureClasses[m_numEdgeTextureClasses].globalTextureClass = globalTextureClass;
		m_edgeTextureClasses[m_numEdgeTextureClasses].firstTile = m_numEdgeTiles;
		firstTile = m_numBitmapTiles;
		m_edgeTextureClasses[m_numEdgeTextureClasses].numTiles = m_globalTextureClasses[globalTextureClass].numTiles;
		m_edgeTextureClasses[m_numEdgeTextureClasses].width = m_globalTextureClasses[globalTextureClass].width;
		m_edgeTextureClasses[m_numEdgeTextureClasses].name = m_globalTextureClasses[globalTextureClass].name;
		m_edgeTextureClasses[m_numEdgeTextureClasses].isBlendEdgeTile = m_globalTextureClasses[globalTextureClass].isBlendEdgeTile;
		m_numEdgeTextureClasses++;
		REF_PTR_RELEASE(m_terrainTex); // need to update the texture.
		updateTileTexturePositions(NULL);
		for (i=0; i<m_numEdgeTextureClasses; i++) {
			if (m_edgeTextureClasses[i].positionInTexture.x == 0) {
				// Couldn't fit the tiles in the texture.
				m_warnTooManyTex = true;
				m_numEdgeTextureClasses--;
				return(-1);
			}
		}
		Int numTiles = m_globalTextureClasses[globalTextureClass].numTiles;
		for (i=0; i<numTiles; i++) {
			REF_PTR_SET(m_edgeTiles[m_numEdgeTiles+i],m_globalTextureClasses[globalTextureClass].tiles[i]); 
		}
		m_numEdgeTiles += numTiles;
		return localClass;

	}
	return -1;
}

/*textureClass is globalTextureClasses index.
 returns index to subtile determined by cell's terrain coordinates*/
Int WorldHeightMapEdit::getTileNdxForClass(Int xIndex, Int yIndex, Int textureClass)
{
	int tileNdx = 0;
	if (textureClass >= 0 && textureClass <m_numGlobalTextureClasses) {
		Int firstTile = getFirstTile(textureClass); //search used TextureClasses for one with the same globalTextureIndex 
		DEBUG_ASSERTCRASH(!m_globalTextureClasses[textureClass].isBlendEdgeTile, ("Shouldn't use blend edge tiles for tiling."));
		if (firstTile == -1) {
			firstTile = allocateTiles(textureClass);
		}
		if (firstTile<0) return 0;

		Int width = m_globalTextureClasses[textureClass].width; 
		tileNdx = firstTile + ((xIndex/2)%width) + 
			width*((yIndex/2)%width);	//2 terrain cells across tile so divide by 2
		/* there are actually 4 subcells in a tile.  So be funky. :) */
		tileNdx = tileNdx << 2;
		Int ySubIndex = yIndex&0x01;
		Int xSubIndex = xIndex&0x01;;
		tileNdx += 2*ySubIndex;
		tileNdx += xSubIndex;
	}
	return tileNdx;
}



/** Returns true if this texture class is already present, or can be added to the map.
*/
Bool WorldHeightMapEdit::canFitTexture(Int textureClass)
{
	if (textureClass >= 0 && textureClass <m_numGlobalTextureClasses) {
		Int firstTile = getFirstTile(textureClass); 
		if (firstTile >= 0) return true; // already present.
		if (this->m_numBitmapTiles + m_globalTextureClasses[textureClass].numTiles>NUM_SOURCE_TILES) {
			// No room at the inn.
			return(false);
		}
		Int i;
		m_textureClasses[m_numTextureClasses].globalTextureClass = textureClass;
		m_textureClasses[m_numTextureClasses].firstTile = 0;
		m_textureClasses[m_numTextureClasses].numTiles = m_globalTextureClasses[textureClass].numTiles;
		m_textureClasses[m_numTextureClasses].width = m_globalTextureClasses[textureClass].width;
		m_numTextureClasses++;

		updateTileTexturePositions(NULL);
		m_numTextureClasses--;
		for (i=0; i<m_numTextureClasses+1; i++) {
			if (m_textureClasses[i].positionInTexture.x == 0) {
				// Couldn't fit the tiles in the texture.
				return false;
			}
		}
		updateTileTexturePositions(NULL);
		return true;
	}
	return false;
}


void WorldHeightMapEdit::blendTile(Int xIndex, Int yIndex, Int srcXIndex, Int srcYIndex, Int textureClass, Int edgeClass) 
{
	Int ndx = (srcYIndex*m_width)+srcXIndex;
	Int blendTileNdx = m_tileNdxes[ndx];	//source tile

	ndx = (yIndex*m_width)+xIndex;			
	Int curTileNdx = m_tileNdxes[ndx];		//destination tile

	if (textureClass < 0) {
		textureClass = getTextureClass(srcXIndex, srcYIndex);	//index into globalTextureClass
	}
	if (textureClass >= 0) {	//get index of sub-tile that would show up here if we continued tiling src.
		blendTileNdx = getBlendTileNdxForClass(xIndex, yIndex, textureClass);
		DEBUG_ASSERTCRASH((blendTileNdx/4 < m_numBitmapTiles),("oops"));	//check it falls into one of the 64x64 tiles of textures used on map.
	}

	if (curTileNdx == blendTileNdx) {//destination already contains continuation of source tile so no blend needed.
		m_tileNdxes[ndx] = blendTileNdx;	
		m_blendTileNdxes[ndx] = 0;
		m_extraBlendTileNdxes[ndx] = 0;
		return;
	}
	//Updates m_blendTileNdxes[] for this coordinate with index into m_blendedTiles[]
	blendSpecificTiles(xIndex, yIndex, srcXIndex, srcYIndex, curTileNdx, blendTileNdx, false, edgeClass); 
}

/*Allocate a new blend description or return existing one out of m_blendedTiles[] and increment m_numBlendedTiles*/
Int WorldHeightMapEdit::findOrCreateBlendTile(TBlendTileInfo *pBlendInfo)
{
	TileData *pBlendTile;
	Short sourceNdx = pBlendInfo->blendNdx;
	sourceNdx = sourceNdx>>2;
	pBlendTile = m_sourceTiles[sourceNdx];
	if (pBlendTile == NULL) {
		return(-1);
	}
	// Make a quick scan through the blended tiles, and see if we already got this one.
	int k;
	for (k=1; k<m_numBlendedTiles; k++) {
		TBlendTileInfo *curInfo = &m_blendedTiles[k];
		#define MATCH(x) (curInfo->x==pBlendInfo->x)
		if (MATCH(blendNdx) && MATCH(horiz) && MATCH(vert) && MATCH(rightDiagonal) &&
				MATCH(leftDiagonal) && MATCH(longDiagonal) && MATCH(inverted) && MATCH(customBlendEdgeClass)) {
			// got a match.
			return k;
		}
		#undef MATCH
	}

	if (m_numBlendedTiles>=NUM_BLEND_TILES) {
		m_warnTooManyBlend = true;
		return -1;  // out of new tiles
	}
	Short newNdx = m_numBlendedTiles;
	m_blendedTiles[newNdx] = *pBlendInfo;
	m_numBlendedTiles++;

	return(newNdx);

}
/*Blend blendTileNdx over curTileNdx. Blend from srcIndex to Index coordinates.
Updates m_blendTileNdxes[] for this coordinate with index into m_blendedTiles[]*/
void WorldHeightMapEdit::blendSpecificTiles(Int xIndex, Int yIndex, Int srcXIndex, Int srcYIndex,
									Int curTileNdx, Int blendTileNdx, Bool longDiagonal, Int edgeClass) 
{
	TBlendTileInfo blendInfo;
	blendInfo.blendNdx=blendTileNdx;	//store the tile that will blend over existing tile
	blendInfo.horiz = false;
	blendInfo.vert = false;
	blendInfo.rightDiagonal = false;
	blendInfo.leftDiagonal = false;
	blendInfo.inverted = false;
	blendInfo.longDiagonal = longDiagonal;
	blendInfo.customBlendEdgeClass = edgeClass;

	//Check if there is already a blend tile at the destination and record its flip state.
	//We need to know this so that we don't accidently apply a third blend layer with
	//a different flip and introduce z-fighting over this tile.
	Bool baseNeedsFlip = false;
	UnsignedByte baseIsDiagonal = 0;
	Int ndx = (yIndex*m_width)+xIndex;
	TBlendTileInfo *baseBlendInfo=NULL;
	if (TheGlobalData->m_use3WayTerrainBlends && m_blendTileNdxes[ndx] != 0)
	{	baseBlendInfo=&m_blendedTiles[m_blendTileNdxes[ndx]];
		//Figure out if this tile will eventually need flipping when rendered
		baseIsDiagonal = baseBlendInfo->rightDiagonal || baseBlendInfo->leftDiagonal;
		if (baseBlendInfo->rightDiagonal && !baseBlendInfo->inverted)
			baseNeedsFlip = true;
		else
		if (baseBlendInfo->leftDiagonal && baseBlendInfo->inverted)
			baseNeedsFlip = true;
	}

	Bool flipped = false;
	if (srcYIndex == yIndex) {	//same vertical coordinates so horizontal blend
		blendInfo.horiz = true;
		blendInfo.inverted = (srcXIndex < xIndex); //blend from opaque at left to transparent at right.
		//check if this blend tile will end up in a third blend layer and if so, copy flip state
		//from base layer.
		if (baseBlendInfo && baseNeedsFlip)
			blendInfo.inverted |= FLIPPED_MASK;
	} else if (srcXIndex == xIndex) {
		blendInfo.vert = true;	//same horizontal coordinates so vertical blend
		blendInfo.inverted = (srcYIndex < yIndex);	//invert if blend from bottom to top
		if (baseBlendInfo && baseNeedsFlip)
			blendInfo.inverted |= FLIPPED_MASK;
	} else {	//diagonal blends
		if (srcXIndex > xIndex) {
			blendInfo.rightDiagonal = true;
		} else {
			blendInfo.leftDiagonal = true;
		}
		blendInfo.inverted = (srcYIndex < yIndex);
		blendInfo.longDiagonal = longDiagonal;
		if (longDiagonal) {	// Flip it.
			blendInfo.inverted = !blendInfo.inverted;
			blendInfo.rightDiagonal = !blendInfo.rightDiagonal;
			blendInfo.leftDiagonal = !blendInfo.leftDiagonal;
		}
		//Figure out if this tile will eventually need flipping when rendered
		if (blendInfo.rightDiagonal && !blendInfo.inverted)
			flipped = true;
		else
		if (blendInfo.leftDiagonal && blendInfo.inverted)
			flipped = true;
		//Check if this tile will end up in 3rd layer and if so, make sure it can match flip of base layer.
		if (baseBlendInfo && baseIsDiagonal && baseNeedsFlip != flipped)
			return;	//the base requires a certain flip state which we can't alter so can't apply extra blend here.
	}
	updateFlatCellForAdjacentCliffs(xIndex, yIndex, getTextureClassFromNdx(blendTileNdx));

	//Check if this tile is really necessary or if the base blend already does the same thing as the 3way blend.
	if (baseBlendInfo && baseBlendInfo->blendNdx == blendTileNdx)
		return;

	Short newNdx = findOrCreateBlendTile(&blendInfo);
	if (newNdx >= 0) {
		Int ndx = (yIndex*m_width)+xIndex;
		m_tileNdxes[ndx] = curTileNdx;
		if (TheGlobalData->m_use3WayTerrainBlends && m_blendTileNdxes[ndx] != 0)
		{	//this tile already has a blend applied to it.  So we put the new blend into the
			//secondary layer.
			m_extraBlendTileNdxes[ndx]=newNdx;
			//force the primary layer to flip if the extra blend layer needs flip.
			//we only do this on vertical/horizontal base blends because they work in either flip cases.
			if (flipped && !baseIsDiagonal)
			{	//Find a new tile so as not to affect other cells using the base one.
				TBlendTileInfo tempBlendTileInfo=m_blendedTiles[m_blendTileNdxes[ndx]];
				tempBlendTileInfo.inverted |= FLIPPED_MASK;
				Short newNdx = findOrCreateBlendTile(&tempBlendTileInfo);
				m_blendTileNdxes[ndx] = newNdx;	//remap this tile to use a new one. 
			}
		}
		else
			m_blendTileNdxes[ndx] = newNdx;
	}
}



/******************************************************************
	autoBlendOut
		auto edges the texture region at xIndex, yIndex outwards.
		The region is defined by the texture at xIndex, yIndex.
		The edges are blended out, to whatever texture is already there.
		If edgeClass == -1, use an alpha blend.  Otherwise, use tile class edgeClass.
*/
void WorldHeightMapEdit::autoBlendOut(Int xIndex, Int yIndex, Int globalEdgeClass) 
{
	Int ndx = (yIndex*m_width)+xIndex;
	Int curTileClass = getTextureClass(xIndex, yIndex);
	if (curTileClass < 0) {
		return;
	}
	Int localEdgeClass=-1;
	static Bool foo = false;
	if (foo) {
		if (globalEdgeClass<0) {
			for (globalEdgeClass=m_numGlobalTextureClasses-1; globalEdgeClass>=0; globalEdgeClass--) {
				if (m_globalTextureClasses[globalEdgeClass].isBlendEdgeTile) {
					break;
				}
			}
		}
	}	
	if (globalEdgeClass>=0) {
		localEdgeClass = allocateEdgeTiles(globalEdgeClass); 
	}

	Int i,j;
	UnsignedByte *pProcessed = new UnsignedByte[m_dataSize];
	for (i=0; i<m_dataSize; i++) {
		pProcessed[i] = false;
	}
	if (pProcessed == NULL) {
		AfxMessageBox(IDS_OUT_OF_MEMORY);
		return;
	}

	CProcessNode *pNodesToProcess = NULL;
	CProcessNode *pProcessedNodes = NULL;
	Int nodesProcessed = 0;
	// Find all the nodes that are in the current tile class.
	pNodesToProcess = new CProcessNode(xIndex, yIndex);
	pProcessed[ndx] = true;
	while (pNodesToProcess) {
		CProcessNode *pCurNode = pNodesToProcess;
		pNodesToProcess = pCurNode->m_next;
		pCurNode->m_next = NULL;
		Int curNdx = (pCurNode->m_y*m_width)+pCurNode->m_x;
		for (i=pCurNode->m_x-1; i<pCurNode->m_x+2; i++) {
			if (i<0) continue;
			if (i>=m_width) continue;
			for (j=pCurNode->m_y-1; j<pCurNode->m_y+2; j++) {
				if (j<0) continue;
				if (j>=m_height) continue;
				curNdx = (j*m_width)+i;
				if (pProcessed[curNdx]) {
					continue;
				}
				if (curTileClass != getTextureClass(i,j, true)) {
					// This is a not us tile.  If it is mostly surrounded by us, 
					// convert it to us, cause if we blend in from 3 sides, it basically
					// obliterates it.
					Int sides, total;
					getTexClassNeighbors(i, j, curTileClass, &sides, &total);
					if (sides>2 || total>5) {
						m_tileNdxes[curNdx] = getTileNdxForClass(i, j, curTileClass);
						m_blendTileNdxes[curNdx] = 0; // no blend.
					} else {
						continue;
					}
				}	else {
					// If we are already blended, skip it.
					if (m_blendTileNdxes[curNdx] > 0) continue;
				}
				CProcessNode *pNewNode = new CProcessNode(i,j);
				pNewNode->m_next = pNodesToProcess;
				pNodesToProcess = pNewNode;
				pProcessed[curNdx] = true; // mark self as processed.
			}
		}
		Int sameSides, sameTotal;
		getTexClassNeighbors(pCurNode->m_x, pCurNode->m_y, curTileClass, &sameSides, &sameTotal);
		if (sameTotal==8) {
			// I am surrounded by my fellow tiles, so I don't need to do any blending.
			delete pCurNode;
		} else {
			// I have some edges with other tiles that will need blending.
			pCurNode->m_next = pProcessedNodes;
			pProcessedNodes = pCurNode;
		}
		nodesProcessed++;
	}

	// Reinit the processed flags;
	for (i=0; i<m_dataSize; i++) {
		pProcessed[i] = false;
	}

	pNodesToProcess = pProcessedNodes;
	pProcessedNodes = NULL;
	while (pNodesToProcess) {
		CProcessNode *pCurNode = pNodesToProcess;
		pNodesToProcess = pCurNode->m_next;
		pCurNode->m_next = NULL;
		Int curNdx = (pCurNode->m_y*m_width)+pCurNode->m_x;
		for (i=pCurNode->m_x-1; i<pCurNode->m_x+2; i++) {
			if (i<0) continue;
			if (i>=m_width) continue;
			for (j=pCurNode->m_y-1; j<pCurNode->m_y+2; j++) {
				if (j<0) continue;
				if (j>=m_height) continue;
//				int thisTexClass = 
				curNdx = (j*m_width)+i;
				if (pProcessed[curNdx]) {
					continue;
				}
				// If already blended to us skip it.
				if (m_blendTileNdxes[curNdx] > 0) {					 
					if (curTileClass == getTextureClassFromNdx(m_blendedTiles[ m_blendTileNdxes[curNdx]].blendNdx)) {					
						continue;
					}
				}
				if (curTileClass != getTextureClass(i,j, true)) {
					// blend this tile.
					blendToThisClass(i,j,curTileClass, localEdgeClass);
				}
				pProcessed[curNdx] = true; // mark self as processed.
			}
		}
		delete pCurNode;
	}

	if (pProcessed) delete pProcessed;
	pProcessed = NULL;
}

/******************************************************************
	blendToThisClass
		blends the texture at xIndex, yIndex to textureClass.
		Used by auto edging.  The texture at xIndex, yIndex is being
		edged into by surrounding textures of textureClass.
*/
void WorldHeightMapEdit::blendToThisClass(Int xIndex, Int yIndex, 
																					Int textureClass, Int edgeClass) 
{
	Int sides, total;
	getTexClassNeighbors(xIndex, yIndex, textureClass, &sides, &total);
	DEBUG_ASSERTCRASH((total>0),("oops"));  // if no neighbors, should not happen.
	DEBUG_ASSERTCRASH((sides<3),("oops"));  // Should have been squished out earlier.
	if (total<1) return;
	Int i,j;
//	Bool longDiagonal = false;
	if (sides==0) {
		// corner blend.
		for (i=xIndex-1; i<xIndex+2; i++) {
			if (i<0) continue;
			if (i>=m_width) continue;
			for (j=yIndex-1; j<yIndex+2; j++) {
				if (j<0) continue;
				if (j>=m_height) continue;
				if (i==xIndex && j==yIndex) continue;  // don't do self.
				Int curClass = this->getTextureClass(i, j);
				if (curClass == textureClass) {
					blendTile(xIndex, yIndex, i, j, -1, edgeClass);
					return; // done!
				}
			}
		}

	} else if (sides==1) {
		// slide blend.
		for (i=xIndex-1; i<xIndex+2; i++) {
			if (i<0) continue;
			if (i>=m_width) continue;
			for (j=yIndex-1; j<yIndex+2; j++) {
				if (i!=xIndex && j!=yIndex) {
					continue;  // skip corners.
				}
				if (j<0) continue;
				if (j>=m_height) continue;
				if (i==xIndex && j==yIndex) continue;  // don't do self.
				Int curClass = this->getTextureClass(i, j);
				if (curClass == textureClass) {
					blendTile(xIndex, yIndex, i, j, -1, edgeClass);
					return; // done!
				}
			}
		}

	} else if (sides==2) {
		// corner funny blend.
		Bool top=false;
		Bool bottom=false;
		Bool left=false;
		Bool right=false;
		for (i=xIndex-1; i<xIndex+2; i++) {
			if (i<0) continue;
			if (i>=m_width) continue;
			for (j=yIndex-1; j<yIndex+2; j++) {
				if (i!=xIndex && j!=yIndex) {
					continue;  // skip corners.
				}
				if (j<0) continue;
				if (j>=m_height) continue;
				if (i==xIndex && j==yIndex) continue;  // don't do self.
				Int curClass = this->getTextureClass(i, j);
				if (curClass == textureClass) {
					if (i==xIndex) {
						if (j>yIndex) top=true;
						else bottom=true;
					} else {
						if (i<xIndex) {
							left = true;
						} else {
							right = true;
						}
					}
				}
			}
		}	

		Int blendTileNdx = getBlendTileNdxForClass(xIndex, yIndex, textureClass);
		Int ndx = (yIndex*m_width)+xIndex;
		Int curTileNdx = m_tileNdxes[ndx];
		// Note - we are doing a long diagonal blend
		// on purpose.  jba.
		Int srcXIndex = xIndex;
		Int srcYIndex = yIndex;
		if (top) srcYIndex--;
		if (bottom) srcYIndex++;
		if (left) srcXIndex++;
		if (right) srcXIndex--;

		blendSpecificTiles(xIndex, yIndex, srcXIndex, srcYIndex,
									curTileNdx, blendTileNdx, true, edgeClass);
	}
}

/******************************************************************
	floodFill
		Fills the region at xIndex, yIndex with the specified texture. DoReplace forces texture to
		be replaced on entire map.
*/
Bool WorldHeightMapEdit::floodFill(Int xIndex, Int yIndex, Int textureClass, Bool doReplace) 
{
	Int ndx = (yIndex*m_width)+xIndex;
	Int curTileClass = getTextureClass(xIndex, yIndex, true);
	if (curTileClass < 0) {
		return(false);
	}
	
	if (curTileClass == textureClass) {
		// already filled with this texture.
		return(false);
	}
	Int i;
	Bool textureExistsInMap = false;
	for (i=0; i<m_numTextureClasses; i++) {
		if (m_textureClasses[i].globalTextureClass==textureClass) {
			textureExistsInMap = true;
		}
	}
	if (!canFitTexture(textureClass) || doReplace) {
		CString confirm;
		confirm.Format(IDS_CONFIRM_REPLACE_TEXTURE, m_globalTextureClasses[curTileClass].name.str());
		Int msg = ::AfxMessageBox(confirm, MB_YESNO);
		::AfxGetMainWnd()->SetFocus();
		if (msg == IDNO) {
			return false;
		}
		doReplace = true;
	}

	if (!textureExistsInMap && doReplace) {
		if (m_globalTextureClasses[textureClass].numTiles <= m_globalTextureClasses[curTileClass].numTiles) {
			Int i;
			Int localClass = -1;
			for (i=0; i<m_numTextureClasses; i++) {
				if (m_textureClasses[i].globalTextureClass == curTileClass) {
					localClass = i;
					break;
				}
			}
			if (localClass<0) return false;
			m_textureClasses[localClass].globalTextureClass = textureClass;
			m_textureClasses[localClass].width = m_globalTextureClasses[textureClass].width;
			m_textureClasses[localClass].name = m_globalTextureClasses[textureClass].name;
			m_textureClasses[localClass].isBlendEdgeTile = m_globalTextureClasses[textureClass].isBlendEdgeTile;
			REF_PTR_RELEASE(m_terrainTex); // need to update the texture.
			Int numTiles = m_globalTextureClasses[textureClass].numTiles;
			for (i=0; i<numTiles; i++) {
				REF_PTR_SET(m_sourceTiles[m_textureClasses[localClass].firstTile+i],m_globalTextureClasses[textureClass].tiles[i]); 
			}
			optimizeTiles();
			return true;
		}
		return false;
	}
	Int j;
	if (doReplace) {
		for (i=0; i<m_width; i++) {
			for (j=0; j<m_height; j++) {
 				ndx = (j*m_width)+i;
				Int blendNdx = m_blendTileNdxes[ndx];
				if (curTileClass == getTextureClass(i,j,true)) {
					setTileNdx(i, j, textureClass, false);
					m_blendTileNdxes[ndx] = blendNdx;
					m_cliffInfoNdxes[ndx] = 0; // remove any cliff adjustment as we are doing a new texture.
				} else {	//adjust blended tiles so the blend texture matches the new filled texture
					/* Check blend */
					if (blendNdx == 0) continue; // no blend.
					TBlendTileInfo blendInfo = m_blendedTiles[blendNdx];
					if (curTileClass == this->getTextureClassFromNdx(blendInfo.blendNdx)) {
						blendInfo.blendNdx = getTileNdxForClass(i, j, textureClass);
						blendNdx = findOrCreateBlendTile(&blendInfo);
						m_blendTileNdxes[ndx] = blendNdx;
						m_cliffInfoNdxes[ndx] = 0;
					}
					continue;
				}
			}
		}
	}	else {
		CProcessNode *pNodesToProcess = NULL;
		Int nodesProcessed = 0;
		pNodesToProcess = new CProcessNode(xIndex, yIndex);
		while (pNodesToProcess) {
 			CProcessNode *pCurNode = pNodesToProcess;
			pNodesToProcess = pCurNode->m_next;
			pCurNode->m_next = NULL;
			Int ndx = (pCurNode->m_y*m_width)+pCurNode->m_x;
			Int blendNdx = m_blendTileNdxes[ndx];
			setTileNdx(pCurNode->m_x, pCurNode->m_y, textureClass, false);
			m_blendTileNdxes[ndx] = blendNdx;
			m_cliffInfoNdxes[ndx] = 0; // remove any cliff adjustment as we are doing a new texture.
			for (i=pCurNode->m_x-1; i<pCurNode->m_x+2; i++) {
				if (i<0) continue;
				if (i>=m_width-1) continue;
				for (j=pCurNode->m_y-1; j<pCurNode->m_y+2; j++) {
					if (j<0) continue;
					if (j>=m_height-1) continue;	 
 					ndx = (j*m_width)+i;
					blendNdx = m_blendTileNdxes[ndx];
					if (curTileClass != getTextureClass(i,j,true)) {
						/* Check blend */
						if (blendNdx == 0) continue; // no blend.
						TBlendTileInfo blendInfo = m_blendedTiles[blendNdx];
						if (curTileClass == this->getTextureClassFromNdx(blendInfo.blendNdx)) {
							blendInfo.blendNdx = getTileNdxForClass(i, j, textureClass);
							blendNdx = findOrCreateBlendTile(&blendInfo);
							m_blendTileNdxes[ndx] = blendNdx;
							m_cliffInfoNdxes[ndx] = 0;
						}
						continue;
					}
					if (i!=pCurNode->m_x && j!=pCurNode->m_y) {
						continue;  // skip corners.
					}
					if (curTileClass == getTextureClass(i,j,true)) {
						// Matches the current fill, so flood into it, 
						CProcessNode *pNewNode = new CProcessNode(i,j);
						pNewNode->m_next = pNodesToProcess;
						pNodesToProcess = pNewNode;
					}
				}
			}
			delete pCurNode;
			nodesProcessed++;
		}

	}

	REF_PTR_RELEASE(m_terrainTex);
	REF_PTR_RELEASE(m_alphaEdgeTex);
	return(true);
}


/******************************************************************
	resetResources
		releases textures so things like device reset can be done.
*/
void WorldHeightMapEdit::resetResources(void)
{
	REF_PTR_RELEASE(m_terrainTex);
	REF_PTR_RELEASE(m_alphaEdgeTex);
}

/******************************************************************
	reloadTextures
		reloads textures from files.
*/
void WorldHeightMapEdit::reloadTextures(void)
{
	Int i;
	for (i=0; i<m_numGlobalTextureClasses; i++) {
		if (m_globalTextureClasses[i].numTiles == 0) continue;
		CachedFileInputStream stream;
		if (!stream.open(m_globalTextureClasses[i].filePath)) {
			continue;
		}
		Int numTiles = WorldHeightMap::countTiles(&stream);
		if (numTiles < m_globalTextureClasses[i].numTiles) {
			continue;
		}
		stream.rewind();
		/*Bool ok =*/ readTiles(&stream, m_globalTextureClasses[i].tiles, m_globalTextureClasses[i].width);
		stream.close();
	}
	if (TheTerrainRenderObject) {
		TheTerrainRenderObject->ReleaseResources();
		TheTerrainRenderObject->ReAcquireResources();
	}
	REF_PTR_RELEASE(m_terrainTex);
	REF_PTR_RELEASE(m_alphaTerrainTex);
	REF_PTR_RELEASE(m_alphaEdgeTex);
}

/******************************************************************
	showTileStatusInfo
		provides human readable tile statistics.
*/
void WorldHeightMapEdit::showTileStatusInfo(void)
{
	CString message;
	Int tilesPerRow = TEXTURE_WIDTH/(2*TILE_PIXEL_EXTENT+TILE_OFFSET);
	Int availableTiles = 4 * tilesPerRow * tilesPerRow;
	Int availableBlends = NUM_BLEND_TILES;

	CString tmp;
	tmp.Format("Base tiles used %d of %d (%d%%)\n", m_numBitmapTiles, availableTiles,
						(m_numBitmapTiles*100+50)/availableTiles);
	message += tmp;
	tmp.Format("Blend tiles used %d of %d (%d%%)\n", m_numBlendedTiles, availableBlends,
						(m_numBlendedTiles*100+50)/availableBlends);
	message += tmp;
	::AfxMessageBox(message);
}


/******************************************************************
	setHeight
		This sets the height, and adjusts the cliff flag for the cells affected.
*/
void WorldHeightMapEdit::setHeight(Int xIndex, Int yIndex, UnsignedByte height) { 
		Int ndx = (yIndex*m_width)+xIndex;
		if ((ndx>=0) && (ndx<m_dataSize) && m_data) m_data[ndx]=height;
		setCellCliffFlagFromHeights(xIndex, yIndex);
		setCellCliffFlagFromHeights(xIndex-1, yIndex);
		setCellCliffFlagFromHeights(xIndex, yIndex-1);
		setCellCliffFlagFromHeights(xIndex-1, yIndex-1);
}


/******************************************************************
	optimizeTiles
		This optimizes the tiles and blend tiles, recalculating them 
		and removing any unused ones.
*/
Bool WorldHeightMapEdit::optimizeTiles(void)
{
	// Run through all the tile indexes changing to tile classes.
	Int i;
	for (i=0; i<m_dataSize; i++) {
		Int texNdx = this->m_tileNdxes[i];
		Int texClass = getTextureClassFromNdx(texNdx);
		DEBUG_ASSERTCRASH((texClass>=0),("oops"));
		if (texClass<0) texClass=0;
		m_tileNdxes[i] = texClass;
	}

	// Run through all the blend tiles changing tile index to class.
	TBlendTileInfo blendInfo[NUM_BLEND_TILES];
	for (i=1; i<m_numBlendedTiles; i++) {
		blendInfo[i] = m_blendedTiles[i];
		blendInfo[i].blendNdx = getTextureClassFromNdx(blendInfo[i].blendNdx);
		DEBUG_ASSERTCRASH((blendInfo[i].blendNdx>=0),("oops"));
		if (blendInfo[i].blendNdx<0) blendInfo[i].blendNdx=0;
	}

	// Run through all the blend tiles changing tile index to class.
	for (i=1; i<m_numCliffInfo; i++) {
		Int texClass  = getTextureClassFromNdx(m_cliffInfo[i].tileIndex);
		if (texClass<0) texClass=0;
		m_cliffInfo[i].tileIndex = texClass;
	}


	// Release all the tiles.
	for (i=0; i<NUM_SOURCE_TILES; i++) {
		REF_PTR_RELEASE(m_sourceTiles[i]);
	}
	m_numBitmapTiles = 0;
	m_numBlendedTiles = 1; // blend 0 is the no blend tile.
	// Now adjust all the texture classes.
	m_numTextureClasses = 0;
	// Reallocate the textures and blends.
	Int x, y;
	for (x=0; x<m_width; x++) {
		for (y=0; y<m_height; y++) {
			i = y*m_width+x;
			Int texClass = m_tileNdxes[i];
			m_tileNdxes[i] = getTileNdxForClass(x,y,texClass);

			//Optimize blend descriptions to remove redundant ones.
			Int blendNdx = m_blendTileNdxes[i];
			Int extraBlendNdx =	m_extraBlendTileNdxes[i];
			int newBlendNdx;
			if (blendNdx == 0) {
				newBlendNdx = 0;
			} else {
				TBlendTileInfo curBlendInfo = blendInfo[blendNdx];
				//remove any forced flips due to 3-way blending if there is no 3-way blend.
				if (!extraBlendNdx)
					curBlendInfo.inverted &= ~FLIPPED_MASK;

				curBlendInfo.blendNdx = getBlendTileNdxForClass(x,y,curBlendInfo.blendNdx);

				if (curBlendInfo.blendNdx == m_tileNdxes[i])
				{	//Tile index same as blend index would mean same texture
					//blending into itself.  Should not happen.
					newBlendNdx = 0;
				}
				else
				{	newBlendNdx = findOrCreateBlendTile(&curBlendInfo);
					if (m_numBlendedTiles < NUM_BLEND_TILES) {
						DEBUG_ASSERTCRASH((newBlendNdx>0),("oops"));
					}
					if (newBlendNdx < 0) newBlendNdx = 0;
				}
			}
			m_blendTileNdxes[i] = blendNdx = newBlendNdx;

			//Optimize blend descriptions to remove redundant ones.
			if (extraBlendNdx == 0) {
				newBlendNdx = 0;
			} else {
				TBlendTileInfo curBlendInfo = blendInfo[extraBlendNdx];
				curBlendInfo.blendNdx = getBlendTileNdxForClass(x,y,curBlendInfo.blendNdx);
				//only allow 3-way blend if base already blended and if base
				//does not already contain the same texture
				if (!blendNdx || curBlendInfo.blendNdx == m_tileNdxes[i] ||
					m_blendedTiles[blendNdx].blendNdx == curBlendInfo.blendNdx)
					newBlendNdx = 0;
				else
				{	newBlendNdx = findOrCreateBlendTile(&curBlendInfo);
					if (m_numBlendedTiles < NUM_BLEND_TILES) {
						DEBUG_ASSERTCRASH((newBlendNdx>0),("oops"));
					}
					if (newBlendNdx < 0) newBlendNdx = 0;
				}
			}
			m_extraBlendTileNdxes[i] = newBlendNdx;

		}
	}		
	// Run through all the blend tiles changing tile index to class.
	for (i=1; i<m_numCliffInfo; i++) {
		Int texClass  = m_cliffInfo[i].tileIndex;
		m_cliffInfo[i].tileIndex = getTileNdxForClass(x,y,texClass);
	}

	REF_PTR_RELEASE(m_terrainTex);
	REF_PTR_RELEASE(m_terrainTex);
	REF_PTR_RELEASE(m_alphaEdgeTex);
	return(true);
}

 

/** ****************************************************************
	resize
		Changes the size of the height map.
*/
Bool WorldHeightMapEdit::resize(Int newXSize, Int newYSize, Int newHeight, Int newBorder, Bool anchorTop, Bool anchorBottom, 
							Bool anchorLeft, Bool anchorRight, Coord3D *pObjOffset)
{
	if (newBorder<0) newBorder = 0;
	newXSize += 2*newBorder;
	newYSize += 2*newBorder;
	Int newDataSize = newXSize*newYSize;
	if (newDataSize<=0) {
		AfxMessageBox(IDS_BAD_VALUE);
		return(false);
	}
	pObjOffset->x = 0;
	pObjOffset->y = 0;
	pObjOffset->z = 0;
	Int xOffset = m_borderSize-newBorder;
	Int sizeChange = (m_width-2*m_borderSize) - (newXSize - 2*newBorder);
	if (anchorLeft) {
		// Nothing - same xOffset.
	} else if (anchorRight) {
		xOffset += sizeChange;
		pObjOffset->x = -sizeChange*MAP_XY_FACTOR;
	} else {
		xOffset += (sizeChange)/2;
		pObjOffset->x = -((sizeChange)/2)*MAP_XY_FACTOR;
	}

	Int yOffset = m_borderSize - newBorder;
	sizeChange = (m_height-2*m_borderSize) - (newYSize - 2*newBorder);
	if (anchorBottom) {
		// Nothing - same yOffset.
	} else if (anchorTop) {
		yOffset += sizeChange;
		pObjOffset->y = -sizeChange*MAP_XY_FACTOR;
	} else {
		yOffset += (sizeChange)/2;
		pObjOffset->y = -((sizeChange)/2)*MAP_XY_FACTOR;
	}


	Short *tileNdxes = new Short[newDataSize];
	Short *blendTileNdxes = new Short[newDataSize];
	Short *extraBlendTileNdxes = new Short[newDataSize];
	UnsignedByte *data = new UnsignedByte[newDataSize];
	Short  *cliffInfoNdxes = new Short[newDataSize];  	
	
	Int i, j;
	for (i=0; i<newXSize; i++) {
		for (j=0; j<newYSize; j++) {
			Int newNdx = i+j*newXSize;	
			Int oldI = i+xOffset;
			Bool inRange = true;
			if(oldI>=m_width-1) {
				oldI=m_width-2;
				inRange = false;
			}	
			if (oldI<0) {
				oldI = 0;
				inRange = false;
			}
			Int oldJ = j+yOffset;
			if (oldJ>=m_height-1) {
				oldJ = m_height-2;
				inRange = false;
			}
			if (oldJ<0) {
				oldJ = 0;
				inRange = false;
			}
			Int oldNdx = oldI+oldJ*m_width;
			if (inRange) {
				data[newNdx] = m_data[oldNdx];
				tileNdxes[newNdx] = m_tileNdxes[oldNdx];
				blendTileNdxes[newNdx] = m_blendTileNdxes[oldNdx];
				extraBlendTileNdxes[newNdx] = m_extraBlendTileNdxes[oldNdx];
				cliffInfoNdxes[newNdx] = m_cliffInfoNdxes[oldNdx];
			} else {
				data[newNdx] = m_data[oldNdx];
				tileNdxes[newNdx] = m_tileNdxes[oldNdx];
				blendTileNdxes[newNdx] = 0;
				extraBlendTileNdxes[newNdx] = 0;
				cliffInfoNdxes[newNdx] = 0;
			}
		}
	}

	delete(m_tileNdxes);
	delete(m_cliffInfoNdxes);
	delete(m_blendTileNdxes);
	delete(m_extraBlendTileNdxes);
	delete(m_data);
	m_tileNdxes = tileNdxes;
	m_blendTileNdxes = blendTileNdxes;
	m_extraBlendTileNdxes = extraBlendTileNdxes;
	m_cliffInfoNdxes = cliffInfoNdxes;
	m_data = data;
	m_width = newXSize;
	m_height = newYSize;
	m_borderSize = newBorder;
	m_dataSize = newDataSize;
	delete(m_cellCliffState);
	delete(m_cellFlipState);
	Int numBytesX = (m_width+7)/8;	//how many bytes to fit all bitflags
 	m_flipStateWidth=numBytesX;

	m_cellFlipState	= MSGNEW("WorldHeightMapEdit::resize") UnsignedByte[numBytesX*m_height];
	m_cellCliffState	= MSGNEW("WorldHeightMapEdit::resize") UnsignedByte[numBytesX*m_height];

	// Verify index remapping
	for(i=0; i<m_dataSize; i++) {
		if (m_cliffInfoNdxes[i]<0 || m_cliffInfoNdxes[i]>= m_numCliffInfo) {
			m_cliffInfoNdxes[i] = 0;		
		}
		if (m_blendTileNdxes[i]<0 || m_blendTileNdxes[i]>= m_numBlendedTiles) {
			m_blendTileNdxes[i] = 0;		
		}
		if (m_extraBlendTileNdxes[i]<0 || m_extraBlendTileNdxes[i]>= m_numBlendedTiles) {
			m_extraBlendTileNdxes[i] = 0;		
		}
	}
	initCliffFlagsFromHeights();

	optimizeTiles();
	return(true);
}


/** Returns true if the texture class is used in the current
map.  If false, the texture is not used or loaded in the 
current map. */
Bool WorldHeightMapEdit::isTexClassUsed(Int textureClass)
{
	Int i;
	for (i=0; i<m_numTextureClasses; i++) {
		Int globalClass = m_textureClasses[i].globalTextureClass;
		if (globalClass == textureClass) {
			return(true);
		}
	}
	return(false);
}
	 
/** Returns the first tile for a texture class.  If the class is not
used, returns -1. */
Int WorldHeightMapEdit::getFirstTile(Int textureClass)
{
	Int i;
	for (i=0; i<m_numTextureClasses; i++) {
		Int globalClass = m_textureClasses[i].globalTextureClass;
		if (globalClass == textureClass) {
			return(m_textureClasses[i].firstTile);
		}
	}
	return(-1);
}
	
 
/**
 dbgVerifyAfterUndo - Verifies that the structure is consistent.
*/	 
void WorldHeightMapEdit::dbgVerifyAfterUndo(void)
{
#ifdef DEBUG_CRASHING
	Int i, j;
	for (i=0; i<m_numGlobalTextureClasses; i++) {
		m_globalTextureClasses[i].forDebugOnly_fileTextureClass = -1;
	}
	for (j=0; j<m_numTextureClasses; j++) {
		Int globalClass = m_textureClasses[j].globalTextureClass;
		DEBUG_ASSERTCRASH((globalClass >= 0),("oops"));
		DEBUG_ASSERTCRASH((globalClass < m_numGlobalTextureClasses),("oops"));
		if (m_globalTextureClasses[globalClass].forDebugOnly_fileTextureClass != j) {
			DEBUG_ASSERTCRASH((m_globalTextureClasses[globalClass].forDebugOnly_fileTextureClass == -1),("oops"));
			m_globalTextureClasses[globalClass].forDebugOnly_fileTextureClass = j;
		}
		DEBUG_ASSERTCRASH((m_textureClasses[j].width == m_globalTextureClasses[globalClass].width),("oops"));
		DEBUG_ASSERTCRASH((m_textureClasses[j].numTiles == m_globalTextureClasses[globalClass].numTiles),("oops"));
		TileData *pTile = m_sourceTiles[m_textureClasses[j].firstTile];
		DEBUG_ASSERTCRASH((pTile == m_globalTextureClasses[globalClass].tiles[0]),("oops")); 
		DEBUG_ASSERTCRASH((j == m_globalTextureClasses[globalClass].forDebugOnly_fileTextureClass),("oops"));
	}

	for (i=0; i<m_numGlobalTextureClasses; i++) {
		Int localClass = m_globalTextureClasses[i].forDebugOnly_fileTextureClass;
		DEBUG_ASSERTCRASH((localClass < NUM_TEXTURE_CLASSES),("oops"));
		if (localClass >= 0) {
			DEBUG_ASSERTCRASH((localClass < m_numTextureClasses),("oops"));
			DEBUG_ASSERTCRASH((m_textureClasses[localClass].globalTextureClass == i),("oops"));
			DEBUG_ASSERTCRASH((m_textureClasses[localClass].width == m_globalTextureClasses[i].width),("oops"));
			DEBUG_ASSERTCRASH((m_textureClasses[localClass].numTiles == m_globalTextureClasses[i].numTiles),("oops"));
			TileData *pTile = m_sourceTiles[m_textureClasses[localClass].firstTile];
			DEBUG_ASSERTCRASH((pTile == m_globalTextureClasses[i].tiles[0]),("oops")); 
			DEBUG_ASSERTCRASH((m_textureClasses[localClass].numTiles == m_globalTextureClasses[i].numTiles),("oops"));
		} 
	}

	for (i=0; i<m_numTextureClasses; i++) {
		DEBUG_ASSERTCRASH((m_textureClasses[i].globalTextureClass >= 0),("oops"));
		if (m_textureClasses[i].globalTextureClass >= 0) {
			AsciiString path1 = m_globalTextureClasses[m_textureClasses[i].globalTextureClass].name;
			AsciiString path2 = m_textureClasses[i].name;
			DEBUG_ASSERTCRASH(path1==path2,("oops"));
			DEBUG_ASSERTCRASH((m_globalTextureClasses[m_textureClasses[i].globalTextureClass].forDebugOnly_fileTextureClass==i),("oops"));
		}
	}

	for (i=0; i<m_dataSize; i++) {
		Int texNdx = this->m_tileNdxes[i];
		DEBUG_ASSERTCRASH(( (texNdx>>2) < m_numBitmapTiles),("oops"));
		Int texClass = getTextureClassFromNdx(texNdx);
		DEBUG_ASSERTCRASH((texClass>=0),("oops"));
	}
#endif
}

void WorldHeightMapEdit::addObject(MapObject *pMapObj)
{
	MapObject *newObj = pMapObj->duplicate();
	newObj->setNextMap(MapObject::TheMapObjectListPtr);
	MapObject::TheMapObjectListPtr = newObj;
}

void WorldHeightMapEdit::removeFirstObject(void)
{
	MapObject *firstObj = MapObject::TheMapObjectListPtr;
	MapObject::TheMapObjectListPtr = firstObj->getNext();
	firstObj->setNextMap(NULL); // so we don't delete the whole list.
	deleteInstance(firstObj);
}

//=============================================================================
// W3DRoadBuffer::selectDuplicates
//=============================================================================
/** Selects any duplicate objects. */
//=============================================================================
Bool WorldHeightMapEdit::selectDuplicates(void)
{
	const float DELTA =  0.05f;
	MapObject *firstObj = MapObject::TheMapObjectListPtr;
	MapObject *pObj;
//	MapObject *pPrevRoad = NULL;
	Bool anySelected = false;
	for (pObj=firstObj; pObj; pObj=pObj->getNext()) {
		pObj->setSelected(false);
		MapObject *prevObj;
		Coord3D curLoc = *pObj->getLocation();

		for (prevObj=firstObj; prevObj != pObj; prevObj=prevObj->getNext()) {
			if (pObj->getName() != prevObj->getName()) {
				continue; // names don't match.
			}
			if (pObj->isWaypoint()) {
				// Don't delete duplicate waypoints.
				continue;
			}
			Coord3D prevLoc = *prevObj->getLocation();
			if (abs(curLoc.x-prevLoc.x)>DELTA) {
				continue; // locations don't match.
			}
			if (abs(curLoc.y-prevLoc.y)>DELTA) {
				continue; // locations don't match.
			}

			if (pObj->getFlag(FLAG_ROAD_FLAGS)) {
				if (pObj->getNext() == NULL) continue;
				if (!pObj->getFlag(FLAG_ROAD_POINT1)) {
					continue;
				}
				if (!prevObj->getFlag(FLAG_ROAD_POINT1)) {
					continue;
				}
				Coord3D nextLoc = *pObj->getNext()->getLocation();
				prevObj = prevObj->getNext();
				if (!prevObj) continue;
				if (!prevObj->getFlag(FLAG_ROAD_POINT2)) {
					continue;
				}
				prevLoc = *prevObj->getLocation();
				if (abs(nextLoc.x-prevLoc.x)>DELTA) {
					continue; // locations don't match.
				}
				if (abs(nextLoc.y-prevLoc.y)>DELTA) {
					continue; // locations don't match.
				}
				pObj->setSelected(true);
				pObj = pObj->getNext();
				pObj->setSelected(true);
				anySelected = true;
				break;
			} else {
				pObj->setSelected(true);
				anySelected = true;
				break;
			}
		}
	}
	return anySelected;
}

//=============================================================================
// selectSimilar
//=============================================================================
/** Selects any similar objects. */
//=============================================================================
Bool WorldHeightMapEdit::selectSimilar(void)
{
//	const float DELTA =  0.05f;
	MapObject *firstObj = MapObject::TheMapObjectListPtr;
	MapObject *selectedObj;
	MapObject *otherObj;
	Bool anySelected = false;
	for (selectedObj=firstObj; selectedObj; selectedObj=selectedObj->getNext()) {
		if (selectedObj->getFlag(FLAG_ROAD_FLAGS)) {
			continue;
		} 
		if (selectedObj->isSelected()) {
			anySelected = true;
			break;
		}
	}
	if (!anySelected) {
		return false;
	}

	for (otherObj=firstObj; otherObj != NULL; otherObj=otherObj->getNext()) {
		if (otherObj->getName() != selectedObj->getName()) {
			continue; // names don't match.
		}

		if (otherObj->getFlag(FLAG_ROAD_FLAGS)) {
			continue;
		} 

		Bool exists;
		AsciiString layerName = otherObj->getProperties()->getAsciiString(TheKey_objectLayer, &exists);
		if (exists && TheLayersList->isLayerHidden(layerName)) {
			continue;			
		}

		otherObj->setSelected(true);
	}
	return anySelected;
}

//=============================================================================
// selectInvalidTeam
//=============================================================================
/** Selects any objects with invalid teams. */
//=============================================================================
Bool WorldHeightMapEdit::selectInvalidTeam(void)
{
	Int i;
	AsciiString name;

	// make a table of valid team names
	std::set<AsciiString> validTeamNames;
	std::set<AsciiString> invalidTeamNames;
	std::set<AsciiString>::const_iterator it;
	for (i = 0; i < TheSidesList->getNumTeams(); i++)
	{
		name = TheSidesList->getTeamInfo(i)->getDict()->getAsciiString(TheKey_teamName);
		if (name == NEUTRAL_TEAM_INTERNAL_STR)
			name = NEUTRAL_TEAM_UI_STR;
		validTeamNames.insert(name);
	}

	MapObject *obj;
	Bool anySelected = FALSE;
	for (obj=MapObject::TheMapObjectListPtr; obj; obj=obj->getNext()) {
		if (obj->getFlag(FLAG_ROAD_FLAGS)) {
			continue;
		} 

		const Dict *d = obj->getProperties();
		if (d)
		{
			Bool exists;
			name = d->getAsciiString(TheKey_originalOwner, &exists);
			if (!exists)
			{
				invalidTeamNames.insert("");
				obj->setSelected(true);
				anySelected = TRUE;
				continue;
			}

			if (name == NEUTRAL_TEAM_INTERNAL_STR)
				name = NEUTRAL_TEAM_UI_STR;

			it = validTeamNames.find(name);
			if (it == validTeamNames.end())
			{
				invalidTeamNames.insert(name);
				obj->setSelected(true);
				anySelected = TRUE;
			}
		}
	}

	AsciiString report = "";
	AsciiString line;

#if 0
	// I'm removing this because its causing buffer overflows, and we're really interested in 
	// teams that are invalid anyways. I've confirmed that this is okay behavior with JL. (jkmcd)
	for (it=validTeamNames.begin(); it!=validTeamNames.end(); ++it)
	{
		line.format("Valid team '%s'\n", it->str());
		report.concat(line);
	}
	report.concat("\n");
#endif

	for (it=invalidTeamNames.begin(); it!=invalidTeamNames.end(); ++it)
	{
		line.format("Invalid team '%s'\n", it->str());
		report.concat(line);
	}

	if (anySelected)
	{
		DEBUG_LOG(("%s\n", report.str()));
		MessageBox(NULL, report.str(), "Missing team report", MB_OK);
	}

	return anySelected;
}





AsciiString WorldHeightMapEdit::getTexClassUiName(int ndx)
{
	return(m_globalTextureClasses[ndx].uiName);
}
 
static const Real HEIGHT_SCALE = MAP_HEIGHT_SCALE / MAP_XY_FACTOR;
static const Real STRETCH_LIMIT = 1.5f;	 // If it is stretching less than this, don't adjust.
static inline Int IABS(Int x) {	if (x>=0) return x; return -x;};

static const Real TEX_PER_CELL = 32.0f/TEXTURE_WIDTH; // we use 32 texels per cell.

static const Real MIN_U_SPAN = TEX_PER_CELL * 0.4f;

static Bool debugToggle = true;

/******************************************************************
	doCliffAdjustment
		Does a cliff adjustment the region at xIndex, yIndex.
*/
Bool WorldHeightMapEdit::doCliffAdjustment(Int xIndex, Int yIndex) 
{

	debugToggle = !debugToggle;

	Int ndx = (yIndex*m_width)+xIndex;
	Int curTileClass = getTextureClass(xIndex, yIndex, true);
	if (curTileClass < 0) {
		return(false);
	}
	Real textureClassExtent = m_globalTextureClasses[curTileClass].width*TILE_PIXEL_EXTENT;
	textureClassExtent /= TEXTURE_WIDTH; 
	Real startU = textureClassExtent/2;	// Center in the texture.
	Real startV = 0; // We'll adjust the V values later.

	Int i, j;
	UnsignedByte *pProcessed = new UnsignedByte[m_dataSize];
	for (i=0; i<m_dataSize; i++) {
		pProcessed[i] = false;
	}
	if (pProcessed == NULL) {
		AfxMessageBox(IDS_OUT_OF_MEMORY);
		return false;
	}

	CProcessNode *pNodesToProcess = NULL;
	CProcessNode *pProcessTail = NULL;
	CProcessNode *pUnCliffyNodes = NULL;
	CProcessNode *pMutantNodes = NULL;
	Int nodesProcessed = 0;
	Region2D uvRange;
	uvRange.lo.x = 1;
	uvRange.lo.y = 1;
	uvRange.hi.x = -1;
	uvRange.hi.y = -1;
	

	pNodesToProcess = new CProcessNode(xIndex, yIndex);
	while (pNodesToProcess) {
		CProcessNode *pCurNode = pNodesToProcess;
		pNodesToProcess = pCurNode->m_next;
		pCurNode->m_next = NULL;			 

		if (pNodesToProcess == NULL) {
			if (pMutantNodes) {
				pNodesToProcess = pMutantNodes;
				pMutantNodes = pMutantNodes->m_next;
				pNodesToProcess->m_next = NULL;
				pProcessTail = pNodesToProcess;
			}
		}

		ndx = (pCurNode->m_y*m_width)+pCurNode->m_x;
		Bool skip = pProcessed[ndx];
		Bool classMatch = false;
		if (curTileClass == getTextureClass(pCurNode->m_x,pCurNode->m_y,true)) {
			classMatch = true;
		}
		Int blend = m_blendTileNdxes[ndx];
		if (blend && curTileClass == getTextureClassFromNdx(m_blendedTiles[blend].blendNdx)) {
			classMatch = true;
		}
		if (!classMatch) skip = true;
		if (skip) {
			delete pCurNode;
			continue;
		}

		static Real HEIGHT_SCALE = MAP_HEIGHT_SCALE / MAP_XY_FACTOR;
		static Real STRETCH_LIMIT = 1.5f;	 // If it is stretching less than this, don't adjust.
		Int h0 = m_data[ndx];
		Int h1 = m_data[ndx+1];
		Int h2 = m_data[ndx+m_width+1];
		Int h3 = m_data[ndx+m_width];

		Int deltaH, maxH;

		maxH = IABS(h0-h1);
		deltaH = IABS(h1-h2);
		if (deltaH>maxH)maxH = deltaH;
		deltaH = IABS(h2-h3);
		if (deltaH>maxH)maxH = deltaH;
 		deltaH = IABS(h3-h0);
		if (deltaH>maxH)maxH = deltaH;

		Bool isCliff = maxH*HEIGHT_SCALE > STRETCH_LIMIT/2.0f;

		Bool flip = IABS(h2-h0) >= IABS(h3-h1);

		// Convert h1-3 to deltas.
		h1 -= h0;
		h2 -= h0;
		h3 -= h0;
		h0 = 0;


		Int minHeightDelta = 0.7f/HEIGHT_SCALE;

		if (!isCliff) {
			// Handle the uncliffy nodes in a moment.
			pCurNode->m_next = pUnCliffyNodes;
			pUnCliffyNodes = pCurNode;
			continue;
		}

		if (isCliff) {
			TCliffInfo cliffInfo;
			if (flip) {
				Vector2 uVec((h3-h0), -(h1-h0));
				Vector2 uVec2((h2-h1), -(h2-h3));
				if (uVec.X==0 && uVec.Y==0) {
					uVec = uVec2;
				}
				if (uVec2.X==0 && uVec2.Y==0) {
					uVec2 = uVec;
				}

				if (IABS(h2-h1)<minHeightDelta && IABS(h2-h3)<minHeightDelta) {
					Int delta, delta2;
					delta = IABS(minHeightDelta - IABS(h2-h1));
					delta2 = IABS(minHeightDelta - IABS(h2-h3));
					if (delta>delta2) delta = delta2;
					if (h2<h0) delta = -delta;
					h2 += delta;
				}

				if (IABS(h0-h1)<minHeightDelta && IABS(h0-h3)<minHeightDelta) {
					Int delta, delta2;
					delta = IABS(minHeightDelta - IABS(h0-h1));
					delta2 = IABS(minHeightDelta - IABS(h0-h3));
					if (delta>delta2) delta = delta2;
					if (h0<h2) delta = -delta;
					h0 += delta;
				}

				uVec.Normalize();
				//uVec /= (fabs(uVec.X)+fabs(uVec.Y));
				uVec *= TEX_PER_CELL;
				uVec2.Normalize();
				uVec2 *= TEX_PER_CELL;
				//uVec2 /= (fabs(uVec2.X)+fabs(uVec2.Y));
				cliffInfo.u0 = startU;
				cliffInfo.v0 = startV- ((h0)*HEIGHT_SCALE*TEX_PER_CELL);
				cliffInfo.u1 = startU+uVec.X ;
				cliffInfo.v1 = startV - ((h1)*HEIGHT_SCALE*TEX_PER_CELL);
				cliffInfo.u2 = startU+uVec.X + uVec2.Y;
				cliffInfo.v2 = startV - ((h2)*HEIGHT_SCALE*TEX_PER_CELL);
				cliffInfo.u3 = startU+uVec.Y;
				cliffInfo.v3 = startV - ((h3)*HEIGHT_SCALE*TEX_PER_CELL);
				cliffInfo.flip = true;
				cliffInfo.mutant = false;
				cliffInfo.tileIndex = getFirstTile(curTileClass)<<2;
			} else {
				Vector2 uVec((h2-h1), -(h1-h0));
				Vector2 uVec2((h3-h0), -(h2-h3));
				if (uVec.X==0 && uVec.Y==0) {
					uVec = uVec2;
				}
				if (uVec2.X==0 && uVec2.Y==0) {
					uVec2 = uVec;
				}

				if (IABS(h1-h0)<minHeightDelta && IABS(h1-h2)<minHeightDelta) {
					Int delta, delta2;
					delta = IABS(minHeightDelta - IABS(h1-h0));
					delta2 = IABS(minHeightDelta - IABS(h1-h2));
					if (delta>delta2) delta = delta2;
					if (h1<h3) delta = -delta;
					h1 += delta;
				}

				if (IABS(h3-h0)<minHeightDelta && IABS(h3-h2)<minHeightDelta) {
					Int delta, delta2;
					delta = IABS(minHeightDelta - IABS(h3-h0));
					delta2 = IABS(minHeightDelta - IABS(h3-h2));
					if (delta>delta2) delta = delta2;
					if (h3<h0) delta = -delta;
					h3 += delta;
				}

				uVec.Normalize();
				uVec *= TEX_PER_CELL;
				uVec2.Normalize();
				uVec2 *= TEX_PER_CELL;
				cliffInfo.u0 = startU;
				cliffInfo.v0 = startV - ((h0)*HEIGHT_SCALE*TEX_PER_CELL);
				cliffInfo.u1 = startU+uVec.X ;
				cliffInfo.v1 = startV - ((h1)*HEIGHT_SCALE*TEX_PER_CELL);
				cliffInfo.u2 = startU+uVec.X + uVec.Y;
				cliffInfo.v2 = startV - ((h2)*HEIGHT_SCALE*TEX_PER_CELL);
				cliffInfo.u3 = startU+uVec2.Y;
				cliffInfo.v3 = startV - ((h3)*HEIGHT_SCALE*TEX_PER_CELL);
				cliffInfo.flip = false;
				cliffInfo.mutant = false;
				cliffInfo.tileIndex = getFirstTile(curTileClass)<<2;
			}

			updateForAdjacentCliffs(pCurNode->m_x, pCurNode->m_y, pProcessed, cliffInfo);

			/*Bool fits =*/ adjustForTiling(cliffInfo, textureClassExtent);

			Int cliffNdx=addCliffInfo(&cliffInfo);
			m_cliffInfoNdxes[ndx] = cliffNdx;
			pProcessed[ndx] = true;
			if (uvRange.lo.x>cliffInfo.u0) uvRange.lo.x=cliffInfo.u0;
			if (uvRange.lo.x>cliffInfo.u1) uvRange.lo.x=cliffInfo.u1;
			if (uvRange.lo.x>cliffInfo.u2) uvRange.lo.x=cliffInfo.u2;
			if (uvRange.lo.x>cliffInfo.u3) uvRange.lo.x=cliffInfo.u3;
			if (uvRange.lo.y>cliffInfo.v0) uvRange.lo.y=cliffInfo.v0;
			if (uvRange.lo.y>cliffInfo.v1) uvRange.lo.y=cliffInfo.v1;
			if (uvRange.lo.y>cliffInfo.v2) uvRange.lo.y=cliffInfo.v2;
			if (uvRange.lo.y>cliffInfo.v3) uvRange.lo.y=cliffInfo.v3;
			if (uvRange.hi.x<cliffInfo.u0) uvRange.hi.x=cliffInfo.u0;
			if (uvRange.hi.x<cliffInfo.u1) uvRange.hi.x=cliffInfo.u1;
			if (uvRange.hi.x<cliffInfo.u2) uvRange.hi.x=cliffInfo.u2;
			if (uvRange.hi.x<cliffInfo.u3) uvRange.hi.x=cliffInfo.u3;
			if (uvRange.hi.y<cliffInfo.v0) uvRange.hi.y=cliffInfo.v0;
			if (uvRange.hi.y<cliffInfo.v1) uvRange.hi.y=cliffInfo.v1;
			if (uvRange.hi.y<cliffInfo.v2) uvRange.hi.y=cliffInfo.v2;
			if (uvRange.hi.y<cliffInfo.v3) uvRange.hi.y=cliffInfo.v3;

			CProcessNode *pNodes[4] = {NULL,NULL,NULL,NULL};
			Int k = 0;
			for (i=pCurNode->m_x-1; i<pCurNode->m_x+2; i++) {
				if (i<0) continue;
				if (i>=m_width-1) continue;
				for (j=pCurNode->m_y-1; j<pCurNode->m_y+2; j++) {
					if (i==pCurNode->m_x && j==pCurNode->m_y) continue;
					if (i!=pCurNode->m_x && j!=pCurNode->m_y) continue;
					if (j<0) continue;
					if (j>=m_height-1) continue;	 
 					ndx = (j*m_width)+i;
					if (pProcessed[ndx]) continue;
					CProcessNode *pNewNode = new CProcessNode(i,j);
					DEBUG_LOG(("Adding node %d, %d\n", i, j));
					pNodes[k++]	= pNewNode;
					Real dx, dy;
					if (i<pCurNode->m_x) {
						// left.
						dx = cliffInfo.u3 - cliffInfo.u0;
						dy = cliffInfo.v3 - cliffInfo.v0;
					}	else if (i>pCurNode->m_x) {
						// right.
						dx = cliffInfo.u2 - cliffInfo.u1;
						dy = cliffInfo.v2 - cliffInfo.v1;
					}	else if (j<pCurNode->m_y) {
						// bottom.
						dx = cliffInfo.u1 - cliffInfo.u0;
						dy = cliffInfo.v1 - cliffInfo.v0;
					}	else if (j<pCurNode->m_y) {
						// top.
						dx = cliffInfo.u2 - cliffInfo.u3;
						dy = cliffInfo.v2 - cliffInfo.v3;
					}	
					pNewNode->m_len = dx*dx + dy*dy;
				}
			}	
			while (k)	{
				Real maxLen = -1;
				Int curNdx = -1;
				for (i=0; i<k; i++) {
					if (pNodes[i]) {
						if (pNodes[i]->m_len > maxLen) {
							curNdx = i;
							maxLen = pNodes[i]->m_len;
						}
					}
				}
				if (curNdx>=0) { 
					if (cliffInfo.mutant) {
						pNodes[curNdx]->m_next = pMutantNodes;
						pMutantNodes = pNodes[curNdx];
						pNodes[curNdx] = NULL;
					}	else {
						if (pProcessTail) {
							pProcessTail->m_next = pNodes[curNdx];
						} 
						pProcessTail = pNodes[curNdx];
						if (pNodesToProcess == NULL) {
							pNodesToProcess = pNodes[curNdx];
						}
						pNodes[curNdx] = NULL;
					}
				}	else {
					k = 0;
				}
			}


			nodesProcessed++;
			//if (nodesProcessed>2) break;
		}	
		delete pCurNode;
	}

	while (pUnCliffyNodes) {
		CProcessNode *pCurNode = pUnCliffyNodes;
		pUnCliffyNodes = pCurNode->m_next;
		pCurNode->m_next = NULL;
		ndx = (pCurNode->m_y*m_width)+pCurNode->m_x;
		if (!pProcessed[ndx]) {
			m_cliffInfoNdxes[ndx] = 0;
			updateFlatCellForAdjacentCliffs(pCurNode->m_x, pCurNode->m_y, curTileClass, pProcessed);
			pProcessed[ndx] = true;
		}
		delete pCurNode;
	}

	if (nodesProcessed) {
		Real vDelta = (-textureClassExtent/2) - (uvRange.hi.y + uvRange.lo.y)/2;
		for (i=0; i<m_width; i++) {
			for (j=0; j<m_height; j++) {
 				ndx = (j*m_width)+i;
				if (pProcessed[ndx]) {
					Int cliffNdx = m_cliffInfoNdxes[ndx];
					if (cliffNdx) {
						m_cliffInfo[cliffNdx].v0 += vDelta;
						m_cliffInfo[cliffNdx].v1 += vDelta;
						m_cliffInfo[cliffNdx].v2 += vDelta;
						m_cliffInfo[cliffNdx].v3 += vDelta;
					}
				}
			}
		}

	}
	return(true);
}

/******************************************************************
	removeCliffMapping
		Removes all cliff adjustments.
*/
Bool WorldHeightMapEdit::removeCliffMapping(void) 
{

	Int ndx;
	for (ndx = 0; ndx<m_dataSize; ndx++) {
		m_cliffInfoNdxes[ndx] = 0;
		m_numCliffInfo = 0;
	}
	return(true);
}


Bool WorldHeightMapEdit::adjustForTiling( TCliffInfo &cliffInfo, Real textureWidth)
{
	Real minU = cliffInfo.u0;
	Real maxU = minU;
	const Real delta = (TEX_PER_CELL/100);
	if (minU>cliffInfo.u1) minU=cliffInfo.u1;
	if (maxU<cliffInfo.u1) maxU=cliffInfo.u1;
	if (minU>cliffInfo.u2) minU=cliffInfo.u2;
	if (maxU<cliffInfo.u2) maxU=cliffInfo.u2;
	if (minU>cliffInfo.u3) minU=cliffInfo.u3;
	if (maxU<cliffInfo.u3) maxU=cliffInfo.u3;

	if (maxU-minU > textureWidth*0.5) {
		Real mid = (minU + maxU) / 2.0f;
		if (-minU > maxU-textureWidth) {
			// biased negative, so add.
			if (cliffInfo.u0 < mid) cliffInfo.u0 += textureWidth;
			if (cliffInfo.u1 < mid) cliffInfo.u1 += textureWidth;
			if (cliffInfo.u2 < mid) cliffInfo.u2 += textureWidth;
			if (cliffInfo.u3 < mid) cliffInfo.u3 += textureWidth;
		}	else {
			// biased past tex width, so add.
			if (cliffInfo.u0 > mid) cliffInfo.u0 -= textureWidth;
			if (cliffInfo.u1 > mid) cliffInfo.u1 -= textureWidth;
			if (cliffInfo.u2 > mid) cliffInfo.u2 -= textureWidth;
			if (cliffInfo.u3 > mid) cliffInfo.u3 -= textureWidth;
		}
		// Recalculate min/max
		minU = cliffInfo.u0;
		maxU = minU;
		if (minU>cliffInfo.u1) minU=cliffInfo.u1;
		if (maxU<cliffInfo.u1) maxU=cliffInfo.u1;
		if (minU>cliffInfo.u2) minU=cliffInfo.u2;
		if (maxU<cliffInfo.u2) maxU=cliffInfo.u2;
		if (minU>cliffInfo.u3) minU=cliffInfo.u3;
		if (maxU<cliffInfo.u3) maxU=cliffInfo.u3;
	}

	if (minU >= textureWidth-delta) {
		cliffInfo.u0 -= textureWidth;
		cliffInfo.u1 -= textureWidth;
		cliffInfo.u2 -= textureWidth;
		cliffInfo.u3 -= textureWidth;
		minU -= textureWidth;
		maxU -= textureWidth;
	}
	if (maxU < delta) {
		cliffInfo.u0 += textureWidth;
		cliffInfo.u1 += textureWidth;
		cliffInfo.u2 += textureWidth;
		cliffInfo.u3 += textureWidth;
		minU += textureWidth;
		maxU += textureWidth;
	}


	Real uBorder = TEX_PER_CELL * 0.5;
	if (minU>=uBorder && maxU<textureWidth-uBorder) return true;
	if (minU>=-delta && maxU<= textureWidth+delta) {
		if (cliffInfo.u0<uBorder) cliffInfo.u0 = 0; 
		if (cliffInfo.u1<uBorder) cliffInfo.u1 = 0; 
		if (cliffInfo.u2<uBorder) cliffInfo.u2 = 0; 
		if (cliffInfo.u3<uBorder) cliffInfo.u3 = 0; 
		if (cliffInfo.u0>textureWidth-uBorder) cliffInfo.u0 = textureWidth; 
		if (cliffInfo.u1>textureWidth-uBorder) cliffInfo.u1 = textureWidth; 
		if (cliffInfo.u2>textureWidth-uBorder) cliffInfo.u2 = textureWidth; 
		if (cliffInfo.u3>textureWidth-uBorder) cliffInfo.u3 = textureWidth; 
		return true;
	}

	if (minU < textureWidth-delta && maxU>textureWidth+delta) {
		cliffInfo.u0 -= textureWidth;
		cliffInfo.u1 -= textureWidth;
		cliffInfo.u2 -= textureWidth;
		cliffInfo.u3 -= textureWidth;
		minU -= textureWidth;
		maxU -= textureWidth;
	}
	TCliffInfo tmpCliff = cliffInfo;
//	Bool doOffset = false;
	Real offset;					 

	DEBUG_ASSERTLOG(minU<-delta && maxU > delta, ("Oops, wrong.\n")) ;

	// Straddles the 0 line.
	if (maxU > -minU) {
		offset = -minU;
		// push min up to 0
		if (cliffInfo.u0<0) cliffInfo.u0 = 0; 
		if (cliffInfo.u1<0) cliffInfo.u1 = 0; 
		if (cliffInfo.u2<0) cliffInfo.u2 = 0; 
		if (cliffInfo.u3<0) cliffInfo.u3 = 0; 
	} else {
		// push max down to 0, & offset.
		offset = -maxU;
		if (cliffInfo.u0>0) cliffInfo.u0 = 0; 
		if (cliffInfo.u1>0) cliffInfo.u1 = 0; 
		if (cliffInfo.u2>0) cliffInfo.u2 = 0; 
		if (cliffInfo.u3>0) cliffInfo.u3 = 0; 
		cliffInfo.u0 += textureWidth;
		cliffInfo.u1 += textureWidth;
		cliffInfo.u2 += textureWidth;
		cliffInfo.u3 += textureWidth;	 
	}
	Real triMinU;
	Real triMaxU;
	Bool tooSmall = false;
	triMinU = cliffInfo.u0;
	triMaxU = triMinU;
	if (triMinU>cliffInfo.u1) triMinU=cliffInfo.u1;
	if (triMaxU<cliffInfo.u1) triMaxU=cliffInfo.u1;
	if (triMinU>cliffInfo.u3) triMinU=cliffInfo.u3;
	if (triMaxU<cliffInfo.u3) triMaxU=cliffInfo.u3;
	if((triMaxU-triMinU)<MIN_U_SPAN) {
		tooSmall = true;
	}
	triMinU = cliffInfo.u1;
	triMaxU = triMinU;
	if (triMinU>cliffInfo.u2) triMinU=cliffInfo.u2;
	if (triMaxU<cliffInfo.u2) triMaxU=cliffInfo.u2;
	if (triMinU>cliffInfo.u3) triMinU=cliffInfo.u3;
	if (triMaxU<cliffInfo.u3) triMaxU=cliffInfo.u3;
	if((triMaxU-triMinU)<MIN_U_SPAN) {
		tooSmall = true;
	}
	triMinU = cliffInfo.u0;
	triMaxU = triMinU;
	if (triMinU>cliffInfo.u1) triMinU=cliffInfo.u1;
	if (triMaxU<cliffInfo.u1) triMaxU=cliffInfo.u1;
	if (triMinU>cliffInfo.u2) triMinU=cliffInfo.u2;
	if (triMaxU<cliffInfo.u2) triMaxU=cliffInfo.u2;
	if((triMaxU-triMinU)<MIN_U_SPAN) {
		tooSmall = true;
	}
	triMinU = cliffInfo.u0;
	triMaxU = triMinU;
	if (triMinU>cliffInfo.u2) triMinU=cliffInfo.u2;
	if (triMaxU<cliffInfo.u2) triMaxU=cliffInfo.u2;
	if (triMinU>cliffInfo.u3) triMinU=cliffInfo.u3;
	if (triMaxU<cliffInfo.u3) triMaxU=cliffInfo.u3;
	if((triMaxU-triMinU)<MIN_U_SPAN) {
		tooSmall = true;
	}
	if(tooSmall) {
		cliffInfo.u0 = tmpCliff.u0 + offset;
		cliffInfo.u1 = tmpCliff.u1 + offset;
		cliffInfo.u2 = tmpCliff.u2 + offset;
		cliffInfo.u3 = tmpCliff.u3 + offset;	 
		if (maxU < -minU) {
			cliffInfo.u0 += textureWidth;
			cliffInfo.u1 += textureWidth;
			cliffInfo.u2 += textureWidth;
			cliffInfo.u3 += textureWidth;	 
		}
		cliffInfo.mutant = true;
	}

	return false;

}


static Bool usMatch(Real u1, Real u2) {
	//return true;
	if (fabs(u1-u2) < TEX_PER_CELL/4) return true;
	if (u1==0 && u2>3*TEX_PER_CELL) return true;
	if (u2==0 && u1>3*TEX_PER_CELL) return true;
	return false;
}

void WorldHeightMapEdit::updateForAdjacentCliffs(Int xIndex, Int yIndex, 
													UnsignedByte *pProcessed, TCliffInfo &cliffInfo)
{
#define PROMOTE_DIAGONALS 0	 // Don't promote them.
#define ADJUST_SNAPS 1
	TCliffInfo tmpCliff = cliffInfo;
	Int useMutant;
	for (useMutant=0; useMutant<2; useMutant++) {
		Bool lock0 = false;
		Bool lock1 = false;
		Bool lock2 = false;
		Bool lock3 = false;
		Bool anyMutant = false;
		Int i, j;
		for (i=xIndex-1; i<xIndex+2; i+=1) {
			if (i<0) continue;
			if (i>=m_width-1) continue;
			for (j=yIndex-1; j<yIndex+2; j+=1) {
				if (i==xIndex && j==yIndex) continue;
				if (i!=xIndex && j!=yIndex) continue;
				if (j<0) continue;
				if (j>=m_height-1) continue;	
 				Int ndx = (j*m_width)+i;
				if (!pProcessed[ndx]) continue;
				if (m_cliffInfoNdxes[ndx]) {
					TCliffInfo info = m_cliffInfo[m_cliffInfoNdxes[ndx]];
					if (info.mutant) {
						anyMutant = true;
						if (!useMutant) {
						continue;  // don't propagate warped stuff.
						}
					}
					Bool shifted = false;
					if (i==xIndex){
						if (j<yIndex) {
							// below 
							DEBUG_ASSERTCRASH(!lock1, ("Shouldn't happen."));
							if (lock0 && !usMatch(tmpCliff.u0, info.u3)) {
								shifted = true;
							}

							if (!shifted) {
								tmpCliff.u0 = info.u3;
								tmpCliff.u1 = info.u2;
								tmpCliff.v0 = info.v3;
								tmpCliff.v1 = info.v2;
								lock0 = lock1 = true;
							}
						} else {
							// above 
							DEBUG_ASSERTCRASH(!lock2, ("Shouldn't happen."));
							if (lock3 && !usMatch(tmpCliff.u3, info.u0)) {
								shifted = true;
							}
							if (!shifted) {
								tmpCliff.u3 = info.u0;
								tmpCliff.u2 = info.u1;
								tmpCliff.v3 = info.v0;
								tmpCliff.v2 = info.v1;
								lock2 = lock3 = true;
							}
						}
					}	else if (j==yIndex) {
						if (i<xIndex) {
							// left
							DEBUG_ASSERTCRASH(!lock0 && !lock3, ("Shouldn't happen."));
							tmpCliff.u0 = info.u1;
							tmpCliff.u3 = info.u2;
							tmpCliff.v0 = info.v1;
							tmpCliff.v3 = info.v2;
							lock0 = lock3 = true;
						} else {
							// right
							if (lock1 && !usMatch(tmpCliff.u1, info.u0)) shifted=true;
							if (lock2 && !usMatch(tmpCliff.u2, info.u3)) shifted=true;
							if (!shifted) {
								tmpCliff.u1 = info.u0;
								tmpCliff.u2 = info.u3;
								tmpCliff.v1 = info.v0;
								tmpCliff.v2 = info.v3;
								lock2 = lock1 = true;
							}
						}
					}	 
					if (shifted) tmpCliff.mutant = true;
				}
			}
		}
		Int lockCount = lock0+lock1+lock2+lock3;
		if (lockCount==0) {
			continue;
		}
		if (lockCount==4) {
			cliffInfo = tmpCliff;
			return; // all were adjusted.
		}
		Real dUa, dUb;
		if (!lock0) {
			dUa = 0;
			dUb = 0;
			if (lock1) {
				dUa =  cliffInfo.u0 - cliffInfo.u1;
				tmpCliff.u0 = tmpCliff.u1 + dUa;
				tmpCliff.v0 = tmpCliff.v1 + cliffInfo.v0 - cliffInfo.v1;
			}	else {
				dUa = cliffInfo.u0 - cliffInfo.u3;
				tmpCliff.u0 = tmpCliff.u3 + dUa;
				tmpCliff.v0 = tmpCliff.v3 + cliffInfo.v0 - cliffInfo.v3;
			}
	#if PROMOTE_DIAGONALS
			if (lock2) {
				dUb = cliffInfo.u0 - cliffInfo.u2;
			}
			if (fabs(dUa)<fabs(dUb)) {
				tmpCliff.u0 = tmpCliff.u2 + dUb;
				tmpCliff.v0 = tmpCliff.v2 + cliffInfo.v0 - cliffInfo.v2;
			}	
	#endif
		}
		if (!lock1) {
			dUa = 0;
			dUb = 0;
			if (lock2) {
				dUa =  cliffInfo.u1 - cliffInfo.u2;
				tmpCliff.u1 = tmpCliff.u2 + dUa;
				tmpCliff.v1 = tmpCliff.v2 + cliffInfo.v1 - cliffInfo.v2;
			}	
			if (lock0) {
				dUa = cliffInfo.u1 - cliffInfo.u0;
				tmpCliff.u1 = tmpCliff.u0 + dUa;
				tmpCliff.v1 = tmpCliff.v0 + cliffInfo.v1 - cliffInfo.v0;
			}
	#if PROMOTE_DIAGONALS
			if (lock3) {
				dUb = cliffInfo.u1 - cliffInfo.u3;
			}
			if (fabs(dUa)<fabs(dUb)) {
				tmpCliff.u1 = tmpCliff.u3 + dUb;
				tmpCliff.v1 = tmpCliff.v3 + cliffInfo.v1 - cliffInfo.v3;
			}	
	#endif
		}
		if (!lock2) {
			dUa = 0;
			dUa = 0;
			if (lock1) {
				dUa =  cliffInfo.u2 - cliffInfo.u1;
				tmpCliff.u2 = tmpCliff.u1 + dUa;
				tmpCliff.v2 = tmpCliff.v1 + cliffInfo.v2 - cliffInfo.v1;
			}	
			if (lock3) {
				dUa = cliffInfo.u2 - cliffInfo.u3;
				tmpCliff.u2 = tmpCliff.u3 + dUa;
				tmpCliff.v2 = tmpCliff.v3 + cliffInfo.v2 - cliffInfo.v3;
			}
	#if PROMOTE_DIAGONALS
			if (lock0) {
				dUb = cliffInfo.u2 - cliffInfo.u0;
			}
			if (fabs(dUa)<fabs(dUb)) {
				tmpCliff.u2 = tmpCliff.u0 + dUb;
				tmpCliff.v2 = tmpCliff.v0 + cliffInfo.v2 - cliffInfo.v0;
			}	
	#endif
		}
		if (!lock3) {
			dUa = 0;
			dUa = 0;
			if (lock2) {
				dUa =  cliffInfo.u3 - cliffInfo.u2;
				tmpCliff.u3 = tmpCliff.u2 + dUa;
				tmpCliff.v3 = tmpCliff.v2 + cliffInfo.v3 - cliffInfo.v2;
			}	
			if (lock0) {
				dUa = cliffInfo.u3 - cliffInfo.u0;
				tmpCliff.u3 = tmpCliff.u0 + dUa;
				tmpCliff.v3 = tmpCliff.v0 + cliffInfo.v3 - cliffInfo.v0;
			}
	#if PROMOTE_DIAGONALS
			if (lock1) {
				dUb = cliffInfo.u3 - cliffInfo.u1;
			}
			if (fabs(dUa)<fabs(dUb)) {
				tmpCliff.u3 = tmpCliff.u1 + dUb;
				tmpCliff.v3 = tmpCliff.v1 + cliffInfo.v3 - cliffInfo.v1;
			}	
	#endif
		}

		if (lockCount==3) {
			Real minU;
			Real maxU;
			Bool tooSmall = false;
			minU = tmpCliff.u0;
			maxU = minU;
			if (minU>tmpCliff.u1) minU=tmpCliff.u1;
			if (maxU<tmpCliff.u1) maxU=tmpCliff.u1;
			if (minU>tmpCliff.u3) minU=tmpCliff.u3;
			if (maxU<tmpCliff.u3) maxU=tmpCliff.u3;
			if((maxU-minU)<MIN_U_SPAN) {
				tooSmall = true;
			}
			minU = tmpCliff.u1;
			maxU = minU;
			if (minU>tmpCliff.u2) minU=tmpCliff.u2;
			if (maxU<tmpCliff.u2) maxU=tmpCliff.u2;
			if (minU>tmpCliff.u3) minU=tmpCliff.u3;
			if (maxU<tmpCliff.u3) maxU=tmpCliff.u3;
			if((maxU-minU)<MIN_U_SPAN) {
				tooSmall = true;
			}
			minU = tmpCliff.u0;
			maxU = minU;
			if (minU>tmpCliff.u1) minU=tmpCliff.u1;
			if (maxU<tmpCliff.u1) maxU=tmpCliff.u1;
			if (minU>tmpCliff.u2) minU=tmpCliff.u2;
			if (maxU<tmpCliff.u2) maxU=tmpCliff.u2;
			if((maxU-minU)<MIN_U_SPAN) {
				tooSmall = true;
			}
			minU = tmpCliff.u0;
			maxU = minU;
			if (minU>tmpCliff.u2) minU=tmpCliff.u2;
			if (maxU<tmpCliff.u2) maxU=tmpCliff.u2;
			if (minU>tmpCliff.u3) minU=tmpCliff.u3;
			if (maxU<tmpCliff.u3) maxU=tmpCliff.u3;
			if((maxU-minU)<MIN_U_SPAN) {
				tooSmall = true;
			}
			if(tooSmall) {
				// Take the center point & recalc.
				if (!lock0) {
					// Leave 2.
					tmpCliff.u1 = tmpCliff.u2 + cliffInfo.u1 - cliffInfo.u2;
					tmpCliff.v1 = tmpCliff.v2 + cliffInfo.v1 - cliffInfo.v2;
					tmpCliff.u3 = tmpCliff.u2 + cliffInfo.u3 - cliffInfo.u2;
					tmpCliff.v3 = tmpCliff.v2 + cliffInfo.v3 - cliffInfo.v2;
					tmpCliff.u0 = tmpCliff.u2 + cliffInfo.u0 - cliffInfo.u2;
					tmpCliff.v0 = tmpCliff.v2 + cliffInfo.v0 - cliffInfo.v2;
				}
			
				if (!lock1) {
					// Leave 3.
					tmpCliff.u1 = tmpCliff.u3 + cliffInfo.u1 - cliffInfo.u3;
					tmpCliff.v1 = tmpCliff.v3 + cliffInfo.v1 - cliffInfo.v3;
					tmpCliff.u2 = tmpCliff.u3 + cliffInfo.u2 - cliffInfo.u3;
					tmpCliff.v2 = tmpCliff.v3 + cliffInfo.v2 - cliffInfo.v3;
					tmpCliff.u0 = tmpCliff.u3 + cliffInfo.u0 - cliffInfo.u3;
					tmpCliff.v0 = tmpCliff.v3 + cliffInfo.v0 - cliffInfo.v3;
				}
			
				if (!lock2) {
					// Leave 0.
					tmpCliff.u1 = tmpCliff.u0 + cliffInfo.u1 - cliffInfo.u0;
					tmpCliff.v1 = tmpCliff.v0 + cliffInfo.v1 - cliffInfo.v0;
					tmpCliff.u3 = tmpCliff.u0 + cliffInfo.u3 - cliffInfo.u0;
					tmpCliff.v3 = tmpCliff.v0 + cliffInfo.v3 - cliffInfo.v0;
					tmpCliff.u2 = tmpCliff.u0 + cliffInfo.u2 - cliffInfo.u0;
					tmpCliff.v2 = tmpCliff.v0 + cliffInfo.v2 - cliffInfo.v0;
				}
			
				if (!lock3) {
					// Leave 1.
					tmpCliff.u0 = tmpCliff.u1 + cliffInfo.u0 - cliffInfo.u1;
					tmpCliff.v0 = tmpCliff.v1 + cliffInfo.v0 - cliffInfo.v1;
					tmpCliff.u3 = tmpCliff.u1 + cliffInfo.u3 - cliffInfo.u1;
					tmpCliff.v3 = tmpCliff.v1 + cliffInfo.v3 - cliffInfo.v1;
					tmpCliff.u2 = tmpCliff.u1 + cliffInfo.u2 - cliffInfo.u1;
					tmpCliff.v2 = tmpCliff.v1 + cliffInfo.v2 - cliffInfo.v1;
				}
				tmpCliff.mutant = true;
			
			}
		}

		Real smallU = TEX_PER_CELL/10.0f;
		Real cliffAvgU;
		Real tmpAvgU;
		Real minUDelta = TEX_PER_CELL*0.7;
		Real uDelta;
		// Adjust for "vertical" edges
		if (lockCount==2) {
			if (!lock0 && !lock1) {
				if (fabs(cliffInfo.u0-cliffInfo.u1)<smallU) {
					cliffAvgU = (cliffInfo.u0+cliffInfo.u1)/2;
					tmpAvgU = (tmpCliff.u0+tmpCliff.u1)/2;
					tmpCliff.u0 = tmpAvgU + cliffInfo.u0 - cliffAvgU;
					tmpCliff.u1 = tmpAvgU + cliffInfo.u1 - cliffAvgU;

					uDelta = cliffInfo.u0-cliffInfo.u3;
					uDelta /= fabs(uDelta);
					uDelta *= TEX_PER_CELL;
					if (fabs(tmpCliff.u0-tmpCliff.u3)<minUDelta) {
						tmpCliff.u3 = tmpCliff.u0 - uDelta;
					}
					if (fabs(tmpCliff.u1-tmpCliff.u2)<minUDelta) {
						tmpCliff.u2 = tmpCliff.u1 - uDelta;
					}

				}
			}	else if (!lock1 && !lock2) {
				if (fabs(cliffInfo.u1-cliffInfo.u2)<smallU) {
					cliffAvgU = (cliffInfo.u1+cliffInfo.u2)/2;
					tmpAvgU = (tmpCliff.u1+tmpCliff.u2)/2;
					tmpCliff.u1 = tmpAvgU + cliffInfo.u1 - cliffAvgU;
					tmpCliff.u2 = tmpAvgU + cliffInfo.u2 - cliffAvgU;

					uDelta = cliffInfo.u1-cliffInfo.u0;
					uDelta /= fabs(uDelta);
					uDelta *= TEX_PER_CELL;
					if (fabs(tmpCliff.u1-tmpCliff.u0)<minUDelta) {
						tmpCliff.u0 = tmpCliff.u1 - uDelta;
					}
					if (fabs(tmpCliff.u2-tmpCliff.u3)<minUDelta) {
						tmpCliff.u3 = tmpCliff.u2 - uDelta;
					}
				}
			}	else if (!lock2 && !lock3) {
				if (fabs(cliffInfo.u2-cliffInfo.u3)<smallU) {
					cliffAvgU = (cliffInfo.u2+cliffInfo.u3)/2;
					tmpAvgU = (tmpCliff.u2+tmpCliff.u3)/2;
					tmpCliff.u2 = tmpAvgU + cliffInfo.u2 - cliffAvgU;
					tmpCliff.u3 = tmpAvgU + cliffInfo.u3 - cliffAvgU;

					uDelta = cliffInfo.u3-cliffInfo.u0;
					uDelta /= fabs(uDelta);
					uDelta *= TEX_PER_CELL;
					if (fabs(tmpCliff.u3-tmpCliff.u0)<minUDelta) {
						tmpCliff.u0 = tmpCliff.u3 - uDelta;
					}
					if (fabs(tmpCliff.u2-tmpCliff.u1)<minUDelta) {
						tmpCliff.u1 = tmpCliff.u2 - uDelta;
					}
				}
			}	else if (!lock3 && !lock0) {
				if (fabs(cliffInfo.u3-cliffInfo.u0)<smallU) {
					cliffAvgU = (cliffInfo.u3+cliffInfo.u0)/2;
					tmpAvgU = (tmpCliff.u3+tmpCliff.u0)/2;
					tmpCliff.u3 = tmpAvgU + cliffInfo.u3 - cliffAvgU;
					tmpCliff.u0 = tmpAvgU + cliffInfo.u0 - cliffAvgU;

					uDelta = cliffInfo.u0-cliffInfo.u1;
					uDelta /= fabs(uDelta);
					uDelta *= TEX_PER_CELL;
					if (fabs(tmpCliff.u0-tmpCliff.u1)<minUDelta) {
						tmpCliff.u1 = tmpCliff.u0 - uDelta;
					}
					if (fabs(tmpCliff.u3-tmpCliff.u2)<minUDelta) {
						tmpCliff.u2 = tmpCliff.u1 - uDelta;
					}
				}
			}

		}
	}

	cliffInfo = tmpCliff;
}



void WorldHeightMapEdit::updateFlatCellForAdjacentCliffs(Int xIndex, Int yIndex, 
													Int curTileClass, UnsignedByte *pProcessed)
{
	TCliffInfo tmpCliff;
	Bool lock0 = false;
	Bool lock1 = false;
	Bool lock2 = false;
	Bool lock3 = false;
	Int ndx = (yIndex*m_width)+xIndex;
	DEBUG_ASSERTCRASH((ndx>=0 && ndx<this->m_dataSize),("oops"));
	if (ndx<0 || ndx >= this->m_dataSize) return;
	Bool okToProcess = true;
	if (pProcessed && pProcessed[ndx]) okToProcess = false;
	if (!pProcessed && m_cliffInfoNdxes[ndx]) {
		if (curTileClass != getTextureClassFromNdx(m_cliffInfo[m_cliffInfoNdxes[ndx]].tileIndex)) {
			okToProcess = false;
		}
	}
	if (!okToProcess) return;
	Real textureClassExtent = m_globalTextureClasses[curTileClass].width*TILE_PIXEL_EXTENT;
	textureClassExtent /= TEXTURE_WIDTH; 
	Int i, j;
	for (i=xIndex-1; i<xIndex+2; i+=1) {
		if (i<0) continue;
		if (i>=m_width-1) continue;
		for (j=yIndex-1; j<yIndex+2; j+=1) {
			if (i==xIndex && j==yIndex) continue;
			if (i!=xIndex && j!=yIndex) continue;
			if (j<0) continue;
			if (j>=m_height-1) continue;	
 			Int ndx = (j*m_width)+i;
			if (m_cliffInfoNdxes[ndx]) {
				TCliffInfo info = m_cliffInfo[m_cliffInfoNdxes[ndx]];
				okToProcess = true;
				if (pProcessed && !pProcessed[ndx]) okToProcess = false;
				if (info.mutant) okToProcess = false;
				if (!pProcessed && m_cliffInfoNdxes[ndx]) {
					if (curTileClass != getTextureClassFromNdx(info.tileIndex)) {
						okToProcess = false;
					}
				}
				if (!okToProcess) continue;
				Bool shifted = false;
				if (i==xIndex){
					if (j<yIndex) {
						// below 
						DEBUG_ASSERTCRASH(!lock1, ("Shouldn't happen."));
						if (lock0 && !usMatch(tmpCliff.u0, info.u3)) {
							shifted = true;
						}

						if (!shifted) {
							tmpCliff.u0 = info.u3;
							tmpCliff.u1 = info.u2;
							tmpCliff.v0 = info.v3;
							tmpCliff.v1 = info.v2;
							lock0 = lock1 = true;
						}
					} else {
						// above 
						DEBUG_ASSERTCRASH(!lock2, ("Shouldn't happen."));
						if (lock3 && !usMatch(tmpCliff.u3, info.u0)) {
							shifted = true;
						}
						if (!shifted) {
							tmpCliff.u3 = info.u0;
							tmpCliff.u2 = info.u1;
							tmpCliff.v3 = info.v0;
							tmpCliff.v2 = info.v1;
							lock2 = lock3 = true;
						}
					}
				}	else if (j==yIndex) {
					if (i<xIndex) {
						// left
						DEBUG_ASSERTCRASH(!lock0 && !lock3, ("Shouldn't happen."));
						tmpCliff.u0 = info.u1;
						tmpCliff.u3 = info.u2;
						tmpCliff.v0 = info.v1;
						tmpCliff.v3 = info.v2;
						lock0 = lock3 = true;
					} else {
						// right
						if (lock1 && !usMatch(tmpCliff.u1, info.u0)) shifted=true;
						if (lock2 && !usMatch(tmpCliff.u2, info.u3)) shifted=true;
						if (!shifted) {
							tmpCliff.u1 = info.u0;
							tmpCliff.u2 = info.u3;
							tmpCliff.v1 = info.v0;
							tmpCliff.v2 = info.v3;
							lock2 = lock1 = true;
						}
					}
				}
			}
		}
	}
	Int lockCount = lock0+lock1+lock2+lock3;
	if (lockCount < 2) return; // no adjustment needed.

	if (lockCount==4)	{
		Real minU = tmpCliff.u0;
		Real maxU = minU;
		const Real delta = (TEX_PER_CELL/100);
		if (minU>tmpCliff.u1) minU=tmpCliff.u1;
		if (maxU<tmpCliff.u1) maxU=tmpCliff.u1;
		if (minU>tmpCliff.u2) minU=tmpCliff.u2;
		if (maxU<tmpCliff.u2) maxU=tmpCliff.u2;
		if (minU>tmpCliff.u3) minU=tmpCliff.u3;
		if (maxU<tmpCliff.u3) maxU=tmpCliff.u3;
		if (maxU-minU<delta) {
			// bad. 
			tmpCliff.mutant = true;
			/* Pick the bigger v */
			if (tmpCliff.v3 - tmpCliff.v0 > tmpCliff.v2-tmpCliff.v0) {
				lock1 = lock2 = false;
			} else {
				lock3 = lock2 = false;
			}
			lockCount = 2;
		}
	}



	if (lockCount==4) {
		adjustForTiling(tmpCliff, textureClassExtent);
		tmpCliff.tileIndex = getFirstTile(curTileClass)<<2;
		Int cliffNdx=addCliffInfo(&tmpCliff);
		m_cliffInfoNdxes[yIndex*m_width + xIndex] = cliffNdx;
		return; // all were adjusted.
	}

	Vector2 xVec(0,0);
	Vector2 yVec(0,0);
	Bool gotXVec = false;
	Bool gotYVec = false;
	if (lock0 && lock1) {
		gotXVec = true;
		xVec.U = tmpCliff.u1 - tmpCliff.u0;
		xVec.V = tmpCliff.v1 - tmpCliff.v0;
	}
	if (lock1 && lock2) {
		gotYVec = true;
		yVec.U = tmpCliff.u2 - tmpCliff.u1;
		yVec.V = tmpCliff.v2 - tmpCliff.v1;
	}
	if (lock2 && lock3) {
		gotXVec = true;
		xVec.U = tmpCliff.u2 - tmpCliff.u3;
		xVec.V = tmpCliff.v2 - tmpCliff.v3;
	}
	if (lock3 && lock0) {
		gotYVec = true;
		yVec.U = tmpCliff.u3 - tmpCliff.u0;
		yVec.V = tmpCliff.v3 - tmpCliff.v0;
	}

	if (!gotXVec && !gotYVec) {
		DEBUG_LOG(("Unexpected.  jba\n"));
		return; 
	}
	if (gotXVec && !gotYVec) {
		yVec.V = -xVec.U;
		yVec.U = xVec.V;
	}
	if (gotYVec && !gotXVec) {
		xVec.V = yVec.U;
		xVec.U = -yVec.V;
	}


	if (!lock0) {
		if (lock1) {
			tmpCliff.u0 = tmpCliff.u1 - xVec.U;
			tmpCliff.v0 = tmpCliff.v1 - xVec.V;
		}	else if (lock3) {
			tmpCliff.u0 = tmpCliff.u3 - yVec.U;
			tmpCliff.v0 = tmpCliff.v3 - yVec.V;
		}
	}
	if (!lock1) {
		if (lock0) {
			tmpCliff.u1 = tmpCliff.u0 + xVec.U;
			tmpCliff.v1 = tmpCliff.v0 + xVec.V;
		}	else if (lock2) {
			tmpCliff.u1 = tmpCliff.u2 - yVec.U;
			tmpCliff.v1 = tmpCliff.v2 - yVec.V;
		}
	}
	if (!lock2) {
		if (lock3) {
			tmpCliff.u2 = tmpCliff.u3 + xVec.U;
			tmpCliff.v2 = tmpCliff.v3 + xVec.V;
		}	else if (lock1) {
			tmpCliff.u2 = tmpCliff.u1 + yVec.U;
			tmpCliff.v2 = tmpCliff.v1 + yVec.V;
		}
	}
	if (!lock3) {
		if (lock2) {
			tmpCliff.u3 = tmpCliff.u2 - xVec.U;
			tmpCliff.v3 = tmpCliff.v2 - xVec.V;
		}	else if (lock0) {
			tmpCliff.u3 = tmpCliff.u0 + yVec.U;
			tmpCliff.v3 = tmpCliff.v0 + yVec.V;
		}
	}
	tmpCliff.mutant = false;
	tmpCliff.tileIndex = getFirstTile(curTileClass)<<2;
	adjustForTiling(tmpCliff, textureClassExtent);
	Int cliffNdx=addCliffInfo(&tmpCliff);
	m_cliffInfoNdxes[yIndex*m_width + xIndex] = cliffNdx;
}


Int WorldHeightMapEdit::addCliffInfo(TCliffInfo *pCliffInfo)
{
	if (m_numCliffInfo>=NUM_CLIFF_INFO) {
		m_warnTooManyBlend = true;
		return 0;  // out of new cliffs
	}
	Short newNdx = m_numCliffInfo;
	m_cliffInfo[newNdx] = *pCliffInfo;
	m_numCliffInfo++;

	return(newNdx);

}

Int WorldHeightMapEdit::getNumBoundaries(void) const
{
	return m_boundaries.size();
}

void WorldHeightMapEdit::getBoundary(Int ndx, ICoord2D* border) const
{
	if (!border || ndx < 0 || ndx >= m_boundaries.size()) {
		DEBUG_CRASH(("Invalid border request. jkmcd"));
		return;
	}

	(*border) = m_boundaries[ndx];
}

void WorldHeightMapEdit::addBoundary(ICoord2D* boundaryToAdd)
{
	if (!boundaryToAdd) {
		DEBUG_CRASH(("Invalid border addition. jkmcd"));
		return;
	}

	m_boundaries.push_back((*boundaryToAdd));
}

void WorldHeightMapEdit::changeBoundary(Int ndx, ICoord2D *border)
{
	if (!border || ndx < 0 || ndx >= m_boundaries.size()) {
		DEBUG_CRASH(("Invalid border change request. jkmcd"));
		return;
	}

	m_boundaries[ndx] = (*border);
}

void WorldHeightMapEdit::removeLastBoundary(void)
{
	if (m_boundaries.size() == 0) {
		DEBUG_CRASH(("Invalid border remove request. jkmcd"));
		return;
	}
	
	m_boundaries.pop_back();
}

void WorldHeightMapEdit::findBoundaryNear(Coord3D *pt, float okDistance, Int *outNdx, Int *outHandle)
{
	if (!outNdx) {
		return;
	}

	if (!pt) {
		(*outNdx) = -1;
	}

	int numBoundaries = m_boundaries.size();
	for (int i = 0; i < numBoundaries; ++i) {
		Vector3 vecPt(pt->x / MAP_XY_FACTOR, pt->y / MAP_XY_FACTOR, 0);

		if (m_boundaries[i].x == 0 || m_boundaries[i].y == 0) {
			continue;
		}

		Vector3 vecTestPt(m_boundaries[i].x, m_boundaries[i].y, 0);
		if (Vector3::Distance(vecPt, vecTestPt) <= okDistance) {
			(*outNdx) = i;
			if (outHandle) {
				(*outHandle) = 2;
			}
			return;
		}

		vecTestPt.X = 0;
		if (Vector3::Distance(vecPt, vecTestPt) <= okDistance) {
			(*outNdx) = i;
			if (outHandle) {
				(*outHandle) = 1;
			}
			return;
		}

		vecTestPt.X = m_boundaries[i].x;
		vecTestPt.Y = 0;
		if (Vector3::Distance(vecPt, vecTestPt) <= okDistance) {
			(*outNdx) = i;
			if (outHandle) {
				(*outHandle) = 3;
			}
			return;
		}

		vecTestPt.X = 0;
		vecTestPt.Y = 0;
		if (Vector3::Distance(vecPt, vecTestPt) <= okDistance) {
			(*outNdx) = i;
			if (outHandle) {
				(*outHandle) = 0;
			}
			return;
		}
	}
	
	(*outNdx) = -1;
	(*outHandle) = -1;
} 