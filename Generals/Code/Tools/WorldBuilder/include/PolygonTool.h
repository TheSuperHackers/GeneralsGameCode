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

// PolygonTool.h
// Polygon area trigger tool for worldbuilder.
// Author: John Ahlquist, Nov. 2001

#pragma once

#ifndef POLYGONTOOL_H
#define POLYGONTOOL_H

#include "Tool.h"
class WorldHeightMapEdit;
class MapObject;
class PolygonTrigger;
class MovePolygonUndoable;
/*************************************************************************/
/**                             PolygonTool
	 Does the add/select polygon operation.
***************************************************************************/
///  Select tile tool.
class PolygonTool : public Tool
{
public:
	PolygonTool(void);
	~PolygonTool(void);

protected:
	Coord3D m_poly_mouseDownPt;
	Coord3D m_poly_unsnappedMouseDownPt;
	Bool		m_poly_isDraggingPoint;
	Bool		m_poly_justPicked;

	Bool m_poly_mouseUpPlus;///< True if we are over the "Add point" hotspot.
	HCURSOR m_poly_plusCursor;
	Bool m_poly_mouseUpMove;///< True if we are over the "move" hotspot.
	HCURSOR m_poly_moveCursor;

	MovePolygonUndoable *m_poly_moveUndoable;

	static Bool		m_poly_isAdding;
	static Int		m_poly_dragPointNdx;
	static Bool		m_poly_isActive;
	static PolygonTrigger *m_poly_curSelectedPolygon;

protected:
	static Int poly_pickPoint(PolygonTrigger *pTrig, CPoint viewPt, WbView* pView);
	static Int poly_getInsertIndex(PolygonTrigger *pTrig, Coord3D loc);
	Bool poly_snapToPoly(Coord3D *pLoc);
	static Bool poly_pickPoly(PolygonTrigger *pTrig, Coord3D pLoc, Int tolerance);
	void poly_pickOnMouseDown(CPoint viewPt, WbView* pView);
	void startMouseDown(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);

public:
	static Bool isActive(void) {return m_poly_isActive;};
	static Bool deleteSelectedPolygon(void);
	static Bool isSelected(PolygonTrigger *pTrig) {return (pTrig && (pTrig==m_poly_curSelectedPolygon));};
	static Int getSelectedPointNdx(void) {return m_poly_dragPointNdx;};
	static PolygonTrigger *pickPolygon(Coord3D loc, CPoint viewPt, WbView* pView);
	static Bool isSelectedOpen(void) {return m_poly_isAdding;};

public:
	/// Perform tool on mouse down.
	virtual void mouseDown(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual void mouseMoved(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual void mouseUp(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual void setCursor(void);
	virtual void activate(); ///< Become the current tool.
	virtual void deactivate(); ///< Become not the current tool.
};


#endif //POLYGONTOOL_H

