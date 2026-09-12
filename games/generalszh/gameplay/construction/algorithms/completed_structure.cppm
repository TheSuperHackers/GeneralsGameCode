module;
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string_view>
export module games.generalszh.gameplay.construction.algorithms.completed_structure;
export import games.generalszh.gameplay.construction.definitions.building_catalog;
export import games.generalszh.gameplay.bounty.components.cash_bounty_cost_binding;
export import games.generalszh.gameplay.capture.components.capture_state;
export import games.generalszh.gameplay.match.systems.defeat_system;
export import games.generalszh.gameplay.selling.components.sale_state;
export import games.generalszh.gameplay.harvesting.revenue.harvest_revenue;
export import games.generalszh.gameplay.production.rally.components.rally_point;
export import games.generalszh.gameplay.production.runtime.production_runtime;
export import games.generalszh.gameplay.demolition.definitions.demolition_trap_definition;
export import games.generalszh.gameplay.demolition.components.demolition_trap;
export import engine.gameplay.combat.death.components.death_weapon_state;
export import engine.gameplay.rts.power.components.power_source;
export import engine.gameplay.rts.repair.components.repair_state;
export import engine.gameplay.rts.repair.components.manual_repair;
export import engine.gameplay.combat.components.health;
export import engine.gameplay.combat.damage.components.damage_packet;
export import engine.gameplay.navigation.grid.navigation_grid;
export import engine.gameplay.rts.economy.components.resource_balance;
export import engine.ecs.commands.command_buffer;
export import engine.gameplay.rts.visibility.components.visibility_observer;

namespace generalszh::construction_detail
{
struct ImmediateStructureCommands
{
    ecs::World &world;

    template<typename Component>
    void Add(ecs::Entity entity)
    {
        if (!world.Add<Component>(entity))
            throw std::logic_error("Completed structure capability was already present");
    }

    template<typename Component>
    void Add(ecs::Entity entity, Component value)
    {
        if (!world.Add<Component>(entity))
            throw std::logic_error("Completed structure capability was already present");
        *world.Get<Component>(entity) = value;
    }
};
}

