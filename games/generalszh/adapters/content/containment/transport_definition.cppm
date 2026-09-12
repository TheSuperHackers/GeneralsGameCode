module;
#include <algorithm>
#include <cstdint>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
export module games.generalszh.adapters.content.containment.transport_definition;
export import games.generalszh.adapters.content.ini.named_block;
export import engine.gameplay.containment.definitions.transport_definition;
export import engine.gameplay.containment.components.passenger_membership;

namespace generalszh::content::transport_detail
{
bool HasWord(std::string_view words,std::string_view expected)
{
    const auto tokens=Tokens(words);
    return std::any_of(tokens.begin(),tokens.end(),[&](auto token){return EqualToken(token,expected);});
}
bool Contains(std::string_view text,std::string_view part)
{
    for(std::size_t i=0;i+part.size()<=text.size();++i)
        if(EqualToken(text.substr(i,part.size()),part)) return true;
    return false;
}
bool Boolean(std::string_view value)
{
    if(EqualToken(value,"Yes")) return true;
    if(EqualToken(value,"No")) return false;
    throw std::invalid_argument("Transport boolean requires Yes or No");
}
std::uint32_t Integer(std::string_view text,std::uint32_t maximum)
{
    text=Trim(text);
    if(text.empty()||text.find_first_not_of("0123456789")!=text.npos)
        throw std::invalid_argument("Transport integer requires unsigned decimal digits");
    const auto value=ScaledDecimal(text,1);
    if(value>maximum) throw std::out_of_range("Transport integer exceeds source/runtime range");
    return static_cast<std::uint32_t>(value);
}
}
export namespace generalszh::content
{
struct InitialPayloadReference
{
    std::string passengerName;
    std::uint32_t count{1};
};
struct DecodedTransport
{
    std::optional<engine::gameplay::containment::TransportDefinition> carrier;
    std::optional<engine::gameplay::containment::PassengerSlots> passenger;
    // This single-object adapter deliberately leaves passengerName unresolved.
    // Complete-catalog binding belongs to the content/session boundary.
    std::optional<InitialPayloadReference> initialPayload;
    std::vector<std::string> omissions;
};
// Resolved Object only. In particular GLAVehicleTechnical is a variation shell;
// the inspected TransportContain lives on GLAVehicleTechnicalChassisOne.
// This decoder does not select variants, resolve inheritance or register systems.
inline DecodedTransport DecodeTransport(std::string_view resolvedObjectText,std::string_view objectName)
{
    using namespace transport_detail;
    const auto object=ReadNamedBlock(resolvedObjectText,"Object",objectName);
    DecodedTransport result;
    auto omit=[&](std::string reason){result.omissions.push_back(std::move(reason));};
    omit("Resolved Object only: inheritance and BuildVariations are not resolved");
    const auto kinds=object.Value("KindOf");
    const bool aircraft=HasWord(kinds,"AIRCRAFT");
    bool ground= !aircraft && !HasWord(kinds,"STRUCTURE");
    bool passengerSupported=ground&&HasWord(kinds,"INFANTRY")&&!HasWord(kinds,"VEHICLE")
        &&!HasWord(kinds,"DOZER")&&!HasWord(kinds,"HARVESTER")&&!HasWord(kinds,"PORTABLE_STRUCTURE");
    bool carrierSupported=ground&&HasWord(kinds,"VEHICLE");
    const IniBlock *transport=nullptr;
    bool anyContain=false;
    for(const auto &field:object.fields)
        if(EqualToken(field.key,"BuildVariations"))
        {
            passengerSupported=false;carrierSupported=false;
            omit("BuildVariations shell cannot activate containment");
        }
    for(const auto &child:object.children)
    {
        if(!EqualToken(child.kind,"Behavior")) continue;
        const auto words=Tokens(child.argument);
        if(words.empty()) {passengerSupported=false;carrierSupported=false;omit("Malformed Behavior");continue;}
        if(Contains(words[0],"Contain"))
        {
            anyContain=true;passengerSupported=false;
            if(!EqualToken(words[0],"TransportContain")||words.size()!=2||transport)
            {carrierSupported=false;omit("Unsupported or duplicate containment: "+child.argument);}
            else transport=&child;
        }
        if(Contains(words[0],"Dozer")||Contains(words[0],"Supply")||Contains(words[0],"Harvest"))
        {passengerSupported=false;omit("Builder/harvester passenger is unsupported: "+child.argument);}
        if(Contains(words[0],"JetAI")||Contains(words[0],"ChinookAI")||Contains(words[0],"HelicopterAI"))
        {carrierSupported=false;passengerSupported=false;omit("Aircraft AI containment is unsupported");}
    }
    if(anyContain) omit("Nested containers cannot enroll as passengers");
    // ThingTemplate.cpp parseUnsignedByte, constructor default 0. Never use the
    // generic PassengerSlots default of 1 to fabricate transportability.
    std::uint32_t slots=0;
    for(const auto &field:object.fields)
        if(EqualToken(field.key,"TransportSlotCount")) slots=Integer(field.value,UINT8_MAX);
    if(passengerSupported&&slots) result.passenger=engine::gameplay::containment::PassengerSlots{slots};
    else omit("Passenger not enrolled: requires ground INFANTRY, positive TransportSlotCount and no unsupported/nested capability");
    if(!transport) {omit("No TransportContain module; carrier not enrolled");return result;}
    // TransportContain defaults: Slots=0, ExitDelay=0, AllowInsideKindOf=INFANTRY.
    // OpenContain defaults: damage=0, firing/turret/weapon-bonus=false.
    engine::gameplay::containment::TransportDefinition definition;
    for(const auto &field:transport->fields)
    {
        const auto key=std::string_view(field.key),value=Trim(field.value);
        auto reject=[&] {carrierSupported=false;omit("Unsupported TransportContain field/value: "+field.key+" = "+field.value);};
        if(EqualToken(key,"Slots")) definition.slots=Integer(value,INT32_MAX);
        else if(EqualToken(key,"ExitDelay"))
        {
            // INI::parseDurationUnsignedInt reads milliseconds and ceil-converts
            // to reference frames. Preserve typed milliseconds; finalized modern
            // step performs its own ceiling, not a hardcoded 30-Hz round trip.
            definition.exitDelay=engine::time::Duration{static_cast<std::int64_t>(Integer(value,UINT32_MAX))*1000000};
        }
        else if(EqualToken(key,"DamagePercentToUnits"))
        {
            auto percent=value;if(!percent.empty()&&percent.back()=='%') percent=Trim(percent.substr(0,percent.size()-1));
            constexpr std::uint64_t denominator=100000000;
            const auto numerator=ScaledDecimal(percent,1000000);
            if(numerator>denominator) throw std::out_of_range("Transport death damage must be within 0..100 percent");
            const auto divisor=std::gcd(numerator,denominator);
            definition.deathDamageNumerator=static_cast<std::uint32_t>(numerator/divisor);
            definition.deathDamageDenominator=static_cast<std::uint32_t>(denominator/divisor);
        }
        else if(EqualToken(key,"AllowInsideKindOf"))
        {const auto allowed=Tokens(value);if(allowed.size()!=1||!EqualToken(allowed[0],"INFANTRY")) reject();}
        else if(EqualToken(key,"ForbidInsideKindOf"))
        {if(!EqualToken(value,"NONE")) reject();}
        else if(EqualToken(key,"InitialPayload"))
        {
            const auto words=Tokens(value);
            if(words.empty()||words.size()>2||words[0].empty())
                throw std::invalid_argument("InitialPayload requires a passenger name and optional count");
            if(result.initialPayload)
            {
                reject();
                continue;
            }
            const auto count=words.size()==1?1u:Integer(words[1],UINT32_MAX);
            result.initialPayload=InitialPayloadReference{std::string(words[0]),count};
            omit("InitialPayload passenger name remains unresolved until complete unit-catalog binding");
        }
        else if(EqualToken(key,"PassengersAllowedToFire")||EqualToken(key,"PassengersInTurret")
            ||EqualToken(key,"WeaponBonusPassedToPassengers")||EqualToken(key,"ArmedRidersUpgradeMyWeaponSet")
            ||EqualToken(key,"KeepContainerVelocityOnExit")||EqualToken(key,"OrientLikeContainerOnExit")
            ||EqualToken(key,"DestroyRidersWhoAreNotFreeToExit")||EqualToken(key,"DelayExitInAir"))
        {if(Boolean(value)) reject();}
        else if(EqualToken(key,"AllowAlliesInside")) {if(!Boolean(value)) reject();}
        else if(EqualToken(key,"AllowEnemiesInside")||EqualToken(key,"AllowNeutralInside"))
        { (void)Boolean(value); } // TransportContain additionally requires same owner.
        else if(EqualToken(key,"ScatterNearbyOnExit")||EqualToken(key,"GoAggressiveOnExit")
            ||EqualToken(key,"ResetMoodCheckTimeOnExit")||EqualToken(key,"BurnedDeathToUnits"))
        { (void)Boolean(value);omit("Approved exit/death-type omission: "+field.key); }
        else if(EqualToken(key,"HealthRegen%PerSec")||EqualToken(key,"ExitPitchRate"))
        {if(ScaledDecimal(value,1000000)!=0) reject();}
        else if(EqualToken(key,"NumberOfExitPaths")) {if(Integer(value,INT32_MAX)!=1) reject();}
        else if(EqualToken(key,"EnterSound")||EqualToken(key,"ExitSound")) omit("Presentation omission: "+field.key);
        else reject(); // Includes bones/doors, DieMux overrides and unknown fields.
    }
    if(!transport->children.empty()) {carrierSupported=false;omit("Nested TransportContain configuration is unsupported");}
    if(!definition.slots) {carrierSupported=false;omit("Slots is absent or zero; carrier not enrolled");}
    omit("Same-owner ground infantry only; no boarding/exit routing, default scatter, aggressive exit or mood reset");
    omit("ExitDelay modernization: authored milliseconds are ceiling-quantized directly at the finalized modern step, without an intermediate reference 30-Hz frame rounding; non-30-Hz deadlines can differ");
    omit("Carrier death damage uses passenger maximum health; burned-death presentation is omitted and integer-health rounding can differ from legacy float");
    if(carrierSupported) result.carrier=definition;
    else omit("Carrier not enrolled: unsupported ground-vehicle transport policy");
    return result;
}
}
