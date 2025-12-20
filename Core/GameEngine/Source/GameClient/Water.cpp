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

// FILE: Water.cpp ////////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, December 2001
// Desc:   Map water settings
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "GameClient/Water.h"
#include "Common/INI.h"

// GLOBALS ////////////////////////////////////////////////////////////////////////////////////////
WaterSetting WaterSettings[ TIME_OF_DAY_COUNT ];
OVERRIDE<WaterTransparencySetting> TheWaterTransparency = NULL;

// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////////
const FieldParse WaterSetting::m_waterSettingFieldParseTable[] =
{

	{ "SkyTexture",									INI::parseAsciiString,			NULL, offsetof( WaterSetting, m_skyTextureFile ) },
	{ "WaterTexture",								INI::parseAsciiString,			NULL, offsetof( WaterSetting, m_waterTextureFile ) },
  { "Vertex00Color",							INI::parseRGBAColorInt,			NULL, offsetof( WaterSetting, m_vertex00Diffuse ) },
	{ "Vertex10Color",							INI::parseRGBAColorInt,			NULL, offsetof( WaterSetting, m_vertex10Diffuse ) },
	{ "Vertex01Color",							INI::parseRGBAColorInt,			NULL, offsetof( WaterSetting, m_vertex01Diffuse ) },
  { "Vertex11Color",							INI::parseRGBAColorInt,			NULL, offsetof( WaterSetting, m_vertex11Diffuse ) },
	{ "DiffuseColor",								INI::parseRGBAColorInt,			NULL, offsetof( WaterSetting, m_waterDiffuseColor ) },
  { "TransparentDiffuseColor",		INI::parseRGBAColorInt,			NULL, offsetof( WaterSetting, m_transparentWaterDiffuse ) },
  { "UScrollPerMS",								INI::parseReal,							NULL, offsetof( WaterSetting, m_uScrollPerMs ) },
  { "VScrollPerMS",								INI::parseReal,							NULL, offsetof( WaterSetting, m_vScrollPerMs ) },
	{ "SkyTexelsPerUnit",						INI::parseReal,							NULL, offsetof( WaterSetting, m_skyTexelsPerUnit ) },
  { "WaterRepeatCount",						INI::parseInt,							NULL, offsetof( WaterSetting, m_waterRepeatCount ) },

	{ NULL,													NULL,												NULL, 0 },

};

// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////////
const FieldParse WaterTransparencySetting::m_waterTransparencySettingFieldParseTable[] =
{

	{ "TransparentWaterDepth",			INI::parseReal,				NULL,			offsetof( WaterTransparencySetting, m_transparentWaterDepth ) },
	{ "TransparentWaterMinOpacity",	INI::parseReal,				NULL,			offsetof( WaterTransparencySetting, m_minWaterOpacity ) },
	{ "StandingWaterColor",	INI::parseRGBColor,			NULL,			offsetof( WaterTransparencySetting, m_standingWaterColor ) },
	{ "StandingWaterTexture",INI::parseAsciiString,		NULL,			offsetof( WaterTransparencySetting, m_standingWaterTexture ) },
	{ "AdditiveBlending", INI::parseBool,				NULL,			offsetof( WaterTransparencySetting, m_additiveBlend) },
	{ "RadarWaterColor", INI::parseRGBColor,			NULL,			offsetof( WaterTransparencySetting, m_radarColor) },
	{ "SkyboxTextureN",							INI::parseAsciiString,NULL,			offsetof( WaterTransparencySetting, m_skyboxTextureN ) },
	{ "SkyboxTextureE",							INI::parseAsciiString,NULL,			offsetof( WaterTransparencySetting, m_skyboxTextureE ) },
	{ "SkyboxTextureS",							INI::parseAsciiString,NULL,			offsetof( WaterTransparencySetting, m_skyboxTextureS ) },
	{ "SkyboxTextureW",							INI::parseAsciiString,NULL,			offsetof( WaterTransparencySetting, m_skyboxTextureW ) },
	{ "SkyboxTextureT",							INI::parseAsciiString,NULL,			offsetof( WaterTransparencySetting, m_skyboxTextureT ) },


	{ 0, 0, 0, 0 },
};


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
WaterSetting::WaterSetting( void )
{

	m_skyTextureFile.clear();
	m_waterTextureFile.clear();
	m_waterRepeatCount = 0;
	m_skyTexelsPerUnit = 0.0f;

	m_vertex00Diffuse.red = 0;
	m_vertex00Diffuse.green = 0;
	m_vertex00Diffuse.blue = 0;
	m_vertex00Diffuse.alpha = 0;

	m_vertex01Diffuse.red = 0;
	m_vertex01Diffuse.green = 0;
	m_vertex01Diffuse.blue = 0;
	m_vertex01Diffuse.alpha = 0;

	m_vertex10Diffuse.red = 0;
	m_vertex10Diffuse.green = 0;
	m_vertex10Diffuse.blue = 0;
	m_vertex10Diffuse.alpha = 0;

	m_vertex11Diffuse.red = 0;
	m_vertex11Diffuse.green = 0;
	m_vertex11Diffuse.blue = 0;
	m_vertex11Diffuse.alpha = 0;

	m_waterDiffuseColor.red = 0;
	m_waterDiffuseColor.green = 0;
	m_waterDiffuseColor.blue = 0;
	m_waterDiffuseColor.alpha = 0;

	m_transparentWaterDiffuse.red = 0;
	m_transparentWaterDiffuse.green = 0;
	m_transparentWaterDiffuse.blue = 0;
	m_transparentWaterDiffuse.alpha = 0;

	m_uScrollPerMs = 0.0f;
	m_vScrollPerMs = 0.0f;

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
WaterSetting::~WaterSetting( void )
{

}


