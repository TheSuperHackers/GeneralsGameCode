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

// TileTool.h
// Texture tiling tools for worldbuilder.
// Author: John Ahlquist, April 2001

#pragma once

#ifndef TILETOOL_H
#define TILETOOL_H

#define TILE_OPTION_PANEL "TileOptionPanel"

#include "Tool.h"
class WorldHeightMapEdit;
/*************************************************************************
**                             TileTool
***************************************************************************/
class TileTool : public Tool 
{
protected:

	// struct TerrainPreviewBuffer {
	// 	int startX, startY;
	// 	int width, height;
	// 	std::vector<std::vector<float>> heightData;
	// 	std::vector<std::vector<int>> textureData;
	// 	bool visible;
	// 	bool hasTexture;
	// };

	// static TerrainPreviewBuffer s_previewBuffer;

	struct TileTextureData {
		int xOffset;
		int yOffset;
		int textureClass;
		int blendTileNdx;
		int extraBlendTileNdx;
	};

	static std::vector<TileTextureData> m_copiedTileTextures;

	WorldHeightMapEdit *m_htMapEditCopy; //< ref counted.
	Int									m_textureClassToDraw;
	CPoint							m_prevViewPt;

public:
	TileTool(void);
	~TileTool(void);

public:
	// struct TBlendTileInfo;   // forward declare the blend tile info struct
	struct TerrainCopyBuffer {
		std::vector<std::vector<UnsignedByte> > heightData;   // Note the space before '>' for VC6
		// std::vector<std::vector<Int> > textureData;           // Same here
		Int width;
		Int height;
		// Bool hasTexture;
	};
	
	static TerrainCopyBuffer s_copyBuffer;

	virtual void mouseDown(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual void mouseUp(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual void mouseMoved(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual WorldHeightMapEdit *getHeightMap(void) {return m_htMapEditCopy;};
	virtual void activate(); ///< Become the current tool.
	virtual void deactivate();
	virtual Int getWidth(void) {return 1;};
	static void clearCopiedTiles(); ///< Called under open new document
	
	static void copyTerrainArea(WorldHeightMapEdit* pMap, Int startX, Int startY, Int size);
	static void applyTerrainArea(WorldHeightMapEdit* pMap, Int destX, Int destY);

	// ::TBlendTileInfo rotateBlendInfo(const ::TBlendTileInfo &info, int rotation);
};

/*************************************************************************
**                             BigTileTool
***************************************************************************/
class BigTileTool : public TileTool 
{

protected:
	static Int m_currentWidth;
	static Int m_currentHeight;
	static Int m_copyModeWidth; 

public:
 	virtual void activate(); ///< Become the current tool.

public:
	BigTileTool(void);

	static void setWidth(Int width) ;
	virtual Int getWidth(void) {return m_currentWidth;};
	static void setHeight(Int height) ;
	virtual Int getHeight(void) {return m_currentHeight;};

	// Used by TerrainMaterial dialog to get/set current width.
	static Int getCurrentHeight(void) {return m_currentHeight;};

	static Int getCopyModeWidth(void) {
		return ::AfxGetApp()->GetProfileInt(TILE_OPTION_PANEL, "CopyModeWidth", 20);
	}
	static Int getTileToolWidth(void) {
		return ::AfxGetApp()->GetProfileInt(TILE_OPTION_PANEL, "TileToolWidth", 3);
	}
	static void setCopyModeWidth(Int width) {
		::AfxGetApp()->WriteProfileInt(TILE_OPTION_PANEL, "CopyModeWidth", width);
	}
	static void setTileToolWidth(Int width) {
		::AfxGetApp()->WriteProfileInt(TILE_OPTION_PANEL, "TileToolWidth", width);
	}
};
#endif //TOOL_H
