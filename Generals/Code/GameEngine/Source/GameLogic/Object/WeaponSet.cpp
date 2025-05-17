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

// FILE: WeaponSet.cpp ///////////////////////////////////////////////////////////////////////////////
// Author: Steven Johnson, March 2002
// Desc:   Weapon descriptions
///////////////////////////////////////////////////////////////////////////////////////////////////


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#define DEFINE_WEAPONSLOTTYPE_NAMES
#define DEFINE_COMMANDSOURCEMASK_NAMES

#include "GameLogic/WeaponSet.h"

#include "Common/INI.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "Common/BitFlagsIO.h"

#include "GameLogic/AI.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/ContainModule.h"
#include "GameLogic/Module/StealthUpdate.h"
#include "GameLogic/Module/SpawnBehavior.h"

#include "GameLogic/Weapon.h"


#ifdef RTS_INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC DATA ////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

const char* WeaponSetFlags::s_bitNameList[] = 
{
	"VETERAN",
	"ELITE",
	"HERO",
	"PLAYER_UPGRADE",
	"CRATEUPGRADE_ONE",
	"CRATEUPGRADE_TWO",
	"VEHICLE_HIJACK",
	"CARBOMB",
	"MINE_CLEARING_DETAIL",

	NULL
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
void WeaponTemplateSet::clear()
{
	m_isReloadTimeShared = false;
	m_isWeaponLockSharedAcrossSets = FALSE;
	m_types.clear();
	for (int i = 0; i < WEAPONSLOT_COUNT; ++i) 
	{
		m_template[i] = NULL;
		m_autoChooseMask[i] = 0xffffffff;					// by default, allow autochoosing from any CommandSource
		CLEAR_KINDOFMASK(m_preferredAgainst[i]);	// by default, weapon isn't preferred against anything in particular
	}
}

//-------------------------------------------------------------------------------------------------
Bool WeaponTemplateSet::hasAnyWeapons() const
{
	for (int i = 0; i < WEAPONSLOT_COUNT; ++i) 
	{
		if (m_template[i])
			return true;
	}
	return false;
}

//-------------------------------------------------------------------------------------------------
void WeaponTemplateSet::parseWeapon(INI* ini, void *instance, void * /*store*/, const void* userData)
{
	WeaponTemplateSet* self = (WeaponTemplateSet*)instance;
	WeaponSlotType wslot = (WeaponSlotType)INI::scanIndexList(ini->getNextToken(), TheWeaponSlotTypeNames);
	INI::parseWeaponTemplate(ini, instance, &self->m_template[wslot], NULL);
}

//-------------------------------------------------------------------------------------------------
void WeaponTemplateSet::parseAutoChoose(INI* ini, void *instance, void * /*store*/, const void* userData)
{
	WeaponTemplateSet* self = (WeaponTemplateSet*)instance;
	WeaponSlotType wslot = (WeaponSlotType)INI::scanIndexList(ini->getNextToken(), TheWeaponSlotTypeNames);
	INI::parseBitString32(ini, instance, &self->m_autoChooseMask[wslot], TheCommandSourceMaskNames);
}

//-------------------------------------------------------------------------------------------------
void WeaponTemplateSet::parsePreferredAgainst(INI* ini, void *instance, void * /*store*/, const void* userData)
{
	WeaponTemplateSet* self = (WeaponTemplateSet*)instance;
	WeaponSlotType wslot = (WeaponSlotType)INI::scanIndexList(ini->getNextToken(), TheWeaponSlotTypeNames);
	KindOfMaskType::parseFromINI(ini, instance, &self->m_preferredAgainst[wslot], NULL);
}

//-------------------------------------------------------------------------------------------------
void WeaponTemplateSet::parseWeaponTemplateSet( INI* ini, const ThingTemplate* tt )
{
	static const FieldParse myFieldParse[] = 
	{
		{ "Conditions", WeaponSetFlags::parseFromINI, NULL, offsetof( WeaponTemplateSet, m_types ) },
		{ "Weapon",	WeaponTemplateSet::parseWeapon,	NULL, 0 },
		{ "AutoChooseSources",	WeaponTemplateSet::parseAutoChoose, NULL, 0 },
		{ "PreferredAgainst", WeaponTemplateSet::parsePreferredAgainst, NULL, 0 },
		{ "ShareWeaponReloadTime", INI::parseBool, NULL, offsetof( WeaponTemplateSet, m_isReloadTimeShared ) },
		{ "WeaponLockSharedAcrossSets", INI::parseBool, NULL, offsetof( WeaponTemplateSet, m_isWeaponLockSharedAcrossSets ) },
		{ 0, 0, 0, 0 }
	};

	ini->initFromINI(this, myFieldParse);
	this->m_thingTemplate = tt;
}

//-------------------------------------------------------------------------------------------------
Bool WeaponTemplateSet::testWeaponSetFlag( WeaponSetType wst ) const
{
	return m_types.test( wst );
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
WeaponSet::WeaponSet()
{
	m_curWeapon = PRIMARY_WEAPON;
	m_curWeaponLockedStatus = NOT_LOCKED;
	m_curWeaponTemplateSet = NULL;
	m_filledWeaponSlotMask = 0;
	m_totalAntiMask = 0;
	m_totalDamageTypeMask = 0;
	DEBUG_ASSERTCRASH(DAMAGE_NUM_TYPES <= 32, ("m_totalDamageTypeMask will need to be enlarged in WeaponSet"));
	m_hasPitchLimit = false;
	m_hasDamageWeapon = false;
	for (Int i = 0; i < WEAPONSLOT_COUNT; ++i)
		m_weapons[i] = NULL;
}

//-------------------------------------------------------------------------------------------------
WeaponSet::~WeaponSet()
{
	for (Int i = 0; i < WEAPONSLOT_COUNT; ++i)
		if (m_weapons[i])
			m_weapons[i]->deleteInstance();
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void WeaponSet::crc( Xfer *xfer )
{

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void WeaponSet::xfer( Xfer *xfer )
{
	// version
	const XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	if (xfer->getXferMode() == XFER_LOAD)
	{
		AsciiString ttName;
		WeaponSetFlags wsFlags;

		xfer->xferAsciiString(&ttName);
		wsFlags.xfer( xfer );

		if (ttName.isEmpty())
		{
			m_curWeaponTemplateSet = NULL;
		}
		else
		{
			const ThingTemplate* tt = TheThingFactory->findTemplate(ttName);
			if (tt == NULL)
				throw INI_INVALID_DATA;

			m_curWeaponTemplateSet = tt->findWeaponTemplateSet(wsFlags);
			if (m_curWeaponTemplateSet == NULL)
				throw INI_INVALID_DATA;
		}
	}
	else if (xfer->getXferMode() == XFER_SAVE)
	{
		AsciiString ttName;				// leave 'em empty in case we're null
		WeaponSetFlags wsFlags;
		if (m_curWeaponTemplateSet != NULL)
		{
			const ThingTemplate* tt = m_curWeaponTemplateSet->friend_getThingTemplate();
			if (tt == NULL)
				throw INI_INVALID_DATA;
		
			ttName = tt->getName();
			wsFlags = m_curWeaponTemplateSet->friend_getWeaponSetFlags();
		}
		xfer->xferAsciiString(&ttName);
		wsFlags.xfer( xfer );
	}

	for (Int i = 0; i < WEAPONSLOT_COUNT; ++i)
	{
		Bool hasWeaponInSlot = (m_weapons[i] != NULL);
		xfer->xferBool(&hasWeaponInSlot);
		if (hasWeaponInSlot)
		{
			if (xfer->getXferMode() == XFER_LOAD && m_weapons[i] == NULL)
			{
				const WeaponTemplate* wt = m_curWeaponTemplateSet->getNth((WeaponSlotType)i);
				if (wt==NULL) {
					DEBUG_CRASH(("xfer backwards compatibility code - old save file??? jba."));
					wt = m_curWeaponTemplateSet->getNth((WeaponSlotType)0);
				}
				m_weapons[i] = TheWeaponStore->allocateNewWeapon(wt, (WeaponSlotType)i);
			}
			xfer->xferSnapshot(m_weapons[i]);
		}
	}
	xfer->xferUser(&m_curWeapon, sizeof(m_curWeapon));
	xfer->xferUser(&m_curWeaponLockedStatus, sizeof(m_curWeaponLockedStatus));
	xfer->xferUnsignedInt(&m_filledWeaponSlotMask);
	xfer->xferInt(&m_totalAntiMask);
	xfer->xferUnsignedInt(&m_totalDamageTypeMask);
	xfer->xferBool(&m_hasDamageWeapon);
	xfer->xferBool(&m_hasDamageWeapon);

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void WeaponSet::loadPostProcess( void )
{

}  // end loadPostProcess

//-------------------------------------------------------------------------------------------------
void WeaponSet::updateWeaponSet(const Object* obj)
{
	const WeaponTemplateSet* set = obj->getTemplate()->findWeaponTemplateSet(obj->getWeaponSetFlags());
	DEBUG_ASSERTCRASH(set, ("findWeaponSet should never return null"));
	if (set && set != m_curWeaponTemplateSet)
	{
		if( ! set->isWeaponLockSharedAcrossSets() )
		{
			DEBUG_ASSERTLOG(!isCurWeaponLocked(), ("changing WeaponSet while Weapon is Locked... implicit unlock occurring!\n"));
			releaseWeaponLock(LOCKED_PERMANENTLY);	// release all locks. sorry!
			m_curWeapon = PRIMARY_WEAPON;
		}
		m_filledWeaponSlotMask = 0;
		m_totalAntiMask = 0;
		m_totalDamageTypeMask = 0;
		m_hasPitchLimit = false;
		m_hasDamageWeapon = false;
		for (Int i = WEAPONSLOT_COUNT - 1; i >= PRIMARY_WEAPON ; --i)
		{
			if (m_weapons[i] != NULL)
			{
				m_weapons[i]->deleteInstance();
				m_weapons[i] = NULL;
			}

			if (set->getNth((WeaponSlotType)i))
			{
				m_weapons[i] = TheWeaponStore->allocateNewWeapon(set->getNth((WeaponSlotType)i), (WeaponSlotType)i);
				m_weapons[i]->loadAmmoNow(obj);	// start 'em all with full clips.
				m_filledWeaponSlotMask |= (1 << i);
				m_totalAntiMask |= m_weapons[i]->getAntiMask();
				m_totalDamageTypeMask |= (1 << m_weapons[i]->getDamageType());
				if (m_weapons[i]->isPitchLimited())
					m_hasPitchLimit = true;
				if (m_weapons[i]->isDamageWeapon())
					m_hasDamageWeapon = true;

				// no, do NOT do this; always start with the cur weapon being primary, even if there is no primary
				// weapon. this is by design, to allow us to have units that have only "spell" weapons and no
				// "normal" weapons. (srj)
				// m_curWeapon = (WeaponSlotType)i;
			}
		}
		m_curWeaponTemplateSet = set;
		//DEBUG_LOG(("WeaponSet::updateWeaponSet -- changed curweapon to %s\n",getCurWeapon()->getName().str()));
	}
}

//-------------------------------------------------------------------------------------------------
/*static*/ ModelConditionFlags WeaponSet::getModelConditionForWeaponSlot(WeaponSlotType wslot, WeaponSetConditionType a)
{
	static const ModelConditionFlagType Nothing[WEAPONSLOT_COUNT] = { MODELCONDITION_INVALID, MODELCONDITION_INVALID, MODELCONDITION_INVALID };
	static const ModelConditionFlagType Firing[WEAPONSLOT_COUNT] = { MODELCONDITION_FIRING_A, MODELCONDITION_FIRING_B, MODELCONDITION_FIRING_C };
	static const ModelConditionFlagType Betweening[WEAPONSLOT_COUNT] = { MODELCONDITION_BETWEEN_FIRING_SHOTS_A, MODELCONDITION_BETWEEN_FIRING_SHOTS_B, MODELCONDITION_BETWEEN_FIRING_SHOTS_C };
	static const ModelConditionFlagType Reloading[WEAPONSLOT_COUNT] = { MODELCONDITION_RELOADING_A, MODELCONDITION_RELOADING_B, MODELCONDITION_RELOADING_C };
	static const ModelConditionFlagType PreAttack[WEAPONSLOT_COUNT] = { MODELCONDITION_PREATTACK_A, MODELCONDITION_PREATTACK_B, MODELCONDITION_PREATTACK_C };
	static const ModelConditionFlagType* Lookup[WSF_COUNT] = { Nothing, Firing, Betweening, Reloading, PreAttack };

	ModelConditionFlags flags;	// defaults to all clear

	ModelConditionFlagType f = Lookup[a][wslot];
	if (f != MODELCONDITION_INVALID)
		flags.set(f);
	
	static const ModelConditionFlagType Using[WEAPONSLOT_COUNT] = { MODELCONDITION_USING_WEAPON_A, MODELCONDITION_USING_WEAPON_B, MODELCONDITION_USING_WEAPON_C };
	if (a != WSF_NONE)
		flags.set(Using[wslot]);

	return flags;
}

//-------------------------------------------------------------------------------------------------
static Int getVictimAntiMask(const Object* victim)
{
	if( victim->isKindOf( KINDOF_MINE ) )
	{
		return WEAPON_ANTI_MINE | WEAPON_ANTI_GROUND;
	}
	else if( victim->isKindOf( KINDOF_SMALL_MISSILE ) )
	{
		//All missiles are also projectiles!
		return WEAPON_ANTI_SMALL_MISSILE;
	}
	else if( victim->isKindOf( KINDOF_BALLISTIC_MISSILE ) )
	{
		return WEAPON_ANTI_BALLISTIC_MISSILE;
	}
	else if( victim->isKindOf( KINDOF_PROJECTILE ) )
	{
		return WEAPON_ANTI_PROJECTILE;
	}
	else if( victim->isAirborneTarget() )
	{
		if( victim->isKindOf( KINDOF_VEHICLE ) )
		{
			return WEAPON_ANTI_AIRBORNE_VEHICLE;
		}
		else if( victim->isKindOf( KINDOF_INFANTRY ) )
		{
			return WEAPON_ANTI_AIRBORNE_INFANTRY;
		}
		else if( victim->isKindOf( KINDOF_PARACHUTE ) )
		{
			return WEAPON_ANTI_PARACHUTE;
		}
		else if( !victim->isKindOf( KINDOF_UNATTACKABLE ) )
		{
			DEBUG_CRASH( ("Object %s is being targetted as airborne, but is not infantry, nor vehicle. Is this legit? -- tell Kris", victim->getTemplate()->getName().str() ) );
		}
		return 0;
	}
	else
	{
		return WEAPON_ANTI_GROUND;
	}
}

//-------------------------------------------------------------------------------------------------
Bool WeaponSet::isAnyWithinTargetPitch(const Object* obj, const Object* victim) const
{
	if (!m_hasPitchLimit)
		return true;

	for( Int i = 0; i < WEAPONSLOT_COUNT;	i++ )
	{
		const Weapon* weapon = m_weapons[ i ];
		if (weapon && weapon->isWithinTargetPitch(obj, victim))
		{
			return true;
		}
	}
	return false;
}

//-------------------------------------------------------------------------------------------------
CanAttackResult WeaponSet::getAbleToAttackSpecificObject( AbleToAttackType attackType, const Object* source, const Object* victim, CommandSourceType commandSource ) const
{

	// basic sanity checks.
	if (!source || 
			!victim || 
			source->isEffectivelyDead() || 
			victim->isEffectivelyDead() || 
			source->isDestroyed() || 
			victim->isDestroyed() || 
			victim == source)
		return ATTACKRESULT_NOT_POSSIBLE;

	Bool sameOwnerForceAttack = ((source->getControllingPlayer() == victim->getControllingPlayer()) && isForcedAttack(attackType));

// ----- examine VICTIM to determine legality of attack

	//
	// an "OBJECT_STATUS_MASKED" object is not really a valid target for attacking, this masked
	// status is an "override" that can be applied to objects that are otherwise totally
	// valid targets when we really really really don't want anything in the world
	// being able to attack stuff
	//
	if (victim->testStatus(OBJECT_STATUS_MASKED))
		return ATTACKRESULT_NOT_POSSIBLE;

	// And this is the KINDOF version, for things that aren't really game objects, but need to have "life"
	// so they can have lifetimeupdates and vision
	if (victim->isKindOf(KINDOF_UNATTACKABLE))
		return ATTACKRESULT_NOT_POSSIBLE;

	// this object is not currently auto-acquireable
	if (victim->testStatus(OBJECT_STATUS_NO_ATTACK_FROM_AI) && commandSource == CMD_FROM_AI)
		return ATTACKRESULT_NOT_POSSIBLE;

  Bool allowStealthToPreventAttacks = TRUE;
	if (source->testStatus(OBJECT_STATUS_IGNORING_STEALTH) || sameOwnerForceAttack)
		allowStealthToPreventAttacks = FALSE;
  if( isForcedAttack( attackType ) && victim->isKindOf( KINDOF_DISGUISER ) )
  {
    // force-attack allows you to attack disguised things, which also happen to be stealthed.
    // since we normally disallow attacking stealthed things (even via force-fire), we check
    // for disguised and explicitly ignore stealth in that case
	  StealthUpdate *update = victim->getStealth();
    if (update && update->isDisguised())
  	  allowStealthToPreventAttacks = FALSE;
  }

	// If an object is stealthed and hasn't been detected yet, then it is not a valid target to fire 
	// on.
	if (allowStealthToPreventAttacks && 
				victim->testStatus(OBJECT_STATUS_STEALTHED) && 
				!victim->testStatus(OBJECT_STATUS_DETECTED))
	{
		if( !victim->isKindOf( KINDOF_DISGUISER ) )
		{
			return ATTACKRESULT_NOT_POSSIBLE;
		}
		else
		{
			//Exception case -- don't return false if we are a bomb truck disguised as an enemy vehicle.
			StealthUpdate *update = victim->getStealth();
			if( update && update->isDisguised() )
			{
				Player *ourPlayer = source->getControllingPlayer();
				Player *otherPlayer = ThePlayerList->getNthPlayer( update->getDisguisedPlayerIndex() );
				if( ourPlayer && otherPlayer )
				{
					if( ourPlayer->getRelationship( otherPlayer->getDefaultTeam() ) != ENEMIES )
					{
						//Our stealthed & undetected object is disguised as a unit not perceived to be our enemy.
						return ATTACKRESULT_NOT_POSSIBLE;
					}
				}
			}
		}
	}

	// If the victim is fully fogged or fully shrouded, then we cannot attack them
	// GS -- Shroud only applies in decision making to Player owned objects and when the source is not from script
	// SRJ -- ignore shroud checks if mode is CONTINUED and the target is immobile, since presumably
	// we know where it is (and it's not going anywhere); this prevents wonky behavior if you have planes attack 
	// a deshrouded thing, which becomes reshrouded before they get there (basically the planes circle once in order
	// to deshroud it again, and get nailed by AA fire in the meantime)

	// GS -- Fog no longer prevents attacking at all.  it prevents targeting in findClosestEnemy.  Leaving this
	// here commented out to show that shroud should not be checked.
//  const Player* sourceController = source->getControllingPlayer();
//	if( sourceController
 //   && sourceController->getPlayerType() == PLAYER_HUMAN
//		&& commandSource != CMD_FROM_SCRIPT
//		&& !(isContinuedAttack(attackType) && victim->isKindOf(KINDOF_IMMOBILE))
//		&&  victim->getShroudedStatus(sourceController->getPlayerIndex()) >= OBJECTSHROUD_FOGGED 
//		)
//	{
//		return ATTACKRESULT_NOT_POSSIBLE;
//	}

	//
	// At least one of the following must be true:
	//
	// -- victim is our enemy
	// -- victim is a Bridge Tower (they are always neutral)
	// -- victim is a non-allied Mine
	// -- attack mode is force attack
	//
	// if none of the above is true, we can't attack. sorry!
	//
	Relationship r = source->getRelationship(victim);
	if (r != ENEMIES &&
			!isForcedAttack(attackType) &&
// GS			!victim->isKindOf( KINDOF_BRIDGE_TOWER ) && // Repairing bridges cut 12/12/02, so attacking is too
			!(victim->isKindOf( KINDOF_MINE ) && r != ALLIES))
	{
		//Only reject this if the command is from a player. If scripts or AI order attacks, they don't
		//care about relationships (and fixes broken scripts).
		if( commandSource == CMD_FROM_PLAYER && (!victim->testScriptStatusBit( OBJECT_STATUS_SCRIPT_TARGETABLE ) || r == ALLIES) )
		{
			//Unless the object has a map propertly that sets it to be targetable (and not allied), then give up.
			return ATTACKRESULT_NOT_POSSIBLE;
		}
	}

	// if the victim is contained within an enclosing container, it cannot be attacked directly
	const Object* victimsContainer = victim->getContainedBy();
	if (victimsContainer != NULL && victimsContainer->getContain()->isEnclosingContainerFor(victim) == TRUE)
	{
		return ATTACKRESULT_NOT_POSSIBLE;
	}

	// check if the container has an apparent controller, that controller must be an enemy
	// (unless this is a force-attack)
	if (!isForcedAttack(attackType))
	{
		const ContainModuleInterface* victimContain = victim->getContain();
		if (victimContain)
		{
			const Player* victimApparentController = victimContain->getApparentControllingPlayer(source->getControllingPlayer());
			if (victimApparentController && source->getTeam()->getRelationship(victimApparentController->getDefaultTeam()) != ENEMIES)
			{
				//Only reject this if the command is from a player. If scripts or AI order attacks, they don't
				//care about relationships (and fixes broken scripts).
				if( commandSource == CMD_FROM_PLAYER && (!victim->testScriptStatusBit( OBJECT_STATUS_SCRIPT_TARGETABLE ) || r == ALLIES) )
				{
					//Unless the object has a map propertly that sets it to be targetable (and not allied), then give up.
					return ATTACKRESULT_NOT_POSSIBLE;
				}
			}
		}
	}

	//Check if the shot itself is valid!
	return getAbleToUseWeaponAgainstTarget( attackType, source, victim, victim->getPosition(), commandSource );
}

//-------------------------------------------------------------------------------------------------
//This is formerly the 2nd half of getAbleToAttackSpecificObject
//This function is responsible for determining if our object is physically capable of attacking the target and it
//supports both victim or position.
CanAttackResult WeaponSet::getAbleToUseWeaponAgainstTarget( AbleToAttackType attackType, const Object *source, const Object *victim, const Coord3D *pos, CommandSourceType commandSource ) const
{

	//First determine if we are attacking an object or the ground and get the 
	//appropriate weapon anti mask.
	WeaponAntiMaskType targetAntiMask;
	if( victim )
	{
		//Attacking a specific object -- get the victim's anti weapon mask!
		targetAntiMask = (WeaponAntiMaskType)getVictimAntiMask( victim );

		//Make sure that the pos matches the victim!
		pos = victim->getPosition();
	}
	else
	{
		//Attacking the ground so this is obvious.
		targetAntiMask = WEAPON_ANTI_GROUND;
	}
	

	// Special Case test for turreted weapons on buildings... since they cannot move, they must be within range of target
	//Kris: Actually -- let's just do this for all immobile objects. If we can give it orders to attack, then we must check
	// the range anyways. This will also fix stinger soldiers.
	//Kris: ***NOTE*** KINDOF_SPAWNS_ARE_THE_WEAPONS was added here because initially I tried to get this to work by
	//      making stinger soldiers immobile. However, doing so prevents them from attacking because they can't turn without
	//      a locomotor! In anycase, the stinger site when ordered to attack will pass the spawn member closest to the target
	//      to this function for evaluation in it's place. So to get that code into this area, giving it the kindof bit will
	//      accomplish that. This is fine because only these two units have this set. Should that change in the future, this
	//      will need re-evaluation (can be solved with a new kindof).
	//
	// srj sez: contained soldiers are a lot like turrets in this sense. they should only attempt to acquire targets
	// within range, even if their container is mobile. otherwise they end up resetting their mood-check
	// times in odd ways in some cases. hence the getContainedBy() check.
	//Kris: Dec 28 -- Now we're testing this for ALL objects, but we only abort if the object is immobile. We'll use the
	//		  information later to determine if the unit must move before firing (cursor reasons).
	Bool withinAttackRange = FALSE;
	const Object *containedBy = source->getContainedBy();
	Bool hasAWeaponInRange = FALSE;
	Bool hasAWeapon				 = FALSE;
	for (Int slot = 0; slot < WEAPONSLOT_COUNT - 1; ++slot)
	{
		Weapon *weaponToTestForRange = m_weapons[ m_curWeapon ];
		if ( weaponToTestForRange )
		{
			hasAWeapon = TRUE;
			if ((m_totalAntiMask & targetAntiMask) == 0)//we don't care to check for this weapon
				continue;

			Bool handled = FALSE;
			ContainModuleInterface *contain = containedBy ? containedBy->getContain() : NULL;
			if( contain && contain->isGarrisonable() )
			{
				//For contained things, we need to fake-move objects to the best garrison point in order
				//to get precise range checks.
				Coord3D targetPos = *pos;
				Coord3D goalPos;
				if( contain->calcBestGarrisonPosition( &goalPos, &targetPos ) )
				{
					withinAttackRange = weaponToTestForRange->isSourceObjectWithGoalPositionWithinAttackRange( source, &goalPos, victim, &targetPos );
					handled = TRUE;
				}
			}
			else if( victim )
				withinAttackRange = weaponToTestForRange->isWithinAttackRange( source, victim );
			else
				withinAttackRange = weaponToTestForRange->isWithinAttackRange( source, pos );
			if( withinAttackRange )
			{
				hasAWeaponInRange = TRUE;
				break;
			}
		}
	}

	if( source->isKindOf( KINDOF_IMMOBILE ) || source->isKindOf( KINDOF_SPAWNS_ARE_THE_WEAPONS ) || containedBy )
	{
		if ( hasAWeapon && !hasAWeaponInRange && attackType != ATTACK_TUNNEL_NETWORK_GUARD )
			return ATTACKRESULT_INVALID_SHOT;

	}

	CanAttackResult okResult = withinAttackRange ? ATTACKRESULT_POSSIBLE : ATTACKRESULT_POSSIBLE_AFTER_MOVING;
	

// ----- things we examine about SOURCE to determine legality of attack

	if (hasAnyDamageWeapon())
	{
		if ((m_totalAntiMask & targetAntiMask) == 0)
			return ATTACKRESULT_INVALID_SHOT;

		//If we don't have a victim, we are force attacking a position. Because
		//of the weapon mask check preceding this, the attack is possible!
		if( !victim )
			return okResult;
		
		if (!isAnyWithinTargetPitch(source, victim))
			return ATTACKRESULT_INVALID_SHOT;

		Int first, last;
		if( isCurWeaponLocked() )
		{
			first = m_curWeapon;
			last = m_curWeapon;
		}
		else
		{
			first = WEAPONSLOT_COUNT - 1;
			last = PRIMARY_WEAPON;
		}

		//Check each weapon, until we find one that can do damage (or fail)
		for( Int i = first; i >= last ; --i )
		{
			Weapon *weapon = m_weapons[ i ];
			if (weapon && weapon->estimateWeaponDamage( source, victim ))
			{
				return okResult;
			}
		}
	}

	// Do a check to see if we have an occupied container (garrisoned building, transport that allows passengers to fire).
	ContainModuleInterface *contain = source->getContain();
	if (contain && contain->isPassengerAllowedToFire())
	{
		// Loop through each member and if just one of them can attack the specific target, then
		// we are good to go!
		const ContainedItemsList* items = contain->getContainedItemsList();
		if (items)
		{
			for (ContainedItemsList::const_iterator it = items->begin(); it != items->end(); ++it)
			{
				Object* garrisonedMember = *it;
				if( garrisonedMember->isAbleToAttack() )
				{
					CanAttackResult result = garrisonedMember->getAbleToUseWeaponAgainstTarget( attackType, victim, pos, commandSource );
					if( result == ATTACKRESULT_POSSIBLE || result == ATTACKRESULT_POSSIBLE_AFTER_MOVING )
					{
						return result;
					}
				}
			}
		}
	}


	// Do a check to see if we have a hive object that has slaved objects.
	SpawnBehaviorInterface* spawnInterface = source->getSpawnBehaviorInterface();
	if( spawnInterface && 
		spawnInterface->getCanAnySlavesUseWeaponAgainstTarget( attackType, victim, pos, commandSource ) == ATTACKRESULT_POSSIBLE )
	{

// srj sez: this bit is intended to fix the situation where you have a stinger site 
// selected and get the "attack if I move" cursor against an enemy. since the stinger site can't
// move or fire, you shouldn't really EVER get this cursor for it. and since we just verified above
// that our slaves (the soldiers) can attack correctly, just nork it.
		if (source->isKindOf( KINDOF_IMMOBILE ) 
				&& source->isKindOf( KINDOF_SPAWNS_ARE_THE_WEAPONS )
				&& okResult == ATTACKRESULT_POSSIBLE_AFTER_MOVING)
			okResult = ATTACKRESULT_POSSIBLE;

		return okResult;
	}

	// Oh well... guess not!
	return ATTACKRESULT_INVALID_SHOT;
}

//-------------------------------------------------------------------------------------------------
Bool WeaponSet::chooseBestWeaponForTarget(const Object* obj, const Object* victim, WeaponChoiceCriteria criteria, CommandSourceType cmdSource)
{
	/*
		1) The first criteria is weapon fitness.  If the object has two weapons that can fire concurrently, 
			find the set of weapons that can hit the given target.  If two weapons can hit the given target,
		2) Figure potential damage. If both weapons have the same potential damage (should never happen) then,
		3) Pick one.

		Another consideration is reloading.  A weapon that is not ready to fire right this moment should
		never be returned if there is another weapon that could be fired.  Readiness trumps Criteria.

		Note that this will never result in being locked into a wrong choice (ie "If only I had waited one
		more frame, I would have gotten the better weapon") because the AI is recomputing this choice
		every frame.  Picture a tank with a slow reloading missile and a fast low damage gun.  This function
		will choose the missile, then choose the gun, then want to choose the missile until the gun reloads,
		then choose the gun and go back to wanting to choose the missile, etc
	*/

	if (victim == NULL)
		return false; // Usually cause victim just got killed.  jba.

	if( isCurWeaponLocked() )
		return TRUE; // I have been forced into choosing a specific weapon, so it is right until someone says otherwise

	Bool found = FALSE;				// A Ready weapon has been found
	Bool foundBackup = FALSE;	// An unready, but valid weapon has been found

	Real longestRange = 0.0f;
	Real bestDamage = 0.0f;
	Real longestRangeBackup = 0.0f;
	Real bestDamageBackup = 0.0f;

	WeaponSlotType currentDecision = PRIMARY_WEAPON;
	WeaponSlotType currentDecisionBackup = PRIMARY_WEAPON;

	// go backwards, so that in the event of ties, the primary weapon is preferred
	for (Int i = WEAPONSLOT_COUNT - 1; i >= PRIMARY_WEAPON ; --i)
	{
		/*
			First: eliminate the weapons that cannot be used.
		*/

		// no weapon in this slot.
		if (!m_weapons[i])
			continue;
		
		// weapon not allowed to be specified via this command source.
		CommandSourceMask okSrcs = m_curWeaponTemplateSet->getNthCommandSourceMask((WeaponSlotType)i);
		if ((okSrcs & (1 << cmdSource)) == 0)
			continue;

		Weapon* weapon = m_weapons[i];
		if (weapon == NULL)
			continue;
		
		// weapon out of range.
		if (!weapon->isWithinAttackRange(obj, victim))
			continue;

		// weapon out of ammo.
		if (weapon->getStatus() == OUT_OF_AMMO && !weapon->getAutoReloadsClip())
			continue;
		
		// weapon not allowed to target this kind of thing.
		if (!(weapon->getAntiMask() & getVictimAntiMask(victim)))
			continue;

		if (!weapon->isWithinTargetPitch(obj, victim))
			continue;
		
		Real damage = weapon->estimateWeaponDamage(obj, victim);
		Real attackRange = weapon->getAttackRange(obj);
		Bool weaponIsReady = (weapon->getStatus() == READY_TO_FIRE);

		const AIUpdateInterface* ai = obj->getAI();
		if (ai && ai->isWeaponSlotOnTurretAndAimingAtTarget((WeaponSlotType)i, victim))
			weaponIsReady = false;

		// weapon would do no damage against this target.
		// exception: 'unresistable' weapons are allowed to do zero damage.
		if (damage <= 0.0f && weapon->getDamageType() != DAMAGE_UNRESISTABLE)
			continue;
	
		/*
			now that we've eliminated the impossible ones, let's decide which
			one we will prefer.
		*/

		/*
			if a weapon matches the magic "preferred" bits, it's ALWAYS preferred over other weapons, regardless
			of actual damage/range. (eg, the Comanche cannon, which is always preferred against infantry, rather
			than its antitank missiles.) we accomplish this cheesily, by magnifying the damage/range, and by declaring
			the weapon "ready" in a more liberal manner.
		*/
		const KindOfMaskType& preferredAgainst = m_curWeaponTemplateSet->getNthPreferredAgainstMask((WeaponSlotType)i);
		if (KINDOFMASK_ANY_SET(preferredAgainst) && victim->isKindOfMulti(preferredAgainst, KINDOFMASK_NONE))
		{
			const Real HUGE_DAMAGE = 1e10;		// wow, that's a lot of damage.
			const Real HUGE_RANGE = 1e10;			
			damage = HUGE_DAMAGE;
			attackRange = HUGE_RANGE;
			// preferred weapons are also kept if they are merely reloading. (if out of ammo, we can punt.)
			weaponIsReady = (weapon->getStatus() != OUT_OF_AMMO);
		}

		switch (criteria)
		{
			case PREFER_MOST_DAMAGE:
				if( !weaponIsReady )
				{
					// If this weapon is not ready, the best it can do is qualify as the Best Backup choice
					if (damage >= bestDamageBackup)
					{
						bestDamageBackup = damage;
						currentDecisionBackup = (WeaponSlotType)i;
						foundBackup = true;
					}
				}
				else
				{
					if (damage >= bestDamage)
					{
						bestDamage = damage;
						currentDecision = (WeaponSlotType)i;
						found = true;
					}
				}
				break;
			case PREFER_LONGEST_RANGE:
				{
					if( !weaponIsReady )
					{
						if (attackRange > longestRangeBackup)
						{
							longestRangeBackup = attackRange;
							currentDecisionBackup = (WeaponSlotType)i;
							foundBackup = true;
						}
					}
					else
					{
						if (attackRange > longestRange)
						{
							longestRange = attackRange;
							currentDecision = (WeaponSlotType)i;
							found = true;
						}
					}
				}
				break;
		}
	}

	if ( found )
	{
		// If we found a good best one, then just use it
		m_curWeapon = currentDecision;
	}
	else if ( foundBackup )
	{
		// No ready weapon was found, so return the most suitable unready one.
		m_curWeapon = currentDecisionBackup;
		found = TRUE;
	}
	else 
	{
		// No weapon at all was found, so we go back to primary.
		m_curWeapon = PRIMARY_WEAPON;
	}

	//DEBUG_LOG(("WeaponSet::chooseBestWeaponForTarget -- changed curweapon to %s\n",getCurWeapon()->getName().str()));

	return found;
}


//-------------------------------------------------------------------------------------------------
void WeaponSet::reloadAllAmmo(const Object *obj, Bool now)
{
	for( Int i = 0; i < WEAPONSLOT_COUNT;	i++ )
	{
		Weapon* weapon = m_weapons[i];
		if (weapon != NULL)
		{
			if (now)
				weapon->loadAmmoNow(obj);
			else
				weapon->reloadAmmo(obj);
		}
	}
}

//-------------------------------------------------------------------------------------------------
Bool WeaponSet::isOutOfAmmo() const
{
	for( Int i = 0; i < WEAPONSLOT_COUNT;	i++ )
	{
		const Weapon* weapon = m_weapons[i];
		if (weapon == NULL)
			continue;
		if (weapon->getStatus() != OUT_OF_AMMO)
		{
			return false;
		}
	}
	return true;
}

//-------------------------------------------------------------------------------------------------
const Weapon* WeaponSet::findAmmoPipShowingWeapon() const
{
	for( Int i = 0; i < WEAPONSLOT_COUNT;	i++ )
	{
		const Weapon *weapon = m_weapons[ i ];
		if (weapon && weapon->isShowsAmmoPips())
		{
			return weapon;
		}
	}
	return NULL;
}

//-------------------------------------------------------------------------------------------------
Weapon* WeaponSet::findWaypointFollowingCapableWeapon()
{
	for( Int i = WEAPONSLOT_COUNT - 1; i >= PRIMARY_WEAPON; i-- )
	{
		if( m_weapons[i] && m_weapons[i]->isCapableOfFollowingWaypoint() )
		{
			return m_weapons[i];
		}
	}
	return NULL;
}

//-------------------------------------------------------------------------------------------------
// A special type of command demands that you use this (normally unchooseable) weapon
// until told otherwise.
Bool WeaponSet::setWeaponLock( WeaponSlotType weaponSlot, WeaponLockType lockType )
{
	if (lockType == NOT_LOCKED)
	{
		DEBUG_CRASH(("calling setWeaponLock with NOT_LOCKED, so I am doing nothing... did you mean to use releaseWeaponLock()?\n"));
		return false;
	}

	// Verify the asked for weapon exists , choose it, and then lock it as choosen until unlocked
	// the old code was just plain wrong. (look at it in perforce and you'll see...)
	if (m_weapons[weaponSlot] != NULL)
	{
		if( lockType == LOCKED_PERMANENTLY )
		{
			m_curWeapon = weaponSlot;
			m_curWeaponLockedStatus = lockType;
			//DEBUG_LOG(("WeaponSet::setWeaponLock permanently -- changed curweapon to %s\n",getCurWeapon()->getName().str()));
		}
		else if( lockType == LOCKED_TEMPORARILY && m_curWeaponLockedStatus != LOCKED_PERMANENTLY )
		{
			m_curWeapon = weaponSlot;
			m_curWeaponLockedStatus = lockType;
			//DEBUG_LOG(("WeaponSet::setWeaponLock temporarily -- changed curweapon to %s\n",getCurWeapon()->getName().str()));
		}

		return true;
	}

	DEBUG_CRASH(("setWeaponLock: weapon %d not found (missing an upgrade?)\n", (Int)weaponSlot));
	return false;
}

//-------------------------------------------------------------------------------------------------
// Either we have successfully fired a full clip of our special attack, or we have switched
// weaponsets entirely, or any Player issued command besides special attack has been given.
void WeaponSet::releaseWeaponLock(WeaponLockType lockType)
{
	if( m_curWeaponLockedStatus == NOT_LOCKED )
		return;// Nothing to do

	if (lockType == LOCKED_PERMANENTLY)
	{
		// all locks released.
		m_curWeaponLockedStatus = NOT_LOCKED;
	}
	else if (lockType == LOCKED_TEMPORARILY)
	{
		// only unlocked if the current lock is temporary.
		if (m_curWeaponLockedStatus == LOCKED_TEMPORARILY)
			m_curWeaponLockedStatus = NOT_LOCKED;
	}
	else
	{
		DEBUG_CRASH(("calling releaseWeaponLock with NOT_LOCKED makes no sense. why did you do this?\n"));
	}
}

//-------------------------------------------------------------------------------------------------
Weapon* WeaponSet::getWeaponInWeaponSlot(WeaponSlotType wslot) const
{ 
	return m_weapons[wslot]; 
}

//-------------------------------------------------------------------------------------------------
	//When an AIAttackState is over, it needs to clean up any weapons that might be in leech range mode
	//or else those weapons will have unlimited range!
void WeaponSet::clearLeechRangeModeForAllWeapons()
{
	for( Int i = 0; i < WEAPONSLOT_COUNT;	i++ )
	{
		Weapon* weapon = m_weapons[ i ];
		if( weapon )
		{
			//Just clear the leech range active flag.
			weapon->setLeechRangeActive( FALSE );
		}
	}
}

//-------------------------------------------------------------------------------------------------
Bool WeaponSet::isSharedReloadTime() const
{
	if (m_curWeaponTemplateSet)
		return m_curWeaponTemplateSet->isSharedReloadTime();
	return false;
}
 
