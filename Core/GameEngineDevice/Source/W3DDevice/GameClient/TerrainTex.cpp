import Assets.Images.PixelEncoding;
import Graphics.RHI;
import Graphics.Resources.Textures.Atlas;
#include <cstdint>
#include <vector>

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

// FILE: TerrainTex.cpp ////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
// Project:   RTS3
//
// File name: TerrainTex.cpp
//
// Created:   John Ahlquist, April 2001
//
// Desc:      W3DTextureHandle overrides to perform custom texturing for the terrain.
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//         Includes
//-----------------------------------------------------------------------------
#include <stdlib.h>
#include <cstring>

#include "W3DDevice/GameClient/TerrainTex.h"
#include "W3DDevice/GameClient/WorldHeightMap.h"
#include "W3DDevice/GameClient/TileData.h"
#include "Common/GlobalData.h"

namespace
{
Graphics::AtlasTile Atlas_Tile(const UnsignedByte* pixels,unsigned extent,unsigned x,unsigned y)
{
    return {{{reinterpret_cast<const std::byte*>(pixels),std::size_t(extent)*extent*4},
        extent,extent,std::size_t(extent)*4,Assets::PixelEncoding::BGRA8},x,y,true};
}
}

/******************************************************************************
						TerrainTextureClass
******************************************************************************/
//-----------------------------------------------------------------------------
//         Public Functions
//-----------------------------------------------------------------------------

//=============================================================================
// TerrainTextureClass::TerrainTextureClass
//=============================================================================
/** Constructor. Calls parent constructor to create a 16 bit per pixel
texture of the desired height and mip level. */
//=============================================================================
TerrainTextureClass::TerrainTextureClass(int height) :
	W3DTextureHandle(TERRAIN_TEXTURE_WIDTH, height,
		Assets::PixelEncoding::BGRA5551, MIP_LEVELS_3 ),
	m_sourceHeightMap(nullptr),
	m_isFlatTexture(false),
	m_flatXCell(0),
	m_flatYCell(0),
	m_flatCellWidth(0),
	m_flatPixelsPerCell(0)
{
}

//=============================================================================
// TerrainTextureClass::TerrainTextureClass
//=============================================================================
/** Constructor. Calls parent constructor to create a 16 bit per pixel
texture of the desired height and mip level. */
//=============================================================================
TerrainTextureClass::TerrainTextureClass(int height, int width) :
	W3DTextureHandle(width, height,
		Assets::PixelEncoding::BGRA5551, MIP_LEVELS_1 ),
	m_sourceHeightMap(nullptr),
	m_isFlatTexture(true),
	m_flatXCell(0),
	m_flatYCell(0),
	m_flatCellWidth(0),
	m_flatPixelsPerCell(0)
{
}

bool TerrainTextureClass::Recreate_Procedural_Texture()
{
	if (!W3DTextureHandle::Recreate_Procedural_Texture()) {
		return false;
	}

	const bool populated = m_isFlatTexture
		? updateFlat(m_sourceHeightMap, m_flatXCell, m_flatYCell, m_flatCellWidth, m_flatPixelsPerCell) != 0
		: update(m_sourceHeightMap) != 0;
	if (!populated) {
		Set_Render_Backend_Texture(0);
	}
	return populated;
}


//=============================================================================
// TerrainTextureClass::update
//=============================================================================
/** Sets the tile bitmap data into the texture.  The tiles are placed with 4
	pixel borders around them, so that when the tiles are scaled and bilinearly
	interpolated, you don't get seams between the tiles.  */
//=============================================================================
int TerrainTextureClass::update(WorldHeightMap *htMap)
{
    if (!htMap) return 0;
    m_sourceHeightMap=htMap; m_isFlatTexture=false;
    auto* texture=Peek_Render_Backend_Texture();
    if (!texture || texture->Description().width<TERRAIN_TEXTURE_WIDTH) return 0;
    std::vector<Graphics::AtlasTile> tiles;
    std::vector<Graphics::AtlasRepeatBorder> borders;
    for (Int i=0;i<htMap->m_numBitmapTiles;++i) {
        auto* tile=htMap->getSourceTile(i);
        if (!tile || tile->m_tileLocationInTexture.x<=0) continue;
        const auto position=tile->m_tileLocationInTexture;
        tiles.push_back(Atlas_Tile(tile->getRGBDataForWidth(TERRAIN_TILE_PIXEL_EXTENT),
            TERRAIN_TILE_PIXEL_EXTENT,position.x,position.y));
    }
    for (Int i=0;i<htMap->m_numTextureClasses;++i) {
        const auto& source=htMap->m_textureClasses[i];
        if (source.positionInTexture.x<=0) continue;
        const unsigned width=source.width*TERRAIN_TILE_PIXEL_EXTENT;
        borders.push_back({static_cast<unsigned>(source.positionInTexture.x),
            static_cast<unsigned>(source.positionInTexture.y),width,width,TERRAIN_TILE_OFFSET/2});
    }
    return Graphics::Upload_Texture_Atlas(*texture,tiles,borders,Graphics::AtlasAlpha::Source,
        Graphics::AtlasBackground::Opaque,true) ? static_cast<int>(texture->Description().height) : 0;
}


