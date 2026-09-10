#include <functional>
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

// FILE: W3DShroud.cpp /////////////////////////////////////////////////////////////////////////////
// Created:   Mark Wilczynski, Jan 2002
// Desc:      Code to support rendering of shrouded units/terrain.
///////////////////////////////////////////////////////////////////////////////////////////////////

import Assets.Images.PixelEncoding;
import Graphics.Materials.State;
import Graphics.Materials.ProceduralPass;
#include "Lib/BaseType.h"
#include "GameClient/View.h"
#include <SDL3/SDL.h>
#include <cstdint>
#include <span>
#include "W3DDevice/GameClient/W3DCamera.h"
#include "W3DDevice/GameClient/W3DTextureHandle.h"
#include "WWLib/simplevec.h"

import Graphics.Resources.Textures.Edit;
#include "Common/MapObject.h"
#include "Common/PerfTimer.h"
#include "W3DDevice/GameClient/BaseHeightMap.h"
#include "W3DDevice/GameClient/WorldHeightMap.h"
#include "W3DDevice/GameClient/W3DPoly.h"
#include "W3DDevice/GameClient/W3DShaderManager.h"
#include "W3DDevice/GameClient/W3DAssetCatalog.h"
#include "W3DDevice/GameClient/W3DShroud.h"
import Graphics.Resources.Textures.Storage;
#include "Common/GlobalData.h"
#include "GameLogic/PartitionManager.h"
#include "W3DDevice/GameClient/W3DGraphicsResources.h"
import Assets.Images.Buffer;
import Graphics.Frame.Runtime;
import Graphics.Scene.Shroud.Image;

struct W3DShroud::GraphicsState
{
    Graphics::ShroudImage image;
    std::uint16_t border_pixel = 0;
};


//-----------------------------------------------------------------------------

// In Global Data now

//#define SHROUD_COLOR	0x00ffffff //temporary test of gray shroud instead of pure black.
//#define MIN_SHROUD_LEVEL	0		//for gray fog

//Int SHROUD_COLOR=0x00808080; //temporary test of gray shroud instead of pure black.
//Int MIN_SHROUD_LEVEL=50;		//for gray fog

//#define SHROUD_COLOR	0x00eeeebff //temporary test of gray shroud instead of pure black.
//#define MIN_SHROUD_LEVEL	254		//for gray fog

//#define SHROUD_COLOR	0x00bbbbbb //temporary test of gray shroud instead of pure black.
//#define SHROUD_COLOR	0x00004080 //temporary test of blue shroud instead of pure black.

//#define MIN_SHROUD_LEVEL	100		//for black fog

//#define MAX_MAP_SHROUDSIZE	1024	//maximum number of shroud cells across entire map.
//#define MAX_VISIBLE_SHROUDSIZE	(MAX_MAP_SHROUDSIZE+1)	//maximum number of shroud vertices visible at any given time
#define DEFAULT_SHROUD_CELL_SIZE	MAP_XY_FACTOR	//assume shroud at same resolution as terrain cells.
#define DEFAULT_TERRAIN_SIZE 1024 //assumed size of largest terrain possible (in vertices)
#define DEFAULT_VISIBLE_TERRAIN 96	//assumed size of visible terrain cells.

//-----------------------------------------------------------------------------
W3DShroud::W3DShroud() : m_graphics(std::make_unique<GraphicsState>())
{
	m_finalFogData=nullptr;
	m_currentFogData=nullptr;
	m_pSrcTexture=nullptr;
	m_pDstTexture=nullptr;
	m_srcTextureData=nullptr;
	m_srcTexturePitch=0;
	m_dstTextureWidth=m_numMaxVisibleCellsX=0;
	m_dstTextureHeight=m_numMaxVisibleCellsY=0;
	m_boderShroudLevel = (W3DShroudLevel)TheGlobalData->m_shroudAlpha;	//assume border is black
	m_clearDstTexture = TRUE;	//force clearing of destination texture;

	m_cellWidth=DEFAULT_SHROUD_CELL_SIZE;
	m_cellHeight=DEFAULT_SHROUD_CELL_SIZE;
	m_numCellsX=0;
	m_numCellsY=0;
	m_shroudFilter=Graphics::SamplingFilter::Default;
}

