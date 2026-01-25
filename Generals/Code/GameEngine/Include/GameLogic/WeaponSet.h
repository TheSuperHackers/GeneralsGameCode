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

// WeaponSet.h

#pragma once

#include "Lib/BaseType.h"
#include "Common/GameType.h"
#include "Common/KindOf.h"
#include "Common/ModelState.h"
#include "Common/SparseMatchFinder.h"
#include "Common/Snapshot.h"
#include "GameLogic/Damage.h"

//-------------------------------------------------------------------------------------------------
class INI;
class Object;
class Weapon;
class WeaponTemplate;

enum CommandSourceType CPP_11(: Int);
enum DamageType CPP_11(: Int);

// for WeaponSetType. Part of detangling.
#include "GameLogic/WeaponSetType.h"

//-------------------------------------------------------------------------------------------------
// for WeaponSetFlags. Part of detangling.
#include "GameLogic/WeaponSetFlags.h"

#ifdef DEFINE_WEAPONSLOTTYPE_NAMES
static const char *const TheWeaponSlotTypeNames[] =
{
	"PRIMARY",
	"SECONDARY",
	"TERTIARY",

	nullptr
};
static_assert(ARRAY_SIZE(TheWeaponSlotTypeNames) == WEAPONSLOT_COUNT + 1, "Incorrect array size");

static const LookupListRec TheWeaponSlotTypeNamesLookupList[] =
{
	{ "PRIMARY",		PRIMARY_WEAPON },
	{ "SECONDARY",	SECONDARY_WEAPON },
	{ "TERTIARY",		TERTIARY_WEAPON },

	{ nullptr, 0	}
};
static_assert(ARRAY_SIZE(TheWeaponSlotTypeNamesLookupList) == WEAPONSLOT_COUNT + 1, "Incorrect array size");

#endif

//-------------------------------------------------------------------------------------------------
#ifdef DEFINE_WEAPONCONDITIONMAP
static const ModelConditionFlagType TheWeaponSetTypeToModelConditionTypeMap[WEAPONSET_COUNT] =
{
	MODELCONDITION_WEAPONSET_VETERAN,
	MODELCONDITION_WEAPONSET_ELITE,
	MODELCONDITION_WEAPONSET_HERO,
	MODELCONDITION_WEAPONSET_PLAYER_UPGRADE,
	MODELCONDITION_WEAPONSET_CRATEUPGRADE_ONE,
	MODELCONDITION_WEAPONSET_CRATEUPGRADE_TWO
};
#endif

//-------------------------------------------------------------------------------------------------
enum WeaponSetConditionType CPP_11(: Int)
{
	WSF_INVALID = -1,

	WSF_NONE = 0,
	WSF_FIRING,
	WSF_BETWEEN,
	WSF_RELOADING,
	WSF_PREATTACK,

	WSF_COUNT
};

//-------------------------------------------------------------------------------------------------
class WeaponTemplateSet
{
private:
	const ThingTemplate*		m_thingTemplate;	// needed for save/load
	WeaponSetFlags					m_types;
	const WeaponTemplate*		m_template[WEAPONSLOT_COUNT];
	UnsignedInt							m_autoChooseMask[WEAPONSLOT_COUNT];
	KindOfMaskType					m_preferredAgainst[WEAPONSLOT_COUNT];
	Bool										m_isReloadTimeShared;
	Bool										m_isWeaponLockSharedAcrossSets; ///< A weapon set so similar that it is safe to hold locks across

	static void parseWeapon(INI* ini, void *instance, void *store, const void* userData);
	static void parseAutoChoose(INI* ini, void *instance, void *store, const void* userData);
	static void parsePreferredAgainst(INI* ini, void *instance, void *store, const void* userData);

public:
	WeaponTemplateSet()
	{
		clear();
	}

	const ThingTemplate* friend_getThingTemplate() const { return m_thingTemplate; }	// only for WeaponSet::xfer
	const WeaponSetFlags& friend_getWeaponSetFlags() const { return m_types; }	// only for WeaponSet::xfer

	void clear();
	void parseWeaponTemplateSet( INI* ini, const ThingTemplate* tt );
	Bool testWeaponSetFlag( WeaponSetType wst ) const;
	Bool isSharedReloadTime( void ) const { return m_isReloadTimeShared; }
	Bool isWeaponLockSharedAcrossSets() const {return m_isWeaponLockSharedAcrossSets; }

	Bool hasAnyWeapons() const;
	const WeaponTemplate* getNth(WeaponSlotType n) const { return m_template[n]; }
	UnsignedInt getNthCommandSourceMask(WeaponSlotType n) const { return m_autoChooseMask[n]; }
	const KindOfMaskType& getNthPreferredAgainstMask(WeaponSlotType n) const { return m_preferredAgainst[n]; }

	Int getConditionsYesCount() const { return 1; }
	const WeaponSetFlags& getNthConditionsYes(Int i) const { return m_types; }
#if defined(RTS_DEBUG)
	inline AsciiString getDescription() const { return "ArmorTemplateSet"; }
#endif
};

