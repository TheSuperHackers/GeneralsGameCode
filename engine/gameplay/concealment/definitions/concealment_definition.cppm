module;
#include <cassert>
#include <chrono>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>
export module engine.gameplay.concealment.definitions.concealment_definition;
export import engine.time.simulation_time;

export namespace engine::gameplay::concealment
{
struct ConcealmentDefinition
{
    time::Duration stealthDelay{};
    bool enabledByDefault{true};
    friend constexpr bool operator==(const ConcealmentDefinition &, const ConcealmentDefinition &) noexcept = default;
};

class ConcealmentDefinitionId
{
public:
    constexpr ConcealmentDefinitionId() noexcept = default;
    constexpr bool operator==(const ConcealmentDefinitionId &) const noexcept = default;
private:
    explicit constexpr ConcealmentDefinitionId(std::uint32_t value) noexcept : value(value) {}
    std::uint32_t value{(std::numeric_limits<std::uint32_t>::max)()};
    friend class ConcealmentDefinitions;
};

struct CompiledConcealmentDefinition
{
    std::uint64_t stealthDelayTicks{};
    bool enabledByDefault{true};
};

class ConcealmentDefinitions
{
public:
    ConcealmentDefinitions(time::FixedStep step, std::span<const ConcealmentDefinition> authored) :
        step(step), authored(authored.begin(), authored.end()), entries(Compile(step, authored)) {}

    time::FixedStep Step() const noexcept { return step; }

    ConcealmentDefinitionId Id(std::uint32_t index) const
    {
        if (index >= entries.size()) throw std::out_of_range("Concealment definition index");
        return ConcealmentDefinitionId{index};
    }

    ConcealmentDefinitionId Id(const ConcealmentDefinition &definition) const
    {
        for (std::uint32_t index = 0; index != authored.size(); ++index)
            if (authored[index] == definition) return ConcealmentDefinitionId{index};
        throw std::out_of_range("Concealment definition is absent from catalog");
    }

    const CompiledConcealmentDefinition &Get(ConcealmentDefinitionId id) const
    {
        assert(id.value < entries.size());
        return entries[id.value];
    }

private:
    static std::vector<CompiledConcealmentDefinition> Compile(time::FixedStep step,
        std::span<const ConcealmentDefinition> authored)
    {
        if (authored.size() >= (std::numeric_limits<std::uint32_t>::max)())
            throw std::length_error("Too many concealment definitions");
        std::vector<CompiledConcealmentDefinition> result;
        result.reserve(authored.size());
        for (const auto &definition : authored)
            result.push_back({step.TicksFor(definition.stealthDelay), definition.enabledByDefault});
        return result;
    }

    time::FixedStep step;
    std::vector<ConcealmentDefinition> authored;
    std::vector<CompiledConcealmentDefinition> entries;
};
}
