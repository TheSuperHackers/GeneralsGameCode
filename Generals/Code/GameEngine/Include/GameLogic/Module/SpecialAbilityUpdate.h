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

// FILE: SpecialAbilityUpdate.h /////////////////////////////////////////////////////////////////////////
// Author: Kris Morness, July 2002
// Desc:   Handles processing of unit special abilities.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/AudioEventRTS.h"
#include "Common/INI.h"
#include "GameLogic/Module/SpecialPowerUpdateModule.h"
#include "GameClient/ParticleSys.h"

class DamageInfo;
class SpecialPowerTemplate;
class SpecialPowerModule;
class FXList;
enum SpecialPowerType CPP_11(: Int);

#define SPECIAL_ABILITY_HUGE_DISTANCE 10000000.0f

//-------------------------------------------------------------------------------------------------
class SpecialAbilityUpdateModuleData : public UpdateModuleData
{
public:
	AsciiString						m_specialObjectName;
	AsciiString						m_specialObjectAttachToBoneName;
	const SpecialPowerTemplate*	m_specialPowerTemplate;		///< pointer to the special power template
	Real									m_startAbilityRange;
	Real									m_abilityAbortRange;
	Real									m_packUnpackVariationFactor;
	Real									m_fleeRangeAfterCompletion;
	Int										m_effectValue;
	Int										m_awardXPForTriggering;
	Int										m_skillPointsForTriggering;
	UnsignedInt						m_preparationFrames;
	UnsignedInt						m_persistentPrepFrames;
	UnsignedInt						m_effectDuration;
	UnsignedInt						m_maxSpecialObjects;
	UnsignedInt						m_packTime;
	UnsignedInt						m_unpackTime;
	UnsignedInt						m_preTriggerUnstealthFrames;
	Bool									m_skipPackingWithNoTarget;
	Bool									m_specialObjectsPersistent;
	Bool									m_uniqueSpecialObjectTargets;
	Bool									m_specialObjectsPersistWhenOwnerDies;
	Bool									m_flipObjectAfterPacking;
	Bool									m_flipObjectAfterUnpacking;
	Bool									m_alwaysValidateSpecialObjects;
	Bool									m_doCaptureFX;					///< the house color flashing while a building is getting captured
	Bool									m_loseStealthOnTrigger;
	Bool									m_approachRequiresLOS;
	const ParticleSystemTemplate *m_disableFXParticleSystem;
	AudioEventRTS					m_packSound;
	AudioEventRTS					m_unpackSound;
	AudioEventRTS					m_prepSoundLoop;
	AudioEventRTS					m_triggerSound;

	SpecialAbilityUpdateModuleData()
	{
		m_specialPowerTemplate = NULL;
		m_startAbilityRange = SPECIAL_ABILITY_HUGE_DISTANCE;
		m_abilityAbortRange = SPECIAL_ABILITY_HUGE_DISTANCE;
		m_preparationFrames = 0;
		m_persistentPrepFrames = 0;
		m_packTime = 0;
		m_unpackTime = 0;
		m_packUnpackVariationFactor = 0.0f;
		m_effectDuration = 0;
		m_maxSpecialObjects = 1;
		m_effectValue = 1;
		m_specialObjectsPersistent = FALSE;
		m_uniqueSpecialObjectTargets = FALSE;
		m_specialObjectsPersistWhenOwnerDies = FALSE;
		m_skipPackingWithNoTarget = FALSE;
		m_flipObjectAfterPacking = FALSE;
		m_flipObjectAfterUnpacking = FALSE;
		m_disableFXParticleSystem = NULL;
		m_fleeRangeAfterCompletion = 0.0f;
		m_doCaptureFX = FALSE;
		m_alwaysValidateSpecialObjects = FALSE;
		m_loseStealthOnTrigger = FALSE;
		m_awardXPForTriggering = 0;
		m_skillPointsForTriggering = -1;
		m_approachRequiresLOS = TRUE;
		m_preTriggerUnstealthFrames = 0;
	}

