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

#pragma once

#include <memory>

#include "W3DDevice/GameClient/W3DRenderContext.h"
import Graphics.Resources.Textures.Sampling;
import Graphics.Resources.Textures.Edit;
import Assets.Images.Buffer;

class AABoxClass;
class WorldHeightMap;

typedef UnsignedByte W3DShroudLevel;
// In Global Data now
//const W3DShroudLevel	FULL_SHROUD_LEVEL=0;
//const W3DShroudLevel	FOG_SHROUD_LEVEL=127;
//const W3DShroudLevel	NO_SHROUD_LEVEL=255;

std::shared_ptr<NativeMaterialPass> Create_W3D_Shroud_Material_Pass();
std::shared_ptr<NativeMaterialPass> Create_W3D_Mask_Material_Pass();

/** Terrain shroud rendering class */
class W3DShroud
{

public:
	W3DShroud();
	~W3DShroud();

	void render(W3DCamera *cam);	///< render the current shroud state as seen from camera
	void init(WorldHeightMap *pMap, Real worldCellSizeX, Real worldCellSizeY);
	void reset();
	W3DTextureHandle *getShroudTexture() { return m_pDstTexture;}	//<return shroud projection texture.
	void ReleaseResources();	///<release resources that can't survive D3D device reset.
	Bool ReAcquireResources();	///<allocate resources that can't survive D3D device reset.
	void fillShroudData(W3DShroudLevel level);	///<sets the state of the current shroud to some constant value
	Int	getNumShroudCellsY()	{return m_numCellsY;}
	Int getNumShroudCellsX()	{return m_numCellsX;}
	Real getCellWidth()			{return m_cellWidth;}	///<world-space width (x) of each shroud cell.
	Real getCellHeight()		{return m_cellHeight;}	///<world-space height (y)of each shroud cell.
	Int	 getTextureWidth()		{return m_dstTextureWidth;}	///<internal use by the shader system.
	Int	 getTextureHeight()		{return m_dstTextureHeight;}
	W3DShroudLevel getShroudLevel(Int x, Int y);
	void setShroudLevel(Int x, Int y, W3DShroudLevel,Bool textureOnly=FALSE);
	void setShroudFilter(Bool enable);	///<turns on bilinear filtering of shroud cells.
	void setBorderShroudLevel(W3DShroudLevel level);	///<color that will appear in unused border terrain.
	Real	getDrawOriginX()	{return m_drawOriginX;}	///<returns ws origin of first pixel in shroud texture.
	Real	getDrawOriginY()	{return m_drawOriginY;}	///<returns ws origin of first pixel in shroud texture.

protected:
	Int m_numCellsX;						///<number of cells covering entire map
	Int m_numCellsY;						///<number of cells covering entire map
	Int m_numMaxVisibleCellsX;				///<maximum number of shroud cells that can be visible
	Int m_numMaxVisibleCellsY;				///<maximum number of shroud cells that can be visible
	Real m_cellWidth;						///<spacing between adjacent cells
	Real m_cellHeight;						///<spacing between adjacent cells
	Byte *m_shroudData;						///<holds amount of shroud per cell.
	Graphics::TextureEdit *m_pSrcTexture;		///<stores sysmem copy of visible shroud.
	void *m_srcTextureData;					///<pointer to shroud data
	UnsignedInt m_srcTexturePitch;			///<width (in bytes) of shroud data buffer.
	W3DTextureHandle *m_pDstTexture;			///<stores vidmem copy of visible shroud.
	Int m_dstTextureWidth;					///<dimensions of m_pDstTexture
	Int m_dstTextureHeight;					///<dimensions of m_pDstTexture
	Graphics::SamplingFilter m_shroudFilter;
	Real m_drawOriginX;
	Real m_drawOriginY;
	Bool m_drawFogOfWar;					///<switch to draw alternate fog style instead of solid black
	Bool m_clearDstTexture;				///<flag indicating we must clear video memory destination texture
	W3DShroudLevel m_boderShroudLevel;			///<color used to clear the shroud border
	W3DShroudLevel *m_finalFogData;			///<copy of logical shroud in an easier to access array.
	W3DShroudLevel *m_currentFogData;		///<copy of intermediate logical shroud while it's interpolated.
	void interpolateFogLevels(const Assets::ImageRegion *rect);		///<fade current fog levels to actual logic side levels.
	void fillBorderShroudData(W3DShroudLevel level);
	struct GraphicsState;
	std::unique_ptr<GraphicsState> m_graphics;
};
