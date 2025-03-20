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

// FILE: WorldBuilderView.cpp ////////////////////////////////////////////////
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
// File name: WorldBuilderView.cpp
//
// Created:   John Ahlquist, April 2001
//
// Desc:      2D view for the map data - textures, object icons and
//						contour info.
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//         Includes                                                      
//-----------------------------------------------------------------------------

#include "StdAfx.h"
#include "WorldBuilder.h"

#include "WorldBuilderDoc.h"
#include "WorldBuilderView.h"
#include "WHeightMapEdit.h"
#include "Common/GlobalData.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "W3DDevice/GameClient/W3DRoadBuffer.h"
#include "CellWidth.h"
#include "ContourOptions.h"
#include "MainFrm.h"
#include "CUndoable.h"
#include "Common/Debug.h"

//-----------------------------------------------------------------------------
//         Private Data                                                     
//-----------------------------------------------------------------------------
#define ROUND(x) (Int)(floor((x)+0.5))

/////////////////////////////////////////////////////////////////////////////
// CWorldBuilderView

//-----------------------------------------------------------------------------
//         Private Functions                                               
//-----------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// CWorldBuilderView construction/destruction

CWorldBuilderView::CWorldBuilderView() :
	m_cellSize(16),
	m_showContours(false),
	mXScrollOffset(0),
	mYScrollOffset(0),
	m_scrollMin(0,0),
	m_scrollMax(0,0),
	mShowGrid(true),
	m_showTexture(true)
{
	Int show;
	show = ::AfxGetApp()->GetProfileInt(MAIN_FRAME_SECTION, "ShowContours", 0);
	m_showContours = (show!=0);

	show = ::AfxGetApp()->GetProfileInt(MAIN_FRAME_SECTION, "ShowGrid", 0);
	mShowGrid = (show!=0);

	show = ::AfxGetApp()->GetProfileInt(MAIN_FRAME_SECTION, "ShowTexture", 1);
	m_showTexture = (show!=0);
	

}

CWorldBuilderView::~CWorldBuilderView()
{
}

BOOL CWorldBuilderView::PreCreateWindow(CREATESTRUCT& cs)
{
	return WbView::PreCreateWindow(cs);
}

int CWorldBuilderView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (WbView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	return 0;
}

void CWorldBuilderView::OnDraw(CDC* pDC)
{
	// Not used.  See OnPaint.
}


/////////////////////////////////////////////////////////////////////////////
// CWorldBuilderView printing

BOOL CWorldBuilderView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CWorldBuilderView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

void CWorldBuilderView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

/////////////////////////////////////////////////////////////////////////////
// CWorldBuilderView diagnostics

#ifdef _DEBUG
void CWorldBuilderView::AssertValid() const
{
	WbView::AssertValid();
}

void CWorldBuilderView::Dump(CDumpContext& dc) const
{
	WbView::Dump(dc);
}

#endif //_DEBUG

/** Set the cell size, and invalidate. */
void CWorldBuilderView::setCellSize(Int cellSize)
{
	if (cellSize >= MIN_CELL_SIZE && cellSize<=MAX_CELL_SIZE) {
		m_cellSize = cellSize;
		adjustDocSize();
		Invalidate();
	}
}

void CWorldBuilderView::setCenterInView(Real x, Real y) 
{
	if (x != m_centerPt.X || y != m_centerPt.Y) {
		m_centerPt.X=x; 
		m_centerPt.Y=y;
		adjustDocSize();
		Invalidate();
	}
}

/** Set the show contours flag, and invalidate. */
void CWorldBuilderView::setShowContours(Bool show)
{
	m_showContours=show;
	Invalidate();
}
//-----------------------------------------------------------------------------
//         Public Functions                                                
//-----------------------------------------------------------------------------

