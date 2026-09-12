module;
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string_view>
export module games.generalszh.gameplay.demolition.systems.demolition_trap_system;
export import games.generalszh.gameplay.demolition.algorithms.demolition_target_index;
export import games.generalszh.gameplay.demolition.definitions.demolition_trap_definition;
export import games.generalszh.gameplay.demolition.inputs.demolition_trap_inputs;
export import games.generalszh.gameplay.construction.components.structure;
export import games.generalszh.gameplay.selling.components.sale_state;
export import engine.ecs.system.system;
export import engine.gameplay.combat.components.health;
export import engine.gameplay.combat.components.projectile;
export import engine.gameplay.combat.death.components.death_weapon_state;
export import engine.gameplay.navigation.components.movement;

export namespace generalszh::demolition
{
class DemolitionTrapSystem
{
public:
    using Query=ecs::Query<ecs::Read<construction::Structure>,
        ecs::Read<engine::gameplay::navigation::GridPosition>,
        ecs::Write<engine::gameplay::combat::Health>,
        ecs::Write<engine::gameplay::combat::LifeState>,
        ecs::Read<engine::gameplay::combat::death::DeathWeaponBinding>,
        ecs::Write<DemolitionTrapState>,ecs::Optional<selling::SaleState>>;
    using AuxiliaryAccess=DemolitionTargetIndex::Access;

    DemolitionTrapSystem(const DemolitionTrapCatalog &catalog,
        DemolitionTargetIndex &targets,DemolitionTrapInputBatch &inputs) noexcept :
        catalog_(catalog),targets_(targets),inputs_(inputs) {}

    void BeforeChunks(Query &,ecs::SystemContext &)
    {
        targets_.Rebuild();
    }

    void AfterChunks(Query &,ecs::SystemContext &) noexcept
    {
        inputs_.Clear();
    }

    void Execute(Query::Chunk chunk,ecs::SystemContext &context)
    {
        const auto structures=chunk.template Get<construction::Structure>();
        const auto positions=chunk.template Get<engine::gameplay::navigation::GridPosition>();
        auto health=chunk.template Get<engine::gameplay::combat::Health>();
        auto life=chunk.template Get<engine::gameplay::combat::LifeState>();
        const auto bindings=chunk.template Get<engine::gameplay::combat::death::DeathWeaponBinding>();
        auto states=chunk.template Get<DemolitionTrapState>();
        const auto sales=chunk.template Get<selling::SaleState>();
        for (std::size_t row=0;row!=chunk.Count();++row)
        {
            const auto *entry=catalog_.Find(bindings[row].definition);
            if (!entry || !structures[row].complete ||
                (!sales.empty() && sales[row].phase!=selling::SalePhase::Idle))
                continue;
            auto &state=states[row];
            if (state.detonated || state.selfDeathApplied) continue;
            const auto entity=chunk.Entities()[row];
            if (!life[row].alive || !health[row].current)
            {
                if (entry->detonateWhenKilled)
                {
                    if (entry->detonationWeapon)
                        Emit(*entry->detonationWeapon,entity,structures[row].account,structures[row].definition,
                            positions[row].cell,targets_.Width(),context);
                    state.detonated=true;
                }
                continue;
            }
            DemolitionDetonationMode commandedMode{};
            if (inputs_.ModeCommand(entity,commandedMode)) state.mode=commandedMode;
            bool trigger=false;
            // The explicit detonation command is independent of the current
            // mode.  Manual mode only disables the autonomous proximity scan.
            trigger=inputs_.Requested(entity);
            if (!trigger && state.mode!=DemolitionDetonationMode::Manual && context.Tick()>=state.nextScanTick)
            {
                state.nextScanTick=AddTicks(context.Tick(),entry->scanIntervalTicks);
                bool friendlyVeto=false;
                bool enemyGroundFound=false;
                const auto center=engine::gameplay::spatial::GridPoint{
                    positions[row].cell%targets_.Width(),positions[row].cell/targets_.Width()};
                targets_.VisitWithinRadius(center,entry->radiusCells,[&](const auto &observation) {
                    // The reference's position overload passes a null queried
                    // object, so it does not self-filter here.  STRUCTURE is
                    // an authored IgnoreTargetTypes policy when self-filtering
                    // is desired; do not invent an unconditional exclusion.
                    if (observation.disarmingDozer) return;
                    if (observation.airborne && entry->ignoreTargetTypes.airborne) return;
                    if (entry->ignoreTargetTypes.structures && observation.structure) return;
                    if (entry->ignoreTargetTypes.unattackable && observation.unattackable) return;
                    const bool enemy=observation.group.IsValid() && observation.group!=structures[row].account;
                    // Ignored targets are removed before the legacy ally/neutral
                    // veto.  Airborne exclusion follows that veto exactly as in
                    // the reference loop.  Visitation order cannot change the
                    // final reduction.
                    if (!enemy)
                    {
                        if (!entry->friendlyDetonation) friendlyVeto=true;
                        return;
                    }
                    if (observation.airborne) return;
                    enemyGroundFound=true;
                });
                trigger=enemyGroundFound && !friendlyVeto;
            }
            if (!trigger) continue;
            if (entry->detonationWeapon)
                Emit(*entry->detonationWeapon,entity,structures[row].account,structures[row].definition,
                    positions[row].cell,targets_.Width(),context);
            health[row].current=0;
            life[row].alive=false;
            state.detonated=true;
            state.selfDeathApplied=true;
        }
    }

private:
    static std::uint64_t AddTicks(const std::uint64_t tick,const std::uint64_t delay)
    {
        if (delay>(std::numeric_limits<std::uint64_t>::max)()-tick)
            throw std::overflow_error("Demolition trap scan deadline exceeds simulation tick range");
        return tick+delay;
    }

    static void Emit(const engine::gameplay::combat::WeaponDefinition &weapon,ecs::Entity source,
        ecs::Entity account,std::uint32_t definition,engine::gameplay::navigation::Cell cell,
        std::uint32_t width,ecs::SystemContext &context)
    {
        const auto projectile=context.Commands().Create();
        const auto flight=(std::max)(std::uint64_t{1},weapon.flightTicks);
        engine::gameplay::combat::ProjectileImpact impact{};
        impact.source=source; impact.sourceAccount=account; impact.sourceDefinition=definition;
        impact.damage=weapon.damage; impact.secondaryDamage=weapon.secondaryDamage;
        impact.launchTick=context.Tick(); impact.arrivalTick=AddTicks(context.Tick(),flight);
        impact.damageType=weapon.damageType; impact.damageChannel=weapon.damageChannel;
        impact.primaryRadiusCells=weapon.primaryRadiusCells; impact.secondaryRadiusCells=weapon.secondaryRadiusCells;
        impact.radiusDamageAffects=weapon.radiusDamageAffects; impact.positionalTarget=true;
        impact.detonationPositionValid=true; impact.detonationX=cell%width; impact.detonationY=cell/width;
        context.Commands().Add<engine::gameplay::combat::ProjectileImpact>(projectile,impact);
        context.Commands().Add<engine::gameplay::combat::ImpactDue>(projectile);
    }

    const DemolitionTrapCatalog &catalog_;
    DemolitionTargetIndex &targets_;
    DemolitionTrapInputBatch &inputs_;
};
}

export namespace ecs
{
template<> struct SystemTraits<generalszh::demolition::DemolitionTrapSystem>
{
    static constexpr std::string_view StableName="games.generalszh.demolition.trap";
    static constexpr SystemPhase Phase=SystemPhase::Simulation;
    using Before=SystemTypeList<>;
    using After=SystemTypeList<>;
};
}
