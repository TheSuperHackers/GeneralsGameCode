module;
#include <cstdint>
#include <optional>
#include <string_view>
export module games.generalszh.gameplay.construction.components.structure;
export import engine.gameplay.rts.construction.components.construction_components;
export import engine.gameplay.rts.repair.components.repair_state;
export import engine.gameplay.combat.damage.components.damage_packet;
export import engine.gameplay.rts.visibility.definitions.visibility_definition;
export import engine.gameplay.rts.visibility.components.visibility_observer;
export namespace generalszh::construction
{
struct Structure
{
    // Sole structure owner. Attached producer/power/victory/supply account fields
    // route those capabilities to this owner and transfer with it at capture.
    ecs::Entity account{};
    std::uint32_t definition{},queueLimit{};
    std::int32_t energy{};
    bool complete{};
    bool supplyDropoff{};
    std::uint32_t supplyBoxValue{75};
    std::optional<std::uint32_t> repairDefinition{};
    bool capturable{};
    std::optional<engine::gameplay::combat::damage::ArmorBinding> armor{};
    std::optional<engine::gameplay::rts::visibility::VisibilityDefinitionId> visibility{};
};

struct Builder { ecs::Entity account{}; };
}
export namespace ecs
{
#define ZH_CONSTRUCTION_COMPONENT(Type,Key,Schema) template<> struct ComponentTraits<generalszh::construction::Type> { \
 static constexpr std::string_view StableName=Key; static constexpr std::uint32_t Version=Schema; \
 static constexpr PersistencePolicy Persistence=PersistencePolicy::Serializable; };
ZH_CONSTRUCTION_COMPONENT(Structure,"games.generalszh.construction.structure",5)
ZH_CONSTRUCTION_COMPONENT(Builder,"games.generalszh.construction.builder",1)
#undef ZH_CONSTRUCTION_COMPONENT
}
