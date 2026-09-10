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

// TerrainTex.h
// Class to generate texture for terrain.
// Author: John Ahlquist, April 2001

#pragma once

//#define DO_8STAGE_TERRAIN_PASS		//optimized terrain rendering for Nvidia based cards

#include "W3DDevice/GameClient/W3DTextureHandle.h"
#include "WWMath/matrix3d.h"
#include "Common/AsciiString.h"
#include "W3DDevice/GameClient/TileData.h"

class WorldHeightMap;
#define TILE_OFFSET 8
#define TERRAIN_TILE_PIXEL_EXTENT TILE_CACHE_PIXEL_EXTENT
#define TERRAIN_TILE_OFFSET 32
#define TERRAIN_TEXTURE_WIDTH 8192
/** ***********************************************************************
**                             TerrainTextureClass
***************************************************************************/
class TerrainTextureClass : public W3DTextureHandle
{
	W3DMPO_CODE(TerrainTextureClass)
protected:
	virtual bool Recreate_Procedural_Texture() override;

	WorldHeightMap *m_sourceHeightMap;
	bool m_isFlatTexture;
	Int m_flatXCell;
	Int m_flatYCell;
	Int m_flatCellWidth;
	Int m_flatPixelsPerCell;

public:
		/// Create texture for a height map.
		TerrainTextureClass(int height);

		/// Create texture for a height map.
		TerrainTextureClass(int height, int width);

		// just use default destructor. ~TerrainTextureClass();
public:
	int update(WorldHeightMap *htMap); ///< Sets the pixels, and returns the actual height of the texture.
	Bool updateFlat(WorldHeightMap *htMap, Int xCell, Int yCell, Int cellWidth, Int pixelsPerCell); ///< Sets the pixels.
	void Clear_Source_Height_Map() { m_sourceHeightMap = nullptr; }
};


class AlphaTerrainTextureClass : public W3DTextureHandle
{
	W3DMPO_CODE(AlphaTerrainTextureClass)
protected:
		virtual bool Recreate_Procedural_Texture() override;

	W3DTextureHandle *m_baseTexture;

public:
		// Create texture for a height map.
		AlphaTerrainTextureClass(W3DTextureHandle *pBaseTex );
		virtual ~AlphaTerrainTextureClass() override;

		// just use default destructor. ~TerrainTextureClass();

};

/** ***********************************************************************
**                             AlphaEdgeTextureClass
***************************************************************************/
class AlphaEdgeTextureClass : public W3DTextureHandle
{
	W3DMPO_CODE(AlphaEdgeTextureClass)
protected:
	virtual bool Recreate_Procedural_Texture() override;
	int update256(WorldHeightMap *htMap);///< Sets the pixels, and returns the actual height of the texture.

	WorldHeightMap *m_sourceHeightMap;

public:
		/// Create texture for a height map.
		AlphaEdgeTextureClass(int height, MipCountType mipLevelCount = MIP_LEVELS_3 );

		// just use default destructor. ~TerrainTextureClass();
public:
	int update(WorldHeightMap *htMap); ///< Sets the pixels, and returns the actual height of the texture.
	void Clear_Source_Height_Map() { m_sourceHeightMap = nullptr; }

};

class LightMapTerrainTextureClass : public W3DTextureHandle
{
	W3DMPO_CODE(LightMapTerrainTextureClass)
protected:

public:
		// Create texture from a height map.
		LightMapTerrainTextureClass( AsciiString name, MipCountType mipLevelCount = MIP_LEVELS_ALL );

		// just use default destructor.
};

class ScorchTextureClass : public W3DTextureHandle
{
	W3DMPO_CODE(ScorchTextureClass)
protected:

public:
		// Create texture.
		ScorchTextureClass( MipCountType mipLevelCount = MIP_LEVELS_3 );

		// just use default destructor. ~ScorchTextureClass();
};

class CloudMapTerrainTextureClass : public W3DTextureHandle
{
	W3DMPO_CODE(CloudMapTerrainTextureClass)
protected:

protected:
		float m_xSlidePerSecond ;	 ///< How far the clouds move per second.
		float m_ySlidePerSecond ;	 ///< How far the clouds move per second.
		int	  m_curTick;
		float m_xOffset;
		float m_yOffset;


public:
		// Create texture from a height map.
		CloudMapTerrainTextureClass( MipCountType mipLevelCount = MIP_LEVELS_ALL );

		// just use default destructor. ~TerrainTextureClass();

		void Update_Animation(float frame_seconds);
		float Get_X_Offset() const { return m_xOffset; }
		float Get_Y_Offset() const { return m_yOffset; }
};