//=============================================================================
// TerrainTextureClass::update
//=============================================================================
/** Sets the tile bitmap data into the texture.  The tiles are placed with 4
	pixel borders around them, so that when the tiles are scaled and bilinearly
	interpolated, you don't get seams between the tiles.  */
//=============================================================================
Bool TerrainTextureClass::updateFlat(WorldHeightMap *htMap, Int xCell, Int yCell, Int cellWidth, Int pixelsPerCell)
{
    if (!htMap || cellWidth<=0 || pixelsPerCell<=0) return false;
    m_sourceHeightMap=htMap; m_isFlatTexture=true;
    m_flatXCell=xCell; m_flatYCell=yCell; m_flatCellWidth=cellWidth; m_flatPixelsPerCell=pixelsPerCell;
    auto* texture=Peek_Render_Backend_Texture();
    const auto extent=std::uint64_t(cellWidth)*pixelsPerCell;
    if (!texture || texture->Description().width!=extent || texture->Description().height!=extent) return false;
    std::vector<Graphics::AtlasTile> tiles;
    // The map's tile reader reuses scratch storage. Snapshot before reading the next cell.
    const std::size_t tile_bytes=std::size_t(pixelsPerCell)*pixelsPerCell*4;
    std::vector<std::byte> pixels(std::size_t(cellWidth)*cellWidth*tile_bytes);
    for (Int x=0;x<cellWidth;++x) for (Int y=0;y<cellWidth;++y) {
        const auto* source=htMap->getPointerToTileData(xCell+x,yCell+y,pixelsPerCell);
        if (!source) continue;
        auto* destination=pixels.data()+(std::size_t(x)*cellWidth+y)*tile_bytes;
        std::memcpy(destination,source,tile_bytes);
        tiles.push_back(Atlas_Tile(reinterpret_cast<const UnsignedByte*>(destination),pixelsPerCell,
            x*pixelsPerCell,(cellWidth-y-1)*pixelsPerCell));
    }
    return Graphics::Upload_Texture_Atlas(*texture,tiles,{},Graphics::AtlasAlpha::Source,
        Graphics::AtlasBackground::Opaque,false);
}


/******************************************************************************
						AlphaTerrainTextureClass
******************************************************************************/
//-----------------------------------------------------------------------------
//         Public Functions
//-----------------------------------------------------------------------------

//=============================================================================
// AlphaTerrainTextureClass::AlphaTerrainTextureClass
//=============================================================================
/** Constructor. Calls parent constructor to creat a throw away 8x8 texture,
then shares the base texture. This way the base tiles pass, drawn
using TerrainTextureClass shares the same texture with the blended edges pass,
saving lots of texture memory, and preventing seams between blended tiles. */
//=============================================================================
AlphaTerrainTextureClass::AlphaTerrainTextureClass( W3DTextureHandle *pBaseTex ):
	W3DTextureHandle(8, 8,
		Assets::PixelEncoding::BGRA5551, MIP_LEVELS_1, W3DTextureHandle::POOL_DEFAULT ),
	m_baseTexture(nullptr)
{
	// Keep the scoped registration from the parent. Its recreation callback
	// dispatches to this alias's override, which retains the base atlas.
	Set_Render_Backend_Texture(0);

	REF_PTR_SET(m_baseTexture, pBaseTex);
	if (m_baseTexture != nullptr) {
		m_baseTexture->Ensure_Render_Backend_Texture();
	}

	// Share the base texture's backend resource.
	Set_Render_Backend_Texture(
		Graphics::Retain_Texture_Resource(
			m_baseTexture != nullptr ? m_baseTexture->Peek_Render_Backend_Texture() : 0));
	Residency().Set_Initialized(Peek_Render_Backend_Texture() != 0);
}

AlphaTerrainTextureClass::~AlphaTerrainTextureClass()
{
	REF_PTR_RELEASE(m_baseTexture);
}

bool AlphaTerrainTextureClass::Recreate_Procedural_Texture()
{
	if (m_baseTexture == nullptr || !m_baseTexture->Ensure_Render_Backend_Texture()) {
		return false;
	}

	Graphics::TextureResource* const base_texture = m_baseTexture->Peek_Render_Backend_Texture();
	if (base_texture == 0) {
		return false;
	}

	Set_Render_Backend_Texture(Graphics::Retain_Texture_Resource(base_texture));
	return Peek_Render_Backend_Texture() != 0;
}



/******************************************************************************
						LightMapTerrainTextureClass
******************************************************************************/
//-----------------------------------------------------------------------------
//         Public Functions
//-----------------------------------------------------------------------------

//=============================================================================
// LightMapTerrainTextureClass::LightMapTerrainTextureClass
//=============================================================================
/** Constructor. Calls parent constructor to load the .tga texture. */
//=============================================================================
LightMapTerrainTextureClass::LightMapTerrainTextureClass(AsciiString name, MipCountType mipLevelCount) :
W3DTextureHandle(name.isEmpty()?"TSNoiseUrb.tga":name.str(),name.isEmpty()?"TSNoiseUrb.tga":name.str(), mipLevelCount )
{
	Get_Sampling().minification = Graphics::SamplingFilter::Best;
	Get_Sampling().magnification = Graphics::SamplingFilter::Best;
	Get_Sampling().address[0] = Graphics::RHISamplerAddress::Wrap;
	Get_Sampling().address[1] = Graphics::RHISamplerAddress::Wrap;
}

