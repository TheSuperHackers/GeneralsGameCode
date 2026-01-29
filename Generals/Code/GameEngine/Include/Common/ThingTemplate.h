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

// FILE: ThingTemplate.h //////////////////////////////////////////////////////////////////////////
// Author: Colin Day, April 2001
// Desc:	 Thing templates are a 'roadmap' to creating things
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Lib/BaseType.h"

#include "Common/AudioEventRTS.h"
#include "Common/FileSystem.h"
#include "Common/GameCommon.h"
#include "Common/Geometry.h"
#include "Common/KindOf.h"
#include "Common/ModuleFactory.h"
#include "Common/Overridable.h"
#include "Common/ProductionPrerequisite.h"
#include "Common/Science.h"
#include "Common/UnicodeString.h"

#include "GameLogic/ArmorSet.h"
#include "GameLogic/WeaponSet.h"
#include "Common/STLTypedefs.h"
#include "GameClient/Color.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class AIUpdateModuleData;
class Image;
class Object;
class Drawable;
class ProductionPrerequisite;
struct FieldParse;
class Player;
class INI;
enum RadarPriorityType CPP_11(: Int);
enum ScienceType CPP_11(: Int);
enum EditorSortingType CPP_11(: Int);
enum ShadowType CPP_11(: Int);
class WeaponTemplateSet;
class ArmorTemplateSet;
class FXList;

// TYPEDEFS FOR FILE //////////////////////////////////////////////////////////////////////////////
typedef std::map<AsciiString, AudioEventRTS> PerUnitSoundMap;
typedef std::map<AsciiString, const FXList*> PerUnitFXMap;

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//Code renderer handles these states now.
//enum InventoryImageType
//{
//	INV_IMAGE_ENABLED = 0,
//	INV_IMAGE_DISABLED,
//	INV_IMAGE_HILITE,
//	INV_IMAGE_PUSHED,
//
//	INV_IMAGE_NUM_IMAGES
//
//};
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
enum
{
	MAX_UPGRADE_CAMEO_UPGRADES = 5
};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
enum ThingTemplateAudioType CPP_11(: Int)
{
	TTAUDIO_voiceSelect,							///< Response when unit is selected
	TTAUDIO_voiceGroupSelect,					///< Response when a group of this unit is selected
	TTAUDIO_voiceSelectElite,					///< Response when unit is selected and elite
	TTAUDIO_voiceMove,								///< Response when unit moves
	TTAUDIO_voiceAttack,							///< Response when unit is told to attack
	TTAUDIO_voiceEnter,								///< Response when unit is told to enter a building
	TTAUDIO_voiceFear,								///< Response when unit is under attack
	TTAUDIO_voiceCreated,							///< Response when unit is created
	TTAUDIO_voiceNearEnemy,						///< Unit is near an enemy
	TTAUDIO_voiceTaskUnable,					///< Unit is told to do something impossible
	TTAUDIO_voiceTaskComplete,				///< Unit completes a move, or other task indicated
	TTAUDIO_voiceMeetEnemy,						///< Unit meets an enemy unit
	TTAUDIO_soundMoveStart,						///< Sound when unit starts moving
	TTAUDIO_soundMoveStartDamaged,		///< Sound when unit starts moving and is damaged
	TTAUDIO_soundMoveLoop,						///< Sound when unit is moving
	TTAUDIO_soundMoveLoopDamaged,			///< Sound when unit is moving and is damaged
	TTAUDIO_soundDie,									///< Sound when unit is dieing
	TTAUDIO_soundCrush,								///< Sound when unit is crushed
	TTAUDIO_soundAmbient,							///< Ambient sound for unit during normal status. Also the default sound
	TTAUDIO_soundAmbientDamaged,			///< Ambient sound for unit if damaged. Corresponds to body info damage
	TTAUDIO_soundAmbientReallyDamaged,///< Ambient sound for unit if badly damaged.
	TTAUDIO_soundAmbientRubble,				///< Ambient sound for unit if it is currently rubble. (Dam, for instance)
	TTAUDIO_soundStealthOn,           ///< Sound when unit stealths
	TTAUDIO_soundStealthOff,          ///< Sound when unit destealths
	TTAUDIO_soundCreated,							///< Sound when unit is created
	TTAUDIO_soundOnDamaged,           ///< Sound when unit enters damaged state
	TTAUDIO_soundOnReallyDamaged,     ///< Sound when unit enters really damaged state
	TTAUDIO_soundDieFire,							///< Sound when unit dies by fire. NOTE: Replaces soundDie if present and unit dies by fire.
	TTAUDIO_soundDieToxin,						///< Sound when unit dies by Toxin. NOTE: Replaces soundDie if present and unit dies by fire.
	TTAUDIO_soundEnter,								///< Sound when another unit enters me.
	TTAUDIO_soundExit,								///< Sound when another unit exits me.
	TTAUDIO_soundPromotedVeteran,			///< Sound when unit gets promoted to Veteran level
	TTAUDIO_soundPromotedElite,				///< Sound when unit gets promoted to Elite level
	TTAUDIO_soundPromotedHero,				///< Sound when unit gets promoted to Hero level
	TTAUDIO_voiceGarrison,						///< Unit is ordered to enter a garrisonable building
	TTAUDIO_soundFalling,							///< This sound is actually called on a unit when it is exiting another.
																		///< However, there is a soundExit which refers to the container, and this is only used for bombs falling from planes.
#ifdef ALLOW_SURRENDER
	TTAUDIO_voiceSurrender,						///< Unit surrenders
#endif
	TTAUDIO_voiceDefect,							///< Unit is forced to defect
	TTAUDIO_voiceAttackSpecial,				///< Unit is ordered to use a special attack
	TTAUDIO_voiceAttackAir,						///< Unit is ordered to attack an airborne unit
	TTAUDIO_voiceGuard,								///< Unit is ordered to guard an area