//=============================================================================
// CWorldBuilderView::getColorForHeight
//=============================================================================
/** Gets the color for a given height in the height map.  Used when we
are not displaying terrain, and for the contour lines. */
//=============================================================================
DWORD CWorldBuilderView::getColorForHeight(UnsignedByte ht)
{
	Int fromR, fromG, fromB;
	Int toR, toG, toB;
	Int min, max;
	// black blue red green magenta cyan yellow white
	// 0x00  0x24 0x48 0x6c 0x90    0xb4 0xd8    0xff
	if (ht<0x24) {
		// 0 - 0x24 goes from black to blue
		fromR = fromG = fromB = toR = toG = 0;
		toB = 0xff;
		min = 0;
		max = 0x24;
	} else if (ht < 0x48) {
		// 0x24-0x48 goes from blue to red
		fromR = 0; fromG =  0; fromB = 0xFF;
		toR =  0xff; toG = 0; toB = 0x00;
		min = 0x24;
		max = 0x48;
	} else if (ht < 0x6c) {
		// 0x48-0x6c goes from red to green
		fromR = 0xFF; fromG =  0; fromB = 0;
		toR =  0; toG = 0xff; toB = 0;
		min = 0x48;
		max = 0x6c;
	} else if (ht < 0x90) {
		// 0x6c-0x90 goes from green to magenta
		fromR = 0; fromG =0xFF; fromB = 0;
		toR =  0xff; toG = 0; toB = 0xff;
		min = 0x6c;
		max = 0x90;
	} else if (ht < 0xb4) {
		// 0x90-0xb4 goes from magenta to cyan
		fromR = 0xff; fromG =0; fromB = 0xff;
		toR =  0; toG = 0xFF; toB = 0xff;
		min = 0x90;
		max = 0xb4;
	} else if (ht < 0xd8) {
		// 0xb4-0xd8 goes from cyan to yellow
		fromR = 0; fromG =0xf; fromB = 0xff;
		toR =  0xff; toG = 0xFF; toB = 0;
		min = 0xb4;
		max = 0xd8;
	} else {
		// 0xd8-0xff goes from yellow to white
		fromR = 0xff; fromG =0xff; fromB = 0;
		toR =  0xff; toG = 0xFF; toB = 0xFF;
		min = 0xd8;
		max = 0xff;
	}
	Int delta = max-min;
	Int to = ht-min;
	Int from = max-ht;
	// Interpolate red value.
	Int r = (fromR*from + toR*to +delta/2)/delta;
	if (r<0) r = 0;
	if (r>255) r = 255;
	// Interpolate green value.
	Int g = (fromG*from + toG*to +delta/2)/delta;
	if (g<0) g = 0;
	if (g>255) g = 255;
	// Interpolate blue value.
	Int b = (fromB*from + toB*to +delta/2)/delta;
	if (b<0) b = 0;
	if (b>255) b = 255;
	return(RGB(r,g,b));
}

//=============================================================================
// CWorldBuilderView::OnPaint
//=============================================================================
/** This draws the window. */
//=============================================================================
void CWorldBuilderView::OnPaint() 
{
	CWorldBuilderDoc* pDoc = WbDoc();
	ASSERT_VALID(pDoc);
	
	// If we are editing with a tool, draw the map being edited, if one exists.
	WorldHeightMapEdit *pMap = getTrackingHeightMap();

	// If no map yet, there is nothing to draw.
	if (pMap==NULL) return;

	Int minJ = 0;
	Int minI = 0;
	Int maxJ = pMap->getYExtent();
	Int maxI = pMap->getXExtent();

	CRect updateRect;
	CRgn updateRgn;
	updateRgn.CreateRectRgn(0,0,1,1);
	if (NULLREGION != GetUpdateRgn(&updateRgn)) {
		// The region isn't null, so get the bounding box.
		updateRgn.GetRgnBox(&updateRect);
		// Get the visible pixel bounds.
		Int left = updateRect.left + mXScrollOffset;
		Int right = updateRect.right + mXScrollOffset;
		Int top = updateRect.top + mYScrollOffset;
		Int bottom = updateRect.bottom + mYScrollOffset;
		// Convert the visible pixel bounds to the index bounds for the map.
		minI = (left/m_cellSize);
		maxI = (right/m_cellSize)+1;
		// Clip the bounds to the size of the actual height map.
		if (minI<0) minI=0;
		if (maxI>=pMap->getXExtent()) maxI = pMap->getXExtent()-1;

		// screen goes down, map goes up, so gotta dance a little on the verticals.
		Int min = (top/m_cellSize);
		Int max = (bottom/m_cellSize)+1;

		if (min<0) min=0;
		// Clip the bounds to the size of the actual height map.
		if (max>=pMap->getYExtent()) max = pMap->getYExtent();


		maxJ = pMap->getYExtent() - min;
		minJ = pMap->getYExtent() - max;
 		if (maxJ>=pMap->getYExtent()) maxJ = pMap->getYExtent()-1;
	}


	CPaintDC dc(this); // device context for painting
	// Offset the origin so that we can draw on 0 based coordinates.
	dc.SetViewportOrg(-mXScrollOffset, -mYScrollOffset);
	updateRgn.OffsetRgn(mXScrollOffset, mYScrollOffset);
	Int i, j;
	CRect rect;
	CBrush brush;
	// gray brush for drawing the grid.
	brush.CreateSolidBrush(RGB(128,128,128));
	CRect clientRect;
	GetClientRect(&clientRect);
	clientRect.OffsetRect(mXScrollOffset, mYScrollOffset);
	if (maxJ == pMap->getYExtent()-1) {
		rect = clientRect; // Do top.
		rect.bottom = (1)*m_cellSize;
		dc.FillSolidRect(&rect, RGB(255,255,255));
	}
	if (minJ == 0) {
		rect = clientRect; // Do bottom. 
		rect.top = (pMap->getYExtent())*m_cellSize;
		dc.FillSolidRect(&rect, RGB(255,255,255));
	}
	if (minI == 0) {
		rect = clientRect; // Do left;
		rect.right = 0;
		dc.FillSolidRect(&rect, RGB(255,255,255));
	}
	if (maxI == pMap->getXExtent()-1) {
		rect = clientRect; // Do left;
		rect.left = (pMap->getYExtent()-1)*m_cellSize;
		dc.FillSolidRect(&rect, RGB(255,255,255));
	}

	for (j=maxJ-1; j>=minJ; j--) { // map goes up, screen goes down.

		rect.bottom = (pMap->getYExtent()-j)*m_cellSize;
		rect.top = rect.bottom-m_cellSize;
		for (i=minI; i<maxI; i++) {
			UnsignedByte ht = pMap->getHeight(i, j);
			rect.left = i*m_cellSize;
			rect.right = rect.left+m_cellSize;
			// If this cell is not visible, don't bother drawing it.
			if (!updateRgn.RectInRegion(&rect)) {
				continue;
			}
#ifdef INTENSE_DEBUG
				dc.FillSolidRect(&rect, RGB(0,255,0));
				::Sleep(5);
#endif
			Int width = m_cellSize;
			UnsignedByte *pData = NULL;
			if (m_showTexture) {
			}
			// Draw the texture if we have one, else the height color.
			if ((pData!=NULL)) {
				drawMyTexture(&dc, &rect, width, pData);
			} else {
				dc.FillSolidRect(&rect, getColorForHeight(ht));
			}
			// Draw the grid.  It doesn't make sense to draw it if the width
			// in pixels is less than 3, as all you see is the grid.
			if (mShowGrid && m_cellSize>=MIN_GRID_SIZE) {
				CRect frameRect=rect;
				frameRect.bottom++; frameRect.right++;
				dc.FrameRect(&frameRect, &brush);
			}
		}
	}

	// Draw the contours if they are on.
	if (m_cellSize>=MIN_GRID_SIZE && m_showContours) {
		drawContours(&dc, &updateRgn, minI, maxI, minJ, maxJ);
	}

	// Draw the icons for the objects if they are turned on.
	if (m_showObjects) {
		MapObject *pMapObj = MapObject::getFirstMapObject();
		while (pMapObj) {
			// Assume objects draw approx grids across.
			CPoint lpt;
			docToViewCoords(*pMapObj->getLocation(), &lpt);
			lpt.x += mXScrollOffset;
			lpt.y += mYScrollOffset;
			// Create a rect centered on the object and 2 cells wide & tall.
			CRect rect;
			rect.bottom = lpt.y - m_cellSize;
			rect.top = lpt.y + m_cellSize;
			rect.left = lpt.x - m_cellSize;
			rect.right = lpt.x + m_cellSize;
			// If visible in the update region, draw it.
			if (updateRgn.RectInRegion(&rect)) 
			{
				drawObjectInView(&dc, pMapObj);
			}
			pMapObj = pMapObj->getNext();
		}
	}

}


