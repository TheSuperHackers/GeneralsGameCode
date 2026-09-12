module;
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <tuple>
#include <vector>
export module games.generalszh.gameplay.production.algorithms.prerequisite_presence;
export import engine.ecs.query.query;
export import engine.gameplay.rts.production.algorithms.prerequisite_evaluation;
export import engine.gameplay.combat.components.health;
export import games.generalszh.gameplay.construction.definitions.building_definition;
export import games.generalszh.gameplay.production.components.production_state;

namespace generalszh::production_detail
{
struct PresenceRow
{
    ecs::Entity owner{};
    std::uint32_t key{};
};

struct OwnerRange
{
    ecs::Entity owner{};
    std::size_t first{};
    std::size_t count{};
};

inline bool EntityKeyLess(ecs::Entity left, ecs::Entity right) noexcept
{
    return std::tie(left.index, left.generation) < std::tie(right.index, right.generation);
}
}

export namespace generalszh::production
{
// A per-admission snapshot of the authoritative OBJECTs owned by each account.
// The vectors are reserved once from the configured observation capacity and
// reused for every joined admission batch. The capacity covers observed live
// unit/structure rows, not merely requests; exceeding it is an explicit
// deterministic configuration failure. Entity generations stay in the owner
// ranges so a stale account handle can never alias a later entity at that index.
class PrerequisitePresence final
{
public:
    using UnitObjects = ecs::Query<ecs::Read<ProducedUnit>, ecs::Read<engine::gameplay::combat::LifeState>>;
    using Structures = ecs::Query<ecs::Read<construction::Structure>, ecs::Read<engine::gameplay::combat::LifeState>>;

    PrerequisitePresence(ecs::World &world, std::size_t capacity)
        : world(world), units(world), structures(world), capacity(capacity)
    {
        rows.reserve(capacity);
        keys.reserve(capacity);
        owners.reserve(capacity);
    }

    void Rebuild()
    {
        rows.clear();
        keys.clear();
        owners.clear();

        units.ForEachChunk([this](auto chunk)
        {
            const auto values = chunk.template Get<ProducedUnit>();
            const auto life = chunk.template Get<engine::gameplay::combat::LifeState>();
            for (std::size_t row = 0; row != chunk.Count(); ++row)
            {
                if (!life[row].alive || !world.IsAlive(values[row].account))
                    continue;
                Append({values[row].account, values[row].definition});
            }
        });
        structures.ForEachChunk([this](auto chunk)
        {
            const auto values = chunk.template Get<construction::Structure>();
            const auto life = chunk.template Get<engine::gameplay::combat::LifeState>();
            for (std::size_t row = 0; row != chunk.Count(); ++row)
            {
                // Modern admission intentionally requires a completed, live
                // structure. This is a documented deterministic deviation from
                // the reference count call's ignoreDead=false default.
                if (!values[row].complete || !life[row].alive || !world.IsAlive(values[row].account))
                    continue;
                Append({values[row].account, values[row].definition});
            }
        });

        std::sort(rows.begin(), rows.end(), [](const auto &left, const auto &right)
        {
            if (left.owner != right.owner)
                return production_detail::EntityKeyLess(left.owner, right.owner);
            return left.key < right.key;
        });
        rows.erase(std::unique(rows.begin(), rows.end(), [](const auto &left, const auto &right)
        {
            return left.owner == right.owner && left.key == right.key;
        }), rows.end());

        for (std::size_t index = 0; index != rows.size();)
        {
            const std::size_t first = index;
            const ecs::Entity owner = rows[index].owner;
            while (index != rows.size() && rows[index].owner == owner)
                ++index;
            owners.push_back({owner, first, index - first});
            for (std::size_t row = first; row != index; ++row)
                keys.push_back(rows[row].key);
        }
    }

    std::span<const std::uint32_t> Keys(ecs::Entity owner) const noexcept
    {
        const auto found = std::lower_bound(owners.begin(), owners.end(), owner,
            [](const auto &range, ecs::Entity value)
            {
                return production_detail::EntityKeyLess(range.owner, value);
            });
        if (found == owners.end() || found->owner != owner)
            return {};
        return {keys.data() + found->first, found->count};
    }

private:
    void Append(production_detail::PresenceRow value)
    {
        if (rows.size() == capacity)
            throw std::length_error("Production prerequisite presence capacity exhausted");
        rows.push_back(value);
    }

    ecs::World &world;
    UnitObjects units;
    Structures structures;
    const std::size_t capacity;
    std::vector<production_detail::PresenceRow> rows;
    std::vector<std::uint32_t> keys;
    std::vector<production_detail::OwnerRange> owners;
};
}
