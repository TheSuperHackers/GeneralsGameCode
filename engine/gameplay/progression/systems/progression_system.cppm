module;
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <tuple>
export module engine.gameplay.progression.systems.progression_system;
export import engine.gameplay.progression.components.progression_state;
export import engine.gameplay.progression.definitions.progression_definition;
export import engine.gameplay.progression.inputs.progression_batch;
export import engine.ecs.system.system;
export namespace engine::gameplay::progression
{
class ProgressionSystem
{
public:
 // Query access covers the same four components used by BeforeChunks. The
  // catalog is immutable; ordered automatic producers append before this
  // system prepares the batch. Any other system reading Results must have an
  // explicit OrderBefore edge from this system: current scheduler metadata
  // does not infer injected-resource edges.
 using Query=ecs::Query<ecs::Write<ProgressionState>,ecs::Read<ProgressionDefinitionRef>,
  ecs::Read<ProgressionEligibility>,ecs::Write<ProgressionInbox>>;
 ProgressionSystem(const ProgressionCatalog &catalog,ProgressionBatch &batch):catalog_(catalog),batch_(batch) {}
 void BeforeChunks(Query &query,ecs::SystemContext &context)
 {
  batch_.Prepare(context.Tick());
  query.ForEachPreparedChunk([](Query::Chunk chunk) {
   for(auto &inbox:chunk.Get<ProgressionInbox>()) inbox={};
  });
  const auto order=batch_.Order();
  auto &world=context.GetWorld();
  for(std::size_t begin=0;begin<order.size();)
  {
   const auto entity=batch_.Input(order[begin]).entity;
   auto end=begin+1;
   while(end<order.size() && batch_.Input(order[end]).entity==entity) ++end;
   auto *inbox=world.Get<ProgressionInbox>(entity);
   const auto *state=world.Get<ProgressionState>(entity);
   const auto *reference=world.Get<ProgressionDefinitionRef>(entity);
   if(inbox && state && reference && world.Get<ProgressionEligibility>(entity))
   {
    // Validate externally bound state once per target at acceptance, not for
    // every award in the chunk hot loop. Failed targets retain explicit results.
    const auto *definition=reference->index<catalog_.Size()?&catalog_.Get(reference->index):nullptr;
    if(!definition || state->experience>definition->maximumExperience || state->level>=definition->thresholds.size())
     for(auto i=begin;i<end;++i) {
      auto &result=batch_.Result(order[i]); result.outcome=ExperienceOutcome::InvalidState;
      result.experience=state->experience; result.oldLevel=result.newLevel=state->level;
     }
    else *inbox={begin,end-begin};
   }
   begin=end;
  }
 }
 void Execute(Query::Chunk chunk,ecs::SystemContext &) const
 {
  auto states=chunk.Get<ProgressionState>();
  const auto refs=chunk.Get<ProgressionDefinitionRef>();
  const auto eligibility=chunk.Get<ProgressionEligibility>();
  auto inboxes=chunk.Get<ProgressionInbox>();
  const auto order=batch_.Order();
  for(std::size_t row=0;row<states.size();++row)
  {
   auto &state=states[row];
   const auto inbox=inboxes[row];
   if(!inbox.count) continue;
   const auto &definition=catalog_.Get(refs[row].index);
   assert(state.experience<=definition.maximumExperience && state.level<definition.thresholds.size());
   for(std::size_t offset=0;offset<inbox.count;++offset)
   {
    const auto index=order[inbox.begin+offset];
    auto &result=batch_.Result(index);
    result.experience=state.experience; result.oldLevel=result.newLevel=state.level;
    if(!eligibility[row].trainable) {result.outcome=ExperienceOutcome::NotTrainable;continue;}
    const auto room=definition.maximumExperience-state.experience;
    result.applied=(std::min)(result.requested,room);
    result.outcome=result.applied==result.requested?ExperienceOutcome::Applied:ExperienceOutcome::Clamped;
    state.experience+=result.applied;
    // As in the reference, even a zero award recomputes the level, and a
    // multi-level award produces exactly one old-to-final transition.
    state.level=0;
    while(state.level+1<definition.thresholds.size()
     && state.experience>=definition.thresholds[state.level+1]) ++state.level;
    result.experience=state.experience; result.newLevel=state.level;
   }
   inboxes[row]={};
  }
 }
 void AfterChunks(Query &,ecs::SystemContext &) { batch_.Publish(); }
private:
 const ProgressionCatalog &catalog_;
 ProgressionBatch &batch_;
};
}
export namespace ecs
{
template<> struct SystemTraits<engine::gameplay::progression::ProgressionSystem>
{
 static constexpr std::string_view StableName="engine.gameplay.progression.system";
 static constexpr SystemPhase Phase=SystemPhase::Simulation;
 using Before=SystemTypeList<>;
 using After=SystemTypeList<>;
};
}
