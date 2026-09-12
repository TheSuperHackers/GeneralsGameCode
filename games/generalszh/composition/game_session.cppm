module;
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>

export module games.generalszh.composition.game_session;
export import engine.ecs.core.entity;
export import engine.ecs.core.world;
export import engine.time.simulation_time;
export import engine.ecs.scheduler.scheduler;
export import engine.gameplay.combat.damage.components.damage_packet;
export import engine.gameplay.combat.damage.definitions.armor_catalog;
export import engine.gameplay.combat.damage.inputs.accepted_hit_batch;
export import engine.gameplay.combat.inputs.health_input_system;
export import engine.gameplay.combat.regeneration.definitions.regeneration_definition;
export import engine.gameplay.containment.definitions.transport_definition;
export import engine.gameplay.containment.inputs.containment_batch;
export import engine.gameplay.concealment.inputs.uncloak_input;
export import engine.gameplay.navigation.inputs.move_input;
export import engine.gameplay.progression.definitions.progression_definition;
export import engine.gameplay.progression.inputs.progression_batch;
export import engine.gameplay.rts.harvesting.components.harvest_state;
export import engine.gameplay.rts.ai.definitions.force_production_plan;
export import engine.gameplay.rts.orders.inputs.order_input;
export import engine.gameplay.rts.repair.definitions.repair_definition;
export import engine.gameplay.rts.rank.definitions.rank_definition;
export import engine.gameplay.rts.rank.inputs.rank_batch;
export import engine.gameplay.rts.rank.components.rank_skill_award_modifier;
export import engine.gameplay.rts.unlocks.definitions.unlock_catalog;
export import engine.gameplay.rts.unlocks.inputs.unlock_batch;
export import games.generalszh.gameplay.bounty.definitions.cash_bounty_definition;
export import games.generalszh.gameplay.bounty.inputs.cash_bounty_batch;
export import games.generalszh.gameplay.capture.definitions.capture_definition;
export import games.generalszh.gameplay.capture.inputs.capture_batch;
export import games.generalszh.gameplay.combat.inputs.poison_input;
export import games.generalszh.gameplay.construction.definitions.building_definition;
export import games.generalszh.gameplay.construction.inputs.construction_batches;
export import games.generalszh.gameplay.crates.components.crate_state;
export import games.generalszh.gameplay.crates.definitions.collector_definition;
export import games.generalszh.gameplay.crates.definitions.crate_definition;
export import games.generalszh.gameplay.crates.inputs.crate_pickup_batch;
export import games.generalszh.gameplay.demolition.definitions.demolition_trap_definition;
export import games.generalszh.gameplay.demolition.inputs.demolition_trap_inputs;
export import games.generalszh.gameplay.economy.transactions.account_batch;
export import games.generalszh.gameplay.match.components.match_state;
export import games.generalszh.gameplay.orders.inputs.attack_input;
export import games.generalszh.gameplay.production.definitions.build_definition;
export import games.generalszh.gameplay.production.doors.definitions.production_exit_definition;
export import games.generalszh.gameplay.production.inputs.build_inputs;
export import games.generalszh.gameplay.production.rally.inputs.rally_batch;
export import games.generalszh.gameplay.progression.definitions.veterancy_definition;
export import games.generalszh.gameplay.repair.inputs.manual_repair_batches;
export import games.generalszh.gameplay.selling.definitions.sell_definition;
export import games.generalszh.gameplay.selling.inputs.sell_batch;

