module;
#include <algorithm>
#include <array>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>
export module games.generalszh.adapters.content.armor.armor_definition;
export import games.generalszh.adapters.content.ini.named_block;
export import engine.gameplay.combat.damage.definitions.armor_catalog;
export namespace generalszh::content {
using engine::gameplay::combat::damage::DamageTypeId;
using engine::gameplay::combat::damage::DamageTypePolicy;
using engine::gameplay::combat::damage::ArmorApplication;
struct DamageTypeMapping {std::string_view name;DamageTypeId id;DamageTypePolicy policy;};
// Explicit Zero Hour content contract from Core Damage.h / Damage.cpp.
// RTS_GENERALS-only FLESHY_SNIPER is intentionally absent. IDs do not depend
// on array traversal, registration order, RTTI or a legacy enum import.
inline constexpr std::array<DamageTypeMapping,38> ZeroHourDamageTypes{{
    {"EXPLOSION",{0},{ArmorApplication::Scale}},
    {"CRUSH",{1},{ArmorApplication::Scale}},
    {"ARMOR_PIERCING",{2},{ArmorApplication::Scale}},
    {"SMALL_ARMS",{3},{ArmorApplication::Scale}},
    {"GATTLING",{4},{ArmorApplication::Scale}},
    {"RADIATION",{5},{ArmorApplication::Scale}},
    {"FLAME",{6},{ArmorApplication::Scale}},
    {"LASER",{7},{ArmorApplication::Scale}},
    {"SNIPER",{8},{ArmorApplication::Scale}},
    {"POISON",{9},{ArmorApplication::Scale}},
    {"HEALING",{10},{ArmorApplication::Unsupported}},
    {"UNRESISTABLE",{11},{ArmorApplication::Bypass}},
    {"WATER",{12},{ArmorApplication::Unsupported}},
    {"DEPLOY",{13},{ArmorApplication::Unsupported}},
    {"SURRENDER",{14},{ArmorApplication::Unsupported}},
    {"HACK",{15},{ArmorApplication::Unsupported}},
    {"KILL_PILOT",{16},{ArmorApplication::Unsupported}},
    {"PENALTY",{17},{ArmorApplication::Unsupported}},
    {"FALLING",{18},{ArmorApplication::Scale}},
    {"MELEE",{19},{ArmorApplication::Scale}},
    {"DISARM",{20},{ArmorApplication::Unsupported}},
    {"HAZARD_CLEANUP",{21},{ArmorApplication::Unsupported}},
    {"PARTICLE_BEAM",{22},{ArmorApplication::Scale}},
    {"TOPPLING",{23},{ArmorApplication::Unsupported}},
    {"INFANTRY_MISSILE",{24},{ArmorApplication::Scale}},
    {"AURORA_BOMB",{25},{ArmorApplication::Scale}},
    {"LAND_MINE",{26},{ArmorApplication::Scale}},
    {"JET_MISSILES",{27},{ArmorApplication::Scale}},
    {"STEALTHJET_MISSILES",{28},{ArmorApplication::Scale}},
    {"MOLOTOV_COCKTAIL",{29},{ArmorApplication::Scale}},
    {"COMANCHE_VULCAN",{30},{ArmorApplication::Scale}},
    {"SUBDUAL_MISSILE",{31},{ArmorApplication::Unsupported}},
    {"SUBDUAL_VEHICLE",{32},{ArmorApplication::Unsupported}},
    {"SUBDUAL_BUILDING",{33},{ArmorApplication::Unsupported}},
    {"SUBDUAL_UNRESISTABLE",{34},{ArmorApplication::Unsupported}},
    {"MICROWAVE",{35},{ArmorApplication::Unsupported}},
    {"KILL_GARRISONED",{36},{ArmorApplication::Unsupported}},
    {"STATUS",{37},{ArmorApplication::Unsupported}}
}};
// Conservative initial executable subset. Special names can be represented in
// tables but cannot be relabelled as ordinary health damage by mapping inputs.
inline ArmorApplication ArmorSemantics(std::string_view name) {
    if(EqualToken(name,"UNRESISTABLE")) return ArmorApplication::Bypass;
    constexpr std::array ordinary{"EXPLOSION","CRUSH","ARMOR_PIERCING","SMALL_ARMS","GATTLING",
        "RADIATION","FLAME","LASER","SNIPER","POISON","FALLING","MELEE","INFANTRY_MISSILE",
        "AURORA_BOMB","LAND_MINE","JET_MISSILES","STEALTHJET_MISSILES","MOLOTOV_COCKTAIL",
        "COMANCHE_VULCAN","PARTICLE_BEAM"};
    for(const auto token:ordinary) if(EqualToken(name,token)) return ArmorApplication::Scale;
    constexpr std::array special{"HEALING","SUBDUAL_UNRESISTABLE","SUBDUAL_MISSILE","SUBDUAL_VEHICLE",
        "SUBDUAL_BUILDING","KILL_PILOT","STATUS","HAZARD_CLEANUP","DISARM","SURRENDER","HACK",
        "DEPLOY","WATER","PENALTY","MICROWAVE","KILL_GARRISONED","TOPPLING"};
    for(const auto token:special) if(EqualToken(name,token)) return ArmorApplication::Unsupported;
    throw std::invalid_argument("Unknown damage type name");
}
inline std::vector<DamageTypePolicy> DecodeDamagePolicies(std::span<const DamageTypeMapping> mapping) {
    if(mapping.size()>UINT32_MAX) throw std::length_error("Damage mapping capacity");
    std::vector<DamageTypePolicy> result(mapping.size());std::vector<bool> seen(mapping.size());
    for(std::size_t i=0;i<mapping.size();++i) {
        const auto &entry=mapping[i];
        if(entry.id.value>=mapping.size()||seen[entry.id.value]) throw std::invalid_argument("Damage mapping IDs must be unique and dense");
        for(std::size_t j=0;j<i;++j) if(EqualToken(entry.name,mapping[j].name)) throw std::invalid_argument("Duplicate damage type name");
        const auto semantics=ArmorSemantics(entry.name);
        if(entry.policy.armor!=semantics) throw std::invalid_argument("Damage mapping changes supported semantic policy");
        seen[entry.id.value]=true;result[entry.id.value]=entry.policy;
    }
    return result;
}
inline engine::gameplay::combat::damage::ArmorDefinition DecodeArmor(const IniBlock &block,std::span<const DamageTypeMapping> mapping) {
    using namespace engine::gameplay::combat::damage;
    (void)DecodeDamagePolicies(mapping);
    if(!EqualToken(block.kind,"Armor")||!block.children.empty()) throw std::invalid_argument("Expected flat Armor block");
    ArmorDefinition result;result.coefficients.resize(mapping.size()); // Reference clear(): 100%.
    for(const auto &field:block.fields) {
        if(!EqualToken(field.key,"Armor")) throw std::invalid_argument("Unsupported Armor field");
        const auto tokens=Tokens(field.value);
        if(tokens.size()!=2||tokens[1].empty()||tokens[1].back()!='%') throw std::invalid_argument("Armor requires type and percentage");
        const auto scaled=ScaledDecimal(tokens[1].substr(0,tokens[1].size()-1),10'000);
        if(scaled>UINT32_MAX) throw std::out_of_range("Armor coefficient exceeds fixed-point range");
        const ArmorMultiplier value{static_cast<std::uint32_t>(scaled)};
        if(EqualToken(tokens[0],"DEFAULT")) {std::fill(result.coefficients.begin(),result.coefficients.end(),value);continue;}
        const auto found=std::find_if(mapping.begin(),mapping.end(),[&](const auto &entry){return EqualToken(entry.name,tokens[0]);});
        if(found==mapping.end()) throw std::invalid_argument("Armor type is absent from explicit damage mapping");
        result.coefficients[found->id.value]=value;
    }
    return result;
}
}
