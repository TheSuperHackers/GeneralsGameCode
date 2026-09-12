module;
#include <cstdint>
#include <string_view>
#include <stdexcept>
export module engine.gameplay.rts.repair.components.repair_state;
export import engine.gameplay.rts.repair.definitions.repair_definition;
export import engine.ecs.core.component_registry;
export namespace engine::gameplay::rts::repair
{
struct RepairBinding { std::uint32_t definition{}; };
struct RepairState { std::uint64_t nextPulse{}; std::uint32_t remainder{}; bool armed{},stopped{}; };
inline RepairState StartRepair(const RepairDefinitions &definitions,std::uint32_t index,time::SimulationTime now)
{
    if(definitions.Step()!=now.Step()) throw std::invalid_argument("Repair enrollment step mismatch");
    const auto &definition=definitions.Get(index);
    return {RepairDeadline(now.Tick(),definition.initialDelay),0,definition.numerator!=0,false};
}
}
export namespace ecs
{
template<> struct ComponentTraits<engine::gameplay::rts::repair::RepairBinding>
{
    static constexpr std::string_view StableName="engine.gameplay.rts.repair.binding";
    static constexpr std::uint32_t Version=1;
    static constexpr PersistencePolicy Persistence=PersistencePolicy::Serializable;
};
template<> struct ComponentTraits<engine::gameplay::rts::repair::RepairState>
{
    static constexpr std::string_view StableName="engine.gameplay.rts.repair.state";
    static constexpr std::uint32_t Version=1;
    static constexpr PersistencePolicy Persistence=PersistencePolicy::Serializable;
};
}
