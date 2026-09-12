module;
#include <algorithm>
#include <charconv>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>
export module games.generalszh.adapters.content.buildings.building_definition;
export import games.generalszh.adapters.content.units.unit_definition;
export import games.generalszh.adapters.content.rules.game_rules;
export import games.generalszh.gameplay.construction.definitions.building_definition;

namespace generalszh::content::building_detail
{
bool Boolean(std::string_view value)
{
    if(EqualToken(value,"Yes")) return true;
    if(EqualToken(value,"No")) return false;
    throw std::invalid_argument("Building boolean requires Yes or No");
}

std::uint32_t Unsigned(std::string_view value,std::uint32_t maximum)
{
    value=Trim(value);
    std::uint32_t result{};
    const auto parsed=std::from_chars(value.data(),value.data()+value.size(),result);
    if(value.empty()||parsed.ec!=std::errc{}||parsed.ptr!=value.data()+value.size())
        throw std::invalid_argument("Building integer requires unsigned decimal digits");
    if(result>maximum) throw std::out_of_range("Building integer exceeds runtime range");
    return result;
}

bool IsZero(std::string_view value)
{
    value=Trim(value);
    if(!value.empty()&&value.back()=='%') value=Trim(value.substr(0,value.size()-1));
    return ScaledDecimal(value,1000000)==0;
}
}

