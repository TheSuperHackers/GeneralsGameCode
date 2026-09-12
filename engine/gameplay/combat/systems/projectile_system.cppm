module;
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
export module engine.gameplay.combat.systems.projectile_system;
export import engine.gameplay.combat.systems.weapon_system;
export import engine.gameplay.combat.damage.definitions.armor_catalog;
export import engine.gameplay.combat.damage.algorithms.resolve_damage;
export import engine.gameplay.combat.damage.inputs.accepted_hit_batch;
export import engine.gameplay.spatial.grid.point_grid;
export namespace engine::gameplay::combat
{
// The default provider is deliberately direct-only. It snapshots the same
// complete health eligibility that the former direct delivery path used, but
// still resolves through typed spans rather than World::Get in the reduction.
struct NoSpatialProjectileProjection
{
    static constexpr bool Spatial = false;
    using Access = ecs::Query<ecs::Read<LifeState>, ecs::Read<Health>, ecs::Read<DamageResult>,
        ecs::Write<PendingDamage>, ecs::Optional<damage::ArmorBinding>>;
    struct TargetRecord { bool hasArmor{}; damage::ArmorDefinitionRef armor{}; };

    NoSpatialProjectileProjection(ecs::World &world, std::size_t entityIndexCapacity) : targets(world),
        generations(entityIndexCapacity), records(entityIndexCapacity)
    {
        if (!entityIndexCapacity) throw std::invalid_argument("Projectile target index capacity must be positive");
        touched.reserve(entityIndexCapacity);
    }
    void Refresh()
    {
        for (const auto index : touched) generations[index] = ecs::Entity::InvalidGeneration;
        touched.clear();
        targets.ForEachChunk([&](auto chunk) {
            const auto lives = chunk.template Get<LifeState>();
            const auto armor = chunk.template Get<damage::ArmorBinding>();
            for (std::size_t row = 0; row != chunk.Count(); ++row)
            {
                if (!lives[row].alive) continue;
                const auto entity = chunk.Entities()[row];
                if (entity.index >= generations.size()) throw std::length_error("Projectile target entity-index capacity exhausted");
                if (generations[entity.index] != ecs::Entity::InvalidGeneration)
                    throw std::logic_error("Duplicate projectile target entity index");
                generations[entity.index] = entity.generation;
                records[entity.index] = { !armor.empty(), armor.empty() ? damage::ArmorDefinitionRef{} : armor[row].definition };
                touched.push_back(entity.index);
            }
        });
    }
    [[nodiscard]] std::optional<TargetRecord> Target(ecs::Entity entity) const noexcept
    {
        if (!entity.IsValid() || entity.index >= generations.size() || generations[entity.index] != entity.generation)
            return std::nullopt;
        return records[entity.index];
    }
    template<typename Function>
    void ForEachTargetChunk(Function &&function)
    {
        targets.ForEachPreparedChunk(std::forward<Function>(function));
    }
    [[nodiscard]] std::size_t EntityCapacity() const noexcept { return generations.size(); }

private:
    Access targets;
    std::vector<ecs::EntityGeneration> generations;
    std::vector<TargetRecord> records;
    std::vector<ecs::EntityIndex> touched;
};

template<typename Projection = NoSpatialProjectileProjection>
struct ProjectileSystemT
{
    template<typename P = Projection>
    ProjectileSystemT(ecs::World &world, const damage::ArmorCatalog &armor, damage::AcceptedHitBatch &hits,
        std::size_t projectileCapacity = 65536, std::size_t radiusResultCapacity = 65536,
        std::size_t entityIndexCapacity = 65536)
        requires std::is_same_v<P, NoSpatialProjectileProjection> :
        ownedProjection(std::in_place, world, entityIndexCapacity), world(world), armor(armor), hits(hits),
        projection(&*ownedProjection), projectileCapacity(projectileCapacity), radiusResultCapacity(radiusResultCapacity)
    {
        Initialize();
    }
    ProjectileSystemT(ecs::World &world, const damage::ArmorCatalog &armor, Projection &projection,
        damage::AcceptedHitBatch &hits, std::size_t projectileCapacity = 65536,
        std::size_t radiusResultCapacity = 65536) :
        ownedProjection(), world(world), armor(armor), hits(hits), projection(&projection),
        projectileCapacity(projectileCapacity), radiusResultCapacity(radiusResultCapacity)
    {
        Initialize();
    }

