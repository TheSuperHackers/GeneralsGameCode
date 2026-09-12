module;
#include <cstdint>
#include <string_view>
export module games.generalszh.gameplay.construction.components.construction_health;
export import engine.ecs.core.component_registry;
export namespace generalszh::construction
{
// Remainder of health earned by integer construction steps, not a second clock.
struct ConstructionHealth { std::uint64_t remainder{}; };
}
export namespace ecs
{
template<> struct ComponentTraits<generalszh::construction::ConstructionHealth>
{
    static constexpr std::string_view StableName="games.generalszh.construction.health_remainder";
    static constexpr std::uint32_t Version=1;
    static constexpr PersistencePolicy Persistence=PersistencePolicy::Serializable;
};
}