//=============================================================================
// CWorldBuilderView::invalObjectInView
//=============================================================================
/** Causes the bounds of an object to be invalidated. */
//=============================================================================
void CWorldBuilderView::invalObjectInView(MapObject *pMapObj)
{
	if (pMapObj == NULL) {
		Invalidate(false);
		return;
	}

	CPoint lpt;
	docToViewCoords(*pMapObj->getLocation(), &lpt);

	// Calculate the bounds of the circle.
	CRect bounds(lpt, lpt);
	bounds.InflateRect(	m_cellSize/2+2, m_cellSize/2+2);

	// Calculate the bounds of the arrowhead.
	Coord3D ac;
	WbDoc()->getObjArrowPoint(pMapObj, &ac);
	CPoint arrpt;
	docToViewCoords(ac, &arrpt);

	CRect arrow(arrpt.x, arrpt.y, arrpt.x, arrpt.y);
	arrow.InflateRect(1,1);
	// Union the two bounds.
	CRect fullBounds;
	fullBounds.UnionRect(&bounds, &arrow);
//	fullBounds.OffsetRect(-mXScrollOffset, -mYScrollOffset);
	// Invalidate it.
	InvalidateRect(&fullBounds, false);
}

//=============================================================================
// CWorldBuilderView::drawObject
//=============================================================================
/** Draws the icon for an object at a given location.  Called by OnPaint. */
//=============================================================================
void CWorldBuilderView::drawObjectInView(CDC *pDc, MapObject *pMapObj)
{
	CPoint lpt;
	docToViewCoords(*pMapObj->getLocation(), &lpt);
	// un-adjuse by scroll offset since we are called from OnPaint, which adjusts by this amt
	lpt.x += mXScrollOffset;
	lpt.y += mYScrollOffset;

	CRect circle(lpt, lpt);
	circle.InflateRect(m_cellSize/2, m_cellSize/2);
	CPen ovalPen;
	CBrush brush;
	CGdiObject *savPen;
	CGdiObject *savBrush;
	// Create a pen to draw the icon of the object's color.
	ovalPen.CreatePen(PS_SOLID, 1, pMapObj->getColor());
	// Null brush, as the object icon is not filled.
	brush.CreateStockObject(NULL_BRUSH);
	// Select the pen & brush.
	savPen = pDc->SelectObject(&ovalPen);
	savBrush = pDc->SelectObject(&brush);
	// Draw the object's circle.
	pDc->Ellipse(&circle);	 

	// Draw the arrow
	Int delta = (m_cellSize+4)/4;
	Int smallDelta = (delta+4)/4;
	
 	float angle = pMapObj->getAngle();
	
	Vector3 head1(m_cellSize - delta, smallDelta, 0);
	Vector3 head2(m_cellSize - delta, -smallDelta, 0);
	head1.Rotate_Z(-angle);
	head2.Rotate_Z(-angle);

	Coord3D cpt2;
	WbDoc()->getObjArrowPoint(pMapObj, &cpt2);
	CPoint pt2;
	docToViewCoords(cpt2, &pt2);
	// un-adjuse by scroll offset since we are called from OnPaint, which adjusts by this amt
	pt2.x += mXScrollOffset;
	pt2.y += mYScrollOffset;
	pDc->MoveTo(lpt.x,lpt.y);
	pDc->LineTo(pt2.x, pt2.y);

	CPoint pt1;
	pt1.x = pt2.x - ROUND(head1.X);
	pt1.y = pt2.y - ROUND(head1.Y);
	pDc->LineTo(pt1);

	pDc->MoveTo(pt2.x, pt2.y);
	pt1.x = pt2.x - ROUND(head2.X);
	pt1.y = pt2.y - ROUND(head2.Y);
	pDc->LineTo(pt1);

	if (pMapObj->isSelected()) {
		// The object is selected, so draw selection feedback.
		CPoint pts[3];
		pts[0].x = lpt.x;
		pts[0].y = lpt.y;
		// Solid yellow pen and brush, as the selection feedback is filled.
		CPen selectPen;
		selectPen.CreatePen(PS_SOLID, 1, RGB(255,255,0));
		CBrush selectBrush;
		selectBrush.CreateSolidBrush(RGB(255,255,0));
		pDc->SelectObject(&selectPen);
		pDc->SelectObject(&selectBrush);
		// Draw a triangle centered about the object center.
		pts[0].x += delta;
		pts[0].y += delta;
		pts[1] = pts[0];
		pts[1].x -= 2*delta;
		pts[2].x = lpt.x;
		pts[2].y = lpt.y;
		pts[2].y -= 2*delta;
		pDc->Polygon(pts, 3);
		// Restore the previous brush & pen to prevent leaks.
		pDc->SelectObject(&ovalPen);
		pDc->SelectObject(&brush);
	}

	// Restore the previous brush & pen to prevent leaks.
	pDc->SelectObject(savPen);
	pDc->SelectObject(savBrush);
}