	TTAUDIO_COUNT
};

class AudioArray
{
public:
	DynamicAudioEventRTS* m_audio[TTAUDIO_COUNT];

	AudioArray()
	{
		for (Int i = 0; i < TTAUDIO_COUNT; ++i)
			m_audio[i] = nullptr;
	}

	~AudioArray()
	{
		for (Int i = 0; i < TTAUDIO_COUNT; ++i)
			deleteInstance(m_audio[i]);
	}

	AudioArray(const AudioArray& that)
	{
		for (Int i = 0; i < TTAUDIO_COUNT; ++i)
		{
			if (that.m_audio[i])
				m_audio[i] = newInstance(DynamicAudioEventRTS)(*that.m_audio[i]);
			else
				m_audio[i] = nullptr;
		}
	}

	AudioArray& operator=(const AudioArray& that)
	{
		if (this != &that)
		{
			for (Int i = 0; i < TTAUDIO_COUNT; ++i)
			{
				if (that.m_audio[i])
				{
					if (m_audio[i])
						*m_audio[i] = *that.m_audio[i];
					else
						m_audio[i] = newInstance(DynamicAudioEventRTS)(*that.m_audio[i]);
				}
				else
				{
					m_audio[i] = nullptr;
				}
			}
		}
		return *this;
	}
};

//-------------------------------------------------------------------------------------------------
/** Object class type enumeration */
//-------------------------------------------------------------------------------------------------
enum BuildCompletionType CPP_11(: Int)
{
	BC_INVALID = 0,
	BC_APPEARS_AT_RALLY_POINT,	///< unit appears at rally point of its #1 prereq
	BC_PLACED_BY_PLAYER,				///< unit must be manually placed by player

	BC_NUM_TYPES
};
#ifdef DEFINE_BUILD_COMPLETION_NAMES
static const char *const BuildCompletionNames[] =
{
	"INVALID",
	"APPEARS_AT_RALLY_POINT",
	"PLACED_BY_PLAYER",

	nullptr
};
static_assert(ARRAY_SIZE(BuildCompletionNames) == BC_NUM_TYPES + 1, "Incorrect array size");
#endif  // end DEFINE_BUILD_COMPLETION_NAMES

enum BuildableStatus CPP_11(: Int)
{
	// saved into savegames... do not change or remove values!
	BSTATUS_YES = 0,
	BSTATUS_IGNORE_PREREQUISITES,
	BSTATUS_NO,
	BSTATUS_ONLY_BY_AI,

	BSTATUS_NUM_TYPES
};

#ifdef DEFINE_BUILDABLE_STATUS_NAMES
static const char *const BuildableStatusNames[] =
{
	"Yes",
	"Ignore_Prerequisites",
	"No",
	"Only_By_AI",
	nullptr
};
static_assert(ARRAY_SIZE(BuildableStatusNames) == BSTATUS_NUM_TYPES + 1, "Incorrect array size");
#endif	// end DEFINE_BUILDABLE_STATUS_NAMES

//-------------------------------------------------------------------------------------------------
enum ModuleParseMode CPP_11(: Int)
{
	MODULEPARSE_NORMAL,
	MODULEPARSE_ADD_REMOVE_REPLACE,
	MODULEPARSE_INHERITABLE
};

