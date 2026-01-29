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

// FILE: UpdateModule.h /////////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, September 2001
// Desc:
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Common/Module.h"
#include "Common/GameType.h"
#include "Common/DisabledTypes.h"
#include "Common/Upgrade.h"
#include "GameLogic/Module/BehaviorModule.h"

#define DIRECT_UPDATEMODULE_ACCESS

//-------------------------------------------------------------------------------------------------
/** OBJECT UPDATE MODULE base class */
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class ProjectileUpdateInterface;
class AIUpdateInterface;		///< @todo Clean up this nasty hack (MSB)
class ExitInterface;
class DelayedUpgradeUpdateInterface;
class DockUpdateInterface;
class RailedTransportDockUpdateInterface;
class SpecialPowerUpdateInterface;
class SlavedUpdateInterface;
class SpawnBehavior;
class SlowDeathBehaviorInterface;
class PowerPlantUpdateInterface;
class ProductionUpdateInterface;
class HordeUpdateInterface;
class SpecialPowerTemplate;
class WeaponTemplate;
class DamageInfo;
class ParticleSystemTemplate;
class CommandButton;
class Waypoint;
enum CommandOption CPP_11(: Int);

//-------------------------------------------------------------------------------------------------
enum UpdateSleepTime CPP_11(: Int)
{
	UPDATE_SLEEP_INVALID	= 0,
	UPDATE_SLEEP_NONE			= 1,
	// (we use 0x3fffffff so that we can add offsets and not overflow...
	//	and also 'cuz we shift the value up by two bits for the phase.
	// note that at 30fps, this is ~414 days...
	UPDATE_SLEEP_FOREVER	= 0x3fffffff
};
#define UPDATE_SLEEP(numFrames)				((UpdateSleepTime)(numFrames))

// this is used to declare in which "phase" sleepy updates are called.
// all phase 0 are called before all phase 1 within a given frame, etc.
// this is really only used to facilitate some internal operations
// in an efficient way while still maintaining order-dependency; you should
// really never specify anything other than PHASE_NORMAL without very
// careful deliberation. If you need to, talk it over with folks first. (srj)
enum SleepyUpdatePhase CPP_11(: Int)
{
	// reserve 2 bits for phase. this still leaves us 30 bits for frame counter,
	// which, at 30fps, will still run for ~414 days without overflowing...
	PHASE_INITIAL				= 0,
	PHASE_PHYSICS				= 1,
	PHASE_NORMAL				= 2,
	PHASE_FINAL					= 3
};

//-------------------------------------------------------------------------------------------------
class UpdateModuleInterface
{
public:

	virtual UpdateSleepTime update() = 0;

	virtual DisabledMaskType getDisabledTypesToProcess() const = 0;

#ifdef DIRECT_UPDATEMODULE_ACCESS
	// these aren't in the interface; they are in the implementation,
	// because making them virtual is simply too much overhead.
	// no, really; I have the profiling data to prove it. (srj)
#else
	// these are for use ONLY by GameLogic scheduler. do not call otherwise,
	// and (generally) don't redefine them either!
	virtual UnsignedInt friend_getPriority() const = 0;
	virtual UnsignedInt friend_getNextCallFrame() const = 0;
	virtual SleepyUpdatePhase friend_getNextCallPhase() const = 0;
	virtual void friend_setNextCallFrame(UnsignedInt frame) = 0;
#endif

};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class UpdateModuleData : public BehaviorModuleData
{
public:

	static void buildFieldParse(MultiIniFieldParse& p)
	{
		BehaviorModuleData::buildFieldParse(p);
	}
};

//-------------------------------------------------------------------------------------------------
class UpdateModule : public BehaviorModule, public UpdateModuleInterface
{

	MEMORY_POOL_GLUE_ABC( UpdateModule )
	MAKE_STANDARD_MODULE_MACRO_ABC( UpdateModule )
	MAKE_STANDARD_MODULE_DATA_MACRO_ABC(UpdateModule, UpdateModuleData)

private:

	// this is an absolute frame, not a frame count.
	// actually, it's not a real frame at all, it has phase info in the lower bits...
	UnsignedInt m_nextCallFrameAndPhase;
	Int m_indexInLogic;

protected:

	// yes, protected: modules should only wake themselves up.
	void setWakeFrame(Object* obj, UpdateSleepTime wakeDelay);

	UpdateSleepTime getWakeFrame() const;

	/*
		You should pretty much never redefine this; it's really used for
		internal purposes by Object. See comment above.
	*/
	virtual SleepyUpdatePhase getUpdatePhase() const
	{
		return PHASE_NORMAL;
	}

	UpdateSleepTime frameToSleepTime(UnsignedInt frame1, UnsignedInt frame2 = FOREVER, UnsignedInt frame3 = FOREVER, UnsignedInt frame4 = FOREVER);

public:

	UpdateModule( Thing *thing, const ModuleData* moduleData );