//-------------------------------------------------------------------------------------------------
typedef std::vector<WeaponTemplateSet> WeaponTemplateSetVector;

//-------------------------------------------------------------------------------------------------
enum WeaponChoiceCriteria CPP_11(: Int)
{
	PREFER_MOST_DAMAGE,		///< choose the weapon that will do the most damage
	PREFER_LONGEST_RANGE	///< choose the weapon with the longest range (that will do nonzero damage)
};

//-------------------------------------------------------------------------------------------------
enum WeaponLockType CPP_11(: Int)
{
	NOT_LOCKED,							///< Weapon is not locked
	LOCKED_TEMPORARILY,			///< Weapon is locked until clip is empty, or current "attack" state exits
	LOCKED_PERMANENTLY			///< Weapon is locked until explicitly unlocked or lock is changed to another weapon
};

//-------------------------------------------------------------------------------------------------
enum CanAttackResult CPP_11(: Int)
{
	//Worst scenario to best scenario -- These must be done this way now!
	ATTACKRESULT_NOT_POSSIBLE,					//Can't possibly attack target.
	ATTACKRESULT_INVALID_SHOT,					//Not a clear shot
	ATTACKRESULT_POSSIBLE_AFTER_MOVING, //I can attack, but after moving closer.
	ATTACKRESULT_POSSIBLE,							//I can attack now.
};

//-------------------------------------------------------------------------------------------------
class WeaponSet : public Snapshot
{
private:
	const WeaponTemplateSet*	m_curWeaponTemplateSet;
	Weapon*										m_weapons[WEAPONSLOT_COUNT];
	WeaponSlotType						m_curWeapon;
	WeaponLockType						m_curWeaponLockedStatus;
	UnsignedInt								m_filledWeaponSlotMask;
	Int												m_totalAntiMask;						///< anti mask of all current weapons
	DamageTypeFlags						m_totalDamageTypeMask;			///< damagetype mask of all current weapons
	Bool											m_hasPitchLimit;
	Bool											m_hasDamageWeapon;

	Bool isAnyWithinTargetPitch(const Object* obj, const Object* victim) const;

protected:
	// snapshot methods
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void );

public:

	WeaponSet();
	~WeaponSet();

	void updateWeaponSet(const Object* obj);
	void reloadAllAmmo(const Object *obj, Bool now);
	Bool isOutOfAmmo() const;
	Bool hasAnyWeapon() const { return m_filledWeaponSlotMask != 0; }
	Bool hasAnyDamageWeapon() const { return m_hasDamageWeapon; }
	Bool hasWeaponToDealDamageType(DamageType typeToDeal) const { return m_totalDamageTypeMask.test(typeToDeal); }
	Bool hasSingleDamageType(DamageType typeToDeal) const { return (m_totalDamageTypeMask.test(typeToDeal) && (m_totalDamageTypeMask.count() == 1) ); }
	Bool isCurWeaponLocked() const { return m_curWeaponLockedStatus != NOT_LOCKED; }
	Weapon* getCurWeapon() { return m_weapons[m_curWeapon]; }
	const Weapon* getCurWeapon() const { return m_weapons[m_curWeapon]; }
	WeaponSlotType getCurWeaponSlot() const { return m_curWeapon; }
	Weapon* findWaypointFollowingCapableWeapon();
	const Weapon* findAmmoPipShowingWeapon() const;

	Bool setWeaponLock( WeaponSlotType weaponSlot, WeaponLockType lockType );
	void releaseWeaponLock(WeaponLockType lockType);
	Bool isSharedReloadTime() const;

	//When an AIAttackState is over, it needs to clean up any weapons that might be in leech range mode
	//or else those weapons will have unlimited range!
	void clearLeechRangeModeForAllWeapons();

	/**
		Determines if the unit has any weapon that could conceivably
		harm the victim. this does not take range, ammo, etc. into
		account, but immutable weapon properties, such as "can you
		target airborne victims".
	*/
	CanAttackResult getAbleToAttackSpecificObject( AbleToAttackType t, const Object* obj, const Object* victim, CommandSourceType commandSource ) const;

	//When calling this function, all conditions must be validated to the point where we have decided that we wish to attack the object (faction checks, etc).
	//Now, we are determining if the attack itself is able to be performed!
	CanAttackResult getAbleToUseWeaponAgainstTarget( AbleToAttackType attackType, const Object *source, const Object *victim, const Coord3D *pos, CommandSourceType commandSource ) const;

	/**
		Selects the best weapon for the given target, and sets it as the current weapon.
		If there is no weapon that can damage the target, false is returned (and the current-weapon is unchanged).
		Note that this DOES take weapon attack range into account.
	*/
	Bool chooseBestWeaponForTarget(const Object* obj, const Object* victim, WeaponChoiceCriteria criteria, CommandSourceType cmdSource);

	Weapon* getWeaponInWeaponSlot(WeaponSlotType wslot) const;

	static ModelConditionFlags getModelConditionForWeaponSlot(WeaponSlotType wslot, WeaponSetConditionType a);
};
