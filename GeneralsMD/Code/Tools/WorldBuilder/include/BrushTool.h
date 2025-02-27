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

// BrushTool.h
// Texture tiling tools for worldbuilder.
// Author: John Ahlquist, April 2001

#pragma once

#include "Tool.h"
class WorldHeightMapEdit;
/*************************************************************************/
/**                             BrushTool
	 Does the Height Brush tool operation.
***************************************************************************/
///  Height brush tool.
class BrushTool : public Tool
{
protected:
	WorldHeightMapEdit *m_htMapEditCopy; ///< ref counted.
	WorldHeightMapEdit *m_htMapFeatherCopy; ///< ref counted.

	static Int m_brushWidth;
	static Int m_brushFeather;
	static Bool m_brushSquare;
	static Int m_brushHeight;

public:
	BrushTool(void);
	~BrushTool(void);

public:
	static Int getWidth(void) {return m_brushWidth;};  ///<Returns width.
	static Int getFeather(void) {return m_brushFeather;}; ///<Returns feather.
	static Int getHeight(void) {return m_brushHeight;}; ///<Returns height.
	static void setWidth(Int width);
	static void setFeather(Int feather);
	static void setHeight(Int height);

public:
	virtual void mouseDown(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual void mouseUp(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual void mouseMoved(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual WorldHeightMapEdit *getHeightMap(void) {return m_htMapEditCopy;};
	virtual void activate(); ///< Become the current tool.
	virtual Bool followsTerrain(void) {return false;};

};