	static void buildFieldParse(MultiIniFieldParse& p)
	{
    UpdateModuleData::buildFieldParse(p);

		static const FieldParse dataFieldParse[] =
		{
			//Primary data values
			{ "SpecialPowerTemplate",				INI::parseSpecialPowerTemplate,		NULL, offsetof( SpecialAbilityUpdateModuleData, m_specialPowerTemplate ) },
			{ "StartAbilityRange",					INI::parseReal,										NULL, offsetof( SpecialAbilityUpdateModuleData, m_startAbilityRange ) },
			{ "AbilityAbortRange",					INI::parseReal,										NULL, offsetof( SpecialAbilityUpdateModuleData, m_abilityAbortRange ) },
			{ "PreparationTime",						INI::parseDurationUnsignedInt,		NULL, offsetof( SpecialAbilityUpdateModuleData, m_preparationFrames ) },
			{ "PersistentPrepTime",					INI::parseDurationUnsignedInt,		NULL, offsetof( SpecialAbilityUpdateModuleData, m_persistentPrepFrames ) },
			{ "PackTime",										INI::parseDurationUnsignedInt,		NULL, offsetof( SpecialAbilityUpdateModuleData, m_packTime ) },
			{ "UnpackTime",									INI::parseDurationUnsignedInt,		NULL, offsetof( SpecialAbilityUpdateModuleData, m_unpackTime ) },
			{ "PreTriggerUnstealthTime",	  INI::parseDurationUnsignedInt,		NULL, offsetof( SpecialAbilityUpdateModuleData, m_preTriggerUnstealthFrames ) },
			{ "SkipPackingWithNoTarget",		INI::parseBool,										NULL, offsetof( SpecialAbilityUpdateModuleData, m_skipPackingWithNoTarget ) },
			{ "PackUnpackVariationFactor",	INI::parseReal,										NULL, offsetof( SpecialAbilityUpdateModuleData, m_packUnpackVariationFactor ) },

			//Secondary data values
			{ "SpecialObject",							INI::parseAsciiString,						NULL, offsetof( SpecialAbilityUpdateModuleData, m_specialObjectName ) },
			{ "SpecialObjectAttachToBone",	INI::parseAsciiString,						NULL, offsetof( SpecialAbilityUpdateModuleData, m_specialObjectAttachToBoneName ) },
			{ "MaxSpecialObjects",					INI::parseUnsignedInt,						NULL, offsetof( SpecialAbilityUpdateModuleData, m_maxSpecialObjects ) },
			{ "SpecialObjectsPersistent",		INI::parseBool,										NULL, offsetof( SpecialAbilityUpdateModuleData, m_specialObjectsPersistent ) },
			{ "EffectDuration",							INI::parseDurationUnsignedInt,		NULL, offsetof( SpecialAbilityUpdateModuleData, m_effectDuration ) },
			{ "EffectValue",								INI::parseInt,										NULL, offsetof( SpecialAbilityUpdateModuleData, m_effectValue ) },
			{ "UniqueSpecialObjectTargets", INI::parseBool,										NULL, offsetof( SpecialAbilityUpdateModuleData, m_uniqueSpecialObjectTargets ) },
			{ "SpecialObjectsPersistWhenOwnerDies", INI::parseBool,						NULL, offsetof( SpecialAbilityUpdateModuleData, m_specialObjectsPersistWhenOwnerDies ) },
			{ "AlwaysValidateSpecialObjects",				INI::parseBool,						NULL, offsetof( SpecialAbilityUpdateModuleData, m_alwaysValidateSpecialObjects ) },
			{ "FlipOwnerAfterPacking",			INI::parseBool,										NULL, offsetof( SpecialAbilityUpdateModuleData, m_flipObjectAfterPacking ) },
			{ "FlipOwnerAfterUnpacking",		INI::parseBool,										NULL, offsetof( SpecialAbilityUpdateModuleData, m_flipObjectAfterUnpacking ) },
			{ "FleeRangeAfterCompletion",		INI::parseReal,										NULL, offsetof( SpecialAbilityUpdateModuleData, m_fleeRangeAfterCompletion ) },
			{ "DisableFXParticleSystem",		INI::parseParticleSystemTemplate, NULL, offsetof( SpecialAbilityUpdateModuleData, m_disableFXParticleSystem ) },
			{ "DoCaptureFX",								INI::parseBool,										NULL, offsetof( SpecialAbilityUpdateModuleData, m_doCaptureFX ) },
			{ "PackSound",									INI::parseAudioEventRTS,					NULL, offsetof( SpecialAbilityUpdateModuleData, m_packSound ) },
			{ "UnpackSound",								INI::parseAudioEventRTS,					NULL, offsetof( SpecialAbilityUpdateModuleData, m_unpackSound ) },
			{ "PrepSoundLoop",							INI::parseAudioEventRTS,					NULL, offsetof( SpecialAbilityUpdateModuleData, m_prepSoundLoop ) },
			{ "TriggerSound",								INI::parseAudioEventRTS,					NULL, offsetof( SpecialAbilityUpdateModuleData, m_triggerSound ) },
			{ "LoseStealthOnTrigger",				INI::parseBool,										NULL, offsetof( SpecialAbilityUpdateModuleData, m_loseStealthOnTrigger ) },
			{ "AwardXPForTriggering",				INI::parseInt,										NULL, offsetof( SpecialAbilityUpdateModuleData, m_awardXPForTriggering ) },
			{ "SkillPointsForTriggering",		INI::parseInt,										NULL, offsetof( SpecialAbilityUpdateModuleData, m_skillPointsForTriggering ) },
			{ "ApproachRequiresLOS",				INI::parseBool,										NULL, offsetof( SpecialAbilityUpdateModuleData, m_approachRequiresLOS ) },
			{ 0, 0, 0, 0 }
		};
    p.add(dataFieldParse);
	}
};