//-------------------------------------------------------------------------------------------------
class ModuleInfo
{
private:
	struct Nugget
	{
		AsciiString first;
		AsciiString m_moduleTag;
		const ModuleData* second;
		Int interfaceMask;
		Bool copiedFromDefault;
		Bool inheritable;

		Nugget(const AsciiString& n, const AsciiString& moduleTag, const ModuleData* d, Int i, Bool inh)
		: first(n),
			m_moduleTag(moduleTag),
			second(d),
			interfaceMask(i),
			copiedFromDefault(false),
			inheritable(inh)
		{
		}

	};
	std::vector<Nugget> m_info;

public:

	ModuleInfo() { }

	void addModuleInfo(ThingTemplate *thingTemplate, const AsciiString& name, const AsciiString& moduleTag, const ModuleData* data, Int interfaceMask, Bool inheritable);
	const ModuleInfo::Nugget *getNuggetWithTag( const AsciiString& tag ) const;

	Int getCount() const
	{
		return m_info.size();
	}

#if defined(RTS_DEBUG)
	Bool containsPartialName(const char* n) const
	{
		for (size_t i = 0; i < m_info.size(); i++)
			if (strstr(m_info[i].first.str(), n) != nullptr)
				return true;
		return false;
	}
#endif

	AsciiString getNthName(size_t i) const
	{
		if (i >= 0 && i < m_info.size())
		{
			return m_info[i].first;
		}
		return AsciiString::TheEmptyString;
	}

	AsciiString getNthTag(size_t i) const
	{
		if (i >= 0 && i < m_info.size())
		{
			return m_info[i].m_moduleTag;
		}
		return AsciiString::TheEmptyString;
	}

	const ModuleData* getNthData(size_t i) const
	{
		if (i >= 0 && i < m_info.size())
		{
			return m_info[i].second;
		}
		return nullptr;
	}

	// for use only by ThingTemplate::friend_getAIModuleInfo
	ModuleData* friend_getNthData(Int i);

	void clear()
	{
		m_info.clear();
	}

	void setCopiedFromDefault(Bool v)
	{
		for (size_t i = 0; i < m_info.size(); i++)
			m_info[i].copiedFromDefault = v;
	}

	Bool clearModuleDataWithTag(const AsciiString& tagToClear, AsciiString& clearedModuleNameOut);
	Bool clearCopiedFromDefaultEntries(Int interfaceMask);
	Bool clearAiModuleInfo();
};

//-------------------------------------------------------------------------------------------------
/** Definition of a thing template to read from our game data framework */
//-------------------------------------------------------------------------------------------------
class ThingTemplate : public Overridable
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(ThingTemplate, "ThingTemplatePool" )

private:

#if defined(_MSC_VER) && _MSC_VER < 1300
	ThingTemplate(const ThingTemplate& that) : m_geometryInfo(that.m_geometryInfo)
	{
		DEBUG_CRASH(("This should never be called"));
	}
#else
	ThingTemplate(const ThingTemplate& that) = delete;
#endif

public:


	ThingTemplate();

	// copy the guts of that into this, but preserve this' name, id, and list-links.
	void copyFrom(const ThingTemplate* that);

	/// called by ThingFactory after all templates have been loaded.
	void resolveNames();

#ifdef LOAD_TEST_ASSETS
	void initForLTA(const AsciiString& name);
	inline AsciiString getLTAName() const { return m_LTAName; }
