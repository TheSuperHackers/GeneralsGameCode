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

// FILE: ToppleUpdate.h /////////////////////////////////////////////////////////////////////////////
// Author:
// Desc:
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __TOPPLEUPDATE_H_
#define __TOPPLEUPDATE_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/BehaviorModule.h"
#include "GameLogic/Module/CollideModule.h"
#include "GameLogic/Module/UpdateModule.h"

//-------------------------------------------------------------------------------------------------
class FXList;

// ------------------------------------------------------------------------------------------------
/** Topple options */
// ------------------------------------------------------------------------------------------------
enum
{
	TOPPLE_OPTIONS_NONE			 = 0x00000000,
	TOPPLE_OPTIONS_NO_BOUNCE = 0x00000001,  ///< do not bounce when hit the ground
	TOPPLE_OPTIONS_NO_FX		 = 0x00000002,	///< do not play any FX when hit the ground
};

//-------------------------------------------------------------------------------------------------
/** Topple update */
//-------------------------------------------------------------------------------------------------
class ToppleUpdateModuleData : public UpdateModuleData
{
public:
	const FXList* m_toppleFX;
	const FXList* m_bounceFX;
	AsciiString m_stumpName;
	Real m_initialVelocityPercent;
	Real m_initialAccelPercent;
	Real m_bounceVelocityPercent;
	Bool m_killWhenToppled;
	Bool m_killWhenStartToppled;
	Bool m_killStumpWhenToppled;
	Bool m_toppleLeftOrRightOnly;	// constrained to topple to my left or right only
	Bool m_reorientToppledRubble;

	ToppleUpdateModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);

};

//-------------------------------------------------------------------------------------------------
class ToppleUpdate : public UpdateModule,
										 public CollideModuleInterface
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( ToppleUpdate, "ToppleUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( ToppleUpdate, ToppleUpdateModuleData )

public:

	ToppleUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	static Int getInterfaceMask() { return UpdateModule::getInterfaceMask() | (MODULEINTERFACE_COLLIDE); }

	// BehaviorModule
	virtual CollideModuleInterface* getCollide() { return this; }

	void applyTopplingForce( const Coord3D* toppleDirection, Real toppleSpeed, UnsignedInt options);
	Bool isAbleToBeToppled() const;

	// UpdateModuleInterface
	virtual UpdateSleepTime update();

	// CollideModuleInterface
	virtual void onCollide( Object *other, const Coord3D *loc, const Coord3D *normal );
	/// this is used for things like pilots, to determine if they can "enter" something
	virtual Bool wouldLikeToCollideWith(const Object* other) const { return false; }
	virtual Bool isHijackedVehicleCrateCollide() const { return false; }
	virtual Bool isCarBombCrateCollide() const { return false; }
	virtual Bool isRailroad() const { return false; }
	virtual Bool isSalvageCrateCollide() const { return false; }
	virtual Bool isSabotageBuildingCrateCollide() const { return FALSE; }

protected:

	enum ToppleState
	{
		TOPPLE_UPRIGHT = 0,
		TOPPLE_FALLING,
		TOPPLE_DOWN
	};

	Real					m_angularVelocity;				///< Velocity in degrees per frame (or is it radians per frame?)
	Real					m_angularAcceleration;		///< Acceleration angularVelocity is increasing
	Coord3D				m_toppleDirection;				///< Z-less direction we are toppling
	ToppleState		m_toppleState;						///< Stage this module is in.
	Real					m_angularAccumulation;		///< How much have I rotated so I know when to bounce.
	Real					m_angleDeltaX;						///< how much to modify x angle each frame
	Int						m_numAngleDeltaX;					///< how many frames to tweak x angle
	Bool					m_doBounceFX;							///< do the bounce FX if we do bounce
	UnsignedInt		m_options;								///< topple options
	ObjectID			m_stumpID;								///< stump generated, if any

};

#endif // end __TOPPLEUPDATE_H_

