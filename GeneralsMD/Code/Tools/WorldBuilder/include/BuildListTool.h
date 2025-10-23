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

// BuildListTool.h
// Build list tool for worldbuilder.
// Author: John Ahlquist, Nov 2001

#pragma once

#include "Tool.h"
#include "Common/AsciiString.h"
#include "PickUnitDialog.h"
class WorldHeightMapEdit;
class BuildListInfo;

/*************************************************************************/
/**                             BuildListTool
	 Does the add item to build list tool operation.
***************************************************************************/
class BuildListTool : public Tool
{
protected:
	CPoint m_downPt2d;
	Coord3D m_downPt3d;
	Coord3D m_prevPt3d;
	BuildListInfo *m_curObject;

	Bool m_moving; ///< True if we are drag moving an object.
	Bool m_rotating; ///< True if we are rotating an object.

	Bool m_mouseUpRotate;///< True if we are over the "rotate" hotspot.
	HCURSOR m_rotateCursor;
	Bool m_mouseUpMove;///< True if we are over the "move" hotspot.
	HCURSOR m_moveCursor;
	HCURSOR m_pointerCursor;
	PickUnitDialog m_pickBuildingDlg;
	Bool m_created;

	static PickUnitDialog *m_static_pickBuildingDlg;
	static Bool	m_isActive;

public:
	BuildListTool(void);
	~BuildListTool(void);

private:
	void createWindow(void);
	Bool isDoingAdd(void);

public:
	static void addBuilding(void);
	static Bool isActive(void) {return m_isActive;};

public:
	/// Perform tool on mouse down.
	virtual void mouseDown(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual void mouseUp(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual void mouseMoved(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual void setCursor(void);
	virtual void activate(); ///< Become the current tool.
	virtual void deactivate(); ///< Become not the current tool.
};