#endif

	/**
		return a unique identifier suitable for identifying this ThingTemplate on machines playing
		across the net. this should be considered a Magic Cookie and used only for net traffic or
		similar sorts of things. To convert an id back to a ThingTemplate, use ThingFactory::findByID().
		Note that 0 is always an invalid id.  NOTE that we are not referencing m_override here
		because even though we actually have multiple templates here representing overrides,
		we still only conceptually have one template and want to always use one single
		pointer for comparisons of templates.  However, even if we did reference m_override
		the IDs would be the same for each one since every override first *COPIES* data
		from the current/parent template data.
	*/
	UnsignedShort getTemplateID() const { return m_templateID; }

	// note that m_override is not used here, see getTemplateID(), for it is the same reasons
	const AsciiString& getName() const { return m_nameString; }  ///< return the name of this template

	/// get the display color (used for the editor)
	Color getDisplayColor() const { return m_displayColor; }

	/// get the editor sorting
	EditorSortingType getEditorSorting() const { return (EditorSortingType)m_editorSorting; }

	/// return true iff the template has the specified kindOf flag set.
	Bool isKindOf(KindOfType t) const
	{
		return TEST_KINDOFMASK(m_kindof, t);
	}

	/// convenience for doing multiple kindof testing at once.
	Bool isKindOfMulti(const KindOfMaskType& mustBeSet, const KindOfMaskType& mustBeClear) const
	{
		return TEST_KINDOFMASK_MULTI(m_kindof, mustBeSet, mustBeClear);
	}

	Bool isAnyKindOf( const KindOfMaskType& anyKindOf ) const
	{
		return TEST_KINDOFMASK_ANY(m_kindof, anyKindOf);
	}

	/// set the display name
	const UnicodeString& getDisplayName() const { return m_displayName; }  ///< return display name

	RadarPriorityType getDefaultRadarPriority() const { return (RadarPriorityType)m_radarPriority; }  ///< return radar priority from INI

	// note, you should not call this directly; rather, call Object::getTransportSlotCount().
	Int getRawTransportSlotCount() const { return m_transportSlotCount; }

	Real getFenceWidth() const { return m_fenceWidth; }  // return fence width

	Real getFenceXOffset() const { return m_fenceXOffset; }  // return fence offset

	Bool isBridge() const { return m_isBridge; }  // return fence offset
	Bool isBridgeLike() const { return isBridge() || isKindOf(KINDOF_WALK_ON_TOP_OF_WALL); }

	// Only Object can ask this.  Everyone else should ask the Object.  In fact, you really should ask the Object everything.
	Real friend_getVisionRange() const { return m_visionRange; }  ///< get vision range
	Real friend_getShroudClearingRange() const { return m_shroudClearingRange; }  ///< get vision range for Shroud ONLY (Design requested split)

	// This function is only for use by the AIUpdateModuleData::parseLocomotorSet function.
	AIUpdateModuleData *friend_getAIModuleInfo(void);

	ShadowType getShadowType() const { return (ShadowType)m_shadowType; }
	Real getShadowSizeX() const { return m_shadowSizeX; }
	Real getShadowSizeY() const { return m_shadowSizeY; }
	Real getShadowOffsetX() const { return m_shadowOffsetX; }
	Real getShadowOffsetY() const { return m_shadowOffsetY; }

	const AsciiString& getShadowTextureName( void ) const { return m_shadowTextureName; }
	UnsignedInt getOcclusionDelay(void) const { return m_occlusionDelay;}

	const ModuleInfo& getBehaviorModuleInfo() const { return m_behaviorModuleInfo; }
	const ModuleInfo& getDrawModuleInfo() const { return m_drawModuleInfo; }
	const ModuleInfo& getClientUpdateModuleInfo() const { return m_clientUpdateModuleInfo; }

	const Image *getSelectedPortraitImage( void ) const { return m_selectedPortraitImage; }
	const Image *getButtonImage( void ) const { return m_buttonImage; }

	//Code renderer handles these states now.
	//const AsciiString& getInventoryImageName( InventoryImageType type ) const { return m_inventoryImage[ type ]; }

	Int getSkillPointValue(Int level) const;

	Int getExperienceValue(Int level) const { return m_experienceValues[level]; }
	Int getExperienceRequired(Int level) const {return m_experienceRequired[level]; }
	Bool isTrainable() const{return m_isTrainable; }

	const AudioEventRTS *getVoiceSelect() const								{ return getAudio(TTAUDIO_voiceSelect); }
	const AudioEventRTS *getVoiceGroupSelect() const					{ return getAudio(TTAUDIO_voiceGroupSelect); }
	const AudioEventRTS *getVoiceMove() const									{ return getAudio(TTAUDIO_voiceMove); }
	const AudioEventRTS *getVoiceAttack() const								{ return getAudio(TTAUDIO_voiceAttack); }
	const AudioEventRTS *getVoiceEnter() const								{ return getAudio(TTAUDIO_voiceEnter); }
	const AudioEventRTS *getVoiceFear() const									{ return getAudio(TTAUDIO_voiceFear); }
	const AudioEventRTS *getVoiceSelectElite() const					{ return getAudio(TTAUDIO_voiceSelectElite); }
	const AudioEventRTS *getVoiceCreated() const							{ return getAudio(TTAUDIO_voiceCreated); }
	const AudioEventRTS *getVoiceNearEnemy() const						{ return getAudio(TTAUDIO_voiceNearEnemy); }
	const AudioEventRTS *getVoiceTaskUnable() const						{ return getAudio(TTAUDIO_voiceTaskUnable); }
	const AudioEventRTS *getVoiceTaskComplete() const					{ return getAudio(TTAUDIO_voiceTaskComplete); }
	const AudioEventRTS *getVoiceMeetEnemy() const						{ return getAudio(TTAUDIO_voiceMeetEnemy); }
	const AudioEventRTS *getVoiceGarrison() const							{ return getAudio(TTAUDIO_voiceGarrison); }
