module;

#include <algorithm>
#include <cstddef>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

export module games.generalszh.adapters.content.science.science_prerequisite_binding;
export import games.generalszh.adapters.content.prerequisites.prerequisite_definition;
export import games.generalszh.adapters.content.science.science_definition;
export import engine.gameplay.rts.unlocks.definitions.unlock_catalog;

export namespace generalszh::content
{
struct BoundProductionPrerequisites final
{
    engine::gameplay::rts::production::PrerequisiteDefinition object{};
    std::vector<engine::gameplay::rts::unlocks::UnlockId> scienceIds;
    std::optional<engine::gameplay::rts::unlocks::UnlockSchemaHash> scienceSchemaHash;
};

inline BoundProductionPrerequisites BindProductionPrerequisites(
    const DecodedPrerequisites &source,
    std::span<const ContentDefinitionIdentity> identities,
    const engine::gameplay::rts::unlocks::UnlockCatalog &scienceCatalog)
{
    if (!scienceCatalog.IsFinalized())
        throw std::logic_error("Science prerequisite binding requires a finalized unlock catalog");

    // Keep the existing object binder as the single source of object
    // prerequisite semantics, while making the science-aware route explicit.
    DecodedPrerequisites objectOnly = source;
    objectOnly.scienceNames.clear();

    BoundProductionPrerequisites result;
    result.object = BindObjectPrerequisites(objectOnly, identities);
    if (source.scienceNames.empty())
        return result;

    result.scienceSchemaHash = scienceCatalog.SchemaHash();
    result.scienceIds.reserve(source.scienceNames.size());
    for (const std::string &name : source.scienceNames)
    {
        if (!engine::gameplay::rts::unlocks::IsValidUnlockName(name))
            throw std::invalid_argument("Invalid canonical Science prerequisite name: " + name);
        const auto id = scienceCatalog.Find(
            engine::gameplay::rts::unlocks::MakeUnlockKey(name));
        if (id == engine::gameplay::rts::unlocks::InvalidUnlockId)
            throw std::invalid_argument("Science prerequisite names an unknown definition: " + name);
        result.scienceIds.push_back(id);
    }

    std::sort(result.scienceIds.begin(), result.scienceIds.end(),
        [](const auto left, const auto right) { return left.value < right.value; });
    result.scienceIds.erase(std::unique(result.scienceIds.begin(), result.scienceIds.end()),
        result.scienceIds.end());
    return result;
}
} // namespace generalszh::content
