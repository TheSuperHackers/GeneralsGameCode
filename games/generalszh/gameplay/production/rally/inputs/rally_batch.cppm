module;
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>
export module games.generalszh.gameplay.production.rally.inputs.rally_batch;
export import engine.ecs.core.entity;
export import engine.gameplay.navigation.grid.navigation_grid;
export namespace generalszh::production::rally
{
struct RallyInput
{
    ecs::Entity producer{},account{};
    engine::gameplay::navigation::Cell destination{engine::gameplay::navigation::InvalidCell};
};
enum class RallyAcceptance : std::uint8_t
{ Accepted, Cleared, InvalidProducer, WrongOwner, NotAlive, Inactive, InvalidDestination, Unreachable };
struct RallyReceipt
{
    ecs::Entity producer{};
    RallyAcceptance status{RallyAcceptance::InvalidProducer};
};
// Root-owned bounded boundary storage, not gameplay state. Staging copies input
// values. Distinct receipt elements may be written by different chunk jobs.
class RallyBatch
{
public:
    explicit RallyBatch(std::size_t capacity):inputs_(capacity),receipts_(capacity) {}
    std::size_t Capacity() const noexcept {return inputs_.size();}
    void SetInputs(std::span<const RallyInput> inputs)
    {
        if(pending_||executing_) throw std::logic_error("Rally input batch already pending or executing");
        if(inputs.size()>inputs_.size()) throw std::length_error("Rally input capacity exhausted");
        for(std::size_t i=0;i<inputs.size();++i) inputs_[i]=inputs[i];
        count_=inputs.size();published_=0;pending_=true;
    }
    std::span<const RallyReceipt> Receipts() const noexcept {return {receipts_.data(),published_};}
    // Scheduler-owned lifecycle. No cross-module friend declarations required.
    std::span<const RallyInput> Prepare()
    {
        if(executing_) throw std::logic_error("Rally batch already executing");
        executing_=true;published_=0;if(!pending_) count_=0;
        for(std::size_t i=0;i<count_;++i) receipts_[i]={inputs_[i].producer,RallyAcceptance::InvalidProducer};
        return {inputs_.data(),count_};
    }
    void Record(std::size_t index,RallyAcceptance status) noexcept
    {assert(executing_&&index<count_);receipts_[index].status=status;}
    void Publish() noexcept
    {assert(executing_);published_=count_;count_=0;pending_=false;executing_=false;}
private:
    std::vector<RallyInput> inputs_;
    std::vector<RallyReceipt> receipts_;
    std::size_t count_{},published_{};
    bool pending_{},executing_{};
};
}