#ifdef ALLOW_SURRENDER
	const AudioEventRTS *getVoiceSurrender() const						{ return getAudio(TTAUDIO_voiceSurrender); }
#endif
	const AudioEventRTS *getVoiceDefect() const								{ return getAudio(TTAUDIO_voiceDefect); }
	const AudioEventRTS *getVoiceAttackSpecial() const				{ return getAudio(TTAUDIO_voiceAttackSpecial); }
	const AudioEventRTS *getVoiceAttackAir() const						{ return getAudio(TTAUDIO_voiceAttackAir); }
	const AudioEventRTS *getVoiceGuard() const								{ return getAudio(TTAUDIO_voiceGuard); }
	const AudioEventRTS *getSoundMoveStart() const						{ return getAudio(TTAUDIO_soundMoveStart); }
	const AudioEventRTS *getSoundMoveStartDamaged() const			{ return getAudio(TTAUDIO_soundMoveStartDamaged); }
	const AudioEventRTS *getSoundMoveLoop() const							{ return getAudio(TTAUDIO_soundMoveLoop); }
	const AudioEventRTS *getSoundMoveLoopDamaged() const			{ return getAudio(TTAUDIO_soundMoveLoopDamaged); }
	const AudioEventRTS *getSoundDie() const									{ return getAudio(TTAUDIO_soundDie); }
	const AudioEventRTS *getSoundCrush() const								{ return getAudio(TTAUDIO_soundCrush); }
	const AudioEventRTS *getSoundAmbient() const							{ return getAudio(TTAUDIO_soundAmbient); }
	const AudioEventRTS *getSoundAmbientDamaged() const				{ return getAudio(TTAUDIO_soundAmbientDamaged); }
	const AudioEventRTS *getSoundAmbientReallyDamaged() const	{ return getAudio(TTAUDIO_soundAmbientReallyDamaged); }
	const AudioEventRTS *getSoundAmbientRubble() const				{ return getAudio(TTAUDIO_soundAmbientRubble); }
	const AudioEventRTS *getSoundStealthOn() const						{ return getAudio(TTAUDIO_soundStealthOn); }
	const AudioEventRTS *getSoundStealthOff() const						{ return getAudio(TTAUDIO_soundStealthOff); }
	const AudioEventRTS *getSoundCreated() const							{ return getAudio(TTAUDIO_soundCreated); }
	const AudioEventRTS *getSoundOnDamaged() const						{ return getAudio(TTAUDIO_soundOnDamaged); }
	const AudioEventRTS *getSoundOnReallyDamaged() const			{ return getAudio(TTAUDIO_soundOnReallyDamaged); }
	const AudioEventRTS *getSoundDieFire() const							{ return getAudio(TTAUDIO_soundDieFire); }
	const AudioEventRTS *getSoundDieToxin() const							{ return getAudio(TTAUDIO_soundDieToxin); }
	const AudioEventRTS *getSoundEnter() const								{ return getAudio(TTAUDIO_soundEnter); }
	const AudioEventRTS *getSoundExit() const									{ return getAudio(TTAUDIO_soundExit); }
	const AudioEventRTS *getSoundPromotedVeteran() const			{ return getAudio(TTAUDIO_soundPromotedVeteran); }
	const AudioEventRTS *getSoundPromotedElite() const				{ return getAudio(TTAUDIO_soundPromotedElite); }
	const AudioEventRTS *getSoundPromotedHero() const					{ return getAudio(TTAUDIO_soundPromotedHero); }
	const AudioEventRTS *getSoundFalling() const							{ return getAudio(TTAUDIO_soundFalling); }

	const AudioEventRTS *getPerUnitSound(const AsciiString& soundName) const;
	const FXList* getPerUnitFX(const AsciiString& fxName) const;

	UnsignedInt getThreatValue() const								{ return m_threatValue; }
	UnsignedInt getMaxSimultaneousOfType() const			{ return m_maxSimultaneousOfType; }

	void validate();

