module;
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>
export module games.generalszh.gameplay.demolition.definitions.demolition_trap_definition;
export import games.generalszh.gameplay.demolition.components.demolition_trap;
export import engine.gameplay.combat.definitions.weapon_definition;
export import engine.gameplay.combat.death.definitions.death_weapon_definition;
export import engine.time.simulation_time;

export namespace generalszh::demolition
{
struct DemolitionTrapDefinition
{
    static constexpr std::uint32_t CurrentVersion=1;
    std::uint32_t version{CurrentVersion};
    std::uint32_t key{};
    DemolitionDetonationMode mode{DemolitionDetonationMode::Proximity};
    bool friendlyDetonation{};
    bool detonateWhenKilled{};
    DemolitionIgnorePolicy ignoreTargetTypes{};
    std::uint32_t radiusCells{1};
    // DemoTrapUpdate's m_scanFrames default is zero.  The adapter preserves a
    // supplied millisecond duration; FixedStep converts it to a deadline.
    engine::time::Duration scanInterval{};
    engine::gameplay::combat::death::DeathWeaponDefinition deathWeapon{};
    std::optional<engine::gameplay::combat::WeaponConfig> detonationWeapon{};
};

struct DemolitionTrapEntry
{
    std::uint32_t key{};
    DemolitionDetonationMode mode{DemolitionDetonationMode::Proximity};
    bool friendlyDetonation{};
    bool detonateWhenKilled{};
    DemolitionIgnorePolicy ignoreTargetTypes{};
    std::uint32_t radiusCells{};
    std::uint64_t scanIntervalTicks{};
    std::optional<engine::gameplay::combat::WeaponDefinition> detonationWeapon{};
};

class DemolitionTrapCatalog
{
public:
    DemolitionTrapCatalog(std::span<const DemolitionTrapDefinition> definitions,
        engine::time::FixedStep step) : step_(step)
    {
        entries_.reserve(definitions.size());
        deathDefinitions_.reserve(definitions.size());
        for (const auto &definition : definitions)
        {
            if (definition.version!=DemolitionTrapDefinition::CurrentVersion)
                throw std::invalid_argument("Unsupported demolition trap definition version");
            if (!definition.key || definition.deathWeapon.key!=definition.key)
                throw std::invalid_argument("Demolition trap and death-weapon keys must match and be nonzero");
            if (!definition.radiusCells)
                throw std::invalid_argument("Demolition trap radius must be positive");
            DemolitionTrapEntry entry{definition.key,definition.mode,definition.friendlyDetonation,
                definition.detonateWhenKilled,definition.ignoreTargetTypes,definition.radiusCells,
                step.TicksFor(definition.scanInterval),{}};
            if (definition.detonationWeapon)
                entry.detonationWeapon=engine::gameplay::combat::AuthorWeapon(*definition.detonationWeapon,step);
            entries_.push_back(std::move(entry));
            deathDefinitions_.push_back(definition.deathWeapon);
        }
        std::sort(entries_.begin(),entries_.end(),[](const auto &left,const auto &right) { return left.key<right.key; });
        for (std::size_t index=1;index<entries_.size();++index)
            if (entries_[index-1].key==entries_[index].key)
                throw std::invalid_argument("Duplicate demolition trap definition key");
        std::sort(deathDefinitions_.begin(),deathDefinitions_.end(),[](const auto &left,const auto &right) { return left.key<right.key; });
    }

    [[nodiscard]] const DemolitionTrapEntry *Find(std::uint32_t key) const noexcept
    {
        const auto found=std::lower_bound(entries_.begin(),entries_.end(),key,
            [](const auto &entry,const auto value){return entry.key<value;});
        return found==entries_.end()||found->key!=key?nullptr:&*found;
    }
    [[nodiscard]] std::span<const engine::gameplay::combat::death::DeathWeaponDefinition> DeathWeapons() const noexcept
    { return deathDefinitions_; }
    [[nodiscard]] engine::time::FixedStep Step() const noexcept { return step_; }

private:
    engine::time::FixedStep step_;
    std::vector<DemolitionTrapEntry> entries_;
    std::vector<engine::gameplay::combat::death::DeathWeaponDefinition> deathDefinitions_;
};
}