	// virtual destructor prototype defined by MemoryPoolObject
	static Int getInterfaceMask() { return (MODULEINTERFACE_UPDATE); }

	// BehaviorModule
	virtual UpdateModuleInterface* getUpdate() { return this; }

	// UpdateModuleInterface
	virtual UpdateSleepTime update() = 0;

	DisabledMaskType getDisabledTypesToProcess() const
	{
		return DISABLEDMASK_NONE;
	}

#ifdef DIRECT_UPDATEMODULE_ACCESS
	#define UPDATEMODULE_FRIEND_DECLARATOR __forceinline
#else
	#define UPDATEMODULE_FRIEND_DECLARATOR virtual
#endif

	// for use ONLY by GameLogic scheduler. do not call otherwise.
	UPDATEMODULE_FRIEND_DECLARATOR UnsignedInt friend_getPriority() const
	{
		return m_nextCallFrameAndPhase;
	}

	UPDATEMODULE_FRIEND_DECLARATOR UnsignedInt friend_getNextCallFrame() const
	{
		return m_nextCallFrameAndPhase >> 2;
	}

	UPDATEMODULE_FRIEND_DECLARATOR SleepyUpdatePhase friend_getNextCallPhase() const
	{
		return (SleepyUpdatePhase)(m_nextCallFrameAndPhase & 3);
	}

	UPDATEMODULE_FRIEND_DECLARATOR void friend_setNextCallFrame(UnsignedInt frame)
	{
		// anything greater than "forever" is still "forever"
		// (this makes setWakeFrame() comparisons simpler and more efficient)
		if (frame > UPDATE_SLEEP_FOREVER)
			frame = UPDATE_SLEEP_FOREVER;
		m_nextCallFrameAndPhase = (frame << 2) | getUpdatePhase();
	}

	UPDATEMODULE_FRIEND_DECLARATOR Int friend_getIndexInLogic() const
	{
		return m_indexInLogic;
	}

	UPDATEMODULE_FRIEND_DECLARATOR void friend_setIndexInLogic(Int i)
	{
		m_indexInLogic = i;
	}

	UPDATEMODULE_FRIEND_DECLARATOR const Object* friend_getObject() const
	{
		return getObject();
	}

};
inline UpdateModule::UpdateModule( Thing *thing, const ModuleData* moduleData ) :
	BehaviorModule( thing, moduleData ),
	m_indexInLogic(-1),
	m_nextCallFrameAndPhase(0)
{
	// nothing
}
inline UpdateModule::~UpdateModule()
{
	DEBUG_ASSERTCRASH(m_indexInLogic == -1, ("destroying an updatemodule still in the logic list"));
}

//-------------------------------------------------------------------------------------------------
#ifdef DIRECT_UPDATEMODULE_ACCESS
typedef UpdateModule* UpdateModulePtr;
#else
typedef UpdateModuleInterface* UpdateModulePtr;
#endif

//-------------------------------------------------------------------------------------------------
class SlavedUpdateInterface
{
public:
	virtual ObjectID getSlaverID() const = 0;
	virtual void onEnslave( const Object *slaver ) = 0;
	virtual void onSlaverDie( const DamageInfo *info ) = 0;
	virtual void onSlaverDamage( const DamageInfo *info ) = 0;
	virtual	Bool isSelfTasking() const = 0;

};

//-------------------------------------------------------------------------------------------------
class ProjectileUpdateInterface
{
public:
	virtual void projectileLaunchAtObjectOrPosition(const Object *victim, const Coord3D* victimPos, const Object *launcher, WeaponSlotType wslot, Int specificBarrelToUse, const WeaponTemplate* detWeap, const ParticleSystemTemplate* exhaustSysOverride) = 0;						///< launch the projectile at the given victim
	virtual void projectileFireAtObjectOrPosition( const Object *victim, const Coord3D *victimPos, const WeaponTemplate *detWeap, const ParticleSystemTemplate* exhaustSysOverride ) = 0;
	virtual Bool projectileIsArmed() const = 0;													///< return true if the projectile is armed and ready to explode
	virtual ObjectID projectileGetLauncherID() const = 0;								///< All projectiles need to keep track of their firer
	virtual Bool projectileHandleCollision(Object *other) = 0;
};

//-------------------------------------------------------------------------------------------------
class DockUpdateInterface
{
public:
	/** Returns true if it is okay for the docker to approach and prepare to dock.
			False could mean the queue is full, for example.
	*/
	virtual Bool isClearToApproach( Object const* docker ) const = 0;

	/** Give me a Queue point to drive to, and record that that point is taken.
			Returning null means there are none free
	*/
	virtual Bool reserveApproachPosition( Object* docker, Coord3D *position, Int *index ) = 0;

	/** Give me the next Queue point to drive to, and record that that point is taken.
	*/
	virtual Bool advanceApproachPosition( Object* docker, Coord3D *position, Int *index ) = 0;