// The version that does not take an Object argument is labeled friend for use by WorldBuilder.  All game requests
// for CommandSet must use Object::getCommandSetString, as we have two different sources for dynamic answers.
	const AsciiString& friend_getCommandSetString() const { return m_commandSetString; }

	const std::vector<AsciiString>& getBuildVariations() const { return m_buildVariations; }

	Real getAssetScale() const { return m_assetScale; }						///< return uniform scaling
	Real getInstanceScaleFuzziness() const { return m_instanceScaleFuzziness; }						///< return uniform scaling
	Real getStructureRubbleHeight() const { return (Real)m_structureRubbleHeight; }						///< return uniform scaling

	/*
		NOTE: if you have a Thing, don't call this function; call Thing::getGeometryInfo instead, since
		geometry can now vary on a per-object basis. Only call this when you have no Thing around,
		and want to get info for the "prototype" (eg, for building new Things)...
	*/
	const GeometryInfo& getTemplateGeometryInfo() const { return m_geometryInfo; }

	//
	// these are intended ONLY for the private use of ThingFactory and do not use
	// the m_override pointer, it deals only with templates at the "top" level
	//
	void friend_setTemplateName( const AsciiString& name ) { m_nameString = name; }
	ThingTemplate *friend_getNextTemplate() const { return m_nextThingTemplate; }
	void friend_setNextTemplate(ThingTemplate *tmplate) { m_nextThingTemplate = tmplate; }
	void friend_setTemplateID(UnsignedShort id) { m_templateID = id; }

	Int getEnergyProduction() const { return m_energyProduction; }
	Int getEnergyBonus() const { return m_energyBonus; }

	// these are NOT publicly available; you should call calcCostToBuild() or calcTimeToBuild()
	// instead, because they will take player handicaps into account.
	// Int getBuildCost() const { return m_buildCost; }

	Int getRefundValue() const { return m_refundValue; }

	BuildCompletionType getBuildCompletion() const { return (BuildCompletionType)m_buildCompletion; }

	BuildableStatus getBuildable() const;

	Int getPrereqCount() const { return m_prereqInfo.size(); }
	const ProductionPrerequisite *getNthPrereq(Int i) const { return &m_prereqInfo[i]; }

	/**
		return the BuildFacilityTemplate, if any.

		if this template needs no build facility, null is returned.

		if the template needs a build facility but the given player doesn't have any in existence,
		null will be returned.

		if you pass null for player, we'll return the 'natural' build facility.
	*/
	const ThingTemplate *getBuildFacilityTemplate( const Player *player ) const;

	Bool isBuildableItem(void) const;

	/// calculate how long (in logic frames) it will take the given player to build this unit
	Int calcTimeToBuild( const Player* player) const;

	/// calculate how much money it will take the given player to build this unit
	Int calcCostToBuild( const Player* player) const;

	/// Used only by Skirmish AI. Everyone else should call calcCostToBuild.
	Int friend_getBuildCost() const { return m_buildCost; }

	const AsciiString& getDefaultOwningSide() const { return m_defaultOwningSide; }

	/// get us the table to parse the fields for thing templates
	const FieldParse* getFieldParse() const { return s_objectFieldParseTable; }
	const FieldParse* getReskinFieldParse() const { return s_objectReskinFieldParseTable; }

	Bool isBuildFacility() const { return m_isBuildFacility; }
	Real getPlacementViewAngle( void ) const { return m_placementViewAngle; }

	Real getFactoryExitWidth() const { return m_factoryExitWidth; }
	Real getFactoryExtraBibWidth() const { return m_factoryExtraBibWidth; }

	void setCopiedFromDefault();

	void setReskinnedFrom(const ThingTemplate* tt) { DEBUG_ASSERTCRASH(m_reskinnedFrom == nullptr, ("should be null")); m_reskinnedFrom = tt; }

	Bool isPrerequisite() const { return m_isPrerequisite; }

	const WeaponTemplateSet* findWeaponTemplateSet(const WeaponSetFlags& t) const;
	const ArmorTemplateSet* findArmorTemplateSet(const ArmorSetFlags& t) const;

	// returns true iff we have at least one weaponset that contains a weapon.
	// returns false if we have no weaponsets, or they are all empty.
	Bool canPossiblyHaveAnyWeapon() const;

	Bool isEquivalentTo(const ThingTemplate* tt) const;

	UnsignedByte getCrushableLevel() const { return m_crushableLevel; }
	UnsignedByte getCrusherLevel() const { return m_crusherLevel; }

	AsciiString getUpgradeCameoName( Int n)const{ return m_upgradeCameoUpgradeNames[n];	}

