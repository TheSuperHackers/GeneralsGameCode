module;
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <limits>
#include <numeric>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
export module games.generalszh.adapters.content.units.unit_definition;
export import games.generalszh.adapters.content.ini.named_block;
export import games.generalszh.gameplay.production.runtime.production_runtime;
export import games.generalszh.adapters.content.progression.veterancy_decoder;
export import games.generalszh.adapters.content.regeneration.auto_heal_definition;
export import games.generalszh.adapters.content.containment.transport_definition;
export import games.generalszh.adapters.content.capture.capture_definition;
export import games.generalszh.adapters.content.armor.armor_definition;
export import games.generalszh.adapters.content.prerequisites.prerequisite_definition;
export import engine.gameplay.concealment.definitions.concealment_definition;
export import engine.gameplay.concealment.definitions.detection_definition;
export import games.generalszh.gameplay.concealment.definitions.stealth_policy;
export import games.generalszh.adapters.content.visibility.visibility_definition;
export namespace generalszh::content
{
struct ContentScale { std::uint32_t healthQuanta{1000},worldUnitsPerCell{10}; };
struct DecodedUnit
{
    std::string name,weaponName,locomotorName;
    production::BuildDefinition definition;
    std::vector<std::string> omittedBehaviors;
    std::optional<progression::VeterancyDefinition> progression;
    bool ignoredInGui{};
    std::optional<DecodedAutoHeal> regeneration;
    std::optional<engine::gameplay::containment::TransportDefinition> transport;
    // Single-object decoding deliberately keeps this unresolved authoring
    // reference. BindDecodedUnitCatalog resolves it only after all units are
    // present and identities have been validated.
    std::optional<InitialPayloadReference> initialPayload;
    std::optional<capture::CaptureDefinition> capture;
    bool captureEnabled{};
    std::string armorName;
    std::vector<engine::gameplay::combat::damage::DamageTypePolicy> damagePolicies;
    std::optional<engine::gameplay::combat::damage::ArmorDefinition> armor;
    DecodedPrerequisites prerequisites;
};
inline std::uint32_t RadiusCells(std::string_view text, ContentScale scale)
{
    const auto microWorldUnits=ScaledDecimal(text,1'000'000);
    const auto microWorldUnitsPerCell=std::uint64_t{scale.worldUnitsPerCell}*1'000'000;
    const auto cells=microWorldUnits/microWorldUnitsPerCell+
        (microWorldUnits%microWorldUnitsPerCell ? 1u : 0u);
    if(cells>(std::numeric_limits<std::uint32_t>::max)())
        throw std::out_of_range("Weapon radius exceeds supported cell capacity");
    return static_cast<std::uint32_t>(cells);
}
inline engine::gameplay::combat::RadiusDamageAffects DecodeRadiusDamageAffects(std::string_view text)
{
    engine::gameplay::combat::RadiusDamageAffects result; result.bits=0;
    bool found=false;
    while(!(text=Trim(text)).empty())
    {
        const auto end=text.find_first_of(" \t,|");
        const auto token=text.substr(0,end);
        if(end==text.npos) text={}; else text.remove_prefix(end+1);
        if(token.empty()) continue;
        found=true;
        using Affect=engine::gameplay::combat::RadiusDamageAffect;
        if(EqualToken(token,"SELF")) result.bits|=static_cast<std::uint8_t>(Affect::Self);
        else if(EqualToken(token,"ALLIES")) result.bits|=static_cast<std::uint8_t>(Affect::Allies);
        else if(EqualToken(token,"ENEMIES")) result.bits|=static_cast<std::uint8_t>(Affect::Enemies);
        else if(EqualToken(token,"NEUTRALS")) result.bits|=static_cast<std::uint8_t>(Affect::Neutrals);
        else if(EqualToken(token,"SUICIDE")||EqualToken(token,"NOT_SIMILAR")||EqualToken(token,"NOT_AIRBORNE"))
            throw std::invalid_argument("Unsupported RadiusDamageAffects flag: "+std::string(token));
        else throw std::invalid_argument("Unknown RadiusDamageAffects flag: "+std::string(token));
    }
    if(!found) throw std::invalid_argument("RadiusDamageAffects requires at least one flag");
    return result;
}
inline std::uint32_t DefinitionKey(std::string_view name) noexcept
{ std::uint32_t hash=2166136261u; for(const unsigned char c:name) {hash^=c;hash*=16777619u;} return hash; }
inline bool DecodeBool(std::string_view value, std::string_view field, bool defaultValue = false)
{
    if (value.empty()) return defaultValue;
    if (EqualToken(value,"Yes")) return true;
    if (EqualToken(value,"No")) return false;
    throw std::invalid_argument("Invalid boolean field: "+std::string(field));
}
inline engine::time::Duration DecodeMilliseconds(std::string_view value, std::string_view field,
    bool positive)
{
    const auto nanos=ScaledDecimal(value,1'000'000);
    if ((positive && !nanos) || nanos>static_cast<std::uint64_t>((std::numeric_limits<std::int64_t>::max)()))
        throw std::invalid_argument("Invalid duration field: "+std::string(field));
    return engine::time::Duration{static_cast<std::int64_t>(nanos)};
}
inline std::uint64_t OptionalScaledDecimal(const auto &child, std::string_view field,
    std::uint64_t scale)
{
    const auto value=child.Value(field);
    return value.empty()?0:ScaledDecimal(value,scale);
}
inline bool IsAbsentOrNone(std::string_view value) noexcept
{ return value.empty() || EqualToken(value,"NONE"); }
inline std::uint8_t DecodeStealthForbiddenConditions(std::string_view value)
{
    std::uint8_t result{};
    for (const auto token : Tokens(value))
    {
        if (EqualToken(token,"MOVING"))
            result |= static_cast<std::uint8_t>(generalszh::concealment::StealthForbiddenCondition::Moving);
        else if (EqualToken(token,"FIRING_PRIMARY") || EqualToken(token,"FIRING_SECONDARY") ||
            EqualToken(token,"FIRING_TERTIARY"))
            result |= static_cast<std::uint8_t>(generalszh::concealment::StealthForbiddenCondition::Firing);
        else if (!EqualToken(token,"NONE"))
            throw std::invalid_argument("Unsupported simulation stealth forbidden condition: "+std::string(token));
    }
    return result;
}
inline void RejectUnsupportedStealthFields(const auto &child)
{
    if (!IsAbsentOrNone(child.Value("HintDetectableConditions")) || !IsAbsentOrNone(child.Value("RequiredStatus")) ||
        !IsAbsentOrNone(child.Value("ForbiddenStatus")) || DecodeBool(child.Value("DisguisesAsTeam"), "DisguisesAsTeam") ||
        DecodeBool(child.Value("UseRiderStealth"), "UseRiderStealth") ||
        DecodeBool(child.Value("GrantedBySpecialPower"), "GrantedBySpecialPower") ||
        OptionalScaledDecimal(child,"RevealDistanceFromTarget",1'000'000) != 0 ||
        DecodeBool(child.Value("OrderIdleEnemiesToAttackMeUponReveal"), "OrderIdleEnemiesToAttackMeUponReveal") ||
        OptionalScaledDecimal(child,"MoveThresholdSpeed",1'000'000) != 0 ||
        OptionalScaledDecimal(child,"BlackMarketCheckDelay",1'000'000) != 0)
        throw std::invalid_argument("Unsupported simulation-affecting StealthUpdate field");
}
inline void RejectUnsupportedDetectorFields(const auto &child)
{
    if (!child.Value("ExtraRequiredKindOf").empty() || !child.Value("ExtraForbiddenKindOf").empty() ||
        DecodeBool(child.Value("CanDetectWhileGarrisoned"), "CanDetectWhileGarrisoned"))
        throw std::invalid_argument("Unsupported simulation-affecting StealthDetectorUpdate field");
}
inline DecodedUnit DecodeUnit(std::string_view objectText,std::string_view weaponText,std::string_view locomotorText,
    std::string_view objectName,ContentScale scale={},std::string_view specialPowerText={},
    CapturePermission capturePermission=CapturePermission::Unresolved,std::string_view armorText={},
    std::span<const UpgradeIdentity> upgradeIdentities={})
{
    if(!scale.healthQuanta||!scale.worldUnitsPerCell) throw std::invalid_argument("Content scales must be positive");
    const auto object=ReadNamedBlock(objectText,"Object",objectName);
    DecodedUnit result; result.name=objectName; auto &definition=result.definition; definition.key=DefinitionKey(objectName);
    // VisionRange/ShroudClearingRange are decoded at the content boundary;
    // GameSession recompiles the typed duration for its actual fixed step.
    if (!object.Value("VisionRange").empty())
        definition.visibility = visibility::DecodeVisibilityDefinition(
            object, 0, visibility::VisibilityContentScale{scale.worldUnitsPerCell}, 0, "5000",
            engine::time::FixedStep{30}).definition;
    const auto kindOf=Tokens(object.Value("KindOf"));
    result.ignoredInGui=std::any_of(kindOf.begin(),kindOf.end(),
        [](const auto token) { return EqualToken(token,"IGNORED_IN_GUI"); });
    result.damagePolicies=DecodeDamagePolicies(ZeroHourDamageTypes);
    result.prerequisites=DecodePrerequisites(object);
    std::string captureTemplate;
    if(!specialPowerText.empty())
    {
        const bool hasAbility=std::any_of(object.children.begin(),object.children.end(),[](const auto &child) {
            const auto words=Tokens(child.argument);
            return EqualToken(child.kind,"Behavior")&&!words.empty()&&EqualToken(words[0],"SpecialAbility");
        });
        if(hasAbility)
        {
            auto decoded=DecodeCapture(objectText,specialPowerText,objectName,capturePermission,upgradeIdentities);
            result.capture=decoded.definition;result.captureEnabled=decoded.enabled;
            if(result.capture) captureTemplate=decoded.specialPower;
            result.omittedBehaviors.insert(result.omittedBehaviors.end(),decoded.omissions.begin(),decoded.omissions.end());
        }
    }
    // This reader selects one Object block; it does not merge DefaultThingTemplate
    // or ChildObject inheritance. Only explicitly supplied capability fields opt in.
    for(const auto &field:object.fields)
        if(EqualToken(field.key,"ExperienceRequired") || EqualToken(field.key,"ExperienceValue") ||
            EqualToken(field.key,"SkillPointValue") || EqualToken(field.key,"IsTrainable"))
        { result.progression=DecodeVeterancy(object); break; }
    const auto cost=ScaledDecimal(object.Require("BuildCost"),1);
    if(cost>65535) throw std::out_of_range("Legacy BuildCost exceeds unsigned short");
    definition.cost=static_cast<std::uint32_t>(cost);
    const auto duration=ScaledDecimal(object.Require("BuildTime"),1000000000);
    if(duration>static_cast<std::uint64_t>((std::numeric_limits<std::int64_t>::max)())) throw std::out_of_range("BuildTime exceeds duration range");
    definition.duration=engine::time::Duration{static_cast<std::int64_t>(duration)};
    bool bodyFound=false,weaponFound=false,lifetimeFound=false,armorSetFound=false,conditionalArmor=false,manualRepairFound=false;
    bool stealthFound=false,detectorFound=false,transportContainAuthored=false;
    // KindOf is present on every resolved Object and is not a containment
    // discriminator. Decode the transport adapter only when the object has a
    // passenger slot declaration or an actual TransportContain behavior.
    bool transportAuthored=!object.Value("TransportSlotCount").empty();
    for(const auto &child:object.children)
    {
        if(EqualToken(child.kind,"ArmorSet"))
        {
            const auto conditions=child.Value("Conditions");
            if(!conditions.empty()&&!EqualToken(conditions,"None"))
            {
                conditionalArmor=true;result.omittedBehaviors.push_back("Conditional ArmorSet: "+std::string(conditions));
                continue;
            }
            if(armorSetFound) throw std::invalid_argument("Duplicate default ArmorSet");
            armorSetFound=true;result.armorName=child.Require("Armor");
            if(armorText.empty()) throw std::invalid_argument("Declared default ArmorSet requires supplied Armor.ini content");
            result.armor=DecodeArmor(ReadNamedBlock(armorText,"Armor",result.armorName),ZeroHourDamageTypes);
            for(const auto &field:child.fields)
                if(!EqualToken(field.key,"Armor")&&!EqualToken(field.key,"Conditions"))
                    result.omittedBehaviors.push_back("ArmorSet field: "+field.key);
        }
        if(EqualToken(child.kind,"Body"))
        {
            const auto words=Tokens(child.argument);
            if(bodyFound||words.empty()||!EqualToken(words[0],"ActiveBody")) throw std::invalid_argument("Unit requires one supported ActiveBody");
            const auto maximum=ScaledDecimal(child.Require("MaxHealth"),scale.healthQuanta);
            definition.maximumHealth=maximum;
            const auto initial=child.Value("InitialHealth"); definition.health=initial.empty()?maximum:ScaledDecimal(initial,scale.healthQuanta); bodyFound=true;
            if(definition.health>maximum) throw std::invalid_argument("InitialHealth exceeds MaxHealth");
        }
        if(EqualToken(child.kind,"WeaponSet")&&(child.Value("Conditions").empty()||EqualToken(child.Value("Conditions"),"None")))
        {
            if(weaponFound) throw std::invalid_argument("Duplicate default WeaponSet"); weaponFound=true;
            for(const auto &field:child.fields) if(EqualToken(field.key,"Weapon"))
            {const auto words=Tokens(field.value); if(words.size()==2&&EqualToken(words[0],"PRIMARY")) result.weaponName=words[1];}
        }
        if(EqualToken(child.kind,"Behavior"))
        {
            const auto words=Tokens(child.argument); if(words.empty()) throw std::invalid_argument("Unnamed Behavior");
            if(!captureTemplate.empty() && child.Value("SpecialPowerTemplate")==captureTemplate &&
                (EqualToken(words[0],"SpecialAbility")||EqualToken(words[0],"SpecialAbilityUpdate")||EqualToken(words[0],"UnpauseSpecialPowerUpgrade")))
                continue; // The exact resolved chain was decoded and its omissions reported above.
            if(EqualToken(words[0],"StealthUpdate"))
            {
                if (stealthFound) throw std::invalid_argument("Duplicate StealthUpdate");
                stealthFound=true; RejectUnsupportedStealthFields(child);
                const auto forbidden=DecodeStealthForbiddenConditions(child.Value("StealthForbiddenConditions"));
                definition.concealment=engine::gameplay::concealment::ConcealmentDefinition{
                    DecodeMilliseconds(child.Require("StealthDelay"),"StealthDelay",false), true};
                definition.stealthPolicy=generalszh::concealment::StealthPolicy{
                    DecodeBool(child.Value("InnateStealth"),"InnateStealth",true), forbidden};
            }
            else if(EqualToken(words[0],"StealthDetectorUpdate"))
            {
                if (detectorFound) throw std::invalid_argument("Duplicate StealthDetectorUpdate");
                detectorFound=true; RejectUnsupportedDetectorFields(child);
                const auto detectionRange=child.Value("DetectionRange");
                const auto authoredRange=detectionRange.empty() || ScaledDecimal(detectionRange,1'000'000)==0
                    ? object.Value("VisionRange") : detectionRange;
                if (authoredRange.empty() || ScaledDecimal(authoredRange,1'000'000)==0)
                    throw std::invalid_argument("StealthDetectorUpdate requires a positive DetectionRange or VisionRange");
                const auto range=RadiusCells(authoredRange,scale);
                if (!range) throw std::invalid_argument("Stealth detector range rounds to zero cells");
                definition.detection=engine::gameplay::concealment::DetectionDefinition{
                    DecodeMilliseconds(child.Require("DetectionRate"),"DetectionRate",true), range,
                    DecodeBool(child.Value("CanDetectWhileContained"),"CanDetectWhileContained"),
                    DecodeBool(child.Value("InitiallyDisabled"),"InitiallyDisabled")};
            }
            else if(EqualToken(words[0],"StealthUpgrade"))
                throw std::invalid_argument("StealthUpgrade requires an unsupported runtime grant path");
            else if(EqualToken(words[0],"AIUpdateInterface"))
                for(const auto flag:Tokens(child.Value("AutoAcquireEnemiesWhenIdle")))
                {
                    if(EqualToken(flag,"Yes")) definition.acquisition.enabled=true;
                    else if(EqualToken(flag,"No")) definition.acquisition.enabled=false;
                    else if(EqualToken(flag,"Attack_Buildings")) definition.acquisition.attackBuildings=true;
                    else if(EqualToken(flag,"Stealthed")||EqualToken(flag,"NotWhileAttacking")) result.omittedBehaviors.push_back("AutoAcquire flag: "+std::string(flag));
                    else throw std::invalid_argument("Unknown AutoAcquireEnemiesWhenIdle flag");
                }
            else if(EqualToken(words[0],"SupplyTruckAIUpdate"))
            {
                if(definition.harvest) throw std::invalid_argument("Duplicate supply truck behavior");
                const auto boxes=ScaledDecimal(child.Require("MaxBoxes"),1);
                const auto pickup=ScaledDecimal(child.Require("SupplyWarehouseActionDelay"),1000000);
                const auto unload=ScaledDecimal(child.Require("SupplyCenterActionDelay"),1000000);
                if(!boxes||boxes>UINT32_MAX||pickup>static_cast<std::uint64_t>((std::numeric_limits<std::int64_t>::max)())
                    ||unload>static_cast<std::uint64_t>((std::numeric_limits<std::int64_t>::max)()))
                    throw std::invalid_argument("Invalid supply truck capacity or duration");
                definition.harvest=engine::gameplay::rts::harvesting::HarvestConfig{static_cast<std::uint32_t>(boxes),
                    engine::time::Duration{static_cast<std::int64_t>(pickup)},engine::time::Duration{static_cast<std::int64_t>(unload)}};
            }
            else if(EqualToken(words[0],"DozerAIUpdate")||EqualToken(words[0],"WorkerAIUpdate"))
            {
                definition.builder=true;
                const auto authoredRate=child.Value("RepairHealthPercentPerSecond");
                if(!authoredRate.empty())
                {
                    if(manualRepairFound||authoredRate.back()!='%')
                        throw std::invalid_argument("Builder repair rate requires one percent declaration");
                    auto percent=authoredRate; percent.remove_suffix(1);
                    const auto scaled=ScaledDecimal(percent,1'000'000);
                    constexpr std::uint64_t percentDenominator=100'000'000;
                    if(!scaled||scaled>percentDenominator||scaled>static_cast<std::uint64_t>((std::numeric_limits<std::uint32_t>::max)()))
                        throw std::invalid_argument("Builder repair percent must be within (0,100]");
                    const auto divisor=std::gcd(scaled,percentDenominator);
                    definition.manualRepair=engine::gameplay::rts::repair::RepairDefinition{
                        static_cast<std::uint32_t>(scaled/divisor),static_cast<std::uint32_t>(percentDenominator/divisor),
                        engine::time::Duration{std::chrono::seconds{1}},{},{},
                        engine::gameplay::rts::repair::RepairIntervalPolicy::SimulationStep};
                    manualRepairFound=true;
                    // Construction search, docking, and the worker's second
                    // supply brain remain intentionally outside this slice.
                    result.omittedBehaviors.emplace_back(std::string(words[0])+": construction/search/docking remains omitted");
                }
                else result.omittedBehaviors.emplace_back(words[0]);
            }
            else if(EqualToken(words[0],"PoisonedBehavior"))
            {
                const auto interval=ScaledDecimal(child.Require("PoisonDamageInterval"),1000000),duration=ScaledDecimal(child.Require("PoisonDuration"),1000000);
                if(!interval||!duration||interval>static_cast<std::uint64_t>((std::numeric_limits<std::int64_t>::max)())||duration>static_cast<std::uint64_t>((std::numeric_limits<std::int64_t>::max)()))
                    throw std::invalid_argument("Poison timing must be positive representable durations");
                definition.poison={engine::time::Duration{static_cast<std::int64_t>(interval)},engine::time::Duration{static_cast<std::int64_t>(duration)}};
            }
            else if(EqualToken(words[0],"TransportContain"))
            { transportAuthored=true; transportContainAuthored=true; } // Full-object validation below.
            else if(EqualToken(words[0],"LifetimeUpdate"))
            {
                if(lifetimeFound) throw std::invalid_argument("Duplicate LifetimeUpdate");
                lifetimeFound=true;
                const auto minimum=child.Value("MinLifetime"),maximum=child.Value("MaxLifetime");
                const auto lower=minimum.empty()?0:ScaledDecimal(minimum,1000000);
                const auto upper=maximum.empty()?0:ScaledDecimal(maximum,1000000);
                if(lower>upper || upper>static_cast<std::uint64_t>((std::numeric_limits<std::int64_t>::max)()))
                    throw std::invalid_argument("Invalid lifetime range");
                if(lower!=upper) result.omittedBehaviors.emplace_back("LifetimeUpdate: randomized lifetime requires deterministic RNG binding");
                else
                {
                    // Reference duration parsing rounds up to 30-Hz frames;
                    // calcSleepDelay clamps the sampled result to one frame.
                    const auto frames=std::max(std::uint64_t{1},upper/1000000000*30+(upper%1000000000*30+999999999)/1000000000);
                    const auto nanos=frames/30*1000000000+frames%30*1000000000/30;
                    if(nanos>static_cast<std::uint64_t>((std::numeric_limits<std::int64_t>::max)()))
                        throw std::out_of_range("Quantized lifetime duration overflow");
                    definition.lifetime=engine::time::Duration{static_cast<std::int64_t>(nanos)};
                }
                for(const auto &field:child.fields)
                    if(!EqualToken(field.key,"MinLifetime")&&!EqualToken(field.key,"MaxLifetime"))
                        result.omittedBehaviors.push_back("LifetimeUpdate field: "+field.key);
            }
            else if(EqualToken(words[0],"AutoHealBehavior"))
            {
                auto radius=child.Value("Radius");
                if(!radius.empty() && (radius.back()=='f'||radius.back()=='F')) radius.remove_suffix(1);
                const auto wholePlayer=child.Value("AffectsWholePlayer");
                if(!wholePlayer.empty()&&!EqualToken(wholePlayer,"Yes")&&!EqualToken(wholePlayer,"No"))
                    throw std::invalid_argument("Invalid auto-heal AffectsWholePlayer boolean");
                if((!radius.empty()&&ScaledDecimal(radius,1000000)!=0)||EqualToken(wholePlayer,"Yes"))
                {
                    result.omittedBehaviors.push_back("AutoHealBehavior "+std::string(child.argument)+": area/player-wide healing");
                    continue;
                }
                auto decoded=DecodeAutoHeal(child,scale.healthQuanta);
                bool unsupportedPolicy=false;
                for(const auto &field:decoded.unimplementedFields)
                {
                    result.omittedBehaviors.push_back("AutoHealBehavior "+std::string(child.argument)+": "+field);
                    // Presentation omissions do not affect self-healing. Unknown
                    // gameplay/upgrade fields cannot safely become active healing.
                    if(!EqualToken(field,"RadiusParticleSystemName")&&!EqualToken(field,"UnitHealPulseParticleSystemName")
                        &&!EqualToken(field,"FXListUpgrade")) unsupportedPolicy=true;
                }
                if(unsupportedPolicy) continue;
                if(result.regeneration) throw std::invalid_argument("Multiple supported self-heal modules require independent bindings");
                result.regeneration=std::move(decoded);
            }
            else result.omittedBehaviors.emplace_back(words[0]);
        }
    }
    if(transportAuthored)
    {
        auto decoded=DecodeTransport(objectText,objectName);
        result.transport=decoded.carrier;
        result.initialPayload=decoded.initialPayload;
        if(decoded.passenger && (definition.builder||definition.harvest))
            result.omittedBehaviors.emplace_back("Passenger work-task interruption is not yet supported");
        else definition.passenger=decoded.passenger;
        result.omittedBehaviors.insert(result.omittedBehaviors.end(),decoded.omissions.begin(),decoded.omissions.end());
    }
    const bool infantry=std::any_of(kindOf.begin(),kindOf.end(),[](const auto token){return EqualToken(token,"INFANTRY");});
    const bool noGarrison=std::any_of(kindOf.begin(),kindOf.end(),[](const auto token){return EqualToken(token,"NO_GARRISON");});
    if(infantry&&!noGarrison&&!definition.builder&&!definition.harvest&&!transportContainAuthored)
        definition.garrisonable=true;
    else if(noGarrison)
        result.omittedBehaviors.emplace_back("Garrison enrollment omitted: NO_GARRISON");
    else if(infantry&&(definition.builder||definition.harvest))
        result.omittedBehaviors.emplace_back("Garrison enrollment omitted: work-capable infantry is outside this bounded slice");
    else if(transportContainAuthored)
        result.omittedBehaviors.emplace_back("Garrison enrollment omitted: a garrison carrier cannot also be a passenger");
    if(!bodyFound||!definition.health) throw std::invalid_argument("Missing positive initial health");
    if(!armorSetFound)
    {
        if(conditionalArmor) throw std::invalid_argument("Conditional ArmorSet requires a supported default ArmorSet");
        result.omittedBehaviors.emplace_back("No ArmorSet in supplied resolved Object: explicit unarmored identity; inheritance not resolved");
    }
    if(weaponFound&&result.weaponName.empty()) throw std::invalid_argument("Default WeaponSet has no supported PRIMARY declaration");
    for(const auto &field:object.fields) if(EqualToken(field.key,"Locomotor"))
    {const auto words=Tokens(field.value);if(words.size()==2&&EqualToken(words[0],"SET_NORMAL")) result.locomotorName=words[1];}
    if(result.locomotorName.empty()) throw std::invalid_argument("Missing normal locomotor");
    const auto locomotor=ReadNamedBlock(locomotorText,"Locomotor",result.locomotorName);
    const auto speed=ScaledDecimal(locomotor.Require("Speed"),1000000)/(std::uint64_t{scale.worldUnitsPerCell}*1000000);
    if(!speed||speed>(std::numeric_limits<std::uint32_t>::max)()) throw std::out_of_range("Locomotor speed cannot fit current grid movement scale");
    definition.cellsPerSecond=static_cast<std::uint32_t>(speed);
    if(!result.weaponName.empty()&&!EqualToken(result.weaponName,"NONE"))
    {
        const auto weapon=ReadNamedBlock(weaponText,"Weapon",result.weaponName);
        const auto authoredType=weapon.Value("DamageType");
        const auto typeName=authoredType.empty()?std::string_view{"EXPLOSION"}:authoredType;
        const auto type=std::find_if(ZeroHourDamageTypes.begin(),ZeroHourDamageTypes.end(),
            [&](const auto &entry){return EqualToken(entry.name,typeName);});
        if(type==ZeroHourDamageTypes.end()) throw std::invalid_argument("Unknown weapon DamageType");
        if(type->policy.armor==engine::gameplay::combat::damage::ArmorApplication::Unsupported)
            throw std::invalid_argument("Weapon DamageType requires unsupported nonordinary damage semantics");
        definition.weapon.damageType=type->id; // Explicit mapping, including source default EXPLOSION.
        definition.weapon.damage=ScaledDecimal(weapon.Require("PrimaryDamage"),scale.healthQuanta);
        const auto secondaryDamage=weapon.Value("SecondaryDamage");
        if(!secondaryDamage.empty()) definition.weapon.secondaryDamage=ScaledDecimal(secondaryDamage,scale.healthQuanta);
        const auto primaryRadius=weapon.Value("PrimaryDamageRadius"),secondaryRadius=weapon.Value("SecondaryDamageRadius");
        if(!primaryRadius.empty()) definition.weapon.primaryRadiusCells=RadiusCells(primaryRadius,scale);
        if(!secondaryRadius.empty()) definition.weapon.secondaryRadiusCells=RadiusCells(secondaryRadius,scale);
        const auto affects=weapon.Value("RadiusDamageAffects");
        if(!affects.empty()) definition.weapon.radiusDamageAffects=DecodeRadiusDamageAffects(affects);
        const auto damageAtSelf=weapon.Value("DamageDealtAtSelfPosition");
        if(!damageAtSelf.empty())
        {
            if(EqualToken(damageAtSelf,"Yes"))
                throw std::invalid_argument("DamageDealtAtSelfPosition=Yes is unsupported by modern projectile impacts");
            if(!EqualToken(damageAtSelf,"No")) throw std::invalid_argument("Invalid DamageDealtAtSelfPosition boolean");
        }
        const auto range=ScaledDecimal(weapon.Require("AttackRange"),1000000)/(std::uint64_t{scale.worldUnitsPerCell}*1000000);
        const auto clipText=weapon.Value("ClipSize");
        const auto clip=clipText.empty()?0:ScaledDecimal(clipText,1);
        if(range>(std::numeric_limits<std::uint32_t>::max)()||clip>(std::numeric_limits<std::uint32_t>::max)())
            throw std::invalid_argument("Weapon range/clip exceeds supported capacity");
        definition.weapon.rangeCells=static_cast<std::uint32_t>(range); definition.weapon.clipSize=static_cast<std::uint32_t>(clip);
        // WeaponTemplate defaults ClipSize to zero; reloadWithBonus treats it as
        // effectively unlimited. Keep that legacy sentinel at this boundary.
        definition.weapon.ammunition=clip?engine::gameplay::combat::AmmunitionPolicy::Limited:engine::gameplay::combat::AmmunitionPolicy::Unlimited;
        const auto shotText=weapon.Value("DelayBetweenShots"),reloadText=weapon.Value("ClipReloadTime"),preAttackText=weapon.Value("PreAttackDelay");
        const auto shot=shotText.empty()?0:ScaledDecimal(shotText,1000000),reload=reloadText.empty()?0:ScaledDecimal(reloadText,1000000),
            preAttack=preAttackText.empty()?0:ScaledDecimal(preAttackText,1000000);
        if(shot>static_cast<std::uint64_t>((std::numeric_limits<std::int64_t>::max)())||
            reload>static_cast<std::uint64_t>((std::numeric_limits<std::int64_t>::max)())||
            preAttack>static_cast<std::uint64_t>((std::numeric_limits<std::int64_t>::max)()))
            throw std::out_of_range("Weapon duration overflow");
        definition.weapon.shotInterval=engine::time::Duration{static_cast<std::int64_t>(shot)};
        definition.weapon.reloadTime=engine::time::Duration{static_cast<std::int64_t>(reload)};
        definition.weapon.preAttackDelay=engine::time::Duration{static_cast<std::int64_t>(preAttack)};
        const auto preAttackType=weapon.Value("PreAttackType");
        if(!preAttackType.empty())
        {
            if(EqualToken(preAttackType,"PER_SHOT")) definition.weapon.preAttackType=engine::gameplay::combat::PrefirePolicy::PerShot;
            else if(EqualToken(preAttackType,"PER_ATTACK")) definition.weapon.preAttackType=engine::gameplay::combat::PrefirePolicy::PerAttack;
            else if(EqualToken(preAttackType,"PER_CLIP")) definition.weapon.preAttackType=engine::gameplay::combat::PrefirePolicy::PerClip;
            else throw std::invalid_argument("Unsupported PreAttackType policy");
        }
        const auto reloadPolicy=weapon.Value("AutoReloadsClip");
        if(!reloadPolicy.empty())
        {
            if(EqualToken(reloadPolicy,"Yes")) definition.weapon.autoReload=true;
            else if(EqualToken(reloadPolicy,"No")) definition.weapon.autoReload=false;
            else throw std::invalid_argument("Unsupported AutoReloadsClip policy");
        }
        // Explicit temporary delivery policy: the current simulation only supports
        // delayed target-following projectiles, not original instant hits/missiles.
        definition.weapon.flightTime=std::chrono::milliseconds{50};
    }
    return result;
}
// Validate catalog identities before publishing immutable definitions to the game.
inline void ValidateUnitIdentities(std::span<const DecodedUnit> units)
{
    for(std::size_t i=0;i!=units.size();++i)
    {
        if(units[i].name.empty()) throw std::invalid_argument("Unit definition name cannot be empty");
        if(units[i].definition.key!=DefinitionKey(units[i].name))
            throw std::invalid_argument("Unit definition key does not match its canonical name");
        for(std::size_t j=0;j<i;++j)
        {
            if(units[i].name==units[j].name) throw std::invalid_argument("Duplicate canonical unit definition name");
            if(units[i].definition.key==units[j].definition.key)
                throw std::invalid_argument("Duplicate/colliding unit definition key");
        }
    }
}
inline void ValidateUnitKeys(const std::vector<DecodedUnit> &units)
{
    ValidateUnitIdentities(std::span<const DecodedUnit>{units.data(),units.size()});
}
// Resolve all InitialPayload names in one explicit whole-catalog pass. The
// decoder cannot do this: it intentionally receives one resolved Object only.
// No definition is published until every reference and capacity check passes.
inline void BindDecodedUnitCatalog(std::span<DecodedUnit> units)
{
    ValidateUnitIdentities(std::span<const DecodedUnit>{units.data(),units.size()});
    std::vector<std::optional<production::InitialPayloadDefinition>> resolved(units.size());
    for(std::size_t index=0;index!=units.size();++index)
    {
        const auto &unit=units[index];
        if(!unit.initialPayload) continue;
        if(unit.definition.kind!=production::EntryKind::Unit||!unit.transport||!unit.transport->slots)
            throw std::invalid_argument("InitialPayload requires a supported transport carrier");
        const auto found=std::find_if(units.begin(),units.end(),[&](const auto &candidate) {
            return candidate.name==unit.initialPayload->passengerName;
        });
        if(found==units.end())
            throw std::invalid_argument("InitialPayload references a missing exact unit definition: "+unit.initialPayload->passengerName);
        if(found->definition.kind!=production::EntryKind::Unit||!found->definition.passenger||
            !found->definition.passenger->slots||found->definition.builder||found->definition.harvest||
            found->transport||found->initialPayload||found->definition.initialPayload)
            throw std::invalid_argument("InitialPayload passenger is not a supported non-nested ground passenger");
        if(unit.initialPayload->count > unit.transport->slots / found->definition.passenger->slots)
            throw std::invalid_argument("InitialPayload exceeds carrier capacity");
        resolved[index]=production::InitialPayloadDefinition{found->definition.key,unit.initialPayload->count};
    }
    for(std::size_t index=0;index!=units.size();++index)
        if(resolved[index]) units[index].definition.initialPayload=resolved[index];
}
}
