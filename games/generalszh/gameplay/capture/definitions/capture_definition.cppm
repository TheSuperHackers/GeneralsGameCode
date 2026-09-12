module;
#include <cassert>
#include <chrono>
#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>
export module games.generalszh.gameplay.capture.definitions.capture_definition;
export import engine.time.simulation_time;
export import games.generalszh.gameplay.capture.components.capture_state;
namespace generalszh::capture::detail {
class TickOverflow final:public std::overflow_error {
public: TickOverflow():std::overflow_error("Capture tick horizon exceeded") {} };
[[noreturn]] void ThrowOverflow() { throw TickOverflow{}; }
}
export namespace generalszh::capture
{
struct CaptureUpgradeBinding
{
    // This is the existing production BuildDefinition key stored by
    // ResearchedUpgrade. It is resolved at the content boundary and checked
    // against the frozen BuildCatalog; it is not a runtime name hash or dense
    // registration-order ID.
    std::uint32_t definition{};
};
struct CaptureDefinition {
    engine::time::Duration unpack{},preparation{},recovery{},recharge{};
    bool startsReady{};
    std::optional<CaptureUpgradeBinding> activationUpgrade{};
};
struct CompiledCaptureDefinition {
    std::uint64_t captureTicks{},recoveryTicks{},unpackTicks{},rechargeTicks{};
    bool startsReady{};
    std::optional<CaptureUpgradeBinding> activationUpgrade{};
};
inline std::uint64_t CaptureDeadline(std::uint64_t tick,std::uint64_t delay)
{ if(delay>UINT64_MAX-tick) detail::ThrowOverflow();return tick+delay; }
inline CaptureRecharge InitialCaptureRecharge(const CompiledCaptureDefinition &definition,std::uint64_t creationTick)
{ return {CaptureDeadline(creationTick,definition.startsReady?0:definition.rechargeTicks),true}; }
class CaptureDefinitions
{
public:
    CaptureDefinitions(engine::time::FixedStep step,std::span<const CaptureDefinition> definitions):step_(step) {
        if(definitions.size()>UINT32_MAX) throw std::length_error("Capture definition capacity");
        entries_.reserve(definitions.size());
        for(const auto &value:definitions) {
            if(value.unpack.count()<0||value.preparation.count()<0||value.recovery.count()<0||value.recharge.count()<0
                ||value.unpack.count()>INT64_MAX-value.preparation.count())
                throw std::invalid_argument("Invalid capture duration");
            entries_.push_back({step.TicksFor(value.unpack+value.preparation),step.TicksFor(value.recovery),
                step.TicksFor(value.unpack),step.TicksFor(value.recharge),value.startsReady,value.activationUpgrade});
        }
    }
    engine::time::FixedStep Step() const noexcept {return step_;}
    const CompiledCaptureDefinition &Get(std::uint32_t index) const {return entries_.at(index);}
    const CompiledCaptureDefinition &GetUnchecked(std::uint32_t index) const noexcept
    {assert(index<entries_.size());return entries_[index];}
private:
    engine::time::FixedStep step_;
    std::vector<CompiledCaptureDefinition> entries_;
};
}
