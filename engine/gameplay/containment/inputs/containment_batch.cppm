module;
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>
export module engine.gameplay.containment.inputs.containment_batch;
export import engine.ecs.core.entity;
export namespace engine::gameplay::containment
{
enum class ContainmentAction : std::uint8_t { Enter, Exit };
struct ContainmentInput { ContainmentAction action{}; ecs::Entity passenger{},carrier{}; };
enum class ContainmentOutcome : std::uint8_t
{
    Entered,Exited,InvalidPassenger,InvalidCarrier,NotAlive,WrongOwner,NotArrived,
    Full,AlreadyContained,NotContained,AwaitingEvacuation,ExitBusy,UnsupportedActor,InvalidAction,
    BoardingQueued,Interrupted
};
struct ContainmentResult
{
    ecs::Entity passenger{},carrier{};
    std::uint64_t tick{};
    ContainmentOutcome outcome{ContainmentOutcome::InvalidPassenger};
};
// One input transaction batch; host staging must be outside scheduled execution.
// Span order is authoritative. Results remain readable until next staging/tick.
class ContainmentBatch
{
public:
    explicit ContainmentBatch(std::size_t capacity):inputs_(capacity),results_(capacity) {}
    void SetInputs(std::span<const ContainmentInput> inputs)
    {
        if(pending_) throw std::logic_error("Containment batch already pending");
        if(inputs.size()>inputs_.size()) throw std::length_error("Containment input capacity exceeded");
        for(std::size_t i=0;i<inputs.size();++i) inputs_[i]=inputs[i];
        count_=inputs.size();published_=0;pending_=true;
    }
    void Prepare(std::uint64_t tick) noexcept
    {
        published_=0;if(!pending_) count_=0;
        for(std::size_t i=0;i<count_;++i) results_[i]={inputs_[i].passenger,inputs_[i].carrier,tick};
    }
    std::span<const ContainmentInput> Inputs() const noexcept {return {inputs_.data(),count_};}
    ContainmentResult &Result(std::size_t index) noexcept {assert(index<count_);return results_[index];}
    void Publish() noexcept {published_=count_;count_=0;pending_=false;}
    std::span<const ContainmentResult> Results() const noexcept {return {results_.data(),published_};}
private:
    std::vector<ContainmentInput> inputs_;
    std::vector<ContainmentResult> results_;
    std::size_t count_{},published_{};
    bool pending_{};
};
}
