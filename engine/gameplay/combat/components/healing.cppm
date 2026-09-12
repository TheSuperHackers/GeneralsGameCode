module;
#include <cstdint>
#include <string_view>
export module engine.gameplay.combat.components.healing;
export import engine.ecs.core.component_registry;
export namespace engine::gameplay::combat
{
struct HealthCapacity { std::uint64_t maximum{}; };
struct PendingHealing { std::uint64_t quantity{}; };
struct HealingResult { std::uint64_t applied{}; };
}
export namespace ecs
{
#define HEAL_COMPONENT(Type,Key,Policy) template<> struct ComponentTraits<engine::gameplay::combat::Type>{ \
 static constexpr std::string_view StableName=Key;static constexpr std::uint32_t Version=1; \
 static constexpr PersistencePolicy Persistence=PersistencePolicy::Policy;};
HEAL_COMPONENT(HealthCapacity,"engine.gameplay.combat.health_capacity",Serializable)
HEAL_COMPONENT(PendingHealing,"engine.gameplay.combat.pending_healing",Transient)
HEAL_COMPONENT(HealingResult,"engine.gameplay.combat.healing_result",Transient)
#undef HEAL_COMPONENT
}
