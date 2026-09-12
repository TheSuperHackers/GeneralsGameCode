module;
#include <cstdint>
#include <string_view>
export module games.generalszh.gameplay.containment.components.initial_payload;
export import engine.ecs.core.component_registry;

export namespace generalszh::containment
{
inline constexpr std::string_view InitialPayloadStableKey =
    "games.generalszh.containment.initial_payload";
inline constexpr std::uint32_t InitialPayloadVersion = 1;

enum class InitialPayloadPhase : std::uint8_t
{
    Pending,
    Consumed,
    Aborted
};

// The passengerDefinition is the stable BuildDefinition key, never a dense
// catalog index. The immutable catalog supplies the spawn recipe at runtime.
// Pending/Consumed/Aborted is authoritative state so zero-count and
// death-before-action cases cannot repeat after a later tick or restore.
struct InitialPayloadBinding
{
    std::uint32_t passengerDefinition{};
    std::uint32_t count{};
    InitialPayloadPhase phase{InitialPayloadPhase::Pending};
};
}

export namespace ecs
{
template<> struct ComponentTraits<generalszh::containment::InitialPayloadBinding>
{
    static constexpr std::string_view StableName =
        generalszh::containment::InitialPayloadStableKey;
    static constexpr std::uint32_t Version =
        generalszh::containment::InitialPayloadVersion;
    static constexpr PersistencePolicy Persistence = PersistencePolicy::Serializable;
};
}
