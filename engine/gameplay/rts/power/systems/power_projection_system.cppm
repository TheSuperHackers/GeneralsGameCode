module;
#include <cstdint>
#include <string_view>
export module engine.gameplay.rts.power.systems.power_projection_system;
export import engine.gameplay.rts.power.algorithms.power_projection;
export import engine.gameplay.combat.systems.health_system;
export namespace engine::gameplay::rts::power
{
struct PowerProjectionSystem
{
    using Query = ecs::Query<ecs::Read<PowerSource>, ecs::Write<PowerContribution>,
        ecs::Optional<engine::gameplay::combat::LifeState>>;
    void Execute(Query::Chunk chunk, ecs::SystemContext &) const noexcept
    {
        const auto sources = chunk.Get<PowerSource>();
        const auto life = chunk.Get<engine::gameplay::combat::LifeState>();
        auto contributions = chunk.Get<PowerContribution>();
        for (std::size_t row = 0; row != chunk.Count(); ++row)
            contributions[row] = ProjectPower(sources[row], life.empty() || life[row].alive);
    }
};
}
export namespace ecs
{
template<> struct SystemTraits<engine::gameplay::rts::power::PowerProjectionSystem>
{
    static constexpr std::string_view StableName = "engine.gameplay.rts.power.project";
    static constexpr SystemPhase Phase = SystemPhase::Simulation;
    using Before = SystemTypeList<>; using After = SystemTypeList<>;
};
}
