module;
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <vector>
export module games.generalszh.hosts.headless.content_match;
export import games.generalszh.hosts.headless.headless_match;
export import games.generalszh.adapters.content.units.unit_definition;
export import games.generalszh.adapters.content.buildings.building_definition;
export import games.generalszh.adapters.content.prerequisites.prerequisite_definition;
export import engine.time.simulation_time;
export import engine.gameplay.navigation.grid.navigation_grid;
export import engine.gameplay.rts.unlocks.definitions.unlock_definition;
export import engine.gameplay.rts.unlocks.definitions.unlock_catalog;
export import games.generalszh.gameplay.bounty.definitions.cash_bounty_definition;
export import games.generalszh.gameplay.bounty.inputs.cash_bounty_batch;
export import engine.gameplay.progression.inputs.progression_batch;
export import engine.gameplay.rts.unlocks.inputs.unlock_batch;
export import games.generalszh.gameplay.production.inputs.build_inputs;
export import engine.gameplay.combat.damage.inputs.accepted_hit_batch;
export namespace generalszh::headless
{
struct ContentExercise
{
    // An explicitly authored accepted award, never inferred from combat kills.
    std::optional<std::uint64_t> experience;
    engine::time::Duration regenerationPhase{};
    // Optional unarmed, passive target is produced by GameSession, not attached
    // to a base by post-spawn mutation. Borrowed for this synchronous exercise.
    const content::DecodedUnit *passiveTarget{};
    std::size_t hitCapacity{16384}; // Bounded diagnostic report, not a production bus.
    const content::DecodedBuilding *initialBuilding{};
    engine::gameplay::navigation::Cell initialBuildingCell{1};
    std::span<const content::ContentDefinitionIdentity> prerequisiteIdentities{};
    // Additional decoded unit definitions required by whole-catalog binding.
    // The primary unit and optional passive target remain explicit parameters;
    // this span supplies payload passengers and any other referenced units.
    std::span<const content::DecodedUnit> supportingCatalog{};
    // Borrowed for this synchronous host exercise. The composition root and
    // BuildCatalog receive this exact finalized catalog; no copy is made.
    const engine::gameplay::rts::unlocks::UnlockCatalog *unlockCatalog{};
    std::span<const engine::gameplay::rts::unlocks::UnlockKey> purchaseUnlocks{};
    std::uint64_t unlockCredits{};
    // Optional decoded Zero Hour policy. An empty span preserves the existing
    // root graph and account archetype; a non-empty span enables enrollment.
    std::span<const generalszh::bounty::CashBountyDefinition> cashBountyDefinitions{};
    // Optional completed-building victim for exercises that must validate the
    // construction completion binding rather than a produced-unit binding.
    const content::DecodedBuilding *passiveBuilding{};
    engine::gameplay::navigation::Cell passiveBuildingCell{32};
    // When set, the passive building is admitted through the real construction
    // path and remains a scaffold long enough for the combat/bounty join to
    // reject it as incomplete.
    bool passiveBuildingUnderConstruction{};
    // If present, request one follow-up production only after the lethal
    // settlement tick. The admission receipt proves the deposited funds were
    // spendable through the normal production path.
    std::optional<std::uint32_t> cashBountyFollowupDefinition;
};
struct DecodedMatchReport : MatchReport
{
    bool progressionPresent{}, regenerationPresent{}, transportPresent{}, passengerPresent{};
    bool initialPayloadPresent{}, initialPayloadConsumed{}, initialPayloadSpawned{};
    bool manualRepairPresent{};
    bool capturePresent{},captureEnabled{};
    bool concealmentPresent{},detectorPresent{},targetConcealmentPresent{},targetDetectorPresent{},targetDetectedAtEnd{};
    std::uint64_t spawnTick{}, spawnHealth{}, experience{};
    std::uint32_t level{};
    std::uint32_t initialPayloadConfiguredCount{}, initialPayloadSpawnCount{};
    std::uint64_t initialPayloadSpawnTick{};
    ecs::Entity initialPayloadCarrier{};
    std::vector<std::uint64_t> initialPayloadState;
    std::vector<engine::gameplay::progression::ExperienceResult> experienceResults;
    std::vector<engine::gameplay::rts::unlocks::UnlockReceipt> unlockReceipts;
    std::vector<::generalszh::BuildReceipt> buildReceipts;
    std::vector<generalszh::bounty::CashBountyAward> bountyAwards;
    std::uint32_t bountyRateNumerator{}, bountyRateDenominator{};
    std::uint32_t attackerBalance{}, attackerIncome{};
    ecs::Entity bountyFollowupEntity{};
    ecs::Entity bountyFollowupAccount{};
    std::uint32_t bountyFollowupDefinition{};
    bool bountyFollowupAccepted{}, bountyFollowupSpawned{};
    bool armorPresent{},targetArmorPresent{};
    bool targetStructureComplete{};
    ecs::Entity attackerAccount{},targetAccount{};
    std::vector<engine::gameplay::combat::damage::ResolvedHit> acceptedHits;
};
struct ContentReport { DecodedMatchReport match; std::string unitName; std::vector<std::string> omittedBehaviors; };
// Authored exercise using a decoded unit and a passive enemy base or produced unit. Not a map
// loader or an AI player: gameplay execution remains entirely GameSession.
DecodedMatchReport RunDecodedMatch(const content::DecodedUnit &unit,std::size_t workers,std::uint64_t tickLimit=4096,
    ContentExercise exercise={});
ContentReport RunContentMatch(const std::filesystem::path &iniRoot,std::size_t workers);
}
