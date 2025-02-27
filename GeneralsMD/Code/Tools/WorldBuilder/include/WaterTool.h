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

// WaterTool.h
// Polygon area trigger tool for worldbuilder.
// Author: John Ahlquist, Nov. 2001

#pragma once

#include "PolygonTool.h"
class WorldHeightMapEdit;
class MapObject;
class PolygonTrigger;
class MovePolygonUndoable;
/*************************************************************************/
/**                             WaterTool
	 Does the add/select water polygon operation.
***************************************************************************/
///  Select tile tool.
class WaterTool : public PolygonTool
{
public:
	WaterTool(void);
	~WaterTool(void);

protected:
	static Bool		m_water_isActive;

	Real	m_currentZ;

public:
	static Bool isActive(void) {return m_water_isActive;};

public:
	/// Perform tool on mouse down.
	virtual void mouseDown(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual void mouseMoved(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual void mouseUp(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual void setCursor(void);
	virtual void activate(); ///< Become the current tool.
	virtual void deactivate(); ///< Become not the current tool.

protected:
	void fillTheArea(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	PolygonTrigger *adjustSpacing(PolygonTrigger *trigger, Real spacing);
};