//-------------------------------------------------------------------------------------------------
class SpecialAbilityUpdate : public UpdateModule, public SpecialPowerUpdateInterface
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( SpecialAbilityUpdate, "SpecialAbilityUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( SpecialAbilityUpdate, SpecialAbilityUpdateModuleData )

public:

	SpecialAbilityUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	// SpecialPowerUpdateInterface
	virtual Bool initiateIntentToDoSpecialPower(const SpecialPowerTemplate *specialPowerTemplate, const Object *targetObj, const Coord3D *targetPos, const Waypoint *way, UnsignedInt commandOptions );
	virtual Bool isSpecialAbility() const { return true; }
	virtual Bool isSpecialPower() const { return false; }
	virtual Bool isActive() const { return m_active; }
	virtual Bool doesSpecialPowerHaveOverridableDestinationActive() const { return false; } //Is it active now?
	virtual Bool doesSpecialPowerHaveOverridableDestination() const { return false; }	//Does it have it, even if it's not active?
	virtual void setSpecialPowerOverridableDestination( const Coord3D *loc ) {}
	virtual Bool isPowerCurrentlyInUse( const CommandButton *command = NULL ) const;

//	virtual Bool isBusy() const { return m_isBusy; }

	// UpdateModule
	virtual SpecialPowerUpdateInterface* getSpecialPowerUpdateInterface() { return this; }
	virtual CommandOption getCommandOption() const { return (CommandOption)0; }
	virtual UpdateSleepTime update();

	// ??? ugh, public stuff that shouldn't be -- hell yeah!
	UnsignedInt getSpecialObjectCount() const;
	UnsignedInt getSpecialObjectMax() const;
	Object* findSpecialObjectWithProducerID( const Object *target );
	SpecialPowerType getSpecialPowerType( void ) const;

protected:
	void onExit( Bool cleanup );

	const SpecialPowerTemplate* getTemplate() const;

	SpecialPowerModuleInterface* getMySPM();

	//Wrapper function that determines if our object is close enough to the target object or position (auto knowledge)... only if necessary.
	//Returns true if it's not necessary, in which case, it can skip any approach.
	Bool isWithinStartAbilityRange() const;
	Bool isWithinAbilityAbortRange() const; //If not, we abort the ability

	Bool initLaser(Object* specialObject, Object* target);

	//Various steps of performing any special ability. Not all special abilities will use all of them.
	Bool approachTarget();			//Approaches the target before starting the special attack (if appropriate)
	void startPreparation();		//Begins the preparation of ability -- like firing off a hacker attack, or laser tracer, etc.
	Bool continuePreparation();	//Updates the preparation of ability -- like recalculating tracer lines.
	void triggerAbilityEffect();	//After the preparation time has elapsed, this part actually triggers the desired effect of the special ability.
	void finishAbility();

	void validateSpecialObjects();

	Object* createSpecialObject();
	void killSpecialObjects();

	Bool handlePackingProcessing();
	void startPacking(Bool success);
	void startUnpacking();
	Bool needToPack() const;
	Bool needToUnpack() const;

	Bool isPreparationComplete() const { return !m_prepFrames; }
	void endPreparation();
	Bool isPersistentAbility() const { return getSpecialAbilityUpdateModuleData()->m_persistentPrepFrames > 0; }
	void resetPreparation() { m_prepFrames = getSpecialAbilityUpdateModuleData()->m_persistentPrepFrames; }

	Bool isFacing();
	Bool needToFace() const;
	void startFacing();

//	void setBusy ( Bool is ) { m_isBusy = is; }
//	Bool m_isBusy; ///< whether I am between trigger and completion

protected:

	UpdateSleepTime calcSleepTime()
	{
		return (m_active || getSpecialAbilityUpdateModuleData()->m_alwaysValidateSpecialObjects) ? UPDATE_SLEEP_NONE : UPDATE_SLEEP_FOREVER;
	}

private:

	enum PackingState
	{
		STATE_NONE,
		STATE_PACKING,
		STATE_UNPACKING,
		STATE_PACKED,
		STATE_UNPACKED,
	};

	AudioEventRTS									m_prepSoundLoop;
	UnsignedInt										m_prepFrames;
	UnsignedInt										m_animFrames;	//Used for packing/unpacking unit before or after using ability.
	ObjectID											m_targetID;
	Coord3D												m_targetPos;
	Int														m_locationCount;
	std::list<ObjectID>						m_specialObjectIDList; //The list of special objects
	UnsignedInt										m_specialObjectEntries;				 //The size of the list of member Objects
	Real													m_captureFlashPhase;    ///< used to track the accellerating flash of the capture FX
	PackingState									m_packingState;
	Bool													m_active;
	Bool													m_noTargetCommand;
	Bool													m_facingInitiated;
	Bool													m_facingComplete;
	Bool													m_withinStartAbilityRange;
	Bool													m_doDisableFXParticles;      // smaller targets cause this flag to toggle, making the particle effect more sparse
};
