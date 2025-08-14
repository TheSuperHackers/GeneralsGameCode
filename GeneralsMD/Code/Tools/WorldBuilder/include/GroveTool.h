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

// GroveTool.h
// Author: John Ahlquist, May 2001

#pragma once

#ifndef GroveTool_H
#define GroveTool_H

#include "Lib/BaseType.h"
#include "Tool.h"
#include "Common/MapObject.h"
class WorldHeightMapEdit;

/*************************************************************************/
/**                             GroveTool
	 Does the add a grove of trees tool operation.
***************************************************************************/
///  Add a grove of trees tool.
class GroveTool : public Tool
{
protected:
	enum {HYSTERESIS = 3};
	CPoint m_downPt;
	Bool m_dragging;
	MapObject *m_headMapObj;

protected:
	void plantTree( Coord3D *pos );
	void plantShrub( Coord3D *pos );
	void plantGrove( Coord3D pos, Coord3D prevDir, Real baseHeight, Int level, CPoint bounds );
	void _plantGroveInBox(CPoint tl, CPoint br, WbView* pView);

	void addObj(Coord3D *pos, AsciiString name);
	void activate();

public:
	GroveTool(void);
	~GroveTool(void);

public:
	/// Perform tool on mouse down.
	virtual void mouseDown(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual void mouseUp(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual void mouseMoved(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
};


#endif //TOOL_H