export namespace generalszh::construction
{
// Shared enrollment for both live construction completion and explicit
// startup structures. The sink is either the scheduler command buffer or the
// immediate setup sink; no capability manager or additional system is created.
template<typename CommandSink>
void EnrollCompletedStructureCapabilities(CommandSink &commands, ecs::Entity entity,
    Structure structure, const engine::gameplay::rts::repair::RepairDefinitions *repairDefinitions,
    engine::time::SimulationTime now, const bool hasVisibilityObserver = false,
    const bool hasVisibilityEligibility = false)
{
    if (structure.capturable)
    {
        commands.template Add<capture::Capturable>(entity, capture::Capturable{true});
        commands.template Add<capture::CaptureState>(entity);
    }
    if (structure.visibility)
    {
        if (!hasVisibilityObserver)
            commands.template Add<engine::gameplay::rts::visibility::VisibilityObserver>(entity,
                engine::gameplay::rts::visibility::VisibilityObserver{{}, *structure.visibility});
        if (!hasVisibilityEligibility)
            commands.template Add<engine::gameplay::rts::visibility::VisibilityEligibility>(entity);
    }
    // Every completed structure is a valid manual-repair target candidate
    // only after this finite, target-owned lease column exists. The lease is
    // not a travel reservation and remains empty until a builder repairs.
    commands.template Add<engine::gameplay::rts::repair::ManualRepairBenefactorLease>(entity);
    if (structure.repairDefinition)
    {
        if (!repairDefinitions)
            throw std::invalid_argument("Completed structure repair requires a repair catalog");
        using namespace engine::gameplay::rts::repair;
        const auto definition = *structure.repairDefinition;
        commands.template Add<RepairBinding>(entity, RepairBinding{definition});
        commands.template Add<RepairState>(entity, StartRepair(*repairDefinitions, definition, now));
    }
    commands.template Add<selling::SaleState>(entity);
    commands.template Add<selling::SaleEligibility>(entity, selling::SaleEligibility{!structure.supplyDropoff});
    if (structure.queueLimit)
    {
        commands.template Add<production::Producer>(entity,
            production::Producer{structure.account, structure.queueLimit, true});
        commands.template Add<engine::gameplay::rts::production::ProductionQueue>(entity);
        commands.template Add<production::rally::RallyPoint>(entity);
    }
    commands.template Add<match::VictoryAsset>(entity, match::VictoryAsset{structure.account});
    if (structure.supplyDropoff)
    {
        commands.template Add<engine::gameplay::rts::harvesting::SupplyDropoff>(entity);
        commands.template Add<harvesting::SupplyDropoffOwner>(entity,
            harvesting::SupplyDropoffOwner{structure.account, structure.supplyBoxValue});
    }
    if (structure.energy)
    {
        commands.template Add<engine::gameplay::rts::power::PowerSource>(entity,
            engine::gameplay::rts::power::PowerSource{structure.account, structure.energy, true});
        commands.template Add<engine::gameplay::rts::power::PowerContribution>(entity);
    }
}

// Explicit initial-world setup, kept outside the tick systems. The caller must
// supply an authored catalog entry; no prerequisite is fulfilled implicitly and
// construction cost is not charged for this pre-existing starting facility.
inline ecs::Entity CreateCompletedStructure(ecs::World &world,
    const engine::gameplay::navigation::NavigationGrid &grid, const BuildingCatalog &catalog,
    const engine::gameplay::rts::repair::RepairDefinitions *repairDefinitions,
    ecs::Entity account, std::uint32_t definitionKey, engine::gameplay::navigation::Cell cell,
    engine::time::SimulationTime now,
    const demolition::DemolitionTrapCatalog *demolitionCatalog=nullptr)
{
    if (!world.IsAlive(account) || !world.Get<engine::gameplay::rts::economy::ResourceBalance>(account))
        throw std::invalid_argument("Initial structure requires a live account");
    if (now.Step() != catalog.Step())
        throw std::invalid_argument("Initial structure step mismatch");
    if (!grid.Walkable(cell))
        throw std::invalid_argument("Initial structure cell must be traversable");
    const auto *entry = catalog.Find(definitionKey);
    if (!entry)
        throw std::invalid_argument("Initial structure definition is not authored");
    if (entry->definition.repairDefinition && !repairDefinitions)
        throw std::invalid_argument("Initial structure repair requires a repair catalog");

    const auto &definition = entry->definition;
    const auto entity = world.Create<Structure, engine::gameplay::navigation::GridPosition,
        engine::gameplay::combat::Health, engine::gameplay::combat::HealthCapacity,
        engine::gameplay::combat::LifeState, engine::gameplay::combat::PendingDamage,
        engine::gameplay::combat::DamageResult, engine::gameplay::combat::PendingHealing,
        engine::gameplay::combat::HealingResult>();
    try
    {
        *world.Get<Structure>(entity) = {account, definition.key, definition.queueLimit, definition.energy, true,
            definition.supplyDropoff, definition.supplyBoxValue, definition.repairDefinition,
            definition.capturable, definition.armor,
            definition.visibility ? std::optional{definition.visibility->id} : std::nullopt};
        *world.Get<engine::gameplay::navigation::GridPosition>(entity) = {cell};
        *world.Get<engine::gameplay::combat::Health>(entity) = {definition.health};
        *world.Get<engine::gameplay::combat::HealthCapacity>(entity) = {definition.health};
        if (demolitionCatalog)
            if (const auto *trap=demolitionCatalog->Find(definition.key))
            {
                construction_detail::ImmediateStructureCommands commands{world};
                commands.Add<engine::gameplay::combat::death::DeathWeaponBinding>(entity,
                    engine::gameplay::combat::death::DeathWeaponBinding{trap->key});
                commands.Add<engine::gameplay::combat::death::DeathWeaponState>(entity);
                commands.Add<demolition::DemolitionTrapState>(entity,
                    demolition::DemolitionTrapState{0,trap->mode,false,false});
            }
        if (definition.armor)
        {
            if (!world.Add<engine::gameplay::combat::damage::ArmorBinding>(entity))
                throw std::logic_error("Initial structure armor enrollment failed");
            *world.Get<engine::gameplay::combat::damage::ArmorBinding>(entity) = *definition.armor;
        }
        construction_detail::ImmediateStructureCommands commands{world};
        if (definition.cashBountyCost)
            commands.Add<generalszh::bounty::CashBountyCostBinding>(entity,*definition.cashBountyCost);
        EnrollCompletedStructureCapabilities(commands, entity, *world.Get<Structure>(entity), repairDefinitions, now);
    }
    catch (...)
    {
        world.Destroy(entity);
        throw;
    }
    return entity;
}
}
