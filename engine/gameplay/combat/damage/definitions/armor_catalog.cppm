module;
#include <cassert>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>
export module engine.gameplay.combat.damage.definitions.armor_catalog;
export import engine.gameplay.combat.damage.components.damage_packet;
export namespace engine::gameplay::combat::damage {
struct ArmorDefinition {std::vector<ArmorMultiplier> coefficients;};
struct CompiledArmorDefinition {std::vector<ArmorMultiplier> coefficients;};
class ArmorCatalog {
public:
    ArmorCatalog(std::span<const DamageTypePolicy> policies,std::span<const ArmorDefinition> definitions)
        :policies_(policies.begin(),policies.end()) {
        if(policies.size()>UINT32_MAX||definitions.size()>UINT32_MAX) throw std::length_error("Armor catalog ID capacity");
        for(const auto policy:policies)
            if(policy.armor!=ArmorApplication::Scale&&policy.armor!=ArmorApplication::Bypass&&policy.armor!=ArmorApplication::Unsupported)
                throw std::invalid_argument("Invalid armor application policy");
        entries_.reserve(definitions.size());
        for(const auto &definition:definitions) {
            if(definition.coefficients.size()!=policies.size()) throw std::invalid_argument("Armor row width does not match damage type mapping");
            entries_.push_back({definition.coefficients});
        }
    }
    const CompiledArmorDefinition &Get(ArmorDefinitionRef id) const {return entries_.at(id.value);}
    const CompiledArmorDefinition &GetUnchecked(ArmorDefinitionRef id) const noexcept
    {assert(id.value<entries_.size());return entries_[id.value];}
    DamageTypePolicy Policy(DamageTypeId id) const {return policies_.at(id.value);}
    DamageTypePolicy PolicyUnchecked(DamageTypeId id) const noexcept
    {assert(id.value<policies_.size());return policies_[id.value];}
    std::size_t TypeCount() const noexcept {return policies_.size();}
private:
    std::vector<DamageTypePolicy> policies_;std::vector<CompiledArmorDefinition> entries_;
};
}