export namespace generalszh
{
// Default authored scenarios use one ordinary direct-health channel. Content
// hosts inject their explicit, deterministically indexed type-policy schema.
inline constexpr std::array DefaultDirectDamagePolicies{
    engine::gameplay::combat::damage::DamageTypePolicy{engine::gameplay::combat::damage::ArmorApplication::Scale}};
// DozerAIUpdate gives the current frame and the next frame to its sole
// benefactor. The target lease uses an exclusive deadline, so the equivalent
// reference window is three legacy 30-Hz intervals: [acquire, acquire+3).
// This is a typed Zero Hour composition default, not a generic tick constant;
// callers may inject another Duration and FixedStep::TicksFor rounds it up.
inline constexpr engine::time::Duration ZeroHourManualRepairRetention=std::chrono::milliseconds{100};
using ForceProductionPolicyInput = engine::gameplay::rts::ai::ForceProductionPolicyInput;
using ForceProductionPlanLimits = engine::gameplay::rts::ai::ForceProductionPlanLimits;
struct GameInputs
{
    std::span<const engine::gameplay::combat::DamageInput> damage;
    std::span<const economy::AccountRequest> accounts;
    std::span<const BuildInput> builds;
    std::span<const engine::gameplay::navigation::MoveInput> moves;
    std::span<const AttackInput> attacks;
    std::span<const engine::gameplay::rts::orders::OrderInput> orders;
    std::span<const engine::gameplay::combat::HealingInput> healing;
    std::span<const PoisonInput> poison;
    std::span<const engine::gameplay::progression::AcceptedExperience> experience;
    std::span<const construction::ConstructionInput> construction;
    std::span<const ecs::Entity> constructionCancellations;
    std::span<const engine::gameplay::rts::harvesting::HarvestInput> harvest;
    std::span<const selling::SellRequest> sales;
    std::span<const engine::gameplay::containment::ContainmentInput> containment;
    std::span<const production::rally::RallyInput> rally;
    std::span<const capture::CaptureInput> capture;
    std::span<const repair::ManualRepairInput> manualRepair;
    std::span<const engine::gameplay::concealment::UncloakInput> uncloak;
    // Appended to preserve existing aggregate callers; the transport and
    // garrison batches have independent ownership and receipts.
    std::span<const engine::gameplay::containment::ContainmentInput> garrison;
    std::span<const demolition::DemolitionTrapInput> demolition;
    std::span<const engine::gameplay::rts::unlocks::UnlockRequest> unlocks;
    std::span<const engine::gameplay::rts::rank::AcceptedRankPoints> rankAwards;
    // Ordered contact facts supplied by the simulation boundary.  Crate
    // systems do not infer contact, distance or AI state from this input.
    std::span<const crates::CratePickupInput> crateContacts;
};

// Authored crate data is borrowed only for this call. ConfigureCrates copies
// every definition and collector row before Finalize builds immutable runtime
// catalogs, so simulation never retains content-adapter storage.
struct CrateConfiguration final
{
    std::span<const crates::CrateDefinition> definitions{};
    std::span<const crates::CollectorDefinition> unitCollectors{};
    std::span<const crates::CollectorDefinition> structureCollectors{};
    std::size_t referenceCount{};
    std::size_t outputAwards{};
};

// One game-instance composition root. The public module keeps only the typed
// boundary and result/query references; all composition state and systems live
// in the private implementation unit.
class GameSession
{
public:
    GameSession(ecs::World &world, engine::jobs::JobSystem &jobs,
        std::span<const production::BuildDefinition> definitions,
        const engine::gameplay::navigation::NavigationGrid &grid,
        std::size_t inputCapacity = 65536,
        std::span<const engine::gameplay::combat::regeneration::RegenerationDefinition> regenerationDefinitions = {},
        std::span<const engine::gameplay::progression::ProgressionDefinition> progressionDefinitions = {},
        std::span<const construction::BuildingDefinition> buildingDefinitions = {},
        std::span<const selling::SellDefinition> saleDefinitions = {},
        std::span<const engine::gameplay::rts::repair::RepairDefinition> repairDefinitions = {},
        std::span<const engine::gameplay::containment::TransportDefinition> transportDefinitions = {},
        std::span<const capture::CaptureDefinition> captureDefinitions = {},
        std::span<const engine::gameplay::combat::damage::DamageTypePolicy> damagePolicies = DefaultDirectDamagePolicies,
        std::span<const engine::gameplay::combat::damage::ArmorDefinition> armorDefinitions = {},
        engine::time::Duration manualRepairRetention = ZeroHourManualRepairRetention,
        std::size_t splashResultCapacity = 0, std::size_t acceptedHitCapacity = 0,
        std::span<const generalszh::progression::VeterancyDefinition> veterancyDefinitions = {},
        std::span<const demolition::DemolitionTrapDefinition> demolitionDefinitions = {},
        const engine::gameplay::rts::unlocks::UnlockCatalog *unlockCatalog = nullptr,
        const engine::gameplay::rts::rank::RankCatalog *rankCatalog = nullptr,
        std::size_t maximumVisibilityObservers = 4096,
        std::size_t maximumVisibilityRegionTransitions = 1,
        std::span<const generalszh::bounty::CashBountyDefinition> cashBountyDefinitions = {});

