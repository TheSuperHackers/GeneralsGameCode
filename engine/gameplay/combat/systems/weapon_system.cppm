module;
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <vector>
export module engine.gameplay.combat.systems.weapon_system;
export import engine.gameplay.combat.systems.health_system;
export import engine.gameplay.containment.components.passenger_membership;
export import engine.gameplay.combat.damage.components.damage_packet;
export import engine.gameplay.combat.definitions.weapon_definition;
export import engine.gameplay.combat.components.projectile;
export import engine.gameplay.spatial.grid.point_grid;
export namespace engine::gameplay::combat
{
struct NoSpatialLaunchSnapshot
{
    static constexpr bool Spatial = false;
    struct Access
    {
        static std::vector<ecs::AccessDescriptor> ResolveAccesses(const ecs::ComponentRegistry &)
        {
            return {};
        }
    };
    bool TryGet(ecs::Entity, engine::gameplay::spatial::SpatialPoint &) const noexcept { return false; }
};

template<typename LaunchSnapshot = NoSpatialLaunchSnapshot>
struct WeaponSystemT
{
    WeaponSystemT() requires std::is_default_constructible_v<LaunchSnapshot> = default;
    explicit WeaponSystemT(LaunchSnapshot snapshot) : launchSnapshot(snapshot) {}

