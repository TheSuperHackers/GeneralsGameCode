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

// HandScrollTool.h
// Scrolling tool for worldbuilder.
// Author: John Ahlquist, May 2001

#pragma once

#include "Tool.h"
/**************************************************************************
                            HandScrollTool
***************************************************************************/
///  Scroll tool.
class HandScrollTool : public Tool
{
public:
	HandScrollTool(void);
	~HandScrollTool(void);

protected:
	enum {HYSTERESIS = 3};
	CPoint	m_prevPt2d;
	CPoint	m_downPt2d;
	Bool		m_scrolling;
	UINT		m_mouseDownTime;		// if m_trackingMode != TRACK_NONE, tickcount when mouse went down

public:
	/// Start scrolling.
	virtual void mouseDown(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	/// Scroll.
	virtual void mouseMoved(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	/// End scroll.
	virtual void mouseUp(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual void activate(); ///< Become the current tool.
	virtual Bool followsTerrain(void) {return false;};
};
