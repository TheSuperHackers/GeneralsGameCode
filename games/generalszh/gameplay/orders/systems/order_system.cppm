module;
#include <span>
#include <stdexcept>
#include <string_view>
export module games.generalszh.gameplay.orders.systems.order_system;
export import engine.gameplay.rts.orders.systems.engagement_system;
export import engine.gameplay.rts.ai.squads.inputs.squad_order_request_batch;
export import games.generalszh.gameplay.combat.acquisition.acquisition_system;
export import games.generalszh.gameplay.orders.inputs.attack_input;
export namespace generalszh
{
// Cohesive order behaviour: accept ordered requests, evaluate pursuit/movement
// intent and publish weapon intent in one system. No harvesting/building policy.
class OrderSystem
{
    using Order = engine::gameplay::rts::orders::UnitOrder;
    using Kind = engine::gameplay::rts::orders::OrderKind;
    using MoveInput = engine::gameplay::navigation::MoveInput;
    using Input = engine::gameplay::rts::orders::OrderInput;
    using SquadOrderRequestBatch = engine::gameplay::rts::ai::squads::SquadOrderRequestBatch;
public:
    using Query = ecs::Query<ecs::Write<Order>, ecs::Read<engine::gameplay::combat::LifeState>,
        ecs::Read<engine::gameplay::navigation::GridPosition>, ecs::Write<engine::gameplay::navigation::MoveGoal>,
        ecs::Write<engine::gameplay::navigation::MoveCredit>,
        ecs::OptionalWrite<engine::gameplay::navigation::MoveRangeGoal>,
        ecs::Write<engine::gameplay::rts::orders::EngagementObservation>, ecs::Write<engine::gameplay::rts::orders::EngagementResult>,
        ecs::Optional<engine::gameplay::combat::WeaponDefinition>, ecs::Optional<combat::AcquisitionPolicy>,
        ecs::OptionalWrite<engine::gameplay::combat::WeaponTarget>, ecs::OptionalWrite<combat::TargetIntent>,
        ecs::Optional<engine::gameplay::containment::PassengerMembership>>;
    OrderSystem(ecs::World &world, const engine::gameplay::navigation::NavigationGrid &grid,
        combat::TargetIndex &index, std::size_t capacity, SquadOrderRequestBatch *squadRequests = nullptr) :
        world(world), grid(grid), index(index), frame(index.Frame()), capacity(capacity), squadRequests(squadRequests) {}
    // Order acceptance reads the last committed visibility map.  The injected
    // TargetIndex does not exempt those ECS reads from graph metadata.
    using AuxiliaryAccess = combat::TargetIndex::VisibilityAccess;

