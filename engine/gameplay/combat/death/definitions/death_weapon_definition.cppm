module;
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>
export module engine.gameplay.combat.death.definitions.death_weapon_definition;
export import engine.gameplay.combat.definitions.weapon_definition;
export import engine.time.simulation_time;

export namespace engine::gameplay::combat::death
{
// Authored values retain typed durations and the existing typed weapon
// authoring schema.  The catalog performs the one startup conversion to
// integer runtime ticks.
struct DeathWeaponDefinition
{
    static constexpr std::uint32_t CurrentVersion=1;
    std::uint32_t version{CurrentVersion};
    std::uint32_t key{};
    engine::time::Duration destructionDelay{};
    std::optional<engine::gameplay::combat::WeaponConfig> initial{};
    std::optional<engine::gameplay::combat::WeaponConfig> final{};
};

struct DeathWeaponEntry
{
    std::uint32_t key{};
    std::uint64_t destructionDelayTicks{};
    std::optional<engine::gameplay::combat::WeaponDefinition> initial{};
    std::optional<engine::gameplay::combat::WeaponDefinition> final{};
};

class DeathWeaponCatalog
{
public:
    DeathWeaponCatalog(std::span<const DeathWeaponDefinition> definitions,
        engine::time::FixedStep step) : step_(step)
    {
        entries_.reserve(definitions.size());
        for (const auto &definition : definitions)
        {
            if (definition.version!=DeathWeaponDefinition::CurrentVersion)
                throw std::invalid_argument("Unsupported death weapon definition version");
            if (!definition.key) throw std::invalid_argument("Death weapon definition key must be nonzero");
            DeathWeaponEntry entry{definition.key,step.TicksFor(definition.destructionDelay),{}, {}};
            if (!definition.initial && !definition.final)
                throw std::invalid_argument("Death weapon definition requires an INITIAL or FINAL weapon");
            if (definition.initial) entry.initial=AuthorPhase(*definition.initial,step);
            if (definition.final) entry.final=AuthorPhase(*definition.final,step);
            entries_.push_back(std::move(entry));
        }
        std::sort(entries_.begin(),entries_.end(),[](const auto &left,const auto &right) {
            return left.key<right.key;
        });
        for (std::size_t index=1;index<entries_.size();++index)
            if (entries_[index-1].key==entries_[index].key)
                throw std::invalid_argument("Duplicate death weapon definition key");
    }

    [[nodiscard]] const DeathWeaponEntry *Find(std::uint32_t key) const noexcept
    {
        const auto found=std::lower_bound(entries_.begin(),entries_.end(),key,
            [](const auto &entry,const auto value){return entry.key<value;});
        return found==entries_.end()||found->key!=key?nullptr:&*found;
    }
    [[nodiscard]] engine::time::FixedStep Step() const noexcept { return step_; }

private:
    static engine::gameplay::combat::WeaponDefinition AuthorPhase(
        const engine::gameplay::combat::WeaponConfig &config, engine::time::FixedStep step)
    {
        if (!config.damage && !config.secondaryDamage)
            throw std::invalid_argument("Death weapon phase must cause damage");
        return engine::gameplay::combat::AuthorWeapon(config,step);
    }

    engine::time::FixedStep step_;
    std::vector<DeathWeaponEntry> entries_;
};
}
