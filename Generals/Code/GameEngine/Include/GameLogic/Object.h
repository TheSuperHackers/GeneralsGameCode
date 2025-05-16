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

// Object.h ///////////////////////////////////////////////////////////////////
// Simple base object
// Author: Michael S. Booth, October 2000

#pragma once
#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "Lib/BaseType.h"

#include "Common/Geometry.h"
#include "Common/Snapshot.h"
#include "Common/SpecialPowerMaskType.h"
#include "Common/DisabledTypes.h"
#include "Common/Thing.h"
#include "Common/ObjectStatusTypes.h"
#include "Common/Upgrade.h"

#include "GameClient/Color.h"

#include "GameLogic/WeaponBonusConditionFlags.h"
#include "GameLogic/WeaponSet.h"
#include "GameLogic/WeaponSetFlags.h"
#include "GameLogic/Module/StealthUpdate.h"

//-----------------------------------------------------------------------------
//           Forward References
//-----------------------------------------------------------------------------

class AIGroup;
class AIUpdateInterface;
class Anim2DTemplate;
class BehaviorModule;
class BehaviorModuleInterface;
class BodyModuleInterface;
class CollideModule;
class CollideModuleInterface;
class CommandButton;
class ContainModuleInterface;
class CreateModuleInterface;
class DamageInfo;
class DamageInfoInput;
class DamageModule;
class DamageModuleInterface;
class DestroyModuleInterface;
class DockUpdateInterface;
class Dict;
class DieModule;
class DieModuleInterface;
class ExitInterface;
class ExperienceTracker;
class FiringTracker;
class Module;
class PartitionData;
class PhysicsBehavior;
class PhysicsUpdate;
class Player;
class PolygonTrigger;
class ProductionUpdateInterface;
class RadarObject;
class SightingInfo;
class SpawnBehaviorInterface;
class SpecialAbilityUpdate;
class SpecialPowerCompletionDie;
class SpecialPowerModuleInterface;
class SpecialPowerTemplate;
class SpecialPowerUpdateInterface;
class Team;
class UpdateModule;
class UpdateModuleInterface;
class UpgradeModule;
class UpgradeModuleInterface;
class UpgradeTemplate;

class ObjectHeldHelper;
class ObjectDisabledHelper;
class ObjectSMCHelper;
class ObjectRepulsorHelper;
class ObjectWeaponStatusHelper;
class ObjectDefectionHelper;

enum CommandSourceType CPP_11(: Int);
enum DamageType CPP_11(: Int);
enum HackerAttackMode CPP_11(: Int);
enum NameKeyType CPP_11(: Int);
enum SpecialPowerType CPP_11(: Int);
enum WeaponBonusConditionType CPP_11(: Int);
enum WeaponChoiceCriteria CPP_11(: Int);
enum WeaponSetConditionType CPP_11(: Int);
enum WeaponSetType CPP_11(: Int);
enum WeaponStatus CPP_11(: Int);
enum RadarPriorityType CPP_11(: Int);
enum CanAttackResult CPP_11(: Int);

// For ObjectStatusTypes
#include "Common/ObjectStatusTypes.h"

// For ObjectScriptStatusBit
#include "GameLogic/ObjectScriptStatusBits.h"

//-----------------------------------------------------------------------------
//           Type Defines
//-----------------------------------------------------------------------------

struct TTriggerInfo
{
	const PolygonTrigger*	pTrigger; ///< The trigger area that the object is inside.
	Byte									entered;	///< True if the object entered this trigger area this frame.
	Byte									exited;		///< True if the object entered this trigger area this frame.
	Byte									isInside;	///< True if the object is inside this trigger area this frame.
	Byte									padding;	///< unused.

	TTriggerInfo() : entered(false), exited(false), isInside(false), padding(false), pTrigger(NULL) { }

};

//----------------------------------------------------


enum CrushSquishTestType CPP_11(: Int)
{
	TEST_CRUSH_ONLY, 
	TEST_SQUISH_ONLY, 
	TEST_CRUSH_OR_SQUISH
};


// ---------------------------------------------------
/**
 * Object definition. Objects are manipulated via TheGameLogic singleton.
 * @todo Create an ObjectInterface class.
 */
class Object : public Thing, public Snapshot
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(Object, "ObjectPool" )		
	/// destructor is non-public in order to require the use of TheGameLogic->destroyObject()
	MEMORY_POOL_DELETEINSTANCE_VISIBILITY(protected)

