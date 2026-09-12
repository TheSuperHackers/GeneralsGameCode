module;
#include <algorithm>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
export module games.generalszh.adapters.content.upgrades.upgrade_definition;
export import games.generalszh.adapters.content.ini.named_block;
export import games.generalszh.gameplay.production.definitions.build_definition;

export namespace generalszh::content
{
enum class UpgradeScope : std::uint8_t
{
    Player,
    Object
};

// Content supplies this mapping explicitly. The numeric value is the same
// production BuildDefinition key carried by ResearchedUpgrade; it is never
// derived from registration order or a truncated name hash.
struct UpgradeIdentity
{
    std::string_view name{};
    std::uint32_t definition{};
    UpgradeScope scope{UpgradeScope::Player};
};

struct DecodedUpgrade
{
    std::string name;
    production::BuildDefinition definition;
    UpgradeScope scope{UpgradeScope::Player};
    std::vector<std::string> omissions;
};

inline void ValidateUpgradeIdentities(std::span<const UpgradeIdentity> identities)
{
    for(std::size_t i=0;i<identities.size();++i)
    {
        if(identities[i].name.empty()) throw std::invalid_argument("Upgrade identity name cannot be empty");
        for(std::size_t j=0;j<i;++j)
            if(EqualToken(identities[i].name,identities[j].name))
                throw std::invalid_argument("Upgrade identity names collide");
            else if(identities[i].definition==identities[j].definition)
                throw std::invalid_argument("Upgrade identity definition keys collide");
    }
}

inline const UpgradeIdentity *FindUpgradeIdentity(std::string_view name,
    std::span<const UpgradeIdentity> identities) noexcept
{
    for(const auto &identity:identities)
        if(EqualToken(identity.name,name)) return &identity;
    return nullptr;
}

// Narrow Upgrade.ini adapter used by the capture activation path. Upgrade
// definitions remain ordinary production entries; effect execution is outside
// this one-off slice.
inline DecodedUpgrade DecodeUpgrade(std::string_view text,std::string_view name,std::uint32_t definitionKey)
{
    const auto block=ReadNamedBlock(text,"Upgrade",name);
    DecodedUpgrade result; result.name=name; result.definition.key=definitionKey;
    result.definition.kind=production::EntryKind::Upgrade; result.definition.quantity=1;
    // UpgradeTemplate initializes m_type to PLAYER when Type is absent.
    for(const auto &field:block.fields)
    {
        const auto key=std::string_view(field.key),value=Trim(field.value);
        if(EqualToken(key,"BuildTime"))
        {
            const auto nanos=ScaledDecimal(value,1000000000);
            if(nanos>static_cast<std::uint64_t>((std::numeric_limits<std::int64_t>::max)()))
                throw std::out_of_range("Upgrade build duration exceeds runtime range");
            result.definition.duration=engine::time::Duration{static_cast<std::int64_t>(nanos)};
        }
        else if(EqualToken(key,"BuildCost"))
        {
            const auto cost=ScaledDecimal(value,1);
            if(cost>UINT32_MAX) throw std::out_of_range("Upgrade build cost exceeds runtime range");
            result.definition.cost=static_cast<std::uint32_t>(cost);
        }
        else if(EqualToken(key,"Type"))
        {
            if(EqualToken(value,"PLAYER")) result.scope=UpgradeScope::Player;
            else if(EqualToken(value,"OBJECT")) result.scope=UpgradeScope::Object;
            else throw std::invalid_argument("Upgrade Type must be PLAYER or OBJECT");
        }
        else result.omissions.push_back("Upgrade field: "+field.key);
    }
    if(result.definition.duration.count()<0) throw std::invalid_argument("Upgrade build duration is invalid");
    return result;
}

inline UpgradeIdentity Identity(const DecodedUpgrade &upgrade) noexcept
{ return {upgrade.name,upgrade.definition.key,upgrade.scope}; }
}