//=============================================================================
// CWorldBuilderView::drawContours
//=============================================================================
/** Draws the height contours on the map.  Called by OnPaint. */
//=============================================================================
void CWorldBuilderView::drawContours(CDC *pDc, CRgn *pRgn, Int minX, Int maxX, Int minY, Int maxY)
{
	CWorldBuilderDoc* pDoc = WbDoc();
	ASSERT_VALID(pDoc);
	
	WorldHeightMapEdit *pMap = getTrackingHeightMap();
	// We do cell and cell+1 in the loop, so trim back the max limits.
	if (maxX == pMap->getXExtent()) maxX--;
	if (maxY == pMap->getYExtent()) maxY--;

	Int curHeight;
	Bool didWater = false;  // We do the water level first, then step through the contours.
	for (curHeight = ContourOptions::getContourOffset(); curHeight < 255; 
		curHeight+= ContourOptions::getContourStep()) {
		Bool doingWater = false;
		CPen pen;
		CGdiObject *savObj;
		Int width = ContourOptions::getContourWidth();
		if (!didWater) {
			// Do the water first.
			didWater = true;
			doingWater = true;
			curHeight = ROUND(TheGlobalData->m_waterPositionZ/MAP_HEIGHT_SCALE);
			pen.CreatePen(PS_SOLID, width+1, RGB(0, 255, 255));
		}	else {
			pen.CreatePen(PS_SOLID, width, getColorForHeight(curHeight));
		}
		savObj = pDc->SelectObject(&pen);
		Int i, j;
		CRect rect;
		for (i=minX; i<maxX; i++) {
			for (j=minY; j<maxY; j++) {
				rect.bottom = (pMap->getYExtent()-j)*m_cellSize;
				rect.top = rect.bottom-m_cellSize;
				rect.left = i*m_cellSize;
				rect.right = rect.left+m_cellSize;
				if (!pRgn->RectInRegion(&rect)) {
					continue;
				}
				CPoint lowerLeftPt, lowerRightPt, upperLeftPt, upperRightPt;
				Int lowerLeft = pMap->getHeight(i, j);
				Int upperLeft = pMap->getHeight(i, j+1);
				Int upperRight = pMap->getHeight(i+1, j+1);
				Int lowerRight = pMap->getHeight(i+1, j);
				lowerLeftPt.x = rect.left;
				lowerLeftPt.y = rect.bottom;
				upperLeftPt.x = rect.left;
				upperLeftPt.y = rect.top;
				lowerRightPt.x = rect.right;
				lowerRightPt.y = rect.bottom;
				upperRightPt.x = rect.right;
				upperRightPt.y = rect.top;

				// if the square is flat, we have no contours.
				if (lowerLeft==upperRight && lowerLeft == upperLeft && upperLeft == lowerRight) {
					continue;
				}
				// If it is all below or all above, we have no contours.
				if (lowerLeft<curHeight && upperLeft<curHeight && upperRight<curHeight && lowerRight<curHeight) {
					continue;
				}
				if (lowerLeft>curHeight && upperLeft>curHeight && upperRight>curHeight && lowerRight>curHeight) {
					continue;
				}
				Bool drawLine = false;
				CPoint pt1, pt2;
				// Consider the lower left, upper left, upper right triangle.
				while (true) {
					if (lowerLeft != upperLeft || lowerLeft != upperRight) {
						if (lowerLeft == curHeight) {
							if (isBetween(curHeight, upperRight, upperLeft)) {
								pt1 = lowerLeftPt;
								interpolate(&pt2, curHeight, upperLeftPt, upperLeft, upperRightPt, upperRight);
								drawLine = true;
							}
							break;
						}
						if (upperLeft == curHeight) {
							if (isBetween(curHeight, upperRight, lowerLeft)) {
								pt1 = upperLeftPt;
								interpolate(&pt2, curHeight, upperRightPt, upperRight, lowerLeftPt, lowerLeft);
								drawLine = true;
							}
							break;
						}
						if (upperRight == curHeight) {
							if (isBetween(curHeight, lowerLeft, upperLeft)) {
								pt1 = upperRightPt;
								interpolate(&pt2, curHeight, lowerLeftPt, lowerLeft, upperLeftPt, upperLeft);
								drawLine = true;
							}
							break;
						}
						if (isBetween(curHeight, lowerLeft, upperLeft)) {
							interpolate(&pt1, curHeight, lowerLeftPt, lowerLeft, upperLeftPt, upperLeft);
							if (isBetween(curHeight, upperRight, upperLeft)) {
								interpolate(&pt2, curHeight, upperLeftPt, upperLeft, upperRightPt, upperRight);
								drawLine = true;
							}
							if (isBetween(curHeight, upperRight, lowerLeft)) {
								interpolate(&pt2, curHeight, upperRightPt, upperRight, lowerLeftPt, lowerLeft);
								drawLine = true;
							}
							break;
						}
						if (isBetween(curHeight, upperRight, upperLeft)) {
							interpolate(&pt1, curHeight, upperLeftPt, upperLeft, upperRightPt, upperRight);
							if (isBetween(curHeight, upperRight, lowerLeft)) {
								interpolate(&pt2, curHeight, upperRightPt, upperRight, lowerLeftPt, lowerLeft);
								drawLine = true;
							}
							break;
						}
					}
					break;
				}
				if (drawLine) {
					pDc->MoveTo(pt1);
					pDc->LineTo(pt2);
				}

				drawLine = false;
				// Consider the lower left, upper right, lowerRight triangle.
				while (true) {
					if (lowerLeft != lowerRight || lowerLeft != upperRight) {
						if (lowerLeft == curHeight) {
							if (isBetween(curHeight, upperRight, lowerRight)) {
								pt1 = lowerLeftPt;
								interpolate(&pt2, curHeight, lowerRightPt, lowerRight, upperRightPt, upperRight);
								drawLine = true;
							}
							break;
						}
						if (lowerRight == curHeight) {
							if (isBetween(curHeight, upperRight, lowerLeft)) {
								pt1 = lowerRightPt;
								interpolate(&pt2, curHeight, upperRightPt, upperRight, lowerLeftPt, lowerLeft);
								drawLine = true;
							}
							break;
						}
						if (upperRight == curHeight) {
							if (isBetween(curHeight, lowerLeft, lowerRight)) {
								pt1 = upperRightPt;
								interpolate(&pt2, curHeight, lowerLeftPt, lowerLeft, lowerRightPt, lowerRight);
								drawLine = true;
							}
							break;
						}

						if (isBetween(curHeight, lowerLeft, lowerRight)) {
							interpolate(&pt1, curHeight, lowerLeftPt, lowerLeft, lowerRightPt, lowerRight);
							if (isBetween(curHeight, upperRight, lowerLeft)) {
								interpolate(&pt2, curHeight, lowerLeftPt, lowerLeft, upperRightPt, upperRight);
								drawLine = true;
							}
							if (isBetween(curHeight, upperRight, lowerRight)) {
								interpolate(&pt2, curHeight, upperRightPt, upperRight, lowerRightPt, lowerRight);
								drawLine = true;
							}
							break;
						}
						if (isBetween(curHeight, upperRight, lowerRight)) {
							interpolate(&pt1, curHeight, lowerRightPt, lowerRight, upperRightPt, upperRight);
							if (isBetween(curHeight, upperRight, lowerLeft)) {
								interpolate(&pt2, curHeight, upperRightPt, upperRight, lowerLeftPt, lowerLeft);
								drawLine = true;
							}
							break;
						}
					}
					break;
				}
				if (drawLine) {
					pDc->MoveTo(pt1);
					pDc->LineTo(pt2);
				}
			}
		}
		pDc->SelectObject(savObj);
		if (doingWater) {
			curHeight = ContourOptions::getContourOffset() - ContourOptions::getContourStep();
		}
	}
}	