    ~GameSession() noexcept;

    GameSession(const GameSession &) = delete;
    GameSession &operator=(const GameSession &) = delete;
    GameSession(GameSession &&) = delete;
    GameSession &operator=(GameSession &&) = delete;

    void ConfigureForceProduction(std::span<const ForceProductionPolicyInput> policies,
        ForceProductionPlanLimits limits);
    void ConfigureCrates(CrateConfiguration configuration);
    static void RegisterComponents(ecs::World &world);
    void Finalize(engine::time::FixedStep rate);
    void RegisterController(ecs::Entity account, std::uint32_t policyKey, bool enabled = true);
    ecs::Entity CreateAccount(std::uint32_t balance, std::uint64_t unlockCredits = 0,
        std::uint64_t intrinsicScienceCredits = 0,
        engine::gameplay::rts::rank::RankSkillAwardModifier skillAwardModifier = {});
    void RegisterCrateAccountPolicy(ecs::Entity account, const bool human, const bool neutral);
    ecs::Entity CreateCrate(crates::CrateState state);
    ecs::Entity CreateProducer(ecs::Entity account, std::uint32_t limit = 9,
        std::uint64_t health = 1000, engine::gameplay::navigation::Cell spawn = 0,
        std::optional<production::ProductionExitDefinition> exit = std::nullopt);
    ecs::Entity CreateCompletedStructure(ecs::Entity account, std::uint32_t definition,
        engine::gameplay::navigation::Cell spawn = 0);
    ecs::Entity CreateSupplySource(std::uint32_t boxes, engine::gameplay::navigation::Cell cell,
        std::uint64_t health = 1000);
    ecs::Entity CreateSupplyDropoff(ecs::Entity account, engine::gameplay::navigation::Cell cell,
        std::uint32_t valuePerBox, std::uint64_t health = 1000);
    void BeginMatch(std::span<const ecs::Entity> roster);
    MatchOutcome Outcome() const;
    bool CancelBuild(ecs::Entity producer, ecs::Entity entry);
    const ecs::SystemRegistry &Systems() const noexcept;
    void EnableRegeneration(ecs::Entity entity, std::uint32_t definition,
        engine::time::SimulationTime time, engine::time::Duration phase, bool active = true);
    void EnrollProgression(ecs::Entity entity, std::uint32_t definition, bool trainable);
    std::span<const engine::gameplay::progression::ExperienceResult> ExperienceResults() const noexcept;
    std::span<const construction::ConstructionReceipt> ConstructionResults() const noexcept;
    std::span<const selling::SellReceipt> SaleReceipts() const noexcept;
    std::span<const selling::SellResult> SaleResults() const noexcept;
    std::span<const engine::gameplay::containment::ContainmentResult> ContainmentResults() const noexcept;
    std::span<const engine::gameplay::containment::ContainmentResult> GarrisonResults() const noexcept;
    std::span<const production::rally::RallyReceipt> RallyReceipts() const noexcept;
    std::span<const capture::CaptureResult> CaptureResults() const noexcept;
    std::span<const engine::gameplay::combat::damage::ResolvedHit> AcceptedHits() const noexcept;
    std::span<const engine::gameplay::rts::unlocks::UnlockReceipt> UnlockReceipts() const noexcept;
    std::span<const engine::gameplay::rts::rank::RankResult> RankResults() const noexcept;
    std::span<const generalszh::bounty::CashBountyAward> CashBountyAwards() const noexcept;
    const ecs::DependencyGraph &Graph() const noexcept;
    const ecs::ExecutionPlan &Plan() const noexcept;
    std::span<const BuildReceipt> Execute(engine::time::SimulationTime time, GameInputs inputs = {});

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};
}
