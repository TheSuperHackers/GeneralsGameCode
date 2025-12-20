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

// WaypointTool.h
// Texture selection tool for worldbuilder.
// Author: John Ahlquist, April 2001

#pragma once

#include "Tool.h"
class WorldHeightMapEdit;
class MapObject;
/*************************************************************************/
/**                             WaypointTool
	 Does the add/select waypoint operation.
***************************************************************************/
///  Select tile tool.
class WaypointTool : public Tool
{
public:
	WaypointTool(void);
	~WaypointTool(void);

protected:
	Int m_downWaypointID;
	Coord3D m_mouseDownPt;
	static Bool m_isActive;

protected:
	MapObject *pickWaypoint(Coord3D loc);

public:
	static Bool isActive(void) {return m_isActive;};

public:
	/// Perform tool on mouse down.
	virtual void mouseDown(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual void mouseMoved(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual void mouseUp(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual void activate(); ///< Become the current tool.
	virtual void deactivate(); ///< Become not the current tool.
};
