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

// FILE: RadiusDecal.h ///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef _RadiusDecal_H_
#define _RadiusDecal_H_

#include "Common/GameCommon.h"
#include "Common/GameType.h"
#include "GameClient/Color.h"

enum ShadowType CPP_11(: Int);
class Player;
class Shadow;
class RadiusDecalTemplate;

// ------------------------------------------------------------------------------------------------
class RadiusDecal
{
	friend class RadiusDecalTemplate;
private:
	const RadiusDecalTemplate*	m_template;
	Shadow*											m_decal;
	Bool												m_empty;
public:
	RadiusDecal();
	RadiusDecal(const RadiusDecal& that);
	RadiusDecal& operator=(const RadiusDecal& that);
	~RadiusDecal();

	void xferRadiusDecal( Xfer *xfer );

	// please note: it is very important, for game/net sync reasons, to ensure that
	// isEmpty() returns the same value, regardless of whether this decal will 
	// be visible to the local player or not.
	Bool isEmpty() const { return m_empty; }
	void clear();
	void update();
	void setPosition(const Coord3D& pos);
	void setOpacity( const Real o );
};

// ------------------------------------------------------------------------------------------------
class RadiusDecalTemplate
{
	friend class RadiusDecal;
private:
	AsciiString		m_name;
	ShadowType		m_shadowType;
	Real					m_minOpacity;
	Real					m_maxOpacity;
	UnsignedInt		m_opacityThrobTime;
	Color					m_color;
	Bool					m_onlyVisibleToOwningPlayer;

public:
	RadiusDecalTemplate();

	Bool valid() const { return m_name.isNotEmpty(); }
	void xferRadiusDecalTemplate( Xfer *xfer );

	// please note: it is very important, for game/net sync reasons, to ensure that
	// a valid radiusdecal is created, even if will not be visible to the local player,
	// since some logic makes decisions based on this.
	void createRadiusDecal(const Coord3D& pos, Real radius, const Player* owningPlayer, RadiusDecal& result) const;

	static void parseRadiusDecalTemplate(INI* ini, void *instance, void * store, const void* /*userData*/);
};

#endif