#define STRETCH_FACTOR ((float)(1/(63.0*MAP_XY_FACTOR/2))) /* covers 63/2 tiles */










/******************************************************************************
						AlphaEdgeTextureClass
******************************************************************************/
//-----------------------------------------------------------------------------
//         Public Functions
//-----------------------------------------------------------------------------

/**
* AlphaEdgeTextureClass - Generates the alpha edge blending for terrain.
*
*/
AlphaEdgeTextureClass::AlphaEdgeTextureClass( int height, MipCountType mipLevelCount) :
//	W3DTextureHandle("EdgingTemplate.tga","EdgingTemplate.tga", mipLevelCount )
	W3DTextureHandle(TERRAIN_TEXTURE_WIDTH, height, Assets::PixelEncoding::BGRA8, mipLevelCount ),
	m_sourceHeightMap(nullptr)
{

}

int AlphaEdgeTextureClass::update256(WorldHeightMap *htMap)
{
	return 1;
}

int AlphaEdgeTextureClass::update(WorldHeightMap *htMap)
{
    if (!htMap) return 0;
    m_sourceHeightMap=htMap;
    auto* texture=Peek_Render_Backend_Texture();
    if (!texture) return 0;
    std::vector<Graphics::AtlasTile> tiles;
    for (Int i=0;i<htMap->m_numEdgeTiles;++i) {
        auto* tile=htMap->getEdgeTile(i);
        if (!tile || tile->m_tileLocationInTexture.x<=0) continue;
        const auto position=tile->m_tileLocationInTexture;
        tiles.push_back(Atlas_Tile(tile->getRGBDataForWidth(TERRAIN_TILE_PIXEL_EXTENT),
            TERRAIN_TILE_PIXEL_EXTENT,position.x,position.y));
    }
    return Graphics::Upload_Texture_Atlas(*texture,tiles,{},Graphics::AtlasAlpha::EdgeMask,
        Graphics::AtlasBackground::EdgeGradient,true) ? static_cast<int>(texture->Description().height) : 0;
}

bool AlphaEdgeTextureClass::Recreate_Procedural_Texture()
{
	if (!W3DTextureHandle::Recreate_Procedural_Texture()) {
		return false;
	}

	if (update(m_sourceHeightMap) == 0) {
		Set_Render_Backend_Texture(0);
		return false;
	}
	return true;
}



/******************************************************************************
						CloudMapTerrainTextureClass
******************************************************************************/
//-----------------------------------------------------------------------------
//         Public Functions
//-----------------------------------------------------------------------------

//=============================================================================
// CloudMapTerrainTextureClass::CloudMapTerrainTextureClass
//=============================================================================
/** Constructor. Calls parent constructor to load the .tga texture, and sets
up the "sliding" parameters for the clouds to slide over the terrain. */
//=============================================================================
//@todo - Allow adjustment of the cloud slide rate, and lose the hard coded "cloudmap.tga"
CloudMapTerrainTextureClass::CloudMapTerrainTextureClass(MipCountType mipLevelCount) :
	W3DTextureHandle("TSCloudMed.tga","TSCloudMed.tga", mipLevelCount )
{
	Get_Sampling().mipmap =  Graphics::SamplingFilter::Fast ;
	m_xSlidePerSecond = -0.02f;
	m_ySlidePerSecond =  1.50f * m_xSlidePerSecond;
	m_curTick = 0;
	m_xOffset = 0;
	m_yOffset = 0;

}

void CloudMapTerrainTextureClass::Update_Animation(float frame_seconds)
{
	m_xOffset += m_xSlidePerSecond * frame_seconds;
	m_yOffset += m_ySlidePerSecond * frame_seconds;

	// Keep the projected cloud coordinates in one texture period. The state is
	// owned by the cloud texture consumed by the terrain material.
	m_xOffset -= static_cast<Int>(m_xOffset);
	m_yOffset -= static_cast<Int>(m_yOffset);
}


/******************************************************************************
						ScorchTextureClass
******************************************************************************/
//-----------------------------------------------------------------------------
//         Public Functions
//-----------------------------------------------------------------------------

//=============================================================================
// ScorchTextureClass::ScorchTextureClass
//=============================================================================
/** Constructor. Calls parent constructor to load the .tga texture. */
//=============================================================================
/// @todo - get "EXScorch01.tga" from not hard coded location.
ScorchTextureClass::ScorchTextureClass(MipCountType mipLevelCount) :
	W3DTextureHandle("EXScorch01.tga","EXScorch01.tga", mipLevelCount )
// Hack to disable texture reduction.
//	W3DTextureHandle("EXScorch01.tga","EXScorch01.tga", mipLevelCount,Assets::PixelEncoding::Unknown,true,false)
{
}
