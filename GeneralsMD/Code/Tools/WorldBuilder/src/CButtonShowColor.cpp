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

#include "StdAfx.h"

#include "GameClient/ParticleSys.h"
#include "CButtonShowColor.h"

// CButtonShowColor ///////////////////////////////////////////////////////////////////////////////
void CButtonShowColor::OnPaint() 
{
	try {
		CPaintDC paintDC(this);
	
		CPen pen(PS_SOLID, 1, 0xFFFFFF00);
		CBrush brush(RGBtoBGR(m_color.getAsInt()));
		// Select my stuff
		CPen *oldPen = paintDC.SelectObject(&pen);

		CRect rect;
		GetWindowRect(&rect);
		ScreenToClient(&rect);

		paintDC.FillRect(&rect, &brush);
		paintDC.MoveTo(rect.left, rect.top);
		paintDC.LineTo(rect.right, rect.top);
		paintDC.LineTo(rect.right, rect.bottom);
		paintDC.LineTo(rect.left, rect.bottom);
		paintDC.LineTo(rect.left, rect.top);

		// Restore the states.
		paintDC.SelectObject(oldPen);
 
	} catch (...) {
		// Unlikely, but possible.
		return;
	}
}

CButtonShowColor::~CButtonShowColor()
{
	DestroyWindow();
}

// Convert from 0x00RRGGBB to 0x00BBGGRR
COLORREF CButtonShowColor::RGBtoBGR(Int color)
{
	return ((color & 0x00FF0000) >> 16 |
					(color & 0x0000FF00) <<  0 |
					(color & 0x000000FF) << 16);
}

// Convert from 0x00BBGGRR to 0x00RRGGBB
Int CButtonShowColor::BGRtoRGB(COLORREF color)
{
	return RGBtoBGR(color);
}

BEGIN_MESSAGE_MAP(CButtonShowColor, CButton)
	ON_WM_PAINT()
END_MESSAGE_MAP()
