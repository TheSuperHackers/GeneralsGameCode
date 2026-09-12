module;
#include <cstdint>
export module games.generalszh.gameplay.containment.definitions.garrison_definition;

export namespace generalszh::containment
{
// Immutable, catalog-bound first-slice building policy. Runtime occupancy and
// passenger lifecycle remain in PassengerMembership; no mutable garrison
// state is copied into Structure.
struct GarrisonDefinition
{
    std::uint32_t capacity{};
};

inline constexpr std::uint32_t GarrisonDefinitionVersion = 1;

constexpr bool IsValid(const GarrisonDefinition &definition) noexcept
{
    return definition.capacity != 0;
}
}
