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

// FILE: INIDrawGroupInfo.cpp /////////////////////////////////////////////////////////////////////
// Author: John McDonald, October 2002
// Desc:   Parsing DrawGroupInfo INI entries
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/INI.h"
#include "GameClient/DrawGroupInfo.h"

void parseInt( INI* ini, void * /*instance*/, void *store, const void* userData )
{
	DrawGroupInfo *dgi = (DrawGroupInfo*) store;
	if (userData == 0) {
		store = &dgi->m_pixelOffsetX;
		dgi->m_usingPixelOffsetX = TRUE;
	} else {
		store = &dgi->m_pixelOffsetY;
		dgi->m_usingPixelOffsetY = TRUE;
	}

	INI::parseInt(ini, NULL, store, NULL);
}

void parsePercentToReal( INI* ini, void * /*instance*/, void *store, const void* userData )
{
	DrawGroupInfo *dgi = (DrawGroupInfo*) store;
	if (userData == 0) {
		store = &dgi->m_pixelOffsetX;
		dgi->m_usingPixelOffsetX = FALSE;
	} else {
		store = &dgi->m_pixelOffsetY;
		dgi->m_usingPixelOffsetY = FALSE;
	}

	INI::parsePercentToReal(ini, NULL, store, NULL);
}

const FieldParse DrawGroupInfo::s_fieldParseTable[] =
{
	{ "UsePlayerColor",												INI::parseBool,						NULL, offsetof( DrawGroupInfo, m_usePlayerColor) },
	{ "ColorForText",													INI::parseColorInt,				NULL, offsetof( DrawGroupInfo, m_colorForText ) },
	{ "ColorForTextDropShadow",								INI::parseColorInt,				NULL, offsetof( DrawGroupInfo, m_colorForTextDropShadow ) },

	{ "FontName",															INI::parseQuotedAsciiString,		NULL, offsetof( DrawGroupInfo, m_fontName ) },
	{ "FontSize",															INI::parseInt,						NULL, offsetof( DrawGroupInfo, m_fontSize ) },
	{ "FontIsBold",														INI::parseBool,						NULL, offsetof( DrawGroupInfo, m_fontIsBold ) },
	{ "DropShadowOffsetX",										INI::parseInt,						NULL, offsetof( DrawGroupInfo, m_dropShadowOffsetX) },
	{ "DropShadowOffsetY",										INI::parseInt,						NULL, offsetof( DrawGroupInfo, m_dropShadowOffsetY) },
	{ "DrawPositionXPixel",			  						parseInt,									(void*)0, 0 },
	{ "DrawPositionXPercent",			  					parsePercentToReal,				(void*)0, 0 },
	{ "DrawPositionYPixel",			  						parseInt,									(void*)1, 0 },
	{ "DrawPositionYPercent",			  					parsePercentToReal,				(void*)1, 0 },

	{ 0, 0, 0, 0 }
};

/*static */ void INI::parseDrawGroupNumberDefinition(INI* ini)
{
	if (!TheDrawGroupInfo) {
		throw INI_UNKNOWN_ERROR;
	}

	ini->initFromINI(TheDrawGroupInfo, TheDrawGroupInfo->getFieldParse());
}

