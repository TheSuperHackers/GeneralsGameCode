module;
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <tuple>
#include <vector>
export module engine.gameplay.progression.inputs.progression_batch;
export import engine.ecs.core.entity;
export namespace engine::gameplay::progression
{
// Final nonnegative XP: attribution, routing and scaling have already happened.
// Span order is authoritative. Identical entries are distinct accepted awards.
struct AcceptedExperience { ecs::Entity entity; std::uint64_t amount{}; };
enum class ExperienceOutcome : std::uint8_t { Applied, Clamped, NotTrainable, InvalidTarget, InvalidState };
struct ExperienceResult
{
 ecs::Entity entity;
 std::uint64_t tick{}, requested{}, applied{}, experience{};
 std::uint32_t oldLevel{}, newLevel{};
 ExperienceOutcome outcome{ExperienceOutcome::InvalidTarget};
 bool LevelChanged() const noexcept { return oldLevel!=newLevel; }
};
// Root-owned bounded storage. Host inputs are staged at the boundary; ordered
// automatic producers may append before ProgressionSystem prepares the batch.
// Consumers of published results must explicitly follow ProgressionSystem in
// the graph.
class ProgressionBatch
{
public:
 explicit ProgressionBatch(std::size_t capacity):inputs_(capacity),results_(capacity),order_(capacity) {}
 void SetInputs(std::span<const AcceptedExperience> inputs)
 {
  if(pending_ || prepared_) throw std::logic_error("Progression inputs already pending");
  if(inputs.size()>inputs_.size()) throw std::length_error("Progression input capacity exceeded");
  for(std::size_t i=0;i<inputs.size();++i) inputs_[i]=inputs[i];
  count_=inputs.size(); published_=0; pending_=true;
 }
 // Automatic producers append after the host's explicitly staged inputs and
 // before ProgressionSystem prepares its canonical order. The capacity check
 // is performed before any copy so a failed append cannot partially publish an
 // authoritative award set.
 void Append(std::span<const AcceptedExperience> inputs)
 {
  if(!pending_ || prepared_) throw std::logic_error("Progression inputs are not appendable");
  if(inputs.size()>inputs_.size()-count_) throw std::length_error("Progression input capacity exceeded");
  for(std::size_t i=0;i<inputs.size();++i) inputs_[count_+i]=inputs[i];
  count_+=inputs.size();
 }
 std::size_t Capacity() const noexcept { return inputs_.size(); }
 // Automatic producers may inspect the host/earlier-producer prefix while the
 // batch is still pending.  The span is deliberately unavailable after
 // ProgressionSystem prepares the canonical order: callers must capture the
 // prefix before appending their own accepted inputs and must never retain the
 // view past the current joined execution boundary.
 std::span<const AcceptedExperience> PendingInputs() const
 {
  if(!pending_ || prepared_)
   throw std::logic_error("Progression inputs are not in the pending inspection window");
  return {inputs_.data(),count_};
 }
 std::span<const ExperienceResult> Results() const noexcept { return {results_.data(),published_}; }
 // Caller-side batch lifecycle. Gameplay remains in ProgressionSystem; storage
 // owns canonical grouping and publication, just like the existing AccountBatch.
 void Prepare(std::uint64_t tick)
 {
  if(prepared_) throw std::logic_error("Progression batch already prepared");
  published_=0;
  if(!pending_) count_=0;
  for(std::size_t i=0;i<count_;++i) {
   order_[i]=i; results_[i]={inputs_[i].entity,tick,inputs_[i].amount};
  }
  std::sort(order_.begin(),order_.begin()+count_,[this](auto a,auto b) {
   const auto x=inputs_[a].entity,y=inputs_[b].entity;
   return std::tuple{x.index,x.generation,a}<std::tuple{y.index,y.generation,b};
  });
  prepared_=true;
 }
 std::span<const std::size_t> Order() const noexcept
 { assert(prepared_); return {order_.data(),count_}; }
 const AcceptedExperience &Input(std::size_t index) const noexcept
 { assert(prepared_ && index<count_); return inputs_[index]; }
 ExperienceResult &Result(std::size_t index) noexcept
 { assert(prepared_ && index<count_); return results_[index]; }
 void Publish()
 {
  if(!prepared_) throw std::logic_error("Progression batch not prepared");
  published_=count_; pending_=false; prepared_=false; count_=0;
 }
private:
 std::vector<AcceptedExperience> inputs_;
 std::vector<ExperienceResult> results_;
 std::vector<std::size_t> order_;
 std::size_t count_{},published_{};
 bool pending_{},prepared_{};
};
}
