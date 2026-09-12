module;
#include <cstdint>
#include <string_view>
export module engine.gameplay.containment.components.passenger_membership;
export import engine.ecs.core.entity;
export import engine.ecs.core.component_registry;
export namespace engine::gameplay::containment
{
enum class ContainmentPhase : std::uint8_t { Outside, Inside, AwaitingEvacuation };

// The owner identifies which containment system is authoritative for this
// relationship. It is deliberately independent from passenger fire policy:
// another reusable container may permit firing without being a garrison.
enum class PassengerContainmentOwner : std::uint8_t { None, Transport, Garrison };
enum class PassengerCombatPolicy : std::uint8_t { Blocked, Allowed };

struct PassengerMembership
{
    ecs::Entity carrier{};
    std::uint64_t evacuationTick{};
    ContainmentPhase phase{ContainmentPhase::Outside};
    PassengerContainmentOwner owner{PassengerContainmentOwner::None};
    PassengerCombatPolicy combat{PassengerCombatPolicy::Blocked};
};
constexpr bool IsContained(const PassengerMembership &membership) noexcept
{ return membership.phase!=ContainmentPhase::Outside; }
constexpr bool IsTransportOwned(const PassengerMembership &membership) noexcept
{ return IsContained(membership) && membership.owner==PassengerContainmentOwner::Transport; }
constexpr bool IsGarrisonOwned(const PassengerMembership &membership) noexcept
{ return IsContained(membership) && membership.owner==PassengerContainmentOwner::Garrison; }
constexpr bool CanPassengerFire(const PassengerMembership &membership) noexcept
{ return IsContained(membership) && membership.combat==PassengerCombatPolicy::Allowed; }
// Game enrollment chooses permitted passenger categories. Slot count is generic
// capacity consumption, not an infantry/FPS/RTS type discriminator.
struct PassengerSlots { std::uint32_t slots{1}; };
struct TransportBinding { std::uint32_t definition{}; };
struct TransportState { std::uint64_t nextExitTick{}; };
}
export namespace ecs
{
#define CONTAINMENT_COMPONENT(Type,Name) template<> struct ComponentTraits<engine::gameplay::containment::Type> { \
 static constexpr std::string_view StableName="engine.gameplay.containment." Name; \
 static constexpr std::uint32_t Version=1; \
 static constexpr PersistencePolicy Persistence=PersistencePolicy::Serializable; };
template<> struct ComponentTraits<engine::gameplay::containment::PassengerMembership> {
 static constexpr std::string_view StableName="engine.gameplay.containment.membership";
 static constexpr std::uint32_t Version=2;
 static constexpr PersistencePolicy Persistence=PersistencePolicy::Serializable;
};
CONTAINMENT_COMPONENT(PassengerSlots,"passenger_slots")
CONTAINMENT_COMPONENT(TransportBinding,"transport_binding")
CONTAINMENT_COMPONENT(TransportState,"transport_state")
#undef CONTAINMENT_COMPONENT
}
