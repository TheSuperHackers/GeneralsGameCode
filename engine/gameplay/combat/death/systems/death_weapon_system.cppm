module;
#include <algorithm>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <type_traits>
export module engine.gameplay.combat.death.systems.death_weapon_system;
export import engine.ecs.system.system;
export import engine.gameplay.combat.components.health;
export import engine.gameplay.combat.components.projectile;
export import engine.gameplay.combat.death.components.death_weapon_state;
export import engine.gameplay.combat.death.definitions.death_weapon_definition;

export namespace engine::gameplay::combat::death
{
template<typename SourceProjection>
struct DeathWeaponSystemT
{
    using Query=ecs::Query<ecs::Read<engine::gameplay::combat::Health>,
        ecs::Read<engine::gameplay::combat::LifeState>,
        ecs::Read<DeathWeaponBinding>,ecs::Write<DeathWeaponState>>;
    using AuxiliaryAccess=typename SourceProjection::Access;

    DeathWeaponSystemT(const DeathWeaponCatalog &catalog, SourceProjection &projection) noexcept :
        catalog_(catalog),projection_(projection) {}

    void BeforeChunks(Query &, ecs::SystemContext &)
    {
        projection_.Refresh();
    }

    void Execute(Query::Chunk chunk, ecs::SystemContext &context)
    {
        const auto health=chunk.template Get<engine::gameplay::combat::Health>();
        const auto life=chunk.template Get<engine::gameplay::combat::LifeState>();
        const auto bindings=chunk.template Get<DeathWeaponBinding>();
        auto states=chunk.template Get<DeathWeaponState>();
        for (std::size_t row=0;row!=chunk.Count();++row)
        {
            auto &state=states[row];
            if (!bindings[row].definition || state.suppressed ||
                health[row].current!=0 || life[row].alive)
                continue;
            const auto *entry=catalog_.Find(bindings[row].definition);
            if (!entry) throw std::logic_error("Death weapon binding has no authored catalog entry");
            const auto entity=chunk.Entities()[row];
            if (!state.armed)
            {
                const auto capture=projection_.Capture(entity);
                if (capture.disposition==DeathSourceDisposition::Exempt)
                {
                    state.suppressed=true;
                    continue;
                }
                if (capture.disposition!=DeathSourceDisposition::Eligible)
                    throw std::logic_error("Death weapon source capture failed for a dead entity");
                state.source={entity,capture.account,capture.definition,capture.position};
                state.deathTick=context.Tick();
                state.destructionDeadline=AddTicks(context.Tick(),entry->destructionDelayTicks);
                state.armed=true;
                if (entry->initial)
                {
                    Emit(*entry->initial,state,context);
                    state.initialEmitted=true;
                }
            }
            if (state.armed && !state.finalEmitted && context.Tick()>=state.destructionDeadline)
            {
                if (entry->final)
                {
                    Emit(*entry->final,state,context);
                    state.finalEmitted=true;
                }
                state.destructionQueued=true;
                context.Commands().Destroy(entity);
            }
        }
    }

private:
    static std::uint64_t AddTicks(const std::uint64_t tick,const std::uint64_t delay)
    {
        if (delay>(std::numeric_limits<std::uint64_t>::max)()-tick)
            throw std::overflow_error("Death weapon deadline exceeds simulation tick range");
        return tick+delay;
    }

    static void Emit(const engine::gameplay::combat::WeaponDefinition &weapon,
        const DeathWeaponState &state, ecs::SystemContext &context)
    {
        const auto projectile=context.Commands().Create();
        const auto flight=(std::max)(std::uint64_t{1},weapon.flightTicks);
        const auto arrival=AddTicks(context.Tick(),flight);
        engine::gameplay::combat::ProjectileImpact impact{};
        impact.source=state.source.entity;
        impact.damage=weapon.damage;
        impact.launchTick=context.Tick();
        impact.arrivalTick=arrival;
        impact.damageType=weapon.damageType;
        impact.damageChannel=weapon.damageChannel;
        impact.sourceAccount=state.source.account;
        impact.sourceDefinition=state.source.definition;
        impact.secondaryDamage=weapon.secondaryDamage;
        impact.primaryRadiusCells=weapon.primaryRadiusCells;
        impact.secondaryRadiusCells=weapon.secondaryRadiusCells;
        impact.radiusDamageAffects=weapon.radiusDamageAffects;
        impact.detonationPositionValid=true;
        impact.detonationX=state.source.position.x;
        impact.detonationY=state.source.position.y;
        impact.positionalTarget=true;
        context.Commands().Add<engine::gameplay::combat::ProjectileImpact>(projectile,impact);
        context.Commands().Add<engine::gameplay::combat::ImpactDue>(projectile);
    }

    const DeathWeaponCatalog &catalog_;
    SourceProjection &projection_;
};
}

export namespace ecs
{
template<typename SourceProjection>
struct SystemTraits<engine::gameplay::combat::death::DeathWeaponSystemT<SourceProjection>>
{
    static constexpr std::string_view StableName="engine.gameplay.combat.death_weapon";
    static constexpr SystemPhase Phase=SystemPhase::PostSimulation;
    using Before=SystemTypeList<>;
    using After=SystemTypeList<>;
};
}
