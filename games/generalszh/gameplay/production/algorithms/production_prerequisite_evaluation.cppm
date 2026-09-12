module;

#include <cstddef>
#include <cstdint>
#include <span>

export module games.generalszh.gameplay.production.algorithms.production_prerequisite_evaluation;
export import games.generalszh.gameplay.production.admission.build_catalog;
export import engine.gameplay.rts.production.algorithms.prerequisite_evaluation;
export import engine.gameplay.rts.unlocks.components.unlock_state;

export namespace generalszh::production
{
inline bool SatisfiesSciencePrerequisites(
    std::span<const engine::gameplay::rts::unlocks::UnlockId> prerequisites,
    const engine::gameplay::rts::unlocks::UnlockState &state) noexcept
{
    for (const auto id : prerequisites)
        if (!state.IsOwned(id))
            return false;
    return true;
}

inline bool SatisfiesObjectPrerequisites(
    const BuildCatalog &catalog,
    const std::size_t definitionIndex,
    std::span<const std::uint32_t> objectKeys) noexcept
{
    if (definitionIndex >= catalog.Definitions().size())
        return false;
    const auto &definition = catalog.Definitions()[definitionIndex];
    return !definition.prerequisites ||
        engine::gameplay::rts::production::SatisfiesPrerequisites(
            *definition.prerequisites, objectKeys);
}

inline bool SatisfiesProductionPrerequisites(
    const BuildCatalog &catalog,
    const std::size_t definitionIndex,
    std::span<const std::uint32_t> objectKeys,
    const engine::gameplay::rts::unlocks::UnlockState &state) noexcept
{
    if (definitionIndex >= catalog.Definitions().size())
        return false;
    return SatisfiesObjectPrerequisites(catalog, definitionIndex, objectKeys) &&
        SatisfiesSciencePrerequisites(catalog.SciencePrerequisites(definitionIndex), state);
}
} // namespace generalszh::production
