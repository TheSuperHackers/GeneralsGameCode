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

// FILE: ActionManager.h //////////////////////////////////////////////////////////////////////////
// Author: Colin Day
// Desc:   TheActionManager is a convenient place for us to wrap up all sorts of logical
//				 queries about what objects can do in the world and to other objects.  The purpose
//				 of having a central place for this logic assists us in making these logical kind
//				 of queries in the user interface and allows us to use the same code to validate
//				 commands as they come in over the network interface in order to do the
//				 real action.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/SubsystemInterface.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Object;
class Player;
class SpecialPowerTemplate;
enum SpecialPowerType CPP_11(: Int);
enum WeaponSlotType CPP_11(: Int);
enum CommandSourceType CPP_11(: Int);
enum CanAttackResult CPP_11(: Int);

enum CanEnterType CPP_11(: Int)
{
	CHECK_CAPACITY,
	DONT_CHECK_CAPACITY,
	COMBATDROP_INTO
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class ActionManager : public SubsystemInterface
{

public:

	ActionManager( void );
	virtual ~ActionManager( void );

	virtual void init( void ) { };							///< subsystem interface
	virtual void reset( void ) { };							///< subsystem interface
	virtual void update( void ) { };						///< subsystem interface

	//Single unit to unit check
	Bool canGetRepairedAt( const Object *obj, const Object *repairDest, CommandSourceType commandSource );
	Bool canTransferSuppliesAt( const Object *obj, const Object *transferDest );
	Bool canDockAt( const Object *obj, const Object *dockDest, CommandSourceType commandSource );
	Bool canGetHealedAt( const Object *obj, const Object *healDest, CommandSourceType commandSource );
	Bool canRepairObject( const Object *obj, const Object *objectToRepair, CommandSourceType commandSource );
	Bool canResumeConstructionOf( const Object *obj, const Object *objectBeingConstructed, CommandSourceType commandSource );
	Bool canEnterObject( const Object *obj, const Object *objectToEnter, CommandSourceType commandSource, CanEnterType mode );
	CanAttackResult getCanAttackObject( const Object *obj, const Object *objectToAttack, CommandSourceType commandSource, AbleToAttackType attackType );
	Bool canConvertObjectToCarBomb( const Object *obj, const Object *objectToConvert, CommandSourceType commandSource );
	Bool canHijackVehicle( const Object *obj, const Object *ObjectToHijack, CommandSourceType commandSource ); // LORENZEN
	Bool canCaptureBuilding( const Object *obj, const Object *objectToCapture, CommandSourceType commandSource );
	Bool canDisableVehicleViaHacking( const Object *obj, const Object *objectToHack, CommandSourceType commandSource, Bool checkSourceRequirements = true );
#ifdef ALLOW_SURRENDER
	Bool canPickUpPrisoner( const Object *obj, const Object *prisoner, CommandSourceType commandSource );
#endif
	Bool canStealCashViaHacking( const Object *obj, const Object *objectToHack, CommandSourceType commandSource );
	Bool canSnipeVehicle( const Object *obj, const Object *objectToSnipe, CommandSourceType commandSource );
	Bool canBribeUnit( const Object *obj, const Object *objectToBribe, CommandSourceType commandSource );
	Bool canCutBuildingPower( const Object *obj, const Object *building, CommandSourceType commandSource );
	Bool canDisableBuildingViaHacking( const Object *obj, const Object *objectToHack, CommandSourceType commandSource );
	Bool canDoSpecialPowerAtLocation( const Object *obj, const Coord3D *loc, CommandSourceType commandSource, const SpecialPowerTemplate *spTemplate, const Object *objectInWay, UnsignedInt commandOptions, Bool checkSourceRequirements = true );
	Bool canDoSpecialPowerAtObject( const Object *obj, const Object *target, CommandSourceType commandSource, const SpecialPowerTemplate *spTemplate, UnsignedInt commandOptions, Bool checkSourceRequirements = true);
  Bool canDoSpecialPower( const Object *obj, const SpecialPowerTemplate *spTemplate, CommandSourceType commandSource, UnsignedInt commandOptions, Bool checkSourceRequirements = true );
	Bool canMakeObjectDefector( const Object *obj, const Object *objectToMakeDefector, CommandSourceType commandSource );
	Bool canFireWeaponAtLocation( const Object *obj, const Coord3D *loc, CommandSourceType commandSource, const WeaponSlotType slot, const Object *objectInWay );
	Bool canFireWeaponAtObject( const Object *obj, const Object *target, CommandSourceType commandSource, const WeaponSlotType slot );
  Bool canFireWeapon( const Object *obj, const WeaponSlotType slot, CommandSourceType commandSource );
	Bool canGarrison( const Object *obj, const Object *target, CommandSourceType commandSource );
	Bool canOverrideSpecialPowerDestination( const Object *obj, const Coord3D *loc, SpecialPowerType spType, CommandSourceType commandSource );

	//Player to unit check
	Bool canPlayerGarrison( const Player *player, const Object *target, CommandSourceType commandSource );

protected:

};

// EXTERNALS //////////////////////////////////////////////////////////////////////////////////////
extern ActionManager *TheActionManager;
