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

// RadiusDecal.cpp ///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#define DEFINE_SHADOW_NAMES

#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/Xfer.h"
#include "GameClient/RadiusDecal.h"
#include "GameClient/Shadow.h"
#include "GameLogic/GameLogic.h"


// ------------------------------------------------------------------------------------------------
RadiusDecalTemplate::RadiusDecalTemplate() :
	m_shadowType(SHADOW_ALPHA_DECAL),
	m_minOpacity(1.0f),
	m_maxOpacity(1.0f),
	m_opacityThrobTime(LOGICFRAMES_PER_SECOND),
	m_color(0),
	m_onlyVisibleToOwningPlayer(true),
	m_name(AsciiString::TheEmptyString)
{
}

// ------------------------------------------------------------------------------------------------
void RadiusDecalTemplate::createRadiusDecal(const Coord3D& pos, Real radius, const Player* owningPlayer, RadiusDecal& result) const
{
	result.clear();

	if (owningPlayer == NULL)
	{
		DEBUG_CRASH(("You MUST specify a non-NULL owningPlayer to createRadiusDecal. (srj)"));
		return;
	}

	if (m_name.isEmpty() || radius <= 0.0f)
		return;

	// it is now considered nonEmpty, regardless of the state of m_decal, etc
	result.m_empty = false;

	if (!m_onlyVisibleToOwningPlayer ||
			owningPlayer->getPlayerIndex() == ThePlayerList->getLocalPlayer()->getPlayerIndex())
	{
		Shadow::ShadowTypeInfo decalInfo;
		decalInfo.allowUpdates = FALSE;										// shadow texture will never update
		decalInfo.allowWorldAlign = TRUE;									// shadow image will wrap around world objects
		decalInfo.m_type = m_shadowType;
		strlcpy(decalInfo.m_ShadowName, m_name.str(), ARRAY_SIZE(decalInfo.m_ShadowName));		// name of your texture
		decalInfo.m_sizeX = radius*2;									// world space dimensions
		decalInfo.m_sizeY = radius*2;									// world space dimensions

		result.m_decal = TheProjectedShadowManager->addDecal(&decalInfo);
		if (result.m_decal)
		{
			result.m_decal->setAngle(0.0f);
			result.m_decal->setColor(m_color == 0 ? owningPlayer->getPlayerColor() : m_color);
			result.m_decal->setPosition(pos.x, pos.y, pos.z);
			result.m_template = this;
		}
		else
		{
			DEBUG_CRASH(("Unable to add decal %s",decalInfo.m_ShadowName));
		}
	}
}

// ------------------------------------------------------------------------------------------------
void RadiusDecalTemplate::xferRadiusDecalTemplate( Xfer *xfer )
{
  // version
  XferVersion currentVersion = 1;
  XferVersion version = currentVersion;
  xfer->xferVersion( &version, currentVersion );

	xfer->xferAsciiString(&m_name);
	xfer->xferUser(&m_shadowType, sizeof(m_shadowType));
	xfer->xferReal(&m_minOpacity);
  xfer->xferReal(&m_maxOpacity);
	xfer->xferUnsignedInt(&m_opacityThrobTime);
	xfer->xferColor(&m_color);
	xfer->xferBool(&m_onlyVisibleToOwningPlayer);
}

// ------------------------------------------------------------------------------------------------
/*static*/ void RadiusDecalTemplate::parseRadiusDecalTemplate(INI* ini, void *instance, void * store, const void* /*userData*/)
{
	static const FieldParse dataFieldParse[] =
	{
		{ "Texture",										INI::parseAsciiString,				NULL,							offsetof( RadiusDecalTemplate, m_name ) },
		{ "Style",											INI::parseBitString32,				TheShadowNames,		offsetof( RadiusDecalTemplate, m_shadowType ) },
		{ "OpacityMin",									INI::parsePercentToReal,			NULL,							offsetof( RadiusDecalTemplate, m_minOpacity ) },
		{ "OpacityMax",									INI::parsePercentToReal,			NULL,							offsetof( RadiusDecalTemplate, m_maxOpacity) },
		{ "OpacityThrobTime",						INI::parseDurationUnsignedInt,NULL,							offsetof( RadiusDecalTemplate, m_opacityThrobTime ) },
		{ "Color",											INI::parseColorInt,						NULL,							offsetof( RadiusDecalTemplate, m_color ) },
		{ "OnlyVisibleToOwningPlayer",	INI::parseBool,								NULL,							offsetof( RadiusDecalTemplate, m_onlyVisibleToOwningPlayer ) },
		{ 0, 0, 0, 0 }
	};

	ini->initFromINI(store, dataFieldParse);
}

// ------------------------------------------------------------------------------------------------
RadiusDecal::RadiusDecal() :
	m_template(NULL),
	m_decal(NULL),
	m_empty(true)
{
}

// ------------------------------------------------------------------------------------------------
RadiusDecal::RadiusDecal(const RadiusDecal& that) :
	m_template(NULL),
	m_decal(NULL),
	m_empty(true)
{
	DEBUG_CRASH(("not fully implemented"));
}

// ------------------------------------------------------------------------------------------------
RadiusDecal& RadiusDecal::operator=(const RadiusDecal& that)
{
	if (this != &that)
	{
		m_template = NULL;
		if (m_decal)
			m_decal->release();
		m_decal = NULL;
		m_empty = true;
		DEBUG_CRASH(("not fully implemented"));
	}
	return *this;
}

// ------------------------------------------------------------------------------------------------
void RadiusDecal::xferRadiusDecal( Xfer *xfer )
{
	/// @todo implement me
	if (xfer->getXferMode() == XFER_LOAD)
	{
		clear();
	}
}

// ------------------------------------------------------------------------------------------------
void RadiusDecal::clear()
{
	m_template = NULL;
	if (m_decal)
	{
		m_decal->release();
	}
	m_decal = NULL;
	m_empty = true;
}

// ------------------------------------------------------------------------------------------------
RadiusDecal::~RadiusDecal()
{
	clear();
}

// ------------------------------------------------------------------------------------------------
void RadiusDecal::update()
{
	if (m_decal && m_template)
	{
		UnsignedInt now = TheGameLogic->getFrame();
		Real theta = (2*PI) * (Real)(now % m_template->m_opacityThrobTime) / (Real)m_template->m_opacityThrobTime;
		Real percent = 0.5f * (Sin(theta) + 1.0f);
		Int opac;
		if( TheGameLogic->getDrawIconUI() )
		{
			opac = REAL_TO_INT((m_template->m_minOpacity + percent * (m_template->m_maxOpacity - m_template->m_minOpacity)) * 255.0f);
		}
		else
		{
			//Scripts turned this off, so don't show them!
			opac = 0;
		}
		m_decal->setOpacity(opac);
	}
}



void RadiusDecal::setOpacity( Real o )
{
	if (m_decal)
	{
		m_decal->setOpacity(REAL_TO_INT(255.0f * o));
	}
}

// ------------------------------------------------------------------------------------------------
void RadiusDecal::setPosition(const Coord3D& pos)
{
	if (m_decal)
	{
		m_decal->setPosition(pos.x, pos.y, pos.z);	//world space position of center of decal
	}
}
