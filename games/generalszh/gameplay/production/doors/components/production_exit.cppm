module;
#include <cstdint>
#include <string_view>
export module games.generalszh.gameplay.production.doors.components.production_exit;
export import engine.ecs.core.component_registry;
export namespace generalszh::production
{
enum class ExitDoorPhase : std::uint8_t { Closed, Opening, Open, Closing };
// Authoritative logical exit state, independent of animation playback. Explicit
// phase permits a real opening at tick zero; no legacy inactive-time sentinel.
struct ProductionExitState
{
    ExitDoorPhase phase{ExitDoorPhase::Closed};
    std::uint64_t since{};
    bool held{};
};
// Compiled startup policy. Systems read it; setup/content authors durations once.
struct ProductionExitTiming { std::uint64_t opening{}, waiting{}, closing{}; };
}
export namespace ecs
{
template<> struct ComponentTraits<generalszh::production::ProductionExitState>
{
    static constexpr std::string_view StableName="games.generalszh.production.exit_state";
    static constexpr std::uint32_t Version=1;
    static constexpr PersistencePolicy Persistence=PersistencePolicy::Serializable;
};
template<> struct ComponentTraits<generalszh::production::ProductionExitTiming>
{
    static constexpr std::string_view StableName="games.generalszh.production.exit_timing";
    static constexpr std::uint32_t Version=1;
    static constexpr PersistencePolicy Persistence=PersistencePolicy::Serializable;
};
}
