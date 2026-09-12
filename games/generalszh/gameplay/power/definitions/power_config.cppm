module;
#include <cstdint>

export module games.generalszh.gameplay.power.definitions.power_config;

export namespace generalszh::power
{
// Basis points, copied and validated once by PowerSystem. Content translation
// belongs to the game adapter, never the execution loop.
struct PowerConfig
{
    std::uint32_t minimumSpeed{5000}, maximumSpeed{8000}, penaltyModifier{10000};
};
}
