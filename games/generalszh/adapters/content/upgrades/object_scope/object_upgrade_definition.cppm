module;

#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <string_view>

export module games.generalszh.adapters.content.upgrades.object_scope.object_upgrade_definition;
export import games.generalszh.adapters.content.ini.named_block;
export import games.generalszh.adapters.content.upgrades.upgrade_definition;
export import games.generalszh.gameplay.upgrades.identity.upgrade_catalog;
export import games.generalszh.gameplay.upgrades.object_scope.inputs.object_upgrade_creation;

export namespace generalszh::content::object_scope
{
using generalszh::content::IniBlock;
using generalszh::content::UpgradeIdentity;
using generalszh::upgrades::InvalidUpgradeId;
using generalszh::upgrades::MakeUpgradeKey;
using generalszh::upgrades::UpgradeCatalog;
using generalszh::upgrades::UpgradeId;
using generalszh::upgrades::object_scope::ObjectUpgradeCreationDefinition;

inline const IniBlock &FindGrantUpgradeCreate(const IniBlock &object)
{
	const IniBlock *found = nullptr;
	for (const IniBlock &child : object.children)
	{
		if (!EqualToken(child.kind, "Behavior"))
			continue;
		const auto words = Tokens(child.argument);
		if (words.empty() || !EqualToken(words.front(), "GrantUpgradeCreate"))
			continue;
		if (found != nullptr)
			throw std::invalid_argument(
				"An object may not contain multiple GrantUpgradeCreate behaviors in this slice");
		found = &child;
	}
	if (found == nullptr)
		throw std::invalid_argument("Object is missing a GrantUpgradeCreate behavior");
	return *found;
}

inline bool DecodeUnderConstructionExemption(const IniBlock &behavior)
{
	bool present = false;
	std::string_view value{};
	for (const auto &field : behavior.fields)
	{
		if (!EqualToken(field.key, "ExemptStatus"))
			continue;
		if (present)
			throw std::invalid_argument("GrantUpgradeCreate contains duplicate ExemptStatus fields");
		present = true;
		value = Trim(field.value);
	}
	if (!present)
		return false;
	const auto tokens = Tokens(value);
	if (tokens.size() != 1 || !EqualToken(tokens.front(), "UNDER_CONSTRUCTION"))
		throw std::invalid_argument(
			"GrantUpgradeCreate ExemptStatus contains an unrepresented exemption variant");
	return true;
}

inline std::string_view DecodeUpgradeName(const IniBlock &behavior)
{
	const std::string_view value = Trim(behavior.Require("UpgradeToGrant"));
	if (value.empty() || Tokens(value).size() != 1)
		throw std::invalid_argument("GrantUpgradeCreate UpgradeToGrant must name one upgrade");
	return value;
}

inline const UpgradeIdentity &FindObjectIdentity(
	const std::string_view name,
		const std::span<const UpgradeIdentity> identities)
{
	const UpgradeIdentity *identity = FindUpgradeIdentity(name, identities);
	if (identity == nullptr)
		throw std::invalid_argument("GrantUpgradeCreate refers to an unknown upgrade identity");
	if (identity->scope != UpgradeScope::Object)
		throw std::invalid_argument("GrantUpgradeCreate upgrade identity is not OBJECT scoped");
	return *identity;
}

inline ObjectUpgradeCreationDefinition BindObjectUpgradeDefinition(
	const IniBlock &object,
	const UpgradeCatalog &catalog,
	const std::span<const UpgradeIdentity> identities)
{
	if (!catalog.IsFinalized())
		throw std::logic_error("Object upgrade content requires a finalized UpgradeCatalog");
	const IniBlock &behavior = FindGrantUpgradeCreate(object);
	const std::string_view name = DecodeUpgradeName(behavior);
	(void)FindObjectIdentity(name, identities);

	const UpgradeId id = catalog.Find(MakeUpgradeKey(name));
	if (id == InvalidUpgradeId)
		throw std::invalid_argument("GrantUpgradeCreate upgrade is absent from the frozen catalog");
	const std::size_t catalogCount = catalog.Count();
	if (catalogCount > (static_cast<std::size_t>((std::numeric_limits<std::uint32_t>::max)()) - 63u))
		throw std::length_error("Object upgrade catalog word count exceeds runtime range");
	const std::size_t wordCount = (catalogCount + 63u) / 64u;
	if (wordCount > static_cast<std::size_t>((std::numeric_limits<std::uint32_t>::max)()))
		throw std::length_error("Object upgrade catalog has too many word rows");

	const std::uint32_t definitionId = id.value;
	const std::uint32_t wordOrdinal = definitionId / 64u;
	const std::uint64_t bitMask = std::uint64_t{1} << (definitionId % 64u);
	const engine::gameplay::rts::upgrades::object_scope::ObjectUpgradeBinding binding{
		definitionId, wordOrdinal, bitMask, catalog.SchemaHash()};
	engine::gameplay::rts::upgrades::object_scope::ValidateCanonicalBinding(binding);
	return {binding, {DecodeUnderConstructionExemption(behavior)}};
}
}
