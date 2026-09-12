module;
#include <cstdint>
#include <string_view>
export module engine.gameplay.rts.power.components.power_source;
export import engine.ecs.core.entity;
export import engine.gameplay.rts.power.ledger.power_ledger;
export namespace engine::gameplay::rts::power
{
// Authored sign: positive generates, negative consumes. Ownership/eligibility
// changes are boundary inputs; no legacy object or separate ledger is consulted.
struct PowerSource { ecs::Entity account{}; std::int32_t authoredPower{}; bool eligible{true}; };
struct PowerContribution { std::uint32_t production{}, consumption{}; };
}
export namespace ecs
{
template<> struct ComponentTraits<engine::gameplay::rts::power::PowerSource>
{
    static constexpr std::string_view StableName = "engine.gameplay.rts.power.source";
    static constexpr std::uint32_t Version = 1;
    static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
template<> struct ComponentTraits<engine::gameplay::rts::power::PowerContribution>
{
    static constexpr std::string_view StableName = "engine.gameplay.rts.power.contribution";
    static constexpr std::uint32_t Version = 1;
    static constexpr PersistencePolicy Persistence = PersistencePolicy::Transient;
};
}