//-----------------------------------------------------------------------------
W3DShroud::~W3DShroud()
{
	ReleaseResources();

	if (m_pSrcTexture)
		delete m_pSrcTexture;

	delete [] m_finalFogData;
	delete [] m_currentFogData;

	m_drawFogOfWar=FALSE;
}

//-----------------------------------------------------------------------------
/**Called to initialize a new shroud for a new map.  Should be done after the map is loaded
   into the terrain object.  worldCellSize is the world-space dimensions of each shroud cell.
   The system will generate enough cells to cover the full map.
*/
void W3DShroud::init(WorldHeightMap *pMap, Real worldCellSizeX, Real worldCellSizeY)
{
	DEBUG_ASSERTCRASH( m_pSrcTexture == nullptr, ("ReAcquire of existing shroud textures"));
	DEBUG_ASSERTCRASH( pMap != nullptr, ("Shroud init with null WorldHeightMap"));

	Int dstTextureWidth=0;
	Int dstTextureHeight=0;
	m_cellWidth=worldCellSizeX;
	m_cellHeight=worldCellSizeY;

	//Precompute a bounding box for entire shroud layer
	if (pMap)
	{
		m_numCellsX = REAL_TO_INT_CEIL((Real)(pMap->getXExtent() - 1 - pMap->getBorderSizeInline()*2)*MAP_XY_FACTOR/m_cellWidth);
		m_numCellsY = REAL_TO_INT_CEIL((Real)(pMap->getYExtent() - 1 - pMap->getBorderSizeInline()*2)*MAP_XY_FACTOR/m_cellHeight);

		//Maximum visible cells will depend on maximum drawable terrain size plus 1 for partial cells (since
		//shroud cells are larger than terrain cells).
		dstTextureWidth=m_numMaxVisibleCellsX=REAL_TO_INT_FLOOR((Real)(pMap->getDrawWidth()-1)*MAP_XY_FACTOR/m_cellWidth)+1;
		dstTextureHeight=m_numMaxVisibleCellsY=REAL_TO_INT_FLOOR((Real)(pMap->getDrawHeight()-1)*MAP_XY_FACTOR/m_cellHeight)+1;

		dstTextureWidth = m_numCellsX;
		dstTextureHeight = m_numCellsY;

		dstTextureWidth += 2;	//enlarge by 2 pixels so we can have a border color all the way around.
		dstTextureHeight += 2;	//enlarge by 2 pixels so we can have border color all the way around.
        auto* device = Graphics::Shared_Frame_Device();
        if (!device) return;
        const auto extent = Graphics::Select_Texture_Extent({unsigned(dstTextureWidth), unsigned(dstTextureHeight), 1}, device->Texture_Limits());
        if (extent.width == 0 || extent.height == 0) return;
        dstTextureWidth = extent.width;
        dstTextureHeight = extent.height;
	}


	UnsignedInt srcWidth,srcHeight;

	srcWidth=m_numCellsX;
	//vertical size is larger by 1 pixel so that we have some unused pixels to use in clearing the video texture.
	//To clear the video texture, I will copy pixels from this unused area.  There is no other way to clear a video
  //memory texture to a known value because you can't lock it - only copy into it.
	srcHeight=m_numCellsY;
	srcHeight += 1;

#ifdef DO_FOG_INTERPOLATION
	m_finalFogData = new W3DShroudLevel[srcWidth*srcHeight];
	m_currentFogData = new W3DShroudLevel[srcWidth*srcHeight];
	//Clear the fog to black
	memset(m_currentFogData,0,srcWidth*srcHeight);
 	memset(m_finalFogData,0,srcWidth*srcHeight);
#endif

#if defined(RTS_DEBUG)
	if (TheGlobalData && TheGlobalData->m_fogOfWarOn)
		m_pSrcTexture = Graphics::TextureEdit::Create(srcWidth,srcHeight, Assets::PixelEncoding::BGRA4444);
	else
#endif
		m_pSrcTexture = Graphics::TextureEdit::Create(srcWidth,srcHeight, Assets::PixelEncoding::BGR565);

	DEBUG_ASSERTCRASH( m_pSrcTexture != nullptr, ("Failed to Allocate Shroud Src Surface"));

	Graphics::ImageMapping rect = {};

	//Get a pointer to source surface pixels.

	const bool locked = m_pSrcTexture != nullptr &&
		!(rect = m_pSrcTexture->Map()).bytes.empty();
	if (locked)
	{
		m_pSrcTexture->Unmap();
	}
	else
	{
		DEBUG_ASSERTCRASH(false, ("Failed to lock shroud src surface"));
		return;
	}

	m_srcTextureData=rect.bytes.data();
	m_srcTexturePitch=rect.row_pitch;

	//clear entire texture to black
	memset(m_srcTextureData,0,m_srcTexturePitch*srcHeight);

#if defined(RTS_DEBUG)
	if (TheGlobalData && TheGlobalData->m_fogOfWarOn)
		fillShroudData(TheGlobalData->m_shroudAlpha);	//initialize shroud to a known value
#endif

	if (dstTextureWidth != m_dstTextureWidth || dstTextureHeight != m_dstTextureHeight )	///@todo: Check if size has changed - probably never
		ReleaseResources();	//need a new sized shroud

	if (!m_pDstTexture )
	{	m_dstTextureWidth = dstTextureWidth;
		m_dstTextureHeight = dstTextureHeight;
		ReAcquireResources();	//allocate video memory surface
	}

	//Force a refresh of shroud data since we just created a new source texture.
	if (ThePartitionManager)
		ThePartitionManager->refreshShroudForLocalPlayer();

}

