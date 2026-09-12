module;
#include <algorithm>
#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
export module games.generalszh.adapters.content.capture.capture_definition;
export import games.generalszh.adapters.content.ini.named_block;
export import games.generalszh.adapters.content.upgrades.upgrade_definition;
export import games.generalszh.gameplay.capture.definitions.capture_definition;

namespace generalszh::content::capture_detail
{
bool Boolean(std::string_view value)
{
    if(EqualToken(value,"Yes")) return true;
    if(EqualToken(value,"No")) return false;
    throw std::invalid_argument("Capture boolean requires Yes or No");
}
engine::time::Duration Milliseconds(std::string_view value)
{
    value=Trim(value);
    if(value.empty()||value.find_first_not_of("0123456789")!=value.npos)
        throw std::invalid_argument("Capture duration requires unsigned integer milliseconds");
    const auto milliseconds=ScaledDecimal(value,1);
    if(milliseconds>UINT32_MAX) throw std::out_of_range("Capture duration exceeds reference unsigned integer");
    return engine::time::Duration{static_cast<std::int64_t>(milliseconds)*1000000};
}
}
export namespace generalszh::content
{
enum class CapturePermission { Unresolved, Denied, Allowed };
struct DecodedCapture
{
    std::optional<capture::CaptureDefinition> definition;
    bool enabled{};
    std::string specialPower;
    std::vector<std::string> omissions;
};
// Actor capability only: no Capturable target inference, upgrade grant, catalog
// index assignment, file loading or runtime dispatch. Caller supplies resolved
// Object and SpecialPower blocks plus explicit command/content permission.
inline DecodedCapture DecodeCapture(std::string_view resolvedObjectText,std::string_view specialPowerText,
    std::string_view objectName,CapturePermission permission,std::span<const UpgradeIdentity> upgradeIdentities={})
{
    using namespace capture_detail;
    ValidateUpgradeIdentities(upgradeIdentities);
    const auto object=ReadNamedBlock(resolvedObjectText,"Object",objectName);
    DecodedCapture result;bool supported=true;std::vector<std::string_view> activationTriggers;
    auto omit=[&](std::string reason){result.omissions.push_back(std::move(reason));};
    auto reject=[&](std::string reason){supported=false;omit(std::move(reason));};
    omit("Resolved actor Object only: inheritance, command permission and runtime upgrades are not inferred");
    if(permission!=CapturePermission::Allowed)
    {omit(permission==CapturePermission::Denied?"Capture permission denied":"Capture permission unresolved");return result;}
    const auto kinds=Tokens(object.Value("KindOf"));
    auto kind=[&](std::string_view name){return std::any_of(kinds.begin(),kinds.end(),[&](auto value){return EqualToken(value,name);});};
    if(!kind("INFANTRY")||kind("VEHICLE")||kind("AIRCRAFT")||kind("STRUCTURE")||kind("DOZER")||kind("HARVESTER"))
        reject("Capture actor requires supported ground INFANTRY");
    if(!object.Value("BuildVariations").empty()) reject("BuildVariations are unresolved");
    const IniBlock *ability=nullptr,*update=nullptr;IniBlock power;
    for(const auto &child:object.children)
    {
        if(!EqualToken(child.kind,"Behavior")) continue;
        const auto words=Tokens(child.argument);
        if(words.empty()) {reject("Malformed Behavior");continue;}
        if(!EqualToken(words[0],"SpecialAbility")) continue;
        if(words.size()!=2||child.Value("SpecialPowerTemplate").empty()) {reject("SpecialAbility lacks exact template binding");continue;}
        const auto name=child.Value("SpecialPowerTemplate");IniBlock candidate;
        try {candidate=ReadNamedBlock(specialPowerText,"SpecialPower",name);}
        catch(const std::invalid_argument &error) {reject("Unresolved SpecialPower "+std::string(name)+": "+error.what());continue;}
        const auto type=candidate.Value("Enum");
        if(type.empty()) {reject("SpecialPower Enum is missing: "+std::string(name));continue;}
        if(!EqualToken(type,"SPECIAL_INFANTRY_CAPTURE_BUILDING"))
        {omit("Other SpecialAbility type not decoded: "+std::string(type));continue;}
        if(ability) {reject("Multiple infantry capture abilities are ambiguous");continue;}
        ability=&child;power=std::move(candidate);result.specialPower=name;
    }
    if(!ability) {omit("No resolved SPECIAL_INFANTRY_CAPTURE_BUILDING ability");return result;}
    for(const auto &child:object.children)
    {
        if(!EqualToken(child.kind,"Behavior")) continue;
        const auto words=Tokens(child.argument);if(words.empty()) continue;
        if(EqualToken(words[0],"SpecialAbilityUpdate") && child.Value("SpecialPowerTemplate")==result.specialPower)
        {
            if(update||words.size()!=2) reject("Duplicate or malformed matching SpecialAbilityUpdate");
            else update=&child;
        }
        if(EqualToken(words[0],"UnpauseSpecialPowerUpgrade"))
        {
            if(words.size()!=2||child.Value("SpecialPowerTemplate")!=result.specialPower||child.Value("TriggeredBy").empty())
                reject("UnpauseSpecialPowerUpgrade binding is unresolved or mismatched");
            else activationTriggers.push_back(child.Value("TriggeredBy"));
            for(const auto &field:child.fields)
                if(!EqualToken(field.key,"SpecialPowerTemplate")&&!EqualToken(field.key,"TriggeredBy"))
                    reject("Unsupported UnpauseSpecialPowerUpgrade field: "+field.key);
            if(!child.children.empty()) reject("Nested UnpauseSpecialPowerUpgrade configuration is unsupported");
        }
    }
    if(!update) {omit("Missing matching SpecialAbilityUpdate");return result;}
    bool paused=false,updateStartsAttack=false;
    capture::CaptureDefinition definition;
    for(const auto &field:ability->fields)
    {
        const auto key=std::string_view(field.key),value=Trim(field.value);
        if(EqualToken(key,"SpecialPowerTemplate")) continue;
        if(EqualToken(key,"StartsPaused")) paused=Boolean(value);
        else if(EqualToken(key,"UpdateModuleStartsAttack")) updateStartsAttack=Boolean(value);
        else if(EqualToken(key,"StartsReady")) definition.startsReady=Boolean(value);
        else if(EqualToken(key,"InitiateSound")) omit("Presentation omission: InitiateSound");
        else if(EqualToken(key,"ScriptedSpecialPowerOnly")) {if(Boolean(value)) reject("Script-only capture is unsupported");}
        else reject("Unsupported SpecialAbility field: "+field.key);
    }
    if(!updateStartsAttack) reject("Capture requires UpdateModuleStartsAttack = Yes");
    for(const auto &field:update->fields)
    {
        const auto key=std::string_view(field.key),value=Trim(field.value);
        if(EqualToken(key,"SpecialPowerTemplate")) continue;
        if(EqualToken(key,"UnpackTime")) definition.unpack=Milliseconds(value);
        else if(EqualToken(key,"PreparationTime")) definition.preparation=Milliseconds(value);
        else if(EqualToken(key,"PackTime")) definition.recovery=Milliseconds(value);
        else if(EqualToken(key,"StartAbilityRange")||EqualToken(key,"AbilityAbortRange")||EqualToken(key,"AwardXPForTriggering"))
        {(void)ScaledDecimal(value,1000000);omit("Bounded capture omission: "+field.key+" = "+field.value);}
        else if(EqualToken(key,"DoCaptureFX")) {(void)Boolean(value);omit("Presentation omission: DoCaptureFX");}
        else if(EqualToken(key,"PackUnpackVariationFactor")) {if(ScaledDecimal(value,1000000)!=0) reject("Randomized pack/unpack is unsupported");}
        else if(EqualToken(key,"PackSound")||EqualToken(key,"UnpackSound")||EqualToken(key,"PrepSoundLoop")||EqualToken(key,"TriggerSound"))
            omit("Presentation omission: "+field.key);
        else reject("Unsupported SpecialAbilityUpdate field: "+field.key);
    }
    for(const auto &field:power.fields)
    {
        if(EqualToken(field.key,"Enum")) continue;
        if(EqualToken(field.key,"ReloadTime")) definition.recharge=Milliseconds(field.value);
        else if(EqualToken(field.key,"PublicTimer")) {(void)Boolean(Trim(field.value));omit("Presentation omission: PublicTimer");}
        else reject("Unsupported capture SpecialPower field: "+field.key);
    }
    if(!ability->children.empty()||!update->children.empty()||!power.children.empty()) reject("Nested capture configuration is unsupported");
    if(activationTriggers.size()>1)
    {
        omit("Multiple UnpauseSpecialPowerUpgrade triggers are ambiguous; no capture activation is granted");
        activationTriggers.clear();
    }
    if(activationTriggers.size()==1)
    {
        const auto trigger=activationTriggers.front();
        const auto *identity=FindUpgradeIdentity(trigger,upgradeIdentities);
        if(!identity)
            omit("Capture upgrade trigger has no explicit upgrade identity: "+std::string(trigger));
        else if(identity->scope!=UpgradeScope::Player)
            omit("Object-scoped capture upgrade is unsupported: "+std::string(trigger));
        else
            definition.activationUpgrade=capture::CaptureUpgradeBinding{identity->definition};
    }
    // ReloadTime is consumed by CaptureSystem's preparation lifecycle, not packing.
    if(paused&&!definition.activationUpgrade)
        omit("StartsPaused remains disabled until a supported player upgrade identity is bound");
    omit("Same-cell modern capture omits approach/range, LOS/facing, XP rewards and capture FX");
    omit("Typed milliseconds are finalized at the modern step without reference frame round-trip; unpack/preparation combined rounding may differ");
    omit("Target permission is separate: IMMUNE_TO_CAPTURE and relationship rules cannot be inferred solely from CAPTURABLE KindOf");
    if(supported) {result.definition=definition;result.enabled=!paused;}
    return result;
}
}
