module;
#include <algorithm>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <vector>
export module games.generalszh.gameplay.selling.definitions.sell_definition;
export import games.generalszh.gameplay.construction.definitions.building_catalog;

namespace generalszh::selling::detail
{
class SellTickOverflow final : public std::overflow_error
{
public:
    SellTickOverflow() : std::overflow_error("Sale deadline exceeds simulation tick horizon") {}
};
[[noreturn]] void ThrowSellTickOverflow() { throw SellTickOverflow{}; }
}
export namespace generalszh::selling
{
struct SellDefinition
{
    std::uint32_t building{};
    engine::time::Duration duration{};
    std::uint32_t refundValue{}; // Reference: zero means use the percentage.
    std::uint32_t percentageNumerator{1}, percentageDenominator{2};
};
struct CompiledSellDefinition { std::uint32_t building{}, refund{}; std::uint64_t delayTicks{}; };
// Startup content boundary, not a runtime modifier service. BuildAssistant.cpp
// sellObject/update resets progress to 99.9, waits 1.5s, then decrements through
// -50 using a 3s divisor. The resulting deadline depends on frame-start/update
// semantics. Callers author a typed duration explicitly; no claim that a literal
// 3s or a universal 179-tick default reproduces every reference dispatch phase.
// Refunds use startup BuildingCatalog cost. Runtime player/handicap changes,
// RefundValue parsing and visual scaffolding remain separate adapter work.
class SellCatalog
{
public:
    SellCatalog(const construction::BuildingCatalog &buildings, std::span<const SellDefinition> source)
        : step(buildings.Step())
    {
        definitions.reserve(source.size());
        for (const auto &definition:source)
        {
            const auto *building=buildings.Find(definition.building);
            if (!building || !definition.percentageDenominator || definition.refundValue>UINT16_MAX)
                throw std::invalid_argument("Invalid sale building, refund override or percentage");
            const auto refund=definition.refundValue ? definition.refundValue :
                static_cast<std::uint64_t>(building->definition.cost)*definition.percentageNumerator/definition.percentageDenominator;
            if (refund>UINT32_MAX) throw std::out_of_range("Sale refund exceeds Zero Hour money range");
            definitions.push_back({definition.building,static_cast<std::uint32_t>(refund),step.TicksFor(definition.duration)});
        }
        std::sort(definitions.begin(),definitions.end(),[](const auto &a,const auto &b){return a.building<b.building;});
        for (std::size_t i=1;i<definitions.size();++i)
            if (definitions[i-1].building==definitions[i].building) throw std::invalid_argument("Duplicate sale definition");
    }
    const CompiledSellDefinition *Find(std::uint32_t building) const noexcept
    {
        const auto it=std::lower_bound(definitions.begin(),definitions.end(),building,
            [](const auto &definition,auto key){return definition.building<key;});
        return it!=definitions.end() && it->building==building ? &*it : nullptr;
    }
    engine::time::FixedStep Step() const noexcept { return step; }
private:
    const engine::time::FixedStep step;
    std::vector<CompiledSellDefinition> definitions;
};
inline std::uint64_t SaleDeadline(std::uint64_t tick,std::uint64_t delay)
{
    if (delay>(std::numeric_limits<std::uint64_t>::max)()-tick) detail::ThrowSellTickOverflow();
    return tick+delay;
}
}
