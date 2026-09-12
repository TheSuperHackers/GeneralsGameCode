module;
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>
export module games.generalszh.gameplay.production.inputs.build_inputs;
export import engine.ecs.system.system;
export import games.generalszh.gameplay.production.components.build_admission;
export namespace generalszh
{
enum class BuildAcceptance { Accepted, InvalidProducer, UnknownDefinition, QueueFull, InsufficientFunds, DuplicateUpgrade, PrerequisitesUnmet };
struct BuildInput { ecs::Entity producer{}; std::uint32_t definition{}; };
struct BuildReceipt { BuildAcceptance status{}; ecs::Entity order{}; };
// The composition root owns lifecycle state. Failure is terminal there.
enum class ProductionBoundaryState { Configuring, Ready, Failed };
inline void RequireProductionBoundary(const ecs::World &world, ProductionBoundaryState state)
{
    if (state != ProductionBoundaryState::Ready || !world.ComponentsFinalized() || world.IsScheduledExecutionActive())
        throw std::logic_error("Production is not at an input boundary");
}
class BuildBatch
{
public:
    explicit BuildBatch(std::size_t capacity = 65536) : capacity_(capacity)
    {
        ownedInputs.reserve(capacity);
        receipts.reserve(capacity);
    }
    void StageInputs(const ecs::World &world, ProductionBoundaryState state, std::span<const BuildInput> value)
    {
        RequireProductionBoundary(world, state);
        if (value.size() > capacity_) throw std::length_error("Build input capacity exhausted");
        // This is the external tick boundary.  It also retires the previous
        // tick's generated copy after admission has consumed its span.
        ownedInputs.clear();
        externalInputs = value;
        externalInputCount = value.size();
        generatedCount = 0;
        staged = true;
        inputs = value;
    }
    // Called only by a scheduler caller-side join (currently the AI
    // AfterChunks hook), never by a worker and never through StageInputs.
    // The external prefix is copied before generated requests are appended so
    // admission receipt indices for external inputs remain unchanged.
    void AppendGenerated(const ecs::World &world, std::span<const BuildInput> value)
    {
        if (!world.IsScheduledExecutionActive())
            throw std::logic_error("Generated production inputs require scheduled execution");
        if (!staged)
            throw std::logic_error("Generated production inputs require staged external inputs");
        if (generatedCount > capacity_ - externalInputs.size())
            throw std::length_error("Generated build input capacity exhausted");
        const auto used = externalInputs.size() + generatedCount;
        if (value.size() > capacity_ - used)
            throw std::length_error("Generated build input capacity exhausted");
        if (value.empty()) return;
        if (ownedInputs.empty() && generatedCount == 0)
        {
            ownedInputs.reserve(capacity_);
            ownedInputs.insert(ownedInputs.end(), externalInputs.begin(), externalInputs.end());
        }
        ownedInputs.insert(ownedInputs.end(), value.begin(), value.end());
        generatedCount += value.size();
        inputs = ownedInputs;
    }
    // Admission takes the combined external-prefix/generated span exactly
    // once.  The owned storage and counters remain readable until the next
    // StageInputs call, but no subsequent producer can republish this tick's
    // requests.
    std::span<const BuildInput> ConsumeInputs() noexcept
    {
        if (!staged) return {};
        const auto value = inputs;
        inputs = {};
        externalInputs = {};
        staged = false;
        return value;
    }
    std::size_t Capacity() const noexcept { return capacity_; }
    std::size_t ExternalInputCount() const noexcept { return externalInputCount; }
    std::size_t GeneratedInputCount() const noexcept { return generatedCount; }
    bool UsesOwnedInputs() const noexcept { return !ownedInputs.empty(); }
    std::span<const BuildReceipt> Receipts() const noexcept { return receipts; }
    // Scheduler producer clears/resizes before receipt jobs; jobs write distinct slots.
    std::span<const BuildInput> inputs;
    std::vector<BuildReceipt> receipts;
private:
    const std::size_t capacity_;
    std::vector<BuildInput> ownedInputs;
    std::span<const BuildInput> externalInputs;
    std::size_t externalInputCount{};
    std::size_t generatedCount{};
    bool staged{};
};
}
