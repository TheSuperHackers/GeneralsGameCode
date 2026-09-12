module;
#include <cstdint>
#include <string_view>
export module games.generalszh.gameplay.match.components.victory_asset;
export import engine.ecs.core.component_registry;
export import engine.ecs.core.entity;
export namespace generalszh::match
{
// Adapter policy: only assets which count for building-elimination victory carry
// this component. Neutral props and ordinary produced units do not qualify.
struct VictoryAsset { ecs::Entity account{}; };
}
export namespace ecs
{
template<> struct ComponentTraits<generalszh::match::VictoryAsset>
{
    static constexpr std::string_view StableName = "games.generalszh.match.victory_asset";
    static constexpr std::uint32_t Version = 1;
    static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
}
