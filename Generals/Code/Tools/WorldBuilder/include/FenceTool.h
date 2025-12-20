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

// FenceTool.h
// Texture tiling tools for worldbuilder.
// Author: John Ahlquist, April 2001

#pragma once

#include "Tool.h"
class WorldHeightMapEdit;
class MapObject;
class Vector3;
/*************************************************************************/
/**                             FenceTool
	 Does the fence tool operation.
***************************************************************************/
class FenceTool : public Tool
{
protected:
	CPoint		m_downPt2d;
	Coord3D		m_downPt3d;
	MapObject *m_mapObjectList;
	Real			m_curObjectWidth;
	Real			m_curObjectOffset;
	Int				m_objectCount;

public:
	FenceTool(void);
	~FenceTool(void);

protected:
	void updateMapObjectList(Coord3D downPt, Coord3D curPt, WbView* pView, CWorldBuilderDoc *pDoc, Bool checkPlayers);

public:
	virtual void mouseDown(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual void mouseUp(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual void mouseMoved(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual void activate(); ///< Become the current tool.
	virtual void deactivate(); ///< Become not the current tool.
};
