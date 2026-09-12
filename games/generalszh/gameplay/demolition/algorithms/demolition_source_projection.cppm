module;
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>
export module games.generalszh.gameplay.demolition.algorithms.demolition_source_projection;
export import games.generalszh.gameplay.construction.components.structure;
export import games.generalszh.gameplay.selling.components.sale_state;
export import engine.gameplay.combat.death.components.death_weapon_state;
export import engine.gameplay.navigation.components.movement;
export import engine.ecs.query.query;

export namespace generalszh::demolition
{
class DemolitionSourceProjection
{
public:
    using Access=ecs::Query<ecs::Read<construction::Structure>,ecs::Read<engine::gameplay::navigation::GridPosition>,
        ecs::Optional<selling::SaleState>>;

    DemolitionSourceProjection(ecs::World &world,const engine::gameplay::navigation::NavigationGrid &grid,
        std::size_t capacity) : grid_(grid),sources_(world),generations_(capacity),records_(capacity)
    {
        if (!capacity) throw std::invalid_argument("Demolition source capacity must be positive");
        touched_.reserve(capacity);
    }

    void Refresh()
    {
        for (const auto index:touched_) generations_[index]=ecs::Entity::InvalidGeneration;
        touched_.clear();
        sources_.ForEachChunk([&](auto chunk) {
            const auto structures=chunk.template Get<construction::Structure>();
            const auto positions=chunk.template Get<engine::gameplay::navigation::GridPosition>();
            const auto sales=chunk.template Get<selling::SaleState>();
            for (std::size_t row=0;row!=chunk.Count();++row)
            {
                const auto entity=chunk.Entities()[row];
                if (entity.index>=generations_.size()) throw std::length_error("Demolition source entity-index capacity exhausted");
                if (generations_[entity.index]!=ecs::Entity::InvalidGeneration)
                    throw std::logic_error("Duplicate demolition source entity index");
                if (positions[row].cell>=grid_.Count()) continue;
                generations_[entity.index]=entity.generation;
                records_[entity.index]={structures[row].account,structures[row].definition,structures[row].complete,
                    !sales.empty() && sales[row].phase!=selling::SalePhase::Idle,positions[row].cell};
                touched_.push_back(entity.index);
            }
        });
    }

    [[nodiscard]] engine::gameplay::combat::death::DeathSourceCapture Capture(ecs::Entity entity) const
    {
        if (!entity.IsValid() || entity.index>=generations_.size() || generations_[entity.index]!=entity.generation)
            return {};
        const auto &record=records_[entity.index];
        if (!record.complete || record.saleActive)
            return {engine::gameplay::combat::death::DeathSourceDisposition::Exempt,{},0,{}};
        if (!record.account.IsValid() || record.position==engine::gameplay::navigation::InvalidCell)
            return {};
        return {engine::gameplay::combat::death::DeathSourceDisposition::Eligible,record.account,record.definition,
            {record.position%grid_.Width(),record.position/grid_.Width()}};
    }

    [[nodiscard]] std::size_t EntityCapacity() const noexcept { return generations_.size(); }

private:
    struct Record
    {
        ecs::Entity account{};
        std::uint32_t definition{};
        bool complete{};
        bool saleActive{};
        engine::gameplay::navigation::Cell position{engine::gameplay::navigation::InvalidCell};
    };
    const engine::gameplay::navigation::NavigationGrid &grid_;
    Access sources_;
    std::vector<ecs::EntityGeneration> generations_;
    std::vector<Record> records_;
    std::vector<ecs::EntityIndex> touched_;
};
}