	/** Return true when it is OK for docker to begin entering the dock
			The Dock will lift the restriction on one particular docker on its own,
			so you must continually ask.
	*/
	virtual Bool isClearToEnter( Object const* docker ) const = 0;

	/** Return true when it is OK for docker to request a new Approach position.  The dock is in
			charge of keeping track of holes in the line, but the docker will remind us of their spot.
	*/
	virtual Bool isClearToAdvance( Object const* docker, Int dockerIndex ) const = 0;

	/** Give me the point that is the start of your docking path
			Returning null means there is none free
			All functions take docker as arg so we could have multiple docks on a building.
			Docker is not assumed, it is recorded and checked.
	*/
	virtual void getEnterPosition( Object* docker, Coord3D *position ) = 0;

	/** Give me the middle point of the dock process where the action() happens */
	virtual void getDockPosition( Object* docker, Coord3D *position ) = 0;

	/** Give me the point to drive to when I am done */
	virtual void getExitPosition( Object* docker, Coord3D *position ) = 0;

	virtual void onApproachReached( Object* docker ) = 0;		///< I have reached the Enter Point.
	virtual void onEnterReached( Object* docker ) = 0;			///< I have reached the Enter Point.
	virtual void onDockReached( Object* docker ) = 0;				///< I have reached the Dock point
	virtual void onExitReached( Object* docker ) = 0;				///< I have reached the exit.  You are no longer busy

	virtual Bool action( Object* docker, Object *drone = nullptr ) = 0;			///< Perform your specific action on me.  Returning FALSE means there is nothing for you to do so I should leave

	virtual void cancelDock( Object* docker ) = 0;	///< Clear me from any reserved points, and if I was the reason you were Busy, you aren't anymore.

	virtual Bool isDockOpen( void ) = 0;						///< Is the dock open to accepting dockers
	virtual void setDockOpen( Bool open ) = 0;			///< Open/Close the dock

	virtual void setDockCrippled( Bool setting ) = 0; ///< Game Logic can set me as inoperative.  I get to decide what that means.

	virtual Bool isAllowPassthroughType() = 0;	///< Not all docks allow you to path through them in your AIDock machine

	virtual Bool isRallyPointAfterDockType() = 0; ///< A minority of docks want to give you a final command to their rally point
};

//-------------------------------------------------------------------------------------------------
enum ExitDoorType CPP_11(: Int)
{
	DOOR_1 = 0,
	DOOR_2 = 1,
	DOOR_3 = 2,
	DOOR_4 = 3,

	DOOR_COUNT_MAX = 4,

	DOOR_NONE_AVAILABLE = -1,	// need a door, but none currently available
	DOOR_NONE_NEEDED		= -2	// don't need a door reservation
};

//-------------------------------------------------------------------------------------------------
///< Different types of modules have an interest in exiting units out of themselves for whatever reason.
class ExitInterface
{
public:
	virtual Bool isExitBusy() const = 0;	///< Contain style exiters are getting the ability to space out exits, so ask this before reserveDoor as a kind of no-commitment check.
	virtual ExitDoorType reserveDoorForExit( const ThingTemplate* objType, Object *specificObject ) = 0;		///< All types can answer if they are free to exit or not, and you can ask about a specific guy or just exit anything in general
	virtual void exitObjectViaDoor( Object *newObj, ExitDoorType exitDoor ) = 0;							///< Here is the object for you to exit to the world in your own special way
	virtual void exitObjectByBudding( Object *newObj, Object *budHost ) = 0;	///< puts new spawn on top of an existing one
	virtual void unreserveDoorForExit( ExitDoorType exitDoor ) = 0;	///< if you get permission to exit, but then don't/can't call exitObjectViaDoor, you should call this to "give up" your permission

	virtual void exitObjectInAHurry( Object *newObj) {}; ///< Special call for objects exiting a tunnel network, does NOT change the ai state. jba.

	virtual void setRallyPoint( const Coord3D *pos ) = 0;				///< define a "rally point" for units to move towards
	virtual const Coord3D *getRallyPoint( void ) const = 0;			///< define a "rally point" for units to move towards
	virtual Bool getNaturalRallyPoint( Coord3D& rallyPoint, Bool offset = TRUE ) const {rallyPoint.x=rallyPoint.y=rallyPoint.z=0; return false;}	///< get the natural "rally point" for units to move towards
	virtual Bool getExitPosition( Coord3D& exitPosition ) const {exitPosition.x=exitPosition.y=exitPosition.z=0; return false;};					///< access to the "Door" position of the production object
};

//-------------------------------------------------------------------------------------------------
class DelayedUpgradeUpdateInterface
{
public:
	virtual Bool isTriggeredBy( const UpgradeMaskType& potentialMask ) = 0;	///< If you were an upgrade, would you trigger for this?
	virtual void setDelay( UnsignedInt startingDelay ) = 0;	///< Start the upgrade doing countdown
};
