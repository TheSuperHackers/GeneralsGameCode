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

// RoadTool.h
// Texture tiling tools for worldbuilder.
// Author: John Ahlquist, June 2001

#pragma once

#ifndef RoadTool_H
#define RoadTool_H

#include "Tool.h"
#include "W3DDevice/GameClient/WorldHeightMap.h"
class WorldHeightMapEdit;
#define ROAD_SNAP_DISTANCE (1.0f)
/*************************************************************************/
/**                             RoadTool
	 Does the Add a section of road tool operation.
***************************************************************************/
///  Road segment tool.
class RoadTool : public Tool
{
protected:
	enum {HYSTERESIS = 3,
				MIN_LENGTH = 4};
	MapObject *m_mapObj;

private:
	MapObject* findSegment(const Coord3D *pLoc, Coord3D *outLoc);

public:
	RoadTool(void);
	~RoadTool(void);

public:
	static Bool snap(Coord3D *pLoc, Bool skipLast);

public:
	/// Perform tool on mouse down.
	virtual void mouseDown(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual void mouseMoved(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual void mouseUp(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual void activate(); ///< Become the current tool.
};


#endif //TOOL_H