    using Query = ecs::Query<ecs::Read<ProjectileImpact>, ecs::Write<ImpactDue>>;
    // Projection::Access is the complete typed source/target join. This keeps
    // position, ownership and containment access visible to the scheduler for
    // the concrete game projection without importing game types here.
    using AuxiliaryAccess = typename Projection::Access;

    void BeforeChunks(Query &, ecs::SystemContext &)
    {
        projection->Refresh();
        hits.BeginTick();
    }
    void Execute(Query::Chunk chunk, ecs::SystemContext &context) const noexcept
    {
        const auto impacts = chunk.template Get<ProjectileImpact>(); auto due = chunk.template Get<ImpactDue>();
        for (std::size_t row = 0; row != chunk.Count(); ++row) due[row].value = context.Tick() >= impacts[row].arrivalTick;
    }
    void AfterChunks(Query &query, ecs::SystemContext &context)
    {
        deliveries.clear(); accepted.clear();
        for (const auto index : touchedAggregates) aggregates[index] = {};
        touchedAggregates.clear();
        query.ForEachPreparedChunk([&](auto chunk) {
            const auto impacts=chunk.template Get<ProjectileImpact>(); const auto due=chunk.template Get<ImpactDue>();
            for (std::size_t row=0; row!=chunk.Count(); ++row)
                if (due[row].value)
                {
                    if (deliveries.size()==projectileCapacity) throw std::length_error("Projectile delivery capacity exhausted");
                    deliveries.push_back({chunk.Entities()[row],impacts[row]});
                }
        });
        std::sort(deliveries.begin(),deliveries.end(),[](const auto &left,const auto &right) {
            return std::tie(left.projectile.index,left.projectile.generation)<std::tie(right.projectile.index,right.projectile.generation);
        });
        for (const auto &delivery : deliveries)
        {
            ValidateImpact(delivery.impact);
            const auto radius = (std::max)(delivery.impact.primaryRadiusCells, delivery.impact.secondaryRadiusCells);
            if (radius == 0U)
            {
                // A ground shot has no primary victim. It still consumes the
                // projectile/ammunition episode, but a zero-radius impact has
                // no area in which to discover a victim.
                if (!delivery.impact.positionalTarget)
                    Consider(delivery.projectile, delivery.impact, delivery.impact.target, nullptr, 0U, 0U);
                continue;
            }
            if constexpr (!Projection::Spatial)
            {
                throw std::invalid_argument("Spatial projectile impact requires an impact projection");
            }
            else
            {
                spatial::SpatialPoint center{};
                if (delivery.impact.positionalTarget)
                {
                    center.x = delivery.impact.detonationX;
                    center.y = delivery.impact.detonationY;
                }
                else if (!projection->Index().TryGet(delivery.impact.target, center))
                {
                    if (!delivery.impact.detonationPositionValid)
                        throw std::invalid_argument("Spatial projectile impact has no valid saved center");
                    center.entity = delivery.impact.target;
                    center.x = delivery.impact.detonationX;
                    center.y = delivery.impact.detonationY;
                }
                const auto &index = projection->Index();
                if (center.x >= index.Width() || center.y >= index.Height())
                    throw std::invalid_argument("Spatial projectile impact center is outside the grid");
                const auto count = index.EnumerateWithinRadius(center.x, center.y, radius,
                    std::span<spatial::SpatialPoint>{candidates.data(), candidates.size()});
                for (std::size_t row = 0; row != count; ++row)
                    Consider(delivery.projectile, delivery.impact, candidates[row].entity, &candidates[row], center.x, center.y);
            }
        }

        // AcceptedHitBatch keeps delivery order for its existing consumers.
        // This separate bounded index makes each target's canonical hit range
        // contiguous without a per-hit world lookup or a quadratic search.
        hitOrder_.resize(accepted.size());
        for (std::size_t index = 0; index != hitOrder_.size(); ++index) hitOrder_[index] = index;
        std::sort(hitOrder_.begin(), hitOrder_.end(), [this](const auto left, const auto right) {
            const auto &a = accepted[left]; const auto &b = accepted[right];
            return std::tie(a.target.index, a.target.generation, left)
                < std::tie(b.target.index, b.target.generation, right);
        });
        for (std::size_t begin = 0; begin != hitOrder_.size();)
        {
            const auto target = accepted[hitOrder_[begin]].target;
            std::size_t end = begin + 1;
            while (end != hitOrder_.size() && accepted[hitOrder_[end]].target == target) ++end;
            if (target.index >= aggregates.size())
                throw std::length_error("Projectile target entity-index capacity exhausted");
            auto &aggregate = aggregates[target.index];
            if (aggregate.generation != target.generation)
                throw std::logic_error("Projectile target generation changed during range reduction");
            aggregate.first = begin;
            aggregate.count = end - begin;
            begin = end;
        }
        if (accepted.size() > hits.Capacity()) throw std::length_error("Accepted hit capacity exhausted");
        for (const auto &hit : accepted)
            hits.Append({hit.target,hit.projectile,hit.impact.launchTick,context.Tick(),
                {hit.impact.damageType,hit.impact.damageChannel,hit.rawDamage,
                    {hit.impact.source,hit.impact.sourceAccount,hit.impact.sourceDefinition}},
                hit.resolvedQuantity,hit.saturated});
        projection->ForEachTargetChunk([&](auto chunk) {
            const auto health = chunk.template Get<Health>();
            const auto lives = chunk.template Get<LifeState>();
            auto pending = chunk.template Get<PendingDamage>();
            const auto entities = chunk.Entities();
            for (std::size_t row = 0; row != chunk.Count(); ++row)
            {
                const auto entity = entities[row];
                if (entity.index >= aggregates.size()) continue;
                const auto &aggregate = aggregates[entity.index];
                if (aggregate.generation != entity.generation) continue;
                if (pending[row].lethalSource.valid &&
                    (pending[row].lethalSource.target != entity || pending[row].lethalSource.tick != context.Tick()))
                    pending[row].lethalSource = {};

                auto cumulative = pending[row].quantity;
                const auto healthAtCrossing = health[row].current;
                for (std::size_t offset = 0; offset != aggregate.count; ++offset)
                {
                    const auto &hit = accepted[hitOrder_[aggregate.first + offset]];
                    const auto before = cumulative;
                    cumulative = SaturatingAdd(cumulative, hit.resolvedQuantity);
                    if (lives[row].alive && healthAtCrossing != 0 && !pending[row].lethalSource.valid
                        && before < healthAtCrossing && cumulative >= healthAtCrossing)
                    {
                        pending[row].lethalSource = {
                            entity, hit.impact.source, hit.impact.sourceAccount, context.Tick(),
                            healthAtCrossing, before, cumulative, true};
                    }
                }
                pending[row].quantity = cumulative;
            }
        });
        for (const auto &delivery : deliveries) context.Commands().Destroy(delivery.projectile);
        hits.Publish();
    }

private:
    struct Delivery { ecs::Entity projectile; ProjectileImpact impact; };
    struct AcceptedDelivery {
        ecs::Entity projectile, target;
        ProjectileImpact impact;
        std::uint64_t rawDamage{}, resolvedQuantity{};
        bool saturated{};
    };
    struct Aggregate
    {
        ecs::EntityGeneration generation{};
        std::size_t first{}, count{};
    };

