module;
#include <algorithm>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>
export module games.generalszh.gameplay.match.components.match_state;
export import engine.ecs.core.world;
export import engine.gameplay.rts.match.systems.elimination_system;
export import engine.gameplay.rts.economy.components.resource_balance;
export namespace generalszh
{
enum class MatchStatus : std::uint8_t { Setup, Running, Won, Draw };
struct MatchOutcome { MatchStatus status{MatchStatus::Setup}; ecs::Entity winner{}; std::uint64_t tick{}; };
}
export namespace generalszh::match
{
struct MatchState { MatchOutcome outcome{}; std::uint32_t rosterSize{}; };
struct MatchMember { ecs::Entity match{}; std::uint32_t ordinal{}; };
}
export namespace ecs
{
template<> struct ComponentTraits<generalszh::match::MatchState>
{
    static constexpr std::string_view StableName = "games.generalszh.match.state";
    static constexpr std::uint32_t Version = 1;
    static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
template<> struct ComponentTraits<generalszh::match::MatchMember>
{
    static constexpr std::string_view StableName = "games.generalszh.match.member";
    static constexpr std::uint32_t Version = 1;
    static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
}
export namespace generalszh::match
{
inline MatchOutcome ReadMatchOutcome(ecs::World &world, ecs::Entity entity)
{
    if (!entity.IsValid()) return {};
    const auto *state = world.Get<MatchState>(entity);
    if (!state) throw std::logic_error("Match state was removed externally");
    return state->outcome;
}
// Explicit boundary operation: no entity is allocated by system construction.
// All roster validation precedes structural writes; allocation failure is fatal
// to this start attempt and must be treated as terminal by the host.
inline ecs::Entity BeginMatch(ecs::World &world, std::span<const ecs::Entity> accounts,
    std::size_t capacity = 65536)
{
    using engine::gameplay::rts::match::Contender;
    using engine::gameplay::rts::match::SurvivalCount;
    if (world.IsScheduledExecutionActive()) throw std::logic_error("Match start requires a world boundary");
    if (accounts.size() < 2 || accounts.size() > capacity || accounts.size() > UINT32_MAX)
        throw std::invalid_argument("Match requires at least two contenders within capacity");
    std::vector<ecs::Entity> roster(accounts.begin(), accounts.end());
    std::sort(roster.begin(), roster.end(), [](auto a, auto b) { return a.index < b.index; });
    for (std::size_t i = 0; i != roster.size(); ++i)
        if (roster[i].index >= capacity || !world.Get<engine::gameplay::rts::economy::ResourceBalance>(roster[i])
            || (i && roster[i].index == roster[i-1].index) || world.Get<Contender>(roster[i])
            || world.Get<SurvivalCount>(roster[i]) || world.Get<MatchMember>(roster[i]))
            throw std::invalid_argument("Match roster requires distinct live unassigned accounts");
    const auto entity = world.Create<MatchState>();
    *world.Get<MatchState>(entity) = {{MatchStatus::Running, {}, 0}, static_cast<std::uint32_t>(roster.size())};
    for (std::size_t i = 0; i != roster.size(); ++i)
    {
        world.Add<Contender>(roster[i]); world.Add<SurvivalCount>(roster[i]); world.Add<MatchMember>(roster[i]);
        *world.Get<MatchMember>(roster[i]) = {entity, static_cast<std::uint32_t>(i)};
    }
    return entity;
}
}
