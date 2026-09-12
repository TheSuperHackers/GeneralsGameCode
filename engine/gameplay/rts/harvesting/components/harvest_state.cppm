module;
#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string_view>
export module engine.gameplay.rts.harvesting.components.harvest_state;
export import engine.gameplay.navigation.systems.movement_system;
export import engine.gameplay.combat.systems.health_system;
export namespace engine::gameplay::rts::harvesting {
struct HarvestConfig { std::uint32_t capacity{1}; engine::time::Duration pickup{}, unload{}; };
struct HarvestPolicy { std::uint32_t capacity{1}; std::uint64_t pickupTicks{1}, unloadTicks{1}; };
inline HarvestPolicy MakeHarvestPolicy(HarvestConfig config, engine::time::FixedStep step) {
    if (!config.capacity) throw std::invalid_argument("Harvest capacity must be positive");
    return {config.capacity, std::max(std::uint64_t{1},step.TicksFor(config.pickup)),
        std::max(std::uint64_t{1},step.TicksFor(config.unload))};
}
enum class HarvestPhase : std::uint8_t { Idle, ToSource, Pickup, ToDropoff, Unload, NeedsInput };
struct HarvestState { ecs::Entity source{}, dropoff{}; HarvestPhase phase{HarvestPhase::Idle}; std::uint64_t remaining{}; };
struct HarvestCargo { std::uint32_t boxes{}; };
struct SupplySource { std::uint32_t boxes{}; bool available{true}; };
struct SupplyDropoff { bool available{true}; };
struct HarvestIntent { bool pickup{}; };
struct HarvestDelivery { ecs::Entity dropoff{}; std::uint32_t boxes{}; };
struct HarvestInput { ecs::Entity actor{}, source{}, dropoff{}; bool cancel{}; };
inline void CancelHarvest(HarvestState &state) noexcept { state={}; }
}
export namespace ecs {
#define HARVEST_COMPONENT(T, P) template<> struct ComponentTraits<engine::gameplay::rts::harvesting::T> { static constexpr std::string_view StableName="engine.gameplay.rts.harvesting." #T; static constexpr std::uint32_t Version=1; static constexpr PersistencePolicy Persistence=PersistencePolicy::P; };
HARVEST_COMPONENT(HarvestPolicy,Serializable)
HARVEST_COMPONENT(HarvestState,Serializable)
HARVEST_COMPONENT(HarvestCargo,Serializable)
HARVEST_COMPONENT(SupplySource,Serializable)
HARVEST_COMPONENT(SupplyDropoff,Serializable)
HARVEST_COMPONENT(HarvestIntent,Transient)
HARVEST_COMPONENT(HarvestDelivery,Transient)
#undef HARVEST_COMPONENT
}
export namespace engine::gameplay::rts::harvesting {
inline void RegisterHarvestComponents(ecs::World &world) {
    world.RegisterComponent<HarvestPolicy>(); world.RegisterComponent<HarvestState>();
    world.RegisterComponent<HarvestCargo>(); world.RegisterComponent<SupplySource>();
    world.RegisterComponent<SupplyDropoff>(); world.RegisterComponent<HarvestIntent>();
    world.RegisterComponent<HarvestDelivery>();
}
}
