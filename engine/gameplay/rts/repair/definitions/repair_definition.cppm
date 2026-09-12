module;
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <numeric>
#include <span>
#include <stdexcept>
#include <vector>
export module engine.gameplay.rts.repair.definitions.repair_definition;
export import engine.time.simulation_time;
namespace engine::gameplay::rts::repair::detail
{
class RepairTickOverflow final : public std::overflow_error
{ public: RepairTickOverflow():std::overflow_error("Repair exceeds simulation tick horizon") {} };
[[noreturn]] void ThrowRepairTickOverflow() { throw RepairTickOverflow{}; }
}
export namespace engine::gameplay::rts::repair
{
enum class RepairIntervalPolicy : std::uint8_t
{
    AuthoredDuration,
    SimulationStep
};
inline constexpr bool IsValidRepairIntervalPolicy(RepairIntervalPolicy policy) noexcept
{
    return policy==RepairIntervalPolicy::AuthoredDuration||policy==RepairIntervalPolicy::SimulationStep;
}
struct RepairDefinition
{
    std::uint32_t rateNumerator{},rateDenominator{1}; // Fraction of capacity per second.
    time::Duration interval{},damageDelay{},initialDelay{};
    // Content that models a per-logic-update repair keeps the source rate per
    // second but asks startup compilation for one fixed-step pulse. The
    // default preserves existing authored-duration definitions.
    RepairIntervalPolicy intervalPolicy{RepairIntervalPolicy::AuthoredDuration};
};
struct CompiledRepairDefinition
{
    std::uint32_t numerator{},denominator{1}; // Fraction of capacity per actual pulse.
    std::uint64_t interval{},damageDelay{},initialDelay{};
};
class RepairDefinitions
{
public:
    RepairDefinitions(time::FixedStep step,std::span<const RepairDefinition> authored):step_(step)
    {
        if(authored.size()>UINT32_MAX) throw std::length_error("Repair catalog too large");
        entries_.reserve(authored.size());
        for(const auto &value:authored)
        {
            if(!IsValidRepairIntervalPolicy(value.intervalPolicy))
                throw std::invalid_argument("Unknown repair interval policy");
            if(!value.rateDenominator||value.interval.count()<=0) throw std::invalid_argument("Invalid repair ratio or interval");
            const auto interval=value.intervalPolicy==RepairIntervalPolicy::SimulationStep
                ? std::uint64_t{1}:step.TicksFor(value.interval);
            auto a=std::uint64_t{value.rateNumerator},b=interval;
            auto c=std::uint64_t{value.rateDenominator},d=std::uint64_t{step.TicksPerSecond()};
            // Cross-reduce before multiplication, then bound the compiled
            // denominator so portable quotient/remainder evaluation fits uint64.
            for(auto *top:{&a,&b}) for(auto *bottom:{&c,&d}) {
                const auto factor=std::gcd(*top,*bottom); *top/=factor; *bottom/=factor;
            }
            if((a&&b>UINT64_MAX/a)||(c&&d>UINT64_MAX/c)) throw std::out_of_range("Repair ratio product overflow");
            auto numerator=a*b,denominator=c*d;
            // More than full capacity per pulse has the same capped health effect.
            if(numerator>=denominator) numerator=denominator=1;
            if(numerator>UINT32_MAX||denominator>UINT32_MAX) throw std::out_of_range("Repair ratio exceeds compiled precision");
            entries_.push_back({static_cast<std::uint32_t>(numerator),static_cast<std::uint32_t>(denominator),
                interval,step.TicksFor(value.damageDelay),step.TicksFor(value.initialDelay)});
        }
    }
    time::FixedStep Step() const noexcept { return step_; }
    std::size_t Count() const noexcept { return entries_.size(); }
    const CompiledRepairDefinition &Get(std::uint32_t index) const { return entries_.at(index); }
    const CompiledRepairDefinition &GetUnchecked(std::uint32_t index) const noexcept
    {
        assert(index<entries_.size());
        return entries_[index];
    }
private:
    time::FixedStep step_;
    std::vector<CompiledRepairDefinition> entries_;
};
inline std::uint64_t RepairDeadline(std::uint64_t tick,std::uint64_t delay)
{
    if(delay>UINT64_MAX-tick) detail::ThrowRepairTickOverflow();
    return tick+delay;
}
}