//-----------------------------------------------------------------------------
///Called on map reset.
void W3DShroud::reset()
{
	//Free old shroud data since it may no longer fit new map.
	if (m_pSrcTexture)
	{
		delete m_pSrcTexture;
		m_pSrcTexture=nullptr;
	}

	delete [] m_finalFogData;
	m_finalFogData=nullptr;

	delete [] m_currentFogData;
	m_currentFogData=nullptr;

	m_clearDstTexture = TRUE;	//always refill the destination texture after a reset
}

//-----------------------------------------------------------------------------
///Release any resources that can't survive a D3D device reset.
void W3DShroud::ReleaseResources()
{
	m_graphics->image.Invalidate_Upload();
	REF_PTR_RELEASE (m_pDstTexture);
}

//-----------------------------------------------------------------------------
///Restore resources that are lost on D3D device reset.
Bool W3DShroud::ReAcquireResources()
{
		if (!m_dstTextureWidth)
			return TRUE;	//nothing to reacquire since shroud was never initialized with valid data

		DEBUG_ASSERTCRASH( m_pDstTexture == nullptr, ("ReAcquire of existing shroud texture"));

		// Create destination texture (stored in video memory).
		// Since we control the video memory copy, we can do partial updates more efficiently. Or do shift blits.
#if defined(RTS_DEBUG)
		if (TheGlobalData && TheGlobalData->m_fogOfWarOn)
			m_pDstTexture = MSGNEW("W3DTextureHandle") W3DTextureHandle(m_dstTextureWidth,m_dstTextureHeight,Assets::PixelEncoding::BGRA4444,MIP_LEVELS_1, W3DTextureHandle::POOL_DEFAULT);
		else
#endif
			m_pDstTexture = MSGNEW("W3DTextureHandle") W3DTextureHandle(m_dstTextureWidth,m_dstTextureHeight,Assets::PixelEncoding::BGR565,MIP_LEVELS_1, W3DTextureHandle::POOL_DEFAULT);

		DEBUG_ASSERTCRASH( m_pDstTexture != nullptr, ("Failed ReAcquire of shroud texture"));

		if (!m_pDstTexture)
		{	//could not create a valid texture
			m_dstTextureWidth = 0;
			m_dstTextureHeight = 0;
			return FALSE;
		}
		m_pDstTexture->Get_Sampling().address[0] = Graphics::RHISamplerAddress::Clamp;
		m_pDstTexture->Get_Sampling().address[1] = Graphics::RHISamplerAddress::Clamp;
		m_pDstTexture->Get_Sampling().mipmap = Graphics::SamplingFilter::Disabled;
		m_clearDstTexture = TRUE;	//force clearing of destination texture first time it's used.

		return TRUE;
}

