module;

#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>

export module games.generalszh.adapters.content.radar.radar_definition;
export import games.generalszh.adapters.content.ini.named_block;
export import games.generalszh.adapters.content.upgrades.upgrade_definition;
export import games.generalszh.gameplay.radar.definitions.radar_provider_definition;
export import games.generalszh.gameplay.upgrades.identity.upgrade_catalog;

export namespace generalszh::content::radar
{
using generalszh::content::EqualToken;
using generalszh::content::IniBlock;
using generalszh::content::Tokens;
using generalszh::content::Trim;
using generalszh::content::UpgradeIdentity;
using generalszh::content::UpgradeScope;
using generalszh::radar::RadarGrantAuthority;
using generalszh::radar::RadarProviderDefinition;
using generalszh::radar::RadarUpgradeBinding;
using generalszh::upgrades::InvalidUpgradeId;
using generalszh::upgrades::MakeUpgradeKey;
using generalszh::upgrades::UpgradeCatalog;
using generalszh::upgrades::UpgradeId;

struct DecodedRadarDefinition final
{
	RadarProviderDefinition definition{};
	bool grantsAtCreation{};
	bool grantsAtBuildCompletion{true};
	// RadarUpdate.RadarExtendTime drives the legacy visual extension only; it is
	// intentionally not copied into authoritative provider definitions.
};

inline const IniBlock *FindBehavior(const IniBlock &object,
	const std::string_view moduleName)
{
	const IniBlock *found = nullptr;
	for (const IniBlock &child : object.children)
	{
		if (!EqualToken(child.kind, "Behavior"))
			continue;
		const auto words = Tokens(child.argument);
		if (words.empty() || !EqualToken(words.front(), moduleName))
			continue;
		if (found != nullptr)
			throw std::invalid_argument("Object contains duplicate " + std::string(moduleName) + " behaviors");
		found = &child;
	}
	return found;
}

inline bool DecodeBoolean(const std::string_view value, const std::string_view field)
{
	if (EqualToken(Trim(value), "YES"))
		return true;
	if (EqualToken(Trim(value), "NO"))
		return false;
	throw std::invalid_argument(std::string(field) + " must be Yes or No");
}

struct RadarUpgradeFields final
{
	std::string_view triggeredBy{};
	bool disableProof{};
};

inline RadarUpgradeFields DecodeRadarUpgrade(const IniBlock &behavior)
{
	RadarUpgradeFields result{};
	bool hasTriggeredBy = false;
	bool hasDisableProof = false;
	for (const auto &field : behavior.fields)
	{
		if (EqualToken(field.key, "TriggeredBy"))
		{
			if (hasTriggeredBy)
				throw std::invalid_argument("RadarUpgrade contains duplicate TriggeredBy fields");
			result.triggeredBy = Trim(field.value);
			hasTriggeredBy = !result.triggeredBy.empty();
			if (!hasTriggeredBy)
				throw std::invalid_argument("RadarUpgrade TriggeredBy cannot be empty");
		}
		else if (EqualToken(field.key, "DisableProof"))
		{
			if (hasDisableProof)
				throw std::invalid_argument("RadarUpgrade contains duplicate DisableProof fields");
			result.disableProof = DecodeBoolean(field.value, "RadarUpgrade DisableProof");
			hasDisableProof = true;
		}
		else
			throw std::invalid_argument("Unsupported RadarUpgrade field: " + field.key);
	}
	if (!hasTriggeredBy)
		throw std::invalid_argument("RadarUpgrade requires TriggeredBy");
	return result;
}

inline bool DecodeCreationExemption(const IniBlock &behavior)
{
	bool present = false;
	for (const auto &field : behavior.fields)
	{
		if (EqualToken(field.key, "UpgradeToGrant"))
			continue;
		if (!EqualToken(field.key, "ExemptStatus"))
			throw std::invalid_argument("Unsupported GrantUpgradeCreate field: " + field.key);
		if (present)
			throw std::invalid_argument("GrantUpgradeCreate contains duplicate ExemptStatus fields");
		present = true;
		const auto tokens = Tokens(Trim(field.value));
		if (tokens.size() != 1 || !EqualToken(tokens.front(), "UNDER_CONSTRUCTION"))
			throw std::invalid_argument(
				"GrantUpgradeCreate ExemptStatus must be UNDER_CONSTRUCTION in the radar slice");
	}
	return present;
}

inline std::string_view DecodeGrantedUpgrade(const IniBlock &behavior)
{
	std::string_view upgrade{};
	bool present = false;
	for (const auto &field : behavior.fields)
	{
		if (!EqualToken(field.key, "UpgradeToGrant"))
			continue;
		if (present)
			throw std::invalid_argument("GrantUpgradeCreate contains duplicate UpgradeToGrant fields");
		const auto tokens = Tokens(Trim(field.value));
		if (tokens.size() != 1 || tokens.front().empty())
			throw std::invalid_argument("GrantUpgradeCreate UpgradeToGrant must name one upgrade");
		upgrade = tokens.front();
		present = true;
	}
	if (!present)
		throw std::invalid_argument("GrantUpgradeCreate requires UpgradeToGrant");
	return upgrade;
}

inline const UpgradeIdentity &FindObjectUpgrade(const std::string_view name,
	const std::span<const UpgradeIdentity> identities)
{
	const UpgradeIdentity *found = generalszh::content::FindUpgradeIdentity(name, identities);
	if (found == nullptr)
		throw std::invalid_argument("Radar provider refers to an unknown upgrade identity");
	if (found->scope != UpgradeScope::Object)
		throw std::invalid_argument("Radar provider upgrade must have OBJECT scope");
	return *found;
}

inline RadarUpgradeBinding BindUpgrade(const std::string_view name,
	const UpgradeCatalog &catalog, const std::span<const UpgradeIdentity> identities)
{
	(void)FindObjectUpgrade(name, identities);
	const UpgradeId id = catalog.Find(MakeUpgradeKey(name));
	if (id == InvalidUpgradeId)
		throw std::invalid_argument("Radar provider upgrade is absent from the finalized catalog");
	const auto definitionId = id.value;
	return {definitionId, definitionId / 64u,
		std::uint64_t{1} << (definitionId % 64u), catalog.SchemaHash()};
}

inline DecodedRadarDefinition DecodeRadarProviderDefinition(
	const IniBlock &object, const std::uint32_t providerId,
	const UpgradeCatalog &catalog, const std::span<const UpgradeIdentity> identities)
{
	if (!catalog.IsFinalized())
		throw std::logic_error("Radar content requires a finalized UpgradeCatalog");
	const IniBlock *radarBehavior = FindBehavior(object, "RadarUpgrade");
	if (radarBehavior == nullptr)
		throw std::invalid_argument("Radar provider object is missing RadarUpgrade");
	const RadarUpgradeFields radar = DecodeRadarUpgrade(*radarBehavior);

	// RadarUpgrade is an effect consumer. A bounded provider must also have a
	// concrete GrantUpgradeCreate producer; China objects that only name a
	// RadarUpgrade therefore remain unsupported instead of being auto-enabled.
	const IniBlock *grantBehavior = FindBehavior(object, "GrantUpgradeCreate");
	if (grantBehavior == nullptr)
		throw std::invalid_argument(
			"Radar provider has no concrete object-scoped grant producer");
	const std::string_view granted = DecodeGrantedUpgrade(*grantBehavior);
	if (!EqualToken(granted, radar.triggeredBy))
		throw std::invalid_argument(
			"GrantUpgradeCreate and RadarUpgrade name different upgrades");
	const bool creationGrant = DecodeCreationExemption(*grantBehavior);
	const RadarUpgradeBinding binding = BindUpgrade(radar.triggeredBy, catalog, identities);
	return {
		RadarProviderDefinition{providerId, binding,
			creationGrant ? RadarGrantAuthority::CreationGrant : RadarGrantAuthority::CompletionGrant,
			radar.disableProof, true},
		creationGrant, true};
}
} // namespace generalszh::content::radar