public:

	/// Object constructor automatically attaches all objects to "TheGameLogic"
	Object(const ThingTemplate *thing, const ObjectStatusMaskType &objectStatusMask, Team *team);

	void initObject();

	void onDestroy();																							///< run during TheGameLogic::destroyObject

	Object* getNextObject() { return m_next; }
	const Object* getNextObject() const { return m_next; }

	void updateObjValuesFromMapProperties(Dict* properties);			///< Brings in properties set in the editor.

	// ids and binding
	ObjectID getID() const { return m_id; }												///< this object's unique ID
	void friend_bindToDrawable( Drawable *draw );									///< set drawable association. for use ONLY by GameLogic!
	Drawable* getDrawable() const { return m_drawable; }					///< drawable (if any) bound to obj

	ObjectID getProducerID() const { return m_producerID; }
	void setProducer(const Object* obj);

	ObjectID getBuilderID() const { return m_builderID; }
	void setBuilder( const Object *obj );

	void enterGroup( AIGroup *group );							///< become a member of the AIGroup
	void leaveGroup( void );												///< leave our current AIGroup
	AIGroup *getGroup(void);

	// physical properties
	Bool isMobile() const;																	///< returns true if object is currently able to move
	Bool isAbleToAttack() const;														///< returns true if object currently has some kind of attack capability

	void maskObject( Bool mask );				///< mask/unmask object

	// cannot set velocity, since this is calculated from position every frame
	Bool isDestroyed() const { return m_status.test( OBJECT_STATUS_DESTROYED ); }		///< Returns TRUE if object has been destroyed
	Bool isAirborneTarget() const { return m_status.test( OBJECT_STATUS_AIRBORNE_TARGET ); }	///< Our locomotor will control marking us as a valid target for anti air weapons or not
	Bool isUsingAirborneLocomotor( void ) const;										///< returns true if the current locomotor is an airborne one

	/// central place for us to put any additional capture logic
	void onCapture( Player *oldOwner, Player *newOwner );

	/// And game death logic.  Destroy is deletion of object as code
	void onDie( DamageInfo *damageInfo );

	// health and damage
	void attemptDamage( DamageInfo *damageInfo );			///< damage object as specified by the info
	void attemptHealing(Real amount, const Object* source);		///< heal object as specified by the info
	Bool attemptHealingFromSoleBenefactor ( Real amount, const Object* source, UnsignedInt duration );///< for the non-stacking healers like ambulance and propaganda
	ObjectID getSoleHealingBenefactor( void ) const;

	Real estimateDamage( DamageInfoInput& damageInfo ) const;
	void kill();																			///< do max health amount of kill damage to object
	void healCompletely();														///< Restore max health to this Object

	void scoreTheKill( const Object *victim );						///< I just killed this object.  
	void onVeterancyLevelChanged( VeterancyLevel oldLevel, VeterancyLevel newLevel );	///< I just achieved this level right this moment
	ExperienceTracker* getExperienceTracker() {return m_experienceTracker;}
	const ExperienceTracker* getExperienceTracker() const {return m_experienceTracker;}
	VeterancyLevel getVeterancyLevel() const;

	inline const AsciiString& getName() const { return m_name; }
	inline void setName( const AsciiString& newName ) { m_name = newName; }

	inline Team* getTeam() { return m_team; }
	inline const Team *getTeam() const { return m_team; }

	void restoreOriginalTeam();

	void setTeam( Team* team );						///< sets the unit's team AND original team
	void setTemporaryTeam( Team* team );	///< sets the unit's team BUT NOT its original team

	Player* getControllingPlayer() const;
	Relationship getRelationship(const Object *that) const;

	Color getIndicatorColor() const;
	Color getNightIndicatorColor() const;
	Bool hasCustomIndicatorColor() const { return m_indicatorColor != 0; }
	void setCustomIndicatorColor(Color c);
	void removeCustomIndicatorColor();

	Bool isLocallyControlled() const;
	Bool isNeutralControlled() const;
	
	Bool getIsUndetectedDefector(void) const { return BitIsSet(m_privateStatus, UNDETECTED_DEFECTOR); }
	void friend_setUndetectedDefector(Bool status);

	inline Bool isOffMap() const { return BitIsSet(m_privateStatus, OFF_MAP); }

	inline Bool isCaptured() const { return BitIsSet(m_privateStatus, CAPTURED); }
	void setCaptured(Bool isCaptured);

	inline const GeometryInfo& getGeometryInfo() const { return m_geometryInfo; }
	void setGeometryInfo(const GeometryInfo& geom);
	void setGeometryInfoZ( Real newZ );

	void onCollide( Object *other, const Coord3D *loc, const Coord3D *normal );
	
	// access to modules
	//-----------------------------------------------------------------------------
	
	//This is a good creation inspector. There's been multitudes of issues with conflicts of
	//Objects getting constructed causing crashes either because the modules aren't created
	//yet, and there's stuff being done inside of setTeam() that cares.
	Bool areModulesReady() const { return m_modulesReady; }

	BehaviorModule** getBehaviorModules() const { return m_behaviors; }

	BodyModuleInterface* getBodyModule() const { return m_body; }
	ContainModuleInterface* getContain() const { return m_contain; }
	StealthUpdate* getStealth() const { return m_stealth; }
	SpawnBehaviorInterface* getSpawnBehaviorInterface() const;


	// special case for the AIUpdateInterface, since it will be referred to a great deal
	inline AIUpdateInterface *getAIUpdateInterface() { return m_ai; }
	inline const AIUpdateInterface* getAIUpdateInterface() const { return m_ai; }

	inline AIUpdateInterface *getAI() { return m_ai; }
	inline const AIUpdateInterface* getAI() const { return m_ai; }

	inline PhysicsBehavior* getPhysics() { return m_physics; }
	inline const PhysicsBehavior* getPhysics() const { return m_physics; }
	void topple( const Coord3D *toppleDirection, Real toppleSpeed, UnsignedInt options );

	UpdateModule* findUpdateModule(NameKeyType key) const { return (UpdateModule*)findModule(key); }
	DamageModule* findDamageModule(NameKeyType key) const { return (DamageModule*)findModule(key); }

	Bool isSalvageCrate() const;

	//
	// Find us our production update interface if we have one.  This method exists simply
	// because we do this in a lot of places in the code and I want a convenient way to get thsi (CBD)
	//
	ProductionUpdateInterface* getProductionUpdateInterface( void );

	// 
	// Find us our dock update interface if we have one.  Again, this method exists simple
	// because we want to do this in a lot of places throughout the code
	//
	DockUpdateInterface *getDockUpdateInterface( void );

	// Ditto for special powers -- Kris
	SpecialPowerModuleInterface* findSpecialPowerModuleInterface( SpecialPowerType type ) const;
	SpecialPowerModuleInterface* findAnyShortcutSpecialPowerModuleInterface() const;
	SpecialAbilityUpdate* findSpecialAbilityUpdate( SpecialPowerType type ) const;
	SpecialPowerCompletionDie* findSpecialPowerCompletionDie() const;
	SpecialPowerUpdateInterface* findSpecialPowerWithOverridableDestinationActive( SpecialPowerType type = SPECIAL_INVALID ) const;
	SpecialPowerUpdateInterface* findSpecialPowerWithOverridableDestination( SpecialPowerType type = SPECIAL_INVALID ) const;

	inline ObjectStatusMaskType getStatusBits() const { return m_status; }
	inline Bool testStatus( ObjectStatusTypes bit ) const { return m_status.test( bit ); }
	void setStatus( ObjectStatusMaskType objectStatus, Bool set = true );
	inline void clearStatus( ObjectStatusMaskType objectStatus ) { setStatus( objectStatus, false ); }
	void updateUpgradeModules();	///< We need to go through our Upgrade Modules and see which should be activated
	UpgradeMaskType getObjectCompletedUpgradeMask() const { return m_objectUpgradesCompleted; } ///< Upgrades I complete locally

	//This function sucks.
	//It was added for objects that can disguise as other objects and contain upgraded subobject overrides. 
	//A concrete example is the bomb truck. Different payloads are displayed based on which upgrades have been
	//made. When the bomb truck disguises as something else, these subobjects are lost because the vector is
	//stored in W3DDrawModule. When we revert back to the original bomb truck, we call this function to 
	//recalculate those upgraded subobjects.
	void forceRefreshSubObjectUpgradeStatus();

	// Useful for status bits that can be set by the scripting system
	inline Bool testScriptStatusBit(ObjectScriptStatusBit b) const { return BitIsSet(m_scriptStatus, b); }
	void setScriptStatus( ObjectScriptStatusBit bit, Bool set = true );
	inline void clearScriptStatus( ObjectScriptStatusBit bit ) { setScriptStatus(bit, false); }

	// Selectable is individually controlled on an object by object basis for design now.
	// It defaults to the thingTemplate->isKindof(KINDOF_SELECTABLE), however, it can be overridden on an 
	// object by object basis.  Finally, it can be temporarily overriden by the OBJECT_STATUS_UNSELECTABLE. 
	// jba.
	void setSelectable(Bool selectable);
	Bool isSelectable() const;
	
	Bool isMassSelectable() const;

	// User specified formation.
	void setFormationID(enum FormationID id) {m_formationID = id;}
	enum FormationID getFormationID(void) const {return m_formationID;}
	void setFormationOffset(const Coord2D& offset) {m_formationOffset = offset;}
	void getFormationOffset(Coord2D* offset) const {*offset = m_formationOffset;}
		