//-----------------------------------------------------------------------------
W3DShroudLevel W3DShroud::getShroudLevel(Int x, Int y)
{
	DEBUG_ASSERTCRASH( m_pSrcTexture != nullptr, ("Reading empty shroud"));

	if (x >= 0 && y >= 0 && x < m_numCellsX && y < m_numCellsY)
	{
		UnsignedShort pixel=*(UnsignedShort *)((Byte *)m_srcTextureData + x*2 + y*m_srcTexturePitch);

#if defined(RTS_DEBUG)
		if (TheGlobalData && TheGlobalData->m_fogOfWarOn)
			//in this mode, alpha channel holds intensity
			return (W3DShroudLevel)((1.0f-((Real)(pixel >> 12)/15.0f))*255.0f);
		else
#endif
			//in this mode, green has the best precision at 6 bits.
			return (W3DShroudLevel)((Real)((pixel >> 5)&0x3f)/63.0f*255.0f);
	}
	return 0;
}

//-----------------------------------------------------------------------------
void W3DShroud::setShroudLevel(Int x, Int y, W3DShroudLevel level, Bool textureOnly)
{
	DEBUG_ASSERTCRASH( m_pSrcTexture != nullptr, ("Writing empty shroud.  Usually means that map failed to load."));

	if (!m_pSrcTexture)
		return;

	if (x < m_numCellsX && y < m_numCellsY)
	{
		if (level < TheGlobalData->m_shroudAlpha)
			level = TheGlobalData->m_shroudAlpha;

#if defined(RTS_DEBUG)
		if (TheGlobalData && TheGlobalData->m_fogOfWarOn)
		{
			///@todo: optimize this case if we end up using fog shroud.
			Int redVal = TheGlobalData->m_shroudColor.red;
			Int greenVal = TheGlobalData->m_shroudColor.green;
			Int blueVal = TheGlobalData->m_shroudColor.blue;
//			Int redVal = (SHROUD_COLOR >> 16) & 0xff;
//			Int greenVal = (SHROUD_COLOR >> 8) & 0xff;
//			Int blueVal = SHROUD_COLOR & 0xff;
			Int alphaVal = 255 - level;

			UnsignedShort pixel=((blueVal>>4)&0xf) | (((greenVal>>4)&0xf)<<4) | (((redVal>>4)&0xf)<<8) | (((alphaVal>>4)&0xf)<<12);
			*(UnsignedShort *)((Byte *)m_srcTextureData + x*2 + y*m_srcTexturePitch)=pixel;
		}
		else
#endif
		{
#ifdef DO_FOG_INTERPOLATION
			if (!textureOnly)
				m_finalFogData[x+y*m_numCellsX]=level;
#endif
			UnsignedInt bluepixel = (UnsignedInt)((Real)level*((Real)(TheGlobalData->m_shroudColor.getAsInt()&0xff)/255.0f));
			UnsignedInt greenpixel = (UnsignedInt)((Real)level*((Real)((TheGlobalData->m_shroudColor.getAsInt()&0xff00)>>8)/255.0f));
			UnsignedInt redpixel = (UnsignedInt)((Real)level*((Real)((TheGlobalData->m_shroudColor.getAsInt()&0xff0000)>>16)/255.0f));
//			UnsignedInt bluepixel = (UnsignedInt)((Real)level*((Real)(SHROUD_COLOR&0xff)/255.0f));
//			UnsignedInt greenpixel = (UnsignedInt)((Real)level*((Real)((SHROUD_COLOR&0xff00)>>8)/255.0f));
//			UnsignedInt redpixel = (UnsignedInt)((Real)level*((Real)((SHROUD_COLOR&0xff0000)>>16)/255.0f));
			if (level == 255)
			{	//unshrouded pixels should be fully lit
				redpixel = 255;
				greenpixel = 255;
				bluepixel = 255;
			}

			UnsignedShort *texel = (UnsignedShort *)((Byte *)m_srcTextureData + x*2 + y*m_srcTexturePitch);

//      For those interested, MLorenzen has this bock commented out until he gets back on Mon, Sept. 30 2002
			// If this code is still here by mid october, nuke it!
//			UnsignedInt texelRed =  ((*texel >> 8 ) & 0xf8);
//			UnsignedInt texelGreen =((*texel >> 3 ) & 0xfc);
//			UnsignedInt texelBlue = ((*texel << 3 ) & 0xf8);
//			if (texelRed < texelGreen && texelGreen > texelBlue)
//			{
//				bluepixel += redpixel;
//				bluepixel -= redpixel;
//			}

			*texel = ( ((bluepixel&0xf8) >> 3) | ((greenpixel&0xfc)<<3) | ((redpixel&0xf8)<<8));
		}
		return;
	}
}

