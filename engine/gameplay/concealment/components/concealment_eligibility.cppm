module;
#include <cstdint>
#include <string_view>
export module engine.gameplay.concealment.components.concealment_eligibility;
export import engine.ecs.core.component_registry;

export namespace engine::gameplay::concealment
{
// Derived policy input for the reusable state machine. It is not an
// authoritative concealment status and is intentionally transient.
struct ConcealmentEligibility { bool allowed{true}; };
}

export namespace ecs
{
template<> struct ComponentTraits<engine::gameplay::concealment::ConcealmentEligibility>
{
    static constexpr std::string_view StableName="engine.gameplay.concealment.eligibility";
    static constexpr std::uint32_t Version=1;
    static constexpr PersistencePolicy Persistence=PersistencePolicy::Transient;
};
}
