module;
#include <cstdint>
#include <stdexcept>
#include <string_view>
export module engine.gameplay.combat.regeneration.components.regeneration;
export import engine.gameplay.combat.regeneration.definitions.regeneration_definition;
export import engine.ecs.core.component_registry;

export namespace engine::gameplay::combat::regeneration
{
struct RegenerationBinding { RegenerationDefinitionId definition{}; };
struct RegenerationState
{
    std::uint64_t nextPulse{}, soonestDamageWake{};
    bool active{false}, stopped{false}, armed{false};
};
// Creation/activation boundary. Phase is explicit; no random phase is invented.
inline RegenerationState StartRegeneration(const RegenerationDefinitions &definitions,
    RegenerationDefinitionId definition, time::SimulationTime now, time::Duration phase, bool active)
{
    (void)definitions.Get(definition);
    if (now.Step() != definitions.Step()) throw std::invalid_argument("Regeneration step mismatch");
    const auto deadline = RegenerationDeadline(now.Tick(), now.Step().TicksFor(phase));
    return {deadline, 0, active, false, active};
}
inline void StopRegeneration(RegenerationState &state) noexcept
{
    state.stopped = true;
    state.armed = false;
}
}
export namespace ecs
{
template<> struct ComponentTraits<engine::gameplay::combat::regeneration::RegenerationBinding>
{
    static constexpr std::string_view StableName="engine.gameplay.combat.regeneration.binding";
    static constexpr std::uint32_t Version=1;
    static constexpr PersistencePolicy Persistence=PersistencePolicy::Serializable;
};
template<> struct ComponentTraits<engine::gameplay::combat::regeneration::RegenerationState>
{
    static constexpr std::string_view StableName="engine.gameplay.combat.regeneration.state";
    static constexpr std::uint32_t Version=1;
    static constexpr PersistencePolicy Persistence=PersistencePolicy::Serializable;
};
}
