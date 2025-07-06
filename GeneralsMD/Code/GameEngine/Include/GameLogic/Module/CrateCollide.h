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

// FILE: CrateCollide.h /////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, March 2002
// Desc:   Abstract base Class Crate Collide
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef CRATE_COLLIDE_H_
#define CRATE_COLLIDE_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/CollideModule.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Thing;
class Anim2DTemplate;
class FXList;
enum ScienceType CPP_11(: Int);

//-------------------------------------------------------------------------------------------------
class CrateCollideModuleData : public CollideModuleData 
{
public:
	KindOfMaskType	m_kindof;				///< the kind(s) of units that can be collided with
	KindOfMaskType	m_kindofnot;		///< the kind(s) of units that CANNOT be collided with
	Bool m_isForbidOwnerPlayer;			///< This crate cannot be picked up by the player of the dead thing that made it.
	Bool m_isBuildingPickup;			///< This crate can be picked up by a Building (bypassing AI requirement)
	Bool m_isHumanOnlyPickup;				///< Can this crate only be picked up by a human player?  (Mission thing)
	ScienceType m_pickupScience;		///< Can only be picked up by a unit whose player has this science
	FXList *m_executeFX;						///< FXList to play when activated
	
	AsciiString m_executionAnimationTemplate;				///< Anim2D to play at crate location
	Real m_executeAnimationDisplayTimeInSeconds;		///< time to play animation for
	Real m_executeAnimationZRisePerSecond;					///< rise animation up while playing
	Bool m_executeAnimationFades;										///< animation fades out

	CrateCollideModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);
};

//-------------------------------------------------------------------------------------------------
class CrateCollide : public CollideModule
{

	MEMORY_POOL_GLUE_ABC( CrateCollide )
	MAKE_STANDARD_MODULE_MACRO_ABC( CrateCollide )
	MAKE_STANDARD_MODULE_DATA_MACRO_ABC( CrateCollide, CrateCollideModuleData )

public:

enum SabotageVictimType CPP_11(: Int)
{
	SAB_VICTIM_GENERIC = 0,
	SAB_VICTIM_COMMAND_CENTER,
	SAB_VICTIM_FAKE_BUILDING,
	SAB_VICTIM_INTERNET_CENTER,
	SAB_VICTIM_MILITARY_FACTORY,
	SAB_VICTIM_POWER_PLANT,
	SAB_VICTIM_SUPERWEAPON,
	SAB_VICTIM_SUPPLY_CENTER,
	SAB_VICTIM_DROP_ZONE,
};

	CrateCollide( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	/// This collide method gets called when collision occur
	virtual void onCollide( Object *other, const Coord3D *loc, const Coord3D *normal );

	virtual Bool wouldLikeToCollideWith(const Object* other) const { return isValidToExecute(other); }

	virtual Bool isRailroad() const { return FALSE;};
 	virtual Bool isCarBombCrateCollide() const { return FALSE; }
	virtual Bool isHijackedVehicleCrateCollide() const { return FALSE; }
	virtual Bool isSabotageBuildingCrateCollide() const { return FALSE; }

  void doSabotageFeedbackFX( const Object *other, SabotageVictimType type = SAB_VICTIM_GENERIC );
  
protected:

	/// This is the game logic execution function that all real CrateCollides will implement
	virtual Bool executeCrateBehavior( Object *other ) = 0;

	/// This allows specific vetoes to certain types of crates and their data
	virtual Bool isValidToExecute( const Object *other ) const;

};

#endif
