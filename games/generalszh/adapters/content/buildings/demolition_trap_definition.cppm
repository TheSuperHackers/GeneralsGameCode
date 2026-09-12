module;
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
export module games.generalszh.adapters.content.buildings.demolition_trap_definition;
export import games.generalszh.adapters.content.ini.named_block;
export import games.generalszh.adapters.content.units.unit_definition;
export import games.generalszh.gameplay.demolition.definitions.demolition_trap_definition;

namespace generalszh::content::demolition_detail
{
std::uint32_t Unsigned(std::string_view value,std::string_view field)
{
    value=Trim(value); const auto parsed=ScaledDecimal(value,1);
    if (parsed>(std::numeric_limits<std::uint32_t>::max)())
        throw std::out_of_range("Demolition field exceeds uint32 range: "+std::string(field));
    return static_cast<std::uint32_t>(parsed);
}

engine::gameplay::combat::WeaponConfig DecodeWeapon(std::string_view text,std::string_view name,ContentScale scale)
{
    const auto block=ReadNamedBlock(text,"Weapon",name);
    engine::gameplay::combat::WeaponConfig result;
    const auto authoredType=block.Value("DamageType");
    const auto typeName=authoredType.empty()?std::string_view{"EXPLOSION"}:authoredType;
    const auto type=std::find_if(ZeroHourDamageTypes.begin(),ZeroHourDamageTypes.end(),
        [&](const auto &entry){return EqualToken(entry.name,typeName);});
    if (type==ZeroHourDamageTypes.end() || type->policy.armor==engine::gameplay::combat::damage::ArmorApplication::Unsupported)
        throw std::invalid_argument("Demolition weapon uses an unsupported DamageType");
    result.damageType=type->id;
    result.damage=ScaledDecimal(block.Require("PrimaryDamage"),scale.healthQuanta);
    const auto secondary=block.Value("SecondaryDamage");
    if (!secondary.empty()) result.secondaryDamage=ScaledDecimal(secondary,scale.healthQuanta);
    const auto primaryRadius=block.Value("PrimaryDamageRadius"),secondaryRadius=block.Value("SecondaryDamageRadius");
    if (!primaryRadius.empty()) result.primaryRadiusCells=RadiusCells(primaryRadius,scale);
    if (!secondaryRadius.empty()) result.secondaryRadiusCells=RadiusCells(secondaryRadius,scale);
    const auto affects=block.Value("RadiusDamageAffects");
    if (!affects.empty()) result.radiusDamageAffects=DecodeRadiusDamageAffects(affects);
    const auto selfDamage=block.Value("DamageDealtAtSelfPosition");
    if (!selfDamage.empty() && !EqualToken(selfDamage,"No"))
        throw std::invalid_argument("Demolition weapon DamageDealtAtSelfPosition=Yes is unsupported");
    const auto range=block.Value("AttackRange");
    result.rangeCells=range.empty()?1:RadiusCells(range,scale);
    const auto clip=block.Value("ClipSize");
    result.clipSize=clip.empty()?0:Unsigned(clip,"ClipSize");
    result.ammunition=result.clipSize?engine::gameplay::combat::AmmunitionPolicy::Limited:
        engine::gameplay::combat::AmmunitionPolicy::Unlimited;
    const auto shot=block.Value("DelayBetweenShots"),reload=block.Value("ClipReloadTime"),pre=block.Value("PreAttackDelay");
    if (!shot.empty()) result.shotInterval=DecodeMilliseconds(shot,"DelayBetweenShots",false);
    if (!reload.empty()) result.reloadTime=DecodeMilliseconds(reload,"ClipReloadTime",false);
    if (!pre.empty()) result.preAttackDelay=DecodeMilliseconds(pre,"PreAttackDelay",false);
    const auto preType=block.Value("PreAttackType");
    if (!preType.empty())
    {
        if (EqualToken(preType,"PER_SHOT")) result.preAttackType=engine::gameplay::combat::PrefirePolicy::PerShot;
        else if (EqualToken(preType,"PER_ATTACK")) result.preAttackType=engine::gameplay::combat::PrefirePolicy::PerAttack;
        else if (EqualToken(preType,"PER_CLIP")) result.preAttackType=engine::gameplay::combat::PrefirePolicy::PerClip;
        else throw std::invalid_argument("Unsupported demolition PreAttackType");
    }
    const auto autoReload=block.Value("AutoReloadsClip");
    if (!autoReload.empty()) result.autoReload=DecodeBool(autoReload,"AutoReloadsClip",true);
    result.flightTime=std::chrono::milliseconds{50};
    if (!result.damage && !result.secondaryDamage)
        throw std::invalid_argument("Demolition weapon must cause damage");
    return result;
}

void DecodeIgnoreTypes(std::string_view value,generalszh::demolition::DemolitionIgnorePolicy &policy)
{
    for (const auto token:Tokens(value))
    {
        if (EqualToken(token,"NONE")) continue;
        if (EqualToken(token,"STRUCTURE")) policy.structures=true;
        else if (EqualToken(token,"AIRCRAFT")||EqualToken(token,"AIRBORNE")) policy.airborne=true;
        else if (EqualToken(token,"PROJECTILE")) policy.projectiles=true;
        else if (EqualToken(token,"UNATTACKABLE")) policy.unattackable=true;
        else
            throw std::invalid_argument("Unsupported DemoTrapUpdate IgnoreTargetTypes token: "+std::string(token));
    }
}

void DecodeSlowDeath(const IniBlock &object,std::string_view weaponText,ContentScale scale,
    generalszh::demolition::DemolitionTrapDefinition &result,std::vector<std::string> &omissions)
{
    bool found=false;
    for (const auto &child:object.children)
    {
        const auto words=Tokens(child.argument);
        if (!EqualToken(child.kind,"Behavior") || words.empty() || !EqualToken(words[0],"SlowDeathBehavior")) continue;
        if (found) throw std::invalid_argument("Duplicate SlowDeathBehavior for demolition trap");
        found=true;
        for (const auto token:Tokens(child.Value("ExemptStatus")))
            if (!EqualToken(token,"UNDER_CONSTRUCTION")&&!EqualToken(token,"SOLD"))
                throw std::invalid_argument("Unsupported SlowDeathBehavior ExemptStatus for demolition trap");
        result.deathWeapon.destructionDelay=DecodeMilliseconds(child.Require("DestructionDelay"),"DestructionDelay",false);
        for (const auto &field:child.fields)
        {
            if (EqualToken(field.key,"ExemptStatus")||EqualToken(field.key,"DestructionDelay")) continue;
            if (EqualToken(field.key,"FX")||EqualToken(field.key,"OCL"))
            {
                omissions.emplace_back("SlowDeathBehavior "+field.key+" is unsupported in the bounded runtime");
                continue;
            }
            if (!EqualToken(field.key,"Weapon"))
                throw std::invalid_argument("Unsupported SlowDeathBehavior field: "+field.key);
            const auto weapon=Tokens(field.value);
            if (weapon.size()!=2 || (!EqualToken(weapon[0],"INITIAL")&&!EqualToken(weapon[0],"FINAL")))
                throw std::invalid_argument("SlowDeathBehavior weapon must name one INITIAL or FINAL phase");
            auto &slot=EqualToken(weapon[0],"INITIAL")?result.deathWeapon.initial:result.deathWeapon.final;
            if (slot) throw std::invalid_argument("Duplicate SlowDeathBehavior weapon phase");
            slot=DecodeWeapon(weaponText,weapon[1],scale);
        }
    }
    if (!found) throw std::invalid_argument("DemoTrapUpdate requires SlowDeathBehavior for this bounded slice");
    if (!result.deathWeapon.initial&&!result.deathWeapon.final)
        throw std::invalid_argument("SlowDeathBehavior has no supported INITIAL or FINAL weapon");
}
}

