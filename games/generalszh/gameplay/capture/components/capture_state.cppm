module;
#include <cstdint>
#include <string_view>
export module games.generalszh.gameplay.capture.components.capture_state;
export import engine.ecs.core.entity;
export import engine.ecs.core.component_registry;
export namespace generalszh::capture
{
struct CaptureCapability { std::uint32_t definition{}; bool enabled{}; };
struct CaptureRecharge { std::uint64_t readyAt{}; bool initialized{}; };
enum class CaptureActorPhase : std::uint8_t { Idle, Capturing, Recovering };
struct CaptureActorState {
    ecs::Entity target{};
    CaptureActorPhase phase{};
    std::uint64_t recoveryUntil{};
};
struct Capturable { bool enabled{}; };
// Structure.account remains the owner. These account values are acceptance
// witnesses used to cancel stale work, never alternative ownership authorities.
struct CaptureState {
    ecs::Entity actor{},acceptedAccount{},originalAccount{};
    std::uint64_t completionTick{};
    bool active{};
    std::uint64_t preparationTick{};
    bool preparationStarted{};
};
constexpr bool IsCaptureBusy(const CaptureActorState &state) noexcept
{ return state.phase!=CaptureActorPhase::Idle; }
}
export namespace ecs
{
#define CAPTURE_COMPONENT(Type,Key) template<> struct ComponentTraits<generalszh::capture::Type> { \
 static constexpr std::string_view StableName="games.generalszh.capture." Key; \
 static constexpr std::uint32_t Version=1; \
 static constexpr PersistencePolicy Persistence=PersistencePolicy::Serializable; };
CAPTURE_COMPONENT(CaptureCapability,"capability")
CAPTURE_COMPONENT(CaptureRecharge,"recharge")
CAPTURE_COMPONENT(CaptureActorState,"actor_state")
CAPTURE_COMPONENT(Capturable,"eligibility")
template<> struct ComponentTraits<generalszh::capture::CaptureState> {
 static constexpr std::string_view StableName="games.generalszh.capture.state";
 static constexpr std::uint32_t Version=2;
 static constexpr PersistencePolicy Persistence=PersistencePolicy::Serializable;
};
#undef CAPTURE_COMPONENT
}
