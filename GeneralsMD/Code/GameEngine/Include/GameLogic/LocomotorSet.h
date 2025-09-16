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

// FILE: LocomotorSet.h /////////////////////////////////////////////////////////////////////////////////
// Author: Steven Johnson, Feb 2002
// Desc: Locomotor Descriptions
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __LocomotorSet_H_
#define __LocomotorSet_H_

// no, please do NOT include this.
//#include "GameLogic/Locomotor.h"
#include "Common/GameCommon.h"
#include "Common/STLTypedefs.h"
#include "Common/Snapshot.h"

class Locomotor;
class LocomotorTemplate;

//-------------------------------------------------------------------------------------------------
//
// Note: these values are saved in save files, so you MUST NOT REMOVE OR CHANGE
// existing values!
//
enum LocomotorSurfaceType CPP_11(: Int)
{
	LOCOMOTORSURFACE_GROUND			= (1 << 0),									///< clear, unobstructed ground
	LOCOMOTORSURFACE_WATER			= (1 << 1),									///< water area
	LOCOMOTORSURFACE_CLIFF			= (1 << 2),									///< steep altitude change
	LOCOMOTORSURFACE_AIR				= (1 << 3),									///< airborne
	LOCOMOTORSURFACE_RUBBLE			= (1 << 4)									///< building rubble
};

typedef Int LocomotorSurfaceTypeMask;

const LocomotorSurfaceTypeMask NO_SURFACES = (LocomotorSurfaceTypeMask)0x0000;
const LocomotorSurfaceTypeMask ALL_SURFACES = (LocomotorSurfaceTypeMask)0xffff;

#ifdef DEFINE_SURFACECATEGORY_NAMES
static const char *const TheLocomotorSurfaceTypeNames[] =
{
	"GROUND",
	"WATER",
	"CLIFF",
	"AIR",
	"RUBBLE",

	NULL
};
#endif

//-------------------------------------------------------------------------------------------------
typedef std::vector< Locomotor* > LocomotorVector;

//-------------------------------------------------------------------------------------------------
class LocomotorSet : public Snapshot
{
private:
	LocomotorVector						m_locomotors;
	LocomotorSurfaceTypeMask	m_validLocomotorSurfaces;
	Bool											m_downhillOnly;

	LocomotorSet(const LocomotorSet& that);
	LocomotorSet& operator=(const LocomotorSet& that);

protected:
	// snapshot methods
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void );

public:

	LocomotorSet();
	~LocomotorSet();

	void clear();

	void addLocomotor(const LocomotorTemplate* lt);

	Locomotor* findLocomotor(LocomotorSurfaceTypeMask t);

	void xferSelfAndCurLocoPtr(Xfer *xfer, Locomotor** loco);

	inline LocomotorSurfaceTypeMask getValidSurfaces() const { return m_validLocomotorSurfaces; }
	inline Bool isDownhillOnly( void ) const { return m_downhillOnly; };

};

#endif
