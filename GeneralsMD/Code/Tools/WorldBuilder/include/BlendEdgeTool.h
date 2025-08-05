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

// BlendEdgeTool.h
// Texture tiling tools for worldbuilder.
// Author: John Ahlquist, April 2001

#pragma once

#ifndef BLEND_EDGE_TOOL_H
#define BLEND_EDGE_TOOL_H

#include "Tool.h"
class WorldHeightMapEdit;
/*************************************************************************/
/**                             BlendEdgeTool
	 Does the BlendEdgesOut tool operation.
***************************************************************************/
///  Blend edges out tool.
class BlendEdgeTool : public Tool
{
protected:
	Coord3D m_downPt;

public:
	BlendEdgeTool(void);
	~BlendEdgeTool(void);

public:
	/// Perform tool on mouse down.
	virtual void mouseDown(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);
	virtual void mouseUp(TTrackingMode m, CPoint viewPt, WbView* pView, CWorldBuilderDoc *pDoc);

};


#endif //TOOL_H
