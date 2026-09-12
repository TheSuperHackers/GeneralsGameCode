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

// The condition suffixes are derived from GeneralsMD/Code/GameEngine/Include/
// Common/ModelState.h. The modern catalog is game-owned: its explicit IDs are
// alphabetized canonical names for version 1 and include SURRENDER in every build.
// These assigned IDs must not shift when new names are added: append new IDs
// and deliberately version the catalog instead of re-sorting existing entries.

module;
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <type_traits>

export module games.generalszh.presentation.model.model_condition;

export namespace generalszh::presentation
{
enum class ModelCondition : std::uint16_t
{
	ACTIVELY_BEING_CONSTRUCTED = 0,
	ACTIVELY_CONSTRUCTING = 1,
	AFLAME = 2,
	ARMED = 3,
	ARMORSET_CRATEUPGRADE_ONE = 4,
	ARMORSET_CRATEUPGRADE_TWO = 5,
	ATTACKING = 6,
	AWAITING_CONSTRUCTION = 7,
	BACKCRUSHED = 8,
	BETWEEN_FIRING_SHOTS_A = 9,
	BETWEEN_FIRING_SHOTS_B = 10,
	BETWEEN_FIRING_SHOTS_C = 11,
	BURNED = 12,
	CAPTURED = 13,
	CARRYING = 14,
	CENTER_TO_LEFT = 15,
	CENTER_TO_RIGHT = 16,
	CLIMBING = 17,
	CONSTRUCTION_COMPLETE = 18,
	CONTINUOUS_FIRE_FAST = 19,
	CONTINUOUS_FIRE_MEAN = 20,
	CONTINUOUS_FIRE_SLOW = 21,
	DAMAGED = 22,
	DEPLOYED = 23,
	DISGUISED = 24,
	DOCKING = 25,
	DOCKING_ACTIVE = 26,
	DOCKING_BEGINNING = 27,
	DOCKING_ENDING = 28,
	DOOR_1_CLOSING = 29,
	DOOR_1_OPENING = 30,
	DOOR_1_WAITING_OPEN = 31,
	DOOR_1_WAITING_TO_CLOSE = 32,
	DOOR_2_CLOSING = 33,
	DOOR_2_OPENING = 34,
	DOOR_2_WAITING_OPEN = 35,
	DOOR_2_WAITING_TO_CLOSE = 36,
	DOOR_3_CLOSING = 37,
	DOOR_3_OPENING = 38,
	DOOR_3_WAITING_OPEN = 39,
	DOOR_3_WAITING_TO_CLOSE = 40,
	DOOR_4_CLOSING = 41,
	DOOR_4_OPENING = 42,
	DOOR_4_WAITING_OPEN = 43,
	DOOR_4_WAITING_TO_CLOSE = 44,
	DYING = 45,
	ENEMYNEAR = 46,
	EXPLODED_BOUNCING = 47,
	EXPLODED_FLAILING = 48,
	FIRING_A = 49,
	FIRING_B = 50,
	FIRING_C = 51,
	FLOODED = 52,
	FREEFALL = 53,
	FRONTCRUSHED = 54,
	GARRISONED = 55,
	JAMMED = 56,
	JETAFTERBURNER = 57,
	JETEXHAUST = 58,
	LEFT_TO_CENTER = 59,
	LOADED = 60,
	MOVING = 61,
	NIGHT = 62,
	OVER_WATER = 63,
	PACKING = 64,
	PANICKING = 65,
	PARACHUTING = 66,
	PARTIALLY_CONSTRUCTED = 67,
	POST_COLLAPSE = 68,
	POWER_PLANT_UPGRADED = 69,
	POWER_PLANT_UPGRADING = 70,
	PREATTACK_A = 71,
	PREATTACK_B = 72,
	PREATTACK_C = 73,
	PREORDER = 74,
	PRONE = 75,
	RADAR_EXTENDING = 76,
	RADAR_UPGRADED = 77,
	RAISING_FLAG = 78,
	RAPPELLING = 79,
	REALLY_DAMAGED = 80,
	RELOADING_A = 81,
	RELOADING_B = 82,
	RELOADING_C = 83,
	RIDER1 = 84,
	RIDER2 = 85,
	RIDER3 = 86,
	RIDER4 = 87,
	RIDER5 = 88,
	RIDER6 = 89,
	RIDER7 = 90,
	RIDER8 = 91,
	RIGHT_TO_CENTER = 92,
	RUBBLE = 93,
	SECOND_LIFE = 94,
	SMOLDERING = 95,
	SNOW = 96,
	SOLD = 97,
	SPECIAL_CHEERING = 98,
	SPECIAL_DAMAGED = 99,
	SPLATTED = 100,
	STUNNED = 101,
	STUNNED_FLAILING = 102,
	SURRENDER = 103,
	TOPPLED = 104,
	TURRET_ROTATE = 105,
	UNPACKING = 106,
	USER_1 = 107,
	USER_2 = 108,
	USING_WEAPON_A = 109,
	USING_WEAPON_B = 110,
	USING_WEAPON_C = 111,
	WEAPONSET_CRATEUPGRADE_ONE = 112,
	WEAPONSET_CRATEUPGRADE_TWO = 113,
	WEAPONSET_ELITE = 114,
	WEAPONSET_HERO = 115,
	WEAPONSET_PLAYER_UPGRADE = 116,
	WEAPONSET_VETERAN = 117,
	Count = 118
};

inline constexpr std::size_t ConditionCount =
	static_cast<std::size_t>(ModelCondition::Count);
inline constexpr std::size_t ConditionWordCount = (ConditionCount + 63u) / 64u;
inline constexpr std::uint32_t SchemaVersion = 1u;

inline constexpr std::array<std::string_view, ConditionCount> ConditionNames{
	"ACTIVELY_BEING_CONSTRUCTED",
	"ACTIVELY_CONSTRUCTING",
	"AFLAME",
	"ARMED",
	"ARMORSET_CRATEUPGRADE_ONE",
	"ARMORSET_CRATEUPGRADE_TWO",
	"ATTACKING",
	"AWAITING_CONSTRUCTION",
	"BACKCRUSHED",
	"BETWEEN_FIRING_SHOTS_A",
	"BETWEEN_FIRING_SHOTS_B",
	"BETWEEN_FIRING_SHOTS_C",
	"BURNED",
	"CAPTURED",
	"CARRYING",
	"CENTER_TO_LEFT",
	"CENTER_TO_RIGHT",
	"CLIMBING",
	"CONSTRUCTION_COMPLETE",
	"CONTINUOUS_FIRE_FAST",
	"CONTINUOUS_FIRE_MEAN",
	"CONTINUOUS_FIRE_SLOW",
	"DAMAGED",
	"DEPLOYED",
	"DISGUISED",
	"DOCKING",
	"DOCKING_ACTIVE",
	"DOCKING_BEGINNING",
	"DOCKING_ENDING",
	"DOOR_1_CLOSING",
	"DOOR_1_OPENING",
	"DOOR_1_WAITING_OPEN",
	"DOOR_1_WAITING_TO_CLOSE",
	"DOOR_2_CLOSING",
	"DOOR_2_OPENING",
	"DOOR_2_WAITING_OPEN",
	"DOOR_2_WAITING_TO_CLOSE",
	"DOOR_3_CLOSING",
	"DOOR_3_OPENING",
	"DOOR_3_WAITING_OPEN",
	"DOOR_3_WAITING_TO_CLOSE",
	"DOOR_4_CLOSING",
	"DOOR_4_OPENING",
	"DOOR_4_WAITING_OPEN",
	"DOOR_4_WAITING_TO_CLOSE",
	"DYING",
	"ENEMYNEAR",
	"EXPLODED_BOUNCING",
	"EXPLODED_FLAILING",
	"FIRING_A",
	"FIRING_B",
	"FIRING_C",
	"FLOODED",
	"FREEFALL",
	"FRONTCRUSHED",
	"GARRISONED",
	"JAMMED",
	"JETAFTERBURNER",
	"JETEXHAUST",
	"LEFT_TO_CENTER",
	"LOADED",
	"MOVING",
	"NIGHT",
	"OVER_WATER",
	"PACKING",
	"PANICKING",
	"PARACHUTING",
	"PARTIALLY_CONSTRUCTED",
	"POST_COLLAPSE",
	"POWER_PLANT_UPGRADED",
	"POWER_PLANT_UPGRADING",
	"PREATTACK_A",
	"PREATTACK_B",
	"PREATTACK_C",
	"PREORDER",
	"PRONE",
	"RADAR_EXTENDING",
	"RADAR_UPGRADED",
	"RAISING_FLAG",
	"RAPPELLING",
	"REALLY_DAMAGED",
	"RELOADING_A",
	"RELOADING_B",
	"RELOADING_C",
	"RIDER1",
	"RIDER2",
	"RIDER3",
	"RIDER4",
	"RIDER5",
	"RIDER6",
	"RIDER7",
	"RIDER8",
	"RIGHT_TO_CENTER",
	"RUBBLE",
	"SECOND_LIFE",
	"SMOLDERING",
	"SNOW",
	"SOLD",
	"SPECIAL_CHEERING",
	"SPECIAL_DAMAGED",
	"SPLATTED",
	"STUNNED",
	"STUNNED_FLAILING",
	"SURRENDER",
	"TOPPLED",
	"TURRET_ROTATE",
	"UNPACKING",
	"USER_1",
	"USER_2",
	"USING_WEAPON_A",
	"USING_WEAPON_B",
	"USING_WEAPON_C",
	"WEAPONSET_CRATEUPGRADE_ONE",
	"WEAPONSET_CRATEUPGRADE_TWO",
	"WEAPONSET_ELITE",
	"WEAPONSET_HERO",
	"WEAPONSET_PLAYER_UPGRADE",
	"WEAPONSET_VETERAN"};

static_assert(ConditionNames.size() == ConditionCount);
static_assert(ConditionWordCount == 2u);

constexpr std::string_view Name(ModelCondition condition) noexcept
{
	const auto id = static_cast<std::size_t>(condition);
	assert(id < ConditionCount);
	return ConditionNames[id];
}

struct ConditionMask
{
	std::array<std::uint64_t, ConditionWordCount> words{};

