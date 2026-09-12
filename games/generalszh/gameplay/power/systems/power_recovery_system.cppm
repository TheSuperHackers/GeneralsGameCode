module;
#include <cstddef>
#include <string_view>
export module games.generalszh.gameplay.power.systems.power_recovery_system;
export import games.generalszh.gameplay.power.components.power_suppression;
export import games.generalszh.gameplay.power.algorithms.power_suppression;
export import games.generalszh.gameplay.power.components.power_recovery;
export import engine.ecs.system.system;
export import engine.gameplay.rts.power.components.power_ledger;
export namespace generalszh::power
{
struct PowerRecoverySystem
{
    using Query = ecs::Query<ecs::Write<PowerSuppression>, ecs::Write<PowerRecovery>, ecs::Read<PowerRecoveryOwner>,
        ecs::Read<engine::gameplay::rts::power::PowerLedger>>;
    void Execute(Query::Chunk chunk, ecs::SystemContext &context) const noexcept
    {
        auto suppression = chunk.Get<PowerSuppression>();
        auto recovery = chunk.Get<PowerRecovery>();
        for (std::size_t row = 0; row != chunk.Count(); ++row)
            recovery[row].recovered = ConsumeSuppressionRecovery(suppression[row], context.Tick());
    }
};
}
export namespace ecs
{
template<> struct SystemTraits<generalszh::power::PowerRecoverySystem>
{
    static constexpr std::string_view StableName = "games.generalszh.power.recovery_system";
    static constexpr SystemPhase Phase = SystemPhase::Simulation;
    using Before = SystemTypeList<>;
    using After = SystemTypeList<>;
};
}