//THIS FUNCTION BELONGS AT THE OBJECT LEVEL BECAUSE THERE IS AT LEAST ONE SPECIAL UNIT
//(ANGRY MOB) WHICH NEEDS LOGIC-SIDE POSITION CALC'S...
//IT WOULD PROBABLY BE WISE TO MOVE ALL THE HARD-CODED DEFAULTS BELOW
//INTO A new Drawable::getHealthBox..() WHICH USES GEOM0INFO, MODEL DATA, INI DATA, ETC.
	void getHealthBoxPosition(Coord3D& pos) const;
	Bool getHealthBoxDimensions(Real &healthBoxHeight, Real &healthBoxWidth) const;

	inline Bool isEffectivelyDead() const { return (m_privateStatus & EFFECTIVELY_DEAD) != 0; }
	void setEffectivelyDead(Bool dead);

	void markSingleUseCommandUsed() { m_singleUseCommandUsed = true; }
	Bool hasSingleUseCommandBeenUsed() const { return m_singleUseCommandUsed; }
	
	/// returns true iff the object can run over the other object. 
	Bool canCrushOrSquish(Object *otherObj, CrushSquishTestType testType = TEST_CRUSH_OR_SQUISH) const;
	UnsignedByte getCrusherLevel() const;
	UnsignedByte getCrushableLevel() const;

	Bool hasUpgrade( const UpgradeTemplate *upgradeT ) const ;			///< does this object already have this upgrade
	Bool affectedByUpgrade( const UpgradeTemplate *upgradeT ) const ; ///< can the object even "have" this upgrade, will it do something?
	void giveUpgrade( const UpgradeTemplate *upgradeT );		///< give upgrade to this object
	void removeUpgrade( const UpgradeTemplate *upgradeT );	///< remove upgrade from this object

	void calcNaturalRallyPoint(Coord2D *pt); ///< calc the "natural" starting rally point
  void setConstructionPercent( Real percent ) { m_constructionPercent = percent; }
	Real getConstructionPercent() const { return m_constructionPercent; }

  void setLayer( PathfindLayerEnum layer );
	PathfindLayerEnum getLayer() const { return m_layer; }

  void setDestinationLayer( PathfindLayerEnum layer );
	PathfindLayerEnum getDestinationLayer() const { return m_destinationLayer; }

	void prependToList(Object **pListHead);
	void removeFromList(Object **pListHead);
	Bool isInList(Object **pListHead) const;

	// this is intended for use ONLY by GameLogic.
	static void friend_deleteInstance(Object* object) { deleteInstance(object); }

	/// cache the partition module (should be called only by PartitionData)
	void friend_setPartitionData(PartitionData *pd) { m_partitionData = pd; }
	PartitionData *friend_getPartitionData() const { return m_partitionData; }
	const PartitionData *friend_getConstPartitionData() const { return m_partitionData; }

	void onPartitionCellChange();///< We have moved a 'significant' amount, so do maintenence that can be considered 'cell-based'
	void handlePartitionCellMaintenance();					///< Undo and redo all shroud actions.  Call when something has changed, like position or ownership or Death

	Real getVisionRange() const;				///< How far can you see?  This is dynamic so it is in Object.
	void setVisionRange( Real newVisionRange );	///< Access to setting someone's Vision distance
	Real getShroudRange() const;				///< How far can you shroud?  Even more dynamic since it'll start at zero for everyone.
	void setShroudRange( Real newShroudRange );	///< Access to setting someone's shrouding distance
	Real getShroudClearingRange() const;				///< How far do you clear shroud?
	void setShroudClearingRange( Real newShroudClearingRange );	///< Access to setting someone's clear shroud distance


	// Both of these calls are intended to only be used by TerrainLogic, specifically setActiveBoundary()
	void friend_prepareForMapBoundaryAdjust(void);
	void friend_notifyOfNewMapBoundary(void);

	// data for the radar
	void friend_setRadarData( RadarObject *rd ) { m_radarData = rd; }
	RadarObject *friend_getRadarData() { return m_radarData; }
	RadarPriorityType getRadarPriority() const;

	// contained-by
	inline Object *getContainedBy() { return m_containedBy; }
	inline const Object *getContainedBy() const { return m_containedBy; }
	inline UnsignedInt getContainedByFrame() const { return m_containedByFrame; }
	inline Bool isContained() const { return m_containedBy != NULL; }
	void onContainedBy( Object *containedBy );
	void onRemovedFrom( Object *removedFrom );
	Int getTransportSlotCount() const;
	void friend_setContainedBy( Object *containedBy ) { m_containedBy = containedBy; }

	// Special Powers -------------------------------------------------------------------------------
	SpecialPowerModuleInterface *getSpecialPowerModule( const SpecialPowerTemplate *specialPowerTemplate ) const;
	void doSpecialPower( const SpecialPowerTemplate *specialPowerTemplate, UnsignedInt commandOptions, Bool forced = false );	///< execute power
	void doSpecialPowerAtObject( const SpecialPowerTemplate *specialPowerTemplate, Object *obj, UnsignedInt commandOptions, Bool forced = false );	///< execute power
	void doSpecialPowerAtLocation( const SpecialPowerTemplate *specialPowerTemplate, const Coord3D *loc, Real angle, UnsignedInt commandOptions, Bool forced = false );	///< execute power
	void doSpecialPowerUsingWaypoints( const SpecialPowerTemplate *specialPowerTemplate, const Waypoint *way, UnsignedInt commandOptions, Bool forced = false );	///< execute power

	void doCommandButton( const CommandButton *commandButton, CommandSourceType cmdSource );
	void doCommandButtonAtObject( const CommandButton *commandButton, Object *obj, CommandSourceType cmdSource );
	void doCommandButtonAtPosition( const CommandButton *commandButton, const Coord3D *pos, CommandSourceType cmdSource );
	
	/**
		 For Object specific dynamic command sets.  Different from the Science specific ones handled in ThingTemplate
	*/
	const AsciiString& getCommandSetString() const;
	void setCommandSetStringOverride( AsciiString newCommandSetString ) { m_commandSetStringOverride = newCommandSetString; }

 	/// People are faking their commandsets, and, Suprise!, they are authoritative.  Challenge everything.
 	Bool canProduceUpgrade( const UpgradeTemplate *upgrade );

	// Weapons & Damage -------------------------------------------------------------------------------------------------
	void reloadAllAmmo(Bool now);
	Bool isOutOfAmmo() const;
	Bool hasAnyWeapon() const;
	Bool hasAnyDamageWeapon() const; //Kris: a should be used for real weapons that directly inflict damage... not deploy, hack, etc.
	Bool hasWeaponToDealDamageType(DamageType typeToDeal) const;
	Real getLargestWeaponRange() const;

	Weapon* getWeaponInWeaponSlot(WeaponSlotType wslot) const { return m_weaponSet.getWeaponInWeaponSlot(wslot); }

	// see if this current weapon set's weapons has shared reload times
	const Bool isReloadTimeShared() const { return m_weaponSet.isSharedReloadTime(); }

	Weapon* getCurrentWeapon(WeaponSlotType* wslot = NULL);
	const Weapon* getCurrentWeapon(WeaponSlotType* wslot = NULL) const;
	void setFiringConditionForCurrentWeapon() const;
	void adjustModelConditionForWeaponStatus();	///< Check to see if I should change my model condition. 
	void fireCurrentWeapon(Object *target);
	void fireCurrentWeapon(const Coord3D* pos);
	void preFireCurrentWeapon( const Object *victim );
	UnsignedInt getLastShotFiredFrame() const;					///< Get the frame a shot was last fired on
	ObjectID getLastVictimID() const;						///< Get the last victim we shot at
	Weapon* findWaypointFollowingCapableWeapon();
	Bool getAmmoPipShowingInfo(Int& numTotal, Int& numFull) const;

	/**
		Determines if the unit has any weapon that could conceivably
		harm the victim. this does not take range, ammo, etc. into 
		account, but immutable weapon properties, such as "can you
		target airborne victims".
	*/
	/*
		NOTE: getAbleToAttackSpecificObject NO LONGER internally calls isAbleToAttack(), 
		since that isn't an incredibly fast call, and this is called repeatedly in some inner loops
		where we already know that isAbleToAttack() == true. so you should always
		call isAbleToAttack prior to calling this! (srj)
	*/
	CanAttackResult getAbleToAttackSpecificObject( AbleToAttackType t, const Object* target, CommandSourceType commandSource ) const;

	//Used for base defenses and otherwise stationary units to see if you can attack a position potentially out of range.
	CanAttackResult getAbleToUseWeaponAgainstTarget( AbleToAttackType attackType, const Object *victim, const Coord3D *pos, CommandSourceType commandSource ) const;

	/**
		Selects the best weapon for the given target, and sets it as the current weapon.
		If there is no weapon that can damage the target, false is returned (and the current-weapon is unchanged).
		Note that this DOES take weapon attack range into account.
	*/
	Bool chooseBestWeaponForTarget(const Object* target, WeaponChoiceCriteria criteria, CommandSourceType cmdSource);

	// set and/or clear a single modelcondition flag
	void setModelConditionState( ModelConditionFlagType a );
	void clearModelConditionState( ModelConditionFlagType a );
	void clearAndSetModelConditionState( ModelConditionFlagType clr, ModelConditionFlagType set );

	//Special model states are states that are turned on for a period of time, and turned off
	//automatically -- used for cheer, and scripted special moment animations. Setting a special
	//state will automatically clear any other special states that may be turned on so you can only
	//have one at a time.
	void setSpecialModelConditionState( ModelConditionFlagType set, UnsignedInt frames = 0 );
	void clearSpecialModelConditionStates();

	// set and/or clear multiple modelcondition flags
	void clearModelConditionFlags( const ModelConditionFlags& clr );
	void setModelConditionFlags( const ModelConditionFlags& set );
	void clearAndSetModelConditionFlags( const ModelConditionFlags& clr, const ModelConditionFlags& set );

	void setWeaponSetFlag(WeaponSetType wst);
	void clearWeaponSetFlag(WeaponSetType wst);
	inline Bool testWeaponSetFlag(WeaponSetType wst) const { return m_curWeaponSetFlags.test(wst); }
	inline const WeaponSetFlags& getWeaponSetFlags() const { return m_curWeaponSetFlags; }
	Bool setWeaponLock( WeaponSlotType weaponSlot, WeaponLockType lockType ){ return m_weaponSet.setWeaponLock( weaponSlot, lockType ); }
	void releaseWeaponLock(WeaponLockType lockType){ m_weaponSet.releaseWeaponLock(lockType); }
	Bool isCurWeaponLocked() const { return m_weaponSet.isCurWeaponLocked(); }

	/// return true if the template has the specified special power flag set
	// @todo: inline
	Bool hasSpecialPower( SpecialPowerType type ) const;

	void setWeaponBonusCondition(WeaponBonusConditionType wst) { m_weaponBonusCondition |= (1 << wst); }
	void clearWeaponBonusCondition(WeaponBonusConditionType wst) { m_weaponBonusCondition &= ~(1 << wst); }
  // note, the !=0 at the end is important, to convert this into a boolean type! (srj)
	Bool testWeaponBonusCondition(WeaponBonusConditionType wst) const { return (m_weaponBonusCondition & (1 << wst)) != 0; }
	inline WeaponBonusConditionFlags getWeaponBonusCondition() const { return m_weaponBonusCondition; }

	Bool getSingleLogicalBonePosition(const char* boneName, Coord3D* position, Matrix3D* transform) const;
	Bool getSingleLogicalBonePositionOnTurret(WhichTurretType whichTurret, const char* boneName, Coord3D* position, Matrix3D* transform) const;
	Int getMultiLogicalBonePosition(const char* boneNamePrefix, Int maxBones, Coord3D* positions, Matrix3D* transforms, Bool convertToWorld = TRUE ) const;
	
	// Entered & exited.
	Bool didEnter(const PolygonTrigger *pTrigger) const;
	Bool didExit(const PolygonTrigger *pTrigger) const;
	Bool isInside(const PolygonTrigger *pTrigger) const;

	// exiting of any kind
	ExitInterface *getObjectExitInterface() const;  ///< get exit interface is present
	Bool hasExitInterface() const { return getObjectExitInterface() != 0; }

	ObjectShroudStatus getShroudedStatus(Int playerIndex) const;

	DisabledMaskType getDisabledFlags() const { return m_disabledMask; }
	Bool isDisabled() const { return m_disabledMask.any(); }
	Bool clearDisabled( DisabledType type );

	void setDisabled( DisabledType type );
	void setDisabledUntil( DisabledType type, UnsignedInt frame );
	Bool isDisabledByType( DisabledType type ) const { return TEST_DISABLEDMASK( m_disabledMask, type ); }

	void pauseAllSpecialPowers( const Bool disabling ) const;	
	
	//Checks any timers and clears disabled statii that have expired.
	void checkDisabledStatus();
	
	//When an AIAttackState is over, it needs to clean up any weapons that might be in leech range mode
	//or else those weapons will have unlimited range!
	void clearLeechRangeModeForAllWeapons();
	
	Int getNumConsecutiveShotsFiredAtTarget( const Object *victim) const;

	void setHealthBoxOffset( const Coord3D& offset ) { m_healthBoxOffset = offset; } ///< for special amorphous like angry mob

	void defect( Team *newTeam, UnsignedInt detectionTime );
	void goInvulnerable( UnsignedInt time );
	
	// This is public, since there is no Thing level master setting of Turret stuff.  It is all done in a sleepy hamlet
	// of a module called TurretAI.
	virtual void reactToTurretChange( WhichTurretType turret, Real oldRotation, Real oldPitch );

	// Convenience function for checking certain kindof bits
	Bool isStructure(void) const;
	
	// Convenience function for checking certain kindof bits
	Bool isFactionStructure(void) const;

	// Convenience function for checking certain kindof bits
	Bool isNonFactionStructure(void) const;

	Bool getReceivingDifficultyBonus() const { return m_isReceivingDifficultyBonus; }
	void setReceivingDifficultyBonus(Bool receive);

	inline UnsignedInt getSafeOcclusionFrame(void) { return m_safeOcclusionFrame; }	//< this is an object specific frame at which it's safe to enable building occlusion.
	inline void	setSafeOcclusionFrame(UnsignedInt frame) { m_safeOcclusionFrame = frame;} 

	// All of our cheating for radars and power go here.
	// This is the function that we now call in becomingTeamMember to adjust our power.
	// If incoming is true, we're working on the incoming player, if its false, we're on the outgoing
	// player. These are friend_s for player.
	void friend_adjustPowerForPlayer( Bool incoming );

	Bool isHero() const;

