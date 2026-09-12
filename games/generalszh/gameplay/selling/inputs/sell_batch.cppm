module;
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>
export module games.generalszh.gameplay.selling.inputs.sell_batch;
export import engine.ecs.core.entity;
export namespace generalszh::selling
{
struct SellRequest { ecs::Entity structure{}, account{}; };
enum class SellAcceptance : std::uint8_t
{
    Accepted, InvalidTarget, UnsupportedSignature, WrongOwner, Ineligible,
    Dead, AlreadySelling, UnknownDefinition, MissingAccount
};
struct SellReceipt { ecs::Entity structure{}; SellAcceptance status{SellAcceptance::InvalidTarget}; };
enum class SellOutcome : std::uint8_t { Sold, CancelledDeath, MissingAccount };
struct SellResult
{
    ecs::Entity structure{}, account{};
    std::uint64_t tick{};
    std::uint32_t refunded{};
    SellOutcome outcome{SellOutcome::Sold};
};
// Root-owned bounded IO only. It contains no authoritative sale state, balance,
// systems or execution graph. A result consumer must be ordered after SellSystem.
class SellBatch
{
public:
    SellBatch(std::size_t requestCapacity,std::size_t resultCapacity)
        : inputs(requestCapacity),receipts(requestCapacity),results(resultCapacity) {}
    void SetInputs(std::span<const SellRequest> source)
    {
        if (pending || executing) throw std::logic_error("Sale input is already pending or executing");
        if (source.size()>inputs.size()) throw std::length_error("Sale input capacity exhausted");
        for (std::size_t i=0;i<source.size();++i) inputs[i]=source[i];
        inputCount=source.size(); pending=true;
    }
    std::span<const SellReceipt> Receipts() const noexcept { return {receipts.data(),receiptCount}; }
    std::span<const SellResult> Results() const noexcept { return {results.data(),resultCount}; }
    std::size_t ResultCapacity() const noexcept { return results.size(); }
    std::span<const SellRequest> BeginTick()
    {
        if (executing) throw std::logic_error("Sale batch execution is not reentrant");
        if (!pending) inputCount=0;
        executing=true; receiptCount=0; resultCount=0;
        return {inputs.data(),inputCount};
    }
    void Accept(std::size_t index,SellReceipt receipt) { receipts.at(index)=receipt; }
    void AddResult(SellResult result)
    {
        if (resultCount==results.size()) throw std::length_error("Sale result capacity exhausted");
        results[resultCount++]=result;
    }
    void Publish() noexcept { receiptCount=inputCount; pending=false; executing=false; inputCount=0; }
private:
    std::vector<SellRequest> inputs;
    std::vector<SellReceipt> receipts;
    std::vector<SellResult> results;
    std::size_t inputCount{},receiptCount{},resultCount{};
    bool pending{},executing{};
};
}
