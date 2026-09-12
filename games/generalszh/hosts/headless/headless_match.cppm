module;
#include <cstddef>
#include <cstdint>
#include <vector>
export module games.generalszh.hosts.headless.headless_match;
export import games.generalszh.gameplay.match.components.match_state;
export namespace generalszh::headless
{
struct MatchReport
{
    MatchOutcome outcome;
    ecs::Entity attacker{}, target{};
    std::uint64_t targetHealth{}, survivingUnits{};
    // Canonical field values, not raw struct/padding hashes or a wire format.
    std::vector<std::uint64_t> state;
};
// Small authored demonstration, not an original-map loader or substitute game
// implementation. All actions use the production GameSession composition.
MatchReport RunMatch(std::size_t workers, std::uint64_t tickLimit = 256);
}