//=============================================================================
// CWorldBuilderView::interpolate
//=============================================================================
/** Given 2 points, pt1 and pt2, with different height valus ht1 and ht2, 
determines where the height ht occurs along the line. */
//=============================================================================
void CWorldBuilderView::interpolate(CPoint *pt, Int ht, CPoint pt1, Int ht1, CPoint pt2, Int ht2)
{
	DEBUG_ASSERTCRASH((ht1!=ht2),("oops"));
	// Paranoid check to avoid divide by zero.
	if (ht1==ht2) {
		*pt = pt1;
		return;
	}
	Int delta = ht2 - ht1;
	Int d1 = ht2-ht;
	Int d2 = ht-ht1;
	DEBUG_ASSERTCRASH((d1+d2==delta),("oops"));
	// Interpolate between pt1 and pt2.
	pt->x = (pt1.x*d1 + pt2.x*d2)/delta;
	pt->y = (pt1.y*d1 + pt2.y*d2)/delta;
}
	
//=============================================================================
// CWorldBuilderView::drawMyTexture
//=============================================================================
/** Draws the tile data rgbData at pRect.  The tile data is square 4 byte 
data, width pixels wide and tall. */
//=============================================================================
void CWorldBuilderView::drawMyTexture(CDC *pDc, CRect *pRect, Int width, UnsignedByte *rgbData)
{
	// Just blast about some dib bits.
	BITMAPINFO bi;
	bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
	bi.bmiHeader.biWidth = width;
	bi.bmiHeader.biHeight = width; /* match display top left == 0,0 */
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;
	bi.bmiHeader.biSizeImage = (width*width)*(bi.bmiHeader.biBitCount/8);
	bi.bmiHeader.biXPelsPerMeter = 1000;
	bi.bmiHeader.biYPelsPerMeter = 1000;
	bi.bmiHeader.biClrUsed = 0;
	bi.bmiHeader.biClrImportant = 0;

	/*int val=*/::StretchDIBits(pDc->m_hDC, pRect->left, pRect->top, pRect->Width(), pRect->Height(), 0, 0, width, width, rgbData, &bi, 
		DIB_RGB_COLORS, SRCCOPY);
}


