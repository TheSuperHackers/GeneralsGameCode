module;
#include <algorithm>
#include <limits>
#include <span>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <vector>
export module games.generalszh.gameplay.tasks.systems.task_selection_system;
export import engine.events.storage.event_batch;
export import engine.gameplay.containment.components.containment_task;
export import games.generalszh.gameplay.orders.systems.order_system;
export import games.generalszh.gameplay.capture.inputs.capture_batch;
export import engine.gameplay.rts.construction.components.construction_components;
export import engine.gameplay.rts.harvesting.components.harvest_state;
export import engine.gameplay.rts.repair.components.manual_repair;

export namespace generalszh::tasks
{
// Cross-capability task interruption belongs here, not in ordinary order input.
// Components remain the sole owners of assignment/order/actuator state. Sorted
// input identities are temporary observation data, shared immutably by chunk jobs.
class TaskSelectionSystem
{
    using Assignment=engine::gameplay::rts::construction::BuilderAssignment;
    using Order=engine::gameplay::rts::orders::UnitOrder;
    using OrderInput=engine::gameplay::rts::orders::OrderInput;
    using MoveInput=engine::gameplay::navigation::MoveInput;
    using Goal=engine::gameplay::navigation::MoveGoal;
    using Range=engine::gameplay::navigation::MoveRangeGoal;
    using Credit=engine::gameplay::navigation::MoveCredit;
    using WeaponTarget=engine::gameplay::combat::WeaponTarget;
    using ContainmentTask=engine::gameplay::containment::ContainmentTask;
    using CaptureInput=capture::CaptureInput;
    using Harvest=engine::gameplay::rts::harvesting::HarvestState;
    using ManualRepairAssignment=engine::gameplay::rts::repair::ManualRepairAssignment;
    using AcceptedRepairBatch=engine::events::PublishedBatch<ecs::Entity>;
public:
    using Query=ecs::Query<ecs::OptionalWrite<Assignment>,ecs::OptionalWrite<Harvest>,ecs::Write<Order>,
        ecs::Read<engine::gameplay::combat::LifeState>,ecs::Write<Goal>,ecs::Write<Credit>,ecs::OptionalWrite<Range>,
        ecs::OptionalWrite<WeaponTarget>,ecs::OptionalWrite<combat::TargetIntent>,ecs::Optional<capture::CaptureActorState>,
        ecs::Optional<ManualRepairAssignment>,ecs::Optional<ContainmentTask>>;
    explicit TaskSelectionSystem(std::size_t capacity,const AcceptedRepairBatch *acceptedRepairs=nullptr):capacity(capacity),
        acceptedRepairs(acceptedRepairs)
    {
        if(capacity>(std::numeric_limits<std::size_t>::max)()/4)
            throw std::length_error("Task input capacity overflow");
        manual.reserve(3*capacity); boardingOverrides.reserve(4*capacity); captureStarts.reserve(capacity);
        cancelled.reserve(capacity);
    }
    void BeforeChunks(Query &,ecs::SystemContext &)
    {
        acceptedActors={};
        if(acceptedRepairs) {
            if(!acceptedRepairs->IsPublished()) throw std::logic_error("Task selection repair batch was not published");
            // Capture this read only after the repair system has published it;
            // SetInputs never holds a stale publication span.
            acceptedActors=acceptedRepairs->Values();
        }
    }
    void SetInputs(std::span<const MoveInput> moveInputs,std::span<const AttackInput> attackInputs,
        std::span<const OrderInput> orderInputs,std::span<const ecs::Entity> cancellations,
        std::span<const CaptureInput> captureInputs={})
    {
        if(moveInputs.size()>capacity || attackInputs.size()>capacity || orderInputs.size()>capacity
            || cancellations.size()>capacity || captureInputs.size()>capacity)
            throw std::length_error("Task input capacity exhausted");
        // Pure normalization at input staging, no ECS gameplay mutation. Earlier
        // gathering consumes this same immutable cancellation set before acting.
        manual.clear(); boardingOverrides.clear(); captureStarts.clear();
        cancelled.assign(cancellations.begin(),cancellations.end());
        for(const auto &input:moveInputs) manual.push_back(input.actor);
        for(const auto &input:attackInputs) manual.push_back(input.actor);
        for(const auto &input:orderInputs)
            if(input.order.kind!=engine::gameplay::rts::orders::OrderKind::None) manual.push_back(input.actor);
        for(const auto &input:captureInputs)
            if(input.action==capture::CaptureAction::Start) captureStarts.push_back(input.actor);
        std::sort(manual.begin(),manual.end(),Less);
        manual.erase(std::unique(manual.begin(),manual.end()),manual.end());
        boardingOverrides=manual;
        boardingOverrides.insert(boardingOverrides.end(),captureStarts.begin(),captureStarts.end());
        std::sort(boardingOverrides.begin(),boardingOverrides.end(),Less);
        boardingOverrides.erase(std::unique(boardingOverrides.begin(),boardingOverrides.end()),boardingOverrides.end());
        std::sort(captureStarts.begin(),captureStarts.end(),Less);
        captureStarts.erase(std::unique(captureStarts.begin(),captureStarts.end()),captureStarts.end());
        std::sort(cancelled.begin(),cancelled.end(),Less);
        cancelled.erase(std::unique(cancelled.begin(),cancelled.end()),cancelled.end());
    }
    std::span<const ecs::Entity> ManualActors() const noexcept { return manual; }
    std::span<const ecs::Entity> BoardingOverrides() const noexcept { return boardingOverrides; }
    void AfterChunks(Query &,ecs::SystemContext &) noexcept
    { manual.clear();boardingOverrides.clear();captureStarts.clear();cancelled.clear();acceptedActors={}; }
    void Execute(Query::Chunk chunk,ecs::SystemContext &) const noexcept
    {
        auto assignments=chunk.Get<Assignment>(); auto orders=chunk.Get<Order>();
        auto harvest=chunk.Get<Harvest>();
        const auto life=chunk.Get<engine::gameplay::combat::LifeState>();
        auto goals=chunk.Get<Goal>(); auto credits=chunk.Get<Credit>(); auto ranges=chunk.Get<Range>();
        auto targets=chunk.Get<WeaponTarget>(); auto intents=chunk.Get<combat::TargetIntent>();
        const auto captureStates=chunk.Get<capture::CaptureActorState>();
        const auto repairAssignments=chunk.Get<ManualRepairAssignment>();
        const auto boardingTasks=chunk.Get<ContainmentTask>();
        for(std::size_t row=0;row<chunk.Count();++row)
        {
            const auto entity=chunk.Entities()[row];
            if(!captureStates.empty() && capture::IsCaptureBusy(captureStates[row]))
            {
                // Capture owns its deadline/reservation; this cross-task system
                // prevents old ordinary intent from running during that task.
                orders[row]={}; goals[row]={}; credits[row]={};
                if(!ranges.empty()) engine::gameplay::navigation::ClearMoveRangeGoal(ranges[row],credits[row]);
                if(!targets.empty()) targets[row]={};
                if(!intents.empty()) intents[row]={};
                continue;
            }
            const bool boardingActive=!boardingTasks.empty()&&life[row].alive&&boardingTasks[row].carrier.IsValid();
            if(boardingActive)
            {
                // Transport owns the live route and wrote MoveGoal before this
                // system. Suppress only stale ordinary intent; do not rewrite
                // the route or its fractional movement credit.
                orders[row]={};
                if(!ranges.empty()) engine::gameplay::navigation::ClearMoveRangeGoal(ranges[row],credits[row]);
                if(!targets.empty()) targets[row]={};
                if(!intents.empty()) intents[row]={};
                continue;
            }
            // Placement precedes manual orders in the current grouped input
            // contract: interruption preserves any already-paid scaffold.
            const bool repairAccepted=std::binary_search(acceptedActors.begin(),acceptedActors.end(),
                entity,Less);
            const bool repairActive=!repairAssignments.empty()&&repairAssignments[row].target.IsValid();
            if(life[row].alive&&(repairAccepted||repairActive))
            {
                if(!assignments.empty()) assignments[row]={};
                if(!harvest.empty()) harvest[row]={};
                orders[row]={};
                if(!ranges.empty()) engine::gameplay::navigation::ClearMoveRangeGoal(ranges[row],credits[row]);
                if(!targets.empty()) targets[row]={};
                if(!intents.empty()) intents[row]={};
                if(repairActive)
                {
                    if(goals[row].cell!=repairAssignments[row].cell) credits[row]={};
                    goals[row].cell=repairAssignments[row].cell;
                }
                else { goals[row]={}; credits[row]={}; }
                continue;
            }
            if(life[row].alive && std::binary_search(manual.begin(),manual.end(),entity,Less))
            {
                if(!assignments.empty()) assignments[row]={};
                if(!ranges.empty()) engine::gameplay::navigation::ClearMoveRangeGoal(ranges[row],credits[row]);
                if(!harvest.empty()) harvest[row]={};
                continue;
            }
            if(!life[row].alive || std::binary_search(cancelled.begin(),cancelled.end(),entity,Less))
            {
                if(!assignments.empty()) assignments[row]={};
                goals[row]={}; credits[row]={};
                if(!ranges.empty()) engine::gameplay::navigation::ClearMoveRangeGoal(ranges[row],credits[row]);
                if(!life[row].alive && !harvest.empty()) harvest[row]={};
            }
            else if(!assignments.empty() && assignments[row].site.IsValid())
            {
                if(!ranges.empty()) engine::gameplay::navigation::ClearMoveRangeGoal(ranges[row],credits[row]);
                if(!harvest.empty()) harvest[row]={};
            }
            else if(harvest.empty() || harvest[row].phase==engine::gameplay::rts::harvesting::HarvestPhase::Idle
                || harvest[row].phase==engine::gameplay::rts::harvesting::HarvestPhase::NeedsInput) continue;
            // Ongoing construction supersedes old movement/fire intent. The
            // later OrderSystem may apply a new manual command, but None leaves
            // BuilderTaskSystem's route intact. No child-system execution here.
            orders[row]={};
            if(!ranges.empty()) engine::gameplay::navigation::ClearMoveRangeGoal(ranges[row],credits[row]);
            if(!targets.empty()) targets[row]={};
            if(!intents.empty()) intents[row]={};
        }
    }
private:
    static bool Less(ecs::Entity a,ecs::Entity b) noexcept
    { return std::tie(a.index,a.generation)<std::tie(b.index,b.generation); }
    std::size_t capacity;
    std::vector<ecs::Entity> manual,boardingOverrides,captureStarts,cancelled;
    const AcceptedRepairBatch *acceptedRepairs{};
    std::span<const ecs::Entity> acceptedActors;
};
}
export namespace ecs
{
template<> struct SystemTraits<generalszh::tasks::TaskSelectionSystem>
{
    static constexpr std::string_view StableName="games.generalszh.tasks.selection";
    static constexpr SystemPhase Phase=SystemPhase::Simulation;
    using Before=SystemTypeList<>; using After=SystemTypeList<>;
};
}
