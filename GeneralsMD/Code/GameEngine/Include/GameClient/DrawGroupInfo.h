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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: DrawGroupInfo.h //////////////////////////////////////////////////////////////////////////
// AudioEventRTS structure
// Author: John K. McDonald, March 2002

#pragma once

struct DrawGroupInfo
{
	AsciiString m_fontName;
	Int m_fontSize;
	Bool m_fontIsBold;

	Bool m_usePlayerColor;
	Color m_colorForText;
	Color m_colorForTextDropShadow;

	Int m_dropShadowOffsetX;
	Int m_dropShadowOffsetY;

	union
	{
		Int m_pixelOffsetX;
		Real m_percentOffsetX;
	};
	Bool m_usingPixelOffsetX;

	union
	{
		Int m_pixelOffsetY;
		Real m_percentOffsetY;
	};
	Bool m_usingPixelOffsetY;

	DrawGroupInfo();

	static const FieldParse s_fieldParseTable[];		///< the parse table for INI definition
	const FieldParse *getFieldParse( void ) const { return s_fieldParseTable; }
};

extern DrawGroupInfo *TheDrawGroupInfo;
