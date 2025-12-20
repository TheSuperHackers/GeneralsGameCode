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

// FloodFillTool.h
// Texture tiling tools for worldbuilder.
// Author: John Ahlquist, April 2001

#pragma once

#include "Tool.h"
class WorldHeightMapEdit;
/**************************************************************************
                            FloodFillTool
***************************************************************************/
///  Fill area with texture tool.
class FloodFillTool : public Tool
{
public:
	FloodFillTool(void);
	~FloodFillTool(void);

protected:
	Int			m_textureClassToDraw; ///< The texture to fill with.  Foreground for mousedDown, background for mouseDownRt.
	HCURSOR m_cliffCursor;
	static Bool m_adjustCliffTextures;

public:
	virtual void mouseUp(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual void activate(); ///< Become the current tool.
	virtual void setCursor(void);

	Bool getAdjustCliffs(void) {return m_adjustCliffTextures;}
	void setAdjustCliffs(Bool val) {m_adjustCliffTextures = val;}

};
