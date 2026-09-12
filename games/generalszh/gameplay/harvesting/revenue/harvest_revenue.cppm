module;
#include <algorithm>
#include <cstdint>
#include <memory>
#include <span>
#include <stdexcept>
#include <vector>
export module games.generalszh.gameplay.harvesting.revenue.harvest_revenue;
export import engine.gameplay.rts.harvesting.snapshots.harvest_snapshots;
export import games.generalszh.gameplay.harvesting.components.supply_dropoff_owner;
export import games.generalszh.gameplay.production.runtime.production_runtime;
export import games.generalszh.gameplay.economy.transactions.account_batch;
export namespace generalszh::harvesting {
using namespace engine::gameplay::rts::harvesting;
// Narrow derived conversion storage, injected into EconomySystem. Its joined
// hook converts delivered boxes to canonical account requests; no extra gameplay
// node, balance owner or child-system execution is needed for that conversion.
class HarvestRevenue {
    using Deliveries=ecs::Query<ecs::Read<HarvestDelivery>,ecs::Read<production::ProducedUnit>,ecs::Read<Health>,ecs::Read<LifeState>>;
    struct Row { ecs::Entity collector; economy::AccountRequest request; };
public:
    using Access=ecs::Query<ecs::Read<HarvestDelivery>,ecs::Read<production::ProducedUnit>,ecs::Read<SupplyDropoffOwner>,ecs::Read<SupplyDropoff>,ecs::Read<Health>,ecs::Read<LifeState>>;
    HarvestRevenue(ecs::World &world,std::size_t capacity=65536):world(world),capacity(capacity) { rows.reserve(capacity); outputs.reserve(capacity); }
    void Configure() {
        if(deliveries) throw std::logic_error("Harvest collection configuration is one-shot");
        deliveries=std::make_unique<Deliveries>(world);
    }
    std::span<const economy::AccountRequest> Outputs() const noexcept { return outputs; }
    std::span<const economy::AccountRequest> Collect() {
        rows.clear(); outputs.clear();
        deliveries->ForEachChunk([&](auto chunk){
            auto values=chunk.template Get<HarvestDelivery>(); auto units=chunk.template Get<production::ProducedUnit>();
            auto health=chunk.template Get<Health>(); auto life=chunk.template Get<LifeState>();
            for(std::size_t i=0;i<chunk.Count();++i) {
                if(!values[i].boxes||!health[i].current||!life[i].alive) continue;
                const auto *owner=world.Get<SupplyDropoffOwner>(values[i].dropoff);
                const auto *dock=world.Get<SupplyDropoff>(values[i].dropoff);
                const auto *dh=world.Get<Health>(values[i].dropoff); const auto *dl=world.Get<LifeState>(values[i].dropoff);
                if(!owner||!dock||!dock->available||!dh||!dl||!dh->current||!dl->alive||!owner->account.IsValid()) continue;
                // Selected allied dropoffs are allowed: the center owner receives
                // the credit, matching SupplyCenterDockUpdate::action.
                if(!units[i].account.IsValid()) continue;
                if(rows.size()==capacity) throw std::length_error("Harvest delivery capacity exhausted");
                const auto value=std::uint64_t{values[i].boxes}*owner->valuePerBox;
                if(value>UINT32_MAX) throw std::overflow_error("Harvest delivery exceeds account command range");
                rows.push_back({chunk.Entities()[i],{owner->account,{economy::AccountOperation::Deposit,static_cast<std::uint32_t>(value),true,true}}});
            }
        });
        std::sort(rows.begin(),rows.end(),[](const Row &a,const Row &b){return HarvestEntityLess(a.collector,b.collector);});
        for(const auto &row:rows) outputs.push_back(row.request);
        return outputs;
    }
private:
    ecs::World &world; std::size_t capacity; std::unique_ptr<Deliveries> deliveries;
    std::vector<Row> rows; std::vector<economy::AccountRequest> outputs;
};
}
