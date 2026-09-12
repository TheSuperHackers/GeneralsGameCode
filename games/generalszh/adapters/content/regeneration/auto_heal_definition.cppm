module;
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
export module games.generalszh.adapters.content.regeneration.auto_heal_definition;
export import engine.gameplay.combat.regeneration.definitions.regeneration_definition;
export import games.generalszh.adapters.content.ini.named_block;

export namespace generalszh::content
{
struct DecodedAutoHeal
{
    engine::gameplay::combat::regeneration::RegenerationDefinition definition;
    bool startsActive{false};
    // Upgrade execution and presentation remain external to this bounded port.
    std::vector<std::string> unimplementedFields;
};
inline DecodedAutoHeal DecodeAutoHeal(const IniBlock &block, std::uint32_t healthQuanta)
{
    const auto words=Tokens(block.argument);
    if (!EqualToken(block.kind,"Behavior") || words.size()!=2 || !EqualToken(words[0],"AutoHealBehavior"))
        throw std::invalid_argument("Expected an AutoHealBehavior module block");
    if (!healthQuanta) throw std::invalid_argument("Health quanta must be positive");
    auto boolean=[](std::string_view text) {
        if (EqualToken(text,"Yes")) return true;
        if (EqualToken(text,"No")) return false;
        throw std::invalid_argument("Expected Yes or No");
    };
    const auto radius=block.Value("Radius");
    // Content uses both plain decimals and C-style f suffixes for Radius.
    auto numericRadius=radius;
    if (!numericRadius.empty() && (numericRadius.back()=='f'||numericRadius.back()=='F')) numericRadius.remove_suffix(1);
    if ((!radius.empty() && ScaledDecimal(numericRadius,1000000)!=0) ||
        (!block.Value("AffectsWholePlayer").empty() && boolean(block.Value("AffectsWholePlayer"))))
        throw std::invalid_argument("Area and player-wide auto healing are unsupported");
    auto duration=[](std::string_view value) {
        // INI::parseDurationUnsignedInt reads unsigned integral milliseconds
        // and rounds up to reference frames. Preserve that quantization here.
        const auto milliseconds=ScaledDecimal(value,1);
        if (milliseconds>UINT32_MAX) throw std::out_of_range("Auto heal milliseconds exceed unsigned range");
        const auto frames=(milliseconds*30+999)/1000;
        const auto nanos=frames*1000000000/30;
        return engine::time::Duration{static_cast<std::int64_t>(nanos)};
    };
    DecodedAutoHeal result;
    // Require an authored finite interval rather than interpreting UINT_MAX's
    // legacy packed sleep sentinel as a portable modern duration.
    result.definition.interval=duration(block.Require("HealingDelay"));
    if (result.definition.interval.count()<=0) throw std::invalid_argument("HealingDelay must be positive");
    if (!block.Value("HealingAmount").empty())
    {
        const auto amount=ScaledDecimal(block.Value("HealingAmount"),1);
        if (amount>INT32_MAX) throw std::out_of_range("HealingAmount exceeds legacy positive Int range");
        result.definition.quantity=amount*healthQuanta;
    }
    if (!block.Value("StartHealingDelay").empty()) result.definition.damageDelay=duration(block.Value("StartHealingDelay"));
    if (!block.Value("StartsActive").empty()) result.startsActive=boolean(block.Value("StartsActive"));
    // UpdateModule.h: UPDATE_SLEEP_NONE == 1; AutoHealBehavior::onDamage:
    // now > soonestHealFrame. Represent the reference 30 Hz wake duration here,
    // truncated to nanoseconds so quantization at 30 Hz remains exactly one tick.
    result.definition.damageWakeLatency=engine::time::Duration{1000000000/30};
    result.definition.strictDamageWakeCooldown=true;
    for (const auto &field:block.fields)
    {
        if (EqualToken(field.key,"HealingAmount") || EqualToken(field.key,"HealingDelay") ||
            EqualToken(field.key,"StartHealingDelay") || EqualToken(field.key,"StartsActive") ||
            EqualToken(field.key,"Radius") || EqualToken(field.key,"AffectsWholePlayer")) continue;
        // These flags are deliberately ignored by the reference self-only branch.
        if (EqualToken(field.key,"SingleBurst") || EqualToken(field.key,"SkipSelfForHealing"))
        { (void)boolean(field.value); continue; }
        if (EqualToken(field.key,"KindOf") || EqualToken(field.key,"ForbiddenKindOf")) continue;
        result.unimplementedFields.push_back(field.key);
    }
    return result;
}
}