protected:

	//
	// these are NOT publicly available; you should call calcCostToBuild() or calcTimeToBuild()
	// instead, because they will take player handicaps into account.
	//
	Int getBuildCost() const { return m_buildCost; }
	Real getBuildTime() const { return m_buildTime; }
	const PerUnitSoundMap* getAllPerUnitSounds( void ) const { return &m_perUnitSounds; }
	void validateAudio();
	const AudioEventRTS* getAudio(ThingTemplateAudioType t) const { return m_audioarray.m_audio[t] ? &m_audioarray.m_audio[t]->m_event : &s_audioEventNoSound; }

	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	/** Table for parsing the object fields */
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	static void parseArmorTemplateSet( INI* ini, void *instance, void *store, const void* /*userData*/ );
	static void parseWeaponTemplateSet( INI* ini, void *instance, void *store, const void* /*userData*/ );
	static void parsePrerequisites( INI* ini, void *instance, void * /*store*/, const void* /*userData*/ );
	static void parseModuleName(INI* ini, void *instance, void* /*store*/, const void* userData);
	static void parseIntList(INI* ini, void *instance, void* store, const void* userData);

	static void parsePerUnitSounds(INI* ini, void *instance, void* store, const void* userData);
	static void parsePerUnitFX(INI* ini, void *instance, void* store, const void* userData);

	static void parseAddModule(INI *ini, void *instance, void *store, const void *userData);
	static void parseRemoveModule(INI *ini, void *instance, void *store, const void *userData);
	static void parseReplaceModule(INI *ini, void *instance, void *store, const void *userData);
	static void parseInheritableModule(INI *ini, void *instance, void *store, const void *userData);

	Bool removeModuleInfo(const AsciiString& moduleToRemove, AsciiString& clearedModuleNameOut);

private:
	static const FieldParse s_objectFieldParseTable[];		///< the parse table
	static const FieldParse s_objectReskinFieldParseTable[];		///< the parse table
	static AudioEventRTS s_audioEventNoSound;

private:

	// ---- Strings
	UnicodeString			m_displayName;			///< UI display for onscreen display
	AsciiString				m_nameString;					///< name of this thing template
	AsciiString				m_defaultOwningSide;	///< default owning side (owning player is inferred)
	AsciiString				m_commandSetString;
	AsciiString				m_selectedPortraitImageName;
	AsciiString				m_buttonImageName;
	AsciiString				m_upgradeCameoUpgradeNames[MAX_UPGRADE_CAMEO_UPGRADES];	///< Use these to find the upgrade images to display on the control bar
	AsciiString				m_shadowTextureName;					///< name of texture to use for shadow decal
	AsciiString				m_moduleBeingReplacedName;		///< used only during map.ini loading... name (not tag) of Module being replaced, or empty if not inside ReplaceModule block
	AsciiString				m_moduleBeingReplacedTag;			///< used only during map.ini loading... tag (not name) of Module being replaced, or empty if not inside ReplaceModule block
#ifdef LOAD_TEST_ASSETS
	AsciiString				m_LTAName;
