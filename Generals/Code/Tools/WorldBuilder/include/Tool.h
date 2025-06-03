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

// Tool.h
// Tool classes for worldbuilder.
// Author: John Ahlquist, April 2001

#pragma once

#ifndef TOOL_H
#define TOOL_H

#include "Lib/BaseType.h"
#include "Common/STLTypedefs.h"

enum TTrackingMode {
	TRACK_NONE,
	TRACK_L,
	TRACK_M,
	TRACK_R
};

#include <vector>
typedef std::vector<CPoint> VecHeightMapIndexes;

#define MAGIC_GROUND_Z (0)

//#define IS_MAGIC_GROUND(z) ((z)==MAGIC_GROUND_Z)

// for backwards compatibility with existing maps:
#define IS_MAGIC_GROUND(z) ((z)==0)

class CWorldBuilderDoc;
class CWorldBuilderView;
class WorldHeightMapEdit;
class WbView;

/*************************************************************************
**                             Tool
***************************************************************************/
class Tool  
{
protected:
	Int	m_toolID;  //< Tool button ui id in resource.
	Int	m_cursorID;  //< Tool button ui id in resource.
	HCURSOR m_cursor;

	Int m_prevXIndex;
	Int m_prevYIndex;
public:
	Tool(Int toolID, Int cursorID);
	virtual ~Tool(void);

public:
	Int getToolID(void) {return m_toolID;}
	virtual void setCursor(void);

	virtual void activate(); ///< Become the current tool.
	virtual void deactivate(){}; ///< Become not the current tool.

	virtual Bool followsTerrain(void) {return true;};	 ///< True if the tool tracks the terrain, generally false if it modifies the terrain heights.

	virtual void mouseMoved(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc) {}
	virtual void mouseDown(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc) {}
	virtual void mouseUp(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc) {}
	virtual WorldHeightMapEdit *getHeightMap(void) {return NULL;}

	static Real calcRoundBlendFactor(CPoint center, Int x, Int y, Int brushWidth, Int featherWidth);
	static Real calcSquareBlendFactor(CPoint center, Int x, Int y, Int brushWidth, Int featherWidth);
	static void getCenterIndex(Coord3D *docLocP, Int brushWidth, CPoint *center, CWorldBuilderDoc *pDoc);
	static void getAllIndexesIn(const Coord3D *bl, const Coord3D *br, 
															const Coord3D *tl, const Coord3D *tr, 
															Int widthOutside, CWorldBuilderDoc *pDoc, 
															VecHeightMapIndexes* allIndices);
};


#endif //TOOL_H
