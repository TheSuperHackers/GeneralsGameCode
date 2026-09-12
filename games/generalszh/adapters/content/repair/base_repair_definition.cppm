module;
#include <cstdint>
#include <numeric>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
export module games.generalszh.adapters.content.repair.base_repair_definition;
export import engine.gameplay.rts.repair.definitions.repair_definition;
export import games.generalszh.adapters.content.ini.named_block;
export namespace generalszh::content
{
struct DecodedBaseRepair
{
    bool enabled{};
    engine::gameplay::rts::repair::RepairDefinition definition;
    std::vector<std::string> omissions;
};
inline DecodedBaseRepair DecodeBaseRepair(std::string_view gameData,const IniBlock &object)
{
    const auto rules=ReadNamedBlock(gameData,"GameData","");
    auto percent=rules.Require("BaseRegenHealthPercentPerSecond");
    if(percent.empty()||percent.back()!='%') throw std::invalid_argument("Base repair rate requires percent suffix");
    percent.remove_suffix(1);
    const auto numerator=ScaledDecimal(percent,1000000);
    if(numerator>UINT32_MAX) throw std::out_of_range("Base repair percent exceeds authored precision");
    const auto milliseconds=ScaledDecimal(rules.Require("BaseRegenDelay"),1);
    if(milliseconds>UINT32_MAX) throw std::out_of_range("Base repair delay exceeds legacy range");
    const auto divisor=std::gcd(numerator,std::uint64_t{100000000});
    DecodedBaseRepair result;
    result.definition={static_cast<std::uint32_t>(numerator/divisor),static_cast<std::uint32_t>(100000000/divisor),
        engine::time::Duration{100000000},engine::time::Duration{static_cast<std::int64_t>(((milliseconds*30+999)/1000)*1000000000/30)},
        engine::time::Duration{1000000000/30}};
    result.omissions.emplace_back("Only explicit BaseRegenerateUpdate enrollment; inherited Object modules are unresolved");
    for(const auto &child:object.children)
    {
        const auto words=Tokens(child.argument);
        if(!EqualToken(child.kind,"Behavior")||words.empty()||!EqualToken(words[0],"BaseRegenerateUpdate")) continue;
        if(words.size()!=2||result.enabled) throw std::invalid_argument("Invalid or duplicate BaseRegenerateUpdate");
        result.enabled=true;
        // Reference module has no own fields. Do not treat unsupported upgrade
        // fields as an unconditionally active capability.
        if(!child.fields.empty()||!child.children.empty()) throw std::invalid_argument("Unsupported BaseRegenerateUpdate fields");
    }
    return result;
}
}
