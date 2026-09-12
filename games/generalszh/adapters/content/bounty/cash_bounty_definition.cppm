module;

#include <cstdint>
#include <initializer_list>
#include <limits>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

export module games.generalszh.adapters.content.bounty.cash_bounty_definition;
import games.generalszh.adapters.content.ini.named_block;
import engine.gameplay.rts.bounty.algorithms.bounty_amount;
export import engine.gameplay.rts.bounty.components.bounty_policy;
export import engine.gameplay.rts.unlocks.definitions.unlock_definition;
export import games.generalszh.gameplay.bounty.components.cash_bounty_cost_binding;
export import games.generalszh.gameplay.bounty.definitions.cash_bounty_definition;
import games.generalszh.gameplay.construction.definitions.building_definition;
import games.generalszh.gameplay.production.definitions.build_definition;

export namespace generalszh::content
{
struct DecodedCashBountyDefinition final
{
    std::string specialPower;
    std::string scienceName;
    engine::gameplay::rts::bounty::BountyRate rate{};
};

struct DecodedCashBountyCatalog final
{
    std::vector<DecodedCashBountyDefinition> definitions;
};

namespace cash_bounty_detail
{
inline void RejectUnknownFields(const IniBlock &block,
    const std::initializer_list<std::string_view> allowed, const std::string_view blockName)
{
    for (const auto &field : block.fields)
    {
        bool known = false;
        for (const auto candidate : allowed)
            known = known || EqualToken(field.key, candidate);
        if (!known)
            throw std::invalid_argument("Unsupported cash bounty field in " + std::string(blockName) + ": " + field.key);
    }
    if (!block.children.empty())
        throw std::invalid_argument("Nested cash bounty fields are unsupported in " + std::string(blockName));
}

inline engine::gameplay::rts::bounty::BountyRate DecodeRate(const std::string_view authored)
{
    auto text = Trim(authored);
    if (text.empty() || text.back() != '%')
        throw std::invalid_argument("Cash bounty must be authored as a percentage");
    text.remove_suffix(1);
    constexpr std::uint64_t Scale = 1'000'000;
    constexpr std::uint64_t PercentDenominator = 100 * Scale;
    const auto scaledPercent = ScaledDecimal(text, Scale);
    if (scaledPercent > PercentDenominator ||
        scaledPercent > static_cast<std::uint64_t>((std::numeric_limits<std::uint32_t>::max)()))
        throw std::out_of_range("Cash bounty percentage is outside the supported [0,100] range");
    return engine::gameplay::rts::bounty::NormalizeBountyRate(
        static_cast<std::uint32_t>(scaledPercent), static_cast<std::uint32_t>(PercentDenominator));
}

inline DecodedCashBountyDefinition DecodeSpecialPower(const std::string_view specialPowerText,
    const std::string_view specialPower)
{
    const auto block = ReadNamedBlock(specialPowerText, "SpecialPower", specialPower);
    RejectUnknownFields(block, {"Enum", "RequiredScience", "PublicTimer"}, "SpecialPower");
    if (!EqualToken(block.Require("Enum"), "SPECIAL_CASH_BOUNTY"))
        throw std::invalid_argument("Cash bounty SpecialPower must use SPECIAL_CASH_BOUNTY");
    if (!EqualToken(block.Require("PublicTimer"), "No"))
        throw std::invalid_argument("Cash bounty SpecialPower requires PublicTimer = No");
    const auto science = Tokens(block.Require("RequiredScience"));
    if (science.size() != 1 || !engine::gameplay::rts::unlocks::IsValidUnlockName(science.front()))
        throw std::invalid_argument("Cash bounty RequiredScience must be one canonical science name");
    return {std::string{specialPower}, std::string{science.front()}, {}};
}
} // namespace cash_bounty_detail

inline DecodedCashBountyCatalog DecodeCashBountyDefinitions(
    const std::string_view objectText, const std::string_view specialPowerText,
    const std::string_view objectName)
{
    const auto object = ReadNamedBlock(objectText, "Object", objectName);
    DecodedCashBountyCatalog result;
    for (const auto &child : object.children)
    {
        const auto words = Tokens(child.argument);
        if (!EqualToken(child.kind, "Behavior") || words.empty() ||
            !EqualToken(words.front(), "CashBountyPower"))
            continue;
		cash_bounty_detail::RejectUnknownFields(child, {"SpecialPowerTemplate", "Bounty"}, "CashBountyPower");
        const auto specialPower = child.Require("SpecialPowerTemplate");
        const auto bounty = child.Require("Bounty");
        auto decoded = cash_bounty_detail::DecodeSpecialPower(specialPowerText, specialPower);
        decoded.rate = cash_bounty_detail::DecodeRate(bounty);
        for (const auto &existing : result.definitions)
            if (EqualToken(existing.specialPower, decoded.specialPower) ||
                EqualToken(existing.scienceName, decoded.scienceName))
                throw std::invalid_argument("Duplicate cash bounty SpecialPower or science");
        result.definitions.push_back(std::move(decoded));
    }
    return result;
}

inline std::vector<generalszh::bounty::CashBountyDefinition> BindCashBountyDefinitions(
    const DecodedCashBountyCatalog &source)
{
    std::vector<generalszh::bounty::CashBountyDefinition> result;
    result.reserve(source.definitions.size());
    for (const auto &decoded : source.definitions)
    {
        if (!engine::gameplay::rts::unlocks::IsValidUnlockName(decoded.scienceName) ||
            decoded.specialPower.empty())
            throw std::invalid_argument("Cash bounty binding contains an invalid stable name");
        result.push_back({engine::gameplay::rts::unlocks::MakeUnlockKey(decoded.scienceName), decoded.rate});
    }
    return result;
}

// The modern cost basis is copied from the immutable current BuildDefinition's
// decoded BuildCost. It is not a paid BuildOrder receipt and does not claim
// parity with legacy owner/KindOf/handicap modifiers. Eligibility is an
// explicit content decision: ignored GUI objects are never enrolled by
// accident and no veterancy state is reused.
inline generalszh::bounty::CashBountyCostBinding BindCashBountyCostBinding(
    const generalszh::production::BuildDefinition &definition, const bool ignoredInGui)
{
    return {definition.cost,
        generalszh::bounty::CashBountyCostBasis::DecodedBuildCost, !ignoredInGui};
}

inline generalszh::bounty::CashBountyCostBinding BindCashBountyCostBinding(
    const generalszh::construction::BuildingDefinition &definition, const bool ignoredInGui)
{
    return {definition.cost,
        generalszh::bounty::CashBountyCostBasis::DecodedBuildCost, !ignoredInGui};
}
} // namespace generalszh::content
