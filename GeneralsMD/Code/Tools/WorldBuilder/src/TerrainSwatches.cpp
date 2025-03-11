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

// TerrainSwatches.cpp : implementation file
//

#include "StdAfx.h"
#include "resource.h"
#include "TerrainSwatches.h"
#include "TerrainMaterial.h"
#include "WorldBuilderDoc.h"
#include "WHeightMapEdit.h"

/////////////////////////////////////////////////////////////////////////////
// TerrainSwatches

TerrainSwatches::TerrainSwatches()
{
}

TerrainSwatches::~TerrainSwatches()
{
}


BEGIN_MESSAGE_MAP(TerrainSwatches, CWnd)
	//{{AFX_MSG_MAP(TerrainSwatches)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

#define SWATCH_OFFSET 20
/////////////////////////////////////////////////////////////////////////////
// TerrainSwatches message handlers

void TerrainSwatches::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CRect clientRect;
	GetClientRect(&clientRect);

	CRect bgRect;
	bgRect = clientRect;
	bgRect.top = bgRect.bottom-TILE_PIXEL_EXTENT;
	bgRect.left = bgRect.right-TILE_PIXEL_EXTENT;

	CRect fgRect = clientRect;
	fgRect.bottom = fgRect.top+TILE_PIXEL_EXTENT;
	fgRect.right = fgRect.left+TILE_PIXEL_EXTENT;

	CBrush brush;
	brush.CreateSolidBrush(RGB(0,0,0));

	Int fgTexClass = TerrainMaterial::getFgTexClass();
	Int bgTexClass = TerrainMaterial::getBgTexClass();
	UnsignedByte *pData;
	pData = WorldHeightMapEdit::getPointerToClassTileData(bgTexClass);
	if (pData) {
		DrawMyTexture(&dc, bgRect.top, bgRect.left, TILE_PIXEL_EXTENT, pData);
	} else {
		dc.FillSolidRect(&bgRect, RGB(0,128,0));
	}
	dc.FrameRect(&bgRect, &brush);
	pData = WorldHeightMapEdit::getPointerToClassTileData(fgTexClass);
	if (pData) {
		DrawMyTexture(&dc, fgRect.top, fgRect.left, TILE_PIXEL_EXTENT, pData);
	} else {
		dc.FillSolidRect(&fgRect, RGB(128,0,0));
	}
	dc.FrameRect(&fgRect, &brush);


}

void TerrainSwatches::DrawMyTexture(CDC *pDc, int top, int left, Int width, UnsignedByte *rgbData)
{
	// Just blast about some dib bits.
	
	LPBITMAPINFO pBI;
//	long bytes = sizeof(BITMAPINFO);
 	pBI = new BITMAPINFO;
	pBI->bmiHeader.biSize = sizeof(pBI->bmiHeader);
	pBI->bmiHeader.biWidth = width;
	pBI->bmiHeader.biHeight = width; /* match display top left == 0,0 */
	pBI->bmiHeader.biPlanes = 1;
	pBI->bmiHeader.biBitCount = 32;
	pBI->bmiHeader.biCompression = BI_RGB;
	pBI->bmiHeader.biSizeImage = (width*width)*(pBI->bmiHeader.biBitCount/8);
	pBI->bmiHeader.biXPelsPerMeter = 1000;
	pBI->bmiHeader.biYPelsPerMeter = 1000;
	pBI->bmiHeader.biClrUsed = 0;
	pBI->bmiHeader.biClrImportant = 0;

	//::Sleep(10);
	/*int val=*/::StretchDIBits(pDc->m_hDC, left, top, width, width, 0, 0, width, width, rgbData, pBI, 
		DIB_RGB_COLORS, SRCCOPY);
	delete(pBI);
}