    using Query = ecs::Query<ecs::Read<LifeState>, ecs::Read<WeaponDefinition>, ecs::Read<WeaponTarget>,
        ecs::Read<WeaponContact>, ecs::Write<WeaponState>,ecs::Optional<engine::gameplay::containment::PassengerMembership>,
        ecs::Optional<DamageEmitter>>;
    using AuxiliaryAccess = typename LaunchSnapshot::Access;
    void Execute(Query::Chunk chunk, ecs::SystemContext &context) const
    {
        const auto life = chunk.Get<LifeState>(); const auto definitions = chunk.Get<WeaponDefinition>();
        const auto targets = chunk.Get<WeaponTarget>(); const auto contacts = chunk.Get<WeaponContact>(); auto states = chunk.Get<WeaponState>();
        const auto membership=chunk.Get<engine::gameplay::containment::PassengerMembership>();
        const auto emitters=chunk.Get<DamageEmitter>();
        const auto tick = context.Tick(); auto &commands = context.Commands();
        for (std::size_t row = 0; row != chunk.Count(); ++row)
        {
            auto &state = states[row]; const auto &definition = definitions[row];
            const bool contained = !membership.empty() && engine::gameplay::containment::IsContained(membership[row]);
            if (!life[row].alive || (contained && !engine::gameplay::containment::CanPassengerFire(membership[row])))
            {
                // Death and containment cancel an unfinished action episode, but
                // retain the existing cooldown/ammunition contract.
                ResetAttackEpisode(state);
                continue;
            }
            ValidateDefinition(definition);
            const bool limited = definition.ammunition == AmmunitionPolicy::Limited;
            // Exhausting the representable simulation horizon is a lifecycle
            // failure in every build, before reload/ammo or command mutations.
            if (tick > (std::numeric_limits<std::uint64_t>::max)() -
                std::max({definition.flightTicks, definition.shotTicks, limited ? definition.reloadTicks : 0u, definition.preAttackTicks}))
                throw std::invalid_argument("Weapon tick horizon is exhausted");

            const auto &target = targets[row];
            // WeaponContact is derived input, not authority for target
            // identity. A stale contact cannot turn a malformed dual-valid or
            // empty WeaponTarget into a launch.
            if (!target.IsValid())
            {
                ResetAttackEpisode(state);
                continue;
            }
            // A target switch cancels only unfinished windup and makes a
            // different generation unable to inherit another target's delay.
            if (state.preAttackActive && !SameIdentity(state.preAttackTarget, state.preAttackPosition,
                state.preAttackPositionValid, target)) ClearWindup(state);
            if (HasIdentity(state.lastAttackTarget, state.lastAttackPositionValid) &&
                !SameIdentity(state.lastAttackTarget, state.lastAttackPosition, state.lastAttackPositionValid, target))
                ClearLastAttack(state);
            if (limited && state.reloading && tick >= state.readyTick) { state.ammo = definition.clipSize; state.reloading = false; }

            if (!contacts[row].valid)
            {
                // An invalid target/explicit stop ends this bounded episode.
                // Range loss is deliberately different: it is handled below
                // without clearing an already armed same-target deadline.
                ResetAttackEpisode(state);
                continue;
            }
            if (!contacts[row].inRange || (limited && !state.ammo) || tick < state.readyTick ||
                (!definition.damage && !definition.secondaryDamage)) continue;

            if (definition.preAttackTicks && NeedsPreAttack(definition, state, target))
            {
                if (!state.preAttackActive)
                {
                    state.preAttackActive = true;
                    state.preAttackTarget = target.IsEntity() ? target.entity : ecs::Entity{};
                    state.preAttackPosition = target.IsPosition() ? target.position : spatial::GridPoint{};
                    state.preAttackPositionValid = target.IsPosition();
                    state.preAttackReadyTick = tick + definition.preAttackTicks;
                    continue;
                }
                if (tick < state.preAttackReadyTick) continue;
            }

            // A spatial shot must capture a real, published launch coordinate
            // before any shot state or structural command is changed. The
            // direct provider deliberately cannot make a missing point (0,0).
            engine::gameplay::spatial::SpatialPoint launchPoint{};
            const bool positionalTarget = target.IsPosition();
            bool detonationPositionValid = positionalTarget;
            if (positionalTarget)
            {
                launchPoint.x = target.position.x;
                launchPoint.y = target.position.y;
            }
            if constexpr (LaunchSnapshot::Spatial)
            {
                if (!positionalTarget && (definition.primaryRadiusCells || definition.secondaryRadiusCells))
                {
                    if (!launchSnapshot.TryGet(target.entity, launchPoint))
                        throw std::invalid_argument("Spatial weapon launch target has no published coordinate");
                    detonationPositionValid = true;
                }
            }
            ClearWindup(state);
            const auto projectile = commands.Create();
            const auto emitter=emitters.empty()?DamageEmitter{}:emitters[row];
            commands.Add<ProjectileImpact>(projectile, ProjectileImpact{chunk.Entities()[row],
                target.IsEntity() ? target.entity : ecs::Entity{}, definition.damage, tick,
                tick + definition.flightTicks, definition.damageType,definition.damageChannel,emitter.account,emitter.definition,
                definition.secondaryDamage,definition.primaryRadiusCells,definition.secondaryRadiusCells,definition.radiusDamageAffects,
                detonationPositionValid,launchPoint.x,launchPoint.y,positionalTarget});
            commands.Add<ImpactDue>(projectile);
            state.lastFireTickValid = true;
            state.lastFireTick = tick;
            if (limited) --state.ammo;
            state.reloading = limited && !state.ammo && definition.autoReload;
            state.readyTick = tick + (state.reloading ? definition.reloadTicks : definition.shotTicks);
            if (definition.preAttackType == PrefirePolicy::PerAttack && target.IsValid())
            {
                state.lastAttackTarget = target.IsEntity() ? target.entity : ecs::Entity{};
                state.lastAttackPosition = target.IsPosition() ? target.position : spatial::GridPoint{};
                state.lastAttackPositionValid = target.IsPosition();
            }
            else ClearLastAttack(state);
        }
    }
private:
    static void ValidateDefinition(const WeaponDefinition &definition)
    {
        if (definition.ammunition != AmmunitionPolicy::Limited && definition.ammunition != AmmunitionPolicy::Unlimited)
            throw std::invalid_argument("Invalid weapon ammunition policy");
        if (definition.ammunition == AmmunitionPolicy::Limited && (!definition.clipSize || !definition.flightTicks))
            throw std::invalid_argument("Invalid limited weapon definition");
        if (definition.ammunition == AmmunitionPolicy::Unlimited && !definition.flightTicks)
            throw std::invalid_argument("Invalid unlimited weapon definition");
        if (definition.preAttackType != PrefirePolicy::PerShot && definition.preAttackType != PrefirePolicy::PerAttack &&
            definition.preAttackType != PrefirePolicy::PerClip)
            throw std::invalid_argument("Invalid weapon pre-attack policy");
        if (!definition.radiusDamageAffects.IsValid())
            throw std::invalid_argument("Invalid radius damage affects mask");
        if constexpr (!LaunchSnapshot::Spatial)
            if (definition.primaryRadiusCells || definition.secondaryRadiusCells)
                throw std::invalid_argument("Spatial weapon requires a spatial launch snapshot");
    }
    static void ClearWindup(WeaponState &state) noexcept
    {
        state.preAttackActive = false;
        state.preAttackReadyTick = 0;
        state.preAttackTarget = {};
        state.preAttackPosition = {};
        state.preAttackPositionValid = false;
    }
    static void ResetAttackEpisode(WeaponState &state) noexcept
    {
        ClearWindup(state);
        ClearLastAttack(state);
    }
    static void ClearLastAttack(WeaponState &state) noexcept
    {
        state.lastAttackTarget = {};
        state.lastAttackPosition = {};
        state.lastAttackPositionValid = false;
    }
    static bool HasIdentity(const ecs::Entity entity, const bool positionValid) noexcept
    { return positionValid || entity.IsValid(); }
    static bool SameIdentity(const ecs::Entity entity, const spatial::GridPoint position,
        const bool positionValid, const WeaponTarget &target) noexcept
    {
        if (target.IsPosition()) return positionValid && position == target.position;
        if (target.IsEntity()) return !positionValid && entity == target.entity;
        return !HasIdentity(entity, positionValid);
    }
    static bool NeedsPreAttack(const WeaponDefinition &definition, const WeaponState &state,
        const WeaponTarget &target) noexcept
    {
        switch (definition.preAttackType)
        {
        case PrefirePolicy::PerShot: return true;
        case PrefirePolicy::PerAttack: return !target.IsValid() ||
            !SameIdentity(state.lastAttackTarget,state.lastAttackPosition,state.lastAttackPositionValid,target);
        case PrefirePolicy::PerClip: return definition.clipSize == 0 || state.ammo >= definition.clipSize;
        }
        assert(false && "Invalid weapon pre-attack policy");
        return true;
    }

    LaunchSnapshot launchSnapshot;
};

using WeaponSystem = WeaponSystemT<NoSpatialLaunchSnapshot>;
}
export namespace ecs
{
template<typename LaunchSnapshot>
struct SystemTraits<engine::gameplay::combat::WeaponSystemT<LaunchSnapshot>>
{
    static constexpr std::string_view StableName = "engine.gameplay.combat.weapon_fire";
    static constexpr SystemPhase Phase = SystemPhase::Simulation;
    using Before = SystemTypeList<>; using After = SystemTypeList<>;
};
}
