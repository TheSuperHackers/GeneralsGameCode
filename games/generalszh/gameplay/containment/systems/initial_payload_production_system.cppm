module;
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string_view>
export module games.generalszh.gameplay.containment.systems.initial_payload_production_system;
export import games.generalszh.gameplay.containment.components.initial_payload;
export import games.generalszh.gameplay.production.boundary.production_operations;
export import games.generalszh.gameplay.production.systems.build_completion_system;
export import engine.gameplay.combat.systems.health_system;
export import engine.gameplay.navigation.components.movement;

export namespace generalszh::containment
{
class InitialPayloadProductionSystem
{
public:
    using Query = ecs::Query<
        ecs::Read<production::ProducedUnit>,
        ecs::Read<engine::gameplay::containment::TransportBinding>,
        ecs::Read<engine::gameplay::combat::LifeState>,
        ecs::Read<engine::gameplay::navigation::GridPosition>,
        ecs::Write<InitialPayloadBinding>>;

    explicit InitialPayloadProductionSystem(const production::BuildCatalog &catalog) : catalog_(catalog) {}

    void Execute(Query::Chunk chunk, ecs::SystemContext &context) const
    {
        const auto produced = chunk.template Get<production::ProducedUnit>();
        const auto transports = chunk.template Get<engine::gameplay::containment::TransportBinding>();
        const auto lives = chunk.template Get<engine::gameplay::combat::LifeState>();
        const auto positions = chunk.template Get<engine::gameplay::navigation::GridPosition>();
        auto bindings = chunk.template Get<InitialPayloadBinding>();
        const auto definitions = catalog_.Definitions();

        for (std::size_t row = 0; row != chunk.Count(); ++row)
        {
            auto &binding = bindings[row];
            if (binding.phase != InitialPayloadPhase::Pending) continue;

            const auto carrierIndex = catalog_.DefinitionIndex(produced[row].definition);
            if (carrierIndex >= definitions.size() || definitions[carrierIndex].key != produced[row].definition)
                throw std::logic_error("Initial payload carrier definition is not in the build catalog");
            const auto &carrierDefinition = definitions[carrierIndex];
            if (!carrierDefinition.transportDefinition
                || *carrierDefinition.transportDefinition != transports[row].definition)
                throw std::logic_error("Initial payload carrier transport binding disagrees with its definition");

            const auto configured = catalog_.InitialPayload(carrierIndex);
            if (!configured || configured->passengerDefinition != binding.passengerDefinition
                || configured->count != binding.count)
                throw std::logic_error("Initial payload runtime binding disagrees with its immutable definition");

            const auto passengerIndex = catalog_.DefinitionIndex(binding.passengerDefinition);
            if (passengerIndex >= definitions.size() || definitions[passengerIndex].key != binding.passengerDefinition)
                throw std::logic_error("Initial payload passenger definition is not in the build catalog");

            const auto carrier = chunk.Entities()[row];
            if (!lives[row].alive)
            {
                binding.phase = InitialPayloadPhase::Aborted;
                continue;
            }
            if (binding.count == 0)
            {
                binding.phase = InitialPayloadPhase::Consumed;
                continue;
            }

            auto order = production::MakePayloadOrder(catalog_, carrier, produced[row].account,
                positions[row].cell, binding.passengerDefinition, passengerIndex);
            engine::gameplay::rts::production::ProductionQuantity quantity{
                static_cast<std::int32_t>(binding.count), 0};
            // Initial payload is transport cargo by construction. CompleteBuild
            // publishes the same explicit Transport-owned membership used by
            // ordinary production; it must not rely on the combat policy or a
            // default Inside aggregate to identify the container kind.
            production::CompleteBuild(order, quantity, context.Commands(), context.Tick(), {}, carrier);
            assert(quantity.completed == binding.count);
            binding.phase = InitialPayloadPhase::Consumed;
        }
    }

private:
    const production::BuildCatalog &catalog_;
};
}

export namespace ecs
{
template<> struct SystemTraits<generalszh::containment::InitialPayloadProductionSystem>
{
    static constexpr std::string_view StableName = "games.generalszh.containment.initial_payload";
    static constexpr SystemPhase Phase = SystemPhase::Simulation;
    using Before = SystemTypeList<>;
    using After = SystemTypeList<>;
};
}
