module;
#include <algorithm>
#include <span>
#include <stdexcept>
#include <vector>
export module games.generalszh.gameplay.construction.definitions.building_catalog;
export import games.generalszh.gameplay.construction.definitions.building_definition;
export import engine.gameplay.combat.damage.definitions.armor_catalog;
export namespace generalszh::construction
{
// Root-owned immutable content. No requests, mutable world state or system ownership.
class BuildingCatalog
{
public:
    struct Entry { BuildingDefinition definition; engine::gameplay::rts::production::BuildWork work; };
    BuildingCatalog(std::span<const BuildingDefinition> definitions,engine::time::FixedStep step,
        const engine::gameplay::rts::repair::RepairDefinitions *repair=nullptr,
        const engine::gameplay::combat::damage::ArmorCatalog *armor=nullptr):step(step)
    {
        entries.reserve(definitions.size());
        for(const auto &definition:definitions)
        {
            if(!definition.health) throw std::invalid_argument("Construction health must be positive");
            if(definition.garrison && !generalszh::containment::IsValid(*definition.garrison))
                throw std::invalid_argument("Garrison capacity must be positive");
            if(definition.armor)
            {
                if(!armor) throw std::invalid_argument("Building armor requires an explicit catalog");
                (void)armor->Get(definition.armor->definition);
            }
            if(definition.repairDefinition)
            {
                if(!repair || repair->Step()!=step) throw std::invalid_argument("Building repair requires a matching compiled catalog");
                (void)repair->Get(*definition.repairDefinition);
            }
            entries.push_back({definition,engine::gameplay::rts::production::AuthorBuild(definition.duration,step)});
        }
        std::sort(entries.begin(),entries.end(),[](const auto &a,const auto &b){return a.definition.key<b.definition.key;});
        for(std::size_t i=1;i<entries.size();++i)
            if(entries[i-1].definition.key==entries[i].definition.key)
                throw std::invalid_argument("Duplicate construction definition");
    }
    const Entry *Find(std::uint32_t key) const noexcept
    {
        const auto it=std::lower_bound(entries.begin(),entries.end(),key,
            [](const auto &entry,auto value){return entry.definition.key<value;});
        return it!=entries.end() && it->definition.key==key ? &*it : nullptr;
    }
    engine::time::FixedStep Step() const noexcept { return step; }
private:
    engine::time::FixedStep step;
    std::vector<Entry> entries;
};
}
