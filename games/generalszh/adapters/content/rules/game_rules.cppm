module;
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string_view>
export module games.generalszh.adapters.content.rules.game_rules;
export import games.generalszh.adapters.content.ini.named_block;
export import games.generalszh.gameplay.power.data.power_frame;
export namespace generalszh::content
{
// Decode once at the content boundary. The root copies these values into its
// immutable supply/power definitions before finalizing the shared gameplay plan.
struct DecodedGameRules
{
    std::uint32_t valuePerSupplyBox{};
    power::PowerConfig power{};
};
inline DecodedGameRules DecodeGameRules(std::string_view text)
{
    const auto block = ReadNamedBlock(text, "GameData", "");
    const auto box = ScaledDecimal(block.Require("ValuePerSupplyBox"), 1);
    const auto minimum = ScaledDecimal(block.Require("MinLowEnergyProductionSpeed"), 10000);
    const auto maximum = ScaledDecimal(block.Require("MaxLowEnergyProductionSpeed"), 10000);
    const auto penalty = ScaledDecimal(block.Require("LowEnergyPenaltyModifier"), 10000);
    if (box > (std::numeric_limits<std::uint32_t>::max)()
        || penalty > (std::numeric_limits<std::uint32_t>::max)())
        throw std::out_of_range("GameData supply value or power penalty exceeds runtime range");
    if (minimum > maximum || maximum > 10000)
        throw std::invalid_argument("GameData power speed bounds must be ordered within 0..1");
    return {static_cast<std::uint32_t>(box),
        {static_cast<std::uint32_t>(minimum), static_cast<std::uint32_t>(maximum),
            static_cast<std::uint32_t>(penalty)}};
}
}
