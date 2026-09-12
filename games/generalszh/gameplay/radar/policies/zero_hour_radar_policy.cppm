module;

#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string_view>

export module games.generalszh.gameplay.radar.policies.zero_hour_radar_policy;
export import engine.gameplay.combat.components.health;
export import engine.gameplay.rts.radar.components.radar_availability;
export import engine.gameplay.rts.radar.components.radar_provider_contribution;
export import engine.gameplay.rts.radar.systems.radar_availability_system;
export import engine.gameplay.rts.upgrades.components.upgrade_words;
export import engine.gameplay.rts.upgrades.object_scope.components.object_upgrade_scope;
export import games.generalszh.gameplay.construction.components.structure;
export import games.generalszh.gameplay.power.components.power_state;
export import games.generalszh.gameplay.power.systems.power_system;
export import games.generalszh.gameplay.production.components.production_state;
export import games.generalszh.gameplay.radar.components.radar_provider_binding;
export import games.generalszh.gameplay.radar.definitions.radar_provider_definition;

export namespace generalszh::radar
{
class ZeroHourRadarPolicy final
{
public:
	static constexpr std::string_view StableName = "games.generalszh.radar.availability_system";
	using Before = ecs::SystemTypeList<>;
	// PowerSystem owns the brownout projection read by AccountSuppressed.  The
	// object-upgrade producer and lifecycle producers are composed by the game
	// root when those systems are registered; this leaf does not invent edges to
	// systems it does not own.
	using After = ecs::SystemTypeList<generalszh::power::PowerSystem>;
	using Grant = RadarProviderGrant;
	using ProviderQuery = ecs::Query<
		ecs::Read<RadarProviderBinding>,
		ecs::Write<RadarProviderGrant>,
		ecs::Write<engine::gameplay::rts::radar::RadarProviderContribution>,
		ecs::Read<engine::gameplay::combat::LifeState>,
		ecs::Optional<generalszh::construction::Structure>,
		ecs::Optional<generalszh::production::ProducedUnit>,
		ecs::Read<engine::gameplay::rts::upgrades::object_scope::ObjectUpgradeTarget>>;
	using AccountQuery = ecs::Query<
		ecs::Write<engine::gameplay::rts::radar::RadarAvailability>,
		ecs::Optional<generalszh::power::PowerState>>;
	using WordQuery = ecs::Query<
		ecs::Read<engine::gameplay::rts::upgrades::UpgradeWordOwner>,
		ecs::Read<engine::gameplay::rts::upgrades::UpgradeWordOrdinal>,
		ecs::Read<engine::gameplay::rts::upgrades::object_scope::ObjectUpgradeWordMarker>,
		ecs::Read<engine::gameplay::rts::upgrades::CompletedUpgradeWord>>;
	using AuxiliaryAccess = ecs::Query<
		ecs::Write<engine::gameplay::rts::radar::RadarAvailability>,
		ecs::Optional<generalszh::power::PowerState>,
		ecs::Read<engine::gameplay::rts::upgrades::UpgradeWordOwner>,
		ecs::Read<engine::gameplay::rts::upgrades::UpgradeWordOrdinal>,
		ecs::Read<engine::gameplay::rts::upgrades::object_scope::ObjectUpgradeWordMarker>,
		ecs::Read<engine::gameplay::rts::upgrades::CompletedUpgradeWord>>;

	explicit ZeroHourRadarPolicy(const RadarProviderDefinitions &definitions) noexcept :
		definitions_(definitions)
	{
	}

	[[nodiscard]] engine::gameplay::rts::radar::RadarWordRecord ExtractWord(
		WordQuery::Chunk chunk, const std::size_t row) const
	{
		const auto owners = chunk.template Get<engine::gameplay::rts::upgrades::UpgradeWordOwner>();
		const auto ordinals = chunk.template Get<engine::gameplay::rts::upgrades::UpgradeWordOrdinal>();
		const auto completed = chunk.template Get<engine::gameplay::rts::upgrades::CompletedUpgradeWord>();
		return {owners[row].value, ordinals[row].value, completed[row].value};
	}

	[[nodiscard]] engine::gameplay::rts::radar::RadarProviderProjection Project(
		ProviderQuery::Chunk chunk, const std::size_t row,
		const engine::gameplay::rts::radar::RadarWordIndex &words) const
	{
		const auto bindings = chunk.template Get<RadarProviderBinding>();
		const auto targets = chunk.template Get<engine::gameplay::rts::upgrades::object_scope::ObjectUpgradeTarget>();
		const auto lives = chunk.template Get<engine::gameplay::combat::LifeState>();
		const auto structures = chunk.template Get<generalszh::construction::Structure>();
		const auto units = chunk.template Get<generalszh::production::ProducedUnit>();
		const auto entity = chunk.Entities()[row];
		const auto &definition = definitions_.Get(bindings[row].definitionId);
		const auto &target = targets[row];
		const auto &upgrade = definition.upgrade;
		if (target.wordCount == 0 || upgrade.wordOrdinal >= target.wordCount)
			throw std::logic_error("Radar provider target does not contain its bound upgrade word");
		if (target.schemaHash != upgrade.schemaHash)
			throw std::logic_error("Radar provider upgrade binding schema does not match its object target");
		assert(std::has_single_bit(upgrade.bitMask));

		const auto *word = words.Find(entity, upgrade.wordOrdinal);
		if (word == nullptr || word->owner != entity || word->ordinal != upgrade.wordOrdinal)
			throw std::logic_error("Radar provider object-word row is missing or stale");

		const auto account = AccountFor(chunk, row);
		bool lifecycleEligible = lives[row].alive;
		if (!structures.empty())
			lifecycleEligible = lifecycleEligible &&
				(!definition.requiresComplete || structures[row].complete);
		else if (units.empty()) throw std::logic_error("Radar provider has no structure or produced-unit owner");

		return {account, (word->completed & upgrade.bitMask) != 0,
			lifecycleEligible, definition.resistant};
	}

	[[nodiscard]] ecs::Entity AccountFor(ProviderQuery::Chunk chunk, const std::size_t row) const
	{
		const auto structures = chunk.template Get<generalszh::construction::Structure>();
		if (!structures.empty()) return structures[row].account;
		const auto units = chunk.template Get<generalszh::production::ProducedUnit>();
		if (!units.empty()) return units[row].account;
		throw std::logic_error("Radar provider has no account-bearing owner component");
	}

	[[nodiscard]] bool AccountSuppressed(AccountQuery::Chunk chunk, const std::size_t row) const noexcept
	{
		const auto power = chunk.template Get<generalszh::power::PowerState>();
		return !power.empty() && power[row].brownout;
	}

private:
	const RadarProviderDefinitions &definitions_;
};
}