#endif

	// ---- Misc Larger-than-int things
	GeometryInfo			m_geometryInfo;			///< geometry information
	KindOfMaskType		m_kindof;					///< kindof bits
	AudioArray				m_audioarray;
	ModuleInfo				m_behaviorModuleInfo;
	ModuleInfo				m_drawModuleInfo;
	ModuleInfo				m_clientUpdateModuleInfo;

	// ---- Misc Arrays-of-things
	Int											m_skillPointValues[LEVEL_COUNT];
	Int											m_experienceValues[LEVEL_COUNT];		///< How much I am worth at each experience level
	Int											m_experienceRequired[LEVEL_COUNT];	///< How many experience points I need for each level

	//Code renderer handles these states now.
	//AsciiString							m_inventoryImage[ INV_IMAGE_NUM_IMAGES ];  ///< portrait inventory pictures

	// TheSuperHackers @bugfix Caball009/xezon 06/07/2025 No longer copy SparseMatchFinder to prevent copied instances linking to unrelated data.
	// This avoids mismatching in certain maps, for example those that spawn units with veterancy.
	typedef SparseMatchFinder<WeaponTemplateSet, WeaponSetFlags, SparseMatchFinderFlags_NoCopy> WeaponTemplateSetFinder;
	typedef SparseMatchFinder<ArmorTemplateSet, ArmorSetFlags, SparseMatchFinderFlags_NoCopy> ArmorTemplateSetFinder;

	// ---- STL-sized things
	std::vector<ProductionPrerequisite>	m_prereqInfo;				///< the unit Prereqs for this tech
	std::vector<AsciiString>						m_buildVariations;	/**< if we build a unit of this type via script or ui, randomly choose one
																														of these templates instead. (doesn't apply to MapObject-created items) */
	WeaponTemplateSetVector							m_weaponTemplateSets;					///< our weaponsets
	WeaponTemplateSetFinder							m_weaponTemplateSetFinder;		///< helper to allow us to find the best sets, quickly
	ArmorTemplateSetVector							m_armorTemplateSets;	///< our armorsets
	ArmorTemplateSetFinder							m_armorTemplateSetFinder;		///< helper to allow us to find the best sets, quickly
	PerUnitSoundMap											m_perUnitSounds;					///< An additional set of sounds that only apply for this template.
	PerUnitFXMap												m_perUnitFX;									///< An additional set of fx that only apply for this template.

	// ---- Pointer-sized things
	ThingTemplate*				m_nextThingTemplate;
	const ThingTemplate*	m_reskinnedFrom;									///< non nullptr if we were generated via a reskin
	const Image *					m_selectedPortraitImage;		/// portrait image when selected (to display in GUI)
	const Image	*					m_buttonImage;

	// ---- Real-sized things
	Real					m_fenceWidth;								///< Fence width for fence type objects.
	Real					m_fenceXOffset;							///< Fence X offset for fence type objects.
	Real					m_visionRange;								///< object "sees" this far around itself
	Real					m_shroudClearingRange;				///< Since So many things got added to "Seeing" functionality, we need to split this part out.
	Real					m_placementViewAngle;				///< when placing buildings this will be the angle of the building when "floating" at the mouse
	Real					m_factoryExitWidth;					///< when placing buildings this will be the width of the reserved exit area on the right side.
	Real					m_factoryExtraBibWidth;					///< when placing buildings this will be the width of the reserved exit area on the right side.
	Real					m_buildTime;									///< Seconds to build
	Real					m_assetScale;
	Real					m_instanceScaleFuzziness; ///< scale randomization tolerance to init for each Drawable instance,
	Real					m_shadowSizeX;				///< world-space extent of decal shadow texture
	Real					m_shadowSizeY;				///< world-space extent of decal shadow texture
	Real					m_shadowOffsetX;			///< world-space offset of decal shadow texture
	Real					m_shadowOffsetY;			///< world-space offset of decal shadow texture

	// ---- Int-sized things
	Int						m_energyProduction;						///< how much Energy this takes (negative values produce Energy, rather than consuming it)
	Int						m_energyBonus;								///< how much extra Energy this produces due to the upgrade
	Color					m_displayColor;								///< for the editor display color
	UnsignedInt		m_occlusionDelay;							///< delay after object creation before building occlusion is allowed.

	// ---- Short-sized things
	UnsignedShort		m_templateID;									///< id for net (etc.) transmission purposes
	UnsignedShort		m_buildCost;									///< money to build (0 == not buildable)
	UnsignedShort		m_refundValue;								///< custom resale value, if sold. (0 == use default)
	UnsignedShort		m_threatValue;								///< Threat map info
	UnsignedShort		m_maxSimultaneousOfType;			///< max simultaneous of this unit we can have (per player) at one time. (0 == unlimited)

	// ---- Bool-sized things
	Bool					m_isPrerequisite;							///< Is this thing considered in a prerequisite for any other thing?
	Bool					m_isBridge;										///< True if this model is a bridge.
 	Bool					m_isBuildFacility;						///< is this the build facility for something? (calculated based on other template's prereqs)
	Bool					m_isTrainable;								///< Whether or not I can even gain experience
	Bool					m_isForbidden;								///< useful when overriding in <mapfile>.ini
	Bool					m_armorCopiedFromDefault;
	Bool					m_weaponsCopiedFromDefault;

	// ---- Byte-sized things
	Byte					m_radarPriority;						///< does object appear on radar, and if so at what priority
	Byte					m_transportSlotCount;				///< how many "slots" we take in a transport (0 == not transportable)
	Byte					m_buildable;								///< is this thing buildable at all?
	Byte					m_buildCompletion;					///< how the units come into the world when build is complete
	Byte					m_editorSorting;						///< editor sorting type, see EditorSortingType enum
	Byte					m_structureRubbleHeight;
	Byte					m_shadowType;								///< settings which determine the type of shadow rendered
	Byte					m_moduleParsingMode;
	UnsignedByte	m_crusherLevel;							///< crusher > crushable level to actually crush
	UnsignedByte	m_crushableLevel;						///< Specifies the level of crushability (must be hit by a crusher greater than this to crush me).


};

//-----------------------------------------------------------------------------
//           Inlining
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//           Externals
//-----------------------------------------------------------------------------
