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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// DrawGroupInfo.cpp //////////////////////////////////////////////////////////////////////////////
// Author: John McDonald, October 2002
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "GameClient/DrawGroupInfo.h"

// Useful defaults.

DrawGroupInfo::DrawGroupInfo()
{
	m_fontName = "Arial";
	m_fontSize = 10;
	m_fontIsBold = FALSE;

	m_usePlayerColor = TRUE;
	m_colorForText = GameMakeColor(255, 255, 255, 255);
	m_colorForTextDropShadow = GameMakeColor(0, 0, 0, 255);

	m_dropShadowOffsetX = -1;
	m_dropShadowOffsetY = -1;

	m_percentOffsetX = -0.05f;
	m_usingPixelOffsetX = FALSE;

	m_pixelOffsetY = -10;
	m_usingPixelOffsetY = TRUE;
}

DrawGroupInfo *TheDrawGroupInfo = nullptr;
