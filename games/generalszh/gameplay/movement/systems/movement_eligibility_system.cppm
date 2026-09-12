module;
#include <string_view>
export module games.generalszh.gameplay.movement.systems.movement_eligibility_system;
export import engine.gameplay.combat.systems.health_system;
export import engine.gameplay.navigation.systems.movement_system;
export import engine.gameplay.containment.components.passenger_membership;
export import games.generalszh.gameplay.capture.components.capture_state;
export namespace generalszh::movement
{
struct MovementEligibilitySystem
{
    using Query = ecs::Query<ecs::Read<engine::gameplay::combat::LifeState>, ecs::Write<engine::gameplay::navigation::MoveEnabled>,
        ecs::Optional<engine::gameplay::containment::PassengerMembership>,ecs::Optional<capture::CaptureActorState>>;
    void Execute(Query::Chunk chunk, ecs::SystemContext &) const noexcept
    {
        const auto life = chunk.Get<engine::gameplay::combat::LifeState>();
        auto enabled = chunk.Get<engine::gameplay::navigation::MoveEnabled>();
        const auto members=chunk.Get<engine::gameplay::containment::PassengerMembership>();
        const auto captures=chunk.Get<capture::CaptureActorState>();
        for (std::size_t row = 0; row != chunk.Count(); ++row)
            enabled[row].value = life[row].alive && (members.empty() || !engine::gameplay::containment::IsContained(members[row]))
                && (captures.empty() || !capture::IsCaptureBusy(captures[row]));
    }
};
}
export namespace ecs
{
template<> struct SystemTraits<generalszh::movement::MovementEligibilitySystem>
{
    static constexpr std::string_view StableName = "games.generalszh.movement.eligibility";
    static constexpr SystemPhase Phase = SystemPhase::PreSimulation;
    using Before = SystemTypeList<>; using After = SystemTypeList<>;
};
}