protected:

	void setOrRestoreTeam( Team* team, Bool restoring );

	void onDisabledEdge(Bool becomingDisabled);
	// All of our cheating for radars and power go here.


	// snapshot methods
	void crc( Xfer *xfer );
	void xfer( Xfer *xfer );
	void loadPostProcess();

	void handleShroud();
	void handleValueMap();
	void handleThreatMap();

	// NOTE NOTE NOTE -- this is a private method. Do Not Ever Make It Public.
	// If you think you need to make it public, you are wrong. Don't do it.
	// It will go away someday. Yeah, right. Just like GlobalData.
	Module* findModule(NameKeyType key) const;

	Bool didEnterOrExit() const;

	void setID( ObjectID id );
	virtual Object *asObjectMeth() { return this; }
	virtual const Object *asObjectMeth() const { return this; }

	virtual Real calculateHeightAboveTerrain(void) const;		// Calculates the actual height above terrain.  Doesn't use cache.

	void updateTriggerAreaFlags(void);
	void setTriggerAreaFlagsForChangeInPosition(void);
	
	/// Look and unlook are protected.  They should be called from Object::reasonToLook.  Like Capture, or death.
	void look();
	void unlook();
	void shroud();
	void unshroud();

	/// value and threat functions are protected, and should only be called from handleValueMap
	void addValue();
	void removeValue();

	void addThreat();
	void removeThreat();

	virtual void reactToTransformChange(const Matrix3D* oldMtx, const Coord3D* oldPos, Real oldAngle);

