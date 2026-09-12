module;
#include <cstdint>
#include <string_view>
export module engine.gameplay.rts.production.components.build_work;
export import engine.ecs.core.component_registry;
export namespace engine::gameplay::rts::production
{
struct BuildWork
{
    std::uint64_t required{}, elapsed{};
    std::uint32_t speedNumerator{1}, speedDenominator{1};
    std::uint64_t remainder{};
};
struct BuildEnabled { bool value{}; };
struct BuildReady { bool value{}; };
}
export namespace ecs
{
#define BUILD_COMPONENT(Type, Key, Policy, Revision) template<> struct ComponentTraits<engine::gameplay::rts::production::Type> { \
 static constexpr std::string_view StableName = Key; static constexpr std::uint32_t Version = Revision; \
 static constexpr PersistencePolicy Persistence = PersistencePolicy::Policy; };
BUILD_COMPONENT(BuildWork, "engine.gameplay.rts.production.build_work", Serializable, 2)
BUILD_COMPONENT(BuildEnabled, "engine.gameplay.rts.production.build_enabled", Transient, 1)
BUILD_COMPONENT(BuildReady, "engine.gameplay.rts.production.build_ready", Transient, 1)
#undef BUILD_COMPONENT
}