//-----------------------------------------------------------------------------
///Quickly sets the shroud level of entire map to a single value
void W3DShroud::fillShroudData(W3DShroudLevel level)
{

	Int x,y;
	UnsignedShort pixel;

	if (level < TheGlobalData->m_shroudAlpha)
		level = TheGlobalData->m_shroudAlpha;

#if defined(RTS_DEBUG)
	//convert value to pixel format
	if (TheGlobalData && TheGlobalData->m_fogOfWarOn)
	{
		Int redVal = TheGlobalData->m_shroudColor.red;
		Int greenVal = TheGlobalData->m_shroudColor.green;
		Int blueVal = TheGlobalData->m_shroudColor.blue;
//		Int redVal = (SHROUD_COLOR >> 16) & 0xff;
//		Int greenVal = (SHROUD_COLOR >> 8) & 0xff;
//		Int blueVal = SHROUD_COLOR & 0xff;
		Int alphaVal = 255 - level;

		pixel=((blueVal>>4)&0xf) | (((greenVal>>4)&0xf)<<4) | (((redVal>>4)&0xf)<<8) | (((alphaVal>>4)&0xf)<<12);
	}
	else
#endif
	{
		UnsignedInt bluepixel = (UnsignedInt)((Real)level*((Real)(TheGlobalData->m_shroudColor.getAsInt()&0xff)/255.0f));
		UnsignedInt greenpixel = (UnsignedInt)((Real)level*((Real)((TheGlobalData->m_shroudColor.getAsInt()&0xff00)>>8)/255.0f));
		UnsignedInt redpixel = (UnsignedInt)((Real)level*((Real)((TheGlobalData->m_shroudColor.getAsInt()&0xff0000)>>16)/255.0f));
//		UnsignedInt bluepixel = (UnsignedInt)((Real)level*((Real)(SHROUD_COLOR&0xff)/255.0f));
//		UnsignedInt greenpixel = (UnsignedInt)((Real)level*((Real)((SHROUD_COLOR&0xff00)>>8)/255.0f));
//		UnsignedInt redpixel = (UnsignedInt)((Real)level*((Real)((SHROUD_COLOR&0xff0000)>>16)/255.0f));

		if (level == 255)
		{	//unshrouded pixels should be fully lit
			redpixel = 255;
			greenpixel = 255;
			bluepixel = 255;
		}
		pixel=( ((bluepixel&0xf8) >> 3) | ((greenpixel&0xfc)<<3) | ((redpixel&0xf8)<<8));
	}

	UnsignedShort *ptr=(UnsignedShort *)m_srcTextureData;
	Int pitch = m_srcTexturePitch >> 1;	//2 bytes per pointer increment
	for (y=0; y<m_numCellsY; y++)
	{
		for (x=0; x<m_numCellsX; x++)
			ptr[x]=pixel;
		ptr	+= pitch;
	}

#ifdef DO_FOG_INTERPOLATION
	//Set the final shroud state.  May differe from current state because of time interpolation.
	W3DShroudLevel *cptr=m_finalFogData;
	pitch = m_numCellsX;
	for (y=0; y<m_numCellsY; y++)
	{
		for (x=0; x<m_numCellsX; x++)
			cptr[x]=level;
		ptr	+= pitch;
	}
#endif
}

void W3DShroud::fillBorderShroudData(W3DShroudLevel level)
{
	Int x;
	UnsignedShort pixel;

	if (level < TheGlobalData->m_shroudAlpha)
		level = TheGlobalData->m_shroudAlpha;

#if defined(RTS_DEBUG)
	//convert value to pixel format
	if (TheGlobalData && TheGlobalData->m_fogOfWarOn)
	{
		Int redVal = TheGlobalData->m_shroudColor.red;
		Int greenVal = TheGlobalData->m_shroudColor.green;
		Int blueVal = TheGlobalData->m_shroudColor.blue;
		Int alphaVal = 255 - level;

		pixel=((blueVal>>4)&0xf) | (((greenVal>>4)&0xf)<<4) | (((redVal>>4)&0xf)<<8) | (((alphaVal>>4)&0xf)<<12);
	}
	else
#endif
	{
		UnsignedInt bluepixel = (UnsignedInt)((Real)level*((Real)(TheGlobalData->m_shroudColor.getAsInt()&0xff)/255.0f));
		UnsignedInt greenpixel = (UnsignedInt)((Real)level*((Real)((TheGlobalData->m_shroudColor.getAsInt()&0xff00)>>8)/255.0f));
		UnsignedInt redpixel = (UnsignedInt)((Real)level*((Real)((TheGlobalData->m_shroudColor.getAsInt()&0xff0000)>>16)/255.0f));

		if (level == 255)
		{	//unshrouded pixels should be fully lit
			redpixel = 255;
			greenpixel = 255;
			bluepixel = 255;
		}
		pixel=( ((bluepixel&0xf8) >> 3) | ((greenpixel&0xfc)<<3) | ((redpixel&0xf8)<<8));
	}

	//Skip to unused texels within the shroud data
	UnsignedShort *ptr=(UnsignedShort *)m_srcTextureData + m_numCellsY*(m_srcTexturePitch >> 1);

	//Fill unused texels with border color
	for (x=0; x<m_numCellsX; x++)
			ptr[x]=pixel;

	m_graphics->border_pixel = pixel;
}

