module;
#include <cstdint>
#include <algorithm>
#include <cassert>
#include <string_view>
export module engine.gameplay.rts.production.systems.build_system;
export import engine.ecs.system.system;
export import engine.time.simulation_time;
export import engine.gameplay.rts.production.algorithms.advance_build;
export import engine.gameplay.rts.production.definitions.build_definition;
export namespace engine::gameplay::rts::production
{
struct BuildSystem
{
    using Query = ecs::Query<ecs::Write<BuildWork>, ecs::Read<BuildEnabled>, ecs::Write<BuildReady>>;
    void Execute(Query::Chunk chunk, ecs::SystemContext &) const noexcept
    {
        auto work = chunk.Get<BuildWork>(); const auto enabled = chunk.Get<BuildEnabled>();
        auto ready = chunk.Get<BuildReady>();
        for (std::size_t row = 0; row != chunk.Count(); ++row)
        {
            AdvanceBuild(work[row], enabled[row], ready[row]);
        }
    }
};
}
export namespace ecs
{
template<> struct SystemTraits<engine::gameplay::rts::production::BuildSystem>
{
    static constexpr std::string_view StableName = "engine.gameplay.rts.production.build";
    static constexpr SystemPhase Phase = SystemPhase::Simulation;
    using Before = SystemTypeList<>; using After = SystemTypeList<>;
};
}
