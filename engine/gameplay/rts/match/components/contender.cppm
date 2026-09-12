module;
#include <cstdint>
#include <string_view>
export module engine.gameplay.rts.match.components.contender;
export import engine.ecs.core.component_registry;
export namespace engine::gameplay::rts::match
{
// Game policy supplies the qualifying assets. Counts are a boundary reduction,
// not another authoritative copy of those assets.
struct SurvivalCount { std::uint64_t value{}; };
struct Contender { bool defeated{}; };
}
export namespace ecs
{
template<> struct ComponentTraits<engine::gameplay::rts::match::SurvivalCount>
{
    static constexpr std::string_view StableName = "engine.gameplay.rts.match.survival_count";
    static constexpr std::uint32_t Version = 1;
    static constexpr PersistencePolicy Persistence = PersistencePolicy::Transient;
};
template<> struct ComponentTraits<engine::gameplay::rts::match::Contender>
{
    static constexpr std::string_view StableName = "engine.gameplay.rts.match.contender";
    static constexpr std::uint32_t Version = 1;
    static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
}