    void Initialize()
    {
        if (!projectileCapacity || !radiusResultCapacity)
            throw std::invalid_argument("Projectile capacities must be positive");
        if (projection == nullptr || projection->EntityCapacity() == 0)
            throw std::invalid_argument("Projectile projection requires positive entity-index capacity");
        deliveries.reserve(projectileCapacity); candidates.resize(radiusResultCapacity);
        accepted.reserve(hits.Capacity()); hitOrder_.reserve(hits.Capacity());
        aggregates.resize(projection->EntityCapacity());
        touchedAggregates.reserve(aggregates.size());
    }
    static std::uint64_t SaturatingAdd(const std::uint64_t left, const std::uint64_t right) noexcept
    {
        const auto room = (std::numeric_limits<std::uint64_t>::max)() - left;
        return left + (right < room ? right : room);
    }
    static void ValidateImpact(const ProjectileImpact &impact)
    {
        if (!impact.radiusDamageAffects.IsValid()) throw std::invalid_argument("Invalid projectile affects mask");
        if (impact.positionalTarget && (impact.target.IsValid() || !impact.detonationPositionValid))
            throw std::invalid_argument("Positional projectile impact has an invalid target identity");
        if (impact.detonationPositionValid &&
            (impact.detonationX == (std::numeric_limits<std::uint32_t>::max)() ||
             impact.detonationY == (std::numeric_limits<std::uint32_t>::max)()))
            throw std::invalid_argument("Invalid saved projectile impact center");
    }
    static bool Affects(const ProjectileImpact &impact, const spatial::SpatialPoint &candidate) noexcept
    {
        // The direct victim exception is evaluated by Consider before this
        // function. Self is a separate authored bit; a deleted source never
        // becomes a different relationship policy through a missing lookup.
        if (candidate.entity == impact.source)
            return impact.radiusDamageAffects.Contains(RadiusDamageAffect::Self);
        const auto relation = !impact.sourceAccount.IsValid() || !candidate.group.IsValid()
            ? RadiusDamageAffect::Neutrals
            : impact.sourceAccount == candidate.group ? RadiusDamageAffect::Allies : RadiusDamageAffect::Enemies;
        return impact.radiusDamageAffects.Contains(relation);
    }
    static bool WithinPrimary(const spatial::SpatialPoint &point, const std::uint32_t centerX,
        const std::uint32_t centerY, const std::uint32_t radius) noexcept
    {
        const std::uint64_t dx = point.x > centerX ? point.x-centerX : centerX-point.x;
        const std::uint64_t dy = point.y > centerY ? point.y-centerY : centerY-point.y;
        const auto radiusSquared = std::uint64_t{radius} * radius;
        const auto dxSquared = dx * dx;
        return dxSquared <= radiusSquared && dy * dy <= radiusSquared-dxSquared;
    }
    void Consider(const ecs::Entity projectile, const ProjectileImpact &impact, const ecs::Entity target,
        const spatial::SpatialPoint *point, const std::uint32_t centerX, const std::uint32_t centerY)
    {
        const auto record = projection->Target(target);
        if (!record) return;
        const bool directVictim = target == impact.target;
        if (!directVictim)
        {
            assert(point != nullptr);
            if (!Affects(impact,*point)) return;
        }
        const std::uint64_t rawDamage = point == nullptr ||
            WithinPrimary(*point,centerX,centerY,impact.primaryRadiusCells) ? impact.damage : impact.secondaryDamage;
        const auto policy=armor.Policy(impact.damageType);
        damage::ArmorMultiplier coefficient;
        if (record->hasArmor) coefficient=armor.Get(record->armor).coefficients[impact.damageType.value];
        const auto resolved=damage::ResolveDamage(rawDamage,coefficient,policy.armor);
        if (accepted.size()==hits.Capacity()) throw std::length_error("Accepted hit capacity exhausted");
        accepted.push_back({projectile,target,impact,rawDamage,resolved.quantity,resolved.saturated});
        if (target.index >= aggregates.size()) throw std::length_error("Projectile target entity-index capacity exhausted");
        auto &aggregate=aggregates[target.index];
        if (aggregate.generation == ecs::Entity::InvalidGeneration)
        {
            aggregate.generation=target.generation; touchedAggregates.push_back(target.index);
        }
        else if (aggregate.generation != target.generation)
            throw std::logic_error("Projectile target generation changed during reduction");
    }

    std::optional<NoSpatialProjectileProjection> ownedProjection;
    ecs::World &world;
    const damage::ArmorCatalog &armor;
    damage::AcceptedHitBatch &hits;
    Projection *projection;
    std::size_t projectileCapacity, radiusResultCapacity;
    std::vector<Delivery> deliveries;
    std::vector<spatial::SpatialPoint> candidates;
    std::vector<AcceptedDelivery> accepted;
    std::vector<std::size_t> hitOrder_;
    std::vector<Aggregate> aggregates;
    std::vector<ecs::EntityIndex> touchedAggregates;
};

using ProjectileSystem = ProjectileSystemT<NoSpatialProjectileProjection>;
}
export namespace ecs
{
template<typename Projection>
struct SystemTraits<engine::gameplay::combat::ProjectileSystemT<Projection>>
{
    static constexpr std::string_view StableName = "engine.gameplay.combat.projectile_arrival";
    static constexpr SystemPhase Phase = SystemPhase::PreSimulation;
    using Before = SystemTypeList<>; using After = SystemTypeList<>;
};
}
