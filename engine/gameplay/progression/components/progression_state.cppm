module;
#include <cstddef>
#include <cstdint>
#include <string_view>
export module engine.gameplay.progression.components.progression_state;
export import engine.ecs.core.component_registry;
export namespace engine::gameplay::progression
{
struct ProgressionState { std::uint64_t experience{}; std::uint32_t level{}; };
struct ProgressionDefinitionRef { std::uint32_t index{}; };
struct ProgressionEligibility { bool trainable{}; };
// Tick-local range into the batch's ordered input indices, never another XP owner.
struct ProgressionInbox { std::size_t begin{}, count{}; };
}
export namespace ecs
{
template<> struct ComponentTraits<engine::gameplay::progression::ProgressionState> {
 static constexpr std::string_view StableName="engine.gameplay.progression.state";
 static constexpr std::uint32_t Version=1;
 static constexpr PersistencePolicy Persistence=PersistencePolicy::Serializable;
};
template<> struct ComponentTraits<engine::gameplay::progression::ProgressionDefinitionRef> {
 static constexpr std::string_view StableName="engine.gameplay.progression.definition_ref";
 static constexpr std::uint32_t Version=1;
 static constexpr PersistencePolicy Persistence=PersistencePolicy::Serializable;
};
template<> struct ComponentTraits<engine::gameplay::progression::ProgressionEligibility> {
 static constexpr std::string_view StableName="engine.gameplay.progression.eligibility";
 static constexpr std::uint32_t Version=1;
 static constexpr PersistencePolicy Persistence=PersistencePolicy::Serializable;
};
template<> struct ComponentTraits<engine::gameplay::progression::ProgressionInbox> {
 static constexpr std::string_view StableName="engine.gameplay.progression.inbox";
 static constexpr std::uint32_t Version=1;
 static constexpr PersistencePolicy Persistence=PersistencePolicy::Transient;
};
}
