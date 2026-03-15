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

// WHeightMapEdit.h
// Class to contain the editing functions subclass of WorldHeightMap.
// Author: John Ahlquist, April 2001

#pragma once

#include "W3DDevice/GameClient/WorldHeightMap.h"

class DataChunkOutput;
class TerrainType;

// helper class
class CProcessNode
{
public:
	const Int m_x, m_y;
	Real m_len; ///< Length of texture coord on this node.
	CProcessNode *m_next;
public:
	CProcessNode(Int x, Int y):m_x(x),m_y(y),m_next(nullptr),m_len(0) {};
	~CProcessNode(void) { };
};

#define MAX_TILES_PER_CLASS 100
/// Struct in memory.
typedef struct
{
#ifdef DEBUG_CRASHING
	Int forDebugOnly_fileTextureClass;
#endif
	Int numTiles;
	Int width;
	Bool isBlendEdgeTile;
	AsciiString name;
	AsciiString filePath;
	AsciiString uiName;
	TileData *tiles[MAX_TILES_PER_CLASS];
	TerrainType *terrainType;
} TGlobalTextureClass;


#define NUM_ALPHA_TILES 8

class WorldHeightMapEdit : public WorldHeightMap
{
protected:
	Bool m_warnTooManyTex;  ///< warning message flag.
	Bool m_warnTooManyBlend;  ///< warning message flag.

	// Texture classes.  There is one texture class for each bitmap read in.
	// A class may have more than one tile.  For example, if the grass bitmap is
	// 128x128, it creates 4 64x64 tiles, so the grass texture class will have 4 tiles.
	static int m_numGlobalTextureClasses;
	static TGlobalTextureClass m_globalTextureClasses[NUM_TEXTURE_CLASSES];

protected:
	static void loadBitmap(char *path, const char *uiName);
	static void loadDirectoryOfImages(const char *path);
	static void loadImagesFromTerrainType( TerrainType *terrain );
	static void loadBaseImages(void);
	Int allocateTiles(Int textureClass);
	Int allocateEdgeTiles(Int textureClass);
	void blendToThisClass(Int xIndex, Int yIndex, Int textureClass, Int edgeClass);
	void blendSpecificTiles(Int xIndex, Int yIndex, Int srcXIndex, Int srcYIndex,
									Int curTileNdx, Int blendTileNdx, Bool longDiagonal, Int edgeClass);
	Int findOrCreateBlendTile(TBlendTileInfo *pBlendInfo);
	Int addCliffInfo(TCliffInfo *pCliffInfo);
	Int getTileIndexFromTerrainType( TerrainType *terrain );
	Int getTileNdxForClass(Int xIndex, Int yIndex, Int textureClass);
	Int getBlendTileNdxForClass(Int xIndex, Int yIndex, Int textureClass);
	Int getTextureClassFromNdx(Int tileNdx);
	void getTexClassNeighbors(Int xIndex, Int yIndex, Int textureClass, Int *pSideCount, Int *pTotalCount);
	void updateForAdjacentCliffs(Int xIndex, Int yIndex,
								UnsignedByte *pProcessed, TCliffInfo &cliffInfo);
	Bool adjustForTiling(TCliffInfo &cliffInfo, Real textureWidth);
	void updateFlatCellForAdjacentCliffs(Int xIndex, Int yIndex,
								Int curTileClass, UnsignedByte *pProcessed=nullptr);

public: // construction
	WorldHeightMapEdit(Int xExtent, Int yExtent, UnsignedByte initialHeight, Int border); ///< create.
	WorldHeightMapEdit(WorldHeightMapEdit *pThis);								///< duplicate.
	WorldHeightMapEdit(ChunkInputStream *pStrm);											///< read from file.
	~WorldHeightMapEdit(void);													    ///< destroy.

	void saveToFile(DataChunkOutput &chunkWriter);
	WorldHeightMapEdit *duplicate(void);

	static void init(void);
	static void shutdown(void);

public: /// Status methods.
	void clearStatus(void) {m_warnTooManyTex = false;m_warnTooManyBlend = false;};
	Bool tooManyTextures(void) {return m_warnTooManyTex;};
	Bool tooManyBlends(void) {return m_warnTooManyBlend;};
	Bool canFitTexture(Int textureClass);  ///< Returns true if we can fit this texture.

public: // Editing methods.
	static UnsignedByte *getPointerToClassTileData(Int texClass);

	void blendTile(Int xIndex, Int yIndex, Int srcXIndex, Int srcYIndex, Int srcClass, Int edgeClass);
	void autoBlendOut(Int xIndex, Int yIndex, Int edgeIndex = -1);
	Int getTextureClass(Int xIndex, Int yIndex, Bool baseClass=false);
	void setHeight(Int xIndex, Int yIndex, UnsignedByte height);
	void setCliff(Int xIndex, Int yIndex, Bool impassable) {setCliffState(xIndex, yIndex, impassable);}
	Bool setTileNdx(Int xIndex, Int yIndex, Int textureClass, Bool singleTile);
	Bool floodFill(Int xIndex, Int yIndex, Int textureClass, Bool doReplace);
	static Int getNumTexClasses(void) {return m_numGlobalTextureClasses;};
	static AsciiString getTexClassName(int ndx) {return m_globalTextureClasses[ndx].name;}
	static AsciiString getTexClassUiName(int ndx) ;
	static Int getTexClassNumTiles(int ndx) {return m_globalTextureClasses[ndx].numTiles;}
	static Int getTexClassIsBlendEdge(int ndx) {return m_globalTextureClasses[ndx].isBlendEdgeTile;}

	void addObject(MapObject *pMapObj);  ///< Adds a map object to the front of the list.
	void removeFirstObject(void);	 ///< Removes the first map object from the list.

	Bool isTexClassUsed(Int textureClass);
	Int getFirstTile(Int textureClass);

	Bool optimizeTiles(void); ///< Optimizes tile allocations.

	void showTileStatusInfo(void); ///< pops up a dialog box with tile mem usage.


	Bool selectDuplicates(void); ///< Selects any dupicate map objects.
	Bool selectSimilar(void); ///< Selects any dupicate map objects.
	Bool selectInvalidTeam(void); ///< Selects any objects with invalid teams.

	Bool resize(Int newXSize, Int newYSize, Int newHeight, Int newBorder, Bool anchorTop, Bool anchorBottom,
							Bool anchorLeft, Bool anchorRight, Coord3D *pObjOffset);
	Bool remapTextures(void); ///< returns true if the operation had an effect.
	void reloadTextures(void); ///< Reloads textures from disk.
	void resetResources(void); ///< Releases textures in preparation for device reset.

	Bool getRawTileData(Short tileNdx, Int width, UnsignedByte *buffer, Int bufLen);

	void dbgVerifyAfterUndo(void); ///< Verifies the structures are still consistent.
	Bool doCliffAdjustment(Int xIndex, Int yIndex);
	Bool removeCliffMapping(void);

	Int getNumBoundaries(void) const ;
	void getBoundary(Int ndx, ICoord2D* border) const;
	void addBoundary(ICoord2D* boundaryToAdd);
	void changeBoundary(Int ndx, ICoord2D *border);
	void removeLastBoundary(void);

	// outNdx must not be null, but outHandle can be.
	// outHandle: 0 means BL, 1 means TL, 2 means TR, 3 means BR
	void findBoundaryNear(Coord3D *pt, float okDistance, Int *outNdx, Int *outHandle);
};