/**Set the shroud color within the border area of the map*/
void W3DShroud::setBorderShroudLevel(W3DShroudLevel level)
{
	m_boderShroudLevel = level;
	m_clearDstTexture = TRUE;
}

//-----------------------------------------------------------------------------
///@todo: remove this
W3DTextureHandle *DummyTexture=nullptr;

//#define LOAD_DUMMY_SHROUD

//-----------------------------------------------------------------------------
//DECLARE_PERF_TIMER(shroudCopy)

//-----------------------------------------------------------------------------
/** Updates video memory surface with currently visible shroud data */
void W3DShroud::render(W3DCamera *cam)
{
	if (!m_pSrcTexture)
		return; //nothing to update from.  Must be in reset state.

	if (!Graphics::Frame_Device_Ready())
		return;	//device not ready to render anything

#if defined(RTS_DEBUG)
	if (TheGlobalData && TheGlobalData->m_fogOfWarOn != m_drawFogOfWar)
	{
		//fog state has changed since last time shroud system was initialized
		reset();
		ReleaseResources();
		init(TheTerrainRenderObject->getMap(),m_cellWidth,m_cellHeight);
		ThePartitionManager->refreshShroudForLocalPlayer();
		m_drawFogOfWar=TheGlobalData->m_fogOfWarOn;
		m_clearDstTexture=TRUE;
	}
#endif

	DEBUG_ASSERTCRASH( m_pSrcTexture != nullptr, ("Updating unallocated shroud texture"));

#ifdef LOAD_DUMMY_SHROUD

	static doInit=1;
	if (doInit)
	{
		//some temporary code here to debug the shroud.

		///@todo: remove this debug buffer fill.
		fillShroudData(1.0f);	//force to all shrouded

		Short *src=(Short *)m_srcTextureData;
		//fill with some dummy values
		src[0]=(char)0xff;
		src[1]=(char)0xff;
		src[m_numCellsX]=(char)0xff;
		src[m_numCellsX+1]=(char)0xff;
		src[m_numCellsX*2]=(char)0xff;

		src[m_numCellsX*9+8]=(char)0xff;
		src[m_numCellsX*8+8]=(char)0xff;
		src[m_numCellsX*7+8]=(char)0xff;
		src[m_numCellsX*8+9]=(char)0xff;
		src[m_numCellsX*8+7]=(char)0xff;

		DummyTexture=W3DAssetCatalog::Get_Instance()->Get_Texture("shroud1024.tga");

		Short *dataDest=(Short *)((char *)m_srcTextureData);	//offset to correct row of full sysmem shroud
		Int pitchDest = m_srcTexturePitch >> 1;	//2 bytes per pixel so divide byte count by 2.

		//Copy the dummy shroud into our game shroud.
		Graphics::TextureEdit *pSurface=DummyTexture->Get_Surface_Level(0);
		const auto mapping=pSurface ? pSurface->Map() : Graphics::ImageMapping{};
        if (mapping.bytes.empty()) { delete pSurface; REF_PTR_RELEASE(DummyTexture); return; }
        Int pitch=static_cast<Int>(mapping.row_pitch);
		Int *dataSrc=reinterpret_cast<Int*>(mapping.bytes.data());	//offset to correct row of full sysmem shroud
		pitch >>= 2;	//4 bytes per pixel so divide byte count by 4.
		Assets::ImageDescription desc;
		desc=pSurface->Image().Description();

		//Check if source data is larger than our current shroud
		desc.width = __min(desc.width,m_numCellsX);
		desc.height = __min(desc.height,m_numCellsY);

		for (Int y=0; y<desc.height; y++)
		{
			for (Int x=0; x<desc.width; x++)
			{
#if defined(RTS_DEBUG)
				if (TheGlobalData && TheGlobalData->m_fogOfWarOn)
				{
					dataDest[x]=((TheGlobalData->m_shroudColor.getAsInt()>>4)&0xf) | (((TheGlobalData->m_shroudColor.getAsInt()>>12)&0xf)<<4) | (((TheGlobalData->m_shroudColor.getAsInt()>>20)&0xf)<<8) | ((((255-(dataSrc[x] & 0xff))>>4)&0xf)<<12);
//					dataDest[x]=((SHROUD_COLOR>>4)&0xf) | (((SHROUD_COLOR>>12)&0xf)<<4) | (((SHROUD_COLOR>>20)&0xf)<<8) | ((((255-(dataSrc[x] & 0xff))>>4)&0xf)<<12);
				}
				else
#endif
				{
					dataDest[x]=((dataSrc[x]>>3)&0x1f) | (((dataSrc[x]>>10)&0x3f)<<5) | (((dataSrc[x]>>19)&0x1f)<<11);
				}
			}

			dataDest += pitchDest;	//skip to next row.
			dataSrc += pitch;	//skip to next row of shroud
		}

		pSurface->Unmap();

		delete pSurface; pSurface = nullptr;
		REF_PTR_RELEASE (DummyTexture);

		doInit=0;
	}

#endif //LOAD_DUMMY_SHROUD


	if (!m_pDstTexture || m_numCellsX <= 0 || m_numCellsY <= 0
		|| m_srcTexturePitch % sizeof(std::uint16_t) != 0) return;

	// The projection covers the full map; its first cell follows a one-texel border.
	m_drawOriginX = 0;
	m_drawOriginY = 0;
	if (m_pDstTexture->Get_Sampling().magnification != m_shroudFilter)
	{
		m_pDstTexture->Get_Sampling().magnification = m_shroudFilter;
		m_pDstTexture->Get_Sampling().minification = m_shroudFilter;
	}

#ifdef DO_FOG_INTERPOLATION
	Assets::ImageRegion cells = {0,0,m_numCellsX,m_numCellsY};
	interpolateFogLevels(&cells);
#endif
	if (m_clearDstTexture) fillBorderShroudData(m_boderShroudLevel);
	auto* device = Graphics::Shared_Frame_Device();
	if (!device) return;
	const auto texture = Resolve_Graphics_Texture(m_pDstTexture);
	const auto stride = m_srcTexturePitch/static_cast<unsigned>(sizeof(std::uint16_t));
	const std::span pixels(static_cast<const std::uint16_t*>(m_srcTextureData),
		std::size_t(stride)*m_numCellsY);
	if (!m_graphics->image.Set_Cells(pixels,m_numCellsX,m_numCellsY,stride,
		m_dstTextureWidth,m_dstTextureHeight,m_graphics->border_pixel)) return;
	if (m_graphics->image.Upload(*device,texture)) m_clearDstTexture = FALSE;
}