private:

	// yes, private. No, really. Private. Don't expose.
	enum ObjectPrivateStatusBits
	{
		EFFECTIVELY_DEAD		= (1 << 0),		///< Object is effectively dead
		UNDETECTED_DEFECTOR	= (1 << 1),		///< set to true when I defect from my team; set to false when I attack anything or when time runs out
		CAPTURED						= (1 << 2),		///< set to true if I've been captured, otherwise, its false. (Note: Never becomes false once it's true)
		OFF_MAP							= (1 << 3)		///< set to true if I am known to be OFF the current map.
		// NOTE: Object currently only uses a Byte for this, so if you add status bits, you may need to enlarge that field.
	};

	ObjectID			m_id;												///< this object's unique ID
	ObjectID			m_producerID;								///< object that produced us, if any
	ObjectID			m_builderID;								///< object that is building or has built us (dozers or workers are builders)
	Drawable*			m_drawable;									///< drawable (if any) for this object
	AsciiString		m_name;										///< internal name

	Object *			m_next;
	Object *			m_prev;
	ObjectStatusMaskType		m_status;									///< status bits (see ObjectStatusMaskType)

	GeometryInfo	m_geometryInfo;

	AIGroup*			m_group;								///< if non-NULL, we are part of this group of agents

	// These will last for my lifetime.  I will reuse them and reset them.  The truly dynamic ones are in PartitionManager
	SightingInfo	*m_partitionLastLook;		///< Where and for whom I last looked, so I can undo its effects when I stop
	SightingInfo	*m_partitionLastShroud;	///< Where and for whom I last shrouded, so I can undo its effects when I stop
	SightingInfo	*m_partitionLastThreat;	///< Where and for whom I last delt with threat, so I can undo its effects when I stop
	SightingInfo	*m_partitionLastValue;	///< Where and for whom I last delt with value, so I can undo its effects when I stop

	Real					m_visionRange;										///< looking range
	Real					m_shroudClearingRange;						///< looking range for shroud ONLY
	Real					m_shroudRange;										///< like looking range, this is how far I shroud others' looks

	DisabledMaskType	m_disabledMask;
	UnsignedInt				m_disabledTillFrame[ DISABLED_COUNT ];

	UnsignedInt		m_smcUntil;

	enum { NUM_SLEEP_HELPERS = 5 };
	ObjectRepulsorHelper*					m_repulsorHelper;
	ObjectSMCHelper*							m_smcHelper;
	ObjectWeaponStatusHelper*			m_wsHelper;
	ObjectDefectionHelper*				m_defectionHelper;
	FiringTracker*								m_firingTracker;	///< Tracker is really a "helper" and is included NUM_SLEEP_HELPERS

	// modules
	BehaviorModule**							m_behaviors;	// BehaviorModule, not BehaviorModuleInterface

	// cache these, for convenience
	ContainModuleInterface*				m_contain;
	BodyModuleInterface*					m_body;
	StealthUpdate*						m_stealth;

	AIUpdateInterface*						m_ai;	///< ai interface (if any), cached for handy access. (duplicate of entry in the module array!)
	PhysicsBehavior*							m_physics;	///< physics interface (if any), cached for handy access. (duplicate of entry in the module array!)

	PartitionData*								m_partitionData;	///< our PartitionData
	RadarObject*									m_radarData;				///< radar data
	ExperienceTracker*						m_experienceTracker;	///< Manages experience, gaining levels, and value when killed

	Object*												m_containedBy;					/**< an object can only be contained by at most one
																	other object, this is that object (if present) */
	ObjectID											m_xferContainedByID;	///< xfer uses IDs to store pointers and looks them up after
	UnsignedInt										m_containedByFrame;	///< frame we were contained by m_containedBy

	Real													m_constructionPercent;			///< for objects being built ... this is the amount completed (0.0 to 100.0)
	UpgradeMaskType								m_objectUpgradesCompleted;	///< Bit field of upgrades locally completed.

	Team*													m_team;								///< team that is current owner of this guy
	AsciiString										m_originalTeamName;		///< team that was the original ("birth") team of this guy
	Color													m_indicatorColor;			///< if nonzero, use this instead of controlling player's color

	Coord3D												m_healthBoxOffset; ///< generally zero, except for special amorphous ones like angry mob
	
	/// @todo srj -- convert to non-DLINK list, after it is once again possible to test the change
	MAKE_DLINK(Object, TeamMemberList)			///< other Things that are members of the same team

	// Weapons & Damage -------------------------------------------------------------------------------------------------
	WeaponSet											m_weaponSet;
	WeaponSetFlags								m_curWeaponSetFlags;
	WeaponBonusConditionFlags			m_weaponBonusCondition;
	Byte													m_lastWeaponCondition[WEAPONSLOT_COUNT];

	SpecialPowerMaskType					m_specialPowerBits; ///< bits determining what kind of special abilities this object has access to.

	//////////////////////////////////////< for the non-stacking healers like ambulance and propaganda
	ObjectID m_soleHealingBenefactorID; ///< who is the only other object that can give me this non-stacking heal benefit?
	UnsignedInt m_soleHealingBenefactorExpirationFrame; ///< on what frame can I accept healing (thus to switch) from a new benefactor
	///////////////////////////////////

	// Entered & exited housekeeping.
	enum { MAX_TRIGGER_AREA_INFOS = 5 };
	TTriggerInfo									m_triggerInfo[MAX_TRIGGER_AREA_INFOS];
	UnsignedInt										m_enteredOrExitedFrame;
	ICoord3D											m_iPos;
	
	PathfindLayerEnum							m_layer;							// Layer object is pathing on.
	PathfindLayerEnum							m_destinationLayer;		// Layer of current path goal.

	// User formations.
	FormationID										m_formationID;
	Coord2D												m_formationOffset;

	AsciiString										m_commandSetStringOverride;///< To allow specific object to switch command sets
	
	UnsignedInt										m_safeOcclusionFrame;	///<flag used by occlusion renderer so it knows when objects have exited their production building.

	// --------- BYTE-SIZED THINGS GO HERE
	Bool													m_isSelectable;
	Bool													m_modulesReady;
#if defined(RTS_DEBUG) || defined(RTS_INTERNAL)
	Bool													m_hasDiedAlready;
#endif
	UnsignedByte									m_scriptStatus;					///< status as set by scripting, corresponds to ORed ObjectScriptStatusBits
	UnsignedByte									m_privateStatus;					///< status bits that are never directly accessible to outside world
	Byte													m_numTriggerAreasActive;
	Bool													m_singleUseCommandUsed;
	Bool													m_isReceivingDifficultyBonus;

};  // end class Object

// describe an object as an AsciiString: e.g. "Object 102 (KillerBuggy) [GLARocketBuggy, owned by player 2 (GLAIntroPlayer)]"
AsciiString DebugDescribeObject(const Object *obj);

#if defined(RTS_DEBUG) || defined(RTS_INTERNAL)
	#define DEBUG_OBJECT_ID_EXISTS
#else
	#undef DEBUG_OBJECT_ID_EXISTS
#endif

#ifdef DEBUG_OBJECT_ID_EXISTS
extern ObjectID TheObjectIDToDebug;
#endif

#endif // _OBJECT_H_
