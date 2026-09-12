module;
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>
export module games.generalszh.gameplay.capture.inputs.capture_batch;
export import engine.ecs.core.entity;
export namespace generalszh::capture
{
enum class CaptureAction : std::uint8_t {Start,Cancel};
struct CaptureInput {CaptureAction action{};ecs::Entity account{},actor{},target{};};
enum class CaptureOutcome : std::uint8_t {
    Started,Cancelled,Captured,Interrupted,InvalidActor,InvalidTarget,WrongOwner,
    Busy,Ineligible,UnsupportedSignature,InvalidBindings,InvalidAction
};
struct CaptureResult {ecs::Entity actor{},target{},account{};std::uint64_t tick{};CaptureOutcome outcome{};};
class CaptureBatch
{
public:
    static std::size_t RequiredResultCapacity(std::size_t inputCapacity,std::size_t entityCapacity) {
        if(entityCapacity>(SIZE_MAX-inputCapacity)/2) throw std::length_error("Capture result capacity overflow");
        return inputCapacity+2*entityCapacity;
    }
    CaptureBatch(std::size_t inputCapacity,std::size_t resultCapacity):capacity_(inputCapacity),resultCapacity_(resultCapacity)
    {inputs_.reserve(inputCapacity);results_.reserve(resultCapacity);}
    void SetInputs(std::span<const CaptureInput> inputs) {
        if(pending_) throw std::logic_error("Capture inputs already pending");
        if(inputs.size()>capacity_) throw std::length_error("Capture input capacity");
        inputs_.assign(inputs.begin(),inputs.end());pending_=true;published_=0;
    }
    std::span<const CaptureInput> BeginTick() {published_=0;results_.clear();return inputs_;}
    void Append(CaptureResult result) {
        if(results_.size()==resultCapacity_) throw std::length_error("Capture result capacity");
        results_.push_back(result);
    }
    void Publish() noexcept {published_=results_.size();inputs_.clear();pending_=false;}
    std::span<const CaptureResult> Results() const noexcept {return {results_.data(),published_};}
    std::size_t InputCapacity() const noexcept {return capacity_;}
    std::size_t ResultCapacity() const noexcept {return resultCapacity_;}
private:
    std::size_t capacity_,resultCapacity_,published_{};bool pending_{};
    std::vector<CaptureInput> inputs_;std::vector<CaptureResult> results_;
};
}
