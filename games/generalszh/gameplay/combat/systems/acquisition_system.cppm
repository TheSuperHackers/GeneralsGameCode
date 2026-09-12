module;
#include <string_view>
export module games.generalszh.gameplay.combat.systems.acquisition_system;
export import games.generalszh.gameplay.combat.components.acquisition_policy;
export import games.generalszh.gameplay.combat.systems.targeting_system;
export import games.generalszh.gameplay.combat.targeting.target_index;
export import engine.gameplay.rts.orders.algorithms.advance_engagement;
export namespace generalszh::combat
{
struct AcquisitionSystem
{
    explicit AcquisitionSystem(TargetIndex &index) : index(index), frame(index.Frame()) {}
    // The target index is an injected cache, but its visibility page reads
    // still participate in scheduler conflict detection explicitly.
    using AuxiliaryAccess = TargetIndex::VisibilityAccess;
    using Query = ecs::Query<ecs::Read<engine::gameplay::combat::LifeState>, ecs::Read<AcquisitionPolicy>,
        ecs::Read<engine::gameplay::combat::WeaponDefinition>, ecs::Write<TargetIntent>, ecs::Write<engine::gameplay::combat::WeaponTarget>,
        ecs::Optional<capture::CaptureActorState>, ecs::Optional<engine::gameplay::rts::orders::UnitOrder>>;
    void BeforeChunks(Query &, ecs::SystemContext &context) { index.Rebuild(context.Tick()); }
    void Execute(Query::Chunk chunk, ecs::SystemContext &) const noexcept
    {
        using namespace engine::gameplay::combat;
        const auto life = chunk.Get<LifeState>(); const auto policies = chunk.Get<AcquisitionPolicy>(); const auto weapons = chunk.Get<WeaponDefinition>();
        auto intents = chunk.Get<TargetIntent>(); auto targets = chunk.Get<WeaponTarget>();
        const auto captures=chunk.Get<capture::CaptureActorState>();
        const auto orders=chunk.Get<engine::gameplay::rts::orders::UnitOrder>();
        for (std::size_t row = 0; row != chunk.Count(); ++row)
        {
            const auto actor = chunk.Entities()[row];
            if (!life[row].alive || (!captures.empty() && capture::IsCaptureBusy(captures[row])))
            { targets[row] = {}; intents[row] = {}; continue; }
            if (!orders.empty() && orders[row].kind==engine::gameplay::rts::orders::OrderKind::AttackPosition)
            {
                // Position identity is already projected by OrderSystem. It
                // is explicit even when no entity occupies the authored cell;
                // acquisition must not reinterpret it as a nearest entity.
                if (!targets[row].IsPosition()) { targets[row] = {}; intents[row] = {}; }
                else intents[row].explicitOrder = true;
                continue;
            }
            if (!frame.Contains(actor)) { targets[row] = {}; intents[row] = {}; continue; }
            if (!orders.empty() && orders[row].kind==engine::gameplay::rts::orders::OrderKind::GuardPosition)
            {
                const auto canonical=orders[row].target;
                const bool valid=frame.CanTarget(actor, canonical) &&
                    engine::gameplay::rts::orders::IsWithinGridRadius(orders[row].destination,frame.Position(canonical),
                        frame.Width(),orders[row].guard.outerRadiusCells);
                targets[row]=valid ? engine::gameplay::combat::WeaponTarget::ForEntity(canonical) : engine::gameplay::combat::WeaponTarget{};
                intents[row].explicitOrder=false;
                continue;
            }
            const auto target=targets[row].entity;
            const bool valid = targets[row].IsEntity() && frame.CanTarget(actor, target);
            if (intents[row].explicitOrder && valid) continue; // Explicit target may be outside current weapon range.
            intents[row].explicitOrder = false;
            if (!policies[row].enabled) { if (!valid) targets[row] = {}; continue; }
            targets[row]=engine::gameplay::combat::WeaponTarget::ForEntity(frame.Nearest(actor,weapons[row].rangeCells,
                UnitTarget | (policies[row].attackBuildings ? BuildingTarget : 0)));
        }
    }
private:
    TargetIndex &index;
    const TargetFrame &frame;
};
}
export namespace ecs
{
template<> struct SystemTraits<generalszh::combat::AcquisitionSystem>
{
    static constexpr std::string_view StableName = "games.generalszh.combat.acquire";
    static constexpr SystemPhase Phase = SystemPhase::PreSimulation;
    using Before = SystemTypeList<generalszh::combat::TargetingSystem>; using After = SystemTypeList<>;
};
}
