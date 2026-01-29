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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "WW3D2/matpass.h"
#include "WW3D2/dx8wrapper.h"

class AABoxClass;
class WorldHeightMap;

typedef UnsignedByte W3DShroudLevel;
// In Global Data now
//const W3DShroudLevel	FULL_SHROUD_LEVEL=0;
//const W3DShroudLevel	FOG_SHROUD_LEVEL=127;
//const W3DShroudLevel	NO_SHROUD_LEVEL=255;

/** Custom W3D material pass which has been modified to apply
	a shroud texture projection.
*/
class W3DShroudMaterialPassClass : public MaterialPassClass
{
public:
	W3DShroudMaterialPassClass(void) : m_isTransparentObjectPass(FALSE) {}
	virtual void	Install_Materials(void) const;
	virtual void	UnInstall_Materials(void) const;
	void enableTransparentObjectPass(Bool enable) {m_isTransparentObjectPass = enable;}
protected:
	//customized version to deal with transparent (alpha-tested) polys.
	Bool m_isTransparentObjectPass;
};

/** Custom W3D material pass which has been modified to apply
	a texture projection.  Similar to the above code except
	that it's more generic so could be used for non-shroud
	projection of various masks.
*/
class W3DMaskMaterialPassClass : public MaterialPassClass
{
public:
	W3DMaskMaterialPassClass(void) : m_texture(nullptr), m_allowUninstall(TRUE) {}
	virtual void	Install_Materials(void) const;
	virtual void	UnInstall_Materials(void) const;
	void	setTexture(TextureClass *texture)	{m_texture=texture;}
	void	setAllowUninstall(Bool state)	{ m_allowUninstall = state;}

protected:
	TextureClass *m_texture;	///<texture to be projected.
	Bool m_allowUninstall;		///<flag which allows uninstalling this material.
};

/** Terrain shroud rendering class */
class W3DShroud
{

public:
	W3DShroud(void);
	~W3DShroud(void);

	void render(CameraClass *cam);	///< render the current shroud state as seen from camera
	void init(WorldHeightMap *pMap, Real worldCellSizeX, Real worldCellSizeY);
	void reset(void);
	TextureClass *getShroudTexture(void) { return m_pDstTexture;}	//<return shroud projection texture.
	void ReleaseResources(void);	///<release resources that can't survive D3D device reset.
	Bool ReAcquireResources(void);	///<allocate resources that can't survive D3D device reset.
	void fillShroudData(W3DShroudLevel level);	///<sets the state of the current shroud to some constant value
	Int	getNumShroudCellsY(void)	{return m_numCellsY;}
	Int getNumShroudCellsX(void)	{return m_numCellsX;}
	Real getCellWidth(void)			{return m_cellWidth;}	///<world-space width (x) of each shroud cell.
	Real getCellHeight(void)		{return m_cellHeight;}	///<world-space height (y)of each shroud cell.
	Int	 getTextureWidth(void)		{return m_dstTextureWidth;}	///<internal use by the shader system.
	Int	 getTextureHeight(void)		{return m_dstTextureHeight;}
	W3DShroudLevel getShroudLevel(Int x, Int y);
	void setShroudLevel(Int x, Int y, W3DShroudLevel,Bool textureOnly=FALSE);
	void setShroudFilter(Bool enable);	///<turns on bilinear filtering of shroud cells.
	void setBorderShroudLevel(W3DShroudLevel level);	///<color that will appear in unused border terrain.
	Real	getDrawOriginX(void)	{return m_drawOriginX;}	///<returns ws origin of first pixel in shroud texture.
	Real	getDrawOriginY(void)	{return m_drawOriginY;}	///<returns ws origin of first pixel in shroud texture.

protected:
	Int m_numCellsX;						///<number of cells covering entire map
	Int m_numCellsY;						///<number of cells covering entire map
	Int m_numMaxVisibleCellsX;				///<maximum number of shroud cells that can be visible
	Int m_numMaxVisibleCellsY;				///<maximum number of shroud cells that can be visible
	Real m_cellWidth;						///<spacing between adjacent cells
	Real m_cellHeight;						///<spacing between adjacent cells
	Byte *m_shroudData;						///<holds amount of shroud per cell.
	IDirect3DSurface8 *m_pSrcTexture;		///<stores sysmem copy of visible shroud.
	void *m_srcTextureData;					///<pointer to shroud data
	UnsignedInt m_srcTexturePitch;			///<width (in bytes) of shroud data buffer.
	TextureClass *m_pDstTexture;			///<stores vidmem copy of visible shroud.
	Int m_dstTextureWidth;					///<dimensions of m_pDstTexture
	Int m_dstTextureHeight;					///<dimensions of m_pDstTexture
	TextureFilterClass::FilterType m_shroudFilter;
	Real m_drawOriginX;
	Real m_drawOriginY;
	Bool m_drawFogOfWar;					///<switch to draw alternate fog style instead of solid black
	Bool m_clearDstTexture;				///<flag indicating we must clear video memory destination texture
	W3DShroudLevel m_boderShroudLevel;			///<color used to clear the shroud border
	W3DShroudLevel *m_finalFogData;			///<copy of logical shroud in an easier to access array.
	W3DShroudLevel *m_currentFogData;		///<copy of intermediate logical shroud while it's interpolated.
	void interpolateFogLevels(RECT *rect);		///<fade current fog levels to actual logic side levels.
	void fillBorderShroudData(W3DShroudLevel level, SurfaceClass* pDestSurface);	///<fill the destination texture with a known value
};
