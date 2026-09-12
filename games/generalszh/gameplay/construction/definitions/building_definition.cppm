module;
#include <cstdint>
#include <optional>
#include <string_view>
export module games.generalszh.gameplay.construction.definitions.building_definition;
export import games.generalszh.gameplay.construction.components.structure;
export import games.generalszh.gameplay.bounty.components.cash_bounty_cost_binding;
export import games.generalszh.gameplay.containment.definitions.garrison_definition;
export import engine.gameplay.rts.visibility.definitions.visibility_definition;
export namespace generalszh::construction
{
struct BuildingDefinition
{
    std::uint32_t key{},cost{};
    engine::time::Duration duration{};
    std::uint64_t health{1000};
    std::uint32_t queueLimit{};
    std::int32_t energy{};
    bool supplyDropoff{};
    std::uint32_t supplyBoxValue{75};
    std::optional<std::uint32_t> repairDefinition{};
    bool capturable{}; // Explicit resolved game eligibility, not an inferred upgrade.
    std::optional<engine::gameplay::combat::damage::ArmorBinding> armor{};
    // The immutable garrison policy stays in BuildingCatalog and is resolved
    // from Structure.definition at runtime; Structure does not copy it.
    std::optional<generalszh::containment::GarrisonDefinition> garrison{};
    std::optional<engine::gameplay::rts::visibility::VisibilityDefinition> visibility{};
    // Optional victim enrollment retained on the construction scaffold; the
    // award system's completion gate makes it eligible only after completion.
    std::optional<generalszh::bounty::CashBountyCostBinding> cashBountyCost{};
};
}