export namespace generalszh::content
{
struct DecodedDemolitionTrap
{
    std::string name;
    demolition::DemolitionTrapDefinition definition;
    std::vector<std::string> omissions;
};

inline std::optional<DecodedDemolitionTrap> TryDecodeDemolitionTrap(std::string_view objectText,
    std::string_view name,std::string_view weaponText,ContentScale scale={})
{
    IniBlock object;
    try { object=ReadNamedBlock(objectText,"Object",name); }
    catch (const std::invalid_argument &error)
    {
        if (std::string_view(error.what()).find("Missing INI block")!=std::string_view::npos) return std::nullopt;
        throw;
    }
    demolition::DemolitionTrapDefinition result;
    result.key=DefinitionKey(name); result.deathWeapon.key=result.key;
    std::vector<std::string> omissions;
    bool updateFound=false;
    for (const auto &child:object.children)
    {
        const auto words=Tokens(child.argument);
        if (!EqualToken(child.kind,"Behavior") || words.empty() || !EqualToken(words[0],"DemoTrapUpdate")) continue;
        if (updateFound) throw std::invalid_argument("Duplicate DemoTrapUpdate for demolition trap");
        updateFound=true;
        result.mode=DecodeBool(child.Value("DefaultProximityMode"),"DefaultProximityMode",false)
            ?demolition::DemolitionDetonationMode::Proximity:demolition::DemolitionDetonationMode::Manual;
        const auto range=child.Value("TriggerDetonationRange");
        result.radiusCells=range.empty()?1:RadiusCells(range,scale);
        const auto scan=child.Value("ScanRate");
        if (!scan.empty())
        {
            // INI::parseDurationUnsignedInt consumes authored milliseconds and
            // only then rounds to legacy frames.  Keep the typed milliseconds;
            // the modern FixedStep performs its own deadline conversion.  The
            // reference countdown checks the next update after assigning
            // m_scanFrames, so its cadence can be one legacy update later than
            // this deadline-based modern scan.
            result.scanInterval=DecodeMilliseconds(scan,"ScanRate",false);
        }
        result.friendlyDetonation=DecodeBool(child.Value("AutoDetonationWithFriendsInvolved"),
            "AutoDetonationWithFriendsInvolved",false);
        result.detonateWhenKilled=DecodeBool(child.Value("DetonateWhenKilled"),"DetonateWhenKilled",false);
        const auto ignored=child.Value("IgnoreTargetTypes");
        if (!ignored.empty()) demolition_detail::DecodeIgnoreTypes(ignored,result.ignoreTargetTypes);
        const auto active=child.Value("DetonationWeapon");
        if (!active.empty()&&!EqualToken(active,"NONE"))
        {
            const auto names=Tokens(active);
            if (names.size()!=1) throw std::invalid_argument("DemoTrapUpdate DetonationWeapon must name one weapon");
            result.detonationWeapon=demolition_detail::DecodeWeapon(weaponText,names[0],scale);
        }
        for (const auto &field:child.fields)
            if (EqualToken(field.key,"DetonationWeaponSlot")||EqualToken(field.key,"ProximityModeWeaponSlot")||
                EqualToken(field.key,"ManualModeWeaponSlot"))
                omissions.emplace_back("DemoTrapUpdate weapon slots are represented by typed mode commands");
    }
    if (!updateFound) return std::nullopt;
    demolition_detail::DecodeSlowDeath(object,weaponText,scale,result,omissions);
    return DecodedDemolitionTrap{std::string(name),std::move(result),std::move(omissions)};
}

inline demolition::DemolitionTargetClassification DecodeDemolitionTargetClassification(
    std::string_view objectText,std::string_view name)
{
    const auto object=ReadNamedBlock(objectText,"Object",name);
    const auto kinds=Tokens(object.Value("KindOf"));
    demolition::DemolitionTargetClassification result;
    result.airborne=std::any_of(kinds.begin(),kinds.end(),[](const auto token) {
        return EqualToken(token,"AIRCRAFT")||EqualToken(token,"AIRBORNE");
    });
    result.unattackable=std::any_of(kinds.begin(),kinds.end(),[](const auto token) {
        return EqualToken(token,"UNATTACKABLE");
    });
    return result;
}

inline std::optional<DecodedDemolitionTrap> DecodeFirstDemolitionTrap(std::string_view objectText,
    std::string_view weaponText,ContentScale scale={})
{
    const auto source=objectText;
    std::vector<std::string_view> names;
    while (!objectText.empty())
    {
        const auto end=objectText.find('\n'); const auto line=Trim(objectText.substr(0,end));
        const auto words=Tokens(line);
        if (words.size()==2&&EqualToken(words[0],"Object")&&
            std::find(names.begin(),names.end(),words[1])==names.end())
        {
            names.push_back(words[1]);
            if (const auto trap=TryDecodeDemolitionTrap(source,words[1],weaponText,scale)) return trap;
        }
        if (end==std::string_view::npos) break;
        objectText.remove_prefix(end+1);
    }
    return std::nullopt;
}
}
