module;
#include <cassert>
#include <chrono>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>
export module engine.gameplay.concealment.definitions.detection_definition;
export import engine.time.simulation_time;

export namespace engine::gameplay::concealment
{
struct DetectionDefinition
{
    time::Duration detectionRate{};
    std::uint32_t detectionRangeCells{};
    // This describes the detector's own contained/transported eligibility.
    // It does not make contained targets detectable.
    bool canDetectWhileContained{};
    bool initiallyDisabled{};
    friend constexpr bool operator==(const DetectionDefinition &, const DetectionDefinition &) noexcept = default;
};

class DetectionDefinitionId
{
public:
    constexpr DetectionDefinitionId() noexcept = default;
    constexpr bool operator==(const DetectionDefinitionId &) const noexcept = default;
private:
    explicit constexpr DetectionDefinitionId(std::uint32_t value) noexcept : value(value) {}
    std::uint32_t value{(std::numeric_limits<std::uint32_t>::max)()};
    friend class DetectionDefinitions;
};

struct CompiledDetectionDefinition
{
    std::uint64_t detectionRateTicks{};
    std::uint32_t detectionRangeCells{};
    bool canDetectWhileContained{};
    bool initiallyDisabled{};
};

class DetectionDefinitions
{
public:
    DetectionDefinitions(time::FixedStep step, std::span<const DetectionDefinition> authored) :
        step(step), authored(authored.begin(), authored.end()), entries(Compile(step, authored)) {}

    time::FixedStep Step() const noexcept { return step; }

    DetectionDefinitionId Id(std::uint32_t index) const
    {
        if (index >= entries.size()) throw std::out_of_range("Detection definition index");
        return DetectionDefinitionId{index};
    }

    DetectionDefinitionId Id(const DetectionDefinition &definition) const
    {
        for (std::uint32_t index = 0; index != authored.size(); ++index)
            if (authored[index] == definition) return DetectionDefinitionId{index};
        throw std::out_of_range("Detection definition is absent from catalog");
    }

    const CompiledDetectionDefinition &Get(DetectionDefinitionId id) const
    {
        assert(id.value < entries.size());
        return entries[id.value];
    }

private:
    static std::vector<CompiledDetectionDefinition> Compile(time::FixedStep step,
        std::span<const DetectionDefinition> authored)
    {
        if (authored.size() >= (std::numeric_limits<std::uint32_t>::max)())
            throw std::length_error("Too many detection definitions");
        std::vector<CompiledDetectionDefinition> result;
        result.reserve(authored.size());
        for (const auto &definition : authored)
        {
            if (definition.detectionRate.count() <= 0)
                throw std::invalid_argument("Detection rate must be positive");
            if (!definition.detectionRangeCells)
                throw std::invalid_argument("Detection range must be positive");
            const auto ticks = step.TicksFor(definition.detectionRate);
            if (!ticks) throw std::invalid_argument("Detection rate rounds to zero simulation ticks");
            result.push_back({ticks, definition.detectionRangeCells, definition.canDetectWhileContained,
                definition.initiallyDisabled});
        }
        return result;
    }

    time::FixedStep step;
    std::vector<DetectionDefinition> authored;
    std::vector<CompiledDetectionDefinition> entries;
};
}
