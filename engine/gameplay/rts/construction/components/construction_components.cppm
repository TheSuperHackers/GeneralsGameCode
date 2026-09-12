module;
#include <algorithm>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <vector>
export module engine.gameplay.rts.construction.components.construction_components;
export import engine.gameplay.rts.production.systems.build_system;
export import engine.gameplay.navigation.systems.movement_system;
export import engine.gameplay.combat.systems.health_system;
export namespace engine::gameplay::rts::construction {
struct BuilderAssignment { ecs::Entity site{}; navigation::Cell cell{navigation::InvalidCell}; };
struct ConstructionSite { ecs::Entity builder{}; };
}
export namespace ecs {
#define CONSTRUCTION_COMPONENT(Type,Key) template<> struct ComponentTraits<engine::gameplay::rts::construction::Type> { \
 static constexpr std::string_view StableName=Key; static constexpr std::uint32_t Version=1; \
 static constexpr PersistencePolicy Persistence=PersistencePolicy::Serializable; };
CONSTRUCTION_COMPONENT(BuilderAssignment,"engine.gameplay.rts.construction.builder_assignment")
CONSTRUCTION_COMPONENT(ConstructionSite,"engine.gameplay.rts.construction.site")
#undef CONSTRUCTION_COMPONENT
}
