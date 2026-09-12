module;
#include <cstddef>
#include <string_view>
export module games.generalszh.gameplay.concealment.systems.stealth_policy_system;
export import engine.ecs.system.system;
export import engine.gameplay.combat.components.health;
export import engine.gameplay.combat.components.weapon;
export import engine.gameplay.concealment.components.concealment_binding;
export import engine.gameplay.concealment.components.concealment_eligibility;
export import engine.gameplay.concealment.systems.concealment_system;
export import engine.gameplay.navigation.components.movement_activity;
export import games.generalszh.gameplay.production.components.production_state;
export import games.generalszh.gameplay.concealment.components.stealth_binding;
export import games.generalszh.gameplay.concealment.definitions.stealth_policy;

export namespace generalszh::concealment
{
class StealthPolicySystem
{
public:
    using OwnerProjection = ecs::Query<ecs::Read<engine::gameplay::combat::LifeState>,
        ecs::Read<production::ProducedUnit>, ecs::Optional<StealthPolicyBinding>,
        ecs::Optional<engine::gameplay::concealment::DetectionBinding>,
        ecs::Write<engine::gameplay::concealment::ConcealmentGroup>>;
    using AuxiliaryAccess = OwnerProjection;
    using Query = ecs::Query<ecs::Read<engine::gameplay::combat::LifeState>, ecs::Read<StealthPolicyBinding>,
        ecs::Write<engine::gameplay::concealment::ConcealmentEligibility>,
        ecs::Read<engine::gameplay::navigation::MovementActivity>,
        ecs::Optional<engine::gameplay::combat::WeaponState>>;

    StealthPolicySystem(const StealthPolicies &policies, ecs::World &world) : policies(policies), owners(world) {}

    void BeforeChunks(Query &, ecs::SystemContext &)
    {
        owners.ForEachChunk([](auto chunk) {
            const auto lives = chunk.template Get<engine::gameplay::combat::LifeState>();
            const auto units = chunk.template Get<production::ProducedUnit>();
            const auto policies = chunk.template Get<StealthPolicyBinding>();
            const auto detectors = chunk.template Get<engine::gameplay::concealment::DetectionBinding>();
            auto groups = chunk.template Get<engine::gameplay::concealment::ConcealmentGroup>();
            for (std::size_t row = 0; row != chunk.Count(); ++row)
                groups[row].value = lives[row].alive && (!policies.empty() || !detectors.empty())
                    ? units[row].account : ecs::Entity{};
        });
    }

    void Execute(Query::Chunk chunk, ecs::SystemContext &context) const noexcept
    {
        const auto lives = chunk.Get<engine::gameplay::combat::LifeState>();
        const auto bindings = chunk.Get<StealthPolicyBinding>();
        auto eligibility = chunk.Get<engine::gameplay::concealment::ConcealmentEligibility>();
        const auto movement = chunk.Get<engine::gameplay::navigation::MovementActivity>();
        const auto weapons = chunk.Get<engine::gameplay::combat::WeaponState>();
        const auto tick = context.Tick();
        for (std::size_t row = 0; row != chunk.Count(); ++row)
        {
            const auto policy = policies.Get(bindings[row].policy);
            const bool isMoving = movement[row].cellsMovedThisTick != 0;
            const bool firedPreviousTick = !weapons.empty() && weapons[row].lastFireTickValid && tick != 0 &&
                weapons[row].lastFireTick == tick - 1;
            const bool firing = !weapons.empty() && (weapons[row].preAttackActive || firedPreviousTick);
            bool allowed = lives[row].alive && policy.innateStealth;
            if (policy.Forbids(StealthForbiddenCondition::Moving)) allowed = allowed && !isMoving;
            if (policy.Forbids(StealthForbiddenCondition::Firing)) allowed = allowed && !firing;
            eligibility[row].allowed = allowed;
        }
    }

private:
    const StealthPolicies &policies;
    OwnerProjection owners;
};
}

export namespace ecs
{
template<> struct SystemTraits<generalszh::concealment::StealthPolicySystem>
{
    static constexpr std::string_view StableName="games.generalszh.concealment.policy";
    static constexpr SystemPhase Phase=SystemPhase::Simulation;
    using Before=SystemTypeList<engine::gameplay::concealment::ConcealmentSystem>;
    using After=SystemTypeList<>;
};
}
