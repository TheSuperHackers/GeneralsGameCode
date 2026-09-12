module;
#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
export module games.generalszh.adapters.content.selling.sale_definition;
export import games.generalszh.adapters.content.buildings.building_definition;
export import games.generalszh.gameplay.selling.definitions.sell_definition;

export namespace generalszh::content
{
enum class SalePermission { Unresolved, Allowed, Unsellable };
struct DecodedSale
{
    std::optional<selling::SellDefinition> definition;
    std::vector<std::string> omissions;
};
// Startup-only resolved Object adapter. Permission is supplied by the resolved
// command/map/script boundary, not inferred from CommandSet or a KindOf flag.
// Core/.../ControlBarCommand.cpp GUI_COMMAND_SELL checks SCRIPT_UNSELLABLE and
// DISABLED_SUBDUED. Runtime changes remain the caller's SaleEligibility policy.
inline DecodedSale DecodeSale(std::string_view resolvedObjectText,const DecodedBuilding &building,
    std::string_view gameDataText,engine::time::Duration duration,SalePermission permission)
{
    if(building.name.empty() || building.definition.key!=DefinitionKey(building.name))
        throw std::invalid_argument("Sale building identity must match its canonical content name");
    const auto object=ReadNamedBlock(resolvedObjectText,"Object",building.name);
    if(object.argument!=building.name || DefinitionKey(object.argument)!=building.definition.key)
        throw std::invalid_argument("Sale Object identity mismatch");
    if(duration.count()<0) throw std::invalid_argument("Sale duration must be nonnegative");
    DecodedSale result; bool supported=true;
    auto reject=[&](std::string reason){supported=false;result.omissions.push_back(std::move(reason));};
    if(permission!=SalePermission::Allowed)
        reject(permission==SalePermission::Unsellable?"Sale permission: unsellable":"Sale permission: unresolved");
    const auto kinds=Tokens(object.Value("KindOf"));
    auto kind=[&](std::string_view name){return std::any_of(kinds.begin(),kinds.end(),[&](auto value){return EqualToken(value,name);});};
    if(!kind("STRUCTURE") || !kind("IMMOBILE")) reject("Sale requires explicit STRUCTURE and IMMOBILE KindOf");
    for(auto name:{"CAN_ATTACK","PORTABLE_STRUCTURE","SUPPLY_SOURCE","SUPPLY_SOURCE_ON_PREVIEW","HARVESTER","VEHICLE","INFANTRY","AIRCRAFT"})
        if(kind(name)) reject("Unsupported sale KindOf: "+std::string(name));
    if(building.definition.supplyDropoff) reject("Supply drop-off sale is unsupported");
    for(const auto &field:object.fields)
        if(EqualToken(field.key,"Locomotor") || EqualToken(field.key,"Weapon") || EqualToken(field.key,"WeaponSet"))
            reject("Unsupported sale field: "+field.key);
    // Explicitly inspected passive/presentation/death modules do not introduce
    // movement, firing, supply, container, parking or mine ownership. They are
    // omissions, not claims that their gameplay has been decoded here.
    constexpr std::array passive{"PowerPlantUpgrade","BaseRegenerateUpdate","PowerPlantUpdate",
        "DestroyDie","CreateObjectDie","FXListDie","FlammableUpdate","TransitionDamageFX"};
    bool body=false;
    for(const auto &child:object.children)
    {
        const auto words=Tokens(child.argument);
        if(EqualToken(child.kind,"WeaponSet")) reject("WeaponSet sale is unsupported");
        else if(EqualToken(child.kind,"Body"))
        {
            if(body || words.size()!=2 || (!EqualToken(words[0],"ActiveBody")&&!EqualToken(words[0],"StructureBody")))
                reject("Sale requires one supported building body");
            body=true;
            if(child.Value("MaxHealth").empty()) reject("Sale body MaxHealth is missing");
        }
        else if(EqualToken(child.kind,"Behavior"))
        {
            if(words.size()!=2) {reject("Malformed sale behavior: "+child.argument);continue;}
            if(EqualToken(words[0],"ProductionUpdate")) continue;
            if(std::any_of(passive.begin(),passive.end(),[&](auto name){return EqualToken(words[0],name);}))
                result.omissions.push_back("Behavior not decoded by sale adapter: "+child.argument);
            else reject("Unsupported sale behavior: "+child.argument);
        }
        else if(!EqualToken(child.kind,"Draw") && !EqualToken(child.kind,"ClientBehavior")
            && !EqualToken(child.kind,"ArmorSet") && !EqualToken(child.kind,"Prerequisites")
            && !EqualToken(child.kind,"UnitSpecificSounds"))
            reject("Unsupported sale block: "+child.kind);
    }
    if(!body || !building.definition.health) reject("Sale requires decoded building health");
    result.omissions.emplace_back("Resolved Object only: inheritance and command/map/script permission must be supplied by caller; runtime script/subdued changes are not decoded");
    result.omissions.emplace_back("Duration is explicitly authored by caller; BuildAssistant frame-start/scaffold timing is not inferred");
    result.omissions.emplace_back("Refund uses startup building cost without player/handicap modifiers; rational percentage arithmetic can differ from legacy float rounding");
    if(!supported) return result;
    selling::SellDefinition definition; definition.building=building.definition.key; definition.duration=duration;
    // ThingTemplate.cpp: RefundValue uses parseUnsignedShort and defaults to 0.
    const auto refund=object.Value("RefundValue");
    for(const auto &field:object.fields)
        if(EqualToken(field.key,"RefundValue") && Trim(field.value).empty())
            throw std::invalid_argument("RefundValue must not be empty");
    const auto value=refund.empty()?0:ScaledDecimal(refund,1);
    if(value>UINT16_MAX) throw std::out_of_range("RefundValue exceeds unsigned short");
    definition.refundValue=static_cast<std::uint32_t>(value);
    const auto data=ReadNamedBlock(gameDataText,"GameData","");
    auto percent=Trim(data.Value("SellPercentage"));
    for(const auto &field:data.fields)
        if(EqualToken(field.key,"SellPercentage") && Trim(field.value).empty())
            throw std::invalid_argument("SellPercentage must not be empty");
    // GlobalData.cpp initializes 1.0; shipped GameData.ini overrides with 50%.
    // INI::parsePercentToReal treats both 50 and 50% as percent units.
    // This bounded adapter accepts fixed decimals at six decimal places in
    // percent units, rejecting unrepresentable precision and exponent syntax.
    if(percent.empty()) {definition.percentageNumerator=1;definition.percentageDenominator=1;}
    else
    {
        if(percent.back()=='%') percent=Trim(percent.substr(0,percent.size()-1));
        constexpr std::uint64_t scale=1000000,denominator=100*scale;
        const auto numerator=ScaledDecimal(percent,scale);
        const auto divisor=std::gcd(numerator,denominator);
        if(numerator/divisor>UINT32_MAX) throw std::out_of_range("SellPercentage exceeds rational runtime range");
        definition.percentageNumerator=static_cast<std::uint32_t>(numerator/divisor);
        definition.percentageDenominator=static_cast<std::uint32_t>(denominator/divisor);
    }
    result.definition=definition;
    return result;
}
}