	constexpr void Set(ModelCondition condition) noexcept
	{
		const auto id = static_cast<std::size_t>(condition);
		assert(id < ConditionCount);
		words[id / 64u] |= std::uint64_t{1} << (id % 64u);
	}

	constexpr void Clear(ModelCondition condition) noexcept
	{
		const auto id = static_cast<std::size_t>(condition);
		assert(id < ConditionCount);
		words[id / 64u] &= ~(std::uint64_t{1} << (id % 64u));
	}

	[[nodiscard]] constexpr bool Contains(ModelCondition condition) const noexcept
	{
		const auto id = static_cast<std::size_t>(condition);
		assert(id < ConditionCount);
		return (words[id / 64u] & (std::uint64_t{1} << (id % 64u))) != 0u;
	}

	[[nodiscard]] constexpr bool Empty() const noexcept
	{
		for (const auto word : words)
			if (word != 0u)
				return false;
		return true;
	}

	constexpr void Reset() noexcept
	{
		words.fill(0u);
	}

	constexpr void Apply(const ConditionMask &clearMask, const ConditionMask &setMask) noexcept
	{
		for (std::size_t index = 0u; index < ConditionWordCount; ++index)
			words[index] = (words[index] & ~clearMask.words[index]) | setMask.words[index];
	}
};

static_assert(std::is_standard_layout_v<ConditionMask>);
static_assert(sizeof(ConditionMask) == ConditionWordCount * sizeof(std::uint64_t));
static_assert(std::is_trivially_copyable_v<ConditionMask>);
static_assert(std::is_nothrow_default_constructible_v<ConditionMask>);
static_assert(std::is_nothrow_copy_constructible_v<ConditionMask>);
static_assert(std::is_nothrow_copy_assignable_v<ConditionMask>);
static_assert(std::is_nothrow_destructible_v<ConditionMask>);
static_assert(!std::is_polymorphic_v<ConditionMask>);
}
