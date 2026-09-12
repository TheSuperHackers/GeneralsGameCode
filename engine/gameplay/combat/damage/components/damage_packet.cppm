module;
#include <cstdint>
#include <string_view>
export module engine.gameplay.combat.damage.components.damage_packet;
export import engine.ecs.core.entity;
export import engine.ecs.core.component_registry;
export namespace engine::gameplay::combat::damage {
// IDs are authored by the composing game, never assigned by registration order.
struct DamageTypeId {std::uint32_t value{};};
struct DamageChannelId {std::uint32_t value{};};
struct ArmorDefinitionRef {std::uint32_t value{};};
struct DamageAttribution {
    ecs::Entity source{},sourceAccount{};
    std::uint32_t sourceDefinition{};
};
struct DamagePacket {
    DamageTypeId type{};DamageChannelId channel{};std::uint64_t quantity{};
    DamageAttribution attribution{};
};
struct ArmorBinding {ArmorDefinitionRef definition{};};
struct ArmorMultiplier {std::uint32_t millionths{1'000'000};};
enum class ArmorApplication : std::uint8_t {Scale,Bypass,Unsupported};
// Default construction cannot accidentally turn a special type into health damage.
struct DamageTypePolicy {ArmorApplication armor{ArmorApplication::Unsupported};};
struct DamageResolution {std::uint64_t quantity{};bool saturated{};};
}
export namespace ecs {
template<> struct ComponentTraits<engine::gameplay::combat::damage::ArmorBinding> {
    static constexpr std::string_view StableName="engine.gameplay.combat.damage.armor_binding";
    static constexpr std::uint32_t Version=1;
    static constexpr PersistencePolicy Persistence=PersistencePolicy::Serializable;
};
}
