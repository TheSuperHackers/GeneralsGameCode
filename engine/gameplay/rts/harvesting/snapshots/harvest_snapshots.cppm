module;
#include <algorithm>
#include <stdexcept>
#include <tuple>
#include <vector>
export module engine.gameplay.rts.harvesting.snapshots.harvest_snapshots;
export import engine.gameplay.rts.harvesting.components.harvest_state;
export namespace engine::gameplay::rts::harvesting {
using namespace engine::gameplay::navigation;
using engine::gameplay::combat::Health;
using engine::gameplay::combat::LifeState;
inline bool HarvestEntityLess(ecs::Entity a,ecs::Entity b) noexcept { return std::tie(a.index,a.generation)<std::tie(b.index,b.generation); }
// Bounded sparse snapshots: storage depends on participating docks, never entity index.
struct HarvestSnapshots {
    struct Dock { ecs::Entity entity; Cell cell; bool available; };
    explicit HarvestSnapshots(std::size_t capacity):capacity(capacity) { sources.reserve(capacity); dropoffs.reserve(capacity); }
    std::size_t capacity;
    std::vector<Dock> sources,dropoffs;
};
inline const HarvestSnapshots::Dock *FindHarvestDock(const std::vector<HarvestSnapshots::Dock> &rows,ecs::Entity entity) noexcept {
        auto it=std::lower_bound(rows.begin(),rows.end(),entity,[](const HarvestSnapshots::Dock &a,ecs::Entity b){return HarvestEntityLess(a.entity,b);});
        return it!=rows.end()&&it->entity==entity?&*it:nullptr;
}
inline void AppendHarvestDock(std::vector<HarvestSnapshots::Dock> &rows,HarvestSnapshots::Dock dock,std::size_t capacity) {
        if(rows.size()>=capacity) throw std::length_error("Harvest dock snapshot capacity exhausted");
        rows.push_back(dock);
}
}
