module;
#include <cstdint>
#include <optional>
#include <string_view>
export module games.generalszh.gameplay.production.components.production_state;
export import engine.ecs.core.component_registry;
export import games.generalszh.gameplay.bounty.components.cash_bounty_cost_binding;
export import engine.ecs.core.entity;
export import engine.time.simulation_time;
export import engine.gameplay.combat.components.weapon;
export import engine.gameplay.combat.components.periodic_damage;
export import engine.gameplay.navigation.grid.navigation_grid;
export import games.generalszh.gameplay.production.entry.production_entry_metadata;
export import games.generalszh.gameplay.combat.acquisition.acquisition_policy;
export import engine.gameplay.rts.harvesting.components.harvest_state;
export import engine.gameplay.combat.regeneration.components.regeneration;
export import engine.gameplay.progression.components.progression_state;
export import engine.gameplay.containment.components.passenger_membership;
export import engine.gameplay.containment.definitions.transport_definition;
export import games.generalszh.gameplay.capture.definitions.capture_definition;
export import engine.gameplay.concealment.components.concealment_binding;
export import games.generalszh.gameplay.concealment.components.stealth_binding;
export import engine.gameplay.rts.visibility.components.visibility_observer;
export namespace generalszh::production
{
// Validated startup values copied into each order. No catalog pointers or
// absolute deadlines: phase starts when the completed unit is created.
struct RegenerationSpawn
{
    engine::gameplay::combat::regeneration::RegenerationDefinitionId definition{};
    std::uint64_t phaseTicks{};
    bool active{};
};
struct ProgressionSpawn
{
    engine::gameplay::progression::ProgressionDefinitionRef definition{};
    bool trainable{};
};
struct VeterancySpawn
{
    std::uint32_t definition{};
    bool awardable{};
};
// Validated content binding carried by a production order. This is deliberately
// not the carrier's runtime phase; InitialPayloadBinding owns Pending/Consumed/
// Aborted after the produced carrier commits to the world.
struct InitialPayloadDefinition
{
    std::uint32_t passengerDefinition{};
    std::uint32_t count{};
};
struct StealthSpawn
{
    engine::gameplay::concealment::ConcealmentDefinitionId concealment{};
    generalszh::concealment::StealthPolicyId policy{};
    std::uint64_t concealDelayTicks{};
    bool enabledByDefault{};
};
struct DetectorSpawn
{
    engine::gameplay::concealment::DetectionDefinitionId detection{};
    bool initiallyDisabled{};
};
struct Producer
{
    // On constructed structures this routes production to Structure.account;
    // it is not a separate ownership decision. Paid BuildOrder.account remains
    // transaction provenance and must never be retargeted by capture.
    ecs::Entity account{};
    std::uint32_t queueLimit{9};
    bool active{true}; // Dead/sold/disabled producers cannot advance ordinary work.
};
struct BuildOrder
{
    ecs::Entity producer{}, account{};
    std::uint32_t definition{}, paid{};
    EntryKind kind{EntryKind::Unit};
    std::uint64_t health{100};
    engine::gameplay::navigation::Cell spawn{};
    std::uint32_t cellsPerSecond{1};
    engine::gameplay::combat::WeaponDefinition weapon{};
    combat::AcquisitionPolicy acquisition{};
    std::uint64_t maximumHealth{100};
    engine::gameplay::combat::PeriodicDamagePolicy poison{};
    std::optional<engine::gameplay::rts::harvesting::HarvestPolicy> harvest{};
    bool builder{};
    std::optional<RegenerationSpawn> regeneration{};
    std::optional<ProgressionSpawn> progression{};
    std::optional<std::uint64_t> lifetimeTicks{};
    std::optional<engine::gameplay::containment::TransportBinding> transport{};
    std::optional<engine::gameplay::containment::PassengerSlots> passenger{};
    std::optional<capture::CaptureCapability> capture{};
    std::optional<engine::gameplay::combat::damage::ArmorBinding> armor{};
    std::optional<capture::CompiledCaptureDefinition> captureTiming{};
    // Session-local immutable repair catalog index. This is appended so the
    // post-repair v14 aggregate layout remains source-compatible by prefix.
    std::optional<std::uint32_t> manualRepairDefinition{};
    std::optional<InitialPayloadDefinition> initialPayload{};
    std::optional<StealthSpawn> stealth{};
    std::optional<DetectorSpawn> detector{};
    // Appended after the v17 fields to preserve existing aggregate callers.
    std::optional<VeterancySpawn> veterancy{};
    // Appended so production-created garrisonable infantry enrolls its
    // membership component at the same structural commit as other units.
    bool garrisonable{};
    // A dense session-local visibility definition ID, copied only when the
    // decoded production definition enrolled this capability.
    std::optional<engine::gameplay::rts::visibility::VisibilityDefinitionId> visibility{};
    // Appended after the existing visibility field so the prior aggregate
    // prefix and visibility binding remain unchanged.
    std::optional<generalszh::bounty::CashBountyCostBinding> cashBountyCost{};
};
struct ProducedUnit { ecs::Entity producer{}, account{}; std::uint32_t definition{}; };
struct ResearchedUpgrade { ecs::Entity account{}; std::uint32_t definition{}; };
}
export namespace ecs
{
#define PRODUCTION_COMPONENT(Type, Key) template<> struct ComponentTraits<generalszh::production::Type> { \
 static constexpr std::string_view StableName = Key; static constexpr std::uint32_t Version = 1; \
 static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable; };
PRODUCTION_COMPONENT(Producer, "games.generalszh.production.producer")
PRODUCTION_COMPONENT(ProducedUnit, "games.generalszh.production.produced_unit")
PRODUCTION_COMPONENT(ResearchedUpgrade, "games.generalszh.production.researched_upgrade")
#undef PRODUCTION_COMPONENT
template<> struct ComponentTraits<generalszh::production::BuildOrder>
{
    static constexpr std::string_view StableName = "games.generalszh.production.build_order";
    static constexpr std::uint32_t Version = 21; // Adds optional cash-bounty enrollment after the v20 visibility binding.
    static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
}
