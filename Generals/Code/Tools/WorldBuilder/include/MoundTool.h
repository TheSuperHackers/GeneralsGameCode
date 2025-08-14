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

// MoundTool.h
// Texture tiling tools for worldbuilder.
// Author: John Ahlquist, April 2001

#pragma once

#ifndef MOUND_TOOL_H
#define MOUND_TOOL_H

#include "Tool.h"
class WorldHeightMapEdit;
/*************************************************************************
**                             MoundTool
***************************************************************************/
class MoundTool : public Tool
{
	enum {MIN_DELAY_TIME=60}; // 60 ms minimum delay between applications.
protected:
	WorldHeightMapEdit *m_htMapEditCopy; //< ref counted.
	WorldHeightMapEdit *m_htMapSaveCopy; //< ref counted.
	Bool								m_raising;  ///< True if mounding, false if digging.
	Int									m_lastMoveTime;  ///< Last system clock time.

	static Int m_moundHeight;
	static Int m_brushWidth;
	static Int m_brushFeather;

public:
	MoundTool(void);
	~MoundTool(void);

public:
	static Int getMoundHeight(void) {return m_moundHeight;};
	static void setMoundHeight(Int height);
	static Int getWidth(void) {return m_brushWidth;};  ///<Returns width.
	static Int getFeather(void) {return m_brushFeather;}; ///<Returns feather.
	static void setWidth(Int width);
	static void setFeather(Int feather);

public:
	virtual void mouseDown(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual void mouseUp(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual void mouseMoved(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual WorldHeightMapEdit *getHeightMap(void) {return m_htMapEditCopy;};
	virtual void activate(); ///< Become the current tool.
};

/*************************************************************************
**                             DigTool
***************************************************************************/
class DigTool : public MoundTool
{
public:
	DigTool(void);

};
#endif //MoundTool_H
