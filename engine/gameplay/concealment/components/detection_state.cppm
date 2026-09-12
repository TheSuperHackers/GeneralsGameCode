module;
#include <cstdint>
#include <string_view>
export module engine.gameplay.concealment.components.detection_state;
export import engine.ecs.core.component_registry;
export import engine.gameplay.concealment.definitions.detection_definition;

export namespace engine::gameplay::concealment
{
struct DetectionState
{
    bool enabled{true};
    std::uint64_t nextScanTick{};
};

inline DetectionState StartDetection(const CompiledDetectionDefinition &definition,
    const std::uint64_t tick) noexcept
{
    return {!definition.initiallyDisabled, tick};
}
}

export namespace ecs
{
template<> struct ComponentTraits<engine::gameplay::concealment::DetectionState>
{
    static constexpr std::string_view StableName="engine.gameplay.concealment.detection_state";
    static constexpr std::uint32_t Version=1;
    static constexpr PersistencePolicy Persistence=PersistencePolicy::Serializable;
};
}
