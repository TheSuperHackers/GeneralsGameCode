module;
#include <cassert>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>
export module engine.gameplay.containment.definitions.transport_definition;
export import engine.time.simulation_time;
namespace engine::gameplay::containment::detail
{
class ContainmentTickOverflow final : public std::overflow_error
{ public: ContainmentTickOverflow():std::overflow_error("Containment exceeds simulation tick horizon") {} };
[[noreturn]] void ThrowTickOverflow() { throw ContainmentTickOverflow{}; }
}
export namespace engine::gameplay::containment
{
struct TransportDefinition
{
    std::uint32_t slots{};
    time::Duration exitDelay{};
    std::uint32_t deathDamageNumerator{},deathDamageDenominator{1};
};
struct CompiledTransportDefinition
{
    std::uint32_t slots{};
    std::uint64_t exitTicks{};
    std::uint32_t deathDamageNumerator{},deathDamageDenominator{1};
};
class TransportDefinitions
{
public:
    TransportDefinitions(time::FixedStep step,std::span<const TransportDefinition> authored):step_(step)
    {
        if(authored.size()>UINT32_MAX) throw std::length_error("Too many transport definitions");
        entries_.reserve(authored.size());
        for(const auto &definition:authored)
        {
            if(!definition.slots||!definition.deathDamageDenominator||definition.deathDamageNumerator>definition.deathDamageDenominator)
                throw std::invalid_argument("Invalid transport capacity or death damage fraction");
            entries_.push_back({definition.slots,step.TicksFor(definition.exitDelay),definition.deathDamageNumerator,definition.deathDamageDenominator});
        }
    }
    time::FixedStep Step() const noexcept {return step_;}
    const CompiledTransportDefinition &Get(std::uint32_t index) const {return entries_.at(index);}
    const CompiledTransportDefinition &GetUnchecked(std::uint32_t index) const noexcept
    {assert(index<entries_.size());return entries_[index];}
private:
    time::FixedStep step_;
    std::vector<CompiledTransportDefinition> entries_;
};
inline std::uint64_t ContainmentDeadline(std::uint64_t tick,std::uint64_t delay)
{if(delay>UINT64_MAX-tick) detail::ThrowTickOverflow();return tick+delay;}
// Floor at the game's health quantum. Split the product to avoid overflow;
// validated numerator <= denominator ensures the final result fits capacity.
inline std::uint64_t PassengerDeathDamage(std::uint64_t capacity,const CompiledTransportDefinition &definition) noexcept
{
    assert(definition.deathDamageDenominator&&definition.deathDamageNumerator<=definition.deathDamageDenominator);
    return (capacity/definition.deathDamageDenominator)*definition.deathDamageNumerator
        +(capacity%definition.deathDamageDenominator)*definition.deathDamageNumerator/definition.deathDamageDenominator;
}
}
