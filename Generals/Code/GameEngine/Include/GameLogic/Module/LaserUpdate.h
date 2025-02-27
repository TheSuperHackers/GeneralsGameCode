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

// FILE: LaserUpdate.h //////////////////////////////////////////////////////////////////////////
// Author: Kris Morness, July 2002
// Desc:   Handles laser update processing for render purposes and game control.
// Modifications: Kris Morness - Oct 2002 -- moved to Client update (will rename later)
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/ClientUpdateModule.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Thing;
class Vector3;
enum ParticleSystemID CPP_11(: Int);

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class LaserUpdateModuleData : public ClientUpdateModuleData
{
public:
	AsciiString m_particleSystemName;  ///< Used for the muzzle flare while laser active.
	AsciiString m_parentFireBoneName;  ///< Used to fire laser at specified parent bone position.
	Bool m_parentFireBoneOnTurret;			///< And used to specifiy where to look for the bone.

	AsciiString m_targetParticleSystemName;  ///< Used for the target effect while laser active.

	LaserUpdateModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);

private:

};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class LaserRadiusUpdate
{
public:
	LaserRadiusUpdate();

	void initRadius( Int sizeDeltaFrames );
	bool updateRadius();
	void setDecayFrames( UnsignedInt decayFrames );
	void xfer( Xfer *xfer );
	Real getWidthScale() const { return m_currentWidthScalar; }

private:
	Bool m_widening;
	Bool m_decaying;
	UnsignedInt m_widenStartFrame;
	UnsignedInt m_widenFinishFrame;
	Real m_currentWidthScalar;
	UnsignedInt m_decayStartFrame;
	UnsignedInt m_decayFinishFrame;
};

//-------------------------------------------------------------------------------------------------
/** The laser update module */
//-------------------------------------------------------------------------------------------------
class LaserUpdate : public ClientUpdateModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( LaserUpdate, "LaserUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( LaserUpdate, LaserUpdateModuleData );

public:

	LaserUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	//Actually puts the laser in the world.
	void initLaser( const Object *parent, const Coord3D *startPos, const Coord3D *endPos, Int sizeDeltaFrames = 0 );

	const LaserRadiusUpdate& getLaserRadiusUpdate() const { return m_laserRadius; }
	void setDecayFrames( UnsignedInt decayFrames ) { m_laserRadius.setDecayFrames(decayFrames); }
	Real getWidthScale() const { return m_laserRadius.getWidthScale(); }

	const Coord3D* getStartPos() const { return &m_startPos; }
	const Coord3D* getEndPos() const { return &m_endPos; }

	Real getTemplateLaserRadius() const;
	Real getCurrentLaserRadius() const;

	void setDirty( Bool dirty ) { m_dirty = dirty; }
	Bool isDirty() const { return m_dirty; }

	virtual void clientUpdate();

protected:

	//If the master dies, so will this laser (although if it has a fade delay, it'll just skip to the fade)
	Coord3D m_startPos;
	Coord3D m_endPos;
	Bool m_dirty;
	ParticleSystemID m_particleSystemID;
	ParticleSystemID m_targetParticleSystemID;

	LaserRadiusUpdate m_laserRadius;
};