//=============================================================================
// CWorldBuilderView::invalidateCellInView
//=============================================================================
/** Invalidates the pixel area for the height map call at a particular location. */
//=============================================================================
void CWorldBuilderView::invalidateCellInView(int xIndex, int yIndex) 
{
	CWorldBuilderDoc* pDoc = WbDoc();
	ASSERT_VALID(pDoc);
	WorldHeightMapEdit *pMap = pDoc->GetHeightMap();

	// Convert the index to pixel coordinates.
	CRect cell;
	// Flip the y.
	cell.bottom = (pMap->getYExtent()-yIndex)*m_cellSize;
	// Adjust for scrolling.
	cell.bottom -= mYScrollOffset; 
	cell.top = cell.bottom-m_cellSize;
	// Convert to pixel coord.
	cell.left = xIndex*m_cellSize;
	// adjust for scrolling.
	cell.left -= mXScrollOffset;
	cell.right = cell.left+m_cellSize;
	InvalidateRect(&cell, false);
}


void CWorldBuilderView::adjustDocSize()
{
	CWorldBuilderDoc* pDoc = WbDoc();
	ASSERT_VALID(pDoc);
	// Initialize to a sensible default as we may get redrawn before we load a map.
	Int height = m_cellSize*256;
	Int width = m_cellSize*256;

	WorldHeightMapEdit *pMap = pDoc->GetHeightMap();
	// If we have a map loaded, use the actual size.
	if (pMap) {
		height = m_cellSize * pMap->getYExtent();
		width = m_cellSize * pMap->getXExtent();
	}
	CRect clientRect;
	GetClientRect(&clientRect);
	Int flippedY = m_centerPt.Y;
	if (pMap && flippedY>0) {
		flippedY = pMap->getYExtent()-flippedY;
	}
	// Calculate the scroll offset to place the cell m_centerX, m_centerY in the middle.
	mXScrollOffset = (m_centerPt.X*m_cellSize)-clientRect.Width()/2;
	mYScrollOffset = (flippedY*m_cellSize)-clientRect.Height()/2;
	// Calculate the scroll bar limits so we can have up to 1/2 screen of white space.
	m_scrollMin.x = -clientRect.Width()/2;
	m_scrollMax.x = width-clientRect.Width()/2;
	m_scrollMin.y = -clientRect.Height()/2;
	m_scrollMax.y = width-clientRect.Height()/2;
	// Clip the offset so it fits inside the range.
	if (mXScrollOffset > m_scrollMax.x) {
		mXScrollOffset = m_scrollMax.x;
	}
	if (mYScrollOffset > m_scrollMax.y) {
		mYScrollOffset = m_scrollMax.y;
	}
	// Set the scroll range
	SetScrollRange(SB_HORZ, m_scrollMin.x, m_scrollMax.x, false);
	SetScrollRange(SB_VERT, m_scrollMin.y, m_scrollMax.y, false);
	// Set the position of the scroll button in the scroll bars.
	SetScrollPos(SB_HORZ, mXScrollOffset, true);
	SetScrollPos(SB_VERT, mYScrollOffset, true);
}

