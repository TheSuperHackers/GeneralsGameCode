module;
#include <cstdint>
#include <string_view>
export module engine.gameplay.rts.match.systems.elimination_system;
export import engine.ecs.system.system;
export import engine.gameplay.rts.match.components.contender;
export namespace engine::gameplay::rts::match
{
struct EliminationSystem
{
    using Query = ecs::Query<ecs::Read<SurvivalCount>, ecs::Write<Contender>>;
    void Execute(Query::Chunk chunk, ecs::SystemContext &) const noexcept
    {
        const auto counts = chunk.Get<SurvivalCount>(); auto contenders = chunk.Get<Contender>();
        for (std::size_t row = 0; row != chunk.Count(); ++row)
            contenders[row].defeated |= counts[row].value == 0;
    }
};
}
export namespace ecs
{
template<> struct SystemTraits<engine::gameplay::rts::match::EliminationSystem>
{
    static constexpr std::string_view StableName = "engine.gameplay.rts.match.elimination";
    static constexpr SystemPhase Phase = SystemPhase::Simulation;
    using Before = SystemTypeList<>; using After = SystemTypeList<>;
};
}
