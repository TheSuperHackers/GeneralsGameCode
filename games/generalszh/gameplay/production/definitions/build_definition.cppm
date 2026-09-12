module;
#include <cstdint>
#include <optional>
#include <vector>
export module games.generalszh.gameplay.production.definitions.build_definition;
export import games.generalszh.gameplay.production.components.production_state;
export import games.generalszh.gameplay.bounty.components.cash_bounty_cost_binding;
export import engine.gameplay.rts.production.definitions.prerequisite_definition;
export import engine.gameplay.rts.unlocks.definitions.unlock_definition;
export import engine.gameplay.combat.definitions.weapon_definition;
export import engine.gameplay.combat.definitions.periodic_damage_definition;
export import engine.gameplay.rts.repair.definitions.repair_definition;
export import engine.gameplay.concealment.definitions.concealment_definition;
export import engine.gameplay.concealment.definitions.detection_definition;
export import games.generalszh.gameplay.concealment.definitions.stealth_policy;
export import engine.gameplay.rts.visibility.definitions.visibility_definition;
export namespace generalszh::production
{
struct RegenerationSpawnConfig
{
    std::uint32_t definition{};
    engine::time::Duration phase{};
    bool active{};
};
struct ProgressionSpawnConfig
{
    std::uint32_t definition{};
    bool trainable{};
};
struct VeterancySpawnConfig
{
    std::uint32_t definition{};
    bool awardable{};
};
// Immutable composition input. Content adapters choose stable numeric definition
// keys and typed durations; workers never parse names/INI or consult legacy types.
struct BuildDefinition
{
    std::uint32_t key{}, cost{};
    engine::time::Duration duration{};
    EntryKind kind{EntryKind::Unit};
    std::int32_t quantity{1};
    std::uint64_t health{100}; // Vertical-slice health quanta; adapters author the scale.
    std::uint32_t cellsPerSecond{1};
    engine::gameplay::combat::WeaponConfig weapon{}; // Zero damage means unarmed.
    combat::AcquisitionPolicy acquisition{};
    std::uint64_t maximumHealth{}; // Zero authors capacity equal to initial health.
    engine::gameplay::combat::PeriodicDamageConfig poison{};
    std::optional<engine::gameplay::rts::harvesting::HarvestConfig> harvest{};
    bool builder{};
    std::optional<RegenerationSpawnConfig> regeneration{};
    std::optional<ProgressionSpawnConfig> progression{};
    std::optional<engine::time::Duration> lifetime{};
    std::optional<std::uint32_t> transportDefinition{};
    std::optional<engine::gameplay::containment::PassengerSlots> passenger{};
    std::optional<capture::CaptureCapability> capture{};
    std::optional<engine::gameplay::combat::damage::ArmorBinding> armor{};
    std::optional<engine::gameplay::rts::production::PrerequisiteDefinition> prerequisites{};
    // Raw typed authoring. BuildCatalog resolves this value to the immutable
    // session RepairDefinitions index before any unit can be produced.
    std::optional<engine::gameplay::rts::repair::RepairDefinition> manualRepair{};
    // Stable passenger key/count only. The runtime carrier phase is a separate
    // serializable ECS component attached by the completion path.
    std::optional<InitialPayloadDefinition> initialPayload{};
    // These are typed authoring values. BuildCatalog binds them to the
    // finalized immutable catalogs; ordinary definitions leave them absent.
    std::optional<engine::gameplay::concealment::ConcealmentDefinition> concealment{};
    std::optional<engine::gameplay::concealment::DetectionDefinition> detection{};
    std::optional<generalszh::concealment::StealthPolicy> stealthPolicy{};
    // Appended to preserve existing scalar/aggregate callers. This is an
    // independent VeterancyCatalog index, never the generic progression index.
    std::optional<VeterancySpawnConfig> veterancy{};
    // Ground infantry eligibility for building garrisons. This is distinct
    // from PassengerSlots: a unit may garrison without being transport cargo.
    bool garrisonable{};
    // Startup-bound Science prerequisites. Content adapters resolve names
    // against the immutable root-owned UnlockCatalog; BuildCatalog copies
    // only dense IDs into its runtime ranges and never performs key lookups.
    std::vector<engine::gameplay::rts::unlocks::UnlockId> sciencePrerequisiteIds{};
    std::optional<engine::gameplay::rts::unlocks::UnlockSchemaHash> scienceSchemaHash{};
    // Decoded content binding. BuildCatalog assigns the dense runtime ID at
    // startup and recompiles its duration for the session fixed step.
    std::optional<engine::gameplay::rts::visibility::VisibilityDefinition> visibility{};
    // Optional victim enrollment copied into the produced unit at completion.
    // The value is the decoded current BuildCost, never a paid receipt.
    std::optional<generalszh::bounty::CashBountyCostBinding> cashBountyCost{};
};
}
