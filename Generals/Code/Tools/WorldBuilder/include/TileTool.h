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

// TileTool.h
// Texture tiling tools for worldbuilder.
// Author: John Ahlquist, April 2001

#pragma once

#ifndef TILETOOL_H
#define TILETOOL_H

#include "Tool.h"
class WorldHeightMapEdit;
/*************************************************************************
**                             TileTool
***************************************************************************/
class TileTool : public Tool
{
protected:
	WorldHeightMapEdit *m_htMapEditCopy; //< ref counted.
	Int									m_textureClassToDraw;
	CPoint							m_prevViewPt;

public:
	TileTool(void);
	~TileTool(void);

public:
	virtual void mouseDown(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual void mouseUp(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual void mouseMoved(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual WorldHeightMapEdit *getHeightMap(void) {return m_htMapEditCopy;};
	virtual void activate(); ///< Become the current tool.
	virtual Int getWidth(void) {return 1;};
};

/*************************************************************************
**                             BigTileTool
***************************************************************************/
class BigTileTool : public TileTool
{

protected:
	static Int m_currentWidth;

public:
 	virtual void activate(); ///< Become the current tool.

public:
	BigTileTool(void);

	static void setWidth(Int width) ;
	virtual Int getWidth(void) {return m_currentWidth;};

};
#endif //TOOL_H
