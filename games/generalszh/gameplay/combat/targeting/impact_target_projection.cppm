module;
#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>
export module games.generalszh.gameplay.combat.targeting.impact_target_projection;
export import games.generalszh.gameplay.production.runtime.production_runtime;
export import games.generalszh.gameplay.construction.definitions.building_definition;
export import engine.gameplay.combat.systems.health_system;
export import engine.gameplay.combat.damage.components.damage_packet;
export import engine.gameplay.containment.components.passenger_membership;
export import engine.gameplay.navigation.components.movement;
export import engine.gameplay.navigation.grid.navigation_grid;
export import engine.gameplay.spatial.grid.point_grid;
export namespace generalszh::combat
{
// Impact-time derived view. ECS components remain authoritative; this object
// only owns a refreshed typed query, a generation-indexed lookup and the
// reusable PointGrid scratch used by ProjectileSystemT.
class ImpactTargetProjection
{
    using Health = engine::gameplay::combat::Health;
    using Life = engine::gameplay::combat::LifeState;
    using Position = engine::gameplay::navigation::GridPosition;
    using Membership = engine::gameplay::containment::PassengerMembership;
    using Armor = engine::gameplay::combat::damage::ArmorBinding;
    using ArmorDefinitionRef = engine::gameplay::combat::damage::ArmorDefinitionRef;
    using Structure = construction::Structure;
public:
    static constexpr bool Spatial = true;
    using Access = ecs::Query<ecs::Read<Life>, ecs::Read<Health>, ecs::Read<engine::gameplay::combat::DamageResult>,
        ecs::Write<engine::gameplay::combat::PendingDamage>, ecs::Read<Position>, ecs::Optional<Membership>,
        ecs::Optional<production::ProducedUnit>, ecs::Optional<production::Producer>, ecs::Optional<Structure>,
        ecs::Optional<Armor>>;
    struct TargetRecord { bool hasArmor{}; ArmorDefinitionRef armor{}; };

    ImpactTargetProjection(ecs::World &world, const engine::gameplay::navigation::NavigationGrid &grid,
        std::size_t entityIndexCapacity, std::size_t pointCapacity) :
        grid(grid), targets(world), index(grid.Width(), grid.Count() / grid.Width(),
            entityIndexCapacity > pointCapacity ? entityIndexCapacity : pointCapacity),
        generations(entityIndexCapacity), records(entityIndexCapacity), pointCapacity(pointCapacity)
    {
        if (!entityIndexCapacity || !pointCapacity)
            throw std::invalid_argument("Impact target capacities must be positive");
        touched.reserve(entityIndexCapacity);
    }

    void Refresh()
    {
        for (const auto entityIndex : touched) generations[entityIndex] = ecs::Entity::InvalidGeneration;
        touched.clear();
        index.Clear();
        std::size_t indexedCount{};
        targets.ForEachChunk([&](auto chunk) {
            const auto lives = chunk.template Get<Life>();
            const auto health = chunk.template Get<Health>();
            const auto positions = chunk.template Get<Position>();
            const auto members = chunk.template Get<Membership>();
            const auto units = chunk.template Get<production::ProducedUnit>();
            const auto producers = chunk.template Get<production::Producer>();
            const auto structures = chunk.template Get<Structure>();
            const auto armor = chunk.template Get<Armor>();
            for (std::size_t row = 0; row != chunk.Count(); ++row)
            {
                (void)health;
                if (!lives[row].alive || (!members.empty() && engine::gameplay::containment::IsContained(members[row]))) continue;
                const auto position = positions[row].cell;
                if (position >= grid.Count()) throw std::invalid_argument("Impact target position is outside navigation grid");
                const auto entity = chunk.Entities()[row];
                if (entity.index >= generations.size()) throw std::length_error("Impact target entity-index capacity exhausted");
                if (generations[entity.index] != ecs::Entity::InvalidGeneration)
                    throw std::logic_error("Duplicate impact target entity index");
                if (indexedCount == pointCapacity) throw std::length_error("Impact target point capacity exhausted");
                const auto owner = !units.empty() ? units[row].account : !producers.empty() ? producers[row].account
                    : !structures.empty() ? structures[row].account : ecs::Entity{};
                index.Add({entity,owner,position % grid.Width(),position / grid.Width(),1});
                ++indexedCount;
                TargetRecord record;
                if (!armor.empty()) record = {true,armor[row].definition};
                else if (!structures.empty() && structures[row].armor) record = {true,structures[row].armor->definition};
                records[entity.index] = record;
                generations[entity.index] = entity.generation;
                touched.push_back(entity.index);
            }
        });
        index.Publish();
    }

    [[nodiscard]] std::optional<TargetRecord> Target(ecs::Entity entity) const noexcept
    {
        if (!entity.IsValid() || entity.index >= generations.size() || generations[entity.index] != entity.generation)
            return std::nullopt;
        return records[entity.index];
    }
    [[nodiscard]] const engine::gameplay::spatial::PointGrid &Index() const noexcept { return index; }
    template<typename Function>
    void ForEachTargetChunk(Function &&function)
    {
        targets.ForEachPreparedChunk(std::forward<Function>(function));
    }
    [[nodiscard]] std::size_t EntityCapacity() const noexcept { return generations.size(); }

private:
    const engine::gameplay::navigation::NavigationGrid &grid;
    Access targets;
    engine::gameplay::spatial::PointGrid index;
    std::vector<ecs::EntityGeneration> generations;
    std::vector<TargetRecord> records;
    std::vector<ecs::EntityIndex> touched;
    std::size_t pointCapacity;
};
}