//=============================================================================
// CWorldBuilderView::OnSize
//=============================================================================
/** Standard window handler method for when the window size changes. */
//=============================================================================
void CWorldBuilderView::OnSize(UINT nType, int cx, int cy) 
{
	// Do parent class processing.
	WbView::OnSize(nType, cx, cy);
	adjustDocSize();
}


//=============================================================================
// CWorldBuilderView::OnVScroll
//=============================================================================
/** Standard window handler method for when the vertical scroll bar is used. */
//=============================================================================
void CWorldBuilderView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	int scrollPixels=0;
	// Based on the opcode, determine how may pixels to scroll.
	switch(nSBCode) {
		case SB_LINEDOWN: scrollPixels = m_cellSize; break;
		case SB_LINEUP: scrollPixels = -m_cellSize; break;
		case SB_PAGEDOWN: scrollPixels = 10*m_cellSize; break;
		case SB_PAGEUP: scrollPixels = -10*m_cellSize; break;
		case SB_THUMBPOSITION: scrollPixels = nPos-mYScrollOffset; break;
		case SB_THUMBTRACK: scrollPixels = nPos-mYScrollOffset; break;
		default: break;
	}
	scrollInView(0, -(Real)scrollPixels/m_cellSize, nSBCode != SB_THUMBTRACK);
}

//=============================================================================
// CWorldBuilderView::OnHScroll
//=============================================================================
/** Standard window handler method for when the horizontal scroll bar is used. */
//=============================================================================
void CWorldBuilderView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	int scrollPixels=0;

	switch(nSBCode) {
		case SB_LINEDOWN: scrollPixels = m_cellSize; break;
		case SB_LINEUP: scrollPixels = -m_cellSize; break;
		case SB_PAGEDOWN: scrollPixels = 10*m_cellSize; break;
		case SB_PAGEUP: scrollPixels = -10*m_cellSize; break;
		case SB_THUMBPOSITION: scrollPixels = nPos-mXScrollOffset; break;
		case SB_THUMBTRACK: scrollPixels = nPos-mXScrollOffset; break;
		default: return;
	}
	scrollInView((Real)scrollPixels/m_cellSize, 0, nSBCode != SB_THUMBTRACK);
}

//=============================================================================
// CWorldBuilderView::scroll
//=============================================================================
/** Scrolls the window. */
//=============================================================================
void CWorldBuilderView::scrollInView(Real xScroll, Real yScroll, Bool end) 
{
	// Calculate new scroll offset.
	Int newOffset = mXScrollOffset + xScroll*m_cellSize;
	// Clip it to our maximum scroll range.
	if (newOffset < m_scrollMin.x) newOffset = m_scrollMin.x;
	if (newOffset > m_scrollMax.x) newOffset = m_scrollMax.x;
	// Back calculate the number of pixels to scroll.
	xScroll = newOffset - mXScrollOffset;
	mXScrollOffset = newOffset;

	// Calculate new scroll offset.
	newOffset = mYScrollOffset - yScroll*m_cellSize;
	// Clip it to our maximum scroll range.
	if (newOffset < m_scrollMin.y) newOffset = m_scrollMin.y;
	if (newOffset > m_scrollMax.y) newOffset = m_scrollMax.y;
	// Back calculate the number of pixels to scroll.
	yScroll = newOffset - mYScrollOffset;
	mYScrollOffset = newOffset;

	if (xScroll || yScroll) {
		ScrollWindow(-xScroll, -yScroll);
		SetScrollPos(SB_HORZ, mXScrollOffset, true);
		SetScrollPos(SB_VERT, mYScrollOffset, true);
		// Update the center indexes.
		CRect client;
		GetClientRect(&client);
		// pt is the center point in pixels, 0 based.
		CPoint pt((client.left+client.right)/2+mXScrollOffset, (client.bottom+client.top)/2+mYScrollOffset);
		CWorldBuilderDoc* pDoc = WbDoc();
		WorldHeightMapEdit *pMap = pDoc->GetHeightMap();
		if (pMap==NULL) return;
		m_centerPt.X = (Real)(pt.x)/m_cellSize;
		m_centerPt.Y = pMap->getYExtent() - (Real)(pt.y)/m_cellSize;
		constrainCenterPt();
	}
	if (end)
		WbDoc()->syncViewCenters(m_centerPt.X, m_centerPt.Y);
}



/** Toggles the show grid flag and invals the window. */
void CWorldBuilderView::OnShowGrid() 
{
	mShowGrid = !mShowGrid;
	::AfxGetApp()->WriteProfileInt(MAIN_FRAME_SECTION, "ShowGrid", mShowGrid?1:0);
	Invalidate(false);
}

