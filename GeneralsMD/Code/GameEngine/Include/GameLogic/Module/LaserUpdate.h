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

	AsciiString m_targetParticleSystemName;  ///< Used for the target effect while laser active.

	Real m_punchThroughScalar;	///< If non-zero, length modifier when we used to have a target object and now don't

	UnsignedInt m_fadeInDurationFrames;  ///< If non-zero, beam fades in over duration
	UnsignedInt m_fadeOutDurationFrames;  ///< If non-zero, beam fades out over duration (tries to get time from lifetimeUpdate)
	UnsignedInt m_widenDurationFrames;  ///< If non-zero, beam grows to max size over duration
	UnsignedInt m_decayDurationFrames;  ///< If non-zero, beam shrinks over duration (tries to get time from lifetimeUpdate)

	Bool m_hasMultiDraw;  ///< Enable this to support tracking multiple LaserDraw modules
	Bool m_useHouseColor;  ///< Enable this to color particles with house color

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

	void initRadius(Int sizeDeltaFrames);
	bool updateRadius();
	void setDecayFrames(UnsignedInt decayFrames);
	void xfer(Xfer* xfer);
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
/** The default	update module */
//-------------------------------------------------------------------------------------------------
class LaserUpdate : public ClientUpdateModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( LaserUpdate, "LaserUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( LaserUpdate, LaserUpdateModuleData );

public:

	LaserUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	//Actually puts the laser in the world.
	void initLaser( const Object *parent, const Object *target, const Coord3D *startPos, const Coord3D *endPos, AsciiString parentBoneName, Int sizeDeltaFrames = 0 );
	
	// TODO: clean this Xhit
	const LaserRadiusUpdate& getLaserRadiusUpdate() const { return m_laserRadius; }

	void setDecayFrames( UnsignedInt decayFrames );

	const Coord3D* getStartPos() { return &m_startPos; }
	const Coord3D* getEndPos() { return &m_endPos; }

	Real getTemplateLaserRadius() const;
	Real getCurrentLaserRadius() const;

	void setDirty( Bool dirty ) { m_dirty = dirty; }
	Bool isDirty() { return m_dirty || getLaserUpdateModuleData()->m_hasMultiDraw; }

	Real getWidthScale() const { return m_currentWidthScalar; }
	Real getAlphaScale() const { return m_currentAlphaScalar; }

	Real getLifeTimeProgress() const;

	Int getPlayerColor() const { return m_hexColor; };

	void updateContinuousLaser(const Object* parent, const Object* target, const Coord3D* startPos, const Coord3D* endPos);

	virtual void clientUpdate();

protected:

	void updateStartPos(); ///< figures out and sets startPos
	void updateEndPos(); ///< figures out and sets endPos

	//If the master dies, so will this laser (although if it has a fade delay, it'll just skip to the fade)
	Coord3D m_startPos;
	Coord3D m_endPos;

	DrawableID m_parentID;
	DrawableID m_targetID;

	UnsignedInt m_startFrame;  ///< the frame this laser is initialized
	UnsignedInt m_dieFrame;   ///< the frame this laser is scheduled to die

	Bool m_dirty;
	ParticleSystemID m_particleSystemID;
	ParticleSystemID m_targetParticleSystemID;
	Bool m_widening;
	Bool m_decaying;
	UnsignedInt m_widenStartFrame;
	UnsignedInt m_widenFinishFrame;
	Real m_currentWidthScalar;
	UnsignedInt m_decayStartFrame;
	UnsignedInt m_decayFinishFrame;

	Bool m_fadingIn;
	Bool m_fadingOut;
	UnsignedInt m_fadeInStartFrame;
	UnsignedInt m_fadeInFinishFrame;
	Real m_currentAlphaScalar;
	UnsignedInt m_fadeOutStartFrame;
	UnsignedInt m_fadeOutFinishFrame;

	AsciiString m_parentBoneName;

	Int m_hexColor;

	// Bool m_isMultiDraw;

	LaserRadiusUpdate m_laserRadius;
};
