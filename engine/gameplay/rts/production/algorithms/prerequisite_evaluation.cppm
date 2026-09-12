module;
#include <algorithm>
#include <cstdint>
#include <span>
export module engine.gameplay.rts.production.algorithms.prerequisite_evaluation;
export import engine.gameplay.rts.production.definitions.prerequisite_definition;
export namespace engine::gameplay::rts::production
{
// ownedKeys is sorted and unique. Object prerequisites only require presence;
// multiplicity is deliberately not part of the legacy isSatisfied contract.
inline bool SatisfiesPrerequisites(const PrerequisiteDefinition &definition,
    std::span<const std::uint32_t> ownedKeys) noexcept
{
    const auto alternatives = definition.Alternatives();
    for (const auto group : definition.Groups())
    {
        bool satisfied = false;
        for (std::uint32_t offset = 0; offset != group.count; ++offset)
            satisfied |= std::binary_search(ownedKeys.begin(), ownedKeys.end(), alternatives[group.first + offset]);
        if (!satisfied) return false;
    }
    return true;
}
}