#define FOG_INTERPOLATION_RATE	(255.0f/1000.0f)	//take one second to go from black to fully lit.
//-----------------------------------------------------------------------------
void W3DShroud::interpolateFogLevels(const Assets::ImageRegion *rect)
{
	static UnsignedInt prevTime = static_cast<UnsignedInt>(SDL_GetTicks());

	UnsignedInt timeDiff=static_cast<UnsignedInt>(SDL_GetTicks())-prevTime;

	if (!timeDiff)
		return;	//no time has elapsed

	prevTime +=timeDiff;	//update for next frame

	Int maxFogChange=FOG_INTERPOLATION_RATE * (Real)timeDiff;	//maximum amount of fog change allowed in frame.
	if (maxFogChange > 255)
		maxFogChange = 255;
	W3DShroudLevel levelDelta = maxFogChange;

	W3DShroudLevel *startLevel=m_currentFogData;
	W3DShroudLevel *finalLevel=m_finalFogData;

	for (Int j=0; j<m_numCellsY; j++)
	{
		for (Int i=0; i<m_numCellsX; i++,startLevel++,finalLevel++)
			if (*startLevel != *finalLevel)
			{	//fog needs fading.
				if (*startLevel == *finalLevel)
					continue;
				else
				if (*finalLevel < *startLevel)
				{
					if ((*startLevel - *finalLevel) < levelDelta)
						*startLevel = *finalLevel;	//change too large so clamp to final value.
					else
						*startLevel -= levelDelta;
				}
				else
				if (*finalLevel > *startLevel)
				{
					if ((*finalLevel - *startLevel) < levelDelta)
						*startLevel = *finalLevel;	//change too large so clamp to final value.
					else
						*startLevel += levelDelta;
				}
				setShroudLevel(i,j,*startLevel,TRUE);
			}
	}
}

