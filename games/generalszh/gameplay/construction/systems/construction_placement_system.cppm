module;
#include <algorithm>
#include <cassert>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <vector>
export module games.generalszh.gameplay.construction.systems.construction_placement_system;
export import games.generalszh.gameplay.construction.inputs.construction_batches;
export import games.generalszh.gameplay.bounty.components.cash_bounty_cost_binding;
export import games.generalszh.gameplay.construction.definitions.building_catalog;
export import games.generalszh.gameplay.construction.systems.construction_system;
export import games.generalszh.gameplay.demolition.definitions.demolition_trap_definition;
export import games.generalszh.gameplay.demolition.components.demolition_trap;
export import engine.gameplay.combat.death.components.death_weapon_state;
export import engine.gameplay.rts.economy.components.resource_balance;
export namespace generalszh::construction {
class ConstructionPlacementSystem {
    using Assignment=engine::gameplay::rts::construction::BuilderAssignment;
    using Site=engine::gameplay::rts::construction::ConstructionSite;
    using Position=engine::gameplay::navigation::GridPosition;
    using Goal=engine::gameplay::navigation::MoveGoal;
    using Range=engine::gameplay::navigation::MoveRangeGoal;
    using Credit=engine::gameplay::navigation::MoveCredit;
    using Life=engine::gameplay::combat::LifeState;
    using Health=engine::gameplay::combat::Health;
    using Balance=engine::gameplay::rts::economy::ResourceBalance;
    using Work=engine::gameplay::rts::production::BuildWork;
    using Enabled=engine::gameplay::rts::production::BuildEnabled;
    using Ready=engine::gameplay::rts::production::BuildReady;
public:
    using Query=ecs::Query<ecs::Read<Builder>,ecs::Write<Assignment>,ecs::Read<Life>,ecs::Read<Position>,
        ecs::Write<Goal>,ecs::Write<Credit>,ecs::OptionalWrite<Range>,ecs::Write<Balance>,ecs::Read<Structure>>;
    ConstructionPlacementSystem(ecs::World &world,const engine::gameplay::navigation::NavigationGrid &grid,
        const BuildingCatalog &catalog,ConstructionRequests &requests,ConstructionReceipts &receipts,std::size_t capacity=65536,
        const demolition::DemolitionTrapCatalog *demolitionCatalog=nullptr):
        world(world),grid(grid),catalog(catalog),requests(requests),receipts(receipts),capacity(capacity),
        demolitionCatalog(demolitionCatalog)
    { occupied.reserve(capacity); pendingBuilders.reserve(capacity); }
    void Configure() { structures=std::make_unique<Structures>(world); }
    void Execute(ecs::SystemContext &context)
    {
        if(context.Time().Step()!=catalog.Step()) throw std::logic_error("Construction step mismatch");
        receipts.Clear(); occupied.clear(); pendingBuilders.clear();
        const auto inputs=requests.Inputs(); const auto cancels=requests.Cancellations();
        // Ordered cancellation is a task stop: the paid scaffold and progress remain.
        for(const auto builder:cancels)
            if(auto *assignment=world.Get<Assignment>(builder); assignment && assignment->site.IsValid())
            {
                *assignment={};
                if(auto *goal=world.Get<Goal>(builder)) *goal={};
                if(auto *credit=world.Get<Credit>(builder))
                {
                    if(auto *range=world.Get<Range>(builder)) engine::gameplay::navigation::ClearMoveRangeGoal(*range,*credit);
                    *credit={};
                }
            }
        structures->ForEachChunk([&](auto chunk) {
            const auto positions=chunk.template Get<Position>(); const auto lives=chunk.template Get<Life>();
            for(std::size_t i=0;i<chunk.Count();++i) if(lives[i].alive)
            {
                if(occupied.size()==capacity) throw std::length_error("Construction site capacity exhausted");
                occupied.push_back(positions[i].cell);
            }
        });
        for(const auto &input:inputs)
        {
            auto status=ConstructionAcceptance::Accepted;
            const auto *builder=world.Get<Builder>(input.builder);
            const auto *life=world.Get<Life>(input.builder);
            const auto *assignment=world.Get<Assignment>(input.builder);
            const auto *entry=catalog.Find(input.definition);
            const auto *it=entry ? &entry->definition : nullptr;
            auto *balance=builder ? world.Get<Balance>(builder->account) : nullptr;
            if(std::find(cancels.begin(),cancels.end(),input.builder)!=cancels.end()) status=ConstructionAcceptance::Interrupted;
            else if(!builder || !life || !life->alive || !balance || !world.Get<Position>(input.builder)
                || !world.Get<Goal>(input.builder) || !world.Get<Credit>(input.builder)) status=ConstructionAcceptance::InvalidBuilder;
            else if((assignment && assignment->site.IsValid()) || std::find(pendingBuilders.begin(),pendingBuilders.end(),input.builder)!=pendingBuilders.end())
                status=ConstructionAcceptance::BuilderBusy;
            else if(!it) status=ConstructionAcceptance::UnknownDefinition;
            else if(!grid.Walkable(input.cell)) status=ConstructionAcceptance::InvalidCell;
            else if(std::find(occupied.begin(),occupied.end(),input.cell)!=occupied.end()) status=ConstructionAcceptance::Occupied;
            else if(balance->quantity<it->cost) status=ConstructionAcceptance::InsufficientFunds;
            const auto receipt=receipts.Append(status);
            if(status!=ConstructionAcceptance::Accepted) continue;
            if(occupied.size()==capacity) throw std::length_error("Construction site capacity exhausted");
            const auto created=context.Commands().Create();
            context.Commands().Add<Structure>(created,Structure{builder->account,it->key,it->queueLimit,it->energy,false,it->supplyDropoff,it->supplyBoxValue,it->repairDefinition,it->capturable,it->armor,
                it->visibility ? std::optional{it->visibility->id} : std::nullopt});
            if (it->cashBountyCost)
                context.Commands().Add<generalszh::bounty::CashBountyCostBinding>(created,*it->cashBountyCost);
            if (it->visibility)
            {
                context.Commands().Add<engine::gameplay::rts::visibility::VisibilityObserver>(created,
                    engine::gameplay::rts::visibility::VisibilityObserver{{}, it->visibility->id});
                context.Commands().Add<engine::gameplay::rts::visibility::VisibilityEligibility>(created);
            }
            // Armor protects the scaffold as soon as it is placed, not only
            // after completion. Definition binding was validated at startup.
            if(it->armor) context.Commands().Add<engine::gameplay::combat::damage::ArmorBinding>(created,*it->armor);
            context.Commands().Add<Site>(created,Site{input.builder});
            context.Commands().Add<Position>(created,Position{input.cell});
            context.Commands().Add<Health>(created,Health{1});
            context.Commands().Add<engine::gameplay::combat::HealthCapacity>(created,engine::gameplay::combat::HealthCapacity{it->health});
            context.Commands().Add<Life>(created);
            context.Commands().Add<engine::gameplay::combat::PendingDamage>(created);
            context.Commands().Add<engine::gameplay::combat::DamageResult>(created);
            context.Commands().Add<engine::gameplay::combat::PendingHealing>(created);
            context.Commands().Add<engine::gameplay::combat::HealingResult>(created);
            if (demolitionCatalog)
                if (const auto *trap=demolitionCatalog->Find(it->key))
                {
                    context.Commands().Add<engine::gameplay::combat::death::DeathWeaponBinding>(created,
                        engine::gameplay::combat::death::DeathWeaponBinding{trap->key});
                    context.Commands().Add<engine::gameplay::combat::death::DeathWeaponState>(created);
                    context.Commands().Add<demolition::DemolitionTrapState>(created,
                        demolition::DemolitionTrapState{0,trap->mode,false,false});
                }
            context.Commands().Add<ConstructionHealth>(created);
            context.Commands().Add<Work>(created,entry->work);
            context.Commands().Add<Enabled>(created); context.Commands().Add<Ready>(created);
            context.Commands().Add<ConstructionAdmission>(created,ConstructionAdmission{receipt});
            if(!assignment) context.Commands().Add<Assignment>(input.builder);
            balance->quantity-=it->cost;
            occupied.push_back(input.cell); pendingBuilders.push_back(input.builder);
        }
        requests.Clear();
    }
private:
    using Structures=ecs::Query<ecs::Read<Structure>,ecs::Read<Position>,ecs::Read<Life>>;
    ecs::World &world;
    const engine::gameplay::navigation::NavigationGrid &grid;
    const BuildingCatalog &catalog;
    ConstructionRequests &requests;
    ConstructionReceipts &receipts;
    std::size_t capacity;
    const demolition::DemolitionTrapCatalog *demolitionCatalog{};
    std::unique_ptr<Structures> structures;
    std::vector<engine::gameplay::navigation::Cell> occupied;
    std::vector<ecs::Entity> pendingBuilders;
};
}
export namespace ecs {
template<> struct SystemTraits<generalszh::construction::ConstructionPlacementSystem> {
 static constexpr std::string_view StableName="games.generalszh.construction.placement";
 static constexpr SystemPhase Phase=SystemPhase::Simulation; static constexpr bool Batch=true;
 using Before=SystemTypeList<>; using After=SystemTypeList<>;
};
}
