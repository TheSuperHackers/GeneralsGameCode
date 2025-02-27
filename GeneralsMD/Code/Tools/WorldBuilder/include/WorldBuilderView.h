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

// WorldBuilderView.h : interface of the CWorldBuilderView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Lib/BaseType.h"
#include "W3DDevice/GameClient/WorldHeightMap.h"
#include "wbview.h"

//#include "WW3D_SimpleWindow.h"

#define MIN_GRID_SIZE 4
class MapObject;
class CWorldBuilderDoc;

class CWorldBuilderView : public WbView
{
protected: // create from serialization only
	CWorldBuilderView();
	DECLARE_DYNCREATE(CWorldBuilderView)

// Operations
public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWorldBuilderView)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnDraw(CDC* pDC);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CWorldBuilderView();
#ifdef RTS_DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	Int			m_cellSize;						///< Size of a height map cell in pixels when drawn in the 2d view.
	Bool		m_showContours;				///< Flag whether contours are drawn in the 2d view.

	Bool		m_showTexture;							///< Flag whether textures are drawn in the 2d view.
	Bool		mShowGrid;								  ///< Flag whether the gray grid is drawn in the 2d view.

	Int			mXScrollOffset;								///< The x offset to the upper left corner of the screen.
	Int			mYScrollOffset;								///< The y offset to the upper left corner of the screen.
	CPoint	m_scrollMin;								///< The minimum scrollbar positions.
	CPoint	m_scrollMax;								///< The maximum scroll bar positions.

protected:

	/// Draw a texture bitmap in a rectangle in the dc.
	void drawMyTexture(CDC *pDc, CRect *pRect, Int width, UnsignedByte *rgbData);

	/// Get a color for a height value.
	DWORD getColorForHeight(UnsignedByte ht);

	/// Draw the contours for the height map in the dc.
	void drawContours(CDC *pDc, CRgn *pRgn, Int minX, Int maxX, Int minY, Int maxY);

	/// Compound boolean expression.
	static inline Bool isBetween(Int cur, Int first, Int second) {
		Bool is = false;
		if (cur>=first && cur<=second) is = true;
		if (cur<=first && cur>=second) is = true;
		return(is);
	}

	/// Interpolate the point at a given height between 2 points.
	void interpolate(CPoint *pt, Int ht, CPoint pt1, Int ht1, CPoint pt2, Int ht2);

	/// Draw the object's icon in the dc at a given point.
	void drawObjectInView(CDC *pDc, MapObject *pMapObj);

public:
	/// Get the current draw size in pixels in the 2d window of one height map cell.
	Int getCellSize(void) {return m_cellSize;}

	/// Sets the current draw size.
	void setCellSize(Int cellSize);

	/// Set whether contours are drawn.
	Bool getShowContours(void) {return m_showContours;}
	/// Set whether contours are drawn.
	void setShowContours(Bool show);
	/// Update the center to match a center point from the 3d view.
	void updateCenterFromMapPoint(Real x, Real y);

protected:

public:
	virtual Bool viewToDocCoords(CPoint curPt, Coord3D *newPt, Bool constrain);
	virtual Bool docToViewCoords(Coord3D curPt, CPoint* newPt);

	/// Set the center for display.
	virtual void setCenterInView(Real x, Real y);

	/// the doc has changed size; readjust view as necessary.
	virtual void adjustDocSize();

	/// Invalidates an object. Pass NULL to inval all objects.
	virtual void invalObjectInView(MapObject *pObj);

	/// Invalidates the area of one height map cell in the 2d view.
	virtual void invalidateCellInView(int xIndex, int yIndex);

	/// Scrolls the window by this amount (doc coords).
	virtual void scrollInView(Real x, Real y, Bool end);

// Generated message map functions
protected:
	//{{AFX_MSG(CWorldBuilderView)
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnShowGrid();
	afx_msg void OnUpdateShowGrid(CCmdUI* pCmdUI);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnViewShowtexture();
	afx_msg void OnUpdateViewShowtexture(CCmdUI* pCmdUI);
//	afx_msg void OnViewShowcontours();
//	afx_msg void OnUpdateViewShowcontours(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
