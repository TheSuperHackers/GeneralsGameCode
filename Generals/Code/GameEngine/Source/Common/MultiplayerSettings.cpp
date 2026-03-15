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

// FILE: MultiplayerSettings.cpp ///////////////////////////////////////////////////////////////////////////
// The MultiplayerSettings object
// Author: Matthew D. Campbell, January 2002
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#define DEFINE_TERRAIN_LOD_NAMES
#define DEFINE_TIME_OF_DAY_NAMES

#include "Common/MultiplayerSettings.h"
#include "Common/INI.h"
#include "GameNetwork/GameInfo.h" // for PLAYERTEMPLATE_*

// PUBLIC DATA ////////////////////////////////////////////////////////////////////////////////////
MultiplayerSettings *TheMultiplayerSettings = nullptr;				///< The MultiplayerSettings singleton

///////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
const FieldParse MultiplayerColorDefinition::m_colorFieldParseTable[] =
{

	{ "TooltipName",	INI::parseAsciiString,	nullptr,	offsetof( MultiplayerColorDefinition, m_tooltipName ) },
	{ "RGBColor",			INI::parseRGBColor,			nullptr,	offsetof( MultiplayerColorDefinition, m_rgbValue ) },
	{ "RGBNightColor",			INI::parseRGBColor,		nullptr,	offsetof( MultiplayerColorDefinition, m_rgbValueNight ) },
	{ nullptr,					nullptr,						nullptr,						0 }

};

const FieldParse MultiplayerSettings::m_multiplayerSettingsFieldParseTable[] =
{

	{ "InitialCreditsMin",				INI::parseInt,	nullptr,	offsetof( MultiplayerSettings, m_initialCreditsMin ) },
	{ "InitialCreditsMax",				INI::parseInt,	nullptr,	offsetof( MultiplayerSettings, m_initialCreditsMax ) },
	{ "StartCountdownTimer",			INI::parseInt,	nullptr,	offsetof( MultiplayerSettings, m_startCountdownTimerSeconds ) },
	{ "MaxBeaconsPerPlayer",			INI::parseInt,	nullptr,	offsetof( MultiplayerSettings, m_maxBeaconsPerPlayer ) },
	{ "UseShroud",								INI::parseBool,	nullptr,	offsetof( MultiplayerSettings, m_isShroudInMultiplayer ) },
	{ "ShowRandomPlayerTemplate",	INI::parseBool,	nullptr,	offsetof( MultiplayerSettings, m_showRandomPlayerTemplate ) },
	{ "ShowRandomStartPos",				INI::parseBool,	nullptr,	offsetof( MultiplayerSettings, m_showRandomStartPos ) },
	{ "ShowRandomColor",					INI::parseBool,	nullptr,	offsetof( MultiplayerSettings, m_showRandomColor ) },

	{ nullptr,					nullptr,						nullptr,						0 }

};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
MultiplayerSettings::MultiplayerSettings()
{
	m_initialCreditsMin = 5000;
	m_initialCreditsMax = 10000;
	m_maxBeaconsPerPlayer = 3;
	m_startCountdownTimerSeconds = 0;
	m_numColors = 0;
	m_isShroudInMultiplayer = TRUE;
	m_showRandomPlayerTemplate = TRUE;
	m_showRandomStartPos = TRUE;
	m_showRandomColor = TRUE;
	m_observerColor;
	m_randomColor;
}

MultiplayerColorDefinition::MultiplayerColorDefinition()
{
	m_tooltipName.clear();
	m_rgbValue.setFromInt(0xFFFFFFFF);
	m_rgbValueNight=m_rgbValue;
	m_color = 0xFFFFFFFF;
	m_colorNight = m_color;
}

MultiplayerColorDefinition * MultiplayerSettings::getColor(Int which)
{
	if (which == PLAYERTEMPLATE_RANDOM)
	{
		return &m_randomColor;
	}
	else if (which == PLAYERTEMPLATE_OBSERVER)
	{
		return &m_observerColor;
	}
	else if (which < 0 || which >= getNumColors())
	{
		return nullptr;
	}

	return &m_colorList[which];
}

MultiplayerColorDefinition * MultiplayerSettings::findMultiplayerColorDefinitionByName(AsciiString name)
{
	MultiplayerColorIter iter = m_colorList.begin();

	while (iter != m_colorList.end())
	{
		if (iter->second.getTooltipName() == name)
			return &(iter->second);

		++iter;
	}

	return nullptr;
}

MultiplayerColorDefinition * MultiplayerSettings::newMultiplayerColorDefinition(AsciiString name)
{
 	MultiplayerColorDefinition tmp;
	Int numColors = getNumColors();

	m_colorList[numColors] = tmp;
	m_numColors = m_colorList.size();

	return &m_colorList[numColors];
}

MultiplayerColorDefinition * MultiplayerColorDefinition::operator =(const MultiplayerColorDefinition& other)
{
	m_tooltipName = other.getTooltipName();
	m_rgbValue = other.getRGBValue();
	m_color = other.getColor();
	m_rgbValueNight = other.getRGBNightValue();
	m_colorNight = other.getNightColor();

	return this;
}

void MultiplayerColorDefinition::setColor( RGBColor rgb )
{
	m_color = rgb.getAsInt() | 0xFF << 24;
}

void MultiplayerColorDefinition::setNightColor( RGBColor rgb )
{
	m_colorNight = rgb.getAsInt() | 0xFF << 24;
}
