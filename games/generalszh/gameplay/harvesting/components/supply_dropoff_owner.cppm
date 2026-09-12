module;

#include <cstdint>
#include <string_view>

export module games.generalszh.gameplay.harvesting.components.supply_dropoff_owner;
export import engine.ecs.core.component_registry;
export import engine.ecs.core.entity;

export namespace generalszh::harvesting
{
struct SupplyDropoffOwner
{
    ecs::Entity account{};
    std::uint32_t valuePerBox{};
};
}

export namespace ecs
{
template<> struct ComponentTraits<generalszh::harvesting::SupplyDropoffOwner>
{
    static constexpr std::string_view StableName = "games.generalszh.harvesting.dropoff_owner";
    static constexpr std::uint32_t Version = 1;
    static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
}