export namespace generalszh::content
{
struct DecodedBuilding
{
    std::string name;
    construction::BuildingDefinition definition;
    std::vector<std::string> omissions;
    bool ignoredInGui{};
    std::string armorName;
    std::optional<engine::gameplay::combat::damage::ArmorDefinition> armor;
};
// One supplied Object block only. Callers must resolve inherited content before
// decoding if they need it; the adapter never invents missing inherited modules.
inline DecodedBuilding DecodeBuilding(std::string_view text,std::string_view name,
    const DecodedGameRules &rules,std::uint32_t healthQuanta=1000,std::string_view armorText={})
{
    if(!healthQuanta) throw std::invalid_argument("Building health scale must be positive");
    const auto object=ReadNamedBlock(text,"Object",name);
    DecodedBuilding result; result.name=name; auto &definition=result.definition;
    definition.key=DefinitionKey(name);
    if (!object.Value("VisionRange").empty())
        definition.visibility = visibility::DecodeVisibilityDefinition(
            object, 0, visibility::VisibilityContentScale{}, 0, "5000",
            engine::time::FixedStep{30}).definition;
    definition.supplyBoxValue=rules.valuePerSupplyBox;
    const auto kinds=Tokens(object.Value("KindOf"));
    definition.capturable=std::any_of(kinds.begin(),kinds.end(),[](const auto token){return EqualToken(token,"CAPTURABLE");});
    result.ignoredInGui=std::any_of(kinds.begin(),kinds.end(),[](const auto token){return EqualToken(token,"IGNORED_IN_GUI");});
    result.omissions.emplace_back("DefaultThingTemplate/ChildObject inheritance is unresolved; only the supplied Object block is decoded");
    // ThingTemplate constructor defaults: cost 0, build time 1 second, energy 0.
    const auto cost=object.Value("BuildCost");
    const auto credits=cost.empty()?0:ScaledDecimal(cost,1);
    if(credits>65535) throw std::out_of_range("Building cost exceeds legacy unsigned short");
    definition.cost=static_cast<std::uint32_t>(credits);
    const auto duration=object.Value("BuildTime");
    const auto nanos=duration.empty()?1000000000ull:ScaledDecimal(duration,1000000000);
    if(nanos>static_cast<std::uint64_t>((std::numeric_limits<std::int64_t>::max)()))
        throw std::out_of_range("Building duration exceeds runtime range");
    definition.duration=engine::time::Duration{static_cast<std::int64_t>(nanos)};
    const auto energy=object.Value("EnergyProduction");
    if(!energy.empty())
    {
        const auto parsed=std::from_chars(energy.data(),energy.data()+energy.size(),definition.energy);
        if(parsed.ec!=std::errc{}||parsed.ptr!=energy.data()+energy.size())
            throw std::invalid_argument("EnergyProduction must be a signed 32-bit integer");
    }
    for(const auto &field:object.fields)
        if(!EqualToken(field.key,"BuildCost")&&!EqualToken(field.key,"BuildTime")&&!EqualToken(field.key,"EnergyProduction")
            &&!EqualToken(field.key,"KindOf"))
            result.omissions.push_back("Object field: "+field.key);
    bool bodyFound=false,productionFound=false,dockFound=false,armorSetFound=false,conditionalArmor=false;
    bool garrisonFound=false,garrisonSupported=true,garrisonCapacitySet=false;
    std::uint32_t garrisonCapacity{};
    for(const auto &child:object.children)
    {
        const auto words=Tokens(child.argument);
        if(EqualToken(child.kind,"ArmorSet"))
        {
            const auto conditions=child.Value("Conditions");
            if(!conditions.empty()&&!EqualToken(conditions,"None"))
            {
                conditionalArmor=true;
                result.omissions.push_back("Conditional ArmorSet: "+std::string(conditions));
                continue;
            }
            if(armorSetFound) throw std::invalid_argument("Duplicate default ArmorSet");
            armorSetFound=true;
            result.armorName=child.Require("Armor");
            if(armorText.empty()) throw std::invalid_argument("Declared default ArmorSet requires supplied Armor.ini content");
            result.armor=DecodeArmor(ReadNamedBlock(armorText,"Armor",result.armorName),ZeroHourDamageTypes);
            for(const auto &field:child.fields)
                if(!EqualToken(field.key,"Armor")&&!EqualToken(field.key,"Conditions"))
                    result.omissions.push_back("ArmorSet field: "+field.key);
        }
        else if(EqualToken(child.kind,"Body"))
        {
            if(bodyFound||words.size()!=2||(!EqualToken(words[0],"ActiveBody")&&!EqualToken(words[0],"StructureBody")))
                throw std::invalid_argument("Building requires one supported ActiveBody or StructureBody");
            bodyFound=true;
            // StructureBodyModuleData delegates its entire field parser to
            // ActiveBodyModuleData. Its runtime constructor ID is not ported here.
            if(EqualToken(words[0],"StructureBody"))
                result.omissions.emplace_back("StructureBody: legacy constructor-object tracking is not decoded");
            definition.health=ScaledDecimal(child.Require("MaxHealth"),healthQuanta);
            if(!definition.health) throw std::invalid_argument("Building MaxHealth must be positive");
            const auto initial=child.Value("InitialHealth");
            if(initial.empty()||ScaledDecimal(initial,healthQuanta)!=definition.health)
                result.omissions.emplace_back("InitialHealth: construction uses its existing scaffold-health progression to MaxHealth");
            for(const auto &field:child.fields)
                if(!EqualToken(field.key,"MaxHealth")&&!EqualToken(field.key,"InitialHealth"))
                    result.omissions.push_back("ActiveBody field: "+field.key);
        }
        else if(EqualToken(child.kind,"Behavior")&&words.size()==2&&EqualToken(words[0],"ProductionUpdate"))
        {
            if(productionFound) throw std::invalid_argument("Duplicate ProductionUpdate");
            productionFound=true;
            const auto authored=child.Value("MaxQueueEntries");
            const auto limit=authored.empty()?9:ScaledDecimal(authored,1);
            if(limit>INT32_MAX) throw std::out_of_range("Production queue exceeds legacy Int range");
            definition.queueLimit=static_cast<std::uint32_t>(limit);
            for(const auto &field:child.fields)
                if(!EqualToken(field.key,"MaxQueueEntries")) result.omissions.push_back("ProductionUpdate field: "+field.key);
            result.omissions.emplace_back("ProductionUpdate: command-set permissions, upgrade research and exit geometry are not decoded");
        }
        else if(EqualToken(child.kind,"Behavior")&&words.size()==2&&EqualToken(words[0],"SupplyCenterDockUpdate"))
        {
            if(dockFound) throw std::invalid_argument("Duplicate SupplyCenterDockUpdate");
            dockFound=true; definition.supplyDropoff=true;
            result.omissions.emplace_back("SupplyCenterDockUpdate: basic drop-off only; docking geometry, approach slots and docking side effects are unsupported");
            for(const auto &field:child.fields) result.omissions.push_back("SupplyCenterDockUpdate field: "+field.key);
        }
        else if(EqualToken(child.kind,"Behavior")&&words.size()==2&&EqualToken(words[0],"GarrisonContain"))
        {
            if(garrisonFound) throw std::invalid_argument("Duplicate GarrisonContain");
            garrisonFound=true;
            auto reject=[&](std::string_view reason) {
                garrisonSupported=false;
                result.omissions.push_back("GarrisonContain unsupported: "+std::string(reason));
            };
            for(const auto &field:child.fields)
            {
                const auto key=std::string_view(field.key),value=Trim(field.value);
                if(EqualToken(key,"ContainMax"))
                {
                    garrisonCapacity=building_detail::Unsigned(value,(std::numeric_limits<std::uint32_t>::max)());
                    garrisonCapacitySet=true;
                    if(!garrisonCapacity) reject("ContainMax must be positive");
                }
                else if(EqualToken(key,"AllowInsideKindOf"))
                {
                    const auto allowed=Tokens(value);
                    if(allowed.size()!=1||!EqualToken(allowed[0],"INFANTRY")) reject("only INFANTRY is supported");
                }
                else if(EqualToken(key,"ForbidInsideKindOf"))
                {
                    if(!EqualToken(value,"NONE")) reject("ForbidInsideKindOf must be NONE");
                }
                else if(EqualToken(key,"IsEnclosingContainer"))
                {
                    if(!building_detail::Boolean(value)) reject("non-enclosing containers are not supported");
                }
                else if(EqualToken(key,"MobileGarrison"))
                {
                    if(building_detail::Boolean(value)) reject("mobile garrisons are not supported");
                }
                else if(EqualToken(key,"HealObjects"))
                {
                    if(building_detail::Boolean(value)) reject("garrison healing is not supported");
                }
                else if(EqualToken(key,"TimeForFullHeal"))
                {
                    if(!building_detail::IsZero(value)) reject("garrison healing time is not supported");
                }
                else if(EqualToken(key,"DamagePercentToUnits"))
                {
                    if(!building_detail::IsZero(value)) reject("passenger damage transfer is not supported");
                }
                else if(EqualToken(key,"InitialRoster"))
                {
                    reject("initial roster is not supported");
                }
                else if(EqualToken(key,"EnterSound")||EqualToken(key,"ExitSound"))
                {
                    result.omissions.push_back("GarrisonContain presentation field: "+field.key);
                }
                else if(EqualToken(key,"ImmuneToClearBuildingAttacks"))
                {
                    result.omissions.push_back("GarrisonContain gameplay field: "+field.key);
                }
                else if(EqualToken(key,"PassengersAllowedToFire"))
                {
                    if(!building_detail::Boolean(value)) reject("PassengersAllowedToFire=No is outside this fire-permitting first slice");
                    else result.omissions.push_back("GarrisonContain fire override: typed DISABLED_SUBDUED suppression remains omitted");
                }
                else
                {
                    reject(std::string("field ")+field.key);
                }
            }
            if(!garrisonCapacitySet) reject("ContainMax is required");
            if(std::none_of(kinds.begin(),kinds.end(),[](const auto token){return EqualToken(token,"STRUCTURE");}))
                reject("container must be a STRUCTURE");
            if(std::any_of(kinds.begin(),kinds.end(),[](const auto token){return EqualToken(token,"GARRISONABLE_UNTIL_DESTROYED");}))
                reject("KINDOF_GARRISONABLE_UNTIL_DESTROYED requires an unavailable body damage-state component");
            result.omissions.emplace_back("GarrisonContain runtime omission: BODY_REALLYDAMAGED ejection and DISABLED_SUBDUED fire suppression require typed state not present in this slice");
            result.omissions.emplace_back("GarrisonContain runtime limitation: same-owner, same-cell direct entry/exit only; geometry, doors, bones and routing are not decoded");
        }
        else result.omissions.push_back(child.kind+": "+child.argument);
    }
    if(!bodyFound) throw std::invalid_argument("Missing building health body");
    if(!armorSetFound)
    {
        if(conditionalArmor) throw std::invalid_argument("Conditional ArmorSet requires a supported default ArmorSet");
        result.omissions.emplace_back("No ArmorSet in supplied resolved Object: explicit unarmored identity; inheritance not resolved");
    }
    if(garrisonFound)
    {
        if(garrisonSupported) definition.garrison=generalszh::containment::GarrisonDefinition{garrisonCapacity};
        else result.omissions.emplace_back("GarrisonContain not enrolled because one or more authored options are unsupported");
    }
    return result;
}
}
