module;
#include <cstddef>
#include <string_view>
export module games.generalszh.gameplay.match.systems.defeat_system;
export import games.generalszh.gameplay.match.components.victory_asset;
export import games.generalszh.gameplay.match.observations.defeat_frame;
export import games.generalszh.gameplay.production.runtime.production_runtime;
export import engine.gameplay.rts.match.systems.elimination_system;
export import games.generalszh.gameplay.construction.definitions.building_definition;
export namespace generalszh::match
{
struct DefeatSystem
{
    explicit DefeatSystem(const DefeatFrame &frame) : frame(frame) {}
    using Query = ecs::Query<ecs::Write<engine::gameplay::combat::Health>, ecs::Write<engine::gameplay::combat::LifeState>,
        ecs::Optional<production::ProducedUnit>, ecs::Optional<production::Producer>, ecs::Optional<construction::Structure>>;
    void Execute(Query::Chunk chunk, ecs::SystemContext &) const noexcept
    {
        auto health = chunk.Get<engine::gameplay::combat::Health>(); auto life = chunk.Get<engine::gameplay::combat::LifeState>();
        const auto units = chunk.Get<production::ProducedUnit>(); const auto producers = chunk.Get<production::Producer>();
        const auto structures = chunk.Get<construction::Structure>();
        for (std::size_t row = 0; row != chunk.Count(); ++row)
        {
            const auto account = !units.empty() ? units[row].account : !producers.empty() ? producers[row].account
                : !structures.empty() ? structures[row].account : ecs::Entity{};
            if (frame.IsDefeated(account)) { health[row].current = 0; life[row].alive = false; }
        }
    }
private:
    const DefeatFrame &frame;
};
}
export namespace ecs
{
template<> struct SystemTraits<generalszh::match::DefeatSystem>
{
    static constexpr std::string_view StableName = "games.generalszh.match.defeat";
    static constexpr SystemPhase Phase = SystemPhase::Simulation;
    using Before = SystemTypeList<>; using After = SystemTypeList<>;
};
}