/** Sets the check in the menu to match the show grid flag. */
void CWorldBuilderView::OnUpdateShowGrid(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(mShowGrid?1:0);
	// Note - grid doesn't show if the cell size < 4, so disable.
	pCmdUI->Enable(m_cellSize>=MIN_GRID_SIZE);
	
}

#if DEAD
/** Toggles the show contours flag and invals the window. */
void CWorldBuilderView::OnViewShowcontours() 
{
	m_showContours = !m_showContours;
	::AfxGetApp()->WriteProfileInt(MAIN_FRAME_SECTION, "ShowContours", m_showContours?1:0);
	Invalidate(false);
}

/** Sets the check in the menu to match the show contours flag. */
void CWorldBuilderView::OnUpdateViewShowcontours(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_showContours?1:0);
	// Note - contours don't show if the cell size < 4, so disable.
	pCmdUI->Enable(m_cellSize>=MIN_GRID_SIZE);
}
#endif 


/** Toggles the show texture flag and invals the window. */
void CWorldBuilderView::OnViewShowtexture() 
{
	m_showTexture = !m_showTexture;
	::AfxGetApp()->WriteProfileInt(MAIN_FRAME_SECTION, "ShowTexture", m_showTexture?1:0);
	Invalidate(false);
}

/** Sets the check in the menu to match the show texture flag. */
void CWorldBuilderView::OnUpdateViewShowtexture(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_showTexture?1:0);
}


// This code confuses doxygen, so stick it at the end of the file.

IMPLEMENT_DYNCREATE(CWorldBuilderView, WbView)

BEGIN_MESSAGE_MAP(CWorldBuilderView, WbView)
	//{{AFX_MSG_MAP(CWorldBuilderView)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_COMMAND(IDM_ShowGrid, OnShowGrid)
	ON_UPDATE_COMMAND_UI(IDM_ShowGrid, OnUpdateShowGrid)
	ON_WM_CREATE()
	ON_COMMAND(ID_VIEW_SHOWTEXTURE, OnViewShowtexture)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWTEXTURE, OnUpdateViewShowtexture)
//	ON_COMMAND(ID_VIEW_SHOWCONTOURS, OnViewShowcontours)
//	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWCONTOURS, OnUpdateViewShowcontours)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()



// ----------------------------------------------------------------------------
Bool CWorldBuilderView::viewToDocCoords(CPoint curPt, Coord3D *newPt, Bool constrain)
{
	// Flip the y.
	WorldHeightMapEdit *pMap = WbDoc()->GetHeightMap();
	newPt->x = (Real)(curPt.x + mXScrollOffset)/m_cellSize;
	newPt->y = pMap->getYExtent() - (Real)(curPt.y + mYScrollOffset)/m_cellSize;
	newPt->x *= MAP_XY_FACTOR;
	newPt->y *= MAP_XY_FACTOR;
	newPt->z = MAGIC_GROUND_Z;
	if (constrain) {
		if (m_doLockAngle) {
			Real dy = fabs(m_mouseDownDocPoint.y - newPt->y);
			Real dx = fabs(m_mouseDownDocPoint.x - newPt->x);
			if (dx>2*dy) {
				// lock to dx.
				newPt->y = m_mouseDownDocPoint.y;
			} else if (dy>2*dx) {
				//lock to dy.
				newPt->x = m_mouseDownDocPoint.x;
			} else {
				// Lock to 45 degree.
				dx = (dx+dy)/2;
				dy = dx;
				if (newPt->x < m_mouseDownDocPoint.x) dx = -dx;
				if (newPt->y < m_mouseDownDocPoint.y) dy = -dy;
				newPt->x = m_mouseDownDocPoint.x+dx;
				newPt->y = m_mouseDownDocPoint.y+dy;
			}
		}
	}
#ifdef X_DEBUG
CPoint curPt2;
docToViewCoords(*newPt, &curPt2);
DEBUG_ASSERTCRASH((curPt.x==curPt2.x),("oops"));
DEBUG_ASSERTCRASH((curPt.y==curPt2.y),("oops"));
#endif
	return true;
}

// ----------------------------------------------------------------------------
Bool CWorldBuilderView::docToViewCoords(Coord3D curPt, CPoint* newPt)
{
	WorldHeightMapEdit *pMap = WbDoc()->GetHeightMap();
	newPt->x = (curPt.x/MAP_XY_FACTOR) * m_cellSize - mXScrollOffset;
	newPt->y = (pMap->getYExtent() - curPt.y/MAP_XY_FACTOR) * m_cellSize - mYScrollOffset;
#ifdef X_DEBUG
Coord3D curPt2;
viewToDocCoords(*newPt, &curPt2);
DEBUG_ASSERTCRASH((abs(curPt.x-curPt2.x)<1),("oops"));
DEBUG_ASSERTCRASH((abs(curPt.y-curPt2.y)<1),("oops"));
#endif
	return true;
}