//-----------------------------------------------------------------------------
void W3DShroud::setShroudFilter(Bool enable)
{
	if (enable)
		m_shroudFilter=Graphics::SamplingFilter::Default;
	else
		m_shroudFilter=Graphics::SamplingFilter::Disabled;
}

//-----------------------------------------------------------------------------
///Prepare the draw data required to render the shroud pass.
bool Describe_W3D_Shroud_Material_Pass(const NativeMaterialPass&, NativeMaterialPass::Description& description)
{
    auto* shroud=TheTerrainRenderObject ? TheTerrainRenderObject->getShroud() : nullptr;
    if (!shroud) return false;
    description.shader=Graphics::MaterialState::MultiplicativeSprite();
#if defined(RTS_DEBUG)
    if (TheGlobalData && TheGlobalData->m_fogOfWarOn) description.shader=Graphics::MaterialState::AlphaSprite();
#endif
    description.shader.Set_Depth_Compare(Graphics::MaterialState::PASS_EQUAL);
    description.shader.Set_Primary_Gradient(Graphics::MaterialState::GRADIENT_DISABLE);
    description.textures[0]=shroud->getShroudTexture();
    description.world_coordinates=true;
    const float width=shroud->getCellWidth();
    const float height=shroud->getCellHeight();
    const float xscale=1/(width*shroud->getTextureWidth());
    const float yscale=1/(height*shroud->getTextureHeight());
    const bool has_map=TheTerrainRenderObject->getMap()!=nullptr;
    description.world_texture_transform[0]=xscale;
    description.world_texture_transform[5]=yscale;
    description.world_texture_transform[3]=has_map ? (-shroud->getDrawOriginX()+width)*xscale : 0;
    description.world_texture_transform[7]=has_map ? (-shroud->getDrawOriginY()+height)*yscale : 0;
    return description.textures[0]!=nullptr;
}

bool Describe_W3D_Mask_Material_Pass(const NativeMaterialPass&, NativeMaterialPass::Description& description)
{
    description.shader=Graphics::MaterialState::Opaque();
    description.shader.Set_Primary_Gradient(Graphics::MaterialState::GRADIENT_DISABLE);
    description.textures[0]=ScreenCrossFadeFilter::getCurrentMaskTexture();
    description.color_write_mask=8;
    description.world_coordinates=true;
    Coord3D center;
    center.zero();
    if (TheTacticalView) {
        ICoord2D screen;
        screen.x=TheTacticalView->getWidth()/2;
        screen.y=TheTacticalView->getHeight()/2;
        TheTacticalView->screenToTerrain(&screen,&center);
    }
    const float extent=(1-ScreenCrossFadeFilter::getCurrentFadeValue())*25*128;
    const float scale=extent!=0 ? 1/extent : 0;
    description.world_texture_transform[0]=scale;
    description.world_texture_transform[5]=scale;
    description.world_texture_transform[3]=extent!=0 ? 0.5f-center.x*scale : 0;
    description.world_texture_transform[7]=extent!=0 ? 0.5f-center.y*scale : 0;
    return description.textures[0]!=nullptr;
}

std::shared_ptr<NativeMaterialPass> Create_W3D_Shroud_Material_Pass()
{
    auto pass=std::make_shared<NativeMaterialPass>();
    pass->prepare=&Describe_W3D_Shroud_Material_Pass;
    return pass;
}

std::shared_ptr<NativeMaterialPass> Create_W3D_Mask_Material_Pass()
{
    auto pass=std::make_shared<NativeMaterialPass>();
    pass->prepare=&Describe_W3D_Mask_Material_Pass;
    return pass;
}
