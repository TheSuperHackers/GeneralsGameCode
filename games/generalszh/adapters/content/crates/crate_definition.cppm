module;

#include <algorithm>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

export module games.generalszh.adapters.content.crates.crate_definition;
export import engine.gameplay.rts.unlocks.definitions.unlock_catalog;
export import games.generalszh.adapters.content.ini.named_block;
export import games.generalszh.adapters.content.upgrades.upgrade_definition;
export import games.generalszh.gameplay.crates.definitions.collector_definition;
export import games.generalszh.gameplay.crates.definitions.crate_definition;

export namespace generalszh::content
{
struct DecodedMoneyUpgradeBoost final
{
	std::string upgradeName{};
	std::uint32_t amount{};
};

struct DecodedCrate final
{
	std::string objectName{};
	crates::CrateDefinition definition{};
	std::vector<std::string> objectKindOfNames{};
	std::vector<std::string> requiredKindOfNames{};
	std::vector<std::string> forbiddenKindOfNames{};
	std::string pickupScienceName{};
	std::vector<DecodedMoneyUpgradeBoost> moneyUpgradeBoosts{};
	// PARACHUTABLE is an authored spawn hint. CrateState remains the
	// authoritative per-instance placement state after spawn.
	bool parachutable{};
	std::vector<std::string> presentationOmissions{};
	std::vector<std::string> objectFieldOmissions{};
	std::vector<std::string> unsupportedSimulationFields{};
	std::vector<std::string> omittedBehaviors{};
};

namespace crate_detail
{
inline bool HasTokenPrefix(const std::string_view token, const std::string_view prefix,
	std::string_view &value) noexcept
{
	if (token.size() < prefix.size() || !EqualToken(token.substr(0, prefix.size()), prefix))
		return false;
	value = token.substr(prefix.size());
	return !value.empty();
}

inline bool IsSabotageModule(const std::string_view module) noexcept
{
	constexpr std::string_view prefix = "Sabotage";
	constexpr std::string_view suffix = "CrateCollide";
	return module.size() > prefix.size() + suffix.size() &&
		EqualToken(module.substr(0, prefix.size()), prefix) &&
		EqualToken(module.substr(module.size() - suffix.size()), suffix);
}

inline std::uint32_t Unsigned(const std::string_view authored, const std::string_view field)
{
	const auto value = Trim(authored);
	std::uint64_t result{};
	const auto parsed = std::from_chars(value.data(), value.data() + value.size(), result);
	if (value.empty() || parsed.ec != std::errc{} || parsed.ptr != value.data() + value.size())
		throw std::invalid_argument("Invalid unsigned crate field: " + std::string(field));
	if (result > (std::numeric_limits<std::uint32_t>::max)())
		throw std::out_of_range("Crate field exceeds uint32 range: " + std::string(field));
	return static_cast<std::uint32_t>(result);
}

inline bool Boolean(const std::string_view authored, const std::string_view field)
{
	const auto value = Trim(authored);
	if (EqualToken(value, "Yes")) return true;
	if (EqualToken(value, "No")) return false;
	throw std::invalid_argument("Crate boolean requires Yes or No: " + std::string(field));
}

inline void RejectDuplicate(const bool alreadySeen, const std::string_view field)
{
	if (alreadySeen)
		throw std::invalid_argument("Duplicate crate module field: " + std::string(field));
}

inline void DecodeKindOfNames(const std::string_view authored, std::vector<std::string> &destination,
	const std::string_view field)
{
	const auto tokens = Tokens(authored);
	if (tokens.empty())
		throw std::invalid_argument("Crate KindOf field cannot be empty: " + std::string(field));
	destination.clear();
	for (const auto token : tokens)
	{
		if (EqualToken(token, "NONE"))
		{
			if (tokens.size() != 1)
				throw std::invalid_argument("NONE cannot be combined in crate KindOf field");
			continue;
		}
		destination.emplace_back(token);
	}
}

inline void DecodeUpgradeBoost(const std::string_view authored, DecodedCrate &result)
{
	const auto tokens = Tokens(authored);
	if (tokens.size() != 2)
		throw std::invalid_argument("UpgradedBoost requires UpgradeType:<name> Boost:<amount>");
	std::string_view upgradeName, amountText;
	if (!HasTokenPrefix(tokens[0], "UpgradeType:", upgradeName) ||
		!HasTokenPrefix(tokens[1], "Boost:", amountText))
		throw std::invalid_argument("UpgradedBoost has invalid legacy token order");
	const auto validIdentityName = [](const std::string_view value) noexcept {
		if (value.empty()) return false;
		for (const auto character : value)
		{
			const auto byte = static_cast<unsigned char>(character);
			const bool letter = (byte >= static_cast<unsigned char>('A') &&
				byte <= static_cast<unsigned char>('Z')) ||
				(byte >= static_cast<unsigned char>('a') &&
				byte <= static_cast<unsigned char>('z'));
			const bool digit = byte >= static_cast<unsigned char>('0') &&
				byte <= static_cast<unsigned char>('9');
			if (!letter && !digit && byte != static_cast<unsigned char>('_') &&
				byte != static_cast<unsigned char>('.') &&
				byte != static_cast<unsigned char>('-'))
				return false;
		}
		return true;
	};
	if (!validIdentityName(upgradeName))
		throw std::invalid_argument("UpgradedBoost names an invalid upgrade identity");
	result.moneyUpgradeBoosts.push_back({std::string{upgradeName}, Unsigned(amountText, "Boost")});
}

inline bool IsPresentationField(const std::string_view key) noexcept
{
	return EqualToken(key, "ExecuteFX") || EqualToken(key, "ExecuteAnimation") ||
		EqualToken(key, "ExecuteAnimationTime") || EqualToken(key, "ExecuteAnimationZRise") ||
		EqualToken(key, "ExecuteAnimationFades");
}

inline bool IsObjectPresentationField(const std::string_view key) noexcept
{
	return EqualToken(key, "DisplayName") || EqualToken(key, "EditorSorting") ||
		EqualToken(key, "Shadow");
}

inline bool IsObjectScopedOmission(const std::string_view key) noexcept
{
	return EqualToken(key, "TransportSlotCount") || EqualToken(key, "Geometry") ||
		EqualToken(key, "GeometryMajorRadius") || EqualToken(key, "GeometryMinorRadius") ||
		EqualToken(key, "GeometryHeight") || EqualToken(key, "GeometryIsSmall");
}

inline bool IsPresentationChild(const std::string_view kind) noexcept
{
	return EqualToken(kind, "Draw") || EqualToken(kind, "ClientBehavior") ||
		EqualToken(kind, "UnitSpecificSounds");
}

inline bool IsKnownSimulationChildOmission(const std::string_view kind) noexcept
{
	return EqualToken(kind, "SquishCollide") || EqualToken(kind, "PhysicsBehavior") ||
		EqualToken(kind, "DeletionUpdate");
}

inline bool IsUnsupportedModuleField(const std::string_view module, const std::string_view key) noexcept
{
	if (EqualToken(module, "SalvageCrateCollide"))
		return EqualToken(key, "WeaponChance") || EqualToken(key, "LevelChance") ||
			EqualToken(key, "MoneyChance") || EqualToken(key, "MinMoney") || EqualToken(key, "MaxMoney");
	if (EqualToken(module, "UnitCrateCollide"))
		return EqualToken(key, "UnitCount") || EqualToken(key, "UnitName");
	if (IsSabotageModule(module))
		return EqualToken(key, "SabotagePowerDuration") || EqualToken(key, "StealCashAmount") ||
			EqualToken(key, "SabotageDuration");
	return false;
}

inline crates::RewardKind RewardForModule(const std::string_view module)
{
	if (EqualToken(module, "MoneyCrateCollide")) return crates::RewardKind::Money;
	if (EqualToken(module, "HealCrateCollide")) return crates::RewardKind::Heal;
	if (EqualToken(module, "VeterancyCrateCollide")) return crates::RewardKind::Veterancy;
	if (EqualToken(module, "SalvageCrateCollide")) return crates::RewardKind::Salvage;
	if (EqualToken(module, "UnitCrateCollide")) return crates::RewardKind::Unit;
	if (EqualToken(module, "ShroudCrateCollide")) return crates::RewardKind::Shroud;
	if (IsSabotageModule(module) || EqualToken(module, "ConvertToCarBombCrateCollide") ||
		EqualToken(module, "ConvertToHijackedVehicleCrateCollide"))
		return crates::RewardKind::Sabotage;
	throw std::invalid_argument("Not a recognized crate collide module: " + std::string(module));
}

inline bool IsCrateModule(const std::string_view module) noexcept
{
	return EqualToken(module, "MoneyCrateCollide") || EqualToken(module, "HealCrateCollide") ||
		EqualToken(module, "VeterancyCrateCollide") || EqualToken(module, "SalvageCrateCollide") ||
		EqualToken(module, "UnitCrateCollide") || EqualToken(module, "ShroudCrateCollide") ||
		IsSabotageModule(module) || EqualToken(module, "ConvertToCarBombCrateCollide") ||
		EqualToken(module, "ConvertToHijackedVehicleCrateCollide");
}

inline void DecodeField(const std::string_view module, const IniField &field, DecodedCrate &result,
	bool &requiredSeen, bool &forbiddenSeen, bool &forbidOwnerSeen, bool &buildingSeen,
	bool &humanSeen, bool &multiSeen, bool &scienceSeen, bool &moneySeen, bool &rangeSeen,
	bool &addsOwnerSeen, bool &pilotSeen)
{
	const auto key = std::string_view{field.key};
	if (IsPresentationField(key))
	{
		result.presentationOmissions.push_back("Crate presentation field: " + field.key);
		return;
	}
	if (EqualToken(key, "RequiredKindOf"))
	{
		RejectDuplicate(requiredSeen, key); requiredSeen = true;
		DecodeKindOfNames(field.value, result.requiredKindOfNames, key); return;
	}
	if (EqualToken(key, "ForbiddenKindOf"))
	{
		RejectDuplicate(forbiddenSeen, key); forbiddenSeen = true;
		DecodeKindOfNames(field.value, result.forbiddenKindOfNames, key); return;
	}
	if (EqualToken(key, "ForbidOwnerPlayer"))
	{
		RejectDuplicate(forbidOwnerSeen, key); forbidOwnerSeen = true;
		result.definition.forbidOwner = Boolean(field.value, key); return;
	}
	if (EqualToken(key, "BuildingPickup"))
	{
		RejectDuplicate(buildingSeen, key); buildingSeen = true;
		result.definition.buildingPickup = Boolean(field.value, key); return;
	}
	if (EqualToken(key, "HumanOnly"))
	{
		RejectDuplicate(humanSeen, key); humanSeen = true;
		result.definition.humanOnly = Boolean(field.value, key); return;
	}
	if (EqualToken(key, "AllowMultiPickup"))
	{
		RejectDuplicate(multiSeen, key); multiSeen = true;
		result.definition.allowMultiPickup = Boolean(field.value, key); return;
	}
	if (EqualToken(key, "PickupScience"))
	{
		RejectDuplicate(scienceSeen, key); scienceSeen = true;
		const auto tokens = Tokens(field.value);
		if (tokens.size() != 1 || !engine::gameplay::rts::unlocks::IsValidUnlockName(tokens.front()))
			throw std::invalid_argument("PickupScience must name one canonical Science definition");
		result.pickupScienceName = std::string{tokens.front()}; return;
	}
	if (EqualToken(module, "MoneyCrateCollide") && EqualToken(key, "MoneyProvided"))
	{
		RejectDuplicate(moneySeen, key); moneySeen = true;
		result.definition.moneyProvided = Unsigned(field.value, key); return;
	}
	if (EqualToken(module, "MoneyCrateCollide") && EqualToken(key, "UpgradedBoost"))
	{
		DecodeUpgradeBoost(field.value, result); return;
	}
	if (EqualToken(module, "VeterancyCrateCollide") && EqualToken(key, "EffectRange"))
	{
		RejectDuplicate(rangeSeen, key); rangeSeen = true;
		result.definition.effectRange = Unsigned(field.value, key); return;
	}
	if (EqualToken(module, "VeterancyCrateCollide") && EqualToken(key, "AddsOwnerVeterancy"))
	{
		RejectDuplicate(addsOwnerSeen, key); addsOwnerSeen = true;
		result.definition.addsOwnerVeterancy = Boolean(field.value, key); return;
	}
	if (EqualToken(module, "VeterancyCrateCollide") && EqualToken(key, "IsPilot"))
	{
		RejectDuplicate(pilotSeen, key); pilotSeen = true;
		result.definition.isPilot = Boolean(field.value, key); return;
	}
	if (IsUnsupportedModuleField(module, key))
	{
		result.unsupportedSimulationFields.push_back("Unsupported crate simulation field: " + field.key);
		return;
	}
	throw std::invalid_argument("Unknown crate simulation field: " + field.key);
}

} // namespace crate_detail

inline DecodedCrate DecodeCrate(const IniBlock &object)
{
	if (!EqualToken(object.kind, "Object") || object.argument.empty())
		throw std::invalid_argument("Crate decoder requires a named Object block");
	DecodedCrate result;
	result.objectName = object.argument;
	bool kindOfSeen = false;
	for (const auto &field : object.fields)
	{
		if (EqualToken(field.key, "KindOf"))
		{
			crate_detail::RejectDuplicate(kindOfSeen, field.key);
			kindOfSeen = true;
			crate_detail::DecodeKindOfNames(field.value, result.objectKindOfNames, field.key);
			for (const auto &token : result.objectKindOfNames)
				if (EqualToken(token, "PARACHUTABLE")) result.parachutable = true;
		}
		else if (crate_detail::IsObjectPresentationField(field.key))
			result.presentationOmissions.push_back("Crate object presentation field: " + field.key);
		else if (crate_detail::IsObjectScopedOmission(field.key))
			result.objectFieldOmissions.push_back("Crate object field outside pickup slice: " + field.key);
		else
			throw std::invalid_argument("Unknown crate Object simulation field: " + field.key);
	}

	bool found = false;
	for (const auto &child : object.children)
	{
		if (!EqualToken(child.kind, "Behavior"))
		{
			if (crate_detail::IsPresentationChild(child.kind))
				result.presentationOmissions.push_back("Crate presentation block: " + child.kind);
			else if (crate_detail::IsKnownSimulationChildOmission(child.kind))
				result.omittedBehaviors.push_back(child.kind + ": " + child.argument);
			else
				throw std::invalid_argument("Unknown crate Object block: " + child.kind);
			continue;
		}
		const auto words = Tokens(child.argument);
		if (words.empty())
			throw std::invalid_argument("Crate object contains an unnamed Behavior");
		if (!crate_detail::IsCrateModule(words.front()))
		{
			result.omittedBehaviors.push_back("Behavior not owned by crate adapter: " + child.argument);
			continue;
		}
		if (words.size() > 2)
			throw std::invalid_argument("Crate Behavior has too many module-name tokens");
		if (found)
			throw std::invalid_argument("Crate Object contains duplicate CrateCollide modules");
		found = true;
		result.definition.reward = crate_detail::RewardForModule(words.front());
		bool requiredSeen = false, forbiddenSeen = false, forbidOwnerSeen = false;
		bool buildingSeen = false, humanSeen = false, multiSeen = false, scienceSeen = false;
		bool moneySeen = false, rangeSeen = false, addsOwnerSeen = false, pilotSeen = false;
		for (const auto &field : child.fields)
			crate_detail::DecodeField(words.front(), field, result, requiredSeen, forbiddenSeen,
				forbidOwnerSeen, buildingSeen, humanSeen, multiSeen, scienceSeen, moneySeen,
				rangeSeen, addsOwnerSeen, pilotSeen);
	}
	if (!found)
		throw std::invalid_argument("Object contains no CrateCollide behavior");
	if (!result.unsupportedSimulationFields.empty())
		result.omittedBehaviors.push_back("Reward family is outside the bounded money/heal/veterancy slice");
	return result;
}

inline DecodedCrate DecodeCrate(const std::string_view text, const std::string_view name)
{
	return DecodeCrate(ReadNamedBlock(text, "Object", name));
}

inline crates::CrateDefinition BindCrateDefinition(const DecodedCrate &source,
	const crates::KindOfCatalog &kindOfCatalog,
	const engine::gameplay::rts::unlocks::UnlockCatalog &scienceCatalog,
	const std::span<const UpgradeIdentity> upgradeIdentities)
{
	if (!scienceCatalog.IsFinalized())
		throw std::logic_error("Crate definition binding requires finalized science catalog");
	if (!source.unsupportedSimulationFields.empty())
		throw std::invalid_argument("Crate contains unsupported simulation fields");
	ValidateUpgradeIdentities(upgradeIdentities);
	crates::CrateDefinition result = source.definition;
	for (const auto &name : source.objectKindOfNames)
		if (!kindOfCatalog.Find(name))
			throw std::invalid_argument("Crate Object references an unknown KindOf: " + name);
	result.requiredKindOf = 0;
	for (const auto &name : source.requiredKindOfNames)
	{
		const auto *binding = kindOfCatalog.Find(name);
		if (!binding)
			throw std::invalid_argument("Crate references an unknown RequiredKindOf: " + name);
		result.requiredKindOf |= binding->mask;
	}
	result.forbiddenKindOf = 0;
	for (const auto &name : source.forbiddenKindOfNames)
	{
		const auto *binding = kindOfCatalog.Find(name);
		if (!binding)
			throw std::invalid_argument("Crate references an unknown ForbiddenKindOf: " + name);
		result.forbiddenKindOf |= binding->mask;
	}
	result.pickupScience = engine::gameplay::rts::unlocks::InvalidUnlockId;
	if (!source.pickupScienceName.empty())
	{
		const auto id = scienceCatalog.Find(
			engine::gameplay::rts::unlocks::MakeUnlockKey(source.pickupScienceName));
		if (id == engine::gameplay::rts::unlocks::InvalidUnlockId)
			throw std::invalid_argument("Crate references an unknown PickupScience: " + source.pickupScienceName);
		result.pickupScience = id;
	}
	result.moneyUpgradeBoosts.clear();
	result.moneyUpgradeBoosts.reserve(source.moneyUpgradeBoosts.size());
	for (const auto &raw : source.moneyUpgradeBoosts)
	{
		const auto *identity = FindUpgradeIdentity(raw.upgradeName, upgradeIdentities);
		if (!identity)
			throw std::invalid_argument("Crate references an unknown UpgradeType: " + raw.upgradeName);
		if (identity->scope != UpgradeScope::Player)
			throw std::invalid_argument("Crate UpgradeType must be player-scoped: " + raw.upgradeName);
		result.moneyUpgradeBoosts.push_back({identity->definition, raw.amount});
	}
	return result;
}
} // namespace generalszh::content