    void SetInputs(std::span<const MoveInput> moves = {}, std::span<const AttackInput> attacks = {},
        std::span<const Input> inputs = {})
    {
        if (world.IsScheduledExecutionActive()) throw std::logic_error("Order inputs require a joined boundary");
        if (moves.size() > capacity || attacks.size() > capacity || inputs.size() > capacity)
            throw std::length_error("Order input capacity exhausted");
        for (const auto &input : moves)
            if (input.destination != engine::gameplay::navigation::InvalidCell && !grid.Walkable(input.destination))
                throw std::invalid_argument("Order destination must be traversable");
        for (const auto &input : inputs) Validate(input.order);
        this->moves = moves; this->attacks = attacks; this->inputs = inputs;
    }
    void BeforeChunks(Query &, ecs::SystemContext &context)
    {
        index.Rebuild(context.Tick());
        if (squadRequests != nullptr)
        {
            if (!squadRequests->IsPublished() || squadRequests->Tick() != context.Tick())
                throw std::logic_error("Squad order requests require a published batch for this step");
            // OrderSystem is the sole consumer and releaser of this injected
            // batch. The composition root must not release it as well: the
            // published lifetime ends here, after the requests have been
            // copied into authoritative UnitOrder state. The next tick's
            // tactical BeforeChunks may then Begin() the same batch.
            // Tactical requests are consumed before this system's ordinary
            // input boundary; manual input below is the same-tick override.
            for (const auto &request : squadRequests->Requests())
                Apply(request.input.actor, request.input.order);
            squadRequests->Release();
        }
        // Accepted group precedence, not worker completion/arrival timing.
        for (const auto &input : moves)
            Apply(input.actor, {input.destination == engine::gameplay::navigation::InvalidCell ? Kind::Stop : Kind::Move,
                input.destination, {}});
        for (const auto &input : attacks)
            Apply(input.actor, {input.target.IsValid() ? Kind::AttackTarget : Kind::Stop,
                engine::gameplay::navigation::InvalidCell, input.target});
        for (const auto &input : inputs) Apply(input.actor, input.order);
        moves = {}; attacks = {}; inputs = {};
    }
    void Execute(Query::Chunk chunk, ecs::SystemContext &) const noexcept
    {
        using namespace engine::gameplay::rts::orders;
        using namespace engine::gameplay::navigation;
        auto orders=chunk.Get<Order>(); const auto life=chunk.Get<engine::gameplay::combat::LifeState>();
        const auto positions=chunk.Get<GridPosition>(); auto goals=chunk.Get<MoveGoal>(); auto credits=chunk.Get<MoveCredit>();
        auto ranges=chunk.Get<MoveRangeGoal>();
        auto observations=chunk.Get<EngagementObservation>(); auto results=chunk.Get<EngagementResult>();
        const auto weapons=chunk.Get<engine::gameplay::combat::WeaponDefinition>(); const auto policies=chunk.Get<combat::AcquisitionPolicy>();
        auto targets=chunk.Get<engine::gameplay::combat::WeaponTarget>(); auto intents=chunk.Get<combat::TargetIntent>();
        const auto members=chunk.Get<engine::gameplay::containment::PassengerMembership>();
        for (std::size_t row=0; row!=chunk.Count(); ++row)
        {
            observations[row]={};
            const bool contained = !members.empty() && engine::gameplay::containment::IsContained(members[row]);
            const bool garrisonFire = contained && engine::gameplay::containment::IsGarrisonOwned(members[row]) &&
                engine::gameplay::containment::CanPassengerFire(members[row]);
            if (contained && !garrisonFire)
            {
                orders[row]={}; goals[row]={}; credits[row]={}; results[row]={};
                if(!ranges.empty()) ClearMoveRangeGoal(ranges[row],credits[row]);
                if(!targets.empty()) targets[row]={};
                if(!intents.empty()) intents[row]={};
                continue;
            }
            if (garrisonFire && !IsGarrisonOrder(orders[row].kind))
            {
                orders[row]={}; goals[row]={}; credits[row]={}; results[row]={};
                if(!ranges.empty()) ClearMoveRangeGoal(ranges[row],credits[row]);
                if(!targets.empty()) targets[row]={};
                if(!intents.empty()) intents[row]={};
                continue;
            }
            const auto actor=chunk.Entities()[row];
            if (!weapons.empty())
            {
                if (orders[row].kind==OrderKind::AttackPosition)
                {
                    const auto aim=orders[row].destination;
                    if (positions[row].cell < grid.Count() && aim < grid.Count())
                        observations[row]={ {}, aim,
                            IsWithinGridRadius(positions[row].cell,aim,grid.Width(),weapons[row].rangeCells), true };
                }
                else if (frame.Contains(actor))
                {
                    ecs::Entity target{};
                    if (orders[row].kind==OrderKind::AttackTarget) target=orders[row].target;
                    else if (orders[row].kind==OrderKind::AttackMove)
                        target=frame.Nearest(actor,weapons[row].rangeCells,combat::UnitTarget |
                            (!policies.empty() && policies[row].attackBuildings ? combat::BuildingTarget : 0));
                    else if (orders[row].kind==OrderKind::GuardPosition)
                    {
                        target=orders[row].target;
                        if (!IsValidGuardTarget(actor,target,orders[row]))
                            target=frame.Nearest(orders[row].destination,frame.Owner(actor),orders[row].guard.innerRadiusCells,
                                combat::UnitTarget);
                        if (IsValidGuardTarget(actor,target,orders[row])) orders[row].target=target;
                        else { orders[row].target={}; target={}; }
                    }
                    if (frame.CanTarget(actor, target))
                    {
                        const auto from=frame.Position(actor), to=frame.Position(target), width=grid.Width();
                        const auto x=from%width,y=from/width,tx=to%width,ty=to/width;
                        const std::uint64_t dx=x>tx?x-tx:tx-x,dy=y>ty?y-ty:ty-y,radius=weapons[row].rangeCells;
                        observations[row]={target,to,dx*dx+dy*dy<=radius*radius,true};
                    }
                }
            }
            else if (orders[row].kind==OrderKind::GuardPosition) orders[row].target={};
            AdvanceEngagement(orders[row],observations[row],life[row],positions[row],goals[row],credits[row],results[row]);
            if (garrisonFire)
            {
                // A garrison may publish weapon intent, but an out-of-range
                // observation must never become a passenger movement goal.
                goals[row]={}; credits[row]={};
            }
            if(!ranges.empty())
            {
                if(!garrisonFire && orders[row].kind==OrderKind::AttackPosition && observations[row].positionValid &&
                    !observations[row].inRange && results[row].positionValid && !weapons.empty())
                    SetMoveRangeGoal(ranges[row],credits[row],observations[row].position,weapons[row].rangeCells);
                else ClearMoveRangeGoal(ranges[row],credits[row]);
            }
            if (results[row].controlsWeapon)
            {
                if (!targets.empty())
                {
                    if (results[row].target.IsValid()) targets[row]=engine::gameplay::combat::WeaponTarget::ForEntity(results[row].target);
                    else if (results[row].positionValid)
                    {
                        const auto width=grid.Width();
                        targets[row]=engine::gameplay::combat::WeaponTarget::ForPosition({results[row].position%width,
                            results[row].position/width});
                    }
                    else targets[row]={};
                }
                if (!intents.empty()) intents[row].explicitOrder=(results[row].target.IsValid() || results[row].positionValid) &&
                    orders[row].kind!=OrderKind::GuardPosition;
            }
        }
    }
private:
    bool IsValidGuardTarget(ecs::Entity actor, ecs::Entity target, const Order &order) const noexcept
    {
        return frame.CanTarget(actor, target) &&
            engine::gameplay::rts::orders::IsWithinGridRadius(order.destination,frame.Position(target),frame.Width(),
                order.guard.outerRadiusCells);
    }
    static bool IsGarrisonOrder(const Kind kind) noexcept
    {
        return kind==Kind::AttackTarget || kind==Kind::AttackPosition ||
            kind==Kind::GuardPosition || kind==Kind::Stop;
    }
    void Validate(Order input) const
    {
        switch (input.kind)
        {
        case Kind::Move: case Kind::AttackMove:
            if (!grid.Walkable(input.destination)) throw std::invalid_argument("Order destination must be traversable");
            break;
        case Kind::GuardPosition:
            if (!grid.Walkable(input.destination)) throw std::invalid_argument("Guard center must be traversable");
            if (input.guard.innerRadiusCells>input.guard.outerRadiusCells)
                throw std::invalid_argument("Guard inner radius must not exceed outer radius");
            break;
        case Kind::AttackPosition:
            if (input.destination == engine::gameplay::navigation::InvalidCell || input.destination >= grid.Count())
                throw std::invalid_argument("Attack position must be inside the navigation grid");
            if (input.target.IsValid()) throw std::invalid_argument("Attack position cannot carry an entity target");
            break;
        case Kind::AttackTarget: case Kind::Stop: case Kind::None: break;
        default: throw std::invalid_argument("Unknown order kind");
        }
    }
    void Apply(ecs::Entity actor, Order input)
    {
        if (input.kind == Kind::None) return;
        if (input.kind == Kind::GuardPosition) input.target={};
        auto *order = world.Get<Order>(actor);
        const auto *life = world.Get<engine::gameplay::combat::LifeState>(actor);
        const auto *member=world.Get<engine::gameplay::containment::PassengerMembership>(actor);
        if (!order || !life || !life->alive) return;
        const bool contained = member && engine::gameplay::containment::IsContained(*member);
        const bool garrisonFire = contained && engine::gameplay::containment::IsGarrisonOwned(*member) &&
            engine::gameplay::containment::CanPassengerFire(*member);
        if (contained && (!garrisonFire || !IsGarrisonOrder(input.kind))) return;
        *order = input;
    }
    ecs::World &world;
    const engine::gameplay::navigation::NavigationGrid &grid;
    combat::TargetIndex &index;
    const combat::TargetFrame &frame;
    std::size_t capacity;
    SquadOrderRequestBatch *squadRequests{};
    std::span<const MoveInput> moves;
    std::span<const AttackInput> attacks;
    std::span<const Input> inputs;
};
}
export namespace ecs
{
template<> struct SystemTraits<generalszh::OrderSystem>
{
    static constexpr std::string_view StableName = "games.generalszh.order_inputs";
    static constexpr SystemPhase Phase = SystemPhase::Simulation;
    using Before = SystemTypeList<>; using After = SystemTypeList<>;
};
}
