module;
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>
export module games.generalszh.gameplay.demolition.algorithms.demolition_target_index;
export import games.generalszh.gameplay.demolition.components.demolition_trap;
export import games.generalszh.gameplay.production.components.production_state;
export import games.generalszh.gameplay.construction.components.structure;
export import engine.gameplay.combat.components.health;
export import engine.gameplay.containment.components.passenger_membership;
export import engine.gameplay.navigation.components.movement;
export import engine.gameplay.spatial.grid.point_grid;
export import engine.ecs.query.query;

export namespace generalszh::demolition
{
struct DemolitionTargetObservation
{
    ecs::Entity entity{};
    ecs::Entity group{};
    bool structure{};
    bool airborne{};
    bool unattackable{};
    bool disarmingDozer{};
};

class DemolitionTargetIndex
{
public:
    using Access=ecs::Query<ecs::Read<engine::gameplay::combat::Health>,ecs::Read<engine::gameplay::combat::LifeState>,
        ecs::Read<engine::gameplay::navigation::GridPosition>,ecs::Optional<production::ProducedUnit>,
        ecs::Optional<production::Producer>,ecs::Optional<construction::Structure>,
        ecs::Optional<engine::gameplay::containment::PassengerMembership>,
        ecs::Optional<DemolitionTargetClassification>>;

    DemolitionTargetIndex(ecs::World &world,const engine::gameplay::navigation::NavigationGrid &grid,
        std::size_t capacity) : world_(world),grid_(grid),targets_(world),index_(grid.Width(),grid.Count()/grid.Width(),capacity),
        generations_(capacity),records_(capacity)
    {
        if (!capacity) throw std::invalid_argument("Demolition target capacity must be positive");
        touched_.reserve(capacity);
    }

    void Rebuild()
    {
        for (const auto index:touched_) generations_[index]=ecs::Entity::InvalidGeneration;
        touched_.clear(); index_.Clear();
        targets_.ForEachChunk([&](auto chunk) {
            const auto health=chunk.template Get<engine::gameplay::combat::Health>();
            const auto life=chunk.template Get<engine::gameplay::combat::LifeState>();
            const auto positions=chunk.template Get<engine::gameplay::navigation::GridPosition>();
            const auto units=chunk.template Get<production::ProducedUnit>();
            const auto producers=chunk.template Get<production::Producer>();
            const auto structures=chunk.template Get<construction::Structure>();
            const auto memberships=chunk.template Get<engine::gameplay::containment::PassengerMembership>();
            const auto classes=chunk.template Get<DemolitionTargetClassification>();
            for (std::size_t row=0;row!=chunk.Count();++row)
            {
                if (!life[row].alive || !health[row].current || positions[row].cell>=grid_.Count()) continue;
                if (!memberships.empty() && engine::gameplay::containment::IsContained(memberships[row])) continue;
                const auto entity=chunk.Entities()[row];
                if (entity.index>=generations_.size()) throw std::length_error("Demolition target entity-index capacity exhausted");
                if (generations_[entity.index]!=ecs::Entity::InvalidGeneration)
                    throw std::logic_error("Duplicate demolition target entity index");
                const auto group=!units.empty()?units[row].account:!producers.empty()?producers[row].account:
                    !structures.empty()?structures[row].account:ecs::Entity{};
                generations_[entity.index]=entity.generation;
                records_[entity.index]={entity,group,!structures.empty(),!classes.empty()&&classes[row].airborne,
                    !classes.empty()&&classes[row].unattackable,
                    !classes.empty()&&classes[row].disarmingDozer};
                touched_.push_back(entity.index);
                index_.Add({entity,group,positions[row].cell%grid_.Width(),positions[row].cell/grid_.Width(),1});
            }
        });
        index_.Publish();
    }

    template<typename Visitor>
    void VisitWithinRadius(engine::gameplay::spatial::GridPoint center,std::uint32_t radius,Visitor &&visitor) const
    {
        index_.VisitWithinRadius(center.x,center.y,radius,[&](const auto point) {
            if (point.entity.index>=generations_.size() || generations_[point.entity.index]!=point.entity.generation) return;
            const auto &record=records_[point.entity.index];
            visitor(DemolitionTargetObservation{record.entity,record.group,record.structure,
                record.airborne,record.unattackable,record.disarmingDozer});
        });
    }

    [[nodiscard]] std::size_t EntityCapacity() const noexcept { return generations_.size(); }
    [[nodiscard]] std::uint32_t Width() const noexcept { return index_.Width(); }

private:
    struct Record
    {
        ecs::Entity entity{};
        ecs::Entity group{};
        bool structure{};
        bool airborne{};
        bool unattackable{};
        bool disarmingDozer{};
    };
    ecs::World &world_;
    const engine::gameplay::navigation::NavigationGrid &grid_;
    Access targets_;
    engine::gameplay::spatial::PointGrid index_;
    std::vector<ecs::EntityGeneration> generations_;
    std::vector<Record> records_;
    std::vector<ecs::EntityIndex> touched_;
};
}
