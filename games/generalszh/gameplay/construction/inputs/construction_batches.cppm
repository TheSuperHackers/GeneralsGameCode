module;
#include <cassert>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>
export module games.generalszh.gameplay.construction.inputs.construction_batches;
export import games.generalszh.gameplay.construction.components.construction_admission;
export import games.generalszh.gameplay.construction.definitions.building_definition;
export namespace generalszh::construction
{
struct ConstructionInput { ecs::Entity builder{}; std::uint32_t definition{}; engine::gameplay::navigation::Cell cell{}; };
enum class ConstructionAcceptance { Accepted,InvalidBuilder,BuilderBusy,UnknownDefinition,InvalidCell,Occupied,InsufficientFunds,Interrupted };
struct ConstructionReceipt { ConstructionAcceptance status{}; ecs::Entity site{}; };
// Root stages borrowed immutable spans at a joined boundary; admission consumes once.
class ConstructionRequests
{
public:
    explicit ConstructionRequests(std::size_t capacity=65536):capacity(capacity) {}
    void SetInputs(std::span<const ConstructionInput> values,std::span<const ecs::Entity> cancellations={})
    {
        if(values.size()>capacity || cancellations.size()>capacity) throw std::length_error("Construction inputs exhausted");
        inputs=values; cancels=cancellations;
    }
    std::span<const ConstructionInput> Inputs() const noexcept { return inputs; }
    std::span<const ecs::Entity> Cancellations() const noexcept { return cancels; }
    void Clear() noexcept { inputs={}; cancels={}; }
private:
    std::size_t capacity;
    std::span<const ConstructionInput> inputs;
    std::span<const ecs::Entity> cancels;
};
// Ordered receipt storage shared only by admission (append) and resolve (entity IDs).
class ConstructionReceipts
{
public:
    explicit ConstructionReceipts(std::size_t capacity=65536):capacity(capacity) { values.reserve(capacity); }
    void Clear() noexcept { values.clear(); }
    std::size_t Append(ConstructionAcceptance status)
    {
        if(values.size()==capacity) throw std::length_error("Construction receipts exhausted");
        const auto index=values.size(); values.push_back({status,{}}); return index;
    }
    void Resolve(std::size_t index,ecs::Entity site) noexcept
    { assert(index<values.size()); values[index].site=site; }
    std::span<const ConstructionReceipt> Receipts() const noexcept { return values; }
private:
    std::size_t capacity;
    std::vector<ConstructionReceipt> values;
};
}
