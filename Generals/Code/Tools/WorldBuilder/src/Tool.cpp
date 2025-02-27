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

// Tool.cpp
// Texture tiling tool for worldbuilder.
// Author: John Ahlquist, April 2001

#include "StdAfx.h"
#include "resource.h"
#include "MainFrm.h"
#include "DrawObject.h"
#include "WorldBuilderDoc.h"
#include "Common/MapObject.h"

#include "Tool.h"
//
/// Tool class.
//
/// Constructor
Tool::Tool(Int toolID, Int cursorID)
{
	m_toolID = toolID;
	m_cursorID = cursorID;
	m_cursor = NULL;
}


/// Destructor
Tool::~Tool(void)
{
	if (m_cursor) {
		::DestroyCursor(m_cursor);
	}
}


/// Shows the "no options"  options panel.
void Tool::activate()
{
	CMainFrame::GetMainFrame()->showOptionsDialog(IDD_NO_OPTIONS);
	DrawObject::setDoBrushFeedback(false);
}


void Tool::setCursor(void)
{
		if (m_cursor == NULL) {
			m_cursor = AfxGetApp()->LoadCursor(MAKEINTRESOURCE(m_cursorID));
		}
		::SetCursor(m_cursor);
}

/// Calculate the round blend factor.
/** Calculates the blend amount of the brush.  1.0 means the brush sets the
height, 0.0 means no change, and between blends proportionally. */
Real Tool::calcRoundBlendFactor(CPoint center, Int x, Int y, Int brushWidth, Int featherWidth)
{
	Real offset = 0;
	if (brushWidth&1) offset = 0.5;
	const Real CLOSE_ENOUGH = 0.05f;	 // We are working on an integer grid.

	Real dx = abs(center.x+offset-x);
	Real dy = abs(center.y+offset-y);

	Real dist = (Real)sqrt(dx*dx+dy*dy);

	if (dist <= (brushWidth/2.0f)+CLOSE_ENOUGH) return (1.0f);

	dist -= (brushWidth/2.0f);

	if (featherWidth < 1) {
		return(0);
	}

	if (dist <= featherWidth) {
		return (featherWidth-dist)/featherWidth;
	}

	return(0);
}

/// Calculate the square blend factor.
/** Calculates the blend amount of the brush.  1.0 means the brush sets the
height, 0.0 means no change, and between blends proportionally. */
Real Tool::calcSquareBlendFactor(CPoint center, Int x, Int y, Int brushWidth, Int featherWidth)
{
	Real offset = 0;
	if (brushWidth&1) offset = 0.5;

	Real dx = abs(center.x+offset-x);
	Real dy = abs(center.y+offset-y);

	Real dist = dx;
	if (dy>dist) dist = dy;

	if (dist <= (brushWidth/2.0f)) return (1.0f);

	dist -= (brushWidth/2.0f);

	if (featherWidth < 1) {
		return(0);
	}

	if (dist <= featherWidth) {
		return (featherWidth-dist)/featherWidth;
	}

	return(0);
}

/// Gets the cell index for the center of the brush.
/** Converts from document coordinates to cell index coordinates. */
void Tool::getCenterIndex(Coord3D *docLocP, Int brushWidth, CPoint *center, CWorldBuilderDoc *pDoc)
{
	Coord3D cpt = *docLocP;
	// center on half pixel for even widths.
	if (!(brushWidth&1)) {
		cpt.x += MAP_XY_FACTOR/2;
		cpt.y += MAP_XY_FACTOR/2;
	}
	if (!pDoc->getCellIndexFromCoord(cpt, center)) {
		return;
	}
}

void Tool::getAllIndexesIn(const Coord3D *bl, const Coord3D *br,
													 const Coord3D *tl, const Coord3D *tr,
													 Int widthOutside, CWorldBuilderDoc *pDoc,
													 VecHeightMapIndexes* allIndices)
{
	if (!(bl && br && tl && tr && pDoc && allIndices)) {
		return;
	}

	pDoc->getAllIndexesInRect(bl, br, tl, tr, widthOutside, allIndices);
}
