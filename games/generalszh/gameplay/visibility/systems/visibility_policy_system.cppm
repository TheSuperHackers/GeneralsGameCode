module;

#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <vector>

export module games.generalszh.gameplay.visibility.systems.visibility_policy_system;
export import games.generalszh.gameplay.visibility.definitions.visibility_policy;
export import games.generalszh.gameplay.production.components.production_state;
export import games.generalszh.gameplay.construction.components.structure;
export import engine.ecs.system.system;
export import engine.gameplay.combat.components.health;
export import engine.gameplay.containment.components.passenger_membership;
export import engine.gameplay.navigation.components.movement;
export import engine.gameplay.rts.visibility.definitions.visibility_definition;
export import engine.gameplay.rts.visibility.definitions.visibility_participant;

export namespace generalszh::visibility
{
// Projects authoritative ZH ownership/life/containment columns into the
// generic observer components.  The carrier scan is only a fixed-capacity,
// generation-checked spatial snapshot used to resolve garrison look origin;
// it is not historical visibility state.
class VisibilityPolicySystem final
{
public:
	using Query = ecs::Query<ecs::Read<engine::gameplay::combat::LifeState>,
		ecs::Read<engine::gameplay::navigation::GridPosition>,
		ecs::Write<engine::gameplay::rts::visibility::VisibilityObserver>,
		ecs::Write<engine::gameplay::rts::visibility::VisibilityEligibility>,
		ecs::Optional<generalszh::production::ProducedUnit>,
		ecs::Optional<generalszh::production::Producer>,
		ecs::Optional<generalszh::construction::Structure>,
		ecs::Optional<engine::gameplay::containment::PassengerMembership>>;
	using CarrierQuery = ecs::Query<ecs::Read<engine::gameplay::combat::LifeState>,
		ecs::Read<engine::gameplay::navigation::GridPosition>>;
	using AuxiliaryAccess = CarrierQuery;

	VisibilityPolicySystem(ecs::World &world,
		const engine::gameplay::rts::visibility::VisibilityDefinitions &definitions,
		const engine::gameplay::rts::visibility::VisibilityParticipantTable &participants,
		const std::size_t entityIndexCapacity) :
		definitions_(definitions), participants_(participants), carriers_(world),
		carrierGenerations_(entityIndexCapacity), carrierCells_(entityIndexCapacity),
		carrierAlive_(entityIndexCapacity), touchedCarriers_()
	{
		if (!entityIndexCapacity) throw std::invalid_argument("Visibility policy entity capacity must be positive");
		touchedCarriers_.reserve(entityIndexCapacity);
	}

	void BeforeChunks(Query &, ecs::SystemContext &)
	{
		for (const auto index : touchedCarriers_) carrierAlive_[index] = false;
		touchedCarriers_.clear();
		carriers_.ForEachChunk([&](CarrierQuery::Chunk chunk) {
			const auto lives = chunk.template Get<engine::gameplay::combat::LifeState>();
			const auto positions = chunk.template Get<engine::gameplay::navigation::GridPosition>();
			for (std::size_t row = 0; row != chunk.Count(); ++row)
			{
				const auto entity = chunk.Entities()[row];
				if (entity.index >= carrierGenerations_.size())
					throw std::length_error("Visibility carrier snapshot entity-index capacity exhausted");
				carrierGenerations_[entity.index] = entity.generation;
				carrierCells_[entity.index] = positions[row].cell;
				carrierAlive_[entity.index] = lives[row].alive && positions[row].cell != engine::gameplay::navigation::InvalidCell;
				touchedCarriers_.push_back(entity.index);
			}
		});
	}

	void Execute(Query::Chunk chunk, ecs::SystemContext &) const
	{
		const auto lives = chunk.Get<engine::gameplay::combat::LifeState>();
		const auto positions = chunk.Get<engine::gameplay::navigation::GridPosition>();
		auto observers = chunk.Get<engine::gameplay::rts::visibility::VisibilityObserver>();
		auto eligibility = chunk.Get<engine::gameplay::rts::visibility::VisibilityEligibility>();
		const auto units = chunk.Get<generalszh::production::ProducedUnit>();
		const auto producers = chunk.Get<generalszh::production::Producer>();
		const auto structures = chunk.Get<generalszh::construction::Structure>();
		const auto memberships = chunk.Get<engine::gameplay::containment::PassengerMembership>();
		for (std::size_t row = 0; row != chunk.Count(); ++row)
		{
			const auto &observer = observers[row];
			const auto &definition = definitions_.Get(observer.definition);
			const auto account = !units.empty() ? units[row].account :
				!producers.empty() ? producers[row].account :
				!structures.empty() ? structures[row].account : ecs::Entity{};
			const auto participant = participants_.Resolve(account);
			if (!participant.IsValid())
				throw std::invalid_argument("Visibility observer owner is not enrolled at account startup");
			observers[row].participant = participant;

			engine::gameplay::navigation::Cell carrierCell = engine::gameplay::navigation::InvalidCell;
			bool carrierValid = false;
			if (!memberships.empty() && IsGarrisonOwned(memberships[row]))
			{
				const auto carrier = memberships[row].carrier;
				carrierValid = carrier.index < carrierGenerations_.size() &&
					carrierGenerations_[carrier.index] == carrier.generation && carrierAlive_[carrier.index];
				if (carrierValid) carrierCell = carrierCells_[carrier.index];
			}
			const bool underConstruction = !structures.empty() && !structures[row].complete;
			const auto constructionRadius = underConstruction && definition.constructionRadiusCells != 0
				? std::optional<std::uint32_t>{definition.constructionRadiusCells} : std::nullopt;
			eligibility[row] = ResolveVisibilityEligibility(VisibilityPolicyInput{
				lives[row].alive, positions[row].cell,
				memberships.empty() ? std::nullopt : std::optional{memberships[row]},
				carrierCell, carrierValid, underConstruction, constructionRadius});
		}
	}

private:
	const engine::gameplay::rts::visibility::VisibilityDefinitions &definitions_;
	const engine::gameplay::rts::visibility::VisibilityParticipantTable &participants_;
	CarrierQuery carriers_;
	std::vector<ecs::EntityGeneration> carrierGenerations_;
	std::vector<engine::gameplay::navigation::Cell> carrierCells_;
	std::vector<bool> carrierAlive_;
	std::vector<ecs::EntityIndex> touchedCarriers_;
};
} // namespace generalszh::visibility

export namespace ecs
{
template<> struct SystemTraits<generalszh::visibility::VisibilityPolicySystem>
{
	static constexpr std::string_view StableName = "games.generalszh.visibility.policy";
	static constexpr SystemPhase Phase = SystemPhase::Simulation;
	using Before = SystemTypeList<>;
	using After = SystemTypeList<>;
};
} // namespace ecs
