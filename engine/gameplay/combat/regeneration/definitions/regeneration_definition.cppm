module;
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>
export module engine.gameplay.combat.regeneration.definitions.regeneration_definition;
export import engine.time.simulation_time;

namespace engine::gameplay::combat::regeneration::detail
{
// Match periodic_income.cppm's tested Windows Clang exception-transfer boundary:
// imported inline throws can omit owning copy metadata for std::exception_ptr.
class RegenerationTickOverflow final : public std::overflow_error
{
public:
    RegenerationTickOverflow() : std::overflow_error("Regeneration exceeds simulation tick horizon") {}
};
[[noreturn]] void ThrowRegenerationTickOverflow() { throw RegenerationTickOverflow{}; }
}

export namespace engine::gameplay::combat::regeneration
{
struct RegenerationDefinition
{
    std::uint64_t quantity{};
    time::Duration interval{};
    time::Duration damageDelay{};
    // Some policies defer a damage-triggered wake even when damageDelay is zero.
    time::Duration damageWakeLatency{};
    bool strictDamageWakeCooldown{false};
};
class RegenerationDefinitionId
{
public:
    constexpr RegenerationDefinitionId() noexcept = default;
    constexpr bool operator==(const RegenerationDefinitionId &) const noexcept = default;
private:
    explicit constexpr RegenerationDefinitionId(std::uint32_t index) noexcept : index(index) {}
    std::uint32_t index{UINT32_MAX};
    friend class RegenerationDefinitions;
};
struct CompiledRegenerationDefinition
{
    std::uint64_t quantity{}, interval{}, damageDelay{}, damageWakeLatency{};
    bool strictDamageWakeCooldown{};
};
class RegenerationDefinitions
{
public:
    RegenerationDefinitions(time::FixedStep step, std::span<const RegenerationDefinition> definitions)
        : step(step), entries(Compile(step, definitions)) {}
    time::FixedStep Step() const noexcept { return step; }
    RegenerationDefinitionId Id(std::uint32_t index) const
    {
        if (index >= entries.size()) throw std::out_of_range("Regeneration definition index");
        return RegenerationDefinitionId{index};
    }
    const CompiledRegenerationDefinition &Get(RegenerationDefinitionId id) const
    {
        if (id.index >= entries.size()) throw std::out_of_range("Unbound regeneration definition");
        return entries[id.index];
    }
private:
    static std::vector<CompiledRegenerationDefinition> Compile(time::FixedStep step,
        std::span<const RegenerationDefinition> definitions)
    {
        if (definitions.size() >= UINT32_MAX) throw std::length_error("Too many regeneration definitions");
        std::vector<CompiledRegenerationDefinition> result;
        result.reserve(definitions.size());
        for (const auto &definition : definitions)
        {
            if (definition.interval.count() <= 0) throw std::invalid_argument("Regeneration interval must be positive");
            result.push_back({definition.quantity, step.TicksFor(definition.interval),
                step.TicksFor(definition.damageDelay), step.TicksFor(definition.damageWakeLatency),
                definition.strictDamageWakeCooldown});
        }
        return result;
    }
    const time::FixedStep step;
    const std::vector<CompiledRegenerationDefinition> entries;
};
// Exhausting the simulation horizon is fatal, never wraparound or silent stalling.
inline std::uint64_t RegenerationDeadline(std::uint64_t tick, std::uint64_t delay)
{
    if (delay > (std::numeric_limits<std::uint64_t>::max)() - tick)
        detail::ThrowRegenerationTickOverflow();
    return tick + delay;
}
}
